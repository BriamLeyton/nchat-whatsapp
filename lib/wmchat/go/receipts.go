// receipts.go
//
// Copyright (c) 2026 Briam Leyton
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

package main

import (
	"context"
	"database/sql"
	"fmt"
	"sync"
	"time"

	_ "github.com/mattn/go-sqlite3"
	"go.mau.fi/whatsmeow"
	"go.mau.fi/whatsmeow/types"
)

// keep in sync with enum DeliveryStatus in protocol.h
var DeliveryStatusUnknown = -1
var DeliveryStatusNone = 0
var DeliveryStatusSent = 1
var DeliveryStatusDelivered = 2
var DeliveryStatusRead = 3

// groups larger than this are aggregated on first receipt, without per-participant tracking
var receiptsMaxTrackedRecipients = 256
var receiptsRetentionDays = 60

type groupRecipients struct {
	count     int
	canonical map[string]string // participant user (phone number or lid) -> canonical user
}

var (
	rmx        sync.Mutex
	receiptDbs map[int]*sql.DB                     = make(map[int]*sql.DB)
	groupInfos map[int]map[string]*groupRecipients = make(map[int]map[string]*groupRecipients)
)

func GetReceiptsStorePath(path string) string {
	return path + "/receipts.db"
}

func OpenReceiptsDb(connId int, path string) {
	dbPath := GetReceiptsStorePath(path)
	db, err := sql.Open("sqlite3", fmt.Sprintf("file:%s?_journal_mode=WAL&_busy_timeout=5000", dbPath))
	if err != nil {
		LOG_WARNING(fmt.Sprintf("receipts db open error %#v", err))
		return
	}

	db.SetMaxOpenConns(1)
	stmts := []string{
		"CREATE TABLE IF NOT EXISTS msgstatus (chatId TEXT, msgId TEXT, status INT, recipients INT, " +
			"timeUpdated INT, PRIMARY KEY(chatId, msgId));",
		"CREATE TABLE IF NOT EXISTS participantreceipts (chatId TEXT, msgId TEXT, participant TEXT, status INT, " +
			"timeUpdated INT, PRIMARY KEY(chatId, msgId, participant));",
	}
	for _, stmt := range stmts {
		if _, err := db.Exec(stmt); err != nil {
			LOG_WARNING(fmt.Sprintf("receipts db init error %#v", err))
			db.Close()
			return
		}
	}

	// prune old entries
	cutoff := time.Now().AddDate(0, 0, -receiptsRetentionDays).Unix()
	db.Exec("DELETE FROM msgstatus WHERE timeUpdated < ?;", cutoff)
	db.Exec("DELETE FROM participantreceipts WHERE timeUpdated < ?;", cutoff)

	rmx.Lock()
	receiptDbs[connId] = db
	groupInfos[connId] = make(map[string]*groupRecipients)
	rmx.Unlock()
}

func CloseReceiptsDb(connId int) {
	rmx.Lock()
	db := receiptDbs[connId]
	delete(receiptDbs, connId)
	delete(groupInfos, connId)
	rmx.Unlock()

	if db != nil {
		db.Close()
	}
}

func getReceiptsDb(connId int) *sql.DB {
	rmx.Lock()
	db := receiptDbs[connId]
	rmx.Unlock()
	return db
}

// GetStoredDeliveryStatus returns status recorded from receipts, or DeliveryStatusUnknown if none.
func GetStoredDeliveryStatus(connId int, chatId string, msgId string) int {
	db := getReceiptsDb(connId)
	if db == nil {
		return DeliveryStatusUnknown
	}

	status := DeliveryStatusUnknown
	row := db.QueryRow("SELECT status FROM msgstatus WHERE chatId = ? AND msgId = ?;", chatId, msgId)
	if err := row.Scan(&status); err != nil {
		return DeliveryStatusUnknown
	}

	return status
}

// GetOutgoingDeliveryStatus returns the initial status for a new message notification.
func GetOutgoingDeliveryStatus(connId int, chatId string, msgId string, fromMe bool, isSyncRead bool) int {
	if !fromMe || isSyncRead {
		return DeliveryStatusUnknown
	}

	status := GetStoredDeliveryStatus(connId, chatId, msgId)
	if status < DeliveryStatusSent {
		status = DeliveryStatusSent
	}

	return status
}

// getGroupRecipients returns number of group participants excluding self, cached per session.
func getGroupRecipients(connId int, client *whatsmeow.Client, chatJid types.JID, chatId string) *groupRecipients {
	rmx.Lock()
	info, ok := groupInfos[connId][chatId]
	rmx.Unlock()
	if ok {
		return info
	}

	info = &groupRecipients{count: 1, canonical: make(map[string]string)}
	groupInfo, err := client.GetGroupInfo(context.Background(), chatJid)
	if err != nil || groupInfo == nil {
		LOG_WARNING(fmt.Sprintf("get group info %s failed %#v, assuming single recipient", chatId, err))
	} else {
		selfUsers := map[string]bool{}
		if client.Store.ID != nil {
			selfUsers[client.Store.ID.User] = true
		}
		if !client.Store.LID.IsEmpty() {
			selfUsers[client.Store.LID.User] = true
		}

		count := 0
		for _, participant := range groupInfo.Participants {
			canonical := participant.JID.User
			isSelf := selfUsers[participant.JID.User] || selfUsers[participant.PhoneNumber.User] ||
				selfUsers[participant.LID.User]
			if isSelf {
				continue
			}

			count++
			for _, user := range []string{participant.JID.User, participant.PhoneNumber.User, participant.LID.User} {
				if user != "" {
					info.canonical[user] = canonical
				}
			}
		}

		if count > 0 {
			info.count = count
		}
	}

	rmx.Lock()
	if groupInfos[connId] != nil {
		groupInfos[connId][chatId] = info
	}
	rmx.Unlock()
	return info
}

// UpdateDeliveryStatus records a receipt and returns the aggregated message status, and whether it increased.
func UpdateDeliveryStatus(connId int, client *whatsmeow.Client, chatJid types.JID, chatId string, msgId string,
	sender types.JID, status int, isGroup bool) (int, bool) {

	db := getReceiptsDb(connId)
	if db == nil {
		return status, true
	}

	curStatus := DeliveryStatusNone
	recipients := 0
	row := db.QueryRow("SELECT status, recipients FROM msgstatus WHERE chatId = ? AND msgId = ?;", chatId, msgId)
	row.Scan(&curStatus, &recipients)

	now := time.Now().Unix()
	newStatus := status
	if isGroup {
		var group *groupRecipients
		if recipients == 0 || recipients <= receiptsMaxTrackedRecipients {
			group = getGroupRecipients(connId, client, chatJid, chatId)
		}

		if recipients == 0 {
			// fixed on first receipt, so later membership changes do not affect sent messages
			recipients = group.count
		}

		if recipients <= receiptsMaxTrackedRecipients {
			participant := sender.ToNonAD().User
			if canonical, ok := group.canonical[participant]; ok {
				participant = canonical
			}

			_, err := db.Exec("INSERT INTO participantreceipts (chatId, msgId, participant, status, timeUpdated) "+
				"VALUES (?, ?, ?, ?, ?) ON CONFLICT(chatId, msgId, participant) "+
				"DO UPDATE SET status = MAX(status, excluded.status), timeUpdated = excluded.timeUpdated;",
				chatId, msgId, participant, status, now)
			if err != nil {
				LOG_WARNING(fmt.Sprintf("receipts db insert error %#v", err))
			}

			readCount := 0
			deliveredCount := 0
			countRow := db.QueryRow("SELECT COALESCE(SUM(status >= ?), 0), COALESCE(SUM(status >= ?), 0) "+
				"FROM participantreceipts WHERE chatId = ? AND msgId = ?;",
				DeliveryStatusRead, DeliveryStatusDelivered, chatId, msgId)
			countRow.Scan(&readCount, &deliveredCount)

			if readCount >= recipients {
				newStatus = DeliveryStatusRead
			} else if deliveredCount >= recipients {
				newStatus = DeliveryStatusDelivered
			} else {
				newStatus = DeliveryStatusSent
			}

			LOG_TRACE(fmt.Sprintf("group receipt %s %s read %d delivered %d of %d", chatId, msgId,
				readCount, deliveredCount, recipients))
		}
	} else {
		recipients = 1
	}

	if newStatus < curStatus {
		newStatus = curStatus
	}

	_, err := db.Exec("INSERT INTO msgstatus (chatId, msgId, status, recipients, timeUpdated) "+
		"VALUES (?, ?, ?, ?, ?) ON CONFLICT(chatId, msgId) "+
		"DO UPDATE SET status = MAX(status, excluded.status), recipients = excluded.recipients, "+
		"timeUpdated = excluded.timeUpdated;",
		chatId, msgId, newStatus, recipients, now)
	if err != nil {
		LOG_WARNING(fmt.Sprintf("receipts db update error %#v", err))
	}

	return newStatus, (newStatus > curStatus)
}

// uimodel.h
//
// Copyright (c) 2019-2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <deque>
#include <mutex>
#include <set>
#include <stack>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "apputil.h"
#include "owned_mutex.h"
#include "protocol.h"

class UiView;

// chat list sections, in display order
enum ChatSection
{
  ChatSectionStatus = 0, // status updates, listed first without a section title
  ChatSectionPinned = 1,
  ChatSectionOthers = 2,
  ChatSectionArchived = 3,
  ChatSectionLocked = 4, // whatsapp locked chats, hidden unless shown with secret code
  ChatSectionCount = 5,
};

// presence of contact, for chat banner colors
enum PresenceType
{
  PresenceNone = 0,
  PresenceTyping,
  PresenceOnline,
  PresenceAway,
  PresenceSeen,
};

// chat banner shown on top of history (contact or group chat)
struct ChatBanner
{
  bool isGroup = false;
  std::string name;
  std::string phone;
  std::string presence;
  int presenceType = PresenceNone;
  bool isMuted = false;
  int memberCount = 0;
  int statusTotal = 0; // status updates of last 24 hours (contact)
  int statusNew = 0;
  std::string avatarPath; // empty if no photo
};

class UiModel
{
private:
  class Impl
  {
  public:
    Impl(UiModel* p_UiModel);
    virtual ~Impl();

    void Init();
    void Cleanup();

    void AnyUserKeyInput();
    void TerminalResize();
    void OnKeyDecreaseListWidth();
    void OnKeyIncreaseListWidth();
    void OnKeyToggleOthers();
    void OnKeyToggleArchived();
    void ToggleSectionCollapsed(int p_Section);
    int GetChatSection(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool IsStatusChat(const std::string& p_ChatId);
    int GetFirstChatIndex();
    bool IsExpiredStatus(const std::string& p_ChatId, const ChatMessage& p_ChatMessage);
    void SetPendingNotice(const std::string& p_Notice);
    std::string TakePendingNotice();
    bool IsStatusFilterActive(const std::string& p_ProfileId, const std::string& p_ChatId);
    std::vector<std::string>& GetShownMessageVec(const std::string& p_ProfileId, const std::string& p_ChatId);
    void RebuildStatusFilter(const std::string& p_ProfileId, const std::string& p_ChatId);
    void FillStatusFilter();
    void SetStatusFilter(const std::string& p_SenderId);
    void ClearStatusFilter();
    void QueueStatusPrefetch(const std::string& p_ProfileId, const std::string& p_ChatId,
                             const std::string& p_MsgId);
    bool IsStatusPrefetchWanted(const std::string& p_ProfileId, const std::string& p_ChatId,
                                const std::string& p_MsgId);
    void ProcessStatusPrefetch();
    void StartStatusVideoCleanup();
    void ProcessStatusVideoCleanup();
    void SetStatusFilterFromSelected();
    std::string GetStatusFilterSenderId();
    bool GetSectionCollapsed(int p_Section);
    void SetSectionCollapsed(int p_Section, bool p_Collapsed);
    bool IsChatHidden(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool SelectNearestVisibleChat();
    std::unordered_set<std::string>& GetLockedChats(const std::string& p_ProfileId);
    void WriteLockedChats(const std::string& p_ProfileId);
    bool IsChatLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool IsChatLockedHidden(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool GetLockedVisible();
    void HideLockedChats();
    void CheckSecretCode(const std::string& p_Code);
    void OnKeyToggleList();
    void OnKeyToggleTop();
    void OnKeyToggleEmoji();
    void OnKeySendMsg();
    void SendMessage();
    void OnKeyOtherCommandsHelp();
    void EntryKeyHandler(wint_t p_Key);
    void SetTyping(const std::string& p_ProfileId, const std::string& p_ChatId, bool p_IsTyping);

    void OnKeyNextChat();
    void OnKeyPrevChat();
    void OnKeyUnreadChat();
    void OnKeyPrevPage();
    void OnKeyNextPage();
    bool OnKeyHomeFetchCache();
    void OnKeyHomeFetchNext();
    void OnKeyEnd();
    void HomeFetchNext(const std::string& p_ProfileId, const std::string& p_ChatId, int p_MsgCount);
    void MarkRead(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_MsgId,
                  bool p_WasUnread,
                  bool p_Opened = false);
    void MarkStatusOpened();
    void SendStatusReply();
    bool IsStatusNew(const std::string& p_ProfileId, const std::string& p_ChatId, const ChatMessage& p_ChatMessage);
    void MarkStatusSeen(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_MsgId);
    void OnStatusUpdate(uint32_t p_Status);
    void DownloadAttachment(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_MsgId,
                            const std::string& p_FileId, DownloadFileAction p_DownloadFileAction);
    void OnKeyDeleteMsg();
    void OnKeyDeleteChat();
    void OnKeyArchiveChat();
    void OnKeyPinChat();
    void OnKeyOpenMsg();
    bool GetMessageAttachmentPath(std::string& p_FilePath, DownloadFileAction p_DownloadFileAction);
    void OnKeyOpenAttachment(std::string p_FilePath = std::string());
    void OpenLink(const std::string& p_Url);
    void OpenAttachment(const std::string& p_Path);
    bool RunCommand(const std::string& p_Cmd, std::string* p_StdOut = nullptr);
    void RunProgram(const std::string& p_Cmd);
    void OnKeyOpenLink();
    std::string OnKeySaveAttachment(std::string p_FilePath = std::string());
    void TransferFile(const std::vector<std::string>& p_FilePaths);
    void InsertEmoji(const std::wstring& p_Emoji);
    void InsertText(const std::wstring& p_Text);
    void RequestGroupMembers(const std::string& p_ProfileId, const std::string& p_ChatId);
    std::vector<std::string> GetGroupMembers(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool GetChatInfoIsGroup(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool HasProtocolFeature(const std::string& p_ProfileId, ProtocolFeature p_ProtocolFeature);
    std::map<std::string, std::string> ParseMentions(const std::string& p_ProfileId, const std::string& p_ChatId,
                                                     const std::string& p_Text);
    void OpenCreateChat(const std::pair<std::string, std::string>& p_Chat);
    void OpenChatByPhone(const std::string& p_ProfileId, const std::string& p_Phone, const std::string& p_Text);
    std::string GetDefaultProfileId();
    void ApplyLinkText(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_Text);
    void FetchCachedMessage(const std::string& p_ProfileId, const std::string& p_ChatId,
                            const std::string& p_MsgId);

    void MessageHandler(std::shared_ptr<ServiceMessage> p_ServiceMessage);
    void AddProtocol(std::shared_ptr<Protocol> p_Protocol);
    std::unordered_map<std::string, std::shared_ptr<Protocol>> GetProtocols();
    bool Process();
    void ProcessTimers();

    std::string GetLastMessageId(const std::string& p_ProfileId, const std::string& p_ChatId);
    void UpdateChatInfoLastMessageTime(const std::string& p_ProfileId, const std::string& p_ChatId);
    void UpdateChatInfoIsUnread(const std::string& p_ProfileId, const std::string& p_ChatId);
    std::string GetContactName(const std::string& p_ProfileId, const std::string& p_ChatId);
    std::string GetContactNameIncludingSelf(const std::string& p_ProfileId, const std::string& p_ChatId);
    std::string GetContactListName(const std::string& p_ProfileId, const std::string& p_ChatId, bool p_AllowId,
                                   bool p_AllowAlias);
    std::string GetContactPhone(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool IsContactSelf(const std::string& p_ProfileId, const std::string& p_ContactId);
    int64_t GetLastMessageTime(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool GetChatIsUnread(const std::string& p_ProfileId, const std::string& p_ChatId);
    std::string GetChatStatus(const std::string& p_ProfileId, const std::string& p_ChatId);
    std::string GetChatPresence(const std::string& p_ProfileId, const std::string& p_ChatId, int& p_PresenceType);
    bool IsBannerChat(const std::string& p_ProfileId, const std::string& p_ChatId);
    bool GetChatBanner(const std::string& p_ProfileId, const std::string& p_ChatId, ChatBanner& p_ChatBanner);
    std::string GetAvatarBasePath(const std::string& p_ProfileId, const std::string& p_ChatId);
    void RequestAvatar(const std::string& p_ProfileId, const std::string& p_ChatId);
    void OpenAvatar();
    void UpdateStatusCounts(const std::string& p_ProfileId, const std::string& p_ContactId);
    bool HasStatusUpdates(const std::string& p_ProfileId, const std::string& p_ContactId);
    void SetStatusReturnChat(const std::pair<std::string, std::string>& p_Chat);
    std::pair<std::string, std::string> TakeStatusReturnChat();

    std::wstring& GetEntryStr();
    int& GetEntryPos();

    std::vector<std::pair<std::string, std::string>>& GetChatVec();
    std::vector<std::pair<std::string, std::string>>& GetChatVecLock();
    std::unordered_map<std::string, std::unordered_map<std::string, ContactInfo>> GetContactInfos();
    int64_t GetContactInfosUpdateTime();
    int64_t GetGroupMembersUpdateTime();
    std::pair<std::string, std::string>& GetCurrentChat();
    bool IsCurrentChat(const std::string& p_ProfileId, const std::string& p_ChatId);
    int& GetCurrentChatIndex();

    std::unordered_map<std::string, ChatMessage>& GetMessages(const std::string& p_ProfileId,
                                                              const std::string& p_ChatId);
    std::vector<std::string>& GetMessageVec(const std::string& p_ProfileId, const std::string& p_ChatId);
    int& GetMessageOffset(const std::string& p_ProfileId, const std::string& p_ChatId);
    int GetHistoryViewOffset(const std::string& p_ProfileId, const std::string& p_ChatId);
    void SetHistoryViewOffset(const std::string& p_ProfileId, const std::string& p_ChatId, int p_ViewOffset);

    void SetStatusOnline(const std::string& p_ProfileId, bool p_IsOnline);
    void RequestContacts();
    int GetScreenWidth();
    int GetScreenHeight();
    void SetRunning(bool p_Running);

    void SetSelectMessageActive(bool p_SelectMessageActive);
    bool GetSelectMessageActive();

    bool GetListDialogActive();
    void SetListDialogActive(bool p_ListDialogActive);

    bool GetFileListDialogActive();
    void SetFileListDialogActive(bool p_FileListDialogActive);

    bool GetMessageDialogActive();
    void SetMessageDialogActive(bool p_MessageDialogActive);

    bool GetEditMessageActive();
    void SetEditMessageActive(bool p_EditMessageActive);

    bool GetFindMessageActive();
    void SetFindMessageActive(bool p_FindMessageActive);

    void SetHelpOffset(int p_HelpOffset);
    int GetHelpOffset();

    bool GetEmojiEnabled();
    bool GetEmojiEnabledLock();
    void SetTerminalActive(bool p_TerminalActive);

    bool IsMultipleProfiles();
    std::string GetProfileDisplayName(const std::string& p_ProfileId);
    std::string GetProfileSuffix(const std::string& p_ProfileId);
    void GetAvailableEmojis(std::set<std::string>& p_AvailableEmojis, bool& p_Pending);
    void OnKeyJumpQuoted();
    void OnKeyJumpPinned();
    void OnKeyPinMsg();

    void Draw();
    void ReinitView();
    void TerminalControlPause();
    void TerminalControlResume();

    void OnKeyQuit();
    void OnKeyExtEdit();
    void Cut();
    void Copy();
    void Paste();
    void OnKeyCancel();
    bool PreEditMsg(std::string& p_ProfileId, std::string& p_ChatId, std::string& p_MsgId,
                    std::string& p_MsgDialogText);
    void StartEditMsg(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_MsgId);
    void OnKeySpell();
    void OnKeyExtCall();
    std::string PreExtCall();
    void StartExtCall(const std::string& p_Phone);

    void GotoChat(const std::pair<std::string, std::string>& p_Chat);
    bool PreReact(std::string& p_ProfileId, std::string& p_ChatId, std::string& p_SenderId,
                  std::string& p_MsgId, std::string& p_SelfEmoji, bool& p_HasLimitedReactions);
    void SetReact(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_SenderId,
                  const std::string& p_MsgId, const std::string& p_SelfEmoji, const std::string& p_Emoji);
    void Find(const std::string& p_FindText);
    void ForwardMessage(const std::pair<std::string, std::string>& p_Chat);
    void PerformForwardMessage(const std::pair<std::string, std::string>& p_Chat);
    bool IsProtocolUiControlActive();
    void HandleProtocolUiControlStart();
    void HandleProtocolUiControlEnd();
    bool AutoCompose();

    static bool IsAttachmentDownloaded(const FileInfo& p_FileInfo);
    static bool IsAttachmentDownloadable(const FileInfo& p_FileInfo, bool p_AllowRetryFailed = false);
    static void SanitizeEntryStr(std::string& p_Str);

  private:
    void SortChats();
    void OnCurrentChatChanged();
    void RequestMessagesCurrentChat();
    void RequestMessagesNextChat();
    void RequestMessages(const std::string& p_ProfileId, const std::string& p_ChatId);
    void RequestUserStatusCurrentChat();
    void RequestUserStatusNextChat();
    void RequestUserStatus(const std::pair<std::string, std::string>& p_Chat);
    void ProtocolSetCurrentChat();
    int GetHistoryLines();
    void UpdateList();
    void UpdateStatus();
    void UpdateHelp();
    void UpdateHistory();
    void UpdateEntry();
    void ResetMessageOffset();
    bool IsCurrentChatSet();
    bool SetCurrentChatIndexIfNotSet();
    void DesktopNotify(const std::string& p_Name, const std::string& p_Text);
    void SetHistoryInteraction(bool p_HistoryInteraction);
    std::string GetSelectedMessageText();
    void Clear(bool p_AllowUndo);
    void SaveEditMessage();
    std::string EntryStrToSendStr(const std::wstring& p_EntryStr);
    void CallExternalEdit(const std::string& p_EditorCmd);
    const std::pair<std::string, std::string>& GetNextChat();
    void SendProtocolRequest(const std::string& p_ProfileId, std::shared_ptr<RequestMessage> p_Request);
    std::string GetSelfId(const std::string& p_ProfileId);
    void Quit();
    void EntryConvertEmojiEnabled();
    void SetProtocolUiControl(const std::string& p_ProfileId, bool& p_IsTakeControl);
    void PerformFindNext(const std::string& p_FindText);
    bool IsChatForceHidden(const std::string& p_ChatId);
    bool IsChatForceMuted(const std::string& p_ChatId);
    void AddQuoteFromSelectedMessage(ChatMessage& p_ChatMessage);

  private:
    bool m_Running = true;
    std::shared_ptr<UiView> m_View;
    int m_TermLines = 0;
    int m_TermCols = 0;

    std::unordered_map<std::string, std::shared_ptr<Protocol>> m_Protocols;

    std::unordered_map<std::string, std::unordered_set<std::string>> m_ChatSet;
    std::vector<std::pair<std::string, std::string>> m_ChatVec;
    std::unordered_map<std::string, std::unordered_map<std::string, ChatInfo>> m_ChatInfos;
    std::unordered_map<std::string, std::unordered_map<std::string, ContactInfo>> m_ContactInfos;
    int64_t m_ContactInfosUpdateTime = 0;
    std::unordered_map<std::string, int64_t> m_ConnectTime;
    int64_t m_LastSyncMessageTime = 0;

    std::pair<std::string, std::string> m_CurrentChat;
    int m_CurrentChatIndex = -1;
    bool m_SectionCollapsed[ChatSectionCount] = { false };
    int m_SectionChatCount[ChatSectionCount] = { 0 };
    static const std::pair<std::string, std::string> s_ChatNone;

    std::string m_EditMessageId;
    std::string m_ProtocolUiControl;

    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> m_MessageVec;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::unordered_map<std::string, ChatMessage>>> m_Messages;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> m_MessageOffset;
    std::unordered_map<std::string, std::unordered_map<std::string, std::stack<int>>> m_MessageOffsetStack;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> m_HistoryViewOffset;

    // status updates filtered by sender, empty sender id when inactive
    std::pair<std::string, std::string> m_StatusFilterChat;
    std::string m_StatusFilterSenderId;
    std::vector<std::string> m_StatusFilterVec;
    std::unordered_set<std::string> m_StatusFilterLoadedIds;
    bool m_StatusFilterExhausted = false;

    // status updates seen in list (selected, not opened), local only, persisted in <profile>/status_seen
    struct StatusSeen
    {
      bool loaded = false;
      int64_t baseline = 0; // status updates sent before this time are never new
      std::unordered_map<std::string, int64_t> msgTimes;
    };
    StatusSeen& GetStatusSeen(const std::string& p_ProfileId);
    void WriteStatusSeen(const std::string& p_ProfileId);
    std::unordered_map<std::string, StatusSeen> m_StatusSeen;

    // chat banner: avatars requested this session, pending avatar open, status counts per contact
    std::unordered_map<std::string, std::unordered_set<std::string>> m_AvatarRequested;
    std::pair<std::string, std::string> m_AvatarOpenChat;
    // locked chats by profile (loaded from profile dir on first use), and whether shown this session
    std::unordered_map<std::string, std::unordered_set<std::string>> m_LockedChats;
    std::unordered_set<std::string> m_LockedChatsLoaded;
    bool m_LockedVisible = false;
    // message text from opened link, by profile and phone number, applied when chat is created
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_PendingLinkText;
    std::unordered_map<std::string, std::unordered_map<std::string, std::pair<int, int>>> m_StatusCounts;
    std::pair<std::string, std::string> m_StatusReturnChat; // chat to return to from status filter (Alt-u)

    // status prefetch queue (one download at a time), entries are profile id, chat id, msg id
    std::deque<std::tuple<std::string, std::string, std::string>> m_StatusPrefetchQueue;
    std::unordered_set<std::string> m_StatusPrefetchQueued;
    std::string m_StatusPrefetchInFlightId;
    int64_t m_StatusPrefetchInFlightTime = 0;

    // status video cleanup (background), results applied to loaded messages under model mutex
    bool m_StatusVideoCleanupStarted = false;
    std::thread m_StatusVideoCleanupThread;
    std::mutex m_StatusVideoCleanupMutex;
    std::vector<std::tuple<std::string, std::string, std::string>> m_StatusVideoCleanupResults;

    // notice from key action, shown after the action without model mutex held
    std::string m_PendingNotice;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::unordered_set<std::string>>> m_MsgFromIdsRequested;
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> m_FetchedAllCache;

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_OldestMessageId;
    std::unordered_map<std::string, std::unordered_map<std::string, int64_t>> m_OldestMessageTime;

    std::unordered_map<std::string, std::unordered_map<std::string, std::wstring>> m_EntryStr;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> m_EntryPos;

    std::unordered_map<std::string, std::unordered_map<std::string, std::wstring>> m_EntryStrCleared;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> m_EntryPosCleared;

    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> m_UsersTyping;
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> m_UserOnline;
    std::unordered_map<std::string, std::unordered_map<std::string, int64_t>> m_UserTimeSeen;

    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> m_GroupMembers;
    int64_t m_GroupMembersUpdateTime = 0;

    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> m_AvailableReactions;
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> m_AvailableReactionsPending;

    bool m_SelectMessageActive = false;
    bool m_ListDialogActive = false;
    bool m_FileListDialogActive = false;
    bool m_MessageDialogActive = false;
    bool m_EditMessageActive = false;
    bool m_FindMessageActive = false;

    bool m_TriggerTerminalBell = false;
    bool m_HomeFetchAll = false;
    bool m_TerminalActive = true;
    bool m_HistoryInteraction = false;

    int m_HelpOffset = 0;
  };

public:
  // Methods acquiring model mutex as needed (intended for main, Ui, Ui*Dialog classes)
  UiModel();
  virtual ~UiModel();
  void Init();
  void Cleanup();
  void ReinitView();

  void AddProtocol(std::shared_ptr<Protocol> p_Protocol);
  std::unordered_map<std::string, std::shared_ptr<Protocol>> GetProtocols();
  bool IsMultipleProfiles();
  std::string GetProfileSuffix(const std::string& p_ProfileId);

  void Draw();
  void KeyHandler(wint_t p_Key);
  void MessageHandler(std::shared_ptr<ServiceMessage> p_ServiceMessage);
  bool Process();
  void RequestContacts();
  int GetScreenWidth();
  int GetScreenHeight();

  void GetAvailableEmojis(std::set<std::string>& p_AvailableEmojis, bool& p_Pending);
  std::vector<std::pair<std::string, std::string>> GetChatVec();
  std::string GetContactListName(const std::string& p_ProfileId, const std::string& p_ChatId, bool p_AllowId,
                                 bool p_AllowAlias);
  bool IsContactSelf(const std::string& p_ProfileId, const std::string& p_ContactId);
  std::unordered_map<std::string, std::unordered_map<std::string, ContactInfo>> GetContactInfos();
  std::string GetProfileDisplayName(const std::string& p_ProfileId);

  int64_t GetContactInfosUpdateTime();
  int64_t GetGroupMembersUpdateTime();
  void RequestGroupMembers(const std::string& p_ProfileId, const std::string& p_ChatId);
  std::vector<std::string> GetGroupMembers(const std::string& p_ProfileId, const std::string& p_ChatId);
  bool GetEmojiEnabled();
  int GetHelpOffset();
  void SetHelpOffset(int p_HelpOffset);
  void SetMessageDialogActive(bool p_MessageDialogActive);
  void SetListDialogActive(bool p_ListDialogActive);
  void SetFileListDialogActive(bool p_FileListDialogActive);
  void SetStatusOnline(const std::string& p_ProfileId, bool p_IsOnline);
  void SetTerminalActive(bool p_TerminalActive);

  // Locked methods require caller to hold model mutex (intended for Ui*View classes)
  bool GetChatIsUnreadLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  int GetChatSectionLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  bool GetSectionCollapsedLocked(int p_Section);
  bool IsChatHiddenLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  bool GetLockedVisibleLocked();
  bool IsChatLockedHidden(const std::string& p_ProfileId, const std::string& p_ChatId);
  std::string GetChatStatusLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  bool GetChatBannerLocked(const std::string& p_ProfileId, const std::string& p_ChatId, ChatBanner& p_ChatBanner);
  std::vector<std::pair<std::string, std::string>>& GetChatVecLocked();
  std::string GetContactListNameLocked(const std::string& p_ProfileId, const std::string& p_ChatId, bool p_AllowId,
                                       bool p_AllowAlias);
  std::string GetContactNameLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  std::string GetContactPhoneLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  int GetCurrentChatIndexLocked();
  std::pair<std::string, std::string>& GetCurrentChatLocked();
  bool GetEditMessageActiveLocked();
  bool GetEmojiEnabledLocked();
  int GetEntryPosLocked();
  std::wstring GetEntryStrLocked();
  int GetHelpOffsetLocked();
  int64_t GetLastMessageTimeLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  bool GetListDialogActiveLocked();
  bool GetFileListDialogActiveLocked();
  bool GetMessageDialogActiveLocked();
  int GetMessageOffsetLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  bool IsExpiredStatusLocked(const std::string& p_ChatId, const ChatMessage& p_ChatMessage);
  std::string GetStatusFilterSenderIdLocked();
  int GetHistoryViewOffsetLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  void SetHistoryViewOffsetLocked(const std::string& p_ProfileId, const std::string& p_ChatId, int p_ViewOffset);
  std::unordered_map<std::string, ChatMessage>& GetMessagesLocked(const std::string& p_ProfileId,
                                                                  const std::string& p_ChatId);
  std::vector<std::string>& GetMessageVecLocked(const std::string& p_ProfileId, const std::string& p_ChatId);
  std::string GetProfileDisplayNameLocked(const std::string& p_ProfileId);
  std::string GetProfileSuffixLocked(const std::string& p_ProfileId);
  bool GetSelectMessageActiveLocked();

  void DownloadAttachmentLocked(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_MsgId,
                                const std::string& p_FileId, DownloadFileAction p_DownloadFileAction);
  void FetchCachedMessageLocked(const std::string& p_ProfileId, const std::string& p_ChatId,
                                const std::string& p_MsgId);
  bool IsMultipleProfilesLocked();
  void MarkReadLocked(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_MsgId,
                      bool p_WasUnread);
  bool IsStatusNewLocked(const std::string& p_ProfileId, const std::string& p_ChatId,
                         const ChatMessage& p_ChatMessage);
  void MarkStatusSeenLocked(const std::string& p_ProfileId, const std::string& p_ChatId, const std::string& p_MsgId);
  void OnStatusUpdateLocked(uint32_t p_Status);

  // Static methods
  static bool IsAttachmentDownloaded(const FileInfo& p_FileInfo);
  static bool IsAttachmentDownloadable(const FileInfo& p_FileInfo, bool p_AllowRetryFailed = false);

private:
  // Encapsulate class data in a nested class using OImpl (Object Implementation, non-opaque
  // variant of PImpl avoiding pointer indirection) for clear separation of internal data access
  // (where ownership of mutex can be assumed) and external interface (managing mutex).
  inline Impl& GetImpl()
  {
#ifndef NCHAT_BUILD_RELEASE
    nc_assert(m_ModelMutex.owns_lock());
#endif
    return m_Impl;
  }

  void OnKeyGotoChat();
  std::vector<std::string> SelectFile();
  void OnKeyTransfer();
  void OnKeySelectEmoji();
  void OnKeySelectContact();
  void OnKeyReact();
  void OnKeyFind();
  void OnKeyToggleLocked();
  void OnKeyFindNext();
  void OnKeyForwardMsg();
  bool MessageDialog(const std::string& p_Title, const std::string& p_Text, float p_WReq, float p_HReq);
  void OnKeyDeleteMsg();
  void OnKeyDeleteChat();
  void OnKeyToggleHelp();
  void OnKeyArchiveChat();
  void OnKeyPin();
  void OnKeySaveAttachment();
  void OnKeyEditMsg();
  void OnKeyQuit();
  void OnKeyExtCall();
  void OnKeyAutoCompose();
  void OnKeySelectMention();
  void OnKeyStatusFilter();
  void OnKeyCut();
  void OnKeyCopy();
  void OnKeyPaste();

private:
  Impl m_Impl;
  owned_mutex m_ModelMutex;

  std::string m_FindText;
};

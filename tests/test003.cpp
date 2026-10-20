// test003.cpp - file category classification
//
// Copyright (c) 2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include <iostream>
#include <string>

#include "fileutil.h"

#include "unittest.h"

static void TestCategories()
{
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("IMG-2026.jpg"), FileCategoryImage);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("sticker.webp"), FileCategoryImage);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("VID-01.mp4"), FileCategoryVideo);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("PTT-01.ogg"), FileCategoryAudio);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("cancion.mp3"), FileCategoryAudio);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("factura.pdf"), FileCategoryPdf);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("informe.docx"), FileCategoryDocument);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("notas.txt"), FileCategoryDocument);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("hoja.xlsx"), FileCategorySpreadsheet);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("datos.csv"), FileCategorySpreadsheet);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("backup.zip"), FileCategoryArchive);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("backup.tar.gz"), FileCategoryArchive);
}

static void TestCaseInsensitive()
{
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("Factura.PDF"), FileCategoryPdf);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("IMG.JpEg"), FileCategoryImage);
}

static void TestOther()
{
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("firmware.bin"), FileCategoryOther);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("LEEME"), FileCategoryOther);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("nombre."), FileCategoryOther);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory(""), FileCategoryOther);
}

static void TestPath()
{
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("/tmp/a.zip/LEEME"), FileCategoryOther);
  unittest::ExpectEqual(int, FileUtil::GetFileCategory("/home/user/.config/nchat/tmp/foto.png"),
                        FileCategoryImage);
}

int main()
{
  int rv = 0;

  try
  {
    TestCategories();
    TestCaseInsensitive();
    TestOther();
    TestPath();
  }
  catch (const std::exception& ex)
  {
    std::cout << "exception: " << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}

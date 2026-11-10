/*
  Copyright (c) 1990-2009 Info-ZIP.  All rights reserved.
*/

#include <intrin.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <format>
#include <array>
#include "uzexampl.h"
#include "../../unzvers.h"

#ifndef _MAX_PATH
#  define _MAX_PATH 260           /* max total file or directory name path */
#endif

#define UNZ_DLL_NAME "UNZIP64.DLL\0"
extern int luzpSession(int argc, char** argv);
extern int unzipSession(int argc, char** argv);
extern int test_cmdline();

HINSTANCE hUnzipDll = nullptr;

////////////////////////////////////////////////////////////
//

/* This is a very simplistic example of how to load and make a call into the
   dll. This has been compiled and tested for a 32-bit console version, but
   not under 16-bit windows. However, the #ifdef's have been left in for the
   16-bit code, simply as an example.
*/
LONG64 muldiv64(LONG64 a, LONG64 b, LONG64 c) {
  if (c == 0 || a == 0) return 0;

  LONG64 high;
  LONG64 low = _mul128(a, b, &high);
  LONGLONG remainder;
  return _div128(high, low, c, &remainder);
}

////////////////////////////////////////////////////////////
//

static BOOL LoadUnzipDll(void)
{
  std::string str(_MAX_PATH, 0);
  if (SearchPath(nullptr, UNZ_DLL_NAME, nullptr, _MAX_PATH, str.data(), nullptr) == 0) {
    constexpr char DLL_WARNING[] = 
      "The Dll must be in the application directory, the path, "\
      "the Windows directory or the Windows System directory.\n";
    std::cout << std::format("Cannot find {}.\n", UNZ_DLL_NAME) << DLL_WARNING;
    return FALSE;
  }
  hUnzipDll = LoadLibrary(UNZ_DLL_NAME);
  if (hUnzipDll == nullptr) {
    std::cout << "Could not load " << UNZ_DLL_NAME << '\n';
    return FALSE;
  }
  return TRUE;
}

void attrib2string(unsigned attrib, char* pattr, size_t len)
{
  std::vector<char> att(8);
  att[0] = (attrib & FILE_ATTRIBUTE_DIRECTORY) ? 'd' : '-';
  att[1] = (attrib & FILE_ATTRIBUTE_READONLY) ? 'a' : '-';
  att[2] = (attrib & FILE_ATTRIBUTE_READONLY) ? 'r' : '-';
  att[3] = (attrib & FILE_ATTRIBUTE_HIDDEN) ? 'h' : '-';
  att[4] = (attrib & FILE_ATTRIBUTE_SYSTEM) ? 's' : '-';
  att[5] = (attrib & FILE_ATTRIBUTE_ENCRYPTED) ? 'E' : '-';
  att[6] = 0;
  (void)lstrcpyn(pattr, att.data(), (int)len - 1);
}

void zipfileInfo(ULONGLONG ucsize, ULONGLONG csiz, const SYSTEMTIME* pst, const char* attr, LPCSTR filename, LPCSTR methbuf)
{
  std::string ratio = std::format("{:4d}%", (int)muldiv64(csiz, 100, ucsize));
  std::string fsize = std::format("{:7d} {:7d}", ucsize, csiz);
  std::string ftime = std::format("{:02d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", pst->wYear, pst->wMonth, pst->wDay, pst->wHour, pst->wMinute, pst->wSecond);
  std::cout << std::format("{} {} {} {} {} {}", fsize.c_str(), ratio.c_str(), methbuf, ftime.c_str(), attr, filename) << '\n';
}

////////////////////////////////////////////////////////////
//

int main(int argc, char** argv)
{
  if (argc < 2) {   /* We must have an archive to unzip */
     const char *progname = strrchr(argv[0], '\\');
     if (progname != nullptr)
        progname++;
     else {
        progname = argv[0];
        if (progname == nullptr || *progname == '\0') progname = "example";
     }
     std::cout << "usage: " << progname << " <zipfile>[entry1[entry2[...]]][-x xentry1[...]]\n";
     return 0;
  }

  if (!LoadUnzipDll()) {
    return -1;
  }

  std::cout << "\nSelect session, luzp-session(enter) or unzip-session(any key + enter) ?: ";
  std::string line;
  std::getline(std::cin, line);
  int res = -1;
  if (line.size() == 0) {
    res = luzpSession(argc, argv);
  }
  else {
    res = unzipSession(argc, argv);
  }

  FreeLibrary(hUnzipDll);
  return res;
}


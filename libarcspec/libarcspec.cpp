// libarcspec.cpp : スタティック ライブラリ用の関数を定義します。
//

#include "stdafx.h"
#include "libarcspec.h"

/////////////////////////////////////////////////////////////////////////
// arcspecify.cpp 共通ソース

static bool endswith(LPCTSTR target, LPCTSTR pattern)
{
  int tlen = lstrlen(target);
  int plen = lstrlen(pattern);
  if (tlen < plen)
    return false;
  return StrCmpI(target + tlen - plen, pattern) == 0;
}

int __cdecl SpecifyArchiveType(LPCTSTR pszName)
{
  if (!pszName || *pszName == 0)
    return ARC_ERROR;

  if (endswith(pszName, _T(".zip")) ||
    endswith(pszName, _T(".jar")) ||
    endswith(pszName, _T(".xpi")) ||
    endswith(pszName, _T(".war")))
    return ARC_ZIP;
  if (endswith(pszName, _T(".tar")) ||
    endswith(pszName, _T(".tgz")) ||
    endswith(pszName, _T(".tar.gz")) ||
    endswith(pszName, _T(".tzst")) ||
    endswith(pszName, _T(".tar.zst")) ||
    endswith(pszName, _T(".tar.bz2")) ||
    endswith(pszName, _T(".tar.lzma")) ||
    endswith(pszName, _T(".tar.xz")) ||
    endswith(pszName, _T(".tar.z")) ||
    endswith(pszName, _T(".deb")) ||
    endswith(pszName, _T(".rpm")))
    return ARC_TAR;
  if (endswith(pszName, _T(".rar")))
    return ARC_RAR;
  if (endswith(pszName, _T(".7z")))
    return ARC_7ZIP;
  if (endswith(pszName, _T(".lzh")))
    return ARC_LZH;
  if (endswith(pszName, _T(".cab")))
    return ARC_CAB;
  if (endswith(pszName, _T(".iso")))
    return ARC_ISO;
#ifndef _WIN64
  if (endswith(pszName, _T(".arj")))
    return ARC_ARJ;
  if (endswith(pszName, _T(".bza")) ||
    endswith(pszName, _T(".gza")))
    return ARC_BGA;
  if (endswith(pszName, _T(".yz1")))
    return ARC_YZ1;
  if (endswith(pszName, _T(".rpm")))
    return ARC_RPM;
  if (endswith(pszName, _T(".ish")))
    return ARC_ISH;
#endif
  return ARC_ERROR;
}

int __cdecl SpecifyArchiveTypeByFileHeader(LPCTSTR pszName)
{
  int type = ARC_ERROR;
  HANDLE hf = CreateFile(pszName, GENERIC_READ,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, 0, NULL);
  if (hf != INVALID_HANDLE_VALUE) {
    BYTE cb[32];
    DWORD dw;
    if (ReadFile(hf, cb, 32, &dw, NULL) && dw >= 8) {
      if (*(DWORD*)cb == 0x04034b50)
        type = ARC_ZIP;
      else if (*(DWORD*)cb == 0x4643534d)
        type = ARC_CAB;
      else if (*(DWORD*)cb == 0xafbc7a37)
        type = ARC_7ZIP;
      else if (*(WORD*)cb == 0x5a42 ||
        *(WORD*)cb == 0x005d ||
        *(WORD*)cb == 0xb528 ||
        *(WORD*)cb == 0x37fd)
        type = ARC_TAR;
      else if (cb[2] == 0x2d &&
        cb[3] == 0x6c &&
        cb[4] == 0x68 &&
        cb[6] == 0x2d)
        type = ARC_LZH;
      else if (*(DWORD*)cb == 0x21726152)
        type = ARC_RAR;
#ifndef _WIN64
      else if (*(DWORD*)cb == 0x0027ea60)
        type = ARC_ARJ;
      else if (*(DWORD*)cb == 0x31307a79)
        type = ARC_YZ1;
      else if (cb[0] == 'G' &&
        cb[1] == 'C' &&
        cb[3] == 'A')
        type = ARC_GCA;
#endif
    }
    CloseHandle(hf);
  }

  if (type == ARC_ERROR)
    type = SpecifyArchiveType(pszName);
  return type;
}

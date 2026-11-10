/*
  Copyright (c) 1990-2003 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
#ifndef __windll_h   /* prevent multiple inclusions */
#define __windll_h

#include <windows.h>
#include <assert.h>    /* required for all Windows applications */
#include <setjmp.h>

#include "../unzip.h"
#include "../windll/structs.h"
#include "../windll/decs.h"

/* Allow compilation under Borland C++ also */
#ifndef __based
#  define __based(A)
#endif

#ifdef UNZIP_INTERNAL

extern HMODULE hUzInstance;        /* current instance */

#ifndef PATH_MAX
#  define PATH_MAX 260            /* max total file or directory name path */
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern jmp_buf dll_error_return;
extern int win_fprintf(zvoid* pG, FILE* file, unsigned int, const char* str);

#ifdef __cplusplus
}
#endif

#endif

int rgGetBinary(LPCSTR pSec, LPCSTR pKey, LPBYTE pData, int nLen);
LPBYTE rgGetBinaryEx(LPCSTR pSec, LPCSTR pKey, LPINT pnLen);
BOOL rgSetBinary(LPCSTR pSec, LPCSTR pKey, LPBYTE pData, int nLen);
int rgGetString(LPCSTR pSec, LPCSTR pKey, LPSTR pStr, int nLen);
BOOL rgSetString(LPCSTR pSec, LPCSTR pKey, LPCSTR pStr);
BOOL rgGetValue(LPCSTR pSec, LPCSTR pKey, LPINT pData);
BOOL rgSetValue(LPCSTR pSec, LPCSTR pKey, int nData);

#endif /* __windll_h */


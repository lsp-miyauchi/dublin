/*
  Copyright (c) 1990-2009 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2009-Jan-02 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
 Example header file

 Do not use this header file in the WiZ application, use WIZ.H
 instead.

*/
#pragma once

#ifndef WIN32   /* this code is currently only tested for 32-bit console */
#  define WIN32
#endif

#if defined(__WIN32__) && !defined(WIN32)
#  define WIN32
#endif

/* Tell Microsoft Visual C++ 2005 to leave us alone and
 * let us use standard C functions the way we're supposed to.
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_WARNINGS
#    define _CRT_NONSTDC_NO_WARNINGS
#  endif
#endif


#include <windows.h>
#include <assert.h>    /* required for all Windows applications */
#include <stdlib.h>
#include <commdlg.h>
#include <windowsx.h>

#include "../../unzip.h"
#include "../structs.h"
#include "../decs.h"
#include "../lspuzp.h"

/* Defines */

typedef const UzpVer * (WINAPI * _DLL_UZVER)(void);
typedef int (WINAPI * _DLL_UNZIP)(int, char **, int, char **, LPUSERFUNCTIONS);

/* Global variables */

extern LPUSERFUNCTIONS lpUserFunctions;
extern LPDCL lpDCL;

extern HINSTANCE hUnzipDll;

/* Global functions */

int WINAPI DisplayBuf(LPSTR, unsigned long);

/* Procedure Calls */
void WINAPI ReceiveDllMessage(z_uint8 ucsize, z_uint8 csize, unsigned cfactor,
    SYSTEMTIME* pst, unsigned attr, char c, LPCSTR filename, LPCSTR methbuf, unsigned long crc, BOOL fCrypt);



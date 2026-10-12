#ifndef __windll_h   /* prevent multiple inclusions */
#define __windll_h

#include <windows.h>
#include <assert.h>    /* required for all Windows applications */
#include <setjmp.h>
#include <commdlg.h>
#if (!defined(__RSXNT__) && !defined(__MINGW32__))
#  include <dlgs.h>
#endif
#define UNZIP_INTERNAL
#include "..\unzip.h"
#include "structs.h"
#include "decs.h"

#ifndef MSWIN
#  define MSWIN
#endif

/* Allow compilation under Borland C++ also */
#ifndef __based
#  define __based(A)
#endif

#ifndef PATH_MAX
#  define PATH_MAX 260            /* max total file or directory name path */
#endif

#define REPLACE_STOP	0
#define REPLACE_NO		1
#define REPLACE_YES		2
#define REPLACE_ALL		3
#define REPLACE_NONE	4
#define REPLACE_RENAME	5

extern jmp_buf dll_error_return;

extern HANDLE hInst;        /* current instance */

#ifdef UNZIP_INTERNAL
void FreeDllMem(__GPRO);
int win_fprintf(zvoid *pG, FILE *file, unsigned int, char far *);
#endif

int rgGetBinary(LPCSTR pSec, LPCSTR pKey, LPBYTE pData, int nLen);
LPBYTE rgGetBinaryEx(LPCSTR pSec, LPCSTR pKey, LPINT pnLen);
BOOL rgSetBinary(LPCSTR pSec, LPCSTR pKey, LPBYTE pData, int nLen);
int rgGetString(LPCSTR pSec, LPCSTR pKey, LPSTR pStr, int nLen);
BOOL rgSetString(LPCSTR pSec, LPCSTR pKey, LPCSTR pStr);
BOOL rgGetValue(LPCSTR pSec, LPCSTR pKey, LPINT pData);
BOOL rgSetValue(LPCSTR pSec, LPCSTR pKey, int nData);


#endif /* __windll_h */

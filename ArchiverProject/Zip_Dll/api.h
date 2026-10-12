/* Only the Windows DLL is currently supported */
#ifndef _ZIPAPI_H
#define _ZIPAPI_H

#include "zip.h"

#ifndef PATH_MAX
# define PATH_MAX 128
#endif

/*---------------------------------------------------------------------------
    Prototypes for public Zip API (DLL) functions.
  ---------------------------------------------------------------------------*/

#define ZPVER_LEN    sizeof(ZpVer)
/* These defines are set to zero for now, until OS/2 comes out
   with a dll.
 */  
#define D2_MAJORVER 0
#define D2_MINORVER 0
#define D2_PATCHLEVEL 0

/* intended to be a private struct: */
typedef struct _zip_ver {
    uch major;              /* e.g., integer 5 */
    uch minor;              /* e.g., 2 */
    uch patchlevel;         /* e.g., 0 */
    uch not_used;
} _zip_version_type;

typedef struct _ZpVer {
    ulg structlen;          /* length of the struct being passed */
    ulg flag;               /* bit 0: is_beta   bit 1: uses_zlib */
    char betalevel[10];     /* e.g., "g BETA" or "" */
    char date[20];          /* e.g., "4 Sep 95" (beta) or "4 September 1995" */
    char zlib_version[10];  /* e.g., "0.95" or NULL */
    _zip_version_type zip;
    _zip_version_type os2dll;
    _zip_version_type windll;
} ZpVer;

#  ifndef EXPENTRY
#    define EXPENTRY WINAPI
#  endif

/* Structures */

void  EXPENTRY ZpVersion(ZpVer far *);

/* Functions not yet supported */
#if 0
int      EXPENTRY ZpMain            (int argc, char **argv);
int      EXPENTRY ZpAltMain         (int argc, char **argv, ZpInit *init);
#endif

void replacechar(LPSTR pszText, int nSrcChar, int nDstChar);
void addsep(LPSTR pszPathName);
char* SkipTabSpace(char* p);
int SeparateString(char* pszText, char** argv, int maxentry);
int DlgModal(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam);
HWND DlgModeless(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam);


#endif _ZIPAPI_H


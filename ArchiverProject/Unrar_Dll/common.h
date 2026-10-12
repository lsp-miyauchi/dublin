//////////////////////////////////////////////////////////////////////
// common.h

#include <io.h>
#include <time.h>
#include "match_exp.h"
#include "unrarapi.h"
#include "strseparator.h"
#include "fileitem.h"
#include "resource.h"

extern LPARCHIVERPROC gpfnProc;
extern UINT nRM_ARCEXTRACT;
extern void (WINAPI* _fSHChangeNotify)(LONG, UINT, LPCVOID, LPCVOID);

///////////////////////////////////////////////////////////////////////
//

#define _A_CRPT			0x4000

///////////////////////////////////////////////////////////////////////
//

#define _ISNETPATH(a)	(wcslen((a)) > 3 && *(a)=='\\' && *((a)+1)=='\\' && *((a)+2)!='?')
#define _ISPATHSEP(a)	((a)=='\\'||(a)=='/')
#define _ISDIGIT(a)		((a)>='0'&&(a)<='9')

////////////////////////////////////////////////////////////////////
// TIME ACCESS DTE_(EXPAND)
//
#define DTE_SEC(d)		(((d) & 0x001f) << 1)
#define DTE_MIN(d)		(((d) & 0x07e0) >> 5)
#define DTE_HOUR(d)		(((d) & 0xf800) >> 11)
#define DTE_DAY(d)		(((d) & 0x001f))
#define DTE_MON(d)		(((d) & 0x01e0) >> 5)
#define DTE_YEAR(d)		(((d) & 0xfe00) >> 9)


////////////////////////////////////////////////////////////////////
// TIME ACCESS DTC_(CONSTRUCT)
//
#define DTC_SEC(d)		(((d) >> 1) & 0x001f)
#define DTC_MIN(d)		(((d) << 5) & 0x07e0)
#define DTC_HOUR(d)		(((d) << 11) & 0xf800)
#define DTC_DAY(d)		(((d) & 0x001f))
#define DTC_MON(d)		(((d) << 5) & 0x01e0)
#define DTC_YEAR(d)		(((d) << 9) & 0xfe00)

///////////////////////////////////////////////////////////////////////
//

#define LODWORD(a)				(*((DWORD*)&(a)))
#define HIDWORD(a)				(*(((DWORD*)&(a)) + 1))
#define SETDWORDLONG(a,b,c)		(*((DWORD*)&(a))=(DWORD)(b),*(((DWORD*)&(a))+1)=(DWORD)(c))

///////////////////////////////////////////////////////////////////////
//

typedef struct tagREPLDATA {
	LPCWSTR lpSrc;
	LPCWSTR lpDst;
	WIN32_FIND_DATA* pwfdSrc;
	WIN32_FIND_DATA* pwfdDst;
	DWORD nAction;
	BOOL bMultiDest;
	BOOL bYesToAll;
} REPLDATA;
typedef REPLDATA* LPREPLDATA;

enum {
	C_QUERY,
	C_NEWONLY,
	C_OVERWRITE,
	C_IGNORE
};


///////////////////////////////////////////////////////////////////////
//

DWORD GetModuleVersion();
BOOL GetFileData(LPCWSTR pszName, WIN32_FIND_DATA* pwfd);
void AttrToString(LPWSTR pszAttr, DWORD attr);
BOOL SetFileDateTimes(LPCTSTR pszName, DWORD attr, LPFILETIME pftCreate, LPFILETIME pftWrite);
LPWSTR FormatDosDate(WORD wDate, LPWSTR pszFormat);
LPWSTR FormatDosTime(WORD wTime, LPWSTR pszFormat);
BOOL RemoveFileNoConfirm(LPCWSTR pszName);


///////////////////////////////////////////////////////////////////////
// replace.cpp

BOOL QueryReplace(HWND hwnd, REPLDATA* prd);

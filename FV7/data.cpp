////////////////////////////////////////////////////////////////
// global data

#include "stdafx.h"
#include "hlink.h"
#include "data.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////
//

// #pragma data_seg(".fv7sh")
	LONG glFVValid = 0;
	LONG glFVAttension = 0;
// #pragma data_seg()
// #pragma comment(linker, " /section:.fv7sh,RWS")

#ifdef _DEBUG
	DWORD __gThreadCount = 0;
#endif

////////////////////////////////////////////////////////////////////////
// ローカルメッセージ

UINT nRM_REGCHECK1ST	= RegisterWindowMessage(RWM_REGCHECK1ST);
UINT nRM_REGDIALOG		= RegisterWindowMessage(RWM_REGDIALOG);
UINT nRM_REGTHANKS		= RegisterWindowMessage(RWM_REGTHANKS);
UINT nRM_REGOK			= RegisterWindowMessage(RWM_REGOK);
UINT nRM_REGADDTHREAD	= RegisterWindowMessage(_T("fv7regaddthread"));
UINT nRM_GREP_FILE		= RegisterWindowMessage(_T("wm_fv7filegrep"));
UINT nRM_GREP_IMAGE		= RegisterWindowMessage(_T("wm_fv7imagegrep"));
UINT nRM_GREP_ICON		= RegisterWindowMessage(_T("wm_fv7icongrep"));
UINT nRM_GREP_DIR		= RegisterWindowMessage(_T("wm_fv7dirgrep"));
UINT nRM_GREP_COLUMN	= RegisterWindowMessage(_T("wm_fv7colgrep"));
UINT nRM_FAVORITE		= RegisterWindowMessage(_T("wm_fv7favorite"));
UINT nRM_SAVEAPPPROP	= RegisterWindowMessage(_T("wm_fv7saveappprop"));
UINT nRM_GETDRAGIMAGE	= RegisterWindowMessage(DI_GETDRAGIMAGE);
UINT nRM_TRAYSERVICE	= RegisterWindowMessage(RWM_TRAYSERVICE);

////////////////////////////////////////////////////////////////////////
// special function

// wrapper functions
int (WINAPI* _fCompareFileName)(LPCTSTR, LPCTSTR);


////////////////////////////////////////////////////////////////////////
// string

const TCHAR gszCrLf[] = _T("\x0d\x0a");
const TCHAR gszAst[] = _T("*.*");
const TCHAR gszTabGroup[] = _T(".tabgroup");

const TCHAR gszShell32[] = _T("shell32.dll");
const TCHAR gszComctl32[] = _T("comctl32.dll");
const TCHAR gszUser32[] = _T("user32.dll");
const TCHAR gszVerbProp[] = _T("properties");
const TCHAR gszCmdPaste[] = _T("paste");
const TCHAR gszAppbarData[] = _T("appbar7.agd");
const TCHAR gszRunDLL[] = _T("rundll32.exe");
const TCHAR gchNil = '\0';

const TCHAR gszDefaultDateTimeForm[] = _T("9999/99/99 99:99:99");

////////////////////////////////////////////////////////////////////////
// miscs

CWinMetrics _GCDATA;
COSPlatform gosp;
CDPIAware gDPIAware;
CIconListStyle gDefaultIconListStyle;
int gnNetSearchResult = -1;
CColorTheme gColors;

////////////////////////////////////////////////////////////////////////
//

DWORD gnListStat;
DWORD gnFileStat;
DWORD gnSysStat;
DWORD gnTsrStat;
DWORD gnFopStat;
DWORD gnMosStat;
DWORD gnBarStat;
DWORD gnTabStat;
DWORD gnDrvStat;
DWORD gnConfStat;
int gnElmaxNameLength;
int gnElmaxExtLength;

////////////////////////////////////////////////////////////////////////
//

const PROPERTYKEY PKEY_ControlPanel =          { { 0x305ca226, 0xd286, 0x468e, { 0xb8, 0x48, 0x2b, 0x2e, 0x8e, 0x69, 0x7b, 0x74 } }, 2 };
const PROPERTYKEY PKEY_RecycleOriginalFolder = { { 0x9B174B33, 0x40FF, 0x11D2, { 0xA2, 0x7E, 0x00, 0xC0, 0x4F, 0xC3, 0x08, 0x71 } }, 2 };
const PROPERTYKEY PKEY_RecycleDateDeleted =    { { 0x9B174B33, 0x40FF, 0x11D2, { 0xA2, 0x7E, 0x00, 0xC0, 0x4F, 0xC3, 0x08, 0x71 } }, 3 };
const PROPERTYKEY PKEY_WorkGroup =             { { 0x5CDE9F0E, 0x1DE4, 0x4453, { 0x96, 0xA9, 0x56, 0xE8, 0x83, 0x2E, 0xFA, 0x3D } }, 2 };
const PROPERTYKEY PKEY_NetworkPlace =          { { 0x28636AA6, 0x953D, 0x11D2, { 0xB5, 0xD6, 0x00, 0xC0, 0x4F, 0xD9, 0x18, 0xD0 } }, 4 };
const PROPERTYKEY PKEY_FontStyle =             { { 0x596FD41B, 0xAF9B, 0x4BA8, { 0x9B, 0x49, 0x33, 0xB1, 0x6F, 0x16, 0x67, 0x8C } },100 };
const PROPERTYKEY PKEY_FontDisplay =           { { 0xD6CF9145, 0xD365, 0x471B, { 0xBC, 0xB8, 0xF0, 0xB4, 0xA9, 0x6B, 0x89, 0x1C } },100 };
const PROPERTYKEY PKEY_FontDesignTarget =      { { 0x5741CF9C, 0x56FE, 0x485B, { 0x89, 0x01, 0x47, 0x86, 0x44, 0x9E, 0x18, 0x8D } },100 };
const PROPERTYKEY PKEY_FontCategory =          { { 0x4B486401, 0x5468, 0x4381, { 0x9B, 0x5A, 0x42, 0xDF, 0x4C, 0xB4, 0x9F, 0x53 } },100 };
const PROPERTYKEY PKEY_FontDesigner =          { { 0x49753869, 0x849C, 0x4323, { 0xA4, 0x1F, 0x26, 0xD7, 0x3F, 0x28, 0xB5, 0x3B } },100 };
const PROPERTYKEY PKEY_FontEmbeded =           { { 0x4530D076, 0xB598, 0x4A81, { 0x88, 0x13, 0x9B, 0x11, 0x28, 0x6E, 0xF6, 0xEA } },2 };


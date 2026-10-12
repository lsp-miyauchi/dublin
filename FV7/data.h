//
// FUNCTION PROTOTYPE DEFINITION
//

#pragma once
#ifndef __DATA_H__
#define __DATA_H__

#include "gcdata.h"
#include "dpiaware.h"
#include "iconstyle.h"
#include "color_theme.h"

//// 仮の設定でここに定義しておく。完了したら oeminc.h へ移動のこと

////////////////////////////////////////////////////////////////////////
//

extern LONG glFVValid;
extern LONG glFVAttension;

////////////////////////////////////////////////////////////////////////
// ローカルメッセージ

extern UINT nRM_REGCHECK1ST;
extern UINT nRM_REGDIALOG;
extern UINT nRM_REGTHANKS;
extern UINT nRM_REGOK;
extern UINT nRM_REGADDTHREAD;
extern UINT nRM_GREP_FILE;
extern UINT nRM_GREP_IMAGE;
extern UINT nRM_GREP_ICON;
extern UINT nRM_GREP_DIR;
extern UINT nRM_GREP_COLUMN;
extern UINT nRM_FAVORITE;
extern UINT nRM_SAVEAPPPROP;
extern UINT nRM_GETDRAGIMAGE;
extern UINT nRM_TRAYSERVICE;

#define FVN_DEVICECHANGE		0x0001
#define FVN_MAPICON				0x0002
#define FVN_MAKEDIR				0x0003


////////////////////////////////////////////////////////////////////////
// string

extern const TCHAR gszCrLf[];
extern const TCHAR gszAst[];
extern const TCHAR gszTabGroup[];

extern const TCHAR gszShell32[];
extern const TCHAR gszComctl32[];
extern const TCHAR gszUser32[];
extern const TCHAR gszVerbProp[];
extern const TCHAR gszCmdPaste[];
extern const TCHAR gszAppbarData[];
extern const TCHAR gszRunDLL[];
extern const TCHAR gchNil;

extern const TCHAR gszDefaultDateTimeForm[];

////////////////////////////////////////////////////////////////////////
// special function

// wrapper functions
extern int (WINAPI* _fCompareFileName)(LPCTSTR, LPCTSTR);


////////////////////////////////////////////////////////////////////////
// miscs

extern CDPIAware gDPIAware;
extern CIconListStyle gDefaultIconListStyle;
extern int gnNetSearchResult;
extern CColorTheme gColors;


////////////////////////////////////////////////////////////////////////
//

extern DWORD gnListStat;
extern DWORD gnFileStat;
extern DWORD gnSysStat;
extern DWORD gnTsrStat;
extern DWORD gnFopStat;
extern DWORD gnMosStat;
extern DWORD gnBarStat;
extern DWORD gnTabStat;
extern DWORD gnDrvStat;
extern DWORD gnConfStat;

extern int gnElmaxNameLength;
extern int gnElmaxExtLength;

//////////////////////////////////////////////////////////////////////
//

extern const PROPERTYKEY PKEY_ControlPanel;
extern const PROPERTYKEY PKEY_RecycleOriginalFolder;
extern const PROPERTYKEY PKEY_RecycleDateDeleted;
extern const PROPERTYKEY PKEY_WorkGroup;
extern const PROPERTYKEY PKEY_NetworkPlace;
extern const PROPERTYKEY PKEY_FontStyle;
extern const PROPERTYKEY PKEY_FontDisplay;
extern const PROPERTYKEY PKEY_FontDesignTarget;
extern const PROPERTYKEY PKEY_FontCategory;
extern const PROPERTYKEY PKEY_FontDesigner;
extern const PROPERTYKEY PKEY_FontEmbeded;


//////////////////////////////////////////////////////////////////////
//

class CSetCursor
{
public:
	HCURSOR m_hcur;
	__inline CSetCursor(HINSTANCE hInst, LPCTSTR pszResourceID) { m_hcur = SetCursor(LoadCursor(hInst, pszResourceID)); }
	__inline CSetCursor::~CSetCursor() { SetCursor(m_hcur); }
};



//////////////////////////////////////////////////////////////////////
//

extern COSPlatform gosp;

__inline bool OsVISTA() { return gosp.OsVISTA(); }
__inline bool Os7() { return gosp.Os7(); }
__inline bool Os8() { return gosp.Os8(); }
__inline bool Os81() { return gosp.Os81(); }
__inline bool Os10() { return gosp.Os10(); }
__inline bool IsWow64() { return gosp.IsWow64(); }
__inline bool IsWindowThemed() { return gosp.IsWindowThemed(); }
__inline bool IsWin64() { return gosp.IsWin64(); }


//////////////////////////////////////////////////////////////////////
//

__inline BYTE swaphilo_byte(BYTE cb)
	{ return (((cb & 0xf0) >> 4) | ((cb & 0x0f) << 4)); }
__inline WORD swaphilo_word(WORD w)
	{ return (((w & 0xff00) >> 8) | ((w & 0x00ff) << 8)); }
__inline DWORD swaphilo_dword(DWORD dw)
	{ return (((dw & 0xffff0000) >> 16) | ((dw & 0x0000ffff) << 16)); }

__inline TCHAR ToUpper(TCHAR ch)
	{ return ((ch >= 'a' && ch <= 'z') ? (ch - ('a' - 'A')) : ch); }
__inline TCHAR ToLower(TCHAR ch)
	{ return ((ch >= 'A' && ch <= 'Z') ? (ch + ('a' - 'A')) : ch); }

__inline int wsz2utf(const WCHAR* wsz, char* utf, int cchMax)
	{ return WideCharToMultiByte(CP_UTF8, 0, wsz, -1, utf, cchMax, NULL, NULL); }
__inline int utf2wsz(const char* utf, WCHAR* wsz, int cchMax)
	{ return MultiByteToWideChar(CP_UTF8, 0, utf, -1, wsz, cchMax); }

__inline BOOL SystemTimeToLocalFileTime(const SYSTEMTIME* pst, FILETIME* pft)
	{ FILETIME ft; return (SystemTimeToFileTime(pst, &ft) && FileTimeToLocalFileTime(&ft, pft)); }
__inline BOOL LocalFileTimeToSystemTime(const FILETIME* pft, SYSTEMTIME* pst)
	{ FILETIME ft; return (LocalFileTimeToFileTime(pft, &ft) && FileTimeToSystemTime(&ft, pst)); }
__inline BOOL SystemTimeToLocalTime(const SYSTEMTIME* ps, SYSTEMTIME* pt)
	{ TIME_ZONE_INFORMATION tz; GetTimeZoneInformation(&tz); return SystemTimeToTzSpecificLocalTime(&tz, ps, pt); }




#endif				// __DATA_H__

///////////////////////////////////////////////////////
// fnc.h

#pragma once
#ifndef __MYFNC_H__
#define __MYFNC_H__

#include "osplatform.h"

_inline void ScreenToClientRect(HWND hwnd, RECT* prc)
	{ ScreenToClient(hwnd, (POINT*)prc);
	  ScreenToClient(hwnd, ((POINT*)prc) + 1); }
_inline void ClientToScreenRect(HWND hwnd, RECT* prc)
	{ ClientToScreen(hwnd, (POINT*)prc);
	  ClientToScreen(hwnd, ((POINT*)prc) + 1); }
_inline BOOL _getfileinfo(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
	{ HANDLE hf = FindFirstFile(pszName, pwfd);
	  DWORD dw = GetLastError();
	  if (hf != INVALID_HANDLE_VALUE)
		  FindClose(hf);
	  return (hf != INVALID_HANDLE_VALUE || dw == ERROR_MORE_DATA); }

///////////////////////////////////////////////////////
// Globals
//

extern COSPlatform gosp;

extern int (WINAPI* _fDrawShadowText)(HDC, LPCWSTR, UINT, const RECT*, DWORD, COLORREF, COLORREF, int, int);
extern int (WINAPI* _fStrCmpLogicalW)(LPCWSTR, LPCWSTR);
extern LPWSTR (WINAPI* _fStrFormatByteSize64)(LONGLONG, LPWSTR, UINT);
extern HRESULT (WINAPI* _fSHGetStockIconInfo)(SHSTOCKICONID, UINT, SHSTOCKICONINFO*);
extern HRESULT (WINAPI* _fSHCreateItemInKnownFolder)(REFKNOWNFOLDERID, DWORD, PCWSTR, REFIID, void**);
extern HRESULT (WINAPI* _fSHCreateItemFromParsingName)(PCWSTR, IBindCtx*, REFIID, void**);
extern HRESULT (WINAPI* _fSHCreateItemFromIDList)(PCIDLIST_ABSOLUTE, REFIID, void**);
extern HRESULT (WINAPI* _fSHGetIDListFromObject)(IUnknown*, PIDLIST_ABSOLUTE*);
extern HRESULT (WINAPI* _fSHGetKnownFolderIDList)(REFKNOWNFOLDERID, DWORD, HANDLE, PIDLIST_ABSOLUTE*);

__inline bool IsComm6() { return gosp.IsComm6(); }
__inline bool OsVISTA() { return gosp.OsVISTA(); }
__inline bool Os7() { return gosp.Os7(); }
__inline bool Os8() { return gosp.Os8(); }
__inline bool IsThemed() { return gosp.IsThemed(); }
__inline bool IsWindowThemed() { return gosp.IsThemed(); }
__inline bool IsRGBA() { return gosp.IsRGBA(); }

enum {
	IMAGERESOURCE_DOTCOLOR = 0,
	IMAGERESOURCE_CHECKMARK = 1,
};
HIMAGELIST GetCommonImageList(int type);




///////////////////////////////////////////////////////
// API entries
//

BOOL InitDir();
void WepDir();

void DrawCoolFrame(HDC hdc, CRect& rc);


#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

LRESULT CALLBACK StaticWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ColorWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MeterWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RectWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CheckWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BrowseExWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PrgListWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ShadeWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LedClockWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AniImageWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TaskLinkWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK UrlWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PalWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ColorPickWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FontPickWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK IDListPickWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DegreeWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TrackWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LogWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ListWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ColoredProgressBarWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK InfoBarWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FlatButtonWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ZoomViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GradEditWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SignalWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SliderWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FolderTreeWnd(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK IDListPickWnd(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DrawListViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TipEditWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SplitterWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CheckTreeViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabExWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SearchPathWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif


#define _CHECKDOT(a)	((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))
#define _ISNETPATH(a)	(lstrlen((a)) > 3 && *(a) == '\\' && *((a) + 1) == '\\' && *((a) + 2) != '?')


#endif			// __MYFNC_H__

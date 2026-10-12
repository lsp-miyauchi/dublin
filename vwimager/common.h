////////////////////////////////////////////////////////////////////
// common.h

#pragma once
#ifndef __VWIMAGE_COMMON_H__
#define __VWIMAGE_COMMON_H__


typedef struct tagIMAGETYPES {
	int type;
	LPCTSTR ext;
	LPCTSTR avail;
} IMAGETYPES;

extern IMAGETYPES img_types[];

extern COSPlatform gosp;
__inline bool OsVISTA() { return gosp.OsVISTA(); }

////////////////////////////////////////////////////////////////////
//
#define	SYSCOLOR_WINDOWTEXT		(GetSysColor(COLOR_WINDOWTEXT))
#define	SYSCOLOR_WINDOW			(GetSysColor(COLOR_WINDOW))
#define	SYSCOLOR_MENUTEXT		(GetSysColor(COLOR_MENUTEXT))
#define	SYSCOLOR_MENU			(GetSysColor(COLOR_MENU))
#define	SYSCOLOR_HIGHLIGHT		(GetSysColor(COLOR_HIGHTLIGHT))
#define	SYSCOLOR_HIGHLIGHTTEXT	(GetSysColor(COLOR_HIGHLIGHTTEXT))


////////////////////////////////////////////////////////////////////
// MAXIMUM NUMBER
//
#define MAXHIAR 				16
#define MAXACCESS				16
#define MAXHISTORY				16


////////////////////////////////////////////////////////////////////
//
enum {
	FILETYPE_TEXT,
	FILETYPE_BINARY,
	FILETYPE_UNKNOWN
};

//----------------------------------------------
// MessageBox ACTION
//
#define MB_ATTENTION			(MB_OK | MB_ICONEXCLAMATION)
#define MB_QUESTION				(MB_YESNOCANCEL | MB_ICONQUESTION)
#define MB_QYESNO				(MB_YESNO | MB_ICONQUESTION)
#define MB_QOKCANCEL			(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QABORT				(MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE)
#define MB_HAND					(MB_OKCANCEL | MB_ICONHAND)
#define MB_STOP					(MB_OK | MB_ICONSTOP)
#define MB_QAST					(MB_OKCANCEL | MB_ICONASTERISK)
#define MB_QRETRY				(MB_RETRYCANCEL | MB_ICONEXCLAMATION)

////////////////////////////////////////////////////////////////////
// MACROS
//
#define _ABS(a,b)				(((a) < (b)) ? (b) - (a) : (a) - (b))
#define TOGGLEFLAG(a,b)			(((a) & (b)) ? ((a) & ~(b)) : ((a) | (b)))


////////////////////////////////////////////////////////////////////
// setting flags

#define BST_TBSMALL				0x00000001
#define BST_GRAB				0x00000010
#define BST_NAVIGATOR			0x00000020
#define BST_SCALE_MASK			0x000F0000
#define BST_REALSCALE			0x00000000
#define BST_AUTOSCALE			0x00010000
#define BST_PREVSCALE			0x00020000



////////////////////////////////////////////////////////////////////
// FUNCTIONS
//

int AppMessage(_U_STRINGorID cap, _U_STRINGorID id, LPCTSTR pszInsert = NULL, UINT nType = MB_STOP);
int LastError(LPCTSTR pszTitle, UINT nType = MB_STOP);
BOOL FileBrowse(HWND hWnd, UINT nIDCaption, UINT nIDFilter, LPTSTR pszName, BOOL bRead, int* filterIndex = NULL);
int ListupSiblings(LPCTSTR lpClass, HWND *phWnd);
void NextPrevWindow(HWND hWnd, LPCTSTR lpClass, BOOL bNext);
HGLOBAL CopyTextToMemory(LPCTSTR lpText);
int HowManySiblings(HWND hwndOwn, LPCTSTR lpClass);
HWND Get1stSiblibgs(HWND hwndOwn, LPCTSTR lpClass);
void FormatText(LPCTSTR lpszSrc, LPTSTR lpszDst, int nTabConv, int nMax);
void PaintBitmap(HDC hdc, CRect& rcd, CRect& rcs, HBITMAP hbm, HPALETTE hpal);
void PaintDIB(HDC hdc, CRect& rcd, CRect& rcs, HDIB hdib);
void calcinsiderect(CRect& rcMax, int x, int y, CRect& rcCalc);
int dibandtodevice(HDC hdc, int sx, int sy, BITMAPINFO *pbi);
BOOL __fastcall ImageInClipboard(HWND hwnd);
HDIB quantize(HDIB hsrc, int method, int colors);
BOOL _check_user();
BOOL user_ristriction();
IActiveDesktop* GetActiveDesktop();
BOOL SetWallpaper(LPTSTR pszFile);
int __fastcall GetCheckedRadioButton(HWND hwnd, int nIDFirstButton, int nIDLastButton);
void __fastcall GetDataFolder(LPTSTR pszPath);
BOOL __fastcall rename_imgextension(LPTSTR pszFile, int type);
void __fastcall InitDlgCtrl(HWND hctl, UINT idResource);
BOOL __fastcall _pathmaketree(LPCTSTR pszPath);
void __fastcall _pathdecorate(LPTSTR psz);
int CountSiblings(HWND hwndOwn);

BOOL CheckDirectoryNameUI(HWND hwnd, LPCTSTR pszPath);
BOOL CheckServerUI(HWND hwnd, LPCTSTR pszFolder);
bool ShowOpenSaveFileDialog(HWND hwnd, bool bOpen, LPCTSTR pszTitle, LPCTSTR pszDir, CString& strFile, const COMDLG_FILTERSPEC* pcfFilter, int numFilters);
double __fastcall CalcInnerImageSize(int cxMax, int cyMax, int* pcx, int* pcy);
void FillShadeRect(HDC hdc, LPCRECT prc, COLORREF c1, COLORREF c2);
void FillShadeRect(HDC hdc, int left, int top, int width, int height, COLORREF c1, COLORREF c2);

///////////////////////////////////////////////////////////////////////
//

HRESULT __fastcall FileDrag(HWND hwnd, LPCTSTR pszFile, LPDWORD pdwEffect);
BOOL __fastcall DoContextVerb(HWND hwnd, LPCONTEXTMENU pcm, LPCTSTR pVerb);
void __fastcall DoFileProperty(HWND hwnd, LPCTSTR pszFile);
HGLOBAL __fastcall copyhandle(HGLOBAL hsrc);

///////////////////////////////////////////////////////////////////////
//

int __fastcall dib_numcolors(void* pv);
int __fastcall dib_palsize(void* pv);
BOOL __fastcall dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits);

_inline BYTE* __fastcall dib_findbits(void* pv)
	{ return ((BYTE*)pv + *(DWORD*)pv + dib_palsize(pv)); }



#endif		// __VWIMAGE_COMMON_H__

////////////////////////////////////////////////////////////////
// pidl.h

#pragma once

#ifndef __PIDL_H__
#define __PIDL_H__


#ifndef MAX_URL_LENGTH
	#define MAX_URL_LENGTH			(2048 + 32 + 8)
#endif
#ifndef DROPEFFECT
	typedef DWORD DROPEFFECT;
#endif

typedef struct tagSDTCOLINFO {
	int iCol;
	int fmt;
	int cxChar;
	DWORD state;
	TCHAR title[80];
} SDTCOLINFO;

#define STR_MYDOCS_CLSID_W	L"::{450d8fba-ad25-11d0-98a8-0800361b1103}"


UINT __fastcall idl_GetNumIds(LPCITEMIDLIST pidl);
LPITEMIDLIST __fastcall idl_GetSubPid(LPCITEMIDLIST pidl, UINT nIndex);
LPITEMIDLIST __fastcall idl_Next(LPCITEMIDLIST pidl);
UINT __fastcall idl_GetSize(LPCITEMIDLIST pidl);
LPITEMIDLIST __fastcall idl_Create(UINT cbSize);
LPITEMIDLIST __fastcall idl_ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
LPITEMIDLIST __fastcall idl_CopyITEMID(LPITEMIDLIST lpi);
BOOL __fastcall idl_GetInfoTip(HWND hwnd, LPITEMIDLIST pidlFq, LPTSTR pTip, int cchMax);
BOOL __fastcall idl_GetInfoTip(HWND hwnd, LPSHELLFOLDER psf, LPITEMIDLIST pidl, LPTSTR pTip, int cchMax);
BOOL __fastcall idl_GetName(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi, DWORD dwFlags, LPTSTR pName, UINT cchName);
BOOL __fastcall idl_GetName(LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pName, UINT cchName);
LPITEMIDLIST __fastcall idl_GetFullyQualPidl(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi);
BOOL __fastcall idl_GetIcon(LPSHELLFOLDER lpsf, LPITEMIDLIST pidl, HICON* phLarge, HICON* phSmall);
LPITEMIDLIST __fastcall idl_Copy(LPCITEMIDLIST pidlSource);
int __fastcall idl_Compare(LPCITEMIDLIST pidlsrc, LPCITEMIDLIST pidldst);
LPCITEMIDLIST __fastcall idl_SkipToLastItem(LPITEMIDLIST pidlFq);
LPITEMIDLIST __fastcall idl_GetParent(LPITEMIDLIST pidlFQ);
LPITEMIDLIST __fastcall idl_GetFromPath(LPCTSTR pszFile);
LPSHELLFOLDER __fastcall idl_GetShellFolder(LPITEMIDLIST pidl);
LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, LPITEMIDLIST pidlFq, REFIID riid);
LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, IShellFolder* psf, LPITEMIDLIST pidl, REFIID riid);
DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, LPITEMIDLIST pidl, LPDATAOBJECT pd, DWORD dwKeyState, DWORD effSupp);
DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, IShellFolder* psf, LPITEMIDLIST pidl, LPDATAOBJECT pd, DWORD dwKeyState, DWORD effSupp);
DROPEFFECT __fastcall idl_Execute(HWND hwnd, LPITEMIDLIST pidl, LPDATAOBJECT pdo, DWORD keyState, DROPEFFECT dropEffect, POINT pt);
DROPEFFECT __fastcall idl_ExecuteDirect(HWND hwnd, LPITEMIDLIST pidl, LPDATAOBJECT pdo, DWORD keyState, DROPEFFECT dropEffect, POINT pt);
IShellFolder* __fastcall idl_GetSpecialFolder(UINT id, LPITEMIDLIST* ppidl);
void __fastcall idl_ValidateCacheFolderInformation(LPSHELLFOLDER psf);
void __fastcall idl_ValidateDriveInformation();
BOOL __fastcall idL_GetShellFolder(LPCTSTR pPath, IShellFolder** ppsf, LPITEMIDLIST* ppidl);
int __fastcall idl_GetSysIconIndex(IShellFolder* psf, LPITEMIDLIST pidl, UINT flg);
int __fastcall idl_GetSysIconIndex(LPITEMIDLIST pidlPar, LPITEMIDLIST pidl, UINT flg);
int __fastcall idl_GetSysIconIndex(LPITEMIDLIST pidlFq, UINT flg);
int __fastcall idl_GetDescriptionId(IShellFolder* psf, LPITEMIDLIST pidl);
int __fastcall idl_GetDescriptionId(LPITEMIDLIST pidlFq);
BOOL __fastcall idl_GetAttributesOf(LPITEMIDLIST pidlFq, SFGAOF* pattr);
HBITMAP __fastcall idl_ExtractImage(IShellFolder* psf, LPITEMIDLIST pidl, SIZE* prgSize, DWORD depth);
BOOL __fastcall idl_IDListIsValid(LPITEMIDLIST pidlFq);
_inline void* idl_Alloc(UINT cb) { return ((cb != 0) ? CoTaskMemAlloc(cb) : NULL); }
_inline void idl_Free(void* pv) { if (pv) CoTaskMemFree(pv); }


#endif				// __PIDL_H__

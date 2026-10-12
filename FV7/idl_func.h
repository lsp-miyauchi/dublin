////////////////////////////////////////////////////////////////
// idl_func.h

#pragma once
#ifndef __IDL_FUNC_H__
#define __IDL_FUNC_H__

#include <propkey.h>
#include <propvarutil.h>

#define IS_INTPIDL(_r)		(((ULONG_PTR)(_r) >> 16) == 0)
#define MAKEINTPIDL(i)		(LPITEMIDLIST)((ULONG_PTR)((WORD)(i)))

// #define PIDL_LOCAL_FUNCTIONS

////////////////////////////////////////////////////////////////////////
//

typedef struct tagSHELLCOLUMNINFO {
	int iCol;
	int fmt;
	int cxChar;
	SHCOLSTATEF state;
	TCHAR title[80];
	SHCOLUMNID scid;
} SHELLCOLUMNINFO;

#define STR_MYDOCS_CLSID_W	L"::{450d8fba-ad25-11d0-98a8-0800361b1103}"

////////////////////////////////////////////////////////////////////////
// DescriptionIDs

#ifndef SHDID_MOBILE_DEVICE
	#define SHDID_MOBILE_DEVICE				21  // PDA/PalmPC
#endif
#ifndef SHDID_COMPUTER_LIBRARIES
	#define SHDID_COMPUTER_LIBRARIES		22
#endif
#ifndef SHDID_SHAREPOINT_SITES
	#define SHDID_SHAREPOINT_SITES			23
#endif


////////////////////////////////////////////////////////////////////////
//

inline BOOL idl_isDesktop(LPCITEMIDLIST pidl) { return (pidl != NULL && (::ILGetSize(pidl) == 2)); }
inline BOOL idl_isEmpty(LPCITEMIDLIST pidl) { return (!pidl || !pidl->mkid.cb); }

UINT __fastcall idl_GetNumIds(LPCITEMIDLIST pidl);
LPITEMIDLIST __fastcall idl_GetSubPid(LPCITEMIDLIST pidl, UINT nIndex);
LPITEMIDLIST __fastcall idl_Create(UINT cbSize);
int idl_GetInfoTip(HWND hwnd, LPCITEMIDLIST pidlFq, CString& strTip);
int idl_GetInfoTip(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidl, CString& strTip);
BOOL idl_GetName(IShellFolder* psf, LPCITEMIDLIST pidl, DWORD dwFlags, CString& strName);
BOOL idl_GetName(LPCITEMIDLIST pidlFq, DWORD dwFlags, CString& strName);
BOOL idl_GetPathFromIDList(LPCITEMIDLIST pidlFq, CString& strPath);
LPITEMIDLIST __fastcall idl_ParseDisplayName(LPCTSTR pszFile, ULONG* pattr = NULL);
LPITEMIDLIST __fastcall idl_GetFullyQualPidl(IShellFolder* lpsf, LPCITEMIDLIST lpi);
int __fastcall idl_Compare(LPCITEMIDLIST pidlsrc, LPCITEMIDLIST pidldst);
LPITEMIDLIST __fastcall idl_GetParent(LPCITEMIDLIST pidlFQ);
IShellFolder* __fastcall idl_GetShellFolder(LPCITEMIDLIST pidl);
IShellFolder* __fastcall idl_BindToShellFolder(IShellFolder* psf, LPCITEMIDLIST pidl);
LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, LPCITEMIDLIST pidlFq, REFIID riid);
LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidl, REFIID riid);
DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, LPCITEMIDLIST pidl, IDataObject* pd, DWORD dwKeyState, DWORD effSupp);
DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidl, IDataObject* pd, DWORD dwKeyState, DWORD effSupp);
DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, IShellFolder* psf, IDataObject* pdo, DWORD keyState, DWORD effSupp);
DROPEFFECT __fastcall idl_Execute(HWND hwnd, LPCITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DROPEFFECT dropEffect, POINT pt);
DROPEFFECT __fastcall idl_ExecuteDirect(HWND hwnd, LPCITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DROPEFFECT dropEffect, POINT pt);
IShellFolder* __fastcall idl_GetSpecialFolder(UINT id, LPITEMIDLIST* ppidl);
SFGAOF __fastcall idl_GetAttributes(LPCITEMIDLIST pidlFullyQual, SFGAOF mask);
void __fastcall idl_ValidateCacheFolderInformation(IShellFolder* psf);
void __fastcall idl_ValidateDriveInformation();
BOOL __fastcall idL_GetShellFolder(LPCTSTR pPath, IShellFolder** ppsf, LPITEMIDLIST* ppidl);
int __fastcall idl_GetSysIconIndex(LPCITEMIDLIST pidlFullyQual, BOOL bDirty = FALSE);
int __fastcall idl_GetShellColumnInfo(IShellFolder* psf, CSimpleArray<SHELLCOLUMNINFO>& arr, ULONG* pSortDefault, ULONG* pDispDefault);
BOOL __fastcall idl_GetPropertyStoreText(LPCITEMIDLIST pidlFq, const SHCOLUMNID& scid, CString& str, DWORDLONG* pdwl);
BOOL __fastcall idl_GetShellPropertyText(IShellFolder2* psf2, LPCITEMIDLIST pidl, const SHCOLUMNID& scid, CString& str, DWORDLONG* pdwl);
BOOL __fastcall idl_GetShellColumnText(IShellFolder2* psf2, LPCITEMIDLIST pidl, int col, CString& str);
void dump_descid(int descid);
int __fastcall idl_GetDescriptionId(IShellFolder* psf, LPCITEMIDLIST pidl);
int __fastcall idl_GetDescriptionId(LPCITEMIDLIST pidlFq);
BOOL __fastcall idl_GetAttributesOf(LPCITEMIDLIST pidlFq, SFGAOF* pattr);
BOOL __fastcall idl_GetShellAttributes(LPCITEMIDLIST pidlFq, SFGAOF mask, SFGAOF* pattr);
HBITMAP __fastcall idl_ExtractImage(IShellFolder* psf, LPCITEMIDLIST pidl, SIZE* prgSize, DWORD depth = 32);
BOOL __fastcall idl_IDListIsValid(LPCITEMIDLIST pidlFullyQual);
BOOL __fastcall idl_SendTo(HWND hwnd, LPCITEMIDLIST pidlFullyQual, IDataObject* pdo);
BOOL __fastcall idl_CopyTo(HWND hwnd, LPCITEMIDLIST pidlFullyQual, IDataObject* pdo, BOOL bCopy);
LPITEMIDLIST idl_GetMyDocument();

////////////////////////////////////////////////////////////////////////
// scid_supp.cpp


#ifdef _DEBUG
LPCTSTR ScidNameDump(const SHCOLUMNID& scid);				// scid_supp.cpp
#endif	// _DEBUG
bool FindScidByVerb(LPCTSTR verb, SHCOLUMNID* pscid);


#endif			// __IDL_FUNC_H__

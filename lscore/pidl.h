////////////////////////////////////////////////////////////////
// pidl.h

#pragma once
#ifndef __PIDL_H__
#define __PIDL_H__

// #define PIDL_LOCAL_FUNCTIONS


////////////////////////////////////////////////////////////////////////
//

#define SETFormatEtc(fe, cf, med, li)\
		{\
			(fe).cfFormat = cf;\
			(fe).dwAspect = DVASPECT_CONTENT;\
			(fe).ptd = NULL;\
			(fe).tymed = med;\
			(fe).lindex = li;\
		};

////////////////////////////////////////////////////////////////////////
//

typedef struct tagSDTCOLINFO {
	int iCol;
	int fmt;
	int cxChar;
	DWORD state;
	TCHAR title[80];
} SDTCOLINFO;

////////////////////////////////////////////////////////////////////////
//

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

UINT ILGetItemCount(LPCITEMIDLIST pidl);

BOOL idl_GetName(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD flags, CString& strName);
BOOL idl_GetName(LPCITEMIDLIST pidl, DWORD flags, CString& strName);
LPITEMIDLIST __fastcall idl_GetFullyQualPidl(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi);
int __fastcall idl_Compare(LPCITEMIDLIST pidlsrc, LPCITEMIDLIST pidldst);
LPITEMIDLIST __fastcall idl_GetParent(LPCITEMIDLIST pidlFullyQual);
IShellFolder* __fastcall idl_GetShellFolder(LPITEMIDLIST pidl);
IShellFolder* __fastcall idl_BindToShellFolder(IShellFolder* psf, LPITEMIDLIST pidl);
LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, LPITEMIDLIST pidlFq, REFIID riid);
LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, IShellFolder* psf, LPITEMIDLIST pidl, REFIID riid);
IShellFolder* __fastcall idl_GetSpecialFolder(UINT id, LPITEMIDLIST* ppidl);
void __fastcall idl_ValidateCacheFolderInformation(LPSHELLFOLDER psf);
void __fastcall idl_ValidateDriveInformation();
BOOL __fastcall idL_GetShellFolder(LPCTSTR pPath, IShellFolder** ppsf, LPITEMIDLIST* ppidl);
int __fastcall idl_GetSysIconIndex(LPCTSTR pszFile, UINT flg, DWORD attr = (DWORD)-1);
int __fastcall idl_GetSysIconIndex(LPITEMIDLIST pidlPar, LPITEMIDLIST pidl, UINT flg);
int __fastcall idl_GetSysIconIndex(LPITEMIDLIST pidlFq, UINT flg);
int __fastcall idl_GetSysIconIndex(IShellFolder* psf, LPITEMIDLIST pidl, UINT flg);
int __fastcall idl_GetShellColumnInfo(IShellFolder* psf, SDTCOLINFO* pcol, int maxarray);
BOOL __fastcall idl_GetShellColumnText(IShellFolder2* psf2, LPITEMIDLIST pidl, int col, LPTSTR pszText, UINT cchText);
BOOL __fastcall idl_GetShellColumnText(IShellDetails* psdt, LPITEMIDLIST pidl, int col, LPTSTR pszText, UINT cchText);
void dump_descid(int descid);
int __fastcall idl_GetDescriptionId(IShellFolder* psf, LPCITEMIDLIST pidl);
int __fastcall idl_GetDescriptionId(LPCITEMIDLIST pidlFq);
BOOL ILGetAttributesOf(LPITEMIDLIST pidlFq, SFGAOF* pattr);
HBITMAP __fastcall idl_ExtractImage(IShellFolder* psf, LPITEMIDLIST pidl, SIZE* prgSize, DWORD depth);
BOOL __fastcall idl_IDListIsValid(LPITEMIDLIST pidlFq);
BOOL __fastcall idl_SendTo(HWND hwnd, LPITEMIDLIST pidlFq, LPDATAOBJECT pdo);

inline BOOL ILIsDesktop(LPCITEMIDLIST pidl) { return (ILGetSize(pidl) == 2); }
inline BOOL idl_IsEmpty(LPCITEMIDLIST pidl) { return (!pidl || !pidl->mkid.cb); }

#endif			// __PIDL_H__

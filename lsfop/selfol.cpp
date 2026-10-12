// NETSEL.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "fopstd.h"
#include "resource.h"

#define BFC_TREE		0x3741
#define BFC_TITLE		0x3742
#define BFC_PATH		0x3743
#define BFC_EDITBOX		0x3744


#define DONT_DISMISS		1

static int CALLBACK bcproc(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
	switch (msg) {
	case BFFM_INITIALIZED:
		if (lpData && _pathfileexists((LPCTSTR)lpData))
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		break;

	case BFFM_SELCHANGED:
	{
		TCHAR buf[MAX_PATH * 2];
		BOOL bOK = SHGetPathFromIDList((LPITEMIDLIST)lParam, buf);
		SendMessage(hwnd, BFFM_ENABLEOK, 0, bOK);
		break;
	}

	case BFFM_VALIDATEFAILEDA:
	case BFFM_VALIDATEFAILEDW:
		return DONT_DISMISS;

	default:
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//

int __fastcall BrowseForFolder(HWND hwnd, LPTSTR pszPath, _RES_STRING caption)
{
	BOOL bOK = FALSE;
	TCHAR title[MAX_PATH];
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = hwnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = title;
	bi.lpszTitle = caption.m_str;
	bi.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS | BIF_VALIDATE;
	bi.lpfn = bcproc;
	bi.lParam = (LPARAM)pszPath;

	TCHAR dir[MAX_PATH * 2];
	GetCurrentDirectory(ARRAYSIZE(dir), dir);
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	SetCurrentDirectory(dir);

	if (pidl) {
		if (SHGetPathFromIDList(pidl, pszPath)) {
			SHFILEINFO sfi;
			sfi.dwAttributes = SFGAO_LINK;
			SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi),
							SHGFI_PIDL | SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
			if (sfi.dwAttributes & SFGAO_LINK) {
				FSLINKINFO li;
				if (fopParseLinkInfo(pszPath, &li))
					lstrcpy(pszPath, li.szTargetName);
			}
			fopSetDirRemind(PathGetDriveNumber(pszPath), pszPath);
			bOK = TRUE;
		}
		idl_Free(pidl);
	}
	return bOK;
}

extern "C"
int WINAPI fopBrowseForFolder(HWND hwnd, LPTSTR pszPath)
{
	return BrowseForFolder(hwnd, pszPath, IDS_TITLE_BROWSEFOLDER);
}

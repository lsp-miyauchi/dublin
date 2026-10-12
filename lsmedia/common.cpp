#include "stdafx.h"
#include "lsmedia.h"


int AppMessage(UINT idTitle, UINT idMsg, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString msg;
	msg.LoadString(idMsg);
	if (pszInsert && *pszInsert)
		msg.Replace(_T("%1"), pszInsert);
	return AtlMessageBox(::GetActiveWindow(), (LPCTSTR)msg, idTitle, nType);
}

int LastError(UINT nTitleID, LPCTSTR pszTitle, UINT nType /*=MB_STOP*/)
{
	int nError = GetLastError();

	CString cs;
	cs.LoadString(nTitleID);
	return lccErrorBox(::GetActiveWindow(), nError, cs, pszTitle, nType);
}

BOOL GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	HANDLE hFind = FindFirstFile(pszName, pwfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);

		FILETIME ft;

		// ƒ[ƒJƒ‹ŽžŠÔ‚É•ÏŠ·
		FileTimeToLocalFileTime(&pwfd->ftCreationTime, &ft);
		pwfd->ftCreationTime = ft;
		FileTimeToLocalFileTime(&pwfd->ftLastWriteTime, &ft);
		pwfd->ftLastWriteTime = ft;

		return TRUE;
	}
	return FALSE;
}

BOOL BrowseForFolder(HWND hwnd, int nFolder, LPTSTR path)
{
	LPITEMIDLIST pidlRoot;
	SHGetFolderLocation(hwnd, nFolder, NULL, 0, &pidlRoot);

	TCHAR buf[MAX_PATH];
	CString cs;
	cs.LoadString(IDS_TITLE_BROWSEFOLDER);
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner        = hwnd;
	bi.pidlRoot         = pidlRoot;
	bi.ulFlags          = BIF_RETURNONLYFSDIRS;
	bi.pszDisplayName   = buf;
	bi.lpszTitle        = cs;

	TCHAR dir[MAX_PATH * 2];
	GetCurrentDirectory(ARRAYSIZE(dir), dir);
	LPITEMIDLIST pidlDest = SHBrowseForFolder(&bi);
	SetCurrentDirectory(dir);

	// did the user select the cancel button
	if (NULL != pidlDest)
	{
		SHGetPathFromIDList(pidlDest, path);
		return TRUE;
	}
	return FALSE;
}

void ZapMessage(HWND hwnd)
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!IsDialogMessage(hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

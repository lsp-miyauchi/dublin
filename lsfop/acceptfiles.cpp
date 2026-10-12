////////////////////////////////////////////////////////////////////////
// acceptfiles.cpp
// fv7より前のバージョンで使用していたエントリー

#include "stdafx.h"
#include "fopstd.h"

extern "C"
BOOL WINAPI fopAcceptFiles(HWND hOwner, HDROP hdrop, DWORD eff, LPCTSTR pszDropPath)
{
//	return ExecDropFileOperation(hOwner, hdrop, eff, pszDropPath, true);
	DROPFILEOPERATION dfo;
	dfo.cbSize = sizeof(dfo);
	dfo.hwnd = hOwner;
	dfo.hDrop = hdrop;
	dfo.dwEffect = eff;
	dfo.pszDropPath = pszDropPath;
	dfo.bAsAdmin = FALSE;
	dfo.bAsyncOperation = FALSE;
	dfo.bCopyAccessRights = FALSE;
	dfo.bSaveFolderHierarchy = TRUE;
	return fopDropFileOperation(&dfo);
}

extern "C"
BOOL WINAPI fopAcceptFilesExec(HDROP hdrop, DWORD eff, LPCTSTR pszDropPath)
{
	DROPFILEOPERATION dfo;
	dfo.cbSize = sizeof(dfo);
	dfo.hwnd = NULL;
	dfo.hDrop = hdrop;
	dfo.dwEffect = eff;
	dfo.pszDropPath = pszDropPath;
	dfo.bAsAdmin = FALSE;
	dfo.bAsyncOperation = TRUE;
	dfo.bCopyAccessRights = FALSE;
	dfo.bSaveFolderHierarchy = TRUE;
	return fopDropFileOperation(&dfo);
}

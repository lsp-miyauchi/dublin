////////////////////////////////////////////////////////////////////////
// FVNET.CPP
#include "stdafx.h"
#include "fv7.h"
#include "shellexec.h"

#include "mainfrm.h"

/* @@@ 参考の為にコードは残しておきます

BOOL CFvApp::DoSharing(LPCTSTR pszPath, BOOL bShare)
{
	CDll dll(_T("NTLANUI.DLL"));
	void (WINAPI* pfn)(HWND, UINT, LPCSTR);
	if (bShare)
		(FARPROC&)pfn = dll.FindFunction("ShareAsDialogA0");
	else
		(FARPROC&)pfn = dll.FindFunction("StopShareDialogA0");

	if (pfn) {
		(*pfn)(wndMain, 1, pszPath);
		wndMain.PostMessage(WM_COMMAND, IDM_NET_RESET);
	}
	return TRUE;
}

@@@ */

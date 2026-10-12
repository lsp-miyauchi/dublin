#include "stdafx.h"
#include <mmsystem.h>
#include "fv7.h"
#include "shellexec.h"
#include "atten.cxx"

////////////////////////////////////////////////////////////////
//

#include "..\lsimg\\imgspecify.cpp"

////////////////////////////////////////////////////////////////
//

static BOOL _win_execute(HWND hwnd, LPCTSTR pszFile)
{
	CString dir;
	PathGetParent(pszFile, dir.GetBuffer(MAX_PATH));
	dir.ReleaseBuffer();

	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	si.lpTitle = PathFindFileName(pszFile);
	PROCESS_INFORMATION pi;
	if (CreateProcess(NULL, (LPTSTR)pszFile, NULL, NULL, FALSE,
			CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,
			NULL, dir.GetString(), &si, &pi))
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return TRUE;
	}
	return FALSE;
}

BOOL AddinEditor(HWND hParent, LPCTSTR pszEditFile, LPCTSTR pszDefaultDir)
{
	CString dir;
	if (pszDefaultDir && *pszDefaultDir)
		dir = pszDefaultDir;

	CString name;
	CString str;
	if (pszEditFile && *pszEditFile) {
		// Quiote整形
		LPTSTR p = name.GetBuffer(lstrlen(pszEditFile) + 4);
		lstrcpy(p, pszEditFile);
		PathQuoteSpaces(p);
		name.ReleaseBuffer();

		// 最初にアドインエディタをチェック
		LPTSTR pExt = PathFindExtension(pszEditFile);
		if (*pExt == '.') {
			CString ext = pExt;
			ext.MakeUpper();

			if (GetRegText(IDS_SEC_ADDIN_EDITOR, (LPCTSTR)ext, str) > 0)
				return DoAddin(hParent, str, pszEditFile, pszDefaultDir);
		}

		// 画像ファイルならPixefLTで編集
		if (imgSpecifyImageType(pszEditFile)) {
//@			return lccRunDll(hParent, DLL_GMIL, _T("pxfRun"), name);
			TCHAR exec[MAX_PATH * 2];
			if (lccRegGetAppPath(_T("pixeflt.exe"), exec, ARRAYSIZE(exec))) {
				return InvokeFile(hParent, exec, name);
			}
		}
		if (dir.IsEmpty()) {
			PathGetParent(pszEditFile, dir.GetBuffer(MAX_PATH * 2));
			dir.ReleaseBuffer();
		}
	}

	if (!GetAddin(addinEditor, str))
		return lccDllExecute(hParent, DLL_MIL, MIL_EDIT, name, _T("/edit"), FALSE);

	if (pszEditFile && *pszEditFile)
		return DoAddin(hParent, str, pszEditFile, pszDefaultDir);

	HINSTANCE hInst = ShellExecute(hParent, NULL, str, NULL, dir, SW_SHOWNORMAL);
	if (hInst <= (HINSTANCE)32) {
		DWORD dw = GetLastError();
		lccErrorBox(hParent, dw, APP_PRODUCT, str, MB_STOP);
		return FALSE;
	}
	return TRUE;
}

BOOL AddinViewer(HWND hParent, LPCTSTR pszFile)
{
	CString str;
	if (GetAddin(addinViewer, str))
		return DoAddin(hParent, str, pszFile, NULL);

	// バイナリファイルの判定をおこなう
	HANDLE hf = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	// 0x01 ～ 0x08 の出現回数でチェック（unicodeは0x00を含むのでカウントしない）
	BYTE cb[512];
	DWORD cnt = 0;
	ReadFile(hf, cb, sizeof(cb), &cnt, NULL);
	CloseHandle(hf);

	int bin = 0;
	for (DWORD i = 0; i < cnt; i++) {
		if (1 <= cb[i] && cb[i] <= 8) bin++;
	}
	if (bin > 1) {
		int res = IDCANCEL;
		// vista以降はTaskDialogを利用
		const TASKDIALOG_BUTTON taskButtons[]   = { 
			{ IDS_OPEN_AS_TEXT,		MAKEINTRESOURCE(IDS_OPEN_AS_TEXT) },
			{ IDS_OPEN_AS_BINARY,	MAKEINTRESOURCE(IDS_OPEN_AS_BINARY) },
			{ IDS_OPEN_WITH,		MAKEINTRESOURCE(IDS_OPEN_WITH) }				// win8以降
		};

		TASKDIALOGCONFIG tdc;
		memset(&tdc, 0, sizeof(tdc));
		tdc.cbSize =				sizeof(tdc);
		tdc.hwndParent =			hParent;
		tdc.hInstance =				RESINST;
		tdc.dwFlags =				TDF_ALLOW_DIALOG_CANCELLATION/* | TDF_USE_COMMAND_LINKS*/;
		tdc.dwCommonButtons =		TDCBF_CANCEL_BUTTON;
		tdc.pszWindowTitle =		MAKEINTRESOURCE(IDR_MAINFRAME);
		tdc.pszMainIcon =			MAKEINTRESOURCE(IDI_MBX_WARNING);
		CString tit;
		tit.Format(IDS_QUERY_OPENBY, PathFindFileName(pszFile));
		tdc.pszMainInstruction =	tit.GetString();
		tdc.pButtons =				taskButtons;
		tdc.cButtons =				ARRAYSIZE(taskButtons);
		tdc.nDefaultButton =		IDS_OPEN_AS_TEXT;
		res = 0;
		if (TaskDialogIndirect(&tdc, &res, NULL, NULL) != S_OK) {
			res = IDCANCEL;
		}

		switch (res) {
		case IDCANCEL:
			return FALSE;
		case IDS_OPEN_AS_BINARY:
			// HexViewerで表示
			str.Format(_T("\"%s\""), pszFile);
			return lccDllExecute(hParent, DLL_BED, BED_EDIT, str, _T("/bdump"), FALSE);
		case IDS_OPEN_WITH:
		{
			SHELLEXECUTEINFO sei = {0};
			sei.cbSize = sizeof(SHELLEXECUTEINFO);
			sei.fMask = SEE_MASK_UNICODE;
			sei.hwnd = hParent;
			sei.nShow = SW_SHOWNORMAL;
			sei.lpFile = _T("openwith.exe");
			sei.lpParameters = pszFile;
			ShellExecuteEx(&sei);
			return TRUE;
		}
		}
	}
	str.Format(_T("/r \"%s\""), pszFile);
	return lccDllExecute(hParent, DLL_MIL, MIL_EDIT, str, _T("/edit"), FALSE);
}

BOOL AddinHexEditor(HWND hParent, LPCTSTR pszFile)
{
	CString str;
	if (GetAddin(addinBDump, str))
		return DoAddin(hParent, str, pszFile, NULL);

	str.Format(_T("\"%s\""), pszFile);
	return lccDllExecute(hParent, DLL_BED, BED_EDIT, str, _T("/bdump"), FALSE);
}

BOOL AddinImageViewer(HWND hParent, LPCTSTR pszFile, bool bRunAsViewer)
{
	CString str;
	if (GetAddin(addinBmpViewer, str))
		return DoAddin(hParent, str, pszFile, NULL);

//	str.Format(_T("\"%s\""), pszFile);
//	return lccRunDll(hParent, DLL_GMIL, bRunAsViewer ? _T("imgRun") : _T("pxfRun"), str);

	str.Format(_T("/%c \"%s\""), bRunAsViewer ? _T('v') : _T('e'), pszFile);
	TCHAR exec[MAX_PATH * 2];
	if (lccRegGetAppPath(_T("pixeflt.exe"), exec, ARRAYSIZE(exec))) {
		return InvokeFile(hParent, exec, str.GetString());
	}
	return FALSE;
}

BOOL DoAddin(HWND hParent, LPCTSTR pExe, LPCTSTR pFile, LPCTSTR pszDefaultDir)
{
	// #であれば何もしない
	if (!pExe || *pExe == _T('#')) return TRUE;

	bool bAsAdmin = false;
	if (*pExe == _T('@')) {
		bAsAdmin = true;
		pExe++;
	}
	CString cmd, exec;
	TCHAR file[MAX_PATH * 2];
	if (*pExe == _T('"')) {
		exec = pExe + 1;
		LPTSTR p = StrChr(exec.GetString(), _T('"'));
		if (p)
			*p++ = 0;
		while (*p == _T(' '))
			p++;
		cmd = p;
	} else {
		exec = pExe;
		LPTSTR p = StrChr(exec.GetString(), _T(' '));
		if (p) {
			*p++ = 0;
			cmd = p;
		}
	}

	lstrcpy(file, pFile);
	PathUnquoteSpaces(file);

	CString dir;
	if (pszDefaultDir && *pszDefaultDir)
		dir = pszDefaultDir;
	else {
		dir = file;
		PathRemoveFileSpec((LPTSTR)dir.GetString());
	}

	LONG dwBinType;
	if (lccGetBinaryType(exec.GetString(), &dwBinType) &&
		(dwBinType == SCS_DOS_BINARY ||
		 dwBinType == SCS_WOW_BINARY ||
		 dwBinType == SCS_PIF_BINARY)) {
		GetShortPathName(file, file, ARRAYSIZE(file));
	} else {
		PathQuoteSpaces(file);
	}
	if (!cmd.IsEmpty()) {
		cmd.AppendFormat(_T(" %s"), file);
		lstrcpy(file, cmd.GetString());
	}

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_UNICODE;
	sei.hwnd = fvApp.wndMain;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpVerb = bAsAdmin ? _T("runas") : NULL;
	sei.lpFile = exec.GetString();
	sei.lpParameters = file;
	sei.lpDirectory = dir.GetString();
	if (!ShellExecuteEx(&sei)) {
		DWORD dw = GetLastError();
		lccErrorBox(hParent, dw, APP_PRODUCT, exec, MB_STOP);
		return FALSE;
	}
	return TRUE;
}

BOOL ExecAddin(HWND hParent, LPCTSTR pszName, LPCITEMIDLIST pidlFq)
{
	__attension_app();

	if (!pszName || *pszName == 0)
		return FALSE;

	LPTSTR pe = PathFindExtension(pszName);
	if (*pe != '.')
		return FALSE;

	// 最初にアドインビュアをチェック
	CString str;
	if (GetRegText(IDS_SEC_ADDIN_VIEWER, pe, str) > 0)
		return DoAddin(hParent, str, pszName, NULL);

	// PerceivedTypeのエントリでチェック
	// （ファイルがwin8以降のデスクトップアプリに関連付けされている場合はこの手法で）
	TCHAR types[256];
	if (lccRegGetPerceivedType(pszName, types, ARRAYSIZE(types))) {
		if (!lstrcmpi(types, _T("video")) ||
			!lstrcmpi(types, _T("audio"))) {
/*
			SHELLEXECUTEINFO se = {0};
			se.cbSize = sizeof(se);
			se.fMask = SEE_MASK_INVOKEIDLIST;
			se.hwnd = hParent;
			se.lpFile = pszName;
			se.nShow = SW_SHOWNORMAL;
			if (ShellExecuteEx(&se)) { return TRUE; }
*/
			if (InvokeFile(hParent, pszName)) { return TRUE; }
		}
	}

	// まず拡張子の種類「だけ」で判定する
	pe++;

	// -----------------------------
	// 書庫ファイル
	// -----------------------------
	int type = SpecifyArchiveType(pszName);
	if (type != ARC_ERROR && type != ARC_ISH) {
		ArcOpen(fvApp.wndMain, pszName);
		return TRUE;
	}
	// -----------------------------
	// 音源ファイル
	// -----------------------------
	if (checktoken(_T("wav mp3"), pe, ' ')) {
/* @@@
		if (!sndPlaySound((LPSTR)pszName, SND_ASYNC))
			return FALSE;
		return TRUE;
@@@ */
		InvokeFile(hParent, pszName);
		return TRUE;
	}
	// -----------------------------
	// フォントファイル
	// -----------------------------
	if (checktoken(_T("ttf ttc otf fon"), pe, ' ')) {
		InvokeFile(hParent, pszName);
		return TRUE;
	}
	// -----------------------------
	// アイコンファイル
	// -----------------------------
	if (!StrCmpI(pe, _T("ico"))) {
		HDROP hdrop = MakeDropHandleFromPath(pszName);
		if (hdrop) {
			fopShowFileInfoEx(hParent, hdrop, FINFO_ICON);
			DragFinish(hdrop);
			return TRUE;
		}
	}
	// -----------------------------
	// タスク(.job)ファイル
	// -----------------------------
	if (!StrCmpI(pe, _T("job"))) {
		if (pidlFq) {
			InvokeIDList(hParent, pidlFq, gszVerbProp);
			return TRUE;
		}
	}
	// -----------------------------
	// 実行・DLLファイル
	// -----------------------------
	if (checktoken(_T("exe dll cpl drv ocx vbx vxd 386 w95 qtc"), pe, ' ')) {
		if (!StrCmpI(pe, _T("exe")) && ArcIsSFX(hParent, pszName) != ARC_ERROR)
			ArcOpen(fvApp.wndMain, pszName);
		else
			ViewFileInformation(hParent, pszName);
		return TRUE;
	} else if (imgSpecifyImageType(pszName)) {
		return AddinImageViewer(fvApp.wndMain, pszName, true);
	} else {
		// -----------------------------
		// ファイルのヘッダで判定する
		// -----------------------------
		int atype = SpecifyArchiveType(pszName);
		if (atype != ARC_ERROR && atype != ARC_ISH) {
			ArcOpen(fvApp.wndMain, pszName);
			return TRUE;
		}
	}
	return FALSE;
}

void ExecNewFrame(LPCITEMIDLIST pidl, LPCTSTR pszParam, bool bAsAdmin)
{
	CString cmd = pszParam;
	if (pidl) {
		TCHAR buf[MAX_PATH];
		if (RegSaveGuidOperation(pidl, ILGetSize(pidl), buf)) {
			cmd.AppendFormat(_T(" /guid %s"), buf);
			cmd.TrimLeft();
		}
	}
	InvokeFile(fvApp.wndMain, fvApp.GetMyModuleName(), cmd, NULL, bAsAdmin ? _T("runas") :  NULL);
}

void ExecNewFrame(LPCTSTR pszFile, LPCTSTR pszParam, bool bAsAdmin)
{
	CIDListPtr idl;
	if (idl.CreateFromPath(pszFile)) {
		ExecNewFrame(idl, pszParam, bAsAdmin);
	}
}

BOOL GetAddin(int nEntry, CString& str)
{
	str.Empty();
	return (GetRegText(IDS_SEC_ADDIN_APPS, IDS_KEY_MSDOS + nEntry, str) > 0);
}

void SetAddin(int nEntry, LPCTSTR pszAddin)
{
	SetRegText(IDS_SEC_ADDIN_APPS, IDS_KEY_MSDOS + nEntry, pszAddin);
}

////////////////////////////////////////////////////////////////////////

static BOOL win32_exec_ex(LPCTSTR pszCmdLine, LPCTSTR pszDir, UINT nShowCmd, bool bAsAdmin)
{
	TCHAR exe[MAX_PATH * 2];
	TCHAR cmd[MAX_PATH * 2];
	LPTSTR p;
	if (*pszCmdLine == '"') {
		lstrcpy(exe, pszCmdLine + 1);
		p = StrChr(exe, '"');
		if (p) {
			*p++ = 0;
			while (*p == ' ')
				p++;
		}
		if (p && *p)
			lstrcpy(cmd, p);
		else
			cmd[0] = 0;
	} else {
		lstrcpy(exe, pszCmdLine);
		p = StrChr(exe, ' ');
		if (p) {
			*p++ = 0;
			lstrcpy(cmd, p);
		}
		else
			cmd[0] = 0;
	}

	SHELLEXECUTEINFO se = {0};
	se.cbSize = sizeof(se);
	se.fMask = 0;
	se.hwnd = GetFocus();
	se.lpVerb = bAsAdmin ? _T("runas") : NULL;
	se.lpFile = exe;
	se.lpParameters = cmd;
	se.lpDirectory = pszDir;
	se.nShow = nShowCmd;
	return ShellExecuteEx(&se);
}

BOOL ExecuteAsAdmin(HWND hwnd, LPCTSTR pszCmdLine, LPCTSTR pszDir, UINT nShowCmd, bool bAsAdmin)
{
	UINT mode = SetErrorMode(0);
	HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
	BOOL bOK = win32_exec_ex(pszCmdLine, pszDir, nShowCmd, bAsAdmin);
	if (!bOK) {
		LPTSTR p;
		TCHAR cmdline[MAX_PATH * 2];
		lstrcpy(cmdline, pszCmdLine);
		p = StrChr(cmdline, ' ');
		if (p)
			*p++ = 0;

		SHELLEXECUTEINFO se = {0};
		se.cbSize = sizeof(se);
		se.fMask = 0;
		se.hwnd = GetFocus();
		se.lpVerb = bAsAdmin ? _T("runas") : NULL;
		se.lpFile = cmdline;
		se.lpParameters = p;
		se.lpDirectory = pszDir;
		se.nShow = nShowCmd;
		bOK = ShellExecuteEx(&se);
	}
	SetCursor(hcur);
	SetErrorMode(mode);
	return bOK;
}

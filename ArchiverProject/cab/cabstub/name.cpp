/////////////////////////////////////////////////////////////////////////////
//

#include "stdafx.h"
#include "cabstub.h"

/////////////////////////////////////////////////////////////////////////////
// NameDlgFn dialog

class CNameDlg
{
public:
	HWND m_hWnd;
	CSfxInfo* m_psi;

public:
	CNameDlg()
		: m_hWnd(NULL)
		, m_psi(NULL)
	{
	}
	~CNameDlg()
	{
	}

	enum { IDD = IDD_EXTRACTTO };

	INT_PTR DoModal(HWND hwndParent)
	{
		return ::DialogBoxParam(ghInst, MAKEINTRESOURCE(IDD), hwndParent, DlgProcEntry, (LPARAM)this);
	}

	static INT_PTR CALLBACK DlgProcEntry(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(wParam);

		if (msg == WM_INITDIALOG) {
			CNameDlg* pThis = (CNameDlg*)lParam;
			pThis->m_hWnd = hwnd;
#ifdef _WIN64
			SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pThis);
#else
			SetWindowLongPtr(hwnd, DWLP_USER, PtrToLong(pThis));
#endif
			return pThis->OnInitDialog(lParam);
		}
		else if (msg == WM_COMMAND) {
#ifdef _WIN64
			CNameDlg* pThis = (CNameDlg*)GetWindowLongPtr(hwnd, DWLP_USER);
#else
			CNameDlg* pThis = (CNameDlg*)LongToPtr(GetWindowLongPtr(hwnd, DWLP_USER));
#endif
			switch (LOWORD(wParam)) {
			case IDOK:
				pThis->OnOK();
				return TRUE;
			case IDCANCEL:
				pThis->OnCancel();
				return TRUE;
			default:
				break;
			}
		}
		return FALSE;
	}

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		if (!m_psi) {
			::EndDialog(m_hWnd, IDCANCEL);
			return TRUE;
		}

		CenterWindow(m_hWnd, NULL);

		if (m_psi->m_strTitle)
			SetWindowTextW(m_hWnd, m_psi->m_strTitle);
		if (m_psi->m_strMessage)
			SetDlgItemTextW(m_hWnd, IDC_TITLE_EXTRACT, m_psi->m_strMessage);
		if (m_psi->m_strPath)
			SetDlgItemTextW(m_hWnd, IDC_PATH, m_psi->m_strPath);
		return TRUE;
	}

	void OnOK()
	{
		WCHAR buf[MAX_PATH], cur[MAX_PATH];
		GetCurrentDirectoryW(ARRAYSIZE(cur), cur);
		if (GetDlgItemTextW(m_hWnd, IDC_PATH, buf, ARRAYSIZE(buf)) < 1)
			wcscpy(buf, cur);
		else {
			if (!xPathIsDirectory(buf)) {
				if (AppMessage(IDS_FOLDER_NOTEXIST, buf, MB_YESNO | MB_ICONQUESTION) != IDYES)
					return;
				if (!xPathMakeTree(buf)) {
					AppMessage(IDS_FOLDER_CREATE_ERROR, buf, MB_OK | MB_ICONSTOP);
					return;
				}
			}
			SetCurrentDirectoryW(buf);
			GetCurrentDirectoryW(ARRAYSIZE(buf), buf);
			SetCurrentDirectoryW(cur);
		}

		alloc_wcscpy(&m_psi->m_strPath, buf);
		::EndDialog(m_hWnd, IDOK);
	}

	void OnCancel()
	{
		::EndDialog(m_hWnd, IDCANCEL);
	}
};

/////////////////////////////////////////////////////////////////////////////
//

BOOL GetExtractPath(CSfxInfo* psi)
{
	WCHAR wsz[MAX_PATH * 2];
	if (psi->m_strPath) {
		WCHAR* env = new WCHAR[MAX_PATH * 2];
		wcscpy(env, psi->m_strPath);
		GetSystemDirectoryW(wsz, ARRAYSIZE(wsz));
		ReplaceString(env, L"%system%", wsz);

		xPathGetSpecialFilesDir(L"ProgramFilesDir", wsz, ARRAYSIZE(wsz));
		ReplaceString(env, L"%program%", wsz);

		xPathGetSpecialFilesDir(L"CommonFilesDir", wsz, ARRAYSIZE(wsz));
		ReplaceString(env, L"%common%", wsz);

		// その他の環境変数を置き換える
		if (ExpandEnvironmentStringsW(env, wsz, ARRAYSIZE(wsz)) < 1)
			wcscpy(wsz, env);
		delete [] env;
	}
	else
		GetCurrentDirectoryW(ARRAYSIZE(wsz), wsz);
	xPathAddBackslashW(wsz);
	alloc_wcscpy(&psi->m_strPath, wsz);

	if (psi->m_wStat & SFX_NOQUERYPATH) {
		// セキュリティ上好ましくないので、Windowsドライブが指定された場合は
		// 解凍先の確認をおこなう
		int drive_no = xPathGetDriveNumber(psi->m_strPath);

		GetWindowsDirectoryW(wsz, ARRAYSIZE(wsz));

		if (drive_no != xPathGetDriveNumber(wsz))
			return TRUE;

		psi->m_wStat &= ~SFX_NOQUERYPATH;
	}

	CNameDlg dlg;
	dlg.m_psi = psi;
	return (dlg.DoModal(NULL) == IDOK);
}

/////////////////////////////////////////////////////////////////////////////
// CQueryCopyDlg

class CQueryCopyDlg
{
public:
	HWND m_hWnd;
	WCHAR m_wsz[MAX_PATH * 2];
	int m_action;
	bool m_yesall;

public:
	CQueryCopyDlg()
		: m_hWnd(NULL)
		, m_action(ACTION_IGNORE)
		, m_yesall(false)
	{
		m_wsz[0] = 0;
	}
	~CQueryCopyDlg()
	{
	}

	enum { IDD = IDD_QUERYCOPY };

	INT_PTR DoModal(HWND hwndParent)
	{
		return ::DialogBoxParam(ghInst, MAKEINTRESOURCE(IDD), hwndParent, DlgProcEntry, (LPARAM)this);
	}

	static INT_PTR CALLBACK DlgProcEntry(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(wParam);

		if (msg == WM_INITDIALOG) {
			CQueryCopyDlg* pThis = (CQueryCopyDlg*)lParam;
			pThis->m_hWnd = hwnd;
#ifdef _WIN64
			SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pThis);
#else
			SetWindowLongPtr(hwnd, DWLP_USER, PtrToLong(pThis));
#endif
			return pThis->OnInitDialog(lParam);
		}
		else if (msg == WM_COMMAND) {
#ifdef _WIN64
			CQueryCopyDlg* pThis = (CQueryCopyDlg*)GetWindowLongPtr(hwnd, DWLP_USER);
#else
			CQueryCopyDlg* pThis = (CQueryCopyDlg*)LongToPtr(GetWindowLongPtr(hwnd, DWLP_USER));
#endif
			switch (LOWORD(wParam)) {
			case IDOK:
				pThis->OnOK();
				return TRUE;
			case IDC_YESALL:
				pThis->OnYesAll();
				return TRUE;
			case IDCANCEL:
				pThis->OnCancel();
				return TRUE;
			default:
				break;
			}
		}
		return FALSE;
	}

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		CenterWindow(m_hWnd, NULL);

		SetDlgItemTextW(m_hWnd, IDC_TEXT, m_wsz);
		if (m_action == ACTION_IGNORE)
			CheckDlgButton(m_hWnd, IDC_ACTION_IGNORE, TRUE);
		else
			CheckDlgButton(m_hWnd, IDC_ACTION_OVERWRITE, TRUE);
		return TRUE;
	}

	void OnOK()
	{
		m_action = IsDlgButtonChecked(m_hWnd, IDC_ACTION_IGNORE) ? ACTION_IGNORE : ACTION_OVERWRITE;
		::EndDialog(m_hWnd, IDOK);
	}

	void OnYesAll()
	{
		m_yesall = true;
		OnOK();
	}

	void OnCancel()
	{
		::EndDialog(m_hWnd, IDCANCEL);
	}
};

/////////////////////////////////////////////////////////////////////////////
//

int GetCopyAction(LPCWSTR psz, int defaction, bool& yesall)
{
	CQueryCopyDlg dlg;
	dlg.m_action = defaction;
	wcscpy(dlg.m_wsz, psz);

	int res = ACTION_CANCEL;
	if (dlg.DoModal(NULL) == IDOK) {
		res = dlg.m_action;
		yesall = dlg.m_yesall;
	}
	return res;
}

// name.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// QueryDirDlg message handlers

class CQueryFolderDlg : public CDialogImpl<CQueryFolderDlg>, public CDialogResize<CQueryFolderDlg>
{
public:
	CQueryFolderDlg() { }

	enum { IDD = IDD_QUERYFOLDER };

	CString m_strRoot;
	CString m_strTo;
	CFolderTreeCtrl m_tree;
	CComboBox m_combo;
	BOOL m_threaded;
	BOOL m_asadmin;

BEGIN_MSG_MAP(CQueryFolderDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CQueryFolderDlg>)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDC_THREADED, OnThreaded)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(FTWN_SELCHANGED, IDC_TREE, OnNotifyTreeChanged)
	ON_NOTIFY(FTWN_QUERYHIDDEN, IDC_TREE, OnNotifyTreeQueryHidden)
	ON_CBN_SELCHANGE(IDC_PATH, OnHistoryChanged)
	ON_WM_DESTROY()
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CQueryFolderDlg)
	DLGRESIZE_CONTROL(IDC_PATH,			DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_TREE,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_ASADMIN,		DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_THREADED,		DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		DlgResize_Init();
		CenterWindow();

		// ダイアログアイコン
		SHFILEINFO sfi = { 0 };
		SHGetFileInfo(m_strRoot, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);
		HIMAGELIST himl, hims;
		Shell_GetImageLists(&himl, &hims);
		SetIcon(ImageList_GetIcon(himl, sfi.iIcon, ILD_TRANSPARENT), true);
		SetIcon(ImageList_GetIcon(hims, sfi.iIcon, ILD_TRANSPARENT), false);

		// asadmin
		CheckDlgButton(IDC_ASADMIN, m_asadmin);

		m_combo = GetDlgItem(IDC_PATH);
		m_tree = GetDlgItem(IDC_TREE);
		m_tree.SetCustomColors(gColors.GetSize(), gColors);

		// ヒストリの設定
		RegLoadHistoryEx(m_combo, IDS_SEC_HISTORY, IDS_KEY_COPYTARGET, MAXACCESS);
		m_combo.SetWindowText(m_strTo);

		// フォルダツリー
		if (!m_strRoot.IsEmpty())
			m_tree.SetRoot(m_strRoot);
		if (_pathisdirectory(m_strTo) || (PathIsUNC(m_strTo) && (gnNetSearchResult > 0)))
			m_tree.SelectByPath(m_strTo);

		// オートコンプリート
		SHAutoComplete(m_combo.GetDlgItem(1001), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYSTEM);

		// デフォルトボタン
		if (m_threaded) {
			GetDlgItem(IDOK).ModifyStyle(BS_DEFPUSHBUTTON, 0);
			GetDlgItem(IDC_THREADED).ModifyStyle(0, BS_DEFPUSHBUTTON);
		} else {
			GetDlgItem(IDC_THREADED).ModifyStyle(BS_DEFPUSHBUTTON, 0);
			GetDlgItem(IDOK).ModifyStyle(0, BS_DEFPUSHBUTTON);
		}

		// ダイアログ位置を復帰
		RegLoadWindowPlacement(m_hWnd, IDS_KEY_DLG_QUERYFOLDER);

		m_combo.SetFocus();
		return FALSE;
	}

	BOOL CheckParam()
	{
		CString str;
		if (m_combo.GetWindowText(str) < 1) {
			MessageBeep(0);
			m_combo.SetFocus();
			return FALSE;
		}

		if (!_pathisdirectory(str)) {
			if (AppMessage(IDS_QUERY_MKDIR, str, MB_QYESNO) != IDYES)
				return FALSE;
			if (!fopMakeTree(str))
				return FALSE;
		}
		m_strTo = str;
		m_asadmin = IsDlgButtonChecked(IDC_ASADMIN);
		return TRUE;
	}

	void OnOK()
	{
		if (CheckParam()) {
			m_threaded = FALSE;
			EndDialog(IDOK);
		}
	}

	void OnThreaded()
	{
		if (CheckParam()) {
			m_threaded = TRUE;
			EndDialog(IDOK);
		}
	}

	void OnCancel() { EndDialog(IDCANCEL); }

	void OnHistoryChanged()
	{
		int sel = m_combo.GetCurSel();
		if (sel >= 0) {
			CString str;
			if (m_combo.GetLBText(sel, str) > 0 && (!PathIsUNC(str) || (gnNetSearchResult > 0)))
				m_tree.SelectByPath(str);
		}
	}

	void OnNotifyTreeChanged(NMHDR*, LRESULT*)
	{
		DWORD attr;
		LPITEMIDLIST pidl = m_tree.GetSelItem(&attr);
		if (pidl) {
			TCHAR name[MAX_PATH * 2];
			if (!SHGetPathFromIDList(pidl, name))
				name[0] = 0;
			m_combo.SetWindowText(name);
		}
	}

	void OnNotifyTreeQueryHidden(NMHDR*, LRESULT* pResult)
	{
		*pResult = FTH_DEFAULT;
		if (gnFileStat & FTS_SUPERHIDDEN) *pResult = FTH_SYSHIDDEN;
		if (gnFileStat & FTS_HIDDEN) *pResult = FTH_HIDDEN;
	}

	void OnDestroy()
	{
		RegSaveWindowPlacement(m_hWnd, IDS_KEY_DLG_QUERYFOLDER);
		RemoveDialogIcon(m_hWnd);
	}

	BOOL OnHelpInfo(HELPINFO* pInfo)
	{
		if (pInfo->iContextType == HELPINFO_WINDOW && pInfo->iCtrlId) {
			ShowCtrlHelp(GetDlgItem(pInfo->iCtrlId), pInfo->iCtrlId);
			return TRUE;
		}
		return FALSE;
	}
};

BOOL QueryFolder(HWND hwnd, LPCTSTR pszRoot, LPTSTR pszPath, int cchMax, BOOL& bThreaded, BOOL& bAsAdmin)
{
	if (!hwnd || !IsWindow(hwnd)) hwnd = fvApp.wndMain;

	CQueryFolderDlg dlg;
	dlg.m_strRoot = pszRoot;
	dlg.m_strTo = pszPath;
	dlg.m_threaded = bThreaded;
	dlg.m_asadmin = bAsAdmin;

	if (dlg.DoModal(hwnd) == IDOK) {
		lstrcpyn(pszPath, dlg.m_strTo, cchMax);
		bThreaded = dlg.m_threaded;
		bAsAdmin = dlg.m_asadmin;
		return TRUE;
	}
	return FALSE;
}

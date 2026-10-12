// name.cpp : implementation file
//

#include "stdafx.h"
#include "common.h"
#include "resource.h"

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
	CQueryFolderDlg() : m_pTo(NULL), m_bCopy(TRUE) { }

	enum { IDD = IDD_QUERYFOLDER };

	BOOL m_bCopy;
	LPTSTR m_pTo;
	CFolderTreeCtrl m_tree;
	CComboBox m_combo;

BEGIN_MSG_MAP(CQueryFolderDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CQueryFolderDlg>)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_COMMAND(IDC_TREE, OnTreeChanged)
	ON_CBN_SELCHANGE(IDC_PATH, OnHistoryChanged)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CQueryFolderDlg)
	DLGRESIZE_CONTROL(IDC_TEXT,			DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_PATH,			DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_TREE,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		DlgResize_Init();
		CenterWindow();
		m_tree = GetDlgItem(IDC_TREE);
		m_combo = GetDlgItem(IDC_PATH);

		CString str;
		str.LoadString(m_bCopy ? IDS_CAPTION_COPY : IDS_CAPTION_MOVE);
		SetDlgItemText(IDC_TEXT, str);

		// ヒストリの設定
		CAppReg reg;
		reg.LoadComboHistory(m_combo, IDS_SEC_SETTING, IDS_KEY_COPYHISTORY, 16);

		m_combo.SetWindowText(m_pTo);
		if (PathIsDirectory(m_pTo) || PathIsUNC(m_pTo))
			m_tree.SelectByPath(m_pTo);

		// オートコンプリート
		SHAutoComplete(m_combo.GetDlgItem(1001), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);

		m_combo.SetFocus();
		return FALSE;
	}

	void OnOK()
	{
		TCHAR buf[MAX_PATH];
		if (m_combo.GetWindowText(buf, _COUNTOF(buf)) < 1) {
			MessageBeep(0);
			m_combo.SetFocus();
			return;
		}

		if (!PathIsDirectory(buf)) {
			if (AppMessage(m_hWnd, IDS_QUERY_MKDIR, buf, MB_QYESNO) != IDYES)
				return;
			if (!fopMakeTree(buf))
				return;
		}
		lstrcpy(m_pTo, buf);

		// ヒストリ保存
		CAppReg reg;
		reg.SaveMruHistory(IDS_SEC_SETTING, IDS_KEY_COPYHISTORY, 16, buf);

		EndDialog(IDOK);
	}

	void OnCancel() { EndDialog(IDCANCEL); }

	void OnHistoryChanged()
	{
		int sel = m_combo.GetCurSel();
		if (sel >= 0) {
			TCHAR buf[MAX_PATH];
			if (m_combo.GetLBText(sel, buf) > 0)
				m_tree.SelectByPath(buf);
		}
	}

	void OnTreeChanged()
	{
		DWORD attr;
		LPITEMIDLIST pidl = m_tree.GetSelItem(&attr);
		if (pidl) {
			TCHAR buf[MAX_PATH * 2];
			if (!SHGetPathFromIDList(pidl, buf))
				buf[0] = '\0';
			else if (attr & SFGAO_LINK) {
				// ターゲットフォルダがリンク属性を持っていることがあるので
				// 実際のリンク先を取得する
				FSLINKINFO li;
				if (fopParseLinkInfo(buf, &li))
					lstrcpy(buf, li.szTargetName);
			}
			m_combo.SetWindowText(buf);
		}
	}
};

BOOL QueryFolder(HWND hwnd, LPTSTR pszPath, BOOL bCopy)
{
	CQueryFolderDlg dlg;
	dlg.m_pTo = pszPath;
	dlg.m_bCopy = bCopy;
	return (dlg.DoModal(hwnd) == IDOK);
}

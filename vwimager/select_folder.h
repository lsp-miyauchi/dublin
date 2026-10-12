/////////////////////////////////////////////////////////////////////////
// select_folder.h
//

#pragma once
#ifndef __SELECT_FOLDER_H__
#define __SELECT_FOLDER_H__

#include "reg.h"
#include "pidl.h"
#include "resource.h"

BOOL CheckDirectoryNameUI(HWND hwnd, LPCTSTR pszPath);
BOOL CheckServerUI(HWND hwnd, LPCTSTR pszFolder);


/////////////////////////////////////////////////////////////////////////////
//

typedef struct tagLOGDISK {
	LPITEMIDLIST pidlBase;
	LPITEMIDLIST pidlTarget;
	TCHAR base[MAX_PATH * 2];
} LOGDISK;

#define MAX_SELFOL_HISTORY			25


/////////////////////////////////////////////////////////////////////////////
// SelectFolderDlg message handlers

class CSelectFolderDlg : public CDialogImpl<CSelectFolderDlg>, public CDialogResize<CSelectFolderDlg>
{
private:
	LOGDISK m_ld;
	CFolderTreeCtrl m_tree;
	CComboBox m_combo;

public:
	CSelectFolderDlg()
	{
		memset(&m_ld, 0, sizeof(m_ld));
	}
	~CSelectFolderDlg()
	{
	}

	enum { IDD = IDD_SELECTFOLDER };

private:

BEGIN_MSG_MAP(CSelectFolderDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CSelectFolderDlg>)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_COMMAND(IDC_TREE, OnTreeChanged)
	ON_CBN_SELCHANGE(IDC_PATH, OnHistoryChanged)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CSelectFolderDlg)
	DLGRESIZE_CONTROL(IDC_PATH,			DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_TREE,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		DlgResize_Init();

		m_tree = GetDlgItem(IDC_TREE);
		m_combo = GetDlgItem(IDC_PATH);

		TCHAR buf[MAX_PATH];
		if (m_ld.base[0])
			m_combo.SetWindowText(buf);

		m_tree.SelectByIDList(m_ld.pidlBase);

		// ヒストリの設定
		CAppReg reg;
		reg.LoadComboHistory(m_combo, IDS_SEC_SELECTFOLDER, IDS_KEY_HISTORY, MAX_SELFOL_HISTORY);

		// オートコンプリート
		SHAutoComplete(m_combo.GetDlgItem(1001), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);

		m_combo.SetWindowText(NULL);
		m_combo.SetFocus();

		// ウィンドウ位置
		WINDOWPLACEMENT wpm;
		if (reg.GetBinary(IDS_SEC_SELECTFOLDER, IDS_KEY_WNDPOS, (BYTE*)&wpm, sizeof(wpm)) == sizeof(wpm)) {
			lccMapRectToProgmanPane(&wpm.rcNormalPosition);
			wpm.showCmd = SW_SHOWNORMAL;
			SetWindowPlacement(&wpm);
		}
		else
			CenterWindow();
		return FALSE;
	}

	void OnOK()
	{
		DWORD attr;
		LPITEMIDLIST pidl = m_tree.GetSelItem(&attr);
		if (!pidl) {
			MessageBeep(0);
			return;
		}

		m_ld.pidlTarget = NULL;
		CString str;
		int len = m_combo.GetWindowText(str);
		if (len < 1)
			m_ld.pidlTarget = ILClone(pidl);
		else {
			CString tmp;
			if (idl_GetPathFromIDList(pidl, tmp) && !lccComparePath(str, tmp))
				m_ld.pidlTarget = ILClone(pidl);

			if (!m_ld.pidlTarget) {
				// 環境変数パス変換
				TCHAR buf[MAX_PATH * 2];
				if (ExpandEnvironmentStrings(str, buf, ARRAYSIZE(buf)) > 0) {
					str = buf;
					len = lstrlen(str);
				}

				// ドライブのみ指定のチェック ex "C:" or "C"
				int drive = _istalpha(str[0]) ? _totupper(str[0]) - _T('A') : -1;
				if (drive >= 0 && ((len == 1) || (len == 2 && str[1] == _T(':')))) {
					if (!fopGetDirRemind(drive, buf))
						PathBuildRoot(buf, drive);
				}
				else {
					// サーバチェック
					if (PathIsUNC(str)) {
						if (!CheckServerUI(m_hWnd, str))
							return;
					}

					lstrcpy(buf, str);
					if (!PathIsUNCServer(buf) && !CheckDirectoryNameUI(m_hWnd, buf))
						return;

					if (PathFileExists(buf))
						PathRemoveFileSpec(buf);
				}
				m_ld.pidlTarget = ILCreateFromPath(buf);
			}
		}

		if (m_ld.pidlTarget) {
			CAppReg reg;
			if (idl_GetPathFromIDList(m_ld.pidlTarget, str))
				reg.SaveComboHistory(m_combo, IDS_SEC_SELECTFOLDER, IDS_KEY_HISTORY, MAX_SELFOL_HISTORY, str);

			// 位置保存
			WINDOWPLACEMENT wpm;
			wpm.length = sizeof(wpm);
			GetWindowPlacement(&wpm);
			reg.SetBinary(IDS_SEC_SELECTFOLDER, IDS_KEY_WNDPOS, &wpm, sizeof(wpm));

			EndDialog(IDOK);
		}
	}

	void OnCancel() { EndDialog(IDCANCEL); }

	void OnTreeChanged()
	{
		DWORD attr;
		LPITEMIDLIST pidl = m_tree.GetSelItem(&attr);
		if (pidl) {
			CString str;
			if (!idl_GetPathFromIDList(pidl, str))
				str.Empty();
			m_combo.SetWindowText(str);
		}
	}

	void OnHistoryChanged()
	{
		int sel = m_combo.GetCurSel();
		if (sel >= 0) {
			CString str;
			if (m_combo.GetLBText(sel, str) > 0)
				m_tree.SelectByPath(str);
		}
	}


public:
	LPITEMIDLIST SelectFolder(HWND hWnd, LPITEMIDLIST pidlBase)
	{
		m_ld.pidlBase = pidlBase;
		if (!SHGetPathFromIDList(pidlBase, m_ld.base))
			m_ld.base[0] = 0;
		m_ld.pidlTarget = NULL;
		if (DoModal(hWnd) == IDOK)
			return m_ld.pidlTarget;
		return NULL;
	}
};

#endif		// __SELECT_FOLDER_H__

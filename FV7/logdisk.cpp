// logdisk.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
//

typedef struct tagLOGDISK {
	LPCITEMIDLIST pidlBase;
	LPCITEMIDLIST pidlTarget;
	TCHAR base[MAX_PATH * 2];
} LOGDISK;

#define MAXLOGDISKHISTORY			25


/////////////////////////////////////////////////////////////////////////////
// LogDiskDlg message handlers

class CLogDiskDlg : public CDialogImpl<CLogDiskDlg>, public CDialogResize<CLogDiskDlg>
{
public:
	CLogDiskDlg() : m_bNewTab(false), m_bNewWindow(false)
	{
		memset(&m_ld, 0, sizeof(m_ld));
	}
	~CLogDiskDlg() { }

	enum { IDD = IDD_LOGDISK };

	LOGDISK m_ld;
	bool m_bNewTab;
	bool m_bNewWindow;
	CFolderTreeCtrl m_tree;
	CComboBox m_combo;

BEGIN_MSG_MAP(CLogDiskDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CLogDiskDlg>)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDC_NEWTAB, OnNewTab)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_QUICK, OnQuick)
	ON_NOTIFY(FTWN_SELCHANGED, IDC_TREE, OnNotifyTreeChanged)
	ON_NOTIFY(FTWN_QUERYHIDDEN, IDC_TREE, OnNotifyTreeQueryHidden)
	ON_CBN_SELCHANGE(IDC_PATH, OnHistoryChanged)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CLogDiskDlg)
	DLGRESIZE_CONTROL(IDC_PATH,			DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_QUICK,		DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_TREE,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_NEWTAB,		DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnNewTab();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnQuick();
	void OnNotifyTreeChanged(NMHDR* phdr, LRESULT* pResult);
	void OnNotifyTreeQueryHidden(NMHDR* phdr, LRESULT* pResult);
	void OnHistoryChanged();
};

BOOL CLogDiskDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();

	m_combo = GetDlgItem(IDC_PATH);
	m_tree = GetDlgItem(IDC_TREE);
	m_tree.SetCustomColors(gColors.GetSize(), gColors);

	TCHAR buf[MAX_PATH];
	if (m_ld.base[0])
		m_combo.SetWindowText(buf);

	m_tree.SelectByIDList(m_ld.pidlBase);

	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_QUICK), 16, RGB(255, 0, 255));
	FlatButton_SetImageList(GetDlgItem(IDC_QUICK), him);

	// 新しいタブ
	CheckDlgButton(IDC_NEWTAB, m_bNewTab);
	// ヒストリの設定
	RegLoadHistoryEx(m_combo, IDS_SEC_HISTORY, IDS_KEY_LOGDISK, MAXLOGDISKHISTORY);
	// オートコンプリート
	SHAutoComplete(m_combo.GetDlgItem(1001), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);

	m_combo.SetWindowText(NULL);
	m_combo.SetFocus();

	// ウィンドウ位置
	RegLoadWindowPlacement(m_hWnd, IDS_KEY_DLG_LOGDISK);
	return FALSE;
}

void CLogDiskDlg::OnNewTab()
{
	m_bNewTab = TRUE;
	OnOK();
}

void CLogDiskDlg::OnOK()
{
	DWORD attr;
	CIDListPtr idl;
	idl.Clone(m_tree.GetSelItem(&attr));

	CString str;
	int len = m_combo.GetWindowText(str);
	if (len > 0) {
		if (str[0] == _T('!')) {
			// 先頭文字が ! は新しいタブを開く
			m_bNewTab = true;
			str.Delete(0, 1);
		} else if (str[0] == _T('$')) {
			// 先頭文字が $ は新しいウィンドウを開く
			m_bNewWindow = true;
			str.Delete(0, 1);
		}

		if (str[0] == _T('@')) {
			str.Delete(0, 1);
			KNOWNFOLDERID id = LookupKnownFolder(str);
			if (id != GUID_NULL) {
				idl.GetKnownFolder(id);
			}
		} else {
			str.GetBuffer(MAX_PATH * 2);
			CIDListPtr tmp;
			tmp.CreateFromPath( PathAnalyze(str.GetBuffer(), str.GetAllocLength()) );
			if (idl != tmp) {
				// 環境変数パス変換
				TCHAR buf[MAX_PATH * 2];
				if (ExpandEnvironmentStrings(str, buf, ARRAYSIZE(buf)) > 0) {
					str = buf;
					len = str.GetLength();
				}

				// ドライブのみ指定のチェック ex "C:" or "C"
				int drive = ISALPHA(str[0]) ? TOUPPER(str[0]) - _T('A') : -1;
				if (drive >= 0 && ((len == 1) || (len == 2 && str[1] == _T(':')))) {
					if (!fopGetDirRemind(drive, buf))
						PathBuildRoot(buf, drive);
				} else {
					// サーバチェック
					if (PathIsUNC(str)) {
						if (!CheckServerUI(m_hWnd, str))
							return;
					}

					lstrcpy(buf, str);
					if (!PathIsUNCServer(buf) && !CheckDirectoryNameUI(m_hWnd, buf))
						return;

					if (_pathisfile(buf))
						PathRemoveFileSpec(buf);
				}
				if (idl.CreateFromPath(buf))
					RegSaveHistory(IDS_SEC_HISTORY, IDS_KEY_LOGDISK, MAXLOGDISKHISTORY, buf);
			}
		}
	}

	if (idl.isNull()) {
		MessageBeep(0);
		return;
	}

	// 位置保存
	RegSaveWindowPlacement(m_hWnd, IDS_KEY_DLG_LOGDISK);

	m_ld.pidlTarget = idl.Detach();
	EndDialog(IDOK);
}

void CLogDiskDlg::OnQuick()
{
	CRect rc;
	GetDlgItem(IDC_QUICK).GetWindowRect(&rc);
	CPoint pt(rc.left, rc.bottom);

	int openMode;
	CIDListPtr idl = TrackFavoriteMenu(m_hWnd, false, true, pt, openMode);
	if (!idl.isNull()) {
		CString str;
		if (idl_GetPathFromIDList(idl, str)) {
			if (!PathIsUNC(str) || gnNetSearchResult > 0)
				m_tree.SelectByIDList(idl);
			m_combo.SetWindowText(str);
		}
		else
			m_tree.SelectByIDList(idl);
	}
}

void CLogDiskDlg::OnHistoryChanged()
{
	int sel = m_combo.GetCurSel();
	if (sel >= 0) {
		CString str;
		if (m_combo.GetLBText(sel, str) > 0 && (!PathIsUNC(str) || (gnNetSearchResult > 0)))
			m_tree.SelectByPath(str);
	}
}

void CLogDiskDlg::OnNotifyTreeChanged(NMHDR*, LRESULT*)
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

void CLogDiskDlg::OnNotifyTreeQueryHidden(NMHDR*, LRESULT* pResult)
{
	*pResult = FTH_DEFAULT;
	if (gnFileStat & FTS_SUPERHIDDEN) *pResult = FTH_SYSHIDDEN;
	if (gnFileStat & FTS_HIDDEN) *pResult = FTH_HIDDEN;
}

LPCITEMIDLIST __fastcall LogDisk(HWND hWnd, LPITEMIDLIST pidlBase, int& mode)
{
	CLogDiskDlg dlg;
	dlg.m_ld.pidlBase = pidlBase;
	dlg.m_bNewWindow = (mode == 2);
	dlg.m_bNewTab = (mode == 1);
	if (!SHGetPathFromIDList(pidlBase, dlg.m_ld.base))
		dlg.m_ld.base[0] = 0;
	dlg.m_ld.pidlTarget = NULL;
	if (dlg.DoModal(hWnd) == IDOK) {
		mode = dlg.m_bNewWindow ? 2 : (dlg.m_bNewTab ? 1 : 0);
		return dlg.m_ld.pidlTarget;
	}
	return NULL;
}

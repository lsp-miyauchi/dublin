// name.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"
#include "mainfrm.h"

/////////////////////////////////////////////////////////////////////////////
//

class CQueryCopyInfo
{
public:
	LPCITEMIDLIST m_pidlBase;
	CString m_strTo;
	IDataObject* m_pdo;
	HDROP m_hDrop;
	BOOL bCopy;
	BOOL bTabOpen;
	BOOL bThreaded;
	BOOL bAsAdmin;

public:
	CQueryCopyInfo()
		: m_pidlBase(NULL)
		, m_pdo(NULL)
		, m_hDrop(NULL)
		, bCopy(TRUE)
		, bTabOpen(FALSE)
		, bThreaded(FALSE)
		, bAsAdmin(FALSE)
	{
	}
};


/////////////////////////////////////////////////////////////////////////////
// QueryCopyDlg message handlers


class CQueryCopyDlg : public CDialogImpl<CQueryCopyDlg>, public CDialogResize<CQueryCopyDlg>
{
public:
	CQueryCopyDlg() { }

	enum { IDD = IDD_QUERYCOPY };

	CQueryCopyInfo m_qci;
	CFolderTreeCtrl m_tree;
	CComboBox m_target;

BEGIN_MSG_MAP(CQueryCopyDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CQueryCopyDlg>)
	ON_WM_HELPINFO()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnOK);
	ON_BN_CLICKED(IDCANCEL, OnCancel);
	ON_BN_CLICKED(IDC_QUICK, OnQuick);
	ON_BN_CLICKED(IDC_JUMP, OnCheckJump);
	ON_BN_CLICKED(IDC_THREADED, OnOKThread);
	ON_BN_CLICKED(IDC_ASADMIN, OnCheckAsAdmin);
	ON_NOTIFY(FTWN_SELCHANGED, IDC_TREE, OnNotifyTreeChanged)
	ON_NOTIFY(FTWN_QUERYHIDDEN, IDC_TREE, OnNotifyTreeQueryHidden)
	ON_CBN_SELCHANGE(IDC_TARGET_PATH, OnHistoryChanged)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CQueryCopyDlg)
	DLGRESIZE_CONTROL(IDC_SOURCE_PATH,	DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_TARGET_PATH,	DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_QUICK,		DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_STATIC_FROM,	DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_STATIC_TO,	DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_TREE,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_ASADMIN,		DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_JUMP,			DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_THREADED,		DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnDestroy();
	void OnOK();
	void OnOKThread();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnQuick();
	void OnNotifyTreeChanged(NMHDR* phdr, LRESULT* pResult);
	void OnNotifyTreeQueryHidden(NMHDR* phdr, LRESULT* pResult);
	void OnHistoryChanged();
	void OnCheckJump();
	void OnCheckAsAdmin();
	bool ConfirmTargetFolder();
};

BOOL CQueryCopyDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();

	CenterWindow();

	UINT idIconResource = m_qci.bCopy ? IDI_COPY : IDI_MOVE;
	SetDialogIcon(m_hWnd, idIconResource);

	m_target = GetDlgItem(IDC_TARGET_PATH);
	m_tree = GetDlgItem(IDC_TREE);
	m_tree.SetCustomColors(gColors.GetSize(), gColors);

	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_QUICK), 16, RGB(255, 0, 255));
	FlatButton_SetImageList(GetDlgItem(IDC_QUICK), him);

	// キャプションとアイコンの設定
	CString str; str.LoadString( m_qci.bCopy ? IDS_DLGTIT_COPY : IDS_DLGTIT_MOVE);
	SetWindowText(str);

	if (m_qci.m_pidlBase) {
		CString s;
		idl_GetName(m_qci.m_pidlBase, SHGDN_INFOLDER, s);
		SetDlgItemText(IDC_PATH, s);
		m_tree.SelectByIDList(m_qci.m_pidlBase);
	}

	if (m_qci.m_pdo) {
		MakeEnumStrFromDataObject(m_qci.m_pdo, str);
		m_qci.bAsAdmin = FALSE;
		m_qci.bThreaded = FALSE;
		GetDlgItem(IDC_ASADMIN).EnableWindow(FALSE);
		GetDlgItem(IDC_THREADED).EnableWindow(FALSE);
	}
	else
		MakeEnumStrFromHdrop(m_qci.m_hDrop, str);
	SetDlgItemText(IDC_SOURCE_PATH, str);

	// ヒストリの設定
	RegLoadHistoryEx(m_target, IDS_SEC_HISTORY, IDS_KEY_COPYTARGET, MAXACCESS);

	// 管理者権限で
	CheckDlgButton(IDC_ASADMIN, m_qci.bAsAdmin);

	// デフォルトボタン
	if (m_qci.bThreaded) {
		GetDlgItem(IDOK).ModifyStyle(BS_DEFPUSHBUTTON, 0);
		GetDlgItem(IDC_THREADED).ModifyStyle(0, BS_DEFPUSHBUTTON);
	}
	else {
		GetDlgItem(IDC_THREADED).ModifyStyle(BS_DEFPUSHBUTTON, 0);
		GetDlgItem(IDOK).ModifyStyle(0, BS_DEFPUSHBUTTON);
	}

	// オートコンプリート
	SHAutoComplete(m_target.GetDlgItem(1001), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYSTEM);

	m_target.SetWindowText(NULL);
	m_target.SetFocus();

	// ダイアログ位置を復帰
	RegLoadWindowPlacement(m_hWnd, IDS_KEY_DLG_QUERYCOPY);
	return FALSE;
}

BOOL CQueryCopyDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW && pInfo->iCtrlId) {
		ShowCtrlHelp(GetDlgItem(pInfo->iCtrlId), pInfo->iCtrlId);
		return TRUE;
	}
	return FALSE;
}

void CQueryCopyDlg::OnDestroy()
{
	RegSaveWindowPlacement(m_hWnd, IDS_KEY_DLG_QUERYCOPY);
	RemoveDialogIcon(m_hWnd);
}

void CQueryCopyDlg::OnOK()
{
	if (ConfirmTargetFolder())
		EndDialog(IDOK);
}

void CQueryCopyDlg::OnOKThread()
{
	if (ConfirmTargetFolder()) {
		m_qci.bThreaded = TRUE;
		m_qci.bTabOpen = FALSE;		// tabで開くことはできない
		EndDialog(IDOK);
	}
}

bool CQueryCopyDlg::ConfirmTargetFolder()
{
	TCHAR buf[MAX_PATH * 2], to[MAX_PATH * 2];
	if (m_target.GetWindowText(buf, ARRAYSIZE(buf)) < 1) {
		MessageBeep(0);
		m_target.SetFocus();
		return false;
	}

	// 環境変数パス変換
	if (ExpandEnvironmentStrings(buf, to, ARRAYSIZE(to)) > 0)
		lstrcpy(buf, to);

	CIDListPtrHandle idl(m_qci.m_pidlBase);
	CString baseFolder;
	idl.GetPath(baseFolder);

	if (!StrChr(buf, _T('\\'))) {
		// パスセパレータが無い場合
		if (ISALPHA(buf[0]) && buf[1] == _T(':') && buf[2] == 0) {
			// ドライブのみ指定の場合（"C:" とか "D:" とか）
			int drive = PathGetDriveNumber(buf);
			if (!fopGetDirRemind(drive, to))
				PathBuildRoot(to, drive);
		}
		else {
			// リネームコピー？の設定
			if (baseFolder.IsEmpty()) {
				MessageBeep(0);
				m_target.SetFocus();
				return false;
			}
			PathCombine(to, baseFolder, buf);
		}
	}
	else {
		if (PathIsUNC(buf) || (ISALPHA(buf[0]) && buf[1] == _T(':')))
			lstrcpy(to, buf);
		else {
			if (baseFolder.IsEmpty()) {
				MessageBeep(0);
				m_target.SetFocus();
				return false;
			}
			if (buf[0] == _T('\\')) {
				lstrcpy(to, baseFolder);
				PathStripToRoot(to);
				PathAppend(to, &buf[1]);
			}
			else
				PathCombine(to, baseFolder, buf);
		}
	}
	if (PathCanonicalize(buf, to))
		lstrcpy(to, buf);

	// ディレクトリの検査…名前不正やネットワークリソースエラーのチェック
	if (!CheckDirectoryNameUI(m_hWnd, to)) return false;

	m_qci.m_strTo = to;
	return true;
}

void CQueryCopyDlg::OnQuick()
{
	CRect rc;
	GetDlgItem(IDC_QUICK).GetWindowRect(&rc);
	CPoint pt(rc.left, rc.bottom);

	int openMode;
	CIDListPtr idl = TrackFavoriteMenu(m_hWnd, true, false, pt, openMode);
	if (!idl.isNull()) {
		CString str;
		if (idl_GetPathFromIDList(idl, str)) {
			if (!PathIsUNC(str) || gnNetSearchResult > 0)
				m_tree.SelectByIDList(idl);
			m_target.SetWindowText(str);
		}
		else
			m_tree.SelectByIDList(idl);
		m_target.SetFocus();
	}
}

void CQueryCopyDlg::OnHistoryChanged()
{
	int sel = m_target.GetCurSel();
	if (sel >= 0) {
		CString str;
		if (m_target.GetLBText(sel, str) > 0 && (!PathIsUNC(str) || (gnNetSearchResult > 0)))
			m_tree.SelectByPath(str);
	}
}

void CQueryCopyDlg::OnNotifyTreeChanged(NMHDR*, LRESULT*)
{
	DWORD attr;
	LPITEMIDLIST pidl = m_tree.GetSelItem(&attr);
	if (pidl) {
		CString str;
		if (!idl_GetPathFromIDList(pidl, str))
			str.Empty();
		m_target.SetWindowText(str);
	}
}

void CQueryCopyDlg::OnNotifyTreeQueryHidden(NMHDR*, LRESULT* pResult)
{
	*pResult = FTH_DEFAULT;
	if (gnFileStat & FTS_SUPERHIDDEN) *pResult = FTH_SYSHIDDEN;
	if (gnFileStat & FTS_HIDDEN) *pResult = FTH_HIDDEN;
}

void CQueryCopyDlg::OnCheckJump()
{
	m_qci.bTabOpen = IsDlgButtonChecked(IDC_JUMP);
}

void CQueryCopyDlg::OnCheckAsAdmin()
{
	m_qci.bAsAdmin = IsDlgButtonChecked(IDC_ASADMIN);
}

INT_PTR QueryCopy2(HWND hWnd, HDROP hdrop, LPCITEMIDLIST pidlBase, LPTSTR pszTo, BOOL bCopy, BOOL& bTabOpen, BOOL& bThreaded, BOOL& bAsAdmin)
{
	CQueryCopyDlg dlg;
	dlg.m_qci.m_pidlBase = pidlBase;
	dlg.m_qci.m_hDrop = hdrop;
	dlg.m_qci.bCopy = bCopy;
	dlg.m_qci.bThreaded = (gnFopStat & FOS_RUNCOPY) ? TRUE : FALSE;
	dlg.m_qci.bTabOpen = FALSE;
	dlg.m_qci.bAsAdmin = bAsAdmin;
	INT_PTR res = dlg.DoModal(hWnd);
	if (res == IDOK) {
		bThreaded = dlg.m_qci.bThreaded;
		bTabOpen = bThreaded ? FALSE : dlg.m_qci.bTabOpen;
		bAsAdmin = dlg.m_qci.bAsAdmin;
		lstrcpy(pszTo, dlg.m_qci.m_strTo);
	}
	return res;
}

INT_PTR QueryCopy(HWND hWnd, IDataObject* pdo, LPCITEMIDLIST pidlBase, LPTSTR pszTo, BOOL bCopy, BOOL& bTabOpen, BOOL& bThreaded, BOOL& bAsAdmin)
{
	CQueryCopyDlg dlg;
	dlg.m_qci.m_pidlBase = pidlBase;
	dlg.m_qci.m_pdo = pdo;
	dlg.m_qci.bCopy = bCopy;
	dlg.m_qci.bThreaded = (gnFopStat & FOS_RUNCOPY) ? TRUE : FALSE;
	dlg.m_qci.bTabOpen = FALSE;
	dlg.m_qci.bAsAdmin = bAsAdmin;
	INT_PTR res = dlg.DoModal(hWnd);
	if (res == IDOK) {
		bThreaded = dlg.m_qci.bThreaded;
		bTabOpen = bThreaded ? FALSE : dlg.m_qci.bTabOpen;
		bAsAdmin = dlg.m_qci.bAsAdmin;
		lstrcpy(pszTo, dlg.m_qci.m_strTo);
	}
	return res;
}

////////////////////////////////////////////////////////////////////////
//

INT_PTR QueryMakeDir(HWND hWndParent, LPCTSTR pszName)
{
	// vista以降はTaskDialogを利用
	const TASKDIALOG_BUTTON taskButtons[] = {
		{ QMD_RENAME,	MAKEINTRESOURCE(IDS_TREATDIR_COPY) },
		{ QMD_MAKEDIR,	MAKEINTRESOURCE(IDS_TREATDIR_MKDIR) }
	};

	TASKDIALOGCONFIG tdc;
	memset(&tdc, 0, sizeof(tdc));
	tdc.cbSize = sizeof(tdc);
	tdc.hwndParent = hWndParent;
	tdc.hInstance = RESINST;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS;
	tdc.dwCommonButtons = TDCBF_CANCEL_BUTTON;
	tdc.pszWindowTitle = MAKEINTRESOURCE(IDS_TREATDIR_TITLE);
	tdc.pszMainIcon = MAKEINTRESOURCE(IDI_MBX_QUESTION);
	tdc.pszMainInstruction = MAKEINTRESOURCE(IDS_TREATDIR_LABEL);
	tdc.pszContent = pszName;
	tdc.cButtons = ARRAYSIZE(taskButtons);
	tdc.pButtons = taskButtons;
	tdc.nDefaultButton = QMD_RENAME;
	tdc.cxWidth = 240;		// DLU
	int nButton = 0;
	if (TaskDialogIndirect(&tdc, &nButton, NULL, NULL) == S_OK)
		return nButton;
	return IDCANCEL;
}


////////////////////////////////////////////////////////////////////////
//

class CQueryDropDlg : public CDialogImpl<CQueryDropDlg>
{
public:
	DWORD m_effect;
	CString m_caption;
	CString m_title;
	BOOL m_threaded;
	BOOL m_asadmin;
	BOOL m_forceoverwrite;

public:
	CQueryDropDlg()
	{
		m_effect = DROPEFFECT_NONE;
		m_caption.Empty();
		m_title.Empty();
		m_threaded = FALSE;
		m_asadmin = FALSE;
	}

	enum { IDD = IDD_QUERYDROP };

BEGIN_MSG_MAP(CQueryDropDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK);
	ON_BN_CLICKED(IDC_THREADED, OnOKThread);
	ON_BN_CLICKED(IDCANCEL, OnCancel);
	ON_BN_CLICKED(IDC_ASADMIN, OnCheckAsAdmin);
	ON_WM_HELPINFO()
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		SetWindowText(m_caption);
		SetDlgItemText(IDC_TITLE, m_title);

		HICON hic = LoadIcon(RESINST, (m_effect == DROPEFFECT_COPY) ? _MKIR(IDI_COPY) : _MKIR(IDI_MOVE));
		SendDlgItemMessage(IDC_APPICON, STM_SETICON, (WPARAM)hic, 0);

		// デフォルトボタン
		if (m_threaded) {
			GetDlgItem(IDOK).ModifyStyle(BS_DEFPUSHBUTTON, 0);
			GetDlgItem(IDC_THREADED).ModifyStyle(0, BS_DEFPUSHBUTTON);
			GetDlgItem(IDC_THREADED).SetFocus();
		} else {
			GetDlgItem(IDC_THREADED).ModifyStyle(BS_DEFPUSHBUTTON, 0);
			GetDlgItem(IDOK).ModifyStyle(0, BS_DEFPUSHBUTTON);
			GetDlgItem(IDOK).SetFocus();
		}

		// 管理者権限で
		CheckDlgButton(IDC_ASADMIN, m_asadmin);
		// 強制上書きモード
		if (m_forceoverwrite) {
			RectCtrl_SetTextColor(GetDlgItem(IDC_TEXT), RGB(192,0,0));
			CString str(MAKEINTRESOURCE(IDS_FORCEOVERWRITE));
			SetDlgItemText(IDC_TEXT, str);
		}
		return FALSE;		// フォーカスをセットしたのでFALSEで返る
	}

	void OnOK()
	{
		m_threaded = FALSE;
		EndDialog(IDOK);
	}

	void OnOKThread()
	{
		m_threaded = TRUE;
		EndDialog(IDOK);
	}

	void OnCancel() { EndDialog(IDCANCEL); }

	void OnCheckAsAdmin()
	{
		m_asadmin = IsDlgButtonChecked(IDC_ASADMIN);
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

BOOL QueryDrop(HWND hwnd, LPCTSTR pszDropTarget, DROPEFFECT eff, BOOL& bThreaded, BOOL& bAsAdmin, BOOL bForceOverwrite)
{
	bThreaded = ((gnFopStat & FOS_RUNCOPY) != 0);

	// 無条件にYESの場合
	if (gnFopStat & FOS_DDNOCONFIRM) return TRUE;

	UINT capid, strid;
	if (eff == DROPEFFECT_COPY) {
		// コピーの場合
		strid = IDS_COPY_QUERYDIR;
		capid = IDS_TITLE_FILECOPY;
	} else if (eff == DROPEFFECT_MOVE) {
		// 移動の場合
		strid = IDS_MOVE_QUERYDIR;
		capid = IDS_TITLE_FILEMOVE;
	} else if (eff == DROPEFFECT_LINK) {
		// リンクの場合
		strid = IDS_LINK_QUERYDIR;
		capid = IDS_TITLE_LINK;
	} else {
		return FALSE;
	}
	CString str, cap;
	str.Format(strid, pszDropTarget);
	cap.LoadString(capid);

	if (GetForegroundWindow() != fvApp.wndMain)
		SetForegroundWindow(fvApp.wndMain);

	BOOL bOK = FALSE;
	if (eff == DROPEFFECT_LINK) {
		bOK = (MessageBox(hwnd, str, cap, MB_QINFO) == IDOK);
	} else {
		CQueryDropDlg dlg;
		dlg.m_threaded = bThreaded;
		dlg.m_asadmin = bAsAdmin;
		dlg.m_effect = eff;
		dlg.m_forceoverwrite = bForceOverwrite;
		dlg.m_caption = cap;
		dlg.m_title = str;
		if (dlg.DoModal(hwnd) == IDOK) {
			bThreaded = dlg.m_threaded;
			bAsAdmin = dlg.m_asadmin;
			bOK = TRUE;
		}
	}
	return bOK;
}


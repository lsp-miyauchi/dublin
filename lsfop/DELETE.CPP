#include "stdafx.h"
#include "fopstd.h"

#ifndef MAXHISTORY
#define MAXHISTORY		8
#endif

//////////////////////////////////////////////////////////////////////
// 削除したフォルダをストックし、重複する子フォルダを排除して
// SHChangeNotify を発行する

static bool isParent(LPCTSTR pszParent, LPCTSTR pszTarget)
{
	TCHAR s[MAX_PATH * 2], t[MAX_PATH * 2];
	lstrcpy(s, pszParent);
	PathAddBackslash(s);
	lstrcpy(t, pszTarget);
	PathAddBackslash(t);
	return (StrCmpNI(s, t, lstrlen(s)) == 0);
}

class CNotifyPathArray : public CSimpleMap<CString, LONG>
{
private:
	UINT m_uDefaultFlags;

public:
	CNotifyPathArray(UINT flags)
		: m_uDefaultFlags(flags)
	{
	}

	~CNotifyPathArray()
	{
		ChangeNotify();
	}

	void ChangeNotify()
	{
		int i, cnt = GetSize();
		for (i = 0; i < cnt; i++) {
			LPCTSTR psz = GetKeyAt(i);
			LONG event = GetValueAt(i);
			SHChangeNotify(event, m_uDefaultFlags | SHCNF_PATH, psz, 0);
		}
		RemoveAll();
	}

	BOOL AddNew(LPCTSTR pszPathNew, LONG event)
	{
		if (!pszPathNew || (*pszPathNew == 0)) {
			return FALSE;
		}

		int i, cnt = GetSize();
		for (i = cnt - 1; i >= 0; i--) {
			LPCTSTR psz = GetKeyAt(i);
			if (isParent(pszPathNew, psz)) {
				RemoveAt(i);
			}
		}
		Add(pszPathNew, event);
		return TRUE;
	}

};

static int lfdTreeRemove(LPCTSTR pszPath, LPDELSTAT pds, CNotifyPathArray* parrFolder);
static int RemoveDirectoryCore(LPCTSTR pszPath, DELSTAT* pds, CNotifyPathArray* parrNotifyFolder);
static int RemoveFileCore(LPCTSTR pszName, LPDELSTAT pds, CNotifyPathArray* parrNotifyFolder);
static BOOL RemoveTreeNoConfirmCore(LPCTSTR pszPath, CNotifyPathArray* parrNotifyFolder);
static BOOL RemoveFileNoConfirmCore(LPCTSTR pszName, CNotifyPathArray* parrNotifyFolder);



//////////////////////////////////////////////////////////////////////
//

class CDeleteProgressDlg : public CDialogImpl<CDeleteProgressDlg>
{
public:
	CDeleteProgressDlg() { m_pbCanceled = NULL; }
	~CDeleteProgressDlg() { }

	enum { IDD = IDD_DELPROG };

BEGIN_MSG_MAP(CDeleteProgressDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnCancel();

	void ZapMessage();
	void SetFile(LPCTSTR lpFile);

	BOOL* m_pbCanceled;
};


/////////////////////////////////////////////////////////////////////////////
// DelProgDlgFn dialog

BOOL CDeleteProgressDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pbCanceled = (BOOL*)lParam;
	CAnimateCtrl anim(GetDlgItem(IDC_AVI));
//@	anim.Open(GetModuleHandle(gszShell32), _MKIR(164));
	anim.Open(IDA_SHELL_164);
	return TRUE;
}

void CDeleteProgressDlg::OnDestroy()
{
	CAnimateCtrl anim(GetDlgItem(IDC_AVI));
	anim.Stop();
}

void CDeleteProgressDlg::OnCancel()
{
	if (m_pbCanceled) *m_pbCanceled = TRUE;
	ShowWindow(SW_HIDE);
}

void CDeleteProgressDlg::ZapMessage()
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!IsDialogMessage(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void CDeleteProgressDlg::SetFile(LPCTSTR lpFile)
{
	if (IsWindow()) {
		TCHAR path[MAX_PATH * 2], name[MAX_PATH * 2];
		PathGetParent(lpFile, path);
		GetDlgItemText(IDC_PATH, name, ARRAYSIZE(name));
		if (_pathcompare(name, path))
			PathSetDlgItemPath(m_hWnd, IDC_PATH, path);

		LPCTSTR p = PathFindFileName(lpFile);
		if (!p)
			p = lpFile;
		PathSetDlgItemPath(m_hWnd, IDC_NAME, p);
		ZapMessage();
	}
}


//////////////////////////////////////////////////////////////////////
//

class CDeleteConfirmDlg : public CDialogImpl<CDeleteConfirmDlg>
{
public:
	CDeleteConfirmDlg() { }
	~CDeleteConfirmDlg() { }

	enum { IDD = IDD_DELATTN };

BEGIN_MSG_MAP(CDeleteProgressDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDYES, OnYes)
	ON_BN_CLICKED(IDNO, OnNo)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		DELSTAT* pds = (DELSTAT*)lParam;
		PathSetDlgItemPath(m_hWnd, IDC_NAME, pds->lpName);

		TCHAR buf[MAX_PATH * 2];
		FormatFileDateTime(&pds->ftLastWriteTime, buf, ARRAYSIZE(buf));
		SetDlgItemText(IDC_DATE, buf);
		CheckDlgButton(IDC_FSYSTEM, (pds->dwAttr & FA_SYSTEM) ? TRUE : FALSE);
		CheckDlgButton(IDC_FHIDDEN, (pds->dwAttr & FA_HIDDEN) ? TRUE : FALSE);
		CheckDlgButton(IDC_FRDONLY, (pds->dwAttr & FA_RDONLY) ? TRUE : FALSE);

		CString str;
		str.LoadString((pds->dwAttr & FA_SUBDIR) ? IDS_DELTYPE_FOLDER : IDS_DELTYPE_FILE);
		CString msg;
		msg.Format(IDS_DELETE_ATTEN, str.GetString());
		SetDlgItemText(IDC_TEXT, msg);

		if (pds->nEntry < 2 && !(pds->dwAttr & FA_SUBDIR)) {
			GetDlgItem(IDOK).EnableWindow(FALSE);
			GetDlgItem(IDCANCEL).EnableWindow(FALSE);
		}
		return TRUE;
	}
	void OnOK()		{ EndDialog(IDOK); }
	void OnCancel()	{ EndDialog(IDCANCEL); }
	void OnYes()	{ EndDialog(IDYES); }
	void OnNo()		{ EndDialog(IDNO); }
};

//////////////////////////////////////////////////////////////////////
//

class CDeleteAction
{
public:
	int m_action;
	CFileInfo m_fi;
public:
	CDeleteAction() : m_action(DELETE_OK) { }
	~CDeleteAction() {}
	inline int Action() { return m_action; }
	inline void SetAction(int action) { m_action = action; }
};

//////////////////////////////////////////////////////////////////////
//

class CDeleteConfirmAgainDlg : public CDialogImpl<CDeleteConfirmAgainDlg>
{
private:
	CDeleteAction* m_pda;
	CIcon m_ic;
public:
	CDeleteConfirmAgainDlg() : m_pda(NULL) { }
	~CDeleteConfirmAgainDlg() { }

	enum { IDD = IDD_QUERYDELAGAIN };

BEGIN_MSG_MAP(CDeleteConfirmAgainDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDABORT, OnAbort)
	ON_BN_CLICKED(IDYES, OnYesAll)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		m_pda = (CDeleteAction*)lParam;
		CString str;
		if (m_pda->m_fi.IsFolder())
			str.Format(IDS_DELETE_DIR, m_pda->m_fi.m_strFullPathName.GetString());
		else
			str.Format(IDS_FILE_DELSINGLE, PathFindFileName(m_pda->m_fi.m_strFullPathName));
		SetDlgItemText(IDC_TEXT, str);
		// アイコン
		SHFILEINFO sfi;
		HIMAGELIST him = (HIMAGELIST)SHGetFileInfo(m_pda->m_fi.m_strFullPathName, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
		m_ic.Attach(ImageList_ExtractIcon(NULL, him, sfi.iIcon));
		if (m_ic.IsNull())
			m_ic.LoadIconW(IDI_MBX_QUESTION);
		CStatic st = GetDlgItem(IDC_APPICON);
		st.SetIcon(m_ic);
		return TRUE;
	}
	inline void OnOK()		{ m_pda->SetAction(DELETE_OK);		EndDialog(IDOK); }
	inline void OnCancel()	{ m_pda->SetAction(DELETE_NO);		EndDialog(IDOK); }
	inline void OnAbort()	{ m_pda->SetAction(DELETE_ABORT);	EndDialog(IDOK); }
	inline void OnYesAll()	{ m_pda->SetAction(DELETE_ALL);		EndDialog(IDOK); }
};

//////////////////////////////////////////////////////////////////////
//

class CDeleteFiles : public CDropFiles
{
public:
	int m_nAction;
	int m_nDelDir;
	int m_nDelFile;
	HWND m_hWnd;
	int m_nTrigger;
	CString m_path;
public:
	CDeleteFiles()
		: m_nAction(DELETE_OK)
		, m_nDelDir(0)
		, m_nDelFile(0)
		, m_hWnd(NULL)
		, m_nTrigger(DELETE_OK)
	{}
	~CDeleteFiles() {}

	void DoDelete();
};

void CDeleteFiles::DoDelete()
{
	CDeleteAction da;

	TCHAR name[MAX_PATH * 2];

	DELSTAT ds;
	memset(&ds, 0, sizeof(ds));
	ds.nAction = DELETE_OK;
	ds.bConfirm = FALSE;
	ds.nEntry = m_nEntry + IsFolderInclude();

	CDeleteProgressDlg* pdp = new CDeleteProgressDlg();
	pdp->Create(m_hWnd, (LPARAM)&ds.bCanceled);
	ds.param = (LPARAM)pdp;

	CNotifyPathArray narr(SHCNF_FLUSHNOWAIT);

	bool bCont = true;
	for (int i = 0; bCont && (i < m_nEntry); i++) {
		DragQueryFile(i, name, ARRAYSIZE(name));
		GetFileInfo(name, da.m_fi);
		if (m_nTrigger == DELETE_OK) {
			CDeleteConfirmAgainDlg again;
			again.DoModal(m_hWnd, (LPARAM)&da);
			if      (da.Action() == DELETE_ABORT) break;
			else if (da.Action() == DELETE_NO)    continue;
			else                                  m_nTrigger = da.Action();
		}

		// ディレクトリチェック
		if (da.m_fi.IsFolder()) {
			switch (RemoveDirectoryCore(name, &ds, &narr)) {
			case CR_DONE:
				m_nDelDir++;
				break;
			case CR_ERROR:
			case CR_ABORT:
				bCont = false;
				break;
			case CR_IGNORED:
			default:
				break;
			}
		}
		else {
			switch (RemoveFileCore(name, &ds, &narr)) {
			case CR_DONE:
				m_nDelFile++;
				break;
			case CR_ERROR:
			case CR_ABORT:
				bCont = false;
				break;
			case CR_IGNORED:
			default:
				break;
			}
		}
	}
	pdp->DestroyWindow();
	delete pdp;
}


//////////////////////////////////////////////////////////////////////
//

class CDeleteDlg : public CDialogImpl<CDeleteDlg>
{
public:
	CDeleteDlg() { m_pdf = NULL; }
	~CDeleteDlg() { }

	enum { IDD = IDD_QUERYDELETE };

BEGIN_MSG_MAP(CDeleteDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_DELALL, OnDelAll)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnDelAll();

	CDeleteFiles* m_pdf;
};


BOOL CDeleteDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pdf = (CDeleteFiles*)lParam;
	CString str;
	m_pdf->EnumItemToStr(str);
	SetDlgItemText(IDC_EDIT, str);
	PathSetDlgItemPath(m_hWnd, IDC_PATH, m_pdf->m_path);

	if (m_pdf->m_nEntry < 2)
		GetDlgItem(IDC_DELALL).EnableWindow(FALSE);
	return TRUE;
}

void CDeleteDlg::OnOK()
{
	m_pdf->m_nTrigger = (m_pdf->m_nEntry > 1) ? DELETE_OK : DELETE_ALL;
	EndDialog(IDOK);
}

void CDeleteDlg::OnDelAll()
{
	m_pdf->m_nTrigger = DELETE_ALL;
	EndDialog(IDOK);
}


//////////////////////////////////////////////////////////////////////
//

class CFilterDeleteParams
{
public:
	CString m_strFilter;
	CSimpleArray<CString> m_arrTargets;
	int m_numFiles;
	int m_numDirs;
	DWORDLONG m_dlTotalDeleted;
	BOOL m_bProcessSubdir;
	BOOL m_bDeleteEmptyDir;
	BOOL m_bDeleteSysHidden;
	BOOL m_bHasFilter;
	BOOL m_bUnmatchFilter;
	BOOL m_bRegExp;

	enum {
		FDPS_SUBDIR = 1,
		FDPS_EMPTYDIR = 2,
		FDPS_SYSHIDDEN = 4,
		FDPS_HASFILTER = 8,
		FDPS_UNMATCHFILTER = 16,
		FDPS_REGEXP = 32,
	};

	CFilterDeleteParams()
		: m_numFiles(0)
		, m_numDirs(0)
		, m_dlTotalDeleted(0)
		, m_bProcessSubdir(FALSE)
		, m_bDeleteEmptyDir(FALSE)
		, m_bDeleteSysHidden(FALSE)
		, m_bHasFilter(FALSE)
		, m_bUnmatchFilter(FALSE)
		, m_bRegExp(FALSE)
	{ }

	void toLabel(CString& str)
	{
		str = _T("");
		if (m_bHasFilter) {
			if (m_bUnmatchFilter)
				str += _T("NOT ");
			if (m_bRegExp) {
				CString tmp(MAKEINTRESOURCE(IDS_REGEXP));
				str += tmp;
			}
			str.AppendFormat(_T("( %s )"), m_strFilter.GetString());
		}
	}
	void toString(CString& str)
	{
		UINT stat = 0;
		if (m_bProcessSubdir)	stat |= FDPS_SUBDIR;
		if (m_bDeleteEmptyDir)	stat |= FDPS_EMPTYDIR;
		if (m_bDeleteSysHidden)	stat |= FDPS_SYSHIDDEN;
		if (m_bHasFilter)		stat |= FDPS_HASFILTER;
		if (m_bUnmatchFilter)	stat |= FDPS_UNMATCHFILTER;
		if (m_bRegExp)			stat |= FDPS_REGEXP;
		str.Format(_T("%d,%s"), stat, m_strFilter.GetString());
	}
	void fromString(LPCTSTR pstr)
	{
		UINT stat = 0;
		CString str = pstr;
		int pos = str.Find(_T(','));
		if (pos > 0) {
			CString tmp = str.Left(pos);
			stat = StrToInt(tmp);
			m_strFilter = str.Mid(pos + 1);
		}
		else {
			stat = StrToInt(str);
			m_strFilter.Empty();
		}
		m_bProcessSubdir	= ((stat & FDPS_SUBDIR) != 0);
		m_bDeleteEmptyDir	= ((stat & FDPS_EMPTYDIR) != 0);
		m_bDeleteSysHidden	= ((stat & FDPS_SYSHIDDEN) != 0);
		m_bHasFilter		= ((stat & FDPS_HASFILTER) != 0);
		m_bUnmatchFilter	= ((stat & FDPS_UNMATCHFILTER) != 0);
		m_bRegExp			= ((stat & FDPS_REGEXP) != 0);
	}
};

static int FilterRemoveDirectory(CFilterDeleteParams* pfd);
static int FilterCheckRemoveDirectory(CFilterDeleteParams* pfd);

////////////////////////////////////////////////////////////////////////
//

class CFilterDeleteDlg : public CDialogImpl<CFilterDeleteDlg>
{
public:
	enum { IDD = IDD_FILTERDEL };

	CComboBox m_combo;
	CWindow m_history;
	CFilterDeleteParams* m_pfd;
	CSimpleArray<CString> m_historyArray;

	CFilterDeleteDlg() { m_pfd = NULL; }
	~CFilterDeleteDlg() { }

BEGIN_MSG_MAP(CFilterDeleteDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_CHECK, OnFilterCheck)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_CBN_SELCHANGE(IDC_FILTERING_TYPE, OnCheckFilter)
	ON_NOTIFY(SEN_GETTEXT, IDC_SRC, OnNotifyBrowse)
	ON_BN_CLICKED(IDC_HISTORY, OnHistory)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);

	void OnFilterCheck();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void SetParams(CFilterDeleteParams* pfd);
	void OnCheckFilter();
	void OnHistory();
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	BOOL CheckParams();
};

BOOL CFilterDeleteDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pfd = (CFilterDeleteParams*)lParam;
	if (!m_pfd) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	m_combo = GetDlgItem(IDC_FILTERING_TYPE);
	InitDlgCtrl(m_combo, MAKEINTRESOURCE(IDS_FILTER_TYPES));

	// 履歴
	m_history = GetDlgItem(IDC_HISTORY);
	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PREVSETTING), 16, RGB(255, 0, 255));
	FlatButton_SetImageList(m_history, him);
	CString str(MAKEINTRESOURCE(IDS_HISTORY));
	FlatButton_SetTipText(m_history, str);
	RegLoadHistory(m_historyArray, MAKEINTRESOURCE(IDS_SEC_HISTORY), MAKEINTRESOURCE(IDS_KEY_FILTERDEL), MAXHISTORY);
	if (m_historyArray.GetSize() < 1)
		m_history.EnableWindow(FALSE);

	str = PathFindFileName(m_pfd->m_arrTargets[0]);
	int cnt = m_pfd->m_arrTargets.GetSize();
	if (cnt > 1) {
		CString tmp1 = PathFindFileName(m_pfd->m_arrTargets[0]);
		CString tmp2 = PathFindFileName(m_pfd->m_arrTargets[1]);
		if (cnt > 2)
			str.Format(IDS_NUMOF_FILES, tmp1.GetString(), tmp2.GetString(), cnt);
		else
			str.Format(_T("%s, %s"), tmp1.GetString(), tmp2.GetString());
	}
	SetDlgItemText(IDC_SRC, str);
	SetParams(m_pfd);

	// オートコンプリート
	SHAutoComplete(GetDlgItem(IDC_SRC), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);
	return TRUE;
}

void CFilterDeleteDlg::SetParams(CFilterDeleteParams* pfd)
{
	CheckDlgButton(IDC_SUBDIR, pfd->m_bProcessSubdir);
	CheckDlgButton(IDC_FILTERDEL_EMPTYDIR, pfd->m_bDeleteEmptyDir);
	CheckDlgButton(IDC_FILTERDEL_SYSHIDDEN, pfd->m_bDeleteSysHidden);
	CheckDlgButton(IDC_USEREGEXP, pfd->m_bRegExp);

	int sel = 0;
	if (pfd->m_bHasFilter) sel = pfd->m_bUnmatchFilter ? 2 : 1;
	m_combo.SetCurSel(sel);

	SetDlgItemText(IDC_FILTER, pfd->m_strFilter);
	OnCheckFilter();
}

void CFilterDeleteDlg::OnHistory()
{
	int cnt = m_historyArray.GetSize();
	if (cnt < 1) return;

	CMenu pop;
	pop.CreatePopupMenu();

	CString str;
	CFilterDeleteParams fd;
	for (int i = 0; i < cnt; i++) {
		fd.fromString(m_historyArray[i]);
		fd.toLabel(str);
		pop.AppendMenu(MF_STRING, i + 1, str);
	}

	CRect rc;
	m_history.GetWindowRect(&rc);
	int cmd = pop.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, rc.left, rc.bottom, m_hWnd);
	if (cmd > 0) {
		m_pfd->fromString(m_historyArray[cmd - 1]);
		SetParams(m_pfd);
	}
}

void CFilterDeleteDlg::OnCheckFilter()
{
	int sel = m_combo.GetCurSel();
	if (sel >= 0) {
		BOOL bEnable = (m_combo.GetCurSel() != 0);
		GetDlgItem(IDC_FILTER).EnableWindow(bEnable);
		GetDlgItem(IDC_USEREGEXP).EnableWindow(bEnable);
	}
}

BOOL CFilterDeleteDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW && pInfo->iCtrlId) {
		ShowCtrlHelp(GetDlgItem(pInfo->iCtrlId), pInfo->iCtrlId);
		return TRUE;
	}
	return FALSE;
}

BOOL CFilterDeleteDlg::CheckParams()
{
	m_pfd->m_bProcessSubdir = IsDlgButtonChecked(IDC_SUBDIR);
	m_pfd->m_bDeleteEmptyDir = IsDlgButtonChecked(IDC_FILTERDEL_EMPTYDIR);
	m_pfd->m_bDeleteSysHidden = IsDlgButtonChecked(IDC_FILTERDEL_SYSHIDDEN);

	// フィルタパタンの取得
	m_pfd->m_strFilter.Empty();
	m_pfd->m_bRegExp = FALSE;

	CString str;
	int sel = m_combo.GetCurSel();
	m_pfd->m_bHasFilter = (sel == 0) ? FALSE : TRUE;
	if (m_pfd->m_bHasFilter) {
		if (GetDlgItemText(IDC_FILTER, str) == 0) {
			MessageBeep(0);
			GetDlgItem(IDC_FILTER).SetFocus();
			return FALSE;
		}
		m_pfd->m_strFilter = str;
		m_pfd->m_bUnmatchFilter = (sel == 2);
		m_pfd->m_bRegExp = IsDlgButtonChecked(IDC_USEREGEXP);
	}
	return TRUE;
}

void CFilterDeleteDlg::OnFilterCheck()
{
	if (CheckParams()) {
		FilterCheckRemoveDirectory(m_pfd);
		if (m_pfd->m_numFiles == 0)
			AppMessage(IDS_TITLE_DELETE, IDS_FILTERDEL_NOTARGET, NULL, MB_INFORMATION);
		else {
			TCHAR ww[64];
			CString txt;
			txt.Format(IDS_FILTERDEL_CHECK, m_pfd->m_numFiles, StrFormatByteSize64(m_pfd->m_dlTotalDeleted, ww, ARRAYSIZE(ww)));

			int res = IDNO;
			int pos = txt.Find(_T('\t'));
			CString str1 = txt.Left(pos);
			CString str2 = txt.Mid(pos + 1);
			TaskDialog(::GetActiveWindow(), RESINST,
						MAKEINTRESOURCE(IDS_TITLE_DELETE),
						str1.GetString(), str2.GetString(),
						TDCBF_YES_BUTTON | TDCBF_NO_BUTTON,
						MAKEINTRESOURCE(IDI_MBX_QUESTION), &res);

			if (res == IDYES) {
				OnOK();
			}
		}
	}
}

void CFilterDeleteDlg::OnOK()
{
	if (CheckParams()) {
		// 履歴保存
		CString str;
		m_pfd->toString(str);
		RegSaveHistory(MAKEINTRESOURCE(IDS_SEC_HISTORY), MAKEINTRESOURCE(IDS_KEY_FILTERDEL), MAXHISTORY, str);
		EndDialog(IDOK);
	}
}

void CFilterDeleteDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (buf[0] == 0 || !_pathisdirectory(buf))
		buf[0] = 0;
	if (fopBrowseForFolder(m_hWnd, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		m_pfd->m_arrTargets.RemoveAll();
		m_pfd->m_arrTargets.Add(buf);
		*pResult = TRUE;
	}
}

//////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI fopDeleteFile(HWND hParent, HDROP hDrop)
{
	CDeleteFiles* pdf = new CDeleteFiles();
	pdf->Attach(hDrop);
	pdf->m_hWnd = (IsWindow(hParent) ? hParent : GetFocus());

	TCHAR buf[MAX_PATH * 2];
	pdf->DragQueryFile(0, buf, ARRAYSIZE(buf));
	PathRemoveFileSpec(buf);
	pdf->m_path = buf;
	CDeleteDlg dlg;
	if (dlg.DoModal(hParent, (LPARAM)pdf) == IDOK)
		pdf->DoDelete();
	pdf->Detach();
	delete pdf;
}

static int RemoveDirectoryCore(LPCTSTR pszPath, DELSTAT* pds, CNotifyPathArray* parrNotifyFolder)
{
	//
	// この関数からリターンしたときのカレントディレクトリは
	// 有効なディレクトリを指しているとは限らない
	//

	if (PathIsRoot(pszPath)) {
		AppMessage(IDS_TITLE_DELETE, IDS_DELETE_ROOT);
		return CR_ERROR;
	}

	// Windowsディレクトリ・システムディレクトリは削除不可
	TCHAR sysdir[MAX_PATH * 2];
	GetSystemDirectory(sysdir, ARRAYSIZE(sysdir));
	if (!_pathcompare(sysdir, pszPath)) {
		AppMessage(IDS_TITLE_DELETE, IDS_DELETE_WINDIR);
		return CR_ERROR;
	}
	GetWindowsDirectory(sysdir, ARRAYSIZE(sysdir));
	if (!_pathcompare(sysdir, pszPath)) {
		AppMessage(IDS_TITLE_DELETE, IDS_DELETE_WINDIR);
		return CR_ERROR;
	}
	// 現在のディレクトリを保存してWindowsディレクトリのルートに移動する
	sysdir[3] = 0;
	CEnterFolder ef(sysdir);
	return lfdTreeRemove(pszPath, pds, parrNotifyFolder);
}

static int RemoveFileCore(LPCTSTR pszName, LPDELSTAT pds, CNotifyPathArray* parrNotifyFolder)
{
	CFileInfo fi;
	if (!GetFileInfo(pszName, fi))
		return CR_ERROR;

	CDeleteProgressDlg* pdp = (CDeleteProgressDlg*)pds->param;
	if (pdp) pdp->SetFile(pszName);
	if (pds->bCanceled)
		return CR_ABORT;

	if (pds->bConfirm) {
		if (AppMessage(IDS_TITLE_DELETE, IDS_FILE_DELSINGLE, fi.m_pszName, MB_QEXCM) == IDCANCEL)
			return CR_IGNORED;
	}

	if (fi.IsAttrib(FA_RDONLY | FA_HIDDEN | FA_SYSTEM)) {
		if (pds->nAction != DELETE_ALL) {
			pds->lpName = fi.m_pszName;
			pds->dwAttr = fi.m_dwAttr;
			pds->ftLastWriteTime = fi.m_ftLastWriteTime;

			MessageBeep(0);
			CDeleteConfirmDlg attn;
			INT_PTR r = attn.DoModal(GetFocus(), (LPARAM)pds);
			switch (r) {
				case IDOK:
					pds->nAction = DELETE_ALL;
					break;
				case IDYES:
					break;
				case IDNO:
					return CR_IGNORED;
				case IDCANCEL:
				default:
					return CR_ABORT;
			}
		}
		SetFileAttributes(pszName, FA_NORMAL);
	}

	while (!DeleteFile(pszName)) {
		switch (LastError(IDS_TITLE_DELETE, pszName, MB_QABORT)) {
			case IDABORT: return CR_ABORT;
			case IDIGNORE: return CR_IGNORED;
			default: break;
		}
		DoMessages();				// これを入れておかないとダメかな
	}

	if (parrNotifyFolder)
		parrNotifyFolder->AddNew(pszName, SHCNE_DELETE);
	else
		SHChangeNotify(SHCNE_DELETE, SHCNF_PATH | SHCNF_FLUSHNOWAIT, pszName, 0);
	return CR_DONE;
}

extern "C"
int WINAPI fopRemoveFile(LPCTSTR pszName, LPDELSTAT pds)
{
	CNotifyPathArray narr(SHCNF_FLUSHNOWAIT);
	return RemoveFileCore(pszName, pds, &narr);
}

static int lfdTreeRemove(LPCTSTR pszPath, LPDELSTAT pds, CNotifyPathArray* parrNotifyFolder)
{
	CDeleteProgressDlg* pdp = (CDeleteProgressDlg*)pds->param;

	CWin32FindData wfd;
	if (!GetFileData(pszPath, wfd)) {
		LastError(IDS_TITLE_DELETE, pszPath);
		return CR_ERROR;
	}

	if (wfd.IsAttrib(FA_RDONLY | FA_SYSTEM | FA_HIDDEN)) {
		if (pds->nAction != DELETE_ALL) {
			pds->lpName = pszPath;
			pds->dwAttr = wfd.dwFileAttributes;
			pds->ftLastWriteTime = wfd.ftLastWriteTime;

			MessageBeep(0);
			CDeleteConfirmDlg attn;
			INT_PTR r = attn.DoModal(GetFocus(), (LPARAM)pds);
			switch (r) {
			case IDYES:		break;
			case IDOK:		pds->nAction = DELETE_ALL; break;
			case IDNO:		return CR_IGNORED;
			default:		return CR_ABORT;
			}
		}
		SetFileAttributes(pszPath, FA_NORMAL);
	}

	// リパースフォルダまたはディレクトリが空であればここで削除が完了する
	if (RemoveDirectory(pszPath) && GetFileAttributes(pszPath) == 0xffffffff) {
		if (parrNotifyFolder)
			parrNotifyFolder->AddNew(pszPath, SHCNE_RMDIR);
		else
			SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, pszPath, 0);
		return CR_DONE;
	}

	// ファイルを検索
	CSimpleArray<CString> arrFile;
	CSimpleArray<CString> arrFolder;
	EnumFileDir(pszPath, gszAst, arrFile, arrFolder);

	TCHAR buf[MAX_PATH * 2];

	// まずファイルを先に削除
	int res;
	int i, cnt = (int)arrFile.GetSize();
	for (i = 0; i < cnt; i++) {
		PathCombine(buf, pszPath, arrFile[i]);
		if (pdp)
			pdp->SetFile(buf);
		// 中止？
		if (pds->bCanceled)
			return CR_ABORT;
		// 削除実行
		res = RemoveFileCore(buf, pds, parrNotifyFolder);
		if (res == CR_ABORT || res == CR_ERROR)
			return res;
	}
	arrFile.RemoveAll();

	// 次にフォルダを削除
	cnt = (int)arrFolder.GetSize();
	for (i = 0; i < cnt; i++) {
		PathCombine(buf, pszPath, arrFolder[i]);
		res = lfdTreeRemove(buf, pds, parrNotifyFolder);
		if (res == CR_ABORT || res == CR_ERROR)
			return res;
	}
	arrFolder.RemoveAll();

	// 自分自身を削除
	if (!_removedirectory(pszPath)) {
		LastError(IDS_TITLE_DELETE, pszPath);
		return CR_ERROR;
	}

	if (parrNotifyFolder)
		parrNotifyFolder->AddNew(pszPath, SHCNE_RMDIR);
	return CR_DONE;
}

extern "C"
int WINAPI fopTrashFile(HWND hwnd, LPTSTR lpFileNames, FILEOP_FLAGS fFlags)
{
	SHFILEOPSTRUCT sfos;
	memset(&sfos, 0, sizeof(SHFILEOPSTRUCT));
	sfos.hwnd = hwnd;
	sfos.wFunc = FO_DELETE;
	sfos.pFrom = lpFileNames;
	sfos.fFlags = fFlags;
	return SHFileOperation(&sfos);
}

int TrashOneFile(HWND hwnd, LPTSTR pszFileName, FILEOP_FLAGS fFlags)
{
	CString str;
	int len = lstrlen(pszFileName);
	LPTSTR p = str.GetBuffer(len + 2);
	lstrcpy(p, pszFileName);
	p[len + 1] = 0;
	return fopTrashFile(hwnd, p, fFlags);
}

static BOOL RemoveFileNoConfirmCore(LPCTSTR pszName, CNotifyPathArray* parrNotifyFolder)
{
	BOOL bOK = FALSE;
	if (SetFileAttributes(pszName, FA_NORMAL)) {
		bOK = DeleteFile(pszName);
		if (bOK) {
			if (parrNotifyFolder)
				parrNotifyFolder->AddNew(pszName, SHCNE_DELETE);
			else
				SHChangeNotify(SHCNE_DELETE, SHCNF_PATH | SHCNF_FLUSHNOWAIT, pszName, 0);
		}
	}
	return bOK;
}

static BOOL RemoveTreeNoConfirmCore(LPCTSTR pszPath, CNotifyPathArray* parrNotifyFolder)
{
	// リパースフォルダまたはディレクトリが空であればここで削除が完了する
	SetFileAttributes(pszPath, FA_NORMAL);
	if (RemoveDirectory(pszPath) && GetFileAttributes(pszPath) == 0xffffffff) {
		if (parrNotifyFolder)
			parrNotifyFolder->AddNew(pszPath, SHCNE_RMDIR);
		else
			SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, pszPath, 0);
		return TRUE;
	}

	int nError = 0;
	CSimpleArray<CString> arrFile;
	CSimpleArray<CString> arrFolder;
	EnumFileDir(pszPath, gszAst, arrFile, arrFolder);

	TCHAR buf[MAX_PATH * 2];

	// 最初にファイルを処理
	int i, cnt = (int)arrFile.GetSize();
	for (i = 0; i < cnt; i++) {
		PathCombine(buf, pszPath, arrFile[i]);
		if (!RemoveFileNoConfirmCore(buf, parrNotifyFolder))
			nError++;
	}
	arrFile.RemoveAll();

	// 次にフォルダを処理
	cnt = (int)arrFolder.GetSize();
	for (i = 0; i < cnt; i++) {
		PathCombine(buf, pszPath, arrFolder[i]);
		if (!RemoveTreeNoConfirmCore(buf, parrNotifyFolder))
			nError++;
	}
	arrFolder.RemoveAll();

	if (!_removedirectory(pszPath))
		nError++;
	else {
		if (parrNotifyFolder)
			parrNotifyFolder->AddNew(pszPath, SHCNE_RMDIR);
		else
			SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, pszPath, 0);
	}
	return (nError > 0) ? FALSE : TRUE;
}

extern "C"
BOOL WINAPI fopRemoveTreeNoConfirm(LPCTSTR pszPath)
{
	CNotifyPathArray narr(SHCNF_FLUSHNOWAIT);
	return RemoveTreeNoConfirmCore(pszPath, &narr);
}

extern "C"
BOOL WINAPI fopRemoveFileNoConfirm(LPCTSTR pszName)
{
	CNotifyPathArray narr(SHCNF_FLUSHNOWAIT);
	return RemoveFileNoConfirmCore(pszName, &narr);
}

static BOOL erase_file(LPCTSTR pszFile, CProgressDialog& progress, CNotifyPathArray* parrNotifyFolder)
{
	CFileInfo fi;
	if (!GetFileInfo(pszFile, fi))
		return FALSE;

	if (progress.HasUserCancelled())
		return FALSE;
	progress.SetLine(3, PathFindFileName(pszFile), TRUE);

	if (!SetFileAttributes(pszFile, FA_NORMAL)) {
		LastError(IDS_TITLE_DELETE, pszFile);
		return FALSE;
	}

	CFileHandle file;
	file.CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, 0, OPEN_EXISTING, 0);
	if (file.IsInvalid()) {
		LastError(IDS_TITLE_DELETE, pszFile);
		return FALSE;
	}

	// ファイルの内容を 'E5' で埋める
	BYTE cb[512];
	memset(cb, 512, 0xE5);
	DWORDLONG fp = 0;
	DWORD dw, dwSize;
	while (fp < fi.m_dlSize) {
		dwSize = (DWORD)((fp + 512 <= fi.m_dlSize) ? 512 : fi.m_dlSize - fp);
		if (!file.WriteFile(cb, dwSize, &dw) || dw != dwSize)
			break;
		fp += (DWORDLONG)dwSize;
	}
	file.FlushFileBuffers();
	file.SetFilePointer(0, NULL, FILE_BEGIN);
	file.SetEndOfFile();
	file.FlushFileBuffers();
	file.CloseHandle();

	BOOL bOK = DeleteFile(pszFile);
	if (bOK) {
		if (parrNotifyFolder)
			parrNotifyFolder->AddNew(pszFile, SHCNE_DELETE);
	}
	return bOK;
}

static BOOL erase_folder(LPCTSTR pszPath, CProgressDialog& progress, CNotifyPathArray* parrNotifyFolder)
{
	if (progress.HasUserCancelled())
		return FALSE;
	progress.SetLine(2, pszPath, TRUE);

	// リパースフォルダまたはディレクトリが空であればここで削除が完了する
	if (!SetFileAttributes(pszPath, FA_NORMAL)) {
		LastError(IDS_TITLE_DELETE, pszPath);
		return FALSE;
	}

	if (RemoveDirectory(pszPath) && GetFileAttributes(pszPath) == 0xffffffff) {
		if (parrNotifyFolder)
			parrNotifyFolder->AddNew(pszPath, SHCNE_RMDIR);
		return TRUE;
	}

	// ファイルを検索
	CSimpleArray<CString> arrFile;
	CSimpleArray<CString> arrFolder;
	EnumFileDir(pszPath, gszAst, arrFile, arrFolder);

	TCHAR buf[MAX_PATH * 2];

	// まずファイルを先に削除
	int i, cnt = (int)arrFile.GetSize();
	for (i = 0; i < cnt; i++) {
		PathCombine(buf, pszPath, arrFile[i]);
		if (!erase_file(buf, progress, parrNotifyFolder)) return FALSE;
	}
	arrFile.RemoveAll();

	// 次にフォルダを削除
	cnt = (int)arrFolder.GetSize();
	for (i = 0; i < cnt; i++) {
		PathCombine(buf, pszPath, arrFolder[i]);
		if (!erase_folder(buf, progress, parrNotifyFolder)) return FALSE;

		if (parrNotifyFolder)
			parrNotifyFolder->AddNew(buf, SHCNE_RMDIR);
	}
	arrFolder.RemoveAll();

	if (!_removedirectory(pszPath)) {
		LastError(IDS_TITLE_DELETE, pszPath);
		return FALSE;
	}

	if (parrNotifyFolder)
		parrNotifyFolder->AddNew(pszPath, SHCNE_RMDIR);
	return TRUE;
}

extern "C"
BOOL WINAPI fopFileShredder(HWND hwnd, HDROP hDrop, BOOL bNeedConfirm)
{
	int cnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if (cnt < 1)
		return FALSE;

	if (bNeedConfirm && AppMessage(IDR_FOPSERV, IDS_TITLE_SHREDDER, NULL, MB_QYESNO) != IDYES)
		return FALSE;

	// 進捗ダイアログ
	CProgressDialog progress;
	progress.SetTitle(IDS_TITLE_DELETE);
	progress.SetMessage(MAKEINTRESOURCE(IDS_ERASE_TITLE));
	progress.SetOperation(SPACTION_DELETING);
	progress.StartProgressDialog(hwnd, PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR);

	CNotifyPathArray narr(SHCNF_FLUSHNOWAIT);

	BOOL bOK = TRUE;
	TCHAR buf[MAX_PATH * 2];
	for (int i = 0; i < cnt; i++) {
		DragQueryFile(hDrop, i, buf, ARRAYSIZE(buf));
		if (!PathIsRoot(buf)) {
			if (_pathisdirectory(buf)) {
				bOK = erase_folder(buf, progress, &narr);
				if (!bOK) break;
			}
			else if (_pathfileexists(buf)) {
				bOK = erase_file(buf, progress, &narr);
				if (!bOK) break;
			}
		}
	}

	progress.StopProgressDialog();
	return bOK;
}

extern "C"
BOOL WINAPI fopEraseFile(HDROP hDrop)
{
	return fopFileShredder(GetFocus(), hDrop, FALSE);
}

extern "C"
void WINAPI fopFilterDeleteEx(HWND hParent, HDROP hdrop)
{
	CFilterDeleteParams fd;
	fd.m_bProcessSubdir = TRUE;
	fd.m_bHasFilter = FALSE;

	int cnt = hdrop ? DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0) : 0;
	for (int i = 0; i < cnt; i++) {
		TCHAR name[MAX_PATH * 2];
		DragQueryFile(hdrop, i, name, ARRAYSIZE(name));
		if (PathIsRoot(name) || _pathisdirectory(name)) {
			fd.m_arrTargets.Add(name);
		}
	}

	if (fd.m_arrTargets.GetSize() == 0)
		return;

	CFilterDeleteDlg dlg;
	if (dlg.DoModal(hParent, (LPARAM)&fd) == IDOK) {
		FilterRemoveDirectory(&fd);
		if (!fd.m_numFiles && !fd.m_numDirs)
			AppMessage(IDS_TITLE_DELETE, IDS_FILTERDEL_NOFILES, NULL, MB_ATTENTION);
		else {
			TCHAR ww[64];
			CString txt;
			txt.Format(IDS_FILTERDEL_RESULT, fd.m_numFiles, fd.m_numDirs, StrFormatByteSize64(fd.m_dlTotalDeleted, ww, ARRAYSIZE(ww)));
			AppMessage(MAKEINTRESOURCE(IDS_TITLE_DELETE), txt, NULL, MB_INFORMATION);
		}
	}
}

extern "C"
void WINAPI fopFilterDelete(HWND hParent, LPCTSTR lpPath)
{
	HDROP hdrop = fopDragInitBlock(CPoint(0, 0), FALSE);
	fopDragAppendFile(hdrop, lpPath);
	fopFilterDeleteEx(hParent, hdrop);
	DragFinish(hdrop);
}

static BOOL FilterTreeRemove(LPCTSTR pszPath, CFilterDeleteParams* pfd)
{
	// この関数は引数で指定されたフォルダの配下のみを削除する

	CFileArray array;
	CFileInfo* pf;

	if (pfd->m_bHasFilter)
		array.BrowseFiles(pszPath, pfd->m_strFilter, pfd->m_bUnmatchFilter, pfd->m_bRegExp);
	else
		array.BrowseFiles(pszPath, gszAst);

	int cnt = (int)array.GetCount();
	DELSTAT ds;
	memset(&ds, 0, sizeof(DELSTAT));
	ds.nAction = DELETE_ALL;					// 問い合わせ無しで削除実行
	ds.bConfirm = FALSE;
	ds.nEntry = cnt + array.IsFolderInclude();
	int i;
	for (i = 0; i < cnt; i++) {
		pf = array[i];
		if (!pfd->m_bDeleteSysHidden && pf->IsSysHidden())
			continue;

		if (fopRemoveFile(pf->m_strFullPathName, &ds) != CR_DONE)
			return CR_ERROR;
		pfd->m_numFiles++;
		pfd->m_dlTotalDeleted += pf->m_dlSize;
	}

	if (pfd->m_bProcessSubdir) {
		array.DeleteAll();
		array.EnumSubFolder(pszPath);
		for (i = 0; i < (int)array.GetCount(); i++) {
			pf = array[i];
			int nRet = FilterTreeRemove(pf->m_strFullPathName, pfd);
			if (nRet < 0)
				return nRet;

			if (pfd->m_bDeleteEmptyDir && !HasFiles(pf->m_strFullPathName)) {
				if (pf->IsAttrib(FA_RDONLY)) {
					// remove RDONLY attrib
					SetFileAttributes(pf->m_strFullPathName, (pf->m_dwAttr & ~FA_RDONLY));
				}

				if (!_removedirectory(pf->m_strFullPathName)) {
					LastError(IDS_TITLE_DELETE, pf->m_strFullPathName);
					return CR_ERROR;
				}
				SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, pf->m_strFullPathName, 0);
				pfd->m_numDirs++;
			}
		}
	}
	return CR_DONE;
}

static int FilterRemoveDirectory(CFilterDeleteParams* pfd)
{
	int numTargets = pfd->m_arrTargets.GetSize();
	// この関数からリターンしたときのカレントディレクトリは
	// 有効なディレクトリを指しているとは限らない

	// Windowsディレクトリ・システムディレクトリは削除不可
	CString windir, dir;
	GetSystemDirectory(dir.GetBuffer(MAX_PATH), MAX_PATH);		dir.ReleaseBuffer();
	GetWindowsDirectory(windir.GetBuffer(MAX_PATH), MAX_PATH);	windir.ReleaseBuffer();

	int i;
	for (i = 0; i < numTargets; i++) {
		if (!_pathcompare(dir, pfd->m_arrTargets[i]) || !_pathcompare(windir, pfd->m_arrTargets[i])) {
			AppMessage(IDS_TITLE_DELETE, IDS_DELETE_WINDIR);
			return CR_ERROR;
		}
	}

	// パラメータリセット
	pfd->m_numFiles = 0;
	pfd->m_numDirs = 0;
	pfd->m_dlTotalDeleted = 0;
	// 現在のディレクトリを保存してWindowsディレクトリに移動する
	GetCurrentDirectory(MAX_PATH * 2, dir.GetBuffer(MAX_PATH * 2));	dir.ReleaseBuffer();
	SetCurrentDirectory(windir);

	CWaitCursor wc;
	int r = CR_ERROR;
	for (i = 0; i < numTargets; i++) {
		r = FilterTreeRemove(pfd->m_arrTargets[i], pfd);
		if (r == CR_ERROR)
			break;
	}

	SetCurrentDirectory(dir);
	return r;
}

static BOOL FilterCheckTreeRemove(LPCTSTR pszPath, CFilterDeleteParams* pfd)
{
	// この関数は引数で指定されたフォルダの配下のみを削除する

	CFileArray array;
	CFileInfo* pf;

	if (pfd->m_bHasFilter)
		array.BrowseFiles(pszPath, pfd->m_strFilter, pfd->m_bUnmatchFilter, pfd->m_bRegExp);
	else
		array.BrowseFiles(pszPath, gszAst);

	int cnt = (int)array.GetCount();
	int i;
	for (i = 0; i < cnt; i++) {
		pf = array[i];
		if (!pfd->m_bDeleteSysHidden && pf->IsSysHidden())
			continue;
		pfd->m_numFiles++;
		pfd->m_dlTotalDeleted += pf->m_dlSize;
	}

	if (pfd->m_bProcessSubdir) {
		array.DeleteAll();
		array.EnumSubFolder(pszPath);
		for (i = 0; i < (int)array.GetCount(); i++) {
			pf = array[i];
			int nRet = FilterCheckTreeRemove(pf->m_strFullPathName, pfd);
			if (nRet < 0)
				return nRet;
		}
	}
	return CR_DONE;
}

static int FilterCheckRemoveDirectory(CFilterDeleteParams* pfd)
{
	int numTargets = pfd->m_arrTargets.GetSize();
	// この関数からリターンしたときのカレントディレクトリは
	// 有効なディレクトリを指しているとは限らない

	// Windowsディレクトリ・システムディレクトリは削除不可
	CString windir, dir;
	GetSystemDirectory(dir.GetBuffer(MAX_PATH), MAX_PATH);		dir.ReleaseBuffer();
	GetWindowsDirectory(windir.GetBuffer(MAX_PATH), MAX_PATH);	windir.ReleaseBuffer();

	int i;
	for (i = 0; i < numTargets; i++) {
		if (!_pathcompare(dir, pfd->m_arrTargets[i]) || !_pathcompare(windir, pfd->m_arrTargets[i])) {
			AppMessage(IDS_TITLE_DELETE, IDS_DELETE_WINDIR);
			return CR_ERROR;
		}
	}

	// パラメータリセット
	pfd->m_numFiles = 0;
	pfd->m_numDirs = 0;
	pfd->m_dlTotalDeleted = 0;
	// 現在のディレクトリを保存してWindowsディレクトリに移動する
	GetCurrentDirectory(MAX_PATH * 2, dir.GetBuffer(MAX_PATH * 2));	dir.ReleaseBuffer();
	SetCurrentDirectory(windir);

	CWaitCursor wc;
	int r = CR_ERROR;
	for (i = 0; i < numTargets; i++) {
		r = FilterCheckTreeRemove(pfd->m_arrTargets[i], pfd);
		if (r == CR_ERROR)
			break;
	}

	SetCurrentDirectory(dir);
	return r;
}


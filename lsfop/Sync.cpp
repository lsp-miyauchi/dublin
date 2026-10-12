// sync.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"
#include "syncdata.h"
#include "taskjob.h"

HRESULT GetSyncDataFolderItem(IShellItem** ppsi);
BOOL SyncFolder(CSyncRelay* psr, CSyncData& sd);
static BOOL SyncTree(CSyncRelay* psr, LPCTSTR pszFrom, LPCTSTR pszTo, CSyncData& sd);
static int SyncFile(CSyncRelay* psr, LPCTSTR pszFrom, LPCTSTR pszTo, CSyncData& sd);

static TCHAR sszFSDW[] = _T("*.fsd");
static TCHAR sszFSDPrev[] = _T("prev.fsd.$$$");

#define NUM_COLUMNS			3
#define BKRGB				(RGB(255,0,255))
#define MAXHISTORY			16

////////////////////////////////////////////////////////////////////////
//

typedef struct __HIMMAP {
	UINT idCtrl;
	UINT idRes;
	int width;
	UINT idTips;
} _HIMMAP;

////////////////////////////////////////////////////////////////////////
//

static RESLISTHEAD sstHeadColumn[NUM_COLUMNS] = {
	_MKIR(IDS_TITLE_NAME),	LVCFMT_LEFT,	200,
	_MKIR(IDS_SYNC_SRC),	LVCFMT_LEFT,	170,
	_MKIR(IDS_SYNC_DST),	LVCFMT_LEFT,	170,
};

static void MakePatPath(HWND hdlg, LPCTSTR pat, LPTSTR path)
{
	TCHAR buf[MAX_PATH * 2], patp[MAX_PATH * 2];

	fopGetSyncDataFolder(patp);
	lstrcpy(buf, pat);
	lstrcat(buf, gszFSD);
	PathCombine(path, patp, buf);
}

static LPCTSTR BuildPrevSettingFileName(CString& str)
{
	LPTSTR p = str.GetBuffer(MAX_PATH * 2);
	fopGetSyncDataFolder(p);
	PathAppend(p, sszFSDPrev);
	str.ReleaseBuffer();
	return str;
}

static void SetButtonImage(HWND hwnd, UINT id, int cx)
{
	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(id), 16, RGB(255, 0, 255));
	FlatButton_SetImageList(hwnd,  him);
}

static void format_datetime(SYSTEMTIME* pst, CString& str)
{
	str.Empty();
	LPTSTR p = str.GetBuffer(256);
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, pst, NULL, p, 256);
	int len = lstrlen(p);
	p[len++] = _T(' ');
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, pst, NULL, p + len, 256 - len);
	str.ReleaseBuffer();
}

static BOOL RenameTemp(LPCTSTR pszFile, CSyncData& sd)
{
	CString str;
	DWORD error = ERROR_ACCESS_DENIED;
	for (int i = 0; i < 1000; i++) {
		str.Format(_T("%s(%03d)"), pszFile, i);
		if (!_pathfileexists(str)) {
			if (!_movefile(pszFile, str)) {
				error = GetLastError();
				break;
			}

			CString tit;
			FormatFromToString(PathFindFileName(pszFile), PathFindFileName(str), tit);
			sd.Report(REPTYPE_RENAME, tit);
			return TRUE;
		}
	}

	sd.Report(REPTYPE_ERROR | REPTYPE_RENAME, pszFile, error);
	return FALSE;
}

static int GetSyncIconSysIndex()
{
	TCHAR buf[MAX_PATH];
	GetModuleFileName(MODINST, buf, ARRAYSIZE(buf));

	int (WINAPI* pfn)(LPCWSTR, int, UINT);
//	CDllHandle dll = GetModuleHandle(gszShell32);
//	(FARPROC&)pfn = dll.FindFunction(OsVISTA() ? "Shell_GetCachedImageIndexW" : "Shell_GetCachedImageIndex");
	#ifdef Shell_GetCachedImageIndex
	#undef Shell_GetCachedImageIndex
	#endif
	pfn = Shell_GetCachedImageIndex;
	return (*pfn)(buf, 0, FALSE);
}

static TCHAR sszFSS[] = _T(".fss");

static BOOL ScriptSyncNew_oldver(HWND hwnd, LPTSTR pszNewName, int cchMax)
{
	TCHAR fpath[MAX_PATH * 2], buf[MAX_PATH * 2];
	CString strFilter, strTitle;

	// ユニークなファイル名をビルド → "新しいフォルダ同期スクリプト(2).fss"
	fopGetSyncDataFolder(fpath);
	strFilter.LoadString(IDS_NEWSYNCSCRIPT);
	PathCombine(buf, fpath, strFilter);
	lstrcat(buf, sszFSS);
	_pathdecorate(buf, pszNewName, cchMax);
	PathStripPath(pszNewName);
	PathRemoveExtension(pszNewName);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrDefExt = sszFSS;
	ofn.lpstrFile = pszNewName;
	ofn.nMaxFile = cchMax;
	buf[0] = 0;
	ofn.lpstrFileTitle = buf;
	ofn.nMaxFileTitle = ARRAYSIZE(buf);
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_CREATEPROMPT;

	strTitle.LoadString(IDS_TITLE_SYNCSCRIPT);
	ofn.lpstrTitle = strTitle;
	strFilter.LoadString(IDS_FILTER_SYNCSCRIPT);
	LPTSTR p = (LPTSTR)(LPCTSTR)strFilter;
	while (p = StrChr(p, '|'))
		*p++ = 0;
	ofn.lpstrFilter = strFilter;
	ofn.lpstrInitialDir = fpath;

	return GetSaveFileName(&ofn);
}

static BOOL ScriptSyncNew(HWND hwnd, LPTSTR pszNewName, int cchMax)
{
	IFileSaveDialog* pfsd;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pfsd));
	if (FAILED(hr))
		return ScriptSyncNew_oldver(hwnd, pszNewName, cchMax);

	// タイトル
	CString str;
	str.LoadString(IDS_TITLE_SYNCSCRIPT);
	pfsd->SetTitle(str.GetString());
	// オプション
	pfsd->SetOptions(FOS_OVERWRITEPROMPT | FOS_CREATEPROMPT | FOS_DONTADDTORECENT | FOS_HIDEMRUPLACES);
	// デフォルト拡張子
	pfsd->SetDefaultExtension(sszFSS + 1);
	// 初期表示フォルダ＆デフォルトフォルダ
	IShellItem* psi = NULL;
	TCHAR path[MAX_PATH * 2];
	fopGetSyncDataFolder(path);
	if (SHCreateItemFromParsingName(path, NULL, IID_IShellItem, (void**)&psi) == S_OK) {
		pfsd->SetFolder(psi);
		pfsd->SetDefaultFolder(psi);
		psi->Release();
	}
	// ユニークなファイル名をビルド → "新しいフォルダ同期スクリプト(2).fss"
	str.LoadString(IDS_NEWSYNCSCRIPT);
	str += sszFSS;
	PathAppend(path, str);
	LPTSTR p = str.GetBuffer(MAX_PATH * 2);
	_pathdecorate(path, p, MAX_PATH * 2);
	PathStripPath(p);
	str.ReleaseBuffer();
	pfsd->SetFileName(str.GetString());
	// 拡張子リスト
	CSimpleArray<COMDLG_FILTERSPEC> arr;
	str.LoadString(IDS_FILTER_SYNCSCRIPT);
	LPTSTR q;
	p = (LPTSTR)str.GetString();
	COMDLG_FILTERSPEC cf;
	while (1) {
		q = StrChr(p, '|');
		if (!q) break;
		*q++ = 0;
		cf.pszName = p;
		p = StrChr(q, '|');
		if (!p) break;
		*p++ = 0;
		cf.pszSpec = q;
		arr.Add(cf);
	}
	pfsd->SetFileTypes(arr.GetSize(), arr.GetData());

	BOOL ok = FALSE;
	psi = NULL;
	if (pfsd->Show(hwnd) == S_OK && pfsd->GetResult(&psi) == S_OK) {
		LPWSTR pwz;
		if (psi->GetDisplayName(SIGDN_FILESYSPATH, &pwz) == S_OK) {
			lstrcpyn(pszNewName, pwz, cchMax);
			CoTaskMemFree(pwz);
			ok = TRUE;
		}
		psi->Release();
	}
	pfsd->Release();

	return ok;
}


/////////////////////////////////////////////////////////////////////
//

class CMSLocal : public CSimpleArray<CSyncData*>
{
private:
	BOOL m_bBackup;

public:
	CMSLocal() : m_bBackup(TRUE)
	{
	}

	~CMSLocal()
	{
		Clear();
	}

	void Clear()
	{
		int i, cnt = GetSize();
		for (i = 0; i < cnt; i++) {
			CSyncData* psd = (*this)[i];
			if (psd)
				delete psd;
		}
		RemoveAll();
	}

	inline BOOL IsBackup() { return m_bBackup; }
	inline void SetBackup(BOOL bBackup) { m_bBackup = bBackup; }
};

/////////////////////////////////////////////////////////////////////////////
// PatEditDlgFn

class CPatEditDlg : public CDialogImpl<CPatEditDlg>
{
public:
	CPatEditDlg() { m_psd = NULL; }
	~CPatEditDlg() { }

	enum { IDD = IDD_PATEDIT };

BEGIN_MSG_MAP(CPatEditDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_SYNC_NOMATCHDEL, OnCheckMirror)
	ON_NOTIFY_CODE(SEN_GETTEXT, OnNotifyBrowse)
	ON_CBN_SELCHANGE(IDC_FILTERING_TYPE, OnCheckExtension)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }
	void OnCheckMirror();
	void OnNotifyBrowse(int idCtrl, NMHDR* pNmhdr, LRESULT* pResult);
	void OnCheckExtension();

	BOOL DoCheckup(CSyncData* psd);
	void SetSyncData(CSyncData* psd);

	CSyncData* m_psd;
};

BOOL CPatEditDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_psd = (CSyncData*)lParam;
	SetDialogIcon(m_hWnd, IDI_DSYNC);

	CComboBox combo = GetDlgItem(IDC_FILTERING_TYPE);
	InitDlgCtrl(combo, MAKEINTRESOURCE(IDS_FILTER_TYPES));

	SetSyncData(m_psd);
	OnCheckMirror();
	OnCheckExtension();

	return TRUE;
}

BOOL CPatEditDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW && pInfo->iCtrlId) {
		ShowCtrlHelp(GetDlgItem(pInfo->iCtrlId), pInfo->iCtrlId);
		return TRUE;
	}
	return FALSE;
}

void CPatEditDlg::OnCheckExtension()
{
	CComboBox combo(GetDlgItem(IDC_FILTERING_TYPE));
	BOOL bEnable = (combo.GetCurSel() != 0);
	GetDlgItem(IDC_FILTER).EnableWindow(bEnable);
	GetDlgItem(IDC_USEREGEXP).EnableWindow(bEnable);
	GetDlgItem(IDC_CASESENSE).EnableWindow(bEnable);
}

void CPatEditDlg::OnCheckMirror()
{
	BOOL bEnable = IsDlgButtonChecked(IDC_SYNC_NOMATCHDEL) ? FALSE : TRUE;
	GetDlgItem(IDC_SYNC_SAVEOLD).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_ONLYEXIST).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_RECYCLE).EnableWindow(!bEnable);
}

void CPatEditDlg::OnNotifyBrowse(int idCtrl, NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (fopBrowseForFolder(m_hWnd, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CPatEditDlg::OnOK()
{
	if (DoCheckup(m_psd)) {
		TCHAR pat[MAX_PATH * 2];
		MakePatPath(m_hWnd, m_psd->m_strTitle, pat);
		m_psd->Save(pat);
		EndDialog(IDOK);
	}
}

BOOL CPatEditDlg::DoCheckup(CSyncData* psd)
{
	CComboBox combo(GetDlgItem(IDC_FILTERING_TYPE));

	CString str;
	if (GetDlgItemText(IDC_TITLE, str) == 0) {
		MessageBeep(0);
		GetDlgItem(IDC_TITLE).SetFocus();
		return FALSE;
	}
	psd->m_strTitle = str;
	psd->m_bMirror = (IsDlgButtonChecked(IDC_SYNC_NOMATCHDEL) != FALSE);
	psd->m_bSaveOld = psd->m_bMirror ?  false : (IsDlgButtonChecked(IDC_SYNC_SAVEOLD) != FALSE);
	psd->m_bSubdir = (IsDlgButtonChecked(IDC_SYNC_SUBDIR) != FALSE);
	int sel = combo.GetCurSel();
	psd->m_bHasFilter = (sel > 0) ? true : false;
	psd->m_bExclude = (sel == 2) ? true : false;
	psd->m_bUseRegExp = psd->m_bHasFilter ? (IsDlgButtonChecked(IDC_USEREGEXP) != FALSE) : false;
	psd->m_bCaseSense = psd->m_bHasFilter ? (IsDlgButtonChecked(IDC_CASESENSE) != FALSE) : false;

	psd->m_bReport = (IsDlgButtonChecked(IDC_SYNC_REPORT) != FALSE);
	psd->m_dwFlags = 0;
	if (!psd->m_bMirror) {
		if (IsDlgButtonChecked(IDC_SYNC_ONLYEXIST))
			psd->m_dwFlags |= SYNC_ONLYEXIST;
	}
	// ごみ箱を使って削除
	if (IsDlgButtonChecked(IDC_SYNC_RECYCLE))
		psd->m_dwFlags |= SYNC_RECYCLE;
	// ACL属性もコピー
	if (IsDlgButtonChecked(IDC_SYNC_COPYSID))
		psd->m_dwFlags |= SYNC_COPYSID;
	// ログファイル
	if (IsDlgButtonChecked(IDC_SYNC_SAVELOG))
		psd->m_dwFlags |= SYNC_SAVELOG;
	// フォルダ１
	if (GetDlgItemText(IDC_SYNC_SRC, str) == 0) {
		MessageBeep(0);
		GetDlgItem(IDC_SYNC_SRC).SetFocus();
		return FALSE;
	}
	psd->m_strSrcFolder = str;

	// フォルダ２
	if (GetDlgItemText(IDC_SYNC_DST, str) == 0) {
		MessageBeep(0);
		GetDlgItem(IDC_SYNC_DST).SetFocus();
		return FALSE;
	}
	psd->m_strDstFolder = str;

	// 同一ディレクトリかどうかのチェック
	if (!_pathcompare(psd->m_strSrcFolder, psd->m_strDstFolder)) {
		AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SAMEPATH);
		return FALSE;
	}

	// 拡張子パタンの取得
	psd->m_strFilter.Empty();
	if (psd->m_bHasFilter && GetDlgItemText(IDC_FILTER, str) > 0)
		psd->m_strFilter = str;

	// 除外フォルダ
	psd->m_strExcludeFolder.Empty();
	if (GetDlgItemText(IDC_SYNC_EXTFOLDER, str) > 0)
		psd->m_strExcludeFolder = str;

	return TRUE;
}

void CPatEditDlg::SetSyncData(CSyncData* psd)
{
	CComboBox combo(GetDlgItem(IDC_FILTERING_TYPE));

	CheckDlgButton(IDC_SYNC_NOMATCHDEL, psd->m_bMirror);
	CheckDlgButton(IDC_SYNC_ONLYEXIST, psd->isOnlyExist());
	CheckDlgButton(IDC_SYNC_SUBDIR, psd->m_bSubdir);
	CheckDlgButton(IDC_SYNC_SAVEOLD, psd->m_bSaveOld);
	CheckDlgButton(IDC_SYNC_REPORT, psd->m_bReport);
	CheckDlgButton(IDC_SYNC_ONLYEXIST, psd->isOnlyExist());
	CheckDlgButton(IDC_SYNC_COPYSID, psd->isCopyAccessRights());
	CheckDlgButton(IDC_SYNC_RECYCLE, psd->isRecycle());
	CheckDlgButton(IDC_SYNC_SAVELOG, m_psd->isLogSave());
	int sel = !psd->m_bHasFilter ? 0 : (psd->m_bExclude ? 2 : 1);
	combo.SetCurSel(sel);
	CheckDlgButton(IDC_USEREGEXP, psd->m_bUseRegExp);
	CheckDlgButton(IDC_CASESENSE, psd->m_bCaseSense);

	SetDlgItemText(IDC_TITLE, psd->m_strTitle);
	SetDlgItemText(IDC_SYNC_SRC, psd->m_strSrcFolder);
	SetDlgItemText(IDC_SYNC_DST, psd->m_strDstFolder);
	SetDlgItemText(IDC_FILTER, psd->m_strFilter);
	SetDlgItemText(IDC_SYNC_EXTFOLDER, psd->m_strExcludeFolder);

	OnCheckMirror();
	OnCheckExtension();
}

/////////////////////////////////////////////////////////////////////////////
// SyncDlgFn

class CSyncDlg : public CDialogImpl<CSyncDlg>
{
private:
	CString m_strPrevSetting;

public:
	CSyncData* m_psd;

public:
	CSyncDlg()
		: m_psd(NULL)
	{
	}

	~CSyncDlg()
	{
	}

	enum { IDD = IDD_SYNCDIR };

BEGIN_MSG_MAP(CSyncDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_SYNC_NOMATCHDEL, OnCheckMirror)
	ON_BN_CLICKED(IDC_SYNC_ONLYEXIST, OnOnlyExist)
	ON_BN_CLICKED(IDC_SYNC_PATSAVE, OnPatSave)
	ON_BN_CLICKED(IDC_SYNC_PATDEL, OnPatDel)
	ON_BN_CLICKED(IDC_SYNC_DOMULTI, OnMultiSync)
	ON_BN_CLICKED(IDC_SYNC_TASKJOB, OnMakeTaskJob)
	ON_BN_CLICKED(IDC_SYNC_PREVSETTING, OnPrevSetting)
	ON_BN_CLICKED(IDC_SYNC_LOGFOLDER, OnLogFolder)
	ON_COMMAND_ID(IDC_SYNC_BACKUP, OnConfirmClose);
	ON_COMMAND_ID(IDC_SYNC_RESTORE, OnConfirmClose);
	ON_COMMAND_ID(IDC_SYNC_MIRROR, OnConfirmClose);
	ON_CBN_SELCHANGE(IDC_SYNC_PATLIST, OnLoadPattern)
	ON_CBN_SELCHANGE(IDC_FILTERING_TYPE, OnCheckExtension)
	ON_NOTIFY_CODE(SEN_GETTEXT, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }
	void OnCheckMirror();
	void OnLogFolder();
	void OnOnlyExist();
	void OnNotifyBrowse(int idCtrl, NMHDR* pNmhdr, LRESULT* pResult);
	void OnPatSave();
	void OnPatDel();
	void OnPrevSetting();
	void OnMultiSync() { EndDialog(IDD_MULTISYNC); }
	void OnConfirmClose(UINT id);
	void OnLoadPattern();
	void OnCheckExtension();
	void OnMakeTaskJob();

	int FindPatItem(LPCTSTR pszTitle);
	BOOL AddPatItem(LPCTSTR pszPat);
	void PatternListup();
	void SetSyncData();
	BOOL DoCheckup();
	int GetSelectedPatName(LPTSTR pszPat);
};

static const _HIMMAP himmap[] = {
	{ IDC_SYNC_BACKUP,		IDB_ARDOWN,			16,	0 },
	{ IDC_SYNC_RESTORE,		IDB_ARUP,			16,	0 },
	{ IDC_SYNC_MIRROR,		IDB_ARUPDOWN,		16,	0 },
	{ IDC_SYNC_PATDEL,		IDB_DELETE,			16,	0 },
	{ IDC_SYNC_DOMULTI,		IDB_MSYNC,			20,	0 },
	{ IDC_SYNC_PATSAVE,		IDB_SAVE,			16,	0 },
	{ IDC_SYNC_TASKJOB,		IDB_TASK,			16,	0 },
	{ IDC_SYNC_PREVSETTING,	IDB_PREVSETTING,	16,	IDS_PREVSETTING },
};

BOOL CSyncDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	SetDialogIcon(m_hWnd, IDI_DSYNC);
	m_psd = (CSyncData*)lParam;

	// ボタンイメージ
	HINSTANCE hInst = RESINST;
	HIMAGELIST him;
	CString str;
	int i;
	for (i = 0; i < ARRAYSIZE(himmap); i++) {
		him = lccLoadImageList(hInst, RT_RCDATA, MAKEINTRESOURCE(himmap[i].idRes), himmap[i].width, RGB(255, 0, 255));
		FlatButton_SetImageList(GetDlgItem(himmap[i].idCtrl), him);
		if (himmap[i].idTips) {
			str.LoadString(himmap[i].idTips);
			FlatButton_SetTipText(GetDlgItem(himmap[i].idCtrl), str);
		}
	}
	FlatButton_SetElevationRequiredState(GetDlgItem(IDC_SYNC_TASKJOB), TRUE);

	CComboBox combo = GetDlgItem(IDC_FILTERING_TYPE);
	InitDlgCtrl(combo, MAKEINTRESOURCE(IDS_FILTER_TYPES));

	SetSyncData();
	OnCheckMirror();
	OnOnlyExist();
	OnCheckExtension();

	// 拡張コンボボックス
	CComboBoxEx comboex(GetDlgItem(IDC_SYNC_PATLIST));
	if (comboex.IsWindow()) {
		HIMAGELIST his, hil;
		Shell_GetImageLists(&hil, &his);
		comboex.SetImageList(his);
		PatternListup();
	}
	else {
		if (!m_psd->m_strTitle.IsEmpty())
			SetWindowText(m_psd->m_strTitle);
	}

	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);

	// 同期フォルダ履歴
	UINT flagFileInfo;
	SHFILEINFO sfi;
	CSimpleArray<CString> array;
	CLccSearchPathCtrl spsrc = GetDlgItem(IDC_SYNC_SRC);
	CLccSearchPathCtrl spdst = GetDlgItem(IDC_SYNC_DST);
	spsrc.SetImageList(himS);
	spdst.SetImageList(himS);

	RegLoadHistory(array, MAKEINTRESOURCE(IDS_SEC_HISTORY), MAKEINTRESOURCE(IDS_KEY_SYNCPATH), MAXHISTORY);
	int cnt = array.GetSize();
	for (i = 0; i < cnt; i++) {
		// ネットワーク先は省略アイコン
		int pos = array[i].Find(_T(';'));
		if (pos > 0) {
			CString src = array[i].Left(pos + 1);
			CString dst = array[i].Mid(pos + 1);
			if (!src.IsEmpty() && dst.IsEmpty()) {
				flagFileInfo = SHGFI_SYSICONINDEX | (PathIsUNC(src) ? SHGFI_USEFILEATTRIBUTES : 0);
				SHGetFileInfo(src, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), flagFileInfo);
				spsrc.AddString(src, sfi.iIcon);

				flagFileInfo = SHGFI_SYSICONINDEX | (PathIsUNC(dst) ? SHGFI_USEFILEATTRIBUTES : 0);
				SHGetFileInfo(dst, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi), flagFileInfo);
				spdst.AddString(dst, sfi.iIcon);
			}
		}
	}

	BuildPrevSettingFileName(m_strPrevSetting);
	if (!_pathisfile(m_strPrevSetting))
		GetDlgItem(IDC_SYNC_PREVSETTING).EnableWindow(FALSE);

	return TRUE;
}

BOOL CSyncDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW && pInfo->iCtrlId) {
		ShowCtrlHelp(GetDlgItem(pInfo->iCtrlId), pInfo->iCtrlId);
		return TRUE;
	}
	return FALSE;
}

void CSyncDlg::OnCheckExtension()
{
	CComboBox combo(GetDlgItem(IDC_FILTERING_TYPE));
	BOOL bEnable = (combo.GetCurSel() != 0);
	GetDlgItem(IDC_FILTER).EnableWindow(bEnable);
	GetDlgItem(IDC_USEREGEXP).EnableWindow(bEnable);
	GetDlgItem(IDC_CASESENSE).EnableWindow(bEnable);
}

void CSyncDlg::OnLoadPattern()
{
	TCHAR pat[MAX_PATH * 2];
	if (GetSelectedPatName(pat) != -1) {
		// comctl32のバージョンによっては、エディットコントロールに
		// 選択された文字列が反映されないから、自力で設定
		TCHAR buf[MAX_PATH * 2], tit[MAX_PATH * 2];
		lstrcpy(buf, PathFindFileName(pat));
		LPTSTR p = StrRChr(buf, NULL, '.');
		if (p)
			*p = '\0';

		CComboBoxEx comboex(GetDlgItem(IDC_SYNC_PATLIST));
		CComboBox combo = comboex.GetComboCtrl();
		if (combo.GetWindowText(tit, ARRAYSIZE(tit)) < 1 || StrCmpI(tit, buf))
			combo.SetWindowText(buf);

		if (m_psd->Load(pat))
			SetSyncData();
		else
			AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SYNC_PATFILEERROR, pat, MB_STOP);
	}
}

void CSyncDlg::OnCheckMirror()
{
	BOOL bEnable = IsDlgButtonChecked(IDC_SYNC_NOMATCHDEL) ? FALSE : TRUE;
	GetDlgItem(IDC_SYNC_SAVEOLD).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_ONLYEXIST).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_MIRROR).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_RECYCLE).EnableWindow(!bEnable);
}

void CSyncDlg::OnOnlyExist()
{
	BOOL bEnable = IsDlgButtonChecked(IDC_SYNC_ONLYEXIST) ? FALSE : TRUE;
	GetDlgItem(IDC_SYNC_MIRROR).EnableWindow(bEnable);
}

void CSyncDlg::OnLogFolder()
{
	TCHAR buf[MAX_PATH * 2];
	GetSyncLogFolder(buf, ARRAYSIZE(buf));
	if (fopBrowseForFolder(m_hWnd, buf)) {
		SetRegText(_MKIR(IDS_SEC_SYNCREPORT), _MKIR(IDS_KEY_LOGFOLDER), buf);
	}
}

void CSyncDlg::OnNotifyBrowse(int idCtrl, NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (fopBrowseForFolder(m_hWnd, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CSyncDlg::OnPatSave()
{
	if (DoCheckup()) {
		CComboBoxEx comboex(GetDlgItem(IDC_SYNC_PATLIST));
		CComboBox combo = comboex.GetComboCtrl();
		TCHAR buf[MAX_PATH * 2];
		if (combo.GetWindowText(buf, ARRAYSIZE(buf)) < 1) {
			AppMessage(IDS_TITLE_FOLDERSYNC, IDS_DSYNC_NOTITLE);
			comboex.SetFocus();
			return;
		}
		m_psd->m_strTitle = buf;

		TCHAR pat[MAX_PATH * 2];
		MakePatPath(m_hWnd, buf, pat);
		if (_pathfileexists(pat) &&
			AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SYNCPAT_FILEEXIST, buf, MB_QYESNO) != IDYES) {
			return;
		}

		if (m_psd->Save(pat)) {
			int index = FindPatItem(buf);
			if (index < 0)
				AddPatItem(pat);
		}
	}
}

void CSyncDlg::OnPrevSetting()
{
	if (_pathisfile(m_strPrevSetting)) {
		CSyncData sd;
		if (sd.Load(m_strPrevSetting)) {
			*m_psd = sd;
			SetSyncData();
		}
		else
			AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SYNC_PATFILEERROR, m_strPrevSetting, MB_STOP);
	}
}


void CSyncDlg::OnMakeTaskJob()
{
	TCHAR name[MAX_PATH * 2];
	if (GetSelectedPatName(name) < 0) {
		AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SYNCPAT_NOTSELECTED, NULL, MB_STOP);
		return;
	}

/* --
	int result = 0;
	TaskDialog(m_hWnd,
				RESINST,
				MAKEINTRESOURCE(IDS_TITLE_FOLDERSYNC),
				MAKEINTRESOURCE(IDS_TASKJOB_NEED_UAC),
				MAKEINTRESOURCE(IDS_UAC_CONTINUED),
				TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON,
				TD_INFORMATION_ICON, &result);
	if (result != IDOK)
		return;
-- */

	ILSFileOperation* pfo;
	HRESULT hr = CoCreateInstanceAsAdmin(m_hWnd, TRUE, __uuidof(LSFileOperation), __uuidof(ILSFileOperation), (void**)&pfo);
	if (hr == S_OK) {
		hr = pfo->MakeTaskJob(m_hWnd, name);
		pfo->Release();
	}

	SetForegroundWindow(m_hWnd);
}

void CSyncDlg::OnPatDel()
{
	TCHAR pat[MAX_PATH * 2];
	int sel = GetSelectedPatName(pat);
	if (sel != -1) {
		if (_pathfileexists(pat) &&
			AppMessage(IDS_CAPTION_DELSOL, IDS_QUERY_DELSOL, PathFindFileName(pat), MB_QYESNO) == IDYES) {
			fopRemoveFileNoConfirm(pat);
			PatternListup();
		}
	}
}

void CSyncDlg::OnConfirmClose(UINT id)
{
	switch (id) {
	case IDC_SYNC_BACKUP:	m_psd->m_nDirection = DIRSYNC_BACKUP; break;
	case IDC_SYNC_RESTORE:	m_psd->m_nDirection = DIRSYNC_RESTORE; break;
	case IDC_SYNC_MIRROR:	m_psd->m_nDirection = DIRSYNC_SYNC; break;
	default: return;
	}
	if (DoCheckup()) {
		CString str;
		str.Format(_T("%s;%s"), m_psd->m_strSrcFolder.GetString(), m_psd->m_strDstFolder.GetString());
		RegSaveHistory(MAKEINTRESOURCE(IDS_SEC_HISTORY), MAKEINTRESOURCE(IDS_KEY_SYNCPATH), MAXHISTORY, str);
		EndDialog(IDOK);
	}
}

int CSyncDlg::FindPatItem(LPCTSTR pszTitle)
{
	int index = -1;
	CComboBoxEx comboex(GetDlgItem(IDC_SYNC_PATLIST));
	COMBOBOXEXITEM ce;
	TCHAR buf[MAX_PATH];
	int cnt = comboex.GetCount();
	for (int i = 0; i < cnt; i++) {
		ce.mask = CBEIF_TEXT;
		ce.iItem = i;
		ce.pszText = buf;
		ce.cchTextMax = MAX_PATH;
		if (!comboex.GetItem(&ce))
			break;

		if (!StrCmpI(pszTitle, ce.pszText)) {
			index = i;
			break;
		}
	}
	return index;
}

BOOL CSyncDlg::AddPatItem(LPCTSTR pszPat)
{
	CSyncData sd;
	if (sd.Load(pszPat)) {
		TCHAR buf[MAX_PATH];
		COMBOBOXEXITEM cei;
		CComboBoxEx comboex(GetDlgItem(IDC_SYNC_PATLIST));
		cei.mask = CBEIF_DI_SETITEM | CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
		cei.iItem = comboex.GetCount();
		cei.iImage = cei.iSelectedImage = GetSyncIconSysIndex();
		lstrcpy(buf, pszPat);
		PathStripPath(buf);
		PathRemoveExtension(buf);
		cei.pszText = buf;
		cei.cchTextMax = lstrlen(buf);
		comboex.InsertItem(&cei);

		GetDlgItem(IDC_SYNC_TASKJOB).EnableWindow(TRUE);
		GetDlgItem(IDC_SYNC_PATDEL).EnableWindow(TRUE);

		return TRUE;
	}
	else
		AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SYNC_PATFILEERROR, pszPat, MB_STOP);
	return FALSE;
}

void CSyncDlg::PatternListup()
{
	CComboBoxEx comboex(GetDlgItem(IDC_SYNC_PATLIST));
	comboex.ResetContent();

	// 設定ファイルのリストアップ
	TCHAR path[MAX_PATH * 2];
	CFileInfo* pf;
	CFileArray array;

	fopGetSyncDataFolder(path);
	array.BrowseFiles(path, sszFSDW);
	array.Sort();

	int cnt = (int)array.GetCount();
	for (int i = 0; i < cnt; i++) {
		pf = array[i];
		AddPatItem(pf->m_strFullPathName);
	}

	BOOL bEnable = (cnt > 0);
	GetDlgItem(IDC_SYNC_DOMULTI).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_TASKJOB).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_PATDEL).EnableWindow(cnt > 0);
}

void CSyncDlg::SetSyncData()
{
	CComboBox combo(GetDlgItem(IDC_FILTERING_TYPE));
	CheckDlgButton(IDC_SYNC_NOMATCHDEL, m_psd->m_bMirror);
	CheckDlgButton(IDC_SYNC_SUBDIR, m_psd->m_bSubdir);
	CheckDlgButton(IDC_SYNC_SAVEOLD, m_psd->m_bSaveOld);
	CheckDlgButton(IDC_SYNC_REPORT, m_psd->m_bReport);
	CheckDlgButton(IDC_SYNC_COPYSID, m_psd->isCopyAccessRights());
	CheckDlgButton(IDC_SYNC_ONLYEXIST, m_psd->isOnlyExist());
	CheckDlgButton(IDC_SYNC_RECYCLE, m_psd->isRecycle());
	CheckDlgButton(IDC_SYNC_SAVELOG, m_psd->isLogSave());
	int sel = !m_psd->m_bHasFilter ? 0 : (m_psd->m_bExclude ? 2 : 1);
	combo.SetCurSel(sel);
	CheckDlgButton(IDC_USEREGEXP, m_psd->m_bUseRegExp);
	CheckDlgButton(IDC_CASESENSE, m_psd->m_bCaseSense);

	SetDlgItemText(IDC_SYNC_SRC, m_psd->m_strSrcFolder);
	SetDlgItemText(IDC_SYNC_DST, m_psd->m_strDstFolder);
	SetDlgItemText(IDC_FILTER, m_psd->m_strFilter);
	SetDlgItemText(IDC_SYNC_EXTFOLDER, m_psd->m_strExcludeFolder);

	OnCheckMirror();
	OnCheckExtension();
}

BOOL CSyncDlg::DoCheckup()
{
	CComboBox combo(GetDlgItem(IDC_FILTERING_TYPE));

	// ミラー
	m_psd->m_bMirror = (IsDlgButtonChecked(IDC_SYNC_NOMATCHDEL) != FALSE);
	// 古いファイルは名前を変えて残す
	m_psd->m_bSaveOld = m_psd->m_bMirror ? false : (IsDlgButtonChecked(IDC_SYNC_SAVEOLD) != FALSE);
	// サブフォルダも対象とする
	m_psd->m_bSubdir = (IsDlgButtonChecked(IDC_SYNC_SUBDIR) != FALSE);
	// 対象のファイル
	int sel = combo.GetCurSel();
	m_psd->m_bHasFilter = (sel > 0);
	m_psd->m_bExclude = (sel == 2) ? true : false;
	// 正規表現
	m_psd->m_bUseRegExp = m_psd->m_bHasFilter ? (IsDlgButtonChecked(IDC_USEREGEXP) != FALSE) : false;
	// 大文字小文字の区別
	m_psd->m_bCaseSense = m_psd->m_bHasFilter ? (IsDlgButtonChecked(IDC_CASESENSE) != FALSE) : false;

	// 結果レポートをおこなう
	m_psd->m_bReport = (IsDlgButtonChecked(IDC_SYNC_REPORT) != FALSE);

	m_psd->m_dwFlags = 0;
	if (!m_psd->m_bMirror) {
		// 存在するファイルのみ
		if (IsDlgButtonChecked(IDC_SYNC_ONLYEXIST))
			m_psd->m_dwFlags |= SYNC_ONLYEXIST;
	}
	// ACL属性もコピー
	if (IsDlgButtonChecked(IDC_SYNC_COPYSID))
		m_psd->m_dwFlags |= SYNC_COPYSID;
	// ゴミ箱に捨てる
	if (IsDlgButtonChecked(IDC_SYNC_RECYCLE))
		m_psd->m_dwFlags |= SYNC_RECYCLE;
	// ログファイル
	if (IsDlgButtonChecked(IDC_SYNC_SAVELOG))
		m_psd->m_dwFlags |= SYNC_SAVELOG;

	CString str;
	if (GetDlgItemText(IDC_SYNC_SRC, str) == 0) {
		MessageBeep(0);
		GetDlgItem(IDC_SYNC_SRC).SetFocus();
		return FALSE;
	}
	m_psd->m_strSrcFolder = str;

	if (GetDlgItemText(IDC_SYNC_DST, str) == 0) {
		MessageBeep(0);
		GetDlgItem(IDC_SYNC_DST).SetFocus();
		return FALSE;
	}
	m_psd->m_strDstFolder = str;

	// 同一ディレクトリかどうかのチェック
	if (!_pathcompare(m_psd->m_strSrcFolder, m_psd->m_strDstFolder)) {
		AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SAMEPATH);
		return FALSE;
	}

	// 拡張子パタンの取得
	m_psd->m_strFilter.Empty();
	if (m_psd->m_bHasFilter && GetDlgItemText(IDC_FILTER, str) > 0)
		m_psd->m_strFilter = str;

	// 除外フォルダ
	m_psd->m_strExcludeFolder.Empty();
	if (GetDlgItemText(IDC_SYNC_EXTFOLDER, str) > 0)
		m_psd->m_strExcludeFolder = str;

	return TRUE;
}

int CSyncDlg::GetSelectedPatName(LPTSTR pszPat)
{
	CComboBoxEx comboex(GetDlgItem(IDC_SYNC_PATLIST));
	int sel = comboex.GetCurSel();
	if (sel != CB_ERR) {
		TCHAR buf[MAX_PATH];
		COMBOBOXEXITEM cei;
		cei.mask = CBEIF_TEXT;
		cei.pszText = buf;
		cei.cchTextMax = ARRAYSIZE(buf);
		cei.iItem = sel;
		if (comboex.GetItem(&cei)) {
			MakePatPath(m_hWnd, buf, pszPat);
			return sel;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// MultiSyncDlgFn

class CMultiSyncDlg : public CDialogImpl<CMultiSyncDlg>, public CDialogResize<CMultiSyncDlg>
{
private:
	gdiplus_using m_gu;
	CListViewCtrl m_list;

public:
	CMultiSyncDlg() : m_pml(NULL) { }

	enum { IDD = IDD_MULTISYNC };


BEGIN_MSG_MAP(CMultiSyncDlg)
	CHAIN_MSG_MAP(CDialogResize<CMultiSyncDlg>)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNotifyClick)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, OnNotifyKeyDown)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST, OnNotifyDeleteItem)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_SELALL, OnSelAll)
	ON_BN_CLICKED(IDC_MAKESCRIPT, OnMakeScript)
	ON_COMMAND_ID(IDC_SYNC_BACKUP, OnOK);
	ON_COMMAND_ID(IDC_SYNC_RESTORE, OnOK);
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CMultiSyncDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_HELPCALL,		DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_SELALL,		DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_SYNC_BACKUP,	DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_SYNC_RESTORE,	DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_MAKESCRIPT,	DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnNotifyClick(LPNMHDR pnmh, LRESULT* pResult);
	void OnNotifyKeyDown(LPNMHDR pnmh, LRESULT* pResult);
	void OnNotifyDeleteItem(LPNMHDR pnmh, LRESULT* pResult);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK(UINT id);
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }
	void OnSelAll();
	void OnMakeScript();

	void PatternListup();
	int PickupSelectedItems(CSimpleArray<CSyncData*>& array);
	void EnableExecuteButtons();
	void ToggleCheck(int index);

	CMSLocal* m_pml;
};

BOOL CMultiSyncDlg::OnInitDialog(LPARAM lParam)
{
	if (!lParam) {
		EndDialog(IDCANCEL);
		return TRUE;
	}
	m_pml = (CMSLocal*)lParam;

	DlgResize_Init();

	m_list = GetDlgItem(IDC_LIST);
	HINSTANCE hInst = RESINST;
	HIMAGELIST his, hil;

	his = lccLoadImageList(hInst, RT_RCDATA, MAKEINTRESOURCE(IDB_STATE), 16, RGB(255, 0, 255));
	m_list.SetImageList(his, LVSIL_STATE);
	Shell_GetImageLists(&hil, &his);
	m_list.SetImageList(his, LVSIL_SMALL);

	// 列の挿入
	for (int i = 0; i < NUM_COLUMNS; i++) {
		CString str(sstHeadColumn[i].label);
		m_list.InsertColumn(i, str, sstHeadColumn[i].fmt, sstHeadColumn[i].cx, i);
	}
	PatternListup();

	SetButtonImage(GetDlgItem(IDC_SYNC_BACKUP), IDB_ARRIGHT, 16);
	SetButtonImage(GetDlgItem(IDC_SYNC_RESTORE), IDB_ARLEFT, 16);
	EnableExecuteButtons();
	m_list.SetFocus();

	// 位置復帰
	WINDOWPLACEMENT wpm;
	if (GetRegBinary(_MKIR(IDS_SEC_WNDPLACE), _MKIR(IDS_KEY_WP_MULTISYNC), (LPBYTE)&wpm, sizeof(wpm)) == sizeof(wpm)) {
		wpm.showCmd = SW_SHOWNA;
		SetWindowPlacement(&wpm);
	}
	CenterWindow();

	return FALSE;
}

void CMultiSyncDlg::OnDestroy()
{
	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wpm);
	SetRegBinary(_MKIR(IDS_SEC_WNDPLACE), _MKIR(IDS_KEY_WP_MULTISYNC), &wpm, sizeof(wpm));
}

void CMultiSyncDlg::OnNotifyClick(LPNMHDR pnmh, LRESULT* pResult)
{
	CPoint pt;
	GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	UINT nFlags;
	int index = m_list.HitTest(pt, &nFlags);
	if (index != -1) {
		if (nFlags & (LVHT_ONITEMICON | LVHT_ONITEMSTATEICON))
			ToggleCheck(index);
	}
}

void CMultiSyncDlg::OnNotifyKeyDown(LPNMHDR pnmh, LRESULT* pResult)
{
	LV_KEYDOWN* plvk = (LV_KEYDOWN*)pnmh;
	if (plvk->wVKey == VK_SPACE) {
		int index = m_list.GetSelectedIndex();
		if (index != -1) {
			ToggleCheck(index);
			int cnt = m_list.GetItemCount();
			if (++index < cnt)
				m_list.SelectItem(index);
		}
	}
}

void CMultiSyncDlg::OnNotifyDeleteItem(LPNMHDR pnmh, LRESULT* pResult)
{
	NMLISTVIEW* pnmlv = (NMLISTVIEW*)pnmh;
	if (pnmlv->lParam != NULL) {
		CSyncData* psd = (CSyncData*)pnmlv->lParam;
		delete psd;
	}
}

void CMultiSyncDlg::OnSelAll()
{
	int cnt = m_list.GetItemCount();
	for (int i = 0; i < cnt; i++)
		m_list.SetItemState(i, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);

	EnableExecuteButtons();
}

void CMultiSyncDlg::OnMakeScript()
{
	CSimpleArray<CSyncData*> array;
	if (PickupSelectedItems(array) < 1) {
		AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SELECT_MULTI_SYNCDATA);
		return;
	}

	CScriptSync script;
	int i, cnt = array.GetSize();
	for (i = 0; i < cnt; i++) {
		CSyncData* psd = array[i];
		CSyncInfo* info = new CSyncInfo;
		info->SetInfo(psd);
		script.AddItem(info);
		delete psd;
	}
	array.RemoveAll();

	TCHAR name[MAX_PATH * 2];
	if (ScriptSyncNew(m_hWnd, name, ARRAYSIZE(name))) {
		script.SaveAs(name);
	}
}

void CMultiSyncDlg::EnableExecuteButtons()
{
	int n = 0;
	int cnt = m_list.GetItemCount();
	for (int i = 0; i < cnt; i++) {
		DWORD state = m_list.GetItemState(i, LVIS_STATEIMAGEMASK);
		if (state == INDEXTOSTATEIMAGEMASK(2))
			n++;
	}

	GetDlgItem(IDC_SYNC_BACKUP).EnableWindow((n != 0));
	GetDlgItem(IDC_SYNC_RESTORE).EnableWindow((n != 0));
	GetDlgItem(IDC_MAKESCRIPT).EnableWindow((n > 1));
}

void CMultiSyncDlg::ToggleCheck(int index)
{
	DWORD state = m_list.GetItemState(index, LVIS_STATEIMAGEMASK);
	int n = (state == INDEXTOSTATEIMAGEMASK(2)) ? 1 : 2;
	m_list.SetItemState(index, INDEXTOSTATEIMAGEMASK(n), LVIS_STATEIMAGEMASK);

	EnableExecuteButtons();
}

BOOL CMultiSyncDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

int CMultiSyncDlg::PickupSelectedItems(CSimpleArray<CSyncData*>& array)
{
	CSyncData* psd;
	int i, cnt = 0, entry = m_list.GetItemCount();
	for (i = 0; i < entry; i++) {
		psd = (CSyncData*)m_list.GetItemData(i);
		if (psd) {
			if (m_list.GetItemState(i, LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK(2)) {
				CSyncData* psdCopy = new CSyncData();
				*psdCopy = *psd;
				array.Add(psdCopy);
				cnt++;
			}
		}
	}
	return cnt;
}

void CMultiSyncDlg::OnOK(UINT id)
{
	m_pml->SetBackup(id == IDC_SYNC_BACKUP);

	// 選択された処理パタンのチェック
/* @@
	CSyncData* psd;
	int i, cnt, entry = m_list.GetItemCount();
	for (i = 0, cnt = 0; i < entry; i++) {
		psd = (CSyncData*)m_list.GetItemData(i);
		if (psd) {
			if (m_list.GetItemState(i, LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK(2)) {
				m_list.SetItemData(i, NULL);
				m_pml->Add(psd);
				cnt++;
			}
		}
	}
	if (cnt < 1) {
		AppMessage(IDS_TITLE_MULTISYNC, IDS_MULTISYNC_NOPAT, NULL, MB_STOP);
		return;
	}
@@ */
	if (PickupSelectedItems(*m_pml) < 1) {
		AppMessage(IDS_TITLE_MULTISYNC, IDS_MULTISYNC_NOPAT, NULL, MB_STOP);
		return;
	}
	EndDialog(IDOK);
}

void CMultiSyncDlg::PatternListup()
{
	// 設定ファイルのリストアップ
	m_list.DeleteAllItems();

	TCHAR path[MAX_PATH * 2];
	fopGetSyncDataFolder(path);

	CFileArray array;
	array.BrowseFiles(path, sszFSDW);

	int i, index;
	for (i = 0; i < (int)array.GetCount(); i++) {
		CFileInfo* pf = array[i];
		CSyncData* psd = new CSyncData;
		if (!psd->Load(pf->m_strFullPathName)) {
			delete psd;
			continue;
		}

		index = m_list.InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM,
								i, psd->m_strTitle,
								INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK,
								GetSyncIconSysIndex(), (LPARAM)psd);
		m_list.SetItemText(index, 1, psd->m_strSrcFolder);
		m_list.SetItemText(index, 2, psd->m_strDstFolder);
	}
	if (m_list.GetItemCount() > 0)
		m_list.SelectItem(0);
	else {
		GetDlgItem(IDOK).EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT).EnableWindow(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// ScriptSyncDlg

class CScriptSyncDlg : public CDialogImpl<CScriptSyncDlg>, public CDialogResize<CScriptSyncDlg>
{
private:
	gdiplus_using m_gu;
	bool m_bUpdate:1;

public:
	CScriptSyncDlg() : m_pss(NULL), m_bUpdate(false) { }

	enum { IDD = IDD_SCRIPTSYNC };

BEGIN_MSG_MAP(CScriptSyncDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CScriptSyncDlg>)
	ON_WM_HELPINFO()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNotifyClick)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, OnNotifyKeyDown)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, OnNotifyCustomDraw)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_SYNC_BACKUP, OnBackup);
	ON_BN_CLICKED(IDC_SYNC_RESTORE, OnRestore);
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_ADD, OnItemAdd)
	ON_BN_CLICKED(IDC_NEW, OnItemNew)
	ON_BN_CLICKED(IDC_EDIT, OnItemEdit)
	ON_BN_CLICKED(IDC_DELETE, OnItemDelete)
	ON_BN_CLICKED(IDC_UP, OnItemMoveUp)
	ON_BN_CLICKED(IDC_DOWN, OnItemMoveDown)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CScriptSyncDlg)
	DLGRESIZE_CONTROL(IDC_LIST,				DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_NEW,				DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_ADD,				DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_EDIT,				DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_DELETE,			DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_UP,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_DOWN,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_HORZ_SEPARATOR,	DLSZ_SIZE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_SYNC_BACKUP,		DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_SYNC_RESTORE,		DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_HELPCALL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnNotifyCustomDraw(LPNMHDR pnmh, LRESULT* pResult);
	void OnNotifyClick(LPNMHDR pnmh, LRESULT* pResult);
	void OnNotifyKeyDown(LPNMHDR pnmh, LRESULT* pResult);
	void OnBackup();
	void OnRestore();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }
	void OnItemAdd();
	void OnItemNew();
	void OnItemEdit();
	void OnItemDelete();
	void OnItemMoveUp();
	void OnItemMoveDown();

	int AddItem(CSyncInfo* info);
	void ToggleMark(int index);
	void EnableButtons();

	CListViewCtrl m_list;
	CScriptSync* m_pss;
};


static const _HIMMAP hmmScr[] = {
	{ IDC_SYNC_RESTORE,	IDB_ARLEFT,		16,	0 },
	{ IDC_SYNC_BACKUP,	IDB_ARRIGHT,	16,	0 },
	{ IDC_NEW,			IDB_NEW,		16,	0 },
	{ IDC_ADD,			IDB_ADD,		16,	0 },
	{ IDC_DELETE,		IDB_DELETE,		16,	0 },
	{ IDC_EDIT,			IDB_EDIT,		16,	0 },
	{ IDC_UP,			IDB_SPINUP,		16,	0 },
	{ IDC_DOWN,			IDB_SPINDOWN,	16,	0 },
};

BOOL CScriptSyncDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();

	CenterWindow();
	SetDialogIcon(m_hWnd, IDI_SCRSYNC);
	m_list = GetDlgItem(IDC_LIST);
	m_pss = (CScriptSync*)lParam;

	HIMAGELIST him, himS;
	Shell_GetImageLists(&him, &himS);
	m_list.SetImageList(himS, LVSIL_SMALL);

	HINSTANCE hInst = RESINST;
	him = lccLoadImageList(hInst, RT_RCDATA, MAKEINTRESOURCE(IDB_STATE), 16, RGB(255, 0, 255));
	m_list.SetImageList(him, LVSIL_STATE);

	// 列の挿入
	int i;
	for(i = 0; i < NUM_COLUMNS; i++) {
		CString str(sstHeadColumn[i].label);
		m_list.InsertColumn(i, str, sstHeadColumn[i].fmt, sstHeadColumn[i].cx, i);
	}

	CSyncInfo* info;
	int cnt = m_pss->GetSize();
	for (i = 0; i < cnt; i++) {
		info = (*m_pss)[i];
		AddItem(info);
	}

	if (m_list.GetItemCount() > 0)
		m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	for (i = 0; i < ARRAYSIZE(hmmScr); i++)
		SetButtonImage(GetDlgItem(hmmScr[i].idCtrl), hmmScr[i].idRes, himmap[i].width);
	EnableButtons();

	return TRUE;
}

void CScriptSyncDlg::OnDestroy()
{
	if (m_bUpdate)
		m_pss->Save();
}

BOOL CScriptSyncDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

#define STATEIMAGEMASKTOINDEX(i) ((i) >> 12)

void CScriptSyncDlg::OnNotifyCustomDraw(LPNMHDR pnmh, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* lvcd = (NMLVCUSTOMDRAW*)pnmh;
	switch (lvcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
	{
		DWORD state = m_list.GetItemState((int)lvcd->nmcd.dwItemSpec, LVIS_STATEIMAGEMASK);
		if (STATEIMAGEMASKTOINDEX(state) == 1)
			lvcd->clrText = GetSysColor(COLOR_GRAYTEXT);
		*pResult = CDRF_DODEFAULT;
		break;
	}
	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}


void CScriptSyncDlg::OnNotifyClick(LPNMHDR pnmh, LRESULT* pResult)
{
	CPoint pt;
	GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	UINT nFlags;
	int index = m_list.HitTest(pt, &nFlags);
	if (index != -1 && nFlags & LVHT_ONITEMSTATEICON)
		ToggleMark(index);
}

void CScriptSyncDlg::OnNotifyKeyDown(LPNMHDR pnmh, LRESULT* pResult)
{
	LV_KEYDOWN* plvk = (LV_KEYDOWN*)pnmh;
	if (plvk->wVKey == VK_SPACE) {
		int index = m_list.GetTopIndex();
		index = m_list.GetNextItem(index - 1, LVNI_SELECTED);
		if (index != -1)
			ToggleMark(index);
	}
}

void CScriptSyncDlg::OnBackup()
{
	m_pss->m_bBackup = TRUE;
	EndDialog(IDOK);
}

void CScriptSyncDlg::OnRestore()
{
	m_pss->m_bBackup = FALSE;
	EndDialog(IDOK);
}

void CScriptSyncDlg::OnItemAdd()
{
	const int bufsize = MAX_PATH * 32;
	TCHAR* pNames = new TCHAR[bufsize];
	TCHAR buf[MAX_PATH * 2];
	fopGetSyncDataFolder(buf);
	if (FileBrowseMultiSel(m_hWnd, IDS_CAPTION_BROWSEFSD, IDS_FILTER_FSD, buf, pNames, bufsize)) {
		LPTSTR pw = pNames;
		LPTSTR pPath = pNames;
		if (!_pathisdirectory(pw))		// 最初がフォルダならば
			pPath = NULL;
		else
			pw += lstrlen(pw) + 1;

		while (*pw) {
			CSyncInfo* info = new CSyncInfo();
			if (pPath)
				PathCombine(buf, pPath, pw);
			else
				lstrcpy(buf, pw);

			info->SetInfo(buf);
			int index = AddItem(info);
			if (index >= 0) {
				m_pss->AddItem(info);
				m_bUpdate = true;

				m_list.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				EnableButtons();
			}
			else
				delete info;

			pw += lstrlen(pw) + 1;
		}
	}
	delete [] pNames;
}

void CScriptSyncDlg::OnItemNew()
{
	CSyncData sd;
	CPatEditDlg dlg;
	if (dlg.DoModal(m_hWnd, (LPARAM)&sd) == IDOK) {
		TCHAR pat[MAX_PATH * 2];
		MakePatPath(m_hWnd, sd.m_strTitle, pat);
		if (sd.Save(pat)) {
			CSyncInfo* info = new CSyncInfo();
			info->SetInfo(pat);
			int index = AddItem(info);
			if (index >= 0) {
				m_pss->AddItem(info);
				m_bUpdate = true;

				m_list.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				EnableButtons();
			}
			else
				delete info;
		}
	}
}

void CScriptSyncDlg::OnItemEdit()
{
	int index = m_list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (index < 0) {
		MessageBeep(0);
		return;
	}

	CSyncInfo* info = (CSyncInfo*)m_list.GetItemData(index);
	CSyncData sd;
	if (sd.Load(info->FileName())) {
		CPatEditDlg dlg;
		if (dlg.DoModal(m_hWnd, (LPARAM)&sd) == IDOK) {
			m_list.SetItemText(index, 0, info->Title());
			m_list.SetItemText(index, 1, info->Source());
			m_list.SetItemText(index, 2, info->Target());
			m_list.SetFocus();
		}
	}
}

void CScriptSyncDlg::OnItemDelete()
{
	int index = m_list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (index >= 0) {
		CSyncInfo* info = (CSyncInfo*)m_list.GetItemData(index);
		if (AppMessage(IDS_CAPTION_DELSOL, IDS_QUERY_REMOVESOL, info->Title(), MB_QYESNO) == IDYES) {
			m_list.DeleteItem(index);
			m_pss->Delete(index);
			m_bUpdate = true;
			EnableButtons();
		}
	}
}

void CScriptSyncDlg::OnItemMoveUp()
{
	int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (sel > 0) {
		CSyncInfo* pSrc = (CSyncInfo*)m_list.GetItemData(sel);
		CSyncInfo* pDst = (CSyncInfo*)m_list.GetItemData(sel - 1);
		DWORD stateSrc = m_list.GetItemState(sel, LVIS_STATEIMAGEMASK);
		DWORD stateDst = m_list.GetItemState(sel - 1, LVIS_STATEIMAGEMASK);

		m_pss->Exchange(sel, sel - 1);

		m_list.SetItem(sel, 0, LVIF_TEXT | LVIF_PARAM, pDst->Title(), 0, 0, 0, (LPARAM)pDst);
		m_list.SetItemText(sel, 1, pDst->Source());
		m_list.SetItemText(sel, 2, pDst->Target());
		m_list.SetItemState(sel, stateDst, LVIS_STATEIMAGEMASK);

		m_list.SetItem(sel - 1, 0, LVIF_TEXT | LVIF_PARAM, pSrc->Title(), 0, 0, 0, (LPARAM)pSrc);
		m_list.SetItemText(sel - 1, 1, pSrc->Source());
		m_list.SetItemText(sel - 1, 2, pSrc->Target());
		m_list.SetItemState(sel - 1, stateSrc, LVIS_STATEIMAGEMASK);
		m_list.SetItemState(sel - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		m_bUpdate = true;
	}
}

void CScriptSyncDlg::OnItemMoveDown()
{
	int cnt = (int)m_list.GetItemCount();
	int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (sel + 1 < cnt) {
		CSyncInfo* pSrc = (CSyncInfo*)m_list.GetItemData(sel);
		CSyncInfo* pDst = (CSyncInfo*)m_list.GetItemData(sel + 1);
		DWORD stateSrc = m_list.GetItemState(sel, LVIS_STATEIMAGEMASK);
		DWORD stateDst = m_list.GetItemState(sel + 1, LVIS_STATEIMAGEMASK);

		m_pss->Exchange(sel, sel + 1);

		m_list.SetItem(sel, 0, LVIF_TEXT | LVIF_PARAM, pDst->Title(), 0, 0, 0, (LPARAM)pDst);
		m_list.SetItemText(sel, 1, pDst->Source());
		m_list.SetItemText(sel, 2, pDst->Target());
		m_list.SetItemState(sel, stateDst, LVIS_STATEIMAGEMASK);

		m_list.SetItem(sel + 1, 0, LVIF_TEXT | LVIF_PARAM, pSrc->Title(), 0, 0, 0, (LPARAM)pSrc);
		m_list.SetItemText(sel + 1, 1, pSrc->Source());
		m_list.SetItemText(sel + 1, 2, pSrc->Target());
		m_list.SetItemState(sel + 1, stateSrc, LVIS_STATEIMAGEMASK);

		m_list.SetItemState(sel + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_bUpdate = true;
	}
}

int CScriptSyncDlg::AddItem(CSyncInfo* info)
{
	int cnt = m_list.GetItemCount();
	CSyncInfo* tmp;
	for (int i = 0; i < cnt; i++) {
		tmp = (CSyncInfo*)m_list.GetItemData(i);
		if (!_pathcompare(info->FileName(), tmp->FileName())) {
			MessageBeep(0);
			return FALSE;
		}
	}

	int index = m_list.InsertItem(cnt, info->Title(), GetSyncIconSysIndex());
	m_list.SetItemData(index, (DWORD_PTR)info);
	m_list.SetItemText(index, 1, info->Source());
	m_list.SetItemText(index, 2, info->Target());
	m_list.SetItemState(index, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
	return index;
}

void CScriptSyncDlg::ToggleMark(int index)
{
	DWORD state = m_list.GetItemState(index, LVIS_STATEIMAGEMASK);
	int n = (state == INDEXTOSTATEIMAGEMASK(2)) ? 1 : 2;
	m_list.SetItemState(index, INDEXTOSTATEIMAGEMASK(n), LVIS_STATEIMAGEMASK);
	m_pss->m_execmap.SetAtIndex(index, n - 1);
}

void CScriptSyncDlg::EnableButtons()
{
	BOOL bEnable = (m_list.GetItemCount() > 0);
	GetDlgItem(IDC_EDIT).EnableWindow(bEnable);
	GetDlgItem(IDC_DELETE).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_BACKUP).EnableWindow(bEnable);
	GetDlgItem(IDC_SYNC_RESTORE).EnableWindow(bEnable);
}


//////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopMultiSyncDir(HWND hwnd)
{
	CMSLocal msl;
	CMultiSyncDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&msl) == IDOK) {
		CScriptSync ss;
		CSyncData* psd;
		for (int i = 0; i < msl.GetSize(); i++) {
			psd = msl[i];
			ss.AddFile(psd->m_strPatName);
		}
		ss.m_bBackup = msl.IsBackup();

		TiXmlDocument xmlDoc;
		xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );
		ss.DoSync(hwnd, &xmlDoc);

		OpenSyncReport(hwnd, &xmlDoc);
		return TRUE;
	}
	return FALSE;
}

BOOL ExecSyncScript(HWND hwnd, LPCTSTR pszScript)
{
	BOOL bOK = FALSE;
	CScriptSync ss;

	TCHAR name[MAX_PATH];
	name[0] = '\0';
	if (pszScript && *pszScript) {
		lstrcpy(name, pszScript);
		PathUnquoteSpaces(name);
	}
	if (ss.Load(name)) {
		CScriptSyncDlg dlg;
		if (dlg.DoModal(hwnd, (LPARAM)&ss) == IDOK) {
			TiXmlDocument xmlDoc;
			xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );
			if (ss.DoSync(hwnd, &xmlDoc)) {
				OpenSyncReport(hwnd, &xmlDoc);
				bOK = TRUE;
			}
		}
	}
	return bOK;
}

//////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopScriptSyncNew(HWND hwnd)
{
	TCHAR scriptName[MAX_PATH * 2];
	if (ScriptSyncNew(hwnd, scriptName, ARRAYSIZE(scriptName))) {
		return ExecSyncScript(hwnd, scriptName);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////
//

static BOOL sync_execute_core(HWND hwnd, CSyncData &sd)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	CSyncRelay sr;
	if (!sd.isSilent()) {
		sr.Create(hwnd);
		sr.ShowWindow(SW_SHOWNORMAL);
	}
	sr.AddPat(sd.m_strTitle);
	sr.SetPatState(0, 1);

	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );
	TiXmlNode* pNode = xmlDoc.InsertEndChild( TiXmlElement( "log" ) );
	sd.SetLogger(pNode->ToElement());

	BOOL bOK = TRUE;
	if (sd.m_nDirection == DIRSYNC_SYNC) {
		// 先にバックアップ方向を実行
		sd.m_nDirection = DIRSYNC_BACKUP;
		bOK = SyncFolder(&sr, sd);
		sd.m_nDirection = DIRSYNC_RESTORE;
	}

	if (bOK)
		bOK = SyncFolder(&sr, sd);
	sr.SetPatState(0, bOK ? 2 : 3);
	sr.DestroyWindow();

	if (sd.isLogSave())
		SaveSyncReport(&xmlDoc);
	if (sd.m_bReport)
		OpenSyncReport(hwnd, &xmlDoc);
	return bOK;
}

BOOL ExecSyncCommand(HWND hwnd, LPCTSTR pszCmdline)
{
	CSyncData sd;
	INT_PTR nRes;

	CSimpleArray<CString> opts;			// オプションスイッチ格納用
	CSimpleArray<CString> targ;			// ターゲットフォルダ・パタンファイル格納用
	if (pszCmdline && *pszCmdline) {
		// 既存設定をオーバーライドするコマンドラインパラメタ
		// -a[0|1] copy accsess rights (-a0 = no-copy, -a1 = copy rights)
		// -d0 ... undelete non-exists folder and files
		// -d1 ... delete non-exists folder and files
		// -d2 ... delete non-exists folder and files width recycle bin
		// -e0 ... target is all files
		// -e1 ... target is exist files only
		// -mb or -m0 ... backup
		// -mr or -m1 ... restore
		// -mm or -m2 ... mirror
		// -y ... say yes for any question
		// -i ... minimize
		// -r[n/s/f] or -r[0/1/2]
		//        n or 0 ... no repoprt
		//        s or 1 ... show repoprt
		//        f or 2 ... save report to logfile
		// -v0 ... no verify
		// -v1 ... verify
		// -f0 ... current directory only
		// -f1 ... directory with child
		// -w0 ... *.*
		// -w1"…" ... match filter
		// -w2"…" ... umatch filter
		// -X"…" ... exclude folders
		// -p0 ... normal priority
		// -p1 ... below normal priority
		// -p2 ... low priority
		// -s0 ... donot rename old file
		// -s1 ... rename old file

		int len = lstrlen(pszCmdline);
		TCHAR* base = new TCHAR[(len + 16) * 2];
		LPTSTR p = base;
		lstrcpy(p, pszCmdline);
		LPTSTR arg = base + len + 16;
		while (*p) {
			p = SkipTabSpace(p);

			LPTSTR q = arg;
			while (*p && *p != ' ') {
				if (*p == '"') {
					*q++ = *p++;
					while (*p && *p != '"')
						*q++ = *p++;
					if (*p == '"')
						*q++ = *p++;
				}
				else
					*q++ = *p++;
			}
			*q = 0;
			if (*arg) {
				if (*arg == '/' || *arg == '-')
					opts.Add(arg + 1);
				else {
					PathUnquoteSpaces(arg);
					targ.Add(arg);
				}
			}
		}
		delete [] base;
	}

	// パタンを読み込む
	LPCTSTR ps = NULL;
	LPCTSTR pd = NULL;
	if (targ.GetSize() > 0) {
		ps = targ[0];
		if (targ.GetSize() > 1) {
			pd = targ[1];
		}
	}
	if (!sd.Load(ps, pd)) {
		AppMessage(IDS_TITLE_FOLDERSYNC, IDS_SYNC_PATFILEERROR, ps, MB_STOP);
		return FALSE;
	}

	// オプションスイッチを適用する
	CString str;
	int priority = THREAD_PRIORITY_NORMAL;
	for (int i = 0; i < opts.GetSize(); i++) {
		LPCTSTR ptr = opts[i];
		switch (*ptr) {
		case 'a':
			ptr++;
			switch (*ptr) {
			case '0': sd.m_dwFlags &= ~SYNC_COPYSID; break;
			case '1': sd.m_dwFlags |= SYNC_COPYSID; break;
			default: return FALSE;
			}
			break;
		case 'd':
			ptr++;
			switch (*ptr) {
			case '0': sd.m_bMirror = false; break;
			case '1': sd.m_bMirror = true; sd.m_dwFlags &= ~SYNC_RECYCLE; break;
			case '2': sd.m_bMirror = true; sd.m_dwFlags |= SYNC_RECYCLE; break;
			default: return FALSE;
			}
			break;
		case 'e':
			ptr++;
			switch (*ptr) {
			case '0': sd.m_dwFlags &= ~SYNC_ONLYEXIST; break;
			case '1': sd.m_dwFlags |= SYNC_ONLYEXIST; break;
			default: return FALSE;
			}
			break;
		case 'm':
			ptr++;
			switch (*ptr) {
			case 'b': case '0': sd.m_nDirection = DIRSYNC_BACKUP; break;
			case 'r': case '1': sd.m_nDirection = DIRSYNC_RESTORE; break;
			case 'm': case '2': sd.m_nDirection = DIRSYNC_SYNC; break;
			default: return FALSE;
			}
			break;
		case 'y': sd.m_dwFlags |= SYNC_NOCONFIRM; break;
		case 'i': sd.m_dwFlags |= SYNC_SILENT; break;
		case 'r':
			ptr++;
			switch (*ptr) {
			case 'n': case '0': sd.m_bReport = FALSE; break;
			case 's': case '1': sd.m_bReport = TRUE; break;
			case 'f': case '2': sd.m_dwFlags |= SYNC_SAVELOG; break;
			default: return FALSE;
			}
			break;
		case 'f':
			ptr++;
			switch (*ptr) {
			case '0': sd.m_bSubdir = FALSE; break;
			case '1': sd.m_bSubdir = TRUE; break;
			default: return FALSE;
			}
			break;
		case 'p':
			ptr++;
			switch (*ptr) {
			case '1': priority = THREAD_PRIORITY_BELOW_NORMAL; break;
			case '2': priority = THREAD_PRIORITY_LOWEST; break;
			default: break;
			}
			break;
		case 's':
			ptr++;
			switch (*ptr) {
			case '0': sd.m_bSaveOld = false; break;
			case '1': sd.m_bSaveOld = true; break;
			default: break;
			}
			break;
		case 'w':
			ptr++;
			switch (*ptr) {
			case '0':
				sd.m_bHasFilter = false;
				sd.m_strFilter.Empty(); break;
				break;
			case '1':
				str = ++ptr;
				sd.m_strFilter = str.Trim('"');
				sd.m_bHasFilter = !sd.m_strFilter.IsEmpty();
				break;
			case '2':
				str = ++ptr;
				sd.m_strFilter = str.Trim('"');
				sd.m_bExclude = sd.m_bHasFilter = !sd.m_strFilter.IsEmpty();
				break;
			default: break;
			}
			break;
		case 'x':
			str = ++ptr;
			sd.m_strExcludeFolder = str.Trim('"');
			break;
		default:
			return FALSE;
		}
	}

	// 開始前の同期内容確認
	if (!sd.isNoConfirm()) {
		CSyncDlg dlg;
sdr_retry:
		nRes = dlg.DoModal(hwnd, (LPARAM)&sd);
		if (nRes == IDD_MULTISYNC) {
			if (!fopMultiSyncDir(hwnd))
				goto sdr_retry;
			return TRUE;
		}
		else if (nRes != IDOK)
			return FALSE;

		// 実行確認
		if (!sd.ExecConfirm())
			return FALSE;
	}

	// プライオリティ設定
	if (priority != THREAD_PRIORITY_NORMAL)
		SetThreadPriority(GetCurrentThread(), priority);

	// 直前の設定として別途記録
	CSyncData tmpsd(sd);
	tmpsd.Save(BuildPrevSettingFileName(str));

	return sync_execute_core(hwnd, sd);
}

static BOOL SyncTree(CSyncRelay* psr, LPCTSTR pszFrom, LPCTSTR pszTo, CSyncData& sd)
{
	if (psr->isCanceled()) return FALSE;

	if (sd.isFolderExclude(pszFrom)) {
		// 対象外のフォルダであればスキップ
		sd.Report(REPTYPE_IGNORE | REPTYPE_SRCPATH, pszFrom);
		return TRUE;
	}

	psr->SetDirectory(pszFrom, pszTo);

	if (!_pathisdirectory(pszTo)) {
		if (!fopMakeTree(pszTo)) {
			sd.Report(REPTYPE_MKDIR | REPTYPE_DSTPATH | REPTYPE_ERROR, pszTo, GetLastError());
			return FALSE;
		}

		// フォルダ属性を同じに
		CWin32FindData wfd;
		if (GetFileData(pszFrom, wfd))
			SetFileProperties(pszTo, wfd.dwFileAttributes, &wfd.ftLastWriteTime, &wfd.ftCreationTime);
	}

	// 相手先ドライブ情報の取得
	sd.m_diskTarget.SetRoot(pszTo);
	if (!sd.m_diskTarget.GetDriveInfo()) return FALSE;

	CFileInfo* pf;
	CFileArray arSrc, arDst;
	if (sd.hasFilter())
		arSrc.BrowseFiles(pszFrom, sd.m_strFilter, sd.UnmatchFilter(), sd.UseRegExp(), sd.isCaseSensitive());	// 指定のファイルを収集
	else
		arSrc.BrowseFiles(pszFrom, gszAst);												// すべてのファイルを収集

	TCHAR dst[MAX_PATH * 2];
	int i;
	for (i = 0; i < (int)arSrc.GetCount(); i++) {
		if (psr->isCanceled()) return FALSE;

		pf = arSrc[i];
		psr->SetLine(pf->m_pszName);
		PathCombine(dst, pszTo, pf->m_pszName);
		int n = SyncFile(psr, pf->m_strFullPathName, dst, sd);
		psr->SetComplete(pf->m_dlSize);

		if (n == CR_ERROR) return FALSE;
	}

	DWORD error = NOERROR;
	if (sd.m_bMirror) {
		arDst.BrowseFiles(pszTo, sd.m_strFilter, sd.UnmatchFilter(), sd.UseRegExp(), sd.isCaseSensitive());

		for (i = 0; i < (int)arDst.GetCount(); i++) {
			error = NOERROR;
			pf = arDst[i];
			pf->GetFullPathName(dst, ARRAYSIZE(dst));
			if (arSrc.GetItemNameIndex(pf->m_pszName) < 0) {
				UINT nRep = REPTYPE_DELETE;
				if (sd.isRecycle())  {
					error = TrashOneFile(NULL, dst, FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI);
					if (error != NOERROR)
						nRep |= REPTYPE_ERROR;
				}
				else {
					if (!fopRemoveFileNoConfirm(dst)) {
						error = GetLastError();
						nRep |= REPTYPE_ERROR;
					}
				}
				sd.Report(nRep | REPTYPE_DSTPATH, dst, error);
			}
		}
	}

	arSrc.DeleteAll();
	arDst.DeleteAll();
	if (sd.m_bSubdir) {
		arSrc.EnumSubFolder(pszFrom);
		for (i = 0; i < (int)arSrc.GetCount(); i++) {
			pf = arSrc[i];
			PathCombine(dst, pszTo, pf->m_pszName);

			// 相手に存在するフォルダのみ？
			if (sd.isOnlyExist() && !_pathisdirectory(dst)) continue;

			if (!SyncTree(psr, pf->m_strFullPathName, dst, sd)) return FALSE;
		}

		if (sd.m_bMirror) {
			arDst.EnumSubFolder(pszTo);
			for (i = 0; i < (int)arDst.GetCount(); i++) {
				pf = arDst[i];
				pf->GetFullPathName(dst, ARRAYSIZE(dst));
				if (arSrc.GetItemNameIndex(pf->m_pszName) < 0) {
					error = NOERROR;
					UINT nRep;
					if (sd.isFolderExclude(dst)) {
						// 対象外として指定されたフォルダは削除しない
						nRep = REPTYPE_IGNORE;
					}
					else {
						nRep = REPTYPE_RMDIR;
						if (sd.isRecycle()) {
							error = TrashOneFile(NULL, dst, FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI);
							if (error != NOERROR)
								nRep |= REPTYPE_ERROR;
						}
						else {
							if (!fopRemoveTreeNoConfirm(dst)) {
								error = GetLastError();
								nRep |= REPTYPE_ERROR;
							}
						}
					}
					sd.Report(nRep | REPTYPE_DSTPATH, dst, error);
				}
			}
		}
	}
	return TRUE;
}

static DWORD CALLBACK SyncCopyProgressCallback(
	LARGE_INTEGER liTotalFileSize,			// バイト単位の総ファイルサイズ
	LARGE_INTEGER liTotalBytesXferred,		// 転送された総バイト数
	LARGE_INTEGER liStreamSize,				// このストリームの総バイト数
	LARGE_INTEGER liStreamBytesXferred,		// このストリームに対して転送された総バイト数
	DWORD nStreamNumber,					// 現在のストリーム
	DWORD nCallbackReason,					// CopyProgressRoutine 関数が呼び出された理由
	HANDLE hsrc,							// コピー元ファイルのハンドル
	HANDLE hdst,							// コピー先ファイルのハンドル
	LPVOID param							// CopyFileEx 関数から渡される
)
{
	DWORD code = PROGRESS_CONTINUE;
	CSyncRelay* psr = (CSyncRelay*)param;
	if (psr) {
		// キャンセルされた？
		if (psr->isCanceled()) {
			CSyncContinue dlg;
			dlg.DoModal(psr->m_hWnd);
			switch (dlg.Result()) {
			case 0: code = PROGRESS_CONTINUE; psr->ResetCancel(); break;
			case 1: code = PROGRESS_QUIET; break;
			case 2: code = PROGRESS_STOP; break;
			}
		}
	}
	return code;
}

static int SyncFile(CSyncRelay* psr, LPCTSTR pszFrom, LPCTSTR pszTo, CSyncData& sd)
{
	// キャンセルされた？
	if (psr->isCanceled()) return CR_ERROR;

	// 相手に存在するファイルのみ？
	BOOL bExist = _pathfileexists(pszTo);
	DWORD err = GetLastError();
	if (!bExist && (err != ERROR_FILE_NOT_FOUND)) {
		// 2007.02.28 何らかの理由でエラーになってしまった場合
		// 例えばネットワークの切断でファイル情報の収集ができない等
		sd.Report(REPTYPE_PREXFER | REPTYPE_DSTPATH | REPTYPE_ERROR, pszTo, err);
		return CR_ERROR;
	}

	if (sd.isOnlyExist() && !bExist) return CR_DONE;

	CFileInfo from;
	GetFileInfo(pszFrom, from);
	DWORDLONG dlDelta = from.m_dlSize;

	if (bExist) {
		CFileInfo to;
		GetFileInfo(pszTo, to);

		long lc = from.CompareLastWriteTime(to, FALSE);		// 秒単位で比較
		if (sd.m_iDosCompare && lc) {
			// FAT/FAT32/exFAT <-> NTFSファイルへのコピーは、秒数の丸めを考慮して比較
			FILETIME fts, ftt;
			fts = from.m_ftLastWriteTime;
			ftt = to.m_ftLastWriteTime;
			if (sd.m_iDosCompare < 0) {
				// DST is NTFS
				_time_NTFS2FAT(ftt);
			}
			else {
				// SRC is NTFS
				_time_NTFS2FAT(fts);
			}
			lc = CompareFileTime(&fts, &ftt);
		}

		if (lc < 0 || (lc == 0 && from.m_dlSize == to.m_dlSize))
			return CR_IGNORED;

		if (sd.m_bSaveOld) {
			// 以前のファイルを名前を変えて残す場合
			if (!RenameTemp(pszTo, sd))
				return CR_IGNORED;
		}
		else {
			SetFileAttributes(pszTo, FILE_ATTRIBUTE_NORMAL);
			dlDelta = (dlDelta > to.m_dlSize) ? dlDelta - to.m_dlSize : 0;
		}
	}

	UINT nRep = bExist ? REPTYPE_OVERWRITE : REPTYPE_NEW;

	static BOOL bCancel = FALSE;
	if (CopyFileEx(pszFrom, pszTo, SyncCopyProgressCallback, (void*)psr, &bCancel, COPY_FILE_ALLOW_DECRYPTED_DESTINATION)) {
		CDiskSpace s(pszFrom, true);
		CDiskSpace t(pszTo, true);
		// 双方がACLサポートならsidをコピー
		if (sd.isCopyAccessRights() && s.isPersistentAcls() && t.isPersistentAcls())
			CopyAccessRights(NULL, pszFrom, pszTo);
		sd.Report(nRep | REPTYPE_DSTPATH, pszTo);
		return CR_DONE;
	}
	err = GetLastError();
	sd.Report(nRep | REPTYPE_DSTPATH | REPTYPE_ERROR, pszTo, err);

	if (err == ERROR_REQUEST_ABORTED) {
		// コピー処理が中途で終わって、ファイルが不完全なため削除する
		fopRemoveFileNoConfirm(pszTo);
	}
	return ((err == ERROR_DISK_FULL ||
			 err == ERROR_HANDLE_DISK_FULL ||
			 err == ERROR_PATH_NOT_FOUND) ? CR_ERROR : CR_IGNORED);
}

static bool GetSyncSize(CSyncRelay* psr, DWORDLONG& dlSize, LPCTSTR pszWork, CSyncData& sd)
{
	// キャンセルされた？
	if (psr->isCanceled()) return false;

	psr->SetDirectory(pszWork, NULL);
	DoMessages();

	register CFileInfo* pf;
	CFileArray arr;
	arr.BrowseFiles(pszWork, sd.m_strFilter, sd.UnmatchFilter(), sd.UseRegExp(), sd.isCaseSensitive());

	int i;
	for (i = 0; i < (int)arr.GetCount(); i++) {
		pf = arr[i];
		dlSize += pf->m_dlSize;
	}

	if (sd.m_bSubdir) {
		arr.DeleteAll();
		arr.EnumSubFolder(pszWork);
		for (i = 0; i < (int)arr.GetCount(); i++) {
			pf = arr[i];
			if (!sd.isFolderExclude(pf->m_strFullPathName)) {
				if (!GetSyncSize(psr, dlSize, pf->m_strFullPathName, sd))
					return false;
			}
		}
	}
	return true;
}

BOOL SyncFolder(CSyncRelay* psr, CSyncData& sd)
{
	// 指定されたモードをチェックして、コピー元とコピー先を確定する
	CString pathFrom, pathTo;
	if (sd.m_nDirection == DIRSYNC_BACKUP) {
		pathFrom = sd.m_strSrcFolder;
		pathTo = sd.m_strDstFolder;
	}
	else {
		pathFrom = sd.m_strDstFolder;
		pathTo = sd.m_strSrcFolder;
	}

	// ------- マクロ置換開始

	// %date%, %ldate% ---> ex. 20081231
	SYSTEMTIME st;
	GetLocalTime(&st);
	CString date;
	date.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);
	pathFrom.Replace(_T("%date%"), date);
	pathTo.Replace(_T("%date%"), date);
	pathFrom.Replace(_T("%ldate%"), date);
	pathTo.Replace(_T("%ldate%"), date);

	// %sdate% ---> ex. 081231
	date.Format(_T("%02d%02d%02d"), st.wYear % 100, st.wMonth, st.wDay);
	pathFrom.Replace(_T("%sdate%"), date);
	pathTo.Replace(_T("%sdate%"), date);

	// %YYYY% ---> 2008年
	date.Format(_T("%04d"), st.wYear);
	pathFrom.Replace(_T("%YYYY%"), date);
	pathTo.Replace(_T("%YYYY%"), date);

	// %YY% ---> 08年
	date.Format(_T("%04d"), st.wYear % 100);
	pathFrom.Replace(_T("%YY%"), date);
	pathTo.Replace(_T("%YY%"), date);

	// %MM% ---> 12月
	date.Format(_T("%02d"), st.wMonth);
	pathFrom.Replace(_T("%MM%"), date);
	pathTo.Replace(_T("%MM%"), date);

	// %DD% ---> 31日
	date.Format(_T("%02d"), st.wDay);
	pathFrom.Replace(_T("%DD%"), date);
	pathTo.Replace(_T("%DD%"), date);

	// %hh% ---> 23時
	date.Format(_T("%02d"), st.wHour);
	pathFrom.Replace(_T("%hh%"), date);
	pathTo.Replace(_T("%hh%"), date);

	// %mm% ---> 59分
	date.Format(_T("%02d"), st.wMinute);
	pathFrom.Replace(_T("%mm%"), date);
	pathTo.Replace(_T("%mm%"), date);

	// %ss% ---> 59秒
	date.Format(_T("%02d"), st.wSecond);
	pathFrom.Replace(_T("%ss%"), date);
	pathTo.Replace(_T("%ss%"), date);

	// ------- マクロ置換終わり

	// コピー元とコピー先が同一パスならば中止
	CString str;
	if (!_pathcompare(pathFrom, pathTo)) {
		str.LoadString(IDS_SAMEPATH);
		sd.Report(REPTYPE_MESSAGE | REPTYPE_ERROR, str, ERROR_BAD_PATHNAME);
		return FALSE;
	}

	// コピー先パスがコピー元パスの子孫ならば中止
	if (!_pathisparent(pathFrom, pathTo)) {
		str.LoadString(IDS_SAMEPATH);
		sd.Report(REPTYPE_MESSAGE | REPTYPE_ERROR, str, ERROR_BAD_PATHNAME);
		return FALSE;
	}
	// パスの確認
	if (!validate_dir(psr->m_hWnd, pathFrom, false)) {
		sd.Report(REPTYPE_MESSAGE | REPTYPE_ERROR, pathFrom, GetLastError());
		return FALSE;
	}
	if (!validate_dir(psr->m_hWnd, pathTo, false)) {
		sd.Report(REPTYPE_MESSAGE | REPTYPE_ERROR, pathTo, GetLastError());
		return FALSE;
	}
	// コピー元のフォルダとコピー先のフォルダの属性を同一にする
	CWin32FindData wfd;
	if (GetFileData(pathFrom, wfd))
		SetFileProperties(pathTo, wfd.dwFileAttributes, &wfd.ftLastWriteTime, &wfd.ftCreationTime);

	// FAT/FAT32/exFAT <-> NTFSファイルへのコピーは、
	// 秒数の丸めを考慮して比較
	CDiskSpace si(pathFrom, true);
	CDiskSpace di(pathTo, true);

	// NTFS->FAT m_iDosCompare > 0, FAT->NTFS m_iDosCompare < 0
	if (!si.isDosDateTime() && di.isDosDateTime())
		sd.m_iDosCompare = 1;
	else if (si.isDosDateTime() && !di.isDosDateTime())
		sd.m_iDosCompare = -1;
	else
		sd.m_iDosCompare = 0;

	// 開始時刻取得
	SYSTEMTIME sts;
	GetLocalTime(&sts);

	// ディレクトリのチェック
	psr->InitSearch();
	psr->SetLine(MAKEINTRESOURCE(IDS_GATHER_INFORMATION));
	DWORDLONG dlSize = 0;
	BOOL ok = FALSE;
	if (GetSyncSize(psr, dlSize, pathFrom, sd)) {
		psr->InitSync(dlSize);
		ok = SyncTree(psr, pathFrom, pathTo, sd);
	}
	// 終了時刻取得
	SYSTEMTIME ste;
	GetLocalTime(&ste);

	// 経過時間計算
	CTime t1(sts);
	CTime t2(ste);
	CTimeSpan ts = t2 - t1;
	long span = (long)ts.GetTotalSeconds();

	CString s, e;
	format_datetime(&sts, s);
	format_datetime(&ste, e);
	str.Format(IDS_SYNC_STARTFINISH, s.GetString(), e.GetString(), span / 60, span % 60);
	sd.SetLogDescription(str);

/* @@ 処理完了時刻を保存する
	if (IsFile(sd.m_strPatName)) {
		GetLocalTime(&sd.m_st);
		sd.Save(sd.m_strPatName);
	}
@@ */
	return ok;
}

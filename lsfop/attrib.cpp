#include "stdafx.h"
#include "fopstd.h"

////////////////////////////////////////////////////////////////
// フラグ

#define SFA_FILEATTRIBUTES		0x00000001			// 属性を変更
#define SFA_CREATIONTIME		0x00000010			// 作成日時を変更
#define SFA_LASTWRITETIME		0x00000020			// 最終書込日時を変更
#define SFA_LASTACCESSTIME		0x00000040			// 最終アクセス日時を変更
#define SFA_TIMEMASK			0x00000070
#define SFA_BASEMASK			0x00000F00

enum {
	FASORT_LASTWRITE = 0,
	FASORT_CREATION,
	FASORT_ENTRY,
};

typedef int (__cdecl * QSORTCOMPPROC)(const void*, const void*, const void*);
void __cdecl qsort_ex(void *base, size_t n, size_t size, QSORTCOMPPROC, const void* param);

static int __cdecl compare_attr(const void* arg1, const void* arg2, const void* param)
{
	WIN32_FIND_DATA* p1 = (WIN32_FIND_DATA*)arg1;
	WIN32_FIND_DATA* p2 = (WIN32_FIND_DATA*)arg2;
	int r = 0;
	switch (PtrToUlong(param)) {
	// 更新日付
	case FASORT_LASTWRITE: r = CompareFileTime(&p1->ftLastWriteTime, &p2->ftLastWriteTime); break;
	// 作成日付
	case FASORT_CREATION: r = CompareFileTime(&p1->ftCreationTime, &p2->ftCreationTime); break;
	// エントリー順
	case FASORT_ENTRY:
	default: r = (int)p1->nFileSizeLow - (int)p2->nFileSizeLow; break;
	}
    return r;
}

class CAttribData
{
public:
	WIN32_FIND_DATA wfd;
	WIN32_FIND_DATA wfdarr[6];
	DWORD flags;
	BOOL subdir;
	BOOL multifiles;
	CAttribData() : flags(0), subdir(FALSE), multifiles(FALSE)
	{
		memset(&wfd, 0, sizeof(wfd));
		memset(&wfdarr, 0, sizeof(wfdarr));
	}
};


/////////////////////////////////////////////////////////////////////////////
//

BOOL SetFileDateTimes(LPCTSTR pszName, LPFILETIME pftCreate, LPFILETIME pftWrite, LPFILETIME pftAccess)
{
	DWORD attr = GetFileAttributes(pszName);
	if (attr == 0xFFFFFFFF)
		return FALSE;

	if (!SetFileAttributes(pszName, attr & ~FILE_ATTRIBUTE_READONLY))
		return FALSE;

	BOOL bOK = FALSE;
	DWORD flags = 0;
	if (attr & FILE_ATTRIBUTE_DIRECTORY)
		flags |= FILE_FLAG_BACKUP_SEMANTICS;

	HANDLE hf = CreateFile(pszName, GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL, OPEN_EXISTING, flags, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		FILETIME ftc, ftw, fta;
		LPFILETIME pc = NULL, pw = NULL, pa = NULL;
		if (pftCreate) {
			LocalFileTimeToFileTime(pftCreate, &ftc);
			pc = &ftc;
		}
		if (pftWrite) {
			LocalFileTimeToFileTime(pftWrite, &ftw);
			pw = &ftw;
		}
		if (pftAccess) {
			LocalFileTimeToFileTime(pftAccess, &fta);
			pa = &fta;
		}
		bOK = SetFileTime(hf, pc, pa, pw);
		CloseHandle(hf);
	}
	SetFileAttributes(pszName, attr);
	return bOK;
}

/////////////////////////////////////////////////////////////////////////////
//

#define IDT_CURRENTTIME		8847

/////////////////////////////////////////////////////////////////////////////
// AttrDlgFn


static TCHAR dtFormat[] = _T("yyyy'/'MM'/'dd HH':'mm':'ss'");
static TCHAR offFormat[] = _T("HH':'mm':'ss'");
static UINT sarAttrs[] = {
	IDC_FSYSTEM, IDC_FHIDDEN, IDC_FRDONLY, IDC_FARCH
};


class CFileAttrib : public CDialogImpl<CFileAttrib>
{
public:
	CFileAttrib() : m_ltimeClear(0), m_ctimeClear(0), m_dosDateTime(true) { memset(&m_ad, 0, sizeof(m_ad)); }
	~CFileAttrib() { }

	enum { IDD = IDD_ATTRIB };
	CAttribData m_ad;
	bool m_dosDateTime;
	WIN32_FIND_DATA m_keyfile[6];

private:
	int m_ltimeClear;
	int m_ctimeClear;

BEGIN_MSG_MAP(CFileAttrib)
	ON_WM_INITDIALOG()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_HELPINFO()
	ON_CBN_SELCHANGE(IDC_COMBO, OnBaseChange)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ATTR_CHANGE, OnChangeAttr)
	ON_BN_CLICKED(IDC_LTIME_CHECK, OnChangeLastWriteDate)
	ON_BN_CLICKED(IDC_LTIME_SET, OnChangeLastWriteDate)
	ON_BN_CLICKED(IDC_LTIME_TOUCH, OnChangeLastWriteDate)
	ON_BN_CLICKED(IDC_CTIME_CHECK, OnChangeCreationDate)
	ON_BN_CLICKED(IDC_CTIME_SET, OnChangeCreationDate)
	ON_BN_CLICKED(IDC_CTIME_TOUCH, OnChangeCreationDate)
	ON_BN_CLICKED(IDC_CTIME_SAMELTIME, OnChangeCreationDate)
	ON_BN_CLICKED(IDC_CLEAR_LTIME, OnClearLTime)
	ON_BN_CLICKED(IDC_CLEAR_CTIME, OnClearCTime)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnTimer(UINT_PTR idEvent);
	void OnDestroy() { KillTimer(IDT_CURRENTTIME); }
	void OnBaseChange();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnChangeAttr();
	void OnChangeLastWriteDate();
	void OnChangeCreationDate();
	void OnClearLTime();
	void OnClearCTime();
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	void ResetFileAttribSettings(int mode);
};

BOOL CFileAttrib::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	int stat = GetRegValue(_MKIR(IDS_SEC_OPERATION), _MKIR(IDS_KEY_ATTRPROP), SFA_FILEATTRIBUTES | SFA_LASTWRITETIME);
	if (stat & SFA_LASTWRITETIME)		CheckDlgButton(IDC_LTIME_CHECK, TRUE);
	if (stat & SFA_CREATIONTIME)		CheckDlgButton(IDC_CTIME_CHECK, TRUE);
	if (stat & SFA_FILEATTRIBUTES)		CheckDlgButton(IDC_ATTR_CHANGE, TRUE);

	CheckDlgButton(IDC_LTIME_SET, TRUE);
	CheckDlgButton(IDC_CTIME_SET, TRUE);

	if (!m_ad.subdir)
		GetDlgItem(IDC_DOSUBDIR).EnableWindow(FALSE);

	if (m_dosDateTime) {
		HMODULE hmod = GetModuleHandle(_T("user32"));
		HICON hic = (HICON)LoadImage(hmod, MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		if (hic) {
			GetDlgItem(IDC_HINT).SendMessage(RM_SETICON, 0, (LPARAM)hic);
			DestroyIcon(hic);
		}
	}
	else
		GetDlgItem(IDC_HINT).ShowWindow(SW_HIDE);

	CString str;
	str.Format(IDS_ATTRIB_BASETYPE,
			m_ad.wfdarr[0].cFileName, m_ad.wfdarr[1].cFileName,
			m_ad.wfdarr[2].cFileName, m_ad.wfdarr[3].cFileName,
			m_ad.wfdarr[4].cFileName, m_ad.wfdarr[5].cFileName);
	CComboBox combo = GetDlgItem(IDC_COMBO);
	InitDlgCtrl(combo, str);

	int sel = (stat & SFA_BASEMASK) >> 8;
	combo.SetCurSel(sel);
	if (!m_ad.multifiles)
		combo.EnableWindow(FALSE);
	// データをセット
	ResetFileAttribSettings(sel);

	SetTimer(IDT_CURRENTTIME, 500, NULL);
	return TRUE;
}

void CFileAttrib::ResetFileAttribSettings(int mode)
{
	// 指定のモードでエントリデータを取り出す
	m_ad.wfd = m_ad.wfdarr[min(ARRAYSIZE(m_ad.wfdarr), max(0, mode))];
	// ダイアログを再設定
	if (m_ad.wfd.dwFileAttributes & FA_SYSTEM)	CheckDlgButton(IDC_FSYSTEM, TRUE);
	if (m_ad.wfd.dwFileAttributes & FA_HIDDEN)	CheckDlgButton(IDC_FHIDDEN, TRUE);
	if (m_ad.wfd.dwFileAttributes & FA_RDONLY)	CheckDlgButton(IDC_FRDONLY, TRUE);
	if (m_ad.wfd.dwFileAttributes & FA_ARCH)	CheckDlgButton(IDC_FARCH, TRUE);

	SYSTEMTIME st;
	FileTimeToSystemTime(&m_ad.wfd.ftLastWriteTime, &st);

	CDateTimePickerCtrl dt(GetDlgItem(IDC_LTIME_PICKER));
	dt.SetFormat(dtFormat);
	dt.SetSystemTime(GDT_VALID, &st);

	FileTimeToSystemTime(&m_ad.wfd.ftCreationTime, &st);
	dt = GetDlgItem(IDC_CTIME_PICKER);
	dt.SetFormat(dtFormat);
	dt.SetSystemTime(GDT_VALID, &st);

	OnChangeLastWriteDate();
	OnChangeCreationDate();
	OnChangeAttr();
}

BOOL CFileAttrib::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CFileAttrib::OnTimer(UINT_PTR idEvent)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	TCHAR buf[MAX_PATH];
	wsprintf(buf, _T("%d/%02d/%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	SetDlgItemText(IDC_LTIME_CURRENT, buf);
	SetDlgItemText(IDC_CTIME_CURRENT, buf);
}

void CFileAttrib::OnBaseChange()
{
	CComboBox combo = GetDlgItem(IDC_COMBO);
	int sel = combo.GetCurSel();
	ResetFileAttribSettings(sel);
}

void CFileAttrib::OnOK()
{
	m_ad.flags = 0;
	m_ad.wfd.dwFileAttributes = 0;
	if (IsDlgButtonChecked(IDC_ATTR_CHANGE)) {
		m_ad.flags |= SFA_FILEATTRIBUTES;
		if (IsDlgButtonChecked(IDC_FSYSTEM))	m_ad.wfd.dwFileAttributes |= FA_SYSTEM;
		if (IsDlgButtonChecked(IDC_FHIDDEN))	m_ad.wfd.dwFileAttributes |= FA_HIDDEN;
		if (IsDlgButtonChecked(IDC_FRDONLY))	m_ad.wfd.dwFileAttributes |= FA_RDONLY;
		if (IsDlgButtonChecked(IDC_FARCH))		m_ad.wfd.dwFileAttributes |= FA_ARCH;
	}

	SYSTEMTIME st;
	CDateTimePickerCtrl dt;

	// 更新日時取得
	if (IsDlgButtonChecked(IDC_LTIME_CHECK)) {
		m_ad.flags |= SFA_LASTWRITETIME;

		if (IsDlgButtonChecked(IDC_LTIME_TOUCH))
			GetLocalTime(&st);
		else {
			dt = GetDlgItem(IDC_LTIME_PICKER);
			memset(&st, 0, sizeof(st));
			dt.GetSystemTime(&st);
		}
		st.wMilliseconds = 0;			// ミリ秒は0クリア
		SystemTimeToFileTime(&st, &m_ad.wfd.ftLastWriteTime);
	}

	// 作成日時取得
	if (IsDlgButtonChecked(IDC_CTIME_CHECK)) {
		m_ad.flags |= SFA_CREATIONTIME;

		if (IsDlgButtonChecked(IDC_CTIME_SAMELTIME)) {
			if (!(m_ad.flags & SFA_LASTWRITETIME)) {
				// 更新日時を取得
				dt = GetDlgItem(IDC_LTIME_PICKER);
				memset(&st, 0, sizeof(st));
				dt.GetSystemTime(&st);
			}
			// stには更新日時が入っている
		}
		else if (IsDlgButtonChecked(IDC_CTIME_TOUCH))
			GetLocalTime(&st);
		else {
			dt = GetDlgItem(IDC_CTIME_PICKER);
			memset(&st, 0, sizeof(st));
			dt.GetSystemTime(&st);
		}
		st.wMilliseconds = 0;			// ミリ秒は0クリア
		SystemTimeToFileTime(&st, &m_ad.wfd.ftCreationTime);
	}

	CComboBox combo = GetDlgItem(IDC_COMBO);
	int sel = combo.GetCurSel();
	if (sel < 0) sel = 0;
	SetRegValue(_MKIR(IDS_SEC_OPERATION), _MKIR(IDS_KEY_ATTRPROP), m_ad.flags | (sel << 8));
	m_ad.subdir = IsDlgButtonChecked(IDC_DOSUBDIR);

	EndDialog(IDOK);
}

void CFileAttrib::OnChangeAttr()
{
	BOOL bCheck = IsDlgButtonChecked(IDC_ATTR_CHANGE);
	for (int i = 0; i < sizeof(sarAttrs) / sizeof(UINT); i++)
		GetDlgItem(sarAttrs[i]).EnableWindow(bCheck);
}

void CFileAttrib::OnChangeLastWriteDate()
{
	BOOL bCheck = IsDlgButtonChecked(IDC_LTIME_CHECK);
	GetDlgItem(IDC_LTIME_SET).EnableWindow(bCheck);
	GetDlgItem(IDC_LTIME_TOUCH).EnableWindow(bCheck);

	if (!bCheck) {
		GetDlgItem(IDC_LTIME_PICKER).EnableWindow(FALSE);
		GetDlgItem(IDC_CLEAR_LTIME).EnableWindow(FALSE);
		GetDlgItem(IDC_LTIME_CURRENT).EnableWindow(FALSE);
	}
	else {
		if (IsDlgButtonChecked(IDC_LTIME_SET)) {
			GetDlgItem(IDC_LTIME_PICKER).EnableWindow(TRUE);
			GetDlgItem(IDC_CLEAR_LTIME).EnableWindow(TRUE);
			GetDlgItem(IDC_LTIME_CURRENT).EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_LTIME_PICKER).EnableWindow(FALSE);
			GetDlgItem(IDC_CLEAR_LTIME).EnableWindow(FALSE);
			GetDlgItem(IDC_LTIME_CURRENT).EnableWindow(TRUE);
		}
	}
}

void CFileAttrib::OnChangeCreationDate()
{
	BOOL bCheck = IsDlgButtonChecked(IDC_CTIME_CHECK);
	GetDlgItem(IDC_CTIME_SET).EnableWindow(bCheck);
	GetDlgItem(IDC_CTIME_TOUCH).EnableWindow(bCheck);
	GetDlgItem(IDC_CTIME_SAMELTIME).EnableWindow(bCheck);

	if (!bCheck) {
		GetDlgItem(IDC_CTIME_PICKER).EnableWindow(FALSE);
		GetDlgItem(IDC_CLEAR_CTIME).EnableWindow(FALSE);
		GetDlgItem(IDC_CTIME_CURRENT).EnableWindow(FALSE);
	}
	else {
		if (IsDlgButtonChecked(IDC_CTIME_SET)) {
			GetDlgItem(IDC_CTIME_PICKER).EnableWindow(TRUE);
			GetDlgItem(IDC_CLEAR_CTIME).EnableWindow(TRUE);
			GetDlgItem(IDC_CTIME_CURRENT).EnableWindow(FALSE);
		}
		else if (IsDlgButtonChecked(IDC_CTIME_TOUCH)) {
			GetDlgItem(IDC_CTIME_PICKER).EnableWindow(FALSE);
			GetDlgItem(IDC_CLEAR_CTIME).EnableWindow(FALSE);
			GetDlgItem(IDC_CTIME_CURRENT).EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_CTIME_PICKER).EnableWindow(FALSE);
			GetDlgItem(IDC_CLEAR_CTIME).EnableWindow(FALSE);
			GetDlgItem(IDC_CTIME_CURRENT).EnableWindow(FALSE);
		}
	}
}

void CFileAttrib::OnClearLTime()
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&m_ad.wfd.ftLastWriteTime, &st);
	switch (m_ltimeClear) {
		case 2: st.wHour = 0;		// fall
		case 1: st.wMinute = 0;		// fall
		case 0: st.wMilliseconds = 0; st.wSecond = 0; break;
		default: break;
	}
	CDateTimePickerCtrl dt = GetDlgItem(IDC_LTIME_PICKER);
	dt.SetSystemTime(GDT_VALID, &st);

	m_ltimeClear = (m_ltimeClear + 1) % 4;
	CString str; str.LoadString(IDS_CLEAR_SEC + m_ltimeClear);
	SetDlgItemText(IDC_CLEAR_LTIME, str.GetString());
}

void CFileAttrib::OnClearCTime()
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&m_ad.wfd.ftCreationTime, &st);
	switch (m_ctimeClear) {
		case 2: st.wHour = 0;		// fall
		case 1: st.wMinute = 0;		// fall
		case 0: st.wMilliseconds = 0; st.wSecond = 0; break;
		default: break;
	}
	CDateTimePickerCtrl dt = GetDlgItem(IDC_CTIME_PICKER);
	dt.SetSystemTime(GDT_VALID, &st);

	m_ctimeClear = (m_ctimeClear + 1) % 4;
	CString str; str.LoadString(IDS_CLEAR_SEC + m_ctimeClear);
	SetDlgItemText(IDC_CLEAR_CTIME, str.GetString());
}

/////////////////////////////////////////////////////////////////////////////
//

BOOL SetFileProperties(LPCTSTR pszFile, DWORD attr, FILETIME* pftLast, FILETIME* pftCreate)
{
	if (attr != (DWORD)-1) {
		if (!SetFileAttributes(pszFile, attr))
			return FALSE;
	}
	if (pftLast || pftCreate) {
		if (!SetFileDateTimes(pszFile, pftCreate, pftLast, NULL)) {
			return FALSE;
		}
	}
	SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, pszFile, NULL);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
//

class CChangeAttrib
{
public:
	HWND m_hWnd;
	CProgressDialog m_progress;
	CAttribData m_ad;
	CSimpleMap<CString, DWORD> m_results;

	CChangeAttrib() : m_hWnd(NULL) { }
	CChangeAttrib(HWND hwnd, CAttribData& ad) : m_hWnd(hwnd), m_ad(ad) { }
	~CChangeAttrib() { }

	int SetFileProps(LPCTSTR pszFile)
	{
		DWORD attr = (m_ad.flags & SFA_FILEATTRIBUTES) ? m_ad.wfd.dwFileAttributes : (DWORD)-1;
		FILETIME* ftc = (m_ad.flags & SFA_CREATIONTIME) ? &m_ad.wfd.ftCreationTime : NULL;
		FILETIME* ftw = (m_ad.flags & SFA_LASTWRITETIME) ? &m_ad.wfd.ftLastWriteTime : NULL;

		int ret = IDOK;
		while (1) {
			if (SetFileProperties(pszFile, attr, ftw, ftc))
				break;

			DWORD code = GetLastError();
			ret = ErrorBox(m_hWnd, code, MAKEINTRESOURCE(IDS_CHGATTR_TITLE), PathFindFileName(pszFile), MB_QABORT);
			if (ret != IDRETRY) {
				m_results.Add(pszFile, code);
				break;
			}
		}

		// IDOK, IDABORT, or IDIGNORE
		return ret;
	}

	BOOL TreeProp(LPCTSTR pszDir)
	{
		if (m_progress.HasUserCancelled()) return FALSE;

		m_progress.SetLine(2, pszDir, TRUE);

		DWORD attr = GetFileAttributes(pszDir);
		if (attr == 0xFFFFFFFF) return FALSE;

		// リパースポイントのチェック
		if (attr & FILE_ATTRIBUTE_REPARSE_POINT) {
			int res = AppMessage(IDS_CHGATTR_TITLE, IDS_QUERY_REPARSEPOINT, pszDir, MB_QUESTION);
			switch (res) {
				case IDYES: break;
				case IDNO: return TRUE;			// このフォルダをスキップ
				case IDCANCEL:
				default: return FALSE;			// 中止
			}
		}

		int ret = SetFileProps(pszDir);
		if (ret == IDABORT)
			return FALSE;
		else if (ret == IDIGNORE)
			return TRUE;

		// ファイルを検索
		CSimpleArray<CString> arrFile;
		CSimpleArray<CString> arrFolder;
		EnumFileDir(pszDir, gszAst, arrFile, arrFolder);

		TCHAR buf[MAX_PATH * 2];
		int i, cnt;

		// まずファイルを先に処理
		cnt = (int)arrFile.GetSize();
		for (i = 0; i < cnt; i++) {
			PathCombine(buf, pszDir, arrFile[i]);
			if (m_progress.HasUserCancelled()) return FALSE;

			m_progress.SetLine(3, PathFindFileName(buf), TRUE);
			ret = SetFileProps(buf);
			if (ret == IDABORT)
				return FALSE;
		}
		arrFile.RemoveAll();

		// 次にフォルダを処理
		cnt = (int)arrFolder.GetSize();
		for (i = 0; i < cnt; i++) {
			PathCombine(buf, pszDir, arrFolder[i]);
			if (!TreeProp(buf)) return FALSE;
		}
		arrFolder.RemoveAll();
		return TRUE;
	}

	void Run(HDROP hdrop)
	{
		m_progress.SetTitle(IDS_CHGATTR_TITLE);
		m_progress.SetMessage(MAKEINTRESOURCE(IDS_DO_CHGATTR));
		m_progress.SetOperation(SPACTION_APPLYINGATTRIBS);
		m_progress.StartProgressDialog(m_hWnd, PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR);

		CDropFiles df(hdrop);
		TCHAR buf[MAX_PATH * 2];
		CFileInfo fi;
		for (int i = 0; i < df.m_nEntry; i++) {
			df.DragQueryFile(i, buf, ARRAYSIZE(buf));
			if (GetFileInfo(buf, fi)) {
				if (fi.IsFolder() && m_ad.subdir) {
					if (!TreeProp(buf)) break;
				}
				else {
					if (m_progress.HasUserCancelled()) break;

					TCHAR dir[MAX_PATH * 2];
					PathGetParent(buf, dir);
					m_progress.SetLine(2, dir, TRUE);
					m_progress.SetLine(3, PathFindFileName(buf), TRUE);
					int ret = SetFileProps(buf);
					if (ret == IDABORT)
						break;
				}
			}
		}
		df.Detach();

		m_progress.StopProgressDialog();
	}
};

/////////////////////////////////////////////////////////////////////////////
// ErrorReportDlg

typedef struct tagCOLHEAD {
	LPTSTR pszText;
	int cx;
} COLHEAD, *LPCOLHEAD;

static COLHEAD cole[3] = {
	{ _T("ファイル名"), 100 },
	{ _T("場所"), 100 },
	{ _T("結果"), 240 }
};

class CErrorReportDlg : public CDialogImpl<CErrorReportDlg>, public CDialogResize<CErrorReportDlg>
{
public:
	CErrorReportDlg() { }
	~CErrorReportDlg() { }

	enum { IDD = IDD_ERRORREPORT };

	CListViewCtrl m_list;

BEGIN_MSG_MAP(CErrorReportDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CErrorReportDlg>)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CErrorReportDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnCancel() { EndDialog(IDCANCEL); }
	BOOL SetSyncResItem(IStream* ps);
};

BOOL CErrorReportDlg::OnInitDialog(LPARAM lParam)
{
	if (!lParam) {
		EndDialog(IDCANCEL);
		return TRUE;
	}
	CSimpleMap<CString, DWORD>* parr = (CSimpleMap<CString, DWORD>*)lParam;

	DlgResize_Init();
	CenterWindow();

	// イメージリスト
	m_list = GetDlgItem(IDC_LIST);
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	m_list.SetImageList(himS, LVSIL_SMALL);

	// カラム設定
	int i;
	for (i = 0; i < 3; i++)
		m_list.InsertColumn(i, cole[i].pszText, LVCFMT_LEFT, cole[i].cx, i);

	// 結果情報の表示
	LV_ITEM item;
	CString str;
	DWORD code;
	TCHAR buf[MAX_PATH * 2];
	int index;

	int cnt = parr->GetSize();
	for (i = 0; i < cnt; i++) {
		str = parr->GetKeyAt(i);
		code = parr->GetValueAt(i);

		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_IMAGE;
		item.pszText = PathFindFileName(str);
		item.iItem = i;

		SHFILEINFO sfi;
		SHGetFileInfo(str, 0, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		item.iImage = sfi.iIcon;

		index = m_list.InsertItem(&item);

		PathGetParent(str, buf);
		m_list.SetItemText(index, 1, buf);

		GetErrorString(code, str);
		m_list.SetItemText(index, 2, str);
	}

	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_list.SetColumnWidth(2, LVSCW_AUTOSIZE);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
//

__inline bool isHasFileTime(FILETIME& ft) { return (*((DWORDLONG*)&ft) != 0); }

extern "C"
BOOL WINAPI fopFileAttributes(HWND hwnd, HDROP hDrop)
{
	if (!hDrop) return FALSE;

	CFileAttrib dlg;
	CString path;
	TCHAR buf[MAX_PATH * 2];
	int cnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	int num = 0;
	CSimpleArray<WIN32_FIND_DATA> arr;
	BOOL hasDir = FALSE;
	for (int i = 0; i < cnt; i++) {
		DragQueryFile(hDrop, i, buf, ARRAYSIZE(buf));
		WIN32_FIND_DATA wfd;
		if (GetFileData(buf, &wfd)) {
			wfd.nFileSizeLow = num++;
			arr.Add(wfd);
			// サブフォルダあり
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				dlg.m_ad.subdir = TRUE;
			// 取得OKのパス名を記録
			path = buf;
		}
	}
	if (!num) return FALSE;

	dlg.m_ad.multifiles = (num > 1);

	dlg.m_ad.wfdarr[4] = arr[0];
	dlg.m_ad.wfdarr[5] = arr[num - 1];
	if (num > 1)
		qsort_ex(arr.GetData(), arr.GetSize(), sizeof(WIN32_FIND_DATA), compare_attr, (const void*)FASORT_LASTWRITE);
	dlg.m_ad.wfdarr[0] = arr[num - 1];
	dlg.m_ad.wfdarr[1] = arr[0];
	if (num > 1)
		qsort_ex(arr.GetData(), arr.GetSize(), sizeof(WIN32_FIND_DATA), compare_attr, (const void*)FASORT_CREATION);
	dlg.m_ad.wfdarr[2] = arr[num - 1];
	dlg.m_ad.wfdarr[3] = arr[0];
	arr.RemoveAll();

	// 対象ドライブのファイルシステム
	CDiskSpace ds(path, true);
	dlg.m_dosDateTime = (!ds.isNTFS() && !ds.isHPFS() && !ds.isEXFAT());

	if (dlg.DoModal(hwnd) == IDOK) {
		CChangeAttrib ca(hwnd, dlg.m_ad);
		ca.Run(hDrop);
		if (ca.m_results.GetSize() > 0) {
			// エラーを表示
			CErrorReportDlg dlg;
			dlg.DoModal(hwnd, (LPARAM)&ca.m_results);
		}
	}
	return TRUE;
}

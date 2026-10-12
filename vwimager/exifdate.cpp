//////////////////////////////////////////////////////////////////////
// converter.cpp

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "exifinfo.h"
#include "dropexec.h"

#define ISDIGITS(a)		('0'<=(a)&&(a)<='9')

void InitComboBoxList(HWND hcombo, UINT idResource)
{
	CComboBox combo = hcombo;
	CString str;
	str.LoadString(idResource);

	combo.ResetContent();
	if (!str.IsEmpty()) {
		LPTSTR p = (LPTSTR)(LPCTSTR)str;
		while (p && *p) {
			LPTSTR q = StrChr(p, L';');
			if (q) *q++ = 0;
			combo.AddString(p);
			p = q;
		}
	}
}

int sei2wa(int& year)
{
	if      (year < 1912) { year -= 1868; return 'M'; }
	else if (year < 1926) { year -= 1911; return 'T'; }
	else if (year < 1989) { year -= 1925; return 'S'; }
	else                  { year -= 1988; return 'H'; }
}

/////////////////////////////////////////////////////////////////////
//

class namemap
{
public:
	CString m_src;
	CString m_dst;
	SYSTEMTIME m_st;

	namemap()
	{
		m_src = _T("");
		m_dst = _T("");
	}

	bool set(LPCTSTR pszFile, SYSTEMTIME* pst, int index = 0)
	{
		if (!PathFileExists(pszFile) || !pst) return false;

		m_src = pszFile;
		m_st = *pst;
		return true;
	}

	void datetime(LPTSTR psz, int cchMax)
	{
//		wsprintf(psz, _T("%04d/%02d/%02d %02d:%02d:%02d"), m_st.wYear, m_st.wMonth, m_st.wDay, m_st.wHour, m_st.wMinute, m_st.wSecond);
		GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &m_st, NULL, psz, cchMax);
		int len = lstrlen(psz);
		psz[len++] = _T(' ');
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &m_st, NULL, psz + len, cchMax - len);
	}

	void typeformat(int index, int type, LPTSTR psz)
	{
		switch (type) {
		case 0:
			*psz = 0;
			break;
		case 1:				// yyyymmdd
			wsprintf(psz, _T("%04d%02d%02d"), m_st.wYear, m_st.wMonth, m_st.wDay);
			break;
		case 2:				// yymmdd
			wsprintf(psz, _T("%02d%02d%02d"), m_st.wYear % 100, m_st.wMonth, m_st.wDay);
			break;
		case 3:				// nnnmmdd
		{
			int year = m_st.wYear;
			TCHAR ch = (TCHAR)sei2wa(year);
			wsprintf(psz, _T("%c%02d%02d%02d"), ch, year, m_st.wMonth, m_st.wDay);
			break;
		}
		case 4:				// mmdd
			wsprintf(psz, _T("%02d%02d"), m_st.wMonth, m_st.wDay);
			break;
		case 5:				// HHMM
			wsprintf(psz, _T("%02d%02d"), m_st.wHour, m_st.wMinute);
			break;
		case 6:				// HHMMSS
			wsprintf(psz, _T("%02d%02d%02d"), m_st.wHour, m_st.wMinute, m_st.wSecond);
			break;
		case 7:				// 1桁連番
			wsprintf(psz, _T("%d"), index);
			break;
		case 8:				// 2桁連番
			wsprintf(psz, _T("%02d"), index);
			break;
		case 9:				// 3桁連番
			wsprintf(psz, _T("%03d"), index);
			break;
		case 10:			// 4桁連番
			wsprintf(psz, _T("%04d"), index);
			break;
		case 11:			// ドキュメントの名前
			lstrcpy(psz, PathFindFileName(m_src));
			PathRemoveExtension(psz);
			break;
		}
	}

	void buildname(int index, int part1, int part2, int part3, LPTSTR psz, LPCTSTR pszDstPath = NULL)
	{
		*psz = 0;
		m_dst = _T("");

		TCHAR name[MAX_PATH * 2], buf[MAX_PATH * 2];
		name[0] = 0;
		buf[0] = '_';
		LPTSTR p = &buf[1];
		typeformat(index, part1, p); if (*p) lstrcat(name, name[0] ? buf : p);
		typeformat(index, part2, p); if (*p) lstrcat(name, name[0] ? buf : p);
		typeformat(index, part3, p); if (*p) lstrcat(name, name[0] ? buf : p);

		if (name[0]) {
			LPTSTR p = PathFindExtension(m_src);
			lstrcat(name, p);

			if (pszDstPath && *pszDstPath) {
				lstrcpy(buf, pszDstPath);
			}
			else {
				lstrcpy(buf, m_src);
				PathRemoveFileSpec(buf);
			}
			PathAppend(buf, name);
			m_dst = buf;
			lstrcpy(psz, m_dst);
		}
	}

	bool validatetarget()
	{
		if (PathFileExists(m_dst)) {
			// 既に存在すれば、サフィックスをつけて再試行
			TCHAR buf[MAX_PATH * 2];
			for (int i = 1; i < 1000; i++) {
				lstrcpy(buf, m_dst);
				LPTSTR p = PathFindExtension(buf);
				CString ext = p;
				*p = 0;
				CString str;
				str.Format(_T("%s(%d)"), buf, i);
				if (!ext.IsEmpty())
					str += ext;

				if (!PathFileExists(str)) {
					m_dst = str;
					return true;
				}
			}
			return false;
		}
		return true;
	}

	bool rename()
	{
		if (m_src.IsEmpty() || m_dst.IsEmpty()) return false;

		// 変換後と同名ならば何もしない
		if (!m_dst.CompareNoCase(m_src)) return true;

		if (!validatetarget()) return false;

		return (MoveFileEx(m_src, m_dst, 0) != FALSE);
	}

	bool copy()
	{
		if (m_src.IsEmpty() || m_dst.IsEmpty()) return false;

		// 双方が同名ならば何もしない
		if (!m_dst.CompareNoCase(m_src)) return true;

		// コピー先のフォルダの確認
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_dst);
		PathRemoveFileSpec(buf);
		if (!PathIsDirectory(buf)) return false;

		if (!validatetarget()) return false;

		BOOL bCancel = false;
		return (CopyFileEx(m_src, m_dst, NULL, NULL, &bCancel, 0) != FALSE);
	}

	int groupid()
	{
		return (m_st.wYear * 1000 + m_st.wMonth * 100 + m_st.wDay);
	}
};

/////////////////////////////////////////////////////////////////////////////
// CExifDateDlg ダイアログ

typedef struct tagELHD {
	int cx;
	UINT ids;
} ELHD;

static ELHD sstHead[] = {
	200, IDS_HEADER_OLDNAME,
	130, IDS_HEADER_DATE,
	200, IDS_HEADER_NEWNAME,
};

class CExifDateDlg : public CDialogImpl<CExifDateDlg>, public CDialogResize<CExifDateDlg>
{
public:
	CExifDateDlg();

	enum { IDD = IDD_RENEXIFDATE };
	CListViewCtrl m_list;
	CComboBox m_combo1;
	CComboBox m_combo2;
	CComboBox m_combo3;
	CComboBox m_combor;

	int m_part1;
	int m_part2;
	int m_part3;
	int m_reset;
	HDROP m_hdrop;

BEGIN_MSG_MAP(CExifDateDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CExifDateDlg>)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_CBN_SELCHANGE(IDC_PART1, OnPart1SelChange)
	ON_CBN_SELCHANGE(IDC_PART2, OnPart2SelChange)
	ON_CBN_SELCHANGE(IDC_PART3, OnPart3SelChange)
	ON_CBN_SELCHANGE(IDC_RESET, OnResetSelChange)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST, OnDeleteItem)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CExifDateDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnPart1SelChange() { m_part1 = m_combo1.GetCurSel(); UpdateTargetNames(); }
	void OnPart2SelChange() { m_part2 = m_combo2.GetCurSel(); UpdateTargetNames(); }
	void OnPart3SelChange() { m_part3 = m_combo3.GetCurSel(); UpdateTargetNames(); }
	void OnResetSelChange() { m_reset = m_combor.GetCurSel(); UpdateTargetNames(); }
	void OnDeleteItem(LPNMHDR pnmh, LRESULT* pResult);
	void UpdateTargetNames();

	static int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareGroupProc(int id1, int id2, void* data);
};


CExifDateDlg::CExifDateDlg()
{
	m_part1 = m_part2 = m_part3 = m_reset = 0;
	m_hdrop = NULL;
}

BOOL CExifDateDlg::OnInitDialog(LPARAM lParam)
{
	CComPtr<IExifParser> pep;
	HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
	if (FAILED(hr)) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	if (!lParam) {
		EndDialog(IDCANCEL);
		return TRUE;
	}
	m_hdrop = (HDROP)lParam;
	int cnt = DragQueryFile(m_hdrop, 0xffffffff, NULL, 0);
	if (cnt < 1) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	DlgResize_Init();
	CenterWindow();

	m_list = GetDlgItem(IDC_LIST);
	m_list.EnableGroupView(TRUE);

	m_combo1 = GetDlgItem(IDC_PART1);
	m_combo2 = GetDlgItem(IDC_PART2);
	m_combo3 = GetDlgItem(IDC_PART3);
	m_combor = GetDlgItem(IDC_RESET);

	CString str;
	for (int i = IDS_DATE_NONE; i <= IDS_DATE_DOCUMENT; i++) {
		str.LoadString(i);
		m_combo1.AddString(str);
		m_combo2.AddString(str);
		m_combo3.AddString(str);
	}

	InitDlgCtrl(m_combor, IDS_MULTI_RESET);

	// 列の挿入
	HIMAGELIST hl, hs;
	Shell_GetImageLists(&hl, &hs);
	m_list.SetImageList(hs, LVSIL_SMALL);
	for (int i = 0; i < 3; i++) {
		str.LoadString(sstHead[i].ids);
		m_list.InsertColumn(i, str.GetString(), LVCFMT_LEFT, sstHead[i].cx, i);
	}
	int index;
	LVITEM item;
	SHFILEINFO sfi;
	TCHAR buf[MAX_PATH * 2], date[MAX_PATH];
	EXIFINFOSTRUCT info;
	namemap* pn;
	for (int i = 0; i < cnt; i++) {
		DragQueryFile(m_hdrop, i, buf, _COUNTOF(buf));

		memset(&info, 0, sizeof(info));
		if (pep->GetExifInfo(buf, NULL, NULL, FALSE, (BYTE*)&info) != S_OK) continue;

		pn = new namemap();
		if (pn) {
			int num = m_list.GetItemCount();
			if (pn->set(buf, &info.DateTime, num + 1)) {

				int id = pn->groupid();
				if (!m_list.HasGroup(id)) {
					GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &info.DateTime, NULL, date, _COUNTOF(date));

					LVGROUP group;
					memset(&group, 0, sizeof(group));
#ifdef _WIN64
					group.cbSize = sizeof(group);
#else
					group.cbSize = OsVISTA() ? sizeof(group) : LVGROUP_V5_SIZE;
#endif
					group.mask = LVGF_ALIGN | LVGF_GROUPID | LVGF_HEADER;
					group.pszHeader = date;
					group.iGroupId = id;
					group.state = LVGS_NORMAL;
					group.uAlign = LVGA_HEADER_LEFT;

					m_list.InsertGroup(-1, &group);
				}

				memset(&sfi, 0, sizeof(sfi));
				SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);

				memset(&item, 0, sizeof(item));
				item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
				item.iItem = i;
				item.pszText = PathFindFileName(buf);
				item.iImage = sfi.iIcon;
				item.lParam = (LPARAM)pn;
				item.mask |= LVIF_GROUPID;
				item.iGroupId = id;
				index = m_list.InsertItem(&item);

				pn->datetime(buf, _COUNTOF(buf));
				m_list.SetItemText(index, 1, buf);
			}
			else
				delete pn;
		}
	}

	if (m_list.GetItemCount() < 1) {
		AtlMessageBox(m_hWnd, IDS_NOEXIFDATE, IDS_TITLE_EXIF, MB_STOP);
		EndDialog(IDCANCEL);
	}

	// 撮影日時でソート
	m_list.SortItems(CompareProc, (DWORD_PTR)&m_list);
	m_list.SortGroups(CompareGroupProc);

	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);

	CAppReg reg;
	DWORD val = (DWORD)reg.GetValue(IDS_SEC_EXIF, IDS_KEY_RENAME, 0);
	if (val) {
		m_part1 = (int)LOBYTE(LOWORD(val));
		m_part2 = (int)HIBYTE(LOWORD(val));
		m_part3 = (int)LOBYTE(HIWORD(val));
		m_reset = (int)HIBYTE(HIWORD(val));
		m_part1 = min(IDS_DATE_4SERIAL - IDS_DATE_NONE, max(0, m_part1));
		m_part2 = min(IDS_DATE_4SERIAL - IDS_DATE_NONE, max(0, m_part2));
		m_part3 = min(IDS_DATE_4SERIAL - IDS_DATE_NONE, max(0, m_part3));
		m_reset = min(3, max(0, m_reset));
	}
	m_combo1.SetCurSel(m_part1);
	m_combo2.SetCurSel(m_part2);
	m_combo3.SetCurSel(m_part3);
	m_combor.SetCurSel(m_reset);

	UpdateTargetNames();
	return TRUE;
}

// Sort the item in reverse alphabetical order.
int CALLBACK CExifDateDlg::CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListViewCtrl* plist = (CListViewCtrl*)lParamSort;
	namemap* pi1 = (namemap*)lParam1;
	namemap* pi2 = (namemap*)lParam2;

	FILETIME ft1, ft2;
	SystemTimeToFileTime(&pi1->m_st, &ft1);
	SystemTimeToFileTime(&pi2->m_st, &ft2);

	int r = (int)CompareFileTime(&ft1, &ft2);
	if (!r) {
		LPCTSTR p1 = PathFindFileName(pi1->m_src);
		LPCTSTR p2 = PathFindFileName(pi2->m_src);
		r = StrCmpLogicalW(PathFindFileName(p1), PathFindFileName(p2));
	}
	return r;
}

int CALLBACK CExifDateDlg::CompareGroupProc(int id1, int id2, void* data)
{
	return (id1 - id2);
}

void CExifDateDlg::OnDeleteItem(LPNMHDR pnmh, LRESULT* pResult)
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pnmh;
	namemap* pn = (namemap*)m_list.GetItemData(pnmlv->iItem);
	if (pn) delete pn;
}

void CExifDateDlg::UpdateTargetNames()
{
	SYSTEMTIME stLast = { 0 };
	TCHAR buf[MAX_PATH * 2];
	int index = 1;
	int cnt = m_list.GetItemCount();
	for (int i = 0; i < cnt; i++) {
		namemap* pn = (namemap*)m_list.GetItemData(i);
		if (pn) {
			switch (m_reset) {
			case 1:
				if (stLast.wDay != pn->m_st.wDay) index = 1;
				break;
			case 2:
				if (stLast.wMonth != pn->m_st.wMonth) index = 1;
				break;
			case 3:
				if (stLast.wYear != pn->m_st.wYear) index = 1;
				break;
			}
			stLast = pn->m_st;

			pn->buildname(index++, m_part1, m_part2, m_part3, buf);
			m_list.SetItemText(i, 2, PathFindFileName(buf));
		}
	}
}


void CExifDateDlg::OnOK()
{
	m_part1 = m_combo1.GetCurSel();
	m_part2 = m_combo2.GetCurSel();
	m_part3 = m_combo3.GetCurSel();
	m_reset = m_combor.GetCurSel();

	if (m_part1 <= 0 && m_part2 <= 0 && m_part3 <= 0) {
		MessageBeep(0);
		m_combo1.SetFocus();
		return;
	}

	if (AtlMessageBox(m_hWnd, IDS_QUERY_EXIFDATE, IDS_TITLE_EXIF, MB_QYESNO) != IDYES)
		return;

	// 進捗ダイアログ
	IProgressDialog* ppd;
	HRESULT hr = CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (LPVOID*)&ppd);
	if (SUCCEEDED(hr)) {
		CString str;
		str.LoadString(IDS_TITLE_EXIF);
		ppd->SetTitle(str);

		str.LoadString(IDS_PROGRESS_EXIFDATE);
		ppd->SetLine(1, str, FALSE, NULL);
		ppd->SetAnimation(GetModuleHandle(DLL_FOPSRV), 165);
		ppd->StartProgressDialog(m_hWnd, NULL, PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR, NULL);
	}
	else
		ppd = NULL;

	int cnt = m_list.GetItemCount();
	for (int i = 0; i < cnt; i++) {
		namemap* pn = (namemap*)m_list.GetItemData(i);
		if (pn) {
			pn->rename();
		}
	}

	if (ppd) {
		ppd->StopProgressDialog();
		ppd->Release();
	}

	CAppReg reg;
	DWORD val = MAKELONG(MAKEWORD(m_part1, m_part2), MAKEWORD(m_part3, m_reset));
	reg.SetValue(IDS_SEC_EXIF, IDS_KEY_RENAME, val);
	EndDialog(IDOK);
}

////////////////////////////////////////////////////////////////////////
//

static BOOL RenameToExifDateCore(HWND hwnd, HDROP hdrop)
{
	CExifDateDlg dlg;
	return (dlg.DoModal(hwnd, (LPARAM)hdrop) == IDOK);
}

extern "C"
BOOL WINAPI vmiRenameToExifDate(HWND hwnd, HDROP hdrop)
{
	return FALSE;
}

extern "C"
void WINAPI RunRenameToExifDateW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	CRegExec re;
	if (re.Initialize(lpCmdLine) && re.m_hmem)
		RenameToExifDateCore(hwnd, (HDROP)re.m_hmem);
}

extern "C"
void WINAPI RunRenameToExifDate(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR cmd[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, _COUNTOF(cmd));
	RunRenameToExifDateW(hwnd, hInst, cmd, nCmdShow);
}


////////////////////////////////////////////////////////////////////////
//

class CPickoutDlg : public CDialogImpl<CPickoutDlg>
{
public:
	CPickoutDlg() { m_path = _T(""); m_primaryimage = true; m_targetpath_priority = false; }

	enum { IDD = IDD_PICKOUTTHUMB };
	CString m_path;
	bool m_primaryimage;
	bool m_targetpath_priority;

BEGIN_MSG_MAP(CPickoutDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(SEN_GETTEXT, IDC_PATH, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
};

BOOL CPickoutDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	TCHAR buf[MAX_PATH * 2];
	if (!m_targetpath_priority) {
		CAppReg reg;
		if (reg.GetText(IDS_SEC_EXIF, IDS_KEY_PICKOUTPATH, buf, ARRAYSIZE(buf)) > 0)
			m_path = buf;
	}
	if (!m_path.IsEmpty())
		SetDlgItemText(IDC_PATH, m_path);
	CheckDlgButton(m_primaryimage ? IDC_MAXSIZE : IDC_MINSIZE, TRUE);

	// オートコンプリート
	SHAutoComplete(GetDlgItem(IDC_PATH), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);
	return TRUE;
}

void CPickoutDlg::OnOK()
{
	TCHAR buf[MAX_PATH * 2];
	CEdit edit = GetDlgItem(IDC_PATH);
	if (edit.GetWindowText(buf, _COUNTOF(buf)) < 1) {
		MessageBeep(0);
		edit.SetFocus();
		return;
	}

	if (!PathIsDirectory(buf)) {
		if (PathFileExists(buf)) {
			MessageBeep(0);
			edit.SetFocus();
			return;
		}
	}
	m_path = buf;
	m_primaryimage = (IsDlgButtonChecked(IDC_MAXSIZE) != FALSE);

	CAppReg reg;
	reg.SetText(IDS_SEC_EXIF, IDS_KEY_PICKOUTPATH, m_path);
	EndDialog(IDOK);
}

void CPickoutDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (fopBrowseForFolder(m_hWnd, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////
//

static int PickoutThumbnailCore(HWND hwnd, HDROP hdrop, LPCTSTR pszTargetPath)
{
	CComPtr<IExifParser> pep;
	HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
	if (FAILED(hr)) return -1;

	TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
	if (!hdrop || DragQueryFile(hdrop, 0, src, _COUNTOF(src)) < 1) return FALSE;

	CPickoutDlg dlg;
	if (!pszTargetPath || *pszTargetPath == 0) {
		PathRemoveFileSpec(src);
		dlg.m_path = src;
	}
	else {
		dlg.m_targetpath_priority = true;
		dlg.m_path = pszTargetPath;
	}

	if (dlg.DoModal(hwnd) != IDOK) return -1;
	if (!_pathmaketree(dlg.m_path)) return -1;

	int success = 0;
	CWaitCursor wc;
	int cnt = DragQueryFile(hdrop, 0xffffffff, NULL, 0);
	for (int i = 0; i < cnt; i++) {
		DragQueryFile(hdrop, i, src, _COUNTOF(src));

		IStream* pstrm;
		if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
			if (pep->GetThumbsnail(src, pstrm, NULL, dlg.m_primaryimage, NULL, 0) == S_OK) {
				HGLOBAL hmem;
				if (GetHGlobalFromStream(pstrm, &hmem) == S_OK) {
					lstrcpy(dst, dlg.m_path);
					PathAppend(dst, PathFindFileName(src));
					PathRemoveExtension(dst);
					lstrcat(dst, dlg.m_primaryimage ? _T(".thm(L)") : _T(".thm(S)"));

					WORD* data = (WORD*)GlobalLock(hmem);
					switch (*data) {
						case 0xd8ff: lstrcat(dst, _T(".jpg")); break;
						case 0x4d42: lstrcat(dst, _T(".bmp")); break;
						default: lstrcat(dst, _T(".thumb")); break;
					}

					HANDLE hf = CreateFile(dst, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_ARCHIVE, NULL);
					if (hf != INVALID_HANDLE_VALUE) {
						DWORD done;
						WriteFile(hf, data, (DWORD)GlobalSize(hmem), &done, NULL);
						CloseHandle(hf);
						success++;
					}
					GlobalUnlock(hmem);
				}
			}
			pstrm->Release();
		}
	}

	return success;
}

// defuncd

extern "C" int WINAPI vmiPickoutThumbnail2(HWND hwnd, HDROP hdrop, LPCTSTR pszTargetPath) { return NULL; }
extern "C" BOOL WINAPI vmiPickoutThumbnail(HWND hwnd, HDROP hdrop) { return NULL; }

extern "C"
void WINAPI RunPickoutThumbnailW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	CRegExec re;
	if (re.Initialize(lpCmdLine) && re.m_hmem)
		PickoutThumbnailCore(hwnd, (HDROP)re.m_hmem, re.m_path);
}

extern "C"
void WINAPI RunPickoutThumbnail(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR cmd[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, _COUNTOF(cmd));
	RunPickoutThumbnailW(hwnd, hInst, cmd, nCmdShow);
}


// Merge.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"
#include "copy.h"

BOOL MergeCopy(LPCTSTR pSrc, LPTSTR pDst, BOOL bCutEof, CCopyProgressDlg* progress);


///////////////////////////////////////////////////////////////////////
// 
//

class CMergeData : public CFileArray
{
public:
	CMergeData();
	~CMergeData() {}

	TCHAR m_szName[MAX_PATH * 2];
	BOOL m_bCutEof;
	DWORDLONG m_dlTotal;
	BOOL m_bQueryNext;
};

CMergeData::CMergeData()
{
	m_szName[0] = '\0';
	m_bCutEof = FALSE;
	m_dlTotal = 0;
	m_bQueryNext = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// MergeDlgFn message handlers

typedef struct tagLISTHEAD {
	LPTSTR pszText;
	int fmt;
} LISTHEAD;
#define NUM_COLUMNS		3
static LISTHEAD lhEntry[NUM_COLUMNS] = {
	{ _T("名前"), HDF_LEFT | HDF_STRING }, 
	{ _T("サイズ"), HDF_RIGHT | HDF_STRING },
	{ _T("更新日時"), HDF_LEFT | HDF_STRING }
};

class CMergeDlg : public CDialogImpl<CMergeDlg>
{
public:
	CMergeDlg() { m_pmd = NULL; }
	~CMergeDlg() { }

	enum { IDD = IDD_MERGE };

BEGIN_MSG_MAP(CMergeDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, OnNotifyKeyDown)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_NOTIFY(SEN_GETTEXT, IDC_EDIT, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnNotifyKeyDown(LPNMHDR pnmh, LRESULT* pResult);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnMoveUp();
	void OnMoveDown();
	void OnAdd();
	void OnDelete();
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	int InsertItem(int nInsertAfter, CFileInfo* pf);

	CMergeData* m_pmd;
};

BOOL CMergeDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	m_pmd = (CMergeData*)lParam;

	CListViewCtrl list(GetDlgItem(IDC_LIST));
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	list.SetImageList(himS, LVSIL_SMALL);

	// 列の挿入
	int i;
	for(i = 0; i < NUM_COLUMNS; i++)
		list.InsertColumn(i, lhEntry[i].pszText, lhEntry[i].fmt, 40, i);

	CFileInfo* pf;
	for (i = 0; i < (int)m_pmd->GetCount(); i++) {
		pf = (*m_pmd)[i];
		InsertItem(-1, pf);
	}

	if (list.GetItemCount() < 1)
		EndDialog(IDCANCEL);
	else {
		list.SetColumnWidth(0, LVSCW_AUTOSIZE);
		list.SetColumnWidth(1, LVSCW_AUTOSIZE);
		list.SetColumnWidth(2, LVSCW_AUTOSIZE);
	}
	return TRUE;
}

BOOL CMergeDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

int CMergeDlg::InsertItem(int nInsertAfter, CFileInfo* pf)
{
	CListViewCtrl list(GetDlgItem(IDC_LIST));
	if (nInsertAfter < 0)
		nInsertAfter = list.GetItemCount();

	SHFILEINFO sfi;
	SHGetFileInfo(pf->m_strFullPathName, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	int index = list.InsertItem(nInsertAfter, pf->m_pszName, sfi.iIcon);
	list.SetItemData(index, (DWORD_PTR)pf);

	TCHAR buf[80];
	pf->FormatSize(buf, ARRAYSIZE(buf));
	list.SetItemText(index, 1, buf);

	pf->FormatDateTime(buf, ARRAYSIZE(buf));
	list.SetItemText(index, 2, buf);
	return index;
}

void CMergeDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2], dir[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (buf[0])
		dir[0] = 0;
	else
		GetCurrentDirectory(ARRAYSIZE(dir), dir);
	if (FileBrowse(TRUE, m_hWnd, IDS_TITLE_MERGE, IDS_FILTER_ALL, dir, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CMergeDlg::OnOK()
{
	// 連結して作成するファイル名の取得
	TCHAR buf[MAX_PATH * 2];
	if (GetDlgItemText(IDC_EDIT, buf, ARRAYSIZE(buf)) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}

	if (!StrRChr(buf, NULL, '\\')) {
		// パス名が含まれていなければ、現在のフォルダを指定する
		GetCurrentDirectory(ARRAYSIZE(m_pmd->m_szName), m_pmd->m_szName);
		PathAppend(m_pmd->m_szName, buf);
	}
	else
		lstrcpy(m_pmd->m_szName, buf);

	// ファイル名の妥当性
	if (!validate_makefile(m_pmd->m_szName)) {
		AppMessage(IDS_TITLE_MERGE, IDS_INVALIDFILENAME, m_pmd->m_szName);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}

	// 連結先ファイルが既に存在する場合には、連結確認をおこなう
	if (_pathfileexists(m_pmd->m_szName)) {
		int res = AppMessage(IDS_TITLE_MERGE, IDS_MERGE_FILEEXIST, m_pmd->m_szName, MB_QUESTION);
		switch (res) {
		case IDYES:
			break;
		case IDNO:
			fopRemoveFileNoConfirm(m_pmd->m_szName);
			break;
		default:
			return;
		}
	}

	// 連結するファイルのリストをピックアップ
	m_pmd->RemoveAll();
	m_pmd->m_dlTotal = 0;
	CListViewCtrl list(GetDlgItem(IDC_LIST));
	int cnt = list.GetItemCount();
	if (cnt > 0) {
		WIN32_FILE_ATTRIBUTE_DATA wfad;
		for (int i = 0; i < cnt; i++) {
			CFileInfo* pf = (CFileInfo*)list.GetItemData(i);

			// 作成ファイルと同一のファイルが無いかチェック
			if (!StrCmpI(pf->m_strFullPathName, m_pmd->m_szName)) {
				AppMessage(IDS_TITLE_MERGE, IDS_SAMEFILENAME);
				return;
			}
			m_pmd->Add(pf);

			GetFileAttributesEx(pf->m_strFullPathName, GetFileExInfoStandard, &wfad);
			DWORDLONG dl;
			SETDWORDLONG(dl, wfad.nFileSizeLow, wfad.nFileSizeHigh);
			m_pmd->m_dlTotal += dl;
		}

		// 残容量のチェック
		CDiskSpace di(m_pmd->m_szName, true);
		if (!di.isSpaceAvailable(m_pmd->m_dlTotal)) {
			AppMessage(IDS_TITLE_MERGE, IDS_DISKFULL);
			return;
		}
	}

	// 連結先ファイルが存在しなければ、サイズ０でファイルを作成
	if (!_pathfileexists(m_pmd->m_szName)) {
		CFileHandle file;
		file.CreateFile(m_pmd->m_szName, GENERIC_READ | GENERIC_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_ARCHIVE);
		if (file.IsInvalid()) {
			AppMessage(IDS_TITLE_MERGE, IDS_FILE_CREATEERROR, m_pmd->m_szName, MB_STOP);
			return;
		}
		file.CloseHandle();
	}

	m_pmd->m_bCutEof = IsDlgButtonChecked(IDC_CUTEOF);
	m_pmd->m_bQueryNext = IsDlgButtonChecked(IDC_QUERYNEXT);
	EndDialog(IDOK);
}

void CMergeDlg::OnMoveUp()
{
	CListViewCtrl list(GetDlgItem(IDC_LIST));
	int sel = list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (sel > 0) {
		CFileInfo* pf = (CFileInfo*)list.GetItemData(sel);
		list.DeleteItem(sel);

		sel = InsertItem(sel - 1, pf);
		list.SetItemState(sel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void CMergeDlg::OnMoveDown()
{
	CListViewCtrl list(GetDlgItem(IDC_LIST));
	int sel = list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	int cnt = list.GetItemCount();
	if (sel != -1 && sel < cnt - 1) {
		CFileInfo* pf = (CFileInfo*)list.GetItemData(sel);
		list.DeleteItem(sel);
		sel = InsertItem(sel + 1, pf);
		list.SetItemState(sel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void CMergeDlg::OnAdd()
{
	TCHAR buf[MAX_PATH * 2];
	buf[0] = '\0';
	if (FileBrowse(TRUE, m_hWnd, IDS_TITLE_MERGE, IDS_FILTER_ALL, NULL, buf)) {
		CFileInfo* pf = new CFileInfo;
		GetFileInfo(buf, *pf);
		int nIndex = InsertItem(-1, pf);
	}
}

void CMergeDlg::OnDelete()
{
	CListViewCtrl list(GetDlgItem(IDC_LIST));
	int sel = list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (sel != -1) {
		CFileInfo* pf = (CFileInfo*)list.GetItemData(sel);
		list.DeleteItem(sel);
		delete pf;
	}
}

void CMergeDlg::OnNotifyKeyDown(LPNMHDR pnmh, LRESULT* pResult)
{
	LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pnmh;
	if (GetKeyState(VK_CONTROL) < 0) {
		if (plvkd->wVKey == VK_UP)
			OnMoveUp();
		else if (plvkd->wVKey == VK_DOWN)
			OnMoveDown();
	}
	else {
		if (plvkd->wVKey == VK_DELETE)
			OnDelete();
	}
}

//////////////////////////////////////////////////////////////////////
//

class CMergeNextDlg : public CDialogImpl<CMergeNextDlg>
{
public:
	CMergeNextDlg() { m_strFile.Empty(); }
	~CMergeNextDlg() { }

	enum { IDD = IDD_MERGENEXT };

BEGIN_MSG_MAP(CMergeNextDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(SEN_GETTEXT, IDC_EDIT, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);

	CString m_strFile;
};

BOOL CMergeNextDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	SetDlgItemText(IDC_EDIT, m_strFile);
	return TRUE;
}

BOOL CMergeNextDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CMergeNextDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2], dir[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (buf[0])
		dir[0] = 0;
	else
		GetCurrentDirectory(ARRAYSIZE(dir), dir);
	if (FileBrowse(TRUE, m_hWnd, IDS_TITLE_MERGE, IDS_FILTER_ALL, dir, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CMergeNextDlg::OnOK()
{
	TCHAR buf[MAX_PATH * 2];
	if (GetDlgItemText(IDC_EDIT, buf, ARRAYSIZE(buf)) > 0) {
		WIN32_FIND_DATA wfd;
		if (GetFileData(buf, &wfd)) {
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				m_strFile = buf;
				EndDialog(IDOK);
				return;
			}
			else
				MessageBeep(0);
		}
		else
			LastError(IDS_TITLE_MERGE, buf, MB_STOP);
	}
	else
		MessageBeep(0);

	GetDlgItem(IDC_EDIT).SetFocus();
}


//////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopMergeFile(HWND hwnd, HDROP hDrop)
{
	if (!hDrop)
		return FALSE;

	CMergeData md;

	// フォルダ以外のファイルを収集する
	int i, cnt, entry = 0;
	CFileInfo* pf;
	TCHAR buf[MAX_PATH * 2];
	if (hDrop) {
		cnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		for (i = 0; i < cnt; i++) {
			DragQueryFile(hDrop, i, buf, ARRAYSIZE(buf));
			pf = new CFileInfo();
			if (!GetFileInfo(buf, *pf) || pf->IsFolder()) {
				delete pf;
				continue;
			}
			md.Add(pf);
			md.m_dlTotal += pf->m_dlSize;
			entry++;
		}
	}
	if (entry < 1)
		return FALSE;

	pf = (CFileInfo*)md[0];
	PathRemoveFileSpec(pf->GetFullPathName(buf, ARRAYSIZE(buf)));
	CEnterFolder ef(buf);

	// 作成する連結先ファイルを問い合わせる
	CMergeDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&md) != IDOK)
		return FALSE;

	// ファイルを連結する
	CCopyProgressDlg gage;
	gage.Start(hwnd, IDS_DO_MERGE, md.m_dlTotal);
	CFileInfo fi;

	// まず配列に格納されているファイル群を連結
	BOOL bOK = TRUE;
	buf[0] = '\0';
	CWin32FindData wfd;
	for (i = 0; i < (int)md.GetCount(); i++) {
		pf = md[i];
		pf->GetFullPathName(buf, ARRAYSIZE(buf));
		GetFileData(buf, wfd);
		gage.SetFile(wfd, buf);
		bOK = MergeCopy(buf, md.m_szName, md.m_bCutEof, &gage);
		if (!bOK) {
			ErrorBox(hwnd, GetLastError(), MAKEINTRESOURCE(IDS_TITLE_MERGE), NULL, MB_STOP);
			gage.Close();
			fopRemoveFileNoConfirm(md.m_szName);
			return FALSE;
		}
	}

	if (bOK) {
		// 次に問い合わせ毎に連結
		while (md.m_bQueryNext) {
			MessageBeep(0);
			CMergeNextDlg dlg;
			dlg.m_strFile = buf;
			if (dlg.DoModal(hwnd) != IDOK)
				break;

			GetFileData(dlg.m_strFile, wfd);
			gage.SetFile(wfd, dlg.m_strFile);
			if (!MergeCopy(dlg.m_strFile, md.m_szName, md.m_bCutEof, &gage)) {
				ErrorBox(hwnd, GetLastError(), MAKEINTRESOURCE(IDS_TITLE_MERGE), NULL, MB_STOP);
				break;
			}
		}
	}
	gage.Close();

	return bOK;
}

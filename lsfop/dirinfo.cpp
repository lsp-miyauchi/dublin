// dirinfo.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"

/////////////////////////////////////////////////////////////////////////////
//

static void ColonCutOff(LPTSTR pszText)
{
	int len = lstrlen(pszText);
	if (len > 0 && pszText[len - 1] == ':')
		pszText[len - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
//

class CDirItem
{
public:
	CDirItem(LPCTSTR pName, CDiskSpace* pDiskSpace);
	~CDirItem() {}

	bool isFolder() { return m_bFolder; }
	void AddFileInfo(CFileInfo* pf);

	CString m_strName;
	bool m_bFolder;
	DWORDLONG m_dlTotalLogicalSize;
	DWORDLONG m_dlTotalRealSize;
	int m_numDir;
	int m_numFiles;
	DWORD m_dwTotalClusters;
	CDiskSpace* m_pDiskSpace;
};

CDirItem::CDirItem(LPCTSTR pName, CDiskSpace* pDiskSpace)
{
	m_strName = pName;
	m_bFolder = (_pathisdirectory(pName) != FALSE);
	m_dlTotalLogicalSize = 0;
	m_dlTotalRealSize = 0;
	m_numDir = 0;
	m_numFiles = 0;
	m_dwTotalClusters = 0;
	m_pDiskSpace = pDiskSpace;
}

void CDirItem::AddFileInfo(CFileInfo* pf)
{
	m_numFiles++;
	m_dlTotalLogicalSize += pf->m_dlSize;

	DWORDLONG dl = pf->m_dlSize;
	if (pf->IsAttrib(FILE_ATTRIBUTE_COMPRESSED)) {
		DWORD hi, lo;
		lo = GetCompressedFileSize(pf->m_strFullPathName, &hi);
		if (lo != 0xffffffff || GetLastError() == NO_ERROR) {
			SETDWORDLONG(dl, lo, hi);
			m_dlTotalRealSize += dl;
		}
		else
			m_dlTotalRealSize += pf->m_dlSize;
	}
	else
		m_dlTotalRealSize += pf->m_dlSize;
	m_dwTotalClusters += (DWORD)m_pDiskSpace->ToClusters(dl);
}


/////////////////////////////////////////////////////////////////////////////
//

class CDirInfo : public CDiskSpace
{
public:
	CString m_strFilter;
	BOOL m_bSearching;
	BOOL m_bCanceled;
	BOOL m_bHasFilter;
	BOOL m_bUnmatchFilter;
	BOOL m_bUseRegExp;
	int m_numDir;
	int m_numFiles;
	DWORDLONG m_dlTotalLogicalSize;
	DWORDLONG m_dlTotalRealSize;
	DWORD m_dwTotalClusters;
//	CDiskSpace m_diskspace;
	CSimpleArray<CDirItem*> m_arrItem;
	CSimpleArray<CString> m_arrTargets;
	CDirItem* m_pItem;			// 処理中のアイテム
	BOOL m_bSortDown;			// 降順ソート

public:
	CDirInfo()
	{
		_init_member();
	}
	CDirInfo(LPCTSTR pszPath)
	{
		_init_member();
		AddTarget(pszPath);
	}
	CDirInfo(HDROP hdrop)
	{
		_init_member();
		AddTarget(hdrop);
	}
	~CDirInfo()
	{
		ResetItem();
	}
	void _init_member()
	{
		m_bSearching = FALSE;
		m_bCanceled = FALSE;
		m_numDir = 0;
		m_numFiles = 0;
		m_dlTotalLogicalSize = 0;
		m_dlTotalRealSize = 0;
		m_dwTotalClusters = 0;
		m_bHasFilter = FALSE;
		m_bUnmatchFilter = FALSE;
		m_bUseRegExp = FALSE;
		m_bSortDown = true;
	}
	void ResetItem()
	{
		_init_member();
		int cnt = m_arrItem.GetSize();
		for (int i = 0; i < cnt; i++)
			delete m_arrItem[i];
		m_arrItem.RemoveAll();
	}
	void ResetTargets()
	{
		m_arrTargets.RemoveAll();
	}
	bool AddTarget(LPCTSTR pszPathFile)
	{
		if (_pathfileexists(pszPathFile)) {
			m_arrTargets.Add(pszPathFile);
			return true;
		}
		return false;
	}
	int AddTarget(HDROP hdrop)
	{
		int num = 0;
		TCHAR buf[MAX_PATH * 2];
		int cnt = ::DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
		for (int i = 0; i < cnt; i++) {
			::DragQueryFile(hdrop, i, buf, ARRAYSIZE(buf));
			if (AddTarget(buf))
				num++;
		}
		return num;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CAssociateDlg dialog

class CDirInfoDlg : public CDialogImpl<CDirInfoDlg>
{
public:
	CDirInfoDlg() { }
	~CDirInfoDlg() { }

	enum { IDD = IDD_DIRINFO };

	CDirInfo m_di;
	CListViewCtrl m_list;
	CComboBox m_combo;

BEGIN_MSG_MAP(CDirInfoDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST, OnColumnClick)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblClick)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_NOTIFY(SEN_GETTEXT, IDC_PATH, OnNotifyBrowse)
	ON_CBN_SELCHANGE(IDC_FILTERING_TYPE, OnCheckFilter)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnColumnClick(LPNMHDR pnmh, LRESULT* pResult);
	void OnDblClick(LPNMHDR pnmh, LRESULT* pResult);
	void OnOK();
	void OnCancel();
	void OnCheckFilter();
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	BOOL TreeWalk(LPCTSTR pszWork);
	void SetSearchCtrl(BOOL bSearching);
	void ShowStatus();
	void InsertItem(int insert, CDirItem* pItem);
	BOOL ZapMessage();
};

int CALLBACK di_SortCompare(LPARAM param1, LPARAM param2, LPARAM pdata)
{
	int r;
	CDirItem* p1 = (CDirItem*)param1;
	CDirItem* p2 = (CDirItem*)param2;
	switch (HIWORD(pdata)) {
		case 0:			// name
			if (p1->isFolder()) {
				if (!p2->isFolder())
					r = -1;
				else
					r = p1->m_strName.Compare(p2->m_strName);
			}
			else if (p2->isFolder())
				r = 1;
			else
				r = p1->m_strName.Compare(p2->m_strName);
			break;

		case 1:			// num folders;
			r = p1->m_numDir - p2->m_numDir;
			break;

		case 2:			// num files;
			r = p1->m_numFiles - p2->m_numFiles;
			break;

		case 3:			// total size;
			r = (p1->m_dlTotalLogicalSize < p2->m_dlTotalLogicalSize) ? -1 : ((p1->m_dlTotalLogicalSize > p2->m_dlTotalLogicalSize) ? 1 : 0);
			break;

		case 4:			// total cluster size;
			r = (p1->m_dwTotalClusters < p2->m_dwTotalClusters) ? -1 : ((p1->m_dwTotalClusters > p2->m_dwTotalClusters) ? 1 : 0);
			break;
	}

	if (!LOWORD(pdata))
		r *= -1;
	return r;
}

BOOL CDirInfoDlg::OnInitDialog(LPARAM)
{
	CenterWindow();
	m_list = GetDlgItem(IDC_LIST);
	m_combo = GetDlgItem(IDC_FILTERING_TYPE);

	int cnt = m_di.m_arrTargets.GetSize();
	if (cnt < 1) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	CString str = PathFindFileName(m_di.m_arrTargets[0]);
	if (cnt > 1) {
		CString tmp1 = PathFindFileName(m_di.m_arrTargets[0]);
		CString tmp2 = PathFindFileName(m_di.m_arrTargets[1]);
		if (cnt > 2)
			str.Format(IDS_NUMOF_FILES, tmp1.GetString(), tmp2.GetString(), cnt);
		else
			str.Format(_T("%s, %s"), tmp1.GetString(), tmp2.GetString());
	}
	SetDlgItemText(IDC_PATH, str);

	InitDlgCtrl(m_combo, MAKEINTRESOURCE(IDS_FILTER_TYPES));
	m_combo.SetCurSel(0);

	OnCheckFilter();
	SetSearchCtrl(FALSE);

	// ListViewヘッダ
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	m_list.SetImageList(himS, LVSIL_SMALL);

	TCHAR buf[MAX_PATH];
	str.LoadString(IDS_TITLE_NAME);
	m_list.InsertColumn(0, str, HDF_LEFT, 10, 0);
	GetDlgItemText(IDC_DIT_NUMFOLDERS, buf, ARRAYSIZE(buf));
	ColonCutOff(buf);
	m_list.InsertColumn(1, buf, HDF_RIGHT, 10, 1);
	GetDlgItemText(IDC_DIT_NUMFILES, buf, ARRAYSIZE(buf));
	ColonCutOff(buf);
	m_list.InsertColumn(2, buf, HDF_RIGHT, 10, 2);
	GetDlgItemText(IDC_DIT_ALLSIZE, buf, ARRAYSIZE(buf));
	ColonCutOff(buf);
	m_list.InsertColumn(3, buf, HDF_RIGHT, 10, 3);
	GetDlgItemText(IDC_DIT_ALLCLUSTERS, buf, ARRAYSIZE(buf));
	ColonCutOff(buf);
	m_list.InsertColumn(4, buf, HDF_RIGHT, 10, 4);
	for (int i = 0; i < 5; i++)
		m_list.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

	return TRUE;
}

BOOL CDirInfoDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW && pInfo->iCtrlId) {
		ShowCtrlHelp(GetDlgItem(pInfo->iCtrlId), pInfo->iCtrlId);
		return TRUE;
	}
	return FALSE;
}

void CDirInfoDlg::OnCancel()
{
	if (m_di.m_bSearching) {
		m_di.m_bCanceled = TRUE;
		m_di.m_bSearching = FALSE;
		SetSearchCtrl(FALSE);
	}
	else
		EndDialog(IDCANCEL);
}

void CDirInfoDlg::OnOK()
{
	m_list.DeleteAllItems();
	m_di.ResetItem();
	m_di.m_bSearching = TRUE;

	int sel = m_combo.GetCurSel();

	if (sel == 0) {
		m_di.m_bHasFilter = FALSE;
		m_di.m_bUnmatchFilter = FALSE;
		m_di.m_bUseRegExp = FALSE;
		m_di.m_strFilter = gszAst;
	}
	else {
		m_di.m_bHasFilter = TRUE;
		m_di.m_bUnmatchFilter = (sel == 2) ? TRUE : FALSE;
		if (GetDlgItemText(IDC_FILTER, m_di.m_strFilter) == 0) {
			MessageBeep(0);
			GetDlgItem(IDC_FILTER).SetFocus();
			return;
		}
		m_di.m_bUseRegExp = IsDlgButtonChecked(IDC_USEREGEXP);
	}

	SetDlgItemText(IDC_BYTESPERSECTOR, NULL);
	SetDlgItemText(IDC_BYTESPERCLUSTER, NULL);

	CDirItem* pItem;
	SetSearchCtrl(TRUE);

	CString str;
	int cnt = m_di.m_arrTargets.GetSize();
	for (int i = 0; i < cnt; i++) {
		// ファイル名を取得
		str = m_di.m_arrTargets[i];
		m_di.SetRoot(str);
		m_di.GetDriveInfo();

		pItem = new CDirItem(str, &m_di);
		m_di.m_pItem = pItem;
		m_di.m_arrItem.Add(pItem);

		if (pItem->isFolder())
			TreeWalk(str);
		else {
			CFileInfo fi;
			if (GetFileInfo(str, fi))
				pItem->AddFileInfo(&fi);
		}
		InsertItem(-1, pItem);

		m_di.m_numFiles += pItem->m_numFiles;
		m_di.m_numDir += pItem->m_numDir;
		m_di.m_dlTotalLogicalSize += pItem->m_dlTotalLogicalSize;
		m_di.m_dlTotalRealSize += pItem->m_dlTotalRealSize;
		m_di.m_dwTotalClusters += pItem->m_dwTotalClusters;
	}
	m_di.m_pItem = NULL;
	MessageBeep(0);
	ShowStatus();

	TCHAR buf[MAX_PATH];
	SetDlgItemText(IDC_BYTESPERSECTOR, StrFormatByteSize(m_di.m_dwBytesPerSector, buf, ARRAYSIZE(buf)));
	SetDlgItemText(IDC_BYTESPERCLUSTER, StrFormatByteSize(m_di.m_dwBytesPerCluster, buf, ARRAYSIZE(buf)));

	m_di.m_bSearching = FALSE;
	SetSearchCtrl(FALSE);
}

void CDirInfoDlg::OnCheckFilter()
{
	BOOL bEnable = (m_combo.GetCurSel() > 0);
	GetDlgItem(IDC_FILTER).EnableWindow(bEnable);
	GetDlgItem(IDC_USEREGEXP).EnableWindow(bEnable);
}

void CDirInfoDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	buf[0] = 0;
	if (fopBrowseForFolder(m_hWnd, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		m_di.ResetTargets();
		m_di.AddTarget(buf);
		*pResult = TRUE;
	}
}

BOOL CDirInfoDlg::TreeWalk(LPCTSTR pszWork)
{
	CDirItem* pItem = m_di.m_pItem;
	pItem->m_numDir++;
	ShowStatus();
	if (!ZapMessage()) return FALSE;

	register CFileInfo* pf;
	CFileArray arr;
	BOOL bUnmatchFilter = (m_di.m_bHasFilter && m_di.m_bUnmatchFilter && lstrlen(m_di.m_strFilter));
	arr.BrowseFiles(pszWork, m_di.m_strFilter, bUnmatchFilter, m_di.m_bUseRegExp);

	int cnt = (int)arr.GetCount();
	int i;
	for (i = 0; i < cnt; i++) {
		pf = arr[i];
		pItem->AddFileInfo(pf);
	}

	arr.DeleteAll();
	arr.EnumSubFolder(pszWork);
	cnt = (int)arr.GetCount();
	for (i = 0; i < cnt; i++) {
		pf = arr[i];
		if (!TreeWalk(pf->m_strFullPathName))
			return FALSE;
	}
	return TRUE;
}

BOOL CDirInfoDlg::ZapMessage()
{
	if (m_di.m_bCanceled)
		return FALSE;

	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!IsDialogMessage(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return TRUE;
}

void CDirInfoDlg::OnColumnClick(LPNMHDR pnmh, LRESULT* pResult)
{
	NMLISTVIEW* plv = (NMLISTVIEW*)pnmh;
	m_di.m_bSortDown = m_di.m_bSortDown ? FALSE : TRUE;
	m_list.SortItems(di_SortCompare, MAKELPARAM(m_di.m_bSortDown, plv->iSubItem));
}

void CDirInfoDlg::OnDblClick(LPNMHDR pnmh, LRESULT* pResult)
{
	CPoint pt;
	GetCursorPos(&pt);
	m_list.ScreenToClient(&pt);
	UINT flags;
	int index = m_list.HitTest(pt, &flags);
	if (index >= 0 && (flags & (LVHT_ONITEMLABEL | LVHT_ONITEMICON))) {
		CDirItem* pItem = (CDirItem*)m_list.GetItemData(index);
		if (pItem->isFolder()) {
			CFileArray array;
			if (array.EnumSubFolder(pItem->m_strName) > 0) {
				HDROP hdrop = array.MakeDropFiles();
				array.DeleteAll();

				fopEnumFileSize(m_hWnd, hdrop);
				DragFinish(hdrop);
			}
		}
	}
}

void CDirInfoDlg::InsertItem(int insert, CDirItem* pItem)
{
	TCHAR buf[MAX_PATH];

	// アイコン
	SHFILEINFO sfi;
	SHGetFileInfo(pItem->m_strName, 0, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
	if (insert < 0)
		insert = m_list.GetItemCount();
	int idx = m_list.InsertItem(insert, PathFindFileName(pItem->m_strName), sfi.iIcon);
	m_list.SetItemData(idx, (DWORD_PTR)pItem);
	_itot_s(pItem->m_numDir, buf, ARRAYSIZE(buf), 10);
	m_list.SetItemText(idx, 1, buf);
	_itot_s(pItem->m_numFiles, buf, ARRAYSIZE(buf), 10);
	m_list.SetItemText(idx, 2, buf);
	StrFormatByteSize64(pItem->m_dlTotalLogicalSize, buf, ARRAYSIZE(buf));
	if (pItem->m_dlTotalLogicalSize != pItem->m_dlTotalRealSize) {
		TCHAR ww[MAX_PATH];
		ww[0] = _T('(');
		StrFormatByteSize64(pItem->m_dlTotalRealSize, ww + 1, ARRAYSIZE(ww));
		int len = lstrlen(ww);
		ww[len++] = _T(')');
		ww[len] = 0;
		lstrcat(buf, ww);
	}
	m_list.SetItemText(idx, 3, buf);
	DWORD dw = pItem->m_dwTotalClusters + pItem->m_numDir;
	DWORDLONG dl = UInt32x32To64(dw, pItem->m_pDiskSpace->m_dwBytesPerCluster);
	StrFormatByteSize64(dl, buf, ARRAYSIZE(buf));
	m_list.SetItemText(idx, 4, buf);

	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_list.SetColumnWidth(3, LVSCW_AUTOSIZE_USEHEADER);
	m_list.SetColumnWidth(4, LVSCW_AUTOSIZE_USEHEADER);
}

void CDirInfoDlg::ShowStatus()
{
	int numFiles = m_di.m_numFiles;
	int numDirs = m_di.m_numDir;
	DWORDLONG dlTotalLogicalSize = m_di.m_dlTotalLogicalSize;
	DWORDLONG dlTotalRealSize = m_di.m_dlTotalRealSize;
	DWORD dwTotalClusters = m_di.m_dwTotalClusters;
	if (m_di.m_pItem) {
		numFiles += m_di.m_pItem->m_numFiles;
		numDirs += m_di.m_pItem->m_numDir;
		dlTotalLogicalSize += m_di.m_pItem->m_dlTotalLogicalSize;
		dlTotalRealSize += m_di.m_pItem->m_dlTotalRealSize;
		dwTotalClusters += m_di.m_pItem->m_dwTotalClusters;
	}

	TCHAR buf[MAX_PATH];
	SetDlgItemInt(IDC_NUMDIR, numDirs, FALSE);
	SetDlgItemInt(IDC_NUMFILES, numFiles, FALSE);
	StrFormatByteSize64(dlTotalLogicalSize, buf, ARRAYSIZE(buf));
	if (dlTotalLogicalSize != dlTotalRealSize) {
		TCHAR ww[MAX_PATH];
		ww[0] = _T('(');
		StrFormatByteSize64(dlTotalRealSize, ww + 1, ARRAYSIZE(ww));
		int len = lstrlen(ww);
		ww[len++] = _T(')');
		ww[len] = 0;
		lstrcat(buf, ww);
	}
	SetDlgItemText(IDC_ALLSIZE, buf);

	// １ディレクトリエントリを１クラスタとして追加計算する
	DWORD dw = dwTotalClusters + numDirs;
	DWORDLONG dl = UInt32x32To64(dw, m_di.m_dwBytesPerCluster);
	SetDlgItemText(IDC_ALLCLUSTERS, StrFormatByteSize64(dl, buf, ARRAYSIZE(buf)));
	dl -= dlTotalRealSize;
	SetDlgItemText(IDC_CLUSTERGAP, StrFormatByteSize64(dl, buf, ARRAYSIZE(buf)));
}

void CDirInfoDlg::SetSearchCtrl(BOOL bSearching)
{
	UINT id = (bSearching) ? IDS_TEXT_ABORT : IDS_TEXT_CLOSE;
	CString cs;
	cs.LoadString(id);
	GetDlgItem(IDCANCEL).SetWindowText(cs);
	GetDlgItem(IDOK).EnableWindow(!bSearching);
}

/////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI fopEnumFileSize(HWND hParent, HDROP hDrop)
{
	CDirInfoDlg dlg;
	dlg.m_di.AddTarget(hDrop);
	dlg.DoModal(hParent);
}

extern "C"
void WINAPI fopEnumPathSize(HWND hParent, LPCTSTR pszDir)
{
	CDirInfoDlg dlg;
	dlg.m_di.AddTarget(pszDir);
	dlg.DoModal(hParent);
}

// selff.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "fileitem.h"
#include "selff.h"
#include "matchspec.h"

#define SEC2NANO(a)				((DWORDLONG)(a)*10000000)
#define MIN2NANO(a)				((DWORDLONG)(a)*600000000)
#define HOUR2NANO(a)			((DWORDLONG)(a)*36000000000)
#define DAY2NANO(a)				((DWORDLONG)(a)*864000000000)
#define WEEK2NANO(a)			((DWORDLONG)(a)*6048000000000)

/////////////////////////////////////////////////////////////////////////////
// CSelFFDlg dialog

CSelFFDlg::CSelFFDlg()
{
	m_fSearchRetry = false;
}

CSelFFDlg::~CSelFFDlg()
{
	if (m_hImage) ImageList_Destroy(m_hImage);
}

/////////////////////////////////////////////////////////////////////////////
// CSelFFDlg message handlers

BOOL CSelFFDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	HINSTANCE hInst = RESINST;
	HICON hic = (HICON)LoadImage(hInst, _MKIR(IDR_FRAME), IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hic, FALSE);
	hic = (HICON)LoadImage(hInst, _MKIR(IDR_FRAME), IMAGE_ICON,
							GetSystemMetrics(SM_CXICON),
							GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hic, TRUE);


	m_comboFindmode = GetDlgItem(IDC_FILETYPE);
	InitDlgCtrl(m_comboFindmode, IDS_SELFF_FILETYPE);

	m_comboSizetype = GetDlgItem(IDC_SIZETYPE);
	InitDlgCtrl(m_comboSizetype, IDS_SELFF_SIZETYPE);

	m_tab = GetDlgItem(IDC_FOLDER);
	m_list = GetDlgItem(IDC_LIST);
	m_tree = GetDlgItem(IDC_TREE);
	m_comboFilter = GetDlgItem(IDC_COMBO);
	m_spinSize = GetDlgItem(IDC_SSPIN);
	m_spinPast = GetDlgItem(IDC_PAST_SPIN);
	m_comboPast = GetDlgItem(IDC_PAST_COMBO);
	InitDlgCtrl(m_comboPast, IDS_SELFF_PERIOD);
	m_dtFrom = GetDlgItem(IDC_FROM);
	m_dtTo = GetDlgItem(IDC_TO);

	// フィルタの履歴を登録
	LoadHistory();

	// イメージリスト作成（複製）
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);

	#ifdef Shell_GetCachedImageIndex
	#undef Shell_GetCachedImageIndex
	#endif
	int (WINAPI* pfn)(LPCWSTR, int, UINT);
	pfn = Shell_GetCachedImageIndex;

	int cx, cy;
	ImageList_GetIconSize(himS, &cx, &cy);
	m_hImage = ImageList_Create(cx, cy, ILC_COLOR32, 2, 1);
	int i = (*pfn)(gszShell32, 3, FALSE);
	hic = ImageList_GetIcon(himS, i, ILD_TRANSPARENT);
	ImageList_ReplaceIcon(m_hImage, -1, hic);
	DestroyIcon(hic);
	i = (*pfn)(gszShell32, 8, FALSE);
	hic = ImageList_GetIcon(himS, i, ILD_TRANSPARENT);
	ImageList_ReplaceIcon(m_hImage, -1, hic);
	DestroyIcon(hic);
	m_tab.SetImageList(m_hImage);

	// タブアイテムの設定
	TCHAR buf[MAX_PATH * 2];
	TCITEM item;
	item.mask = TCIF_TEXT | TCIF_IMAGE;
	item.iImage = 0;
	LoadString(RESINST, IDS_FOLDER_SELFF, buf, ARRAYSIZE(buf));
	item.pszText = buf;
	item.cchTextMax = lstrlen(buf);
	m_tab.InsertItem(0, &item);

	item.iImage = 1;
	LoadString(RESINST, IDS_DRIVE_SELFF, buf, ARRAYSIZE(buf));
	item.pszText = buf;
	item.cchTextMax = lstrlen(buf);
	m_tab.InsertItem(1, &item);

	// タブ矩形の中へ調整
	CRect rc;
	m_tab.GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_tab.AdjustRect(FALSE, &rc);

	m_list.MoveWindow(&rc, FALSE);
	CRect rce;
	GetDlgItem(IDC_PATH).GetWindowRect(&rce);
	rc.top += rce.Height() + 4;
	m_tree.MoveWindow(&rc, FALSE);

	// ツリー配色
	m_tree.SetCustomColors(gColors.GetSize(), gColors);
	// フォルダ１初期設定
	if (m_psp->strStartPath.IsEmpty()) {
		GetCurrentDirectory(ARRAYSIZE(buf), buf);
		m_psp->strStartPath = buf;
	}
	m_tree.SelectByPath(m_psp->strStartPath);

	// フォルダ２初期設定
	m_list.GetClientRect(&rc);

	m_list.SetImageList(himS, LVSIL_SMALL);
	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, rc.Width(), 0);
	int sel = -1;
	int curdrv = ToUpper(m_psp->strStartPath[0]) - 'A';
	SHFILEINFO sfi;
	for (int i = 0; i < 26; i++) {
		PathBuildRoot(buf, i);
		if (GetDriveType(buf) > 1) 	{
			SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME);
			int index = m_list.InsertItem(i, sfi.szDisplayName, sfi.iIcon);
			m_list.SetItemData(index, i);
			if (sel == -1 && curdrv == i)
				sel = index;
		}
	}
	if (sel != -1)
		m_list.SetItemState(sel, LVIS_SELECTED, LVIS_SELECTED);

	// 前回のタブ位置を復元
	CAppReg reg;
	m_psp->nTargetMode = reg.GetValue(IDS_SEC_SETTING, IDS_KEY_LASTFOLDER, 0);
	m_tab.SetCurSel(m_psp->nTargetMode);
	SelectTab();

	m_spinPast.SetRange32(1, 100);
	m_spinPast.SetPos32(1);
	m_spinSize.SetRange32(0, INT_MAX);
	m_spinSize.SetPos32(0);

	m_psp->nSearchType = 0;
	m_psp->bFilterExclude = FALSE;

	if (m_fSearchRetry) {
		m_comboFilter.SetCurSel(0);
		OnHistoryChange();
	}
	else
		ApplyCondition();

	// 
	CEdit edit = m_comboFilter.GetDlgItem(1001);
	CString str; str.LoadString(IDS_FILTER_CUEBANNER);
	edit.SetCueBannerText(str);
	edit.SetFocus();

	return FALSE;
}

void CSelFFDlg::OnDestroy()
{
	HICON hic = GetIcon(TRUE);
	if (hic) DestroyIcon(hic);
	hic = GetIcon(FALSE);
	if (hic) DestroyIcon(hic);
}

void CSelFFDlg::ApplyCondition()
{
	// パス・ドライブ
	switch (m_psp->nTargetMode) {
	case 0:									// パス
		if (PathFileExists(m_psp->strStartPath)) {
			if (!PathIsUNC(m_psp->strStartPath))
				m_tree.SelectByPath(m_psp->strStartPath);
			else
				SetDlgItemText(IDC_PATH, m_psp->strStartPath);
		}
		break;
	case 1:									// ドライブ
	{
		CSimpleArray<CString> array;
		int argc = SeparateString(m_psp->strStartPath, _T(';'), array);
		if (argc > 0) {
			int cnt = m_list.GetItemCount();
			for (int i = 0; i < cnt; i++) {
				m_list.SetItemState(i, 0, LVIS_SELECTED);
				int id = (int)m_list.GetItemData(i);
				for (int j = 0; j < argc; j++) {
					if (id == PathGetDriveNumber(array[j])) {
						m_list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
						break;
					}
				}
			}
		}
		break;
	}
	}
	m_tab.SetCurSel(m_psp->nTargetMode);
	SelectTab();

	// パラメタ
	CheckDlgButton(IDC_CHECK, m_psp->bFilterExclude);
	m_comboFindmode.SetCurSel(m_psp->nSearchType);
	if (m_psp->nSearchType == 0)
		m_comboSizetype.SetCurSel(0);
	else {
		if (m_psp->nDateType == 0) {
			CheckDlgButton(IDC_DATERANGE, TRUE);

			SYSTEMTIME st;
			FileTimeToSystemTime(&m_psp->ftStart, &st);
			m_dtFrom.SetSystemTime(GDT_VALID, &st);

			FileTimeToSystemTime(&m_psp->ftEnd, &st);
			m_dtTo.SetSystemTime(GDT_VALID, &st);
		}
		else {
			CheckDlgButton(IDC_PAST_DATE, TRUE);
			m_spinPast.SetPos32(m_psp->nLimitDate);
			m_comboPast.SetCurSel(m_psp->nDateType - 1);
		}
	}

	CheckDlgButton(IDC_REGEXP, m_psp->bRegExp);
	CheckDlgButton(IDC_CASESENSE, m_psp->bCaseSense);

	CheckDlgButton(IDC_FA_SYSTEM, (m_psp->attr & FAT_SYSTEM) ? TRUE : FALSE);
	CheckDlgButton(IDC_FA_HIDDEN, (m_psp->attr & FAT_HIDDEN) ? TRUE : FALSE);
	CheckDlgButton(IDC_FA_RDONLY, (m_psp->attr & FAT_RDONLY) ? TRUE : FALSE);
	CheckDlgButton(IDC_FA_ARCHIVE, (m_psp->attr & FAT_ARCH) ? TRUE : FALSE);
	CheckDlgButton(IDC_FA_CRYPT, (m_psp->attr & FAT_ENCRYPT) ? TRUE : FALSE);
	CheckDlgButton(IDC_FA_COMPRESS, (m_psp->attr & FAT_CMPRSD) ? TRUE : FALSE);
	CheckDlgButton(IDC_FA_SPARSE, (m_psp->attr & FAT_SPARSE) ? TRUE : FALSE);
	CheckDlgButton(IDC_FA_OFFLINE, (m_psp->attr & FAT_OFFLINE) ? TRUE : FALSE);

	m_comboSizetype.SetCurSel(m_psp->nSizeType);
	if (m_psp->nSizeType > 0)
		m_spinSize.SetPos32(m_psp->nLimitSize);

	OnFindType();
}

void CSelFFDlg::OnOK()
{
	// 現在のフォルダ位置を取得
	m_psp->nTargetMode = m_tab.GetCurSel();
	CAppReg reg;
	reg.SetValue(IDS_SEC_SETTING, IDS_KEY_LASTFOLDER, m_psp->nTargetMode);

	TCHAR buf[MAX_PATH * 2];
	if (m_psp->nTargetMode == 0) {
		// 検索開始パス指定
		CString str;
		if (GetDlgItemText(IDC_PATH, str) == 0 || !PathIsDirectory(str)) {
			MessageBeep(0);
			GetDlgItem(IDC_PATH).SetFocus();
			return;
		}
		m_psp->strStartPath = str;
	}
	else {
		// 複数ドライブ指定
		m_psp->strStartPath.Empty();
		int index = -1;
		while (1) {
			index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
			if (index < 0)
				break;

			if (!m_psp->strStartPath.IsEmpty())
				m_psp->strStartPath += ';';
			PathBuildRoot(buf, (int)m_list.GetItemData(index));
			m_psp->strStartPath += buf;
		}

		if (m_psp->strStartPath.IsEmpty()) {
			MessageBeep(0);
			return;
		}
	}

	// フィルタの取得
	CString str;
	if (m_comboFilter.GetWindowText(str) < 1) {
		MessageBeep(0);
		m_comboFilter.SetFocus();
		return;
	}
	int sel = m_comboFilter.GetCurSel();
	if (sel >= 0) {
		CString lbstr;
		m_comboFilter.GetLBText(sel, lbstr);
		if (lbstr == str)
			str = m_history[sel].strSearchMask;
	}
	m_psp->strSearchMask = str;

	// 正規表現
	m_psp->bRegExp = IsDlgButtonChecked(IDC_REGEXP);

	// 大文字小文字の区別
	m_psp->bCaseSense = IsDlgButtonChecked(IDC_CASESENSE);

	// 属性
	m_psp->attr = 0;
	if (IsDlgButtonChecked(IDC_FA_SYSTEM))		m_psp->attr |= FAT_SYSTEM;
	if (IsDlgButtonChecked(IDC_FA_HIDDEN))		m_psp->attr |= FAT_HIDDEN;
	if (IsDlgButtonChecked(IDC_FA_RDONLY))		m_psp->attr |= FAT_RDONLY;
	if (IsDlgButtonChecked(IDC_FA_ARCHIVE))		m_psp->attr |= FAT_ARCH;
	if (IsDlgButtonChecked(IDC_FA_CRYPT))		m_psp->attr |= FAT_ENCRYPT;
	if (IsDlgButtonChecked(IDC_FA_COMPRESS))	m_psp->attr |= FAT_CMPRSD;
	if (IsDlgButtonChecked(IDC_FA_SPARSE))		m_psp->attr |= FAT_SPARSE;
	if (IsDlgButtonChecked(IDC_FA_OFFLINE))		m_psp->attr |= FAT_OFFLINE;

	// フィルタの反転
	m_psp->bFilterExclude = IsDlgButtonChecked(IDC_CHECK);

	m_psp->nSearchType = m_comboFindmode.GetCurSel();

	// 日付時刻
	SYSTEMTIME st;
	if (IsDlgButtonChecked(IDC_DATERANGE)) {
		// 期間指定
		m_psp->nDateType = 0;

		m_dtFrom.GetSystemTime(&st);
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		SystemTimeToFileTime(&st, &m_psp->ftStart);

		m_dtTo.GetSystemTime(&st);
		st.wHour = 23;
		st.wMinute = 59;
		st.wSecond = 59;
		st.wMilliseconds = 999;
		SystemTimeToFileTime(&st, &m_psp->ftEnd);
	}
	else if (IsDlgButtonChecked(IDC_PAST_DATE)) {
		// 過去
		m_psp->nDateType = m_comboPast.GetCurSel() + 1;
		m_psp->nLimitDate = m_spinPast.GetPos32();

		GetLocalTime(&st);
		SystemTimeToFileTime(&st, &m_psp->ftEnd);

		if (m_psp->nDateType == 1) {
			// 過去？日
			DWORDLONG dl;
			SETDWORDLONG(dl, m_psp->ftEnd.dwLowDateTime, m_psp->ftEnd.dwHighDateTime);
			dl -= DAY2NANO(m_psp->nLimitDate);
			m_psp->ftStart.dwLowDateTime = LODWORD(dl);
			m_psp->ftStart.dwHighDateTime = HIDWORD(dl);
		}
		else if (m_psp->nDateType == 2) {
			// 過去？週間
			DWORDLONG dl;
			SETDWORDLONG(dl, m_psp->ftEnd.dwLowDateTime, m_psp->ftEnd.dwHighDateTime);
			dl -= WEEK2NANO(m_psp->nLimitDate);
			m_psp->ftStart.dwLowDateTime = LODWORD(dl);
			m_psp->ftStart.dwHighDateTime = HIDWORD(dl);
		}
		else if (m_psp->nDateType == 3) {
			// 過去？ヶ月
			DWORD dw = ((DWORD)st.wYear * 12 + (DWORD)(st.wMonth - 1)) - m_psp->nLimitDate;
			st.wYear = (WORD)(dw / 12);
			st.wMonth = (WORD)(dw % 12) + 1;
			switch (st.wMonth) 	{
			case 2:
				if (st.wDay > 28)
					st.wDay = 28;
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				if (st.wDay > 30)
					st.wDay = 30;
				break;
			}
			SystemTimeToFileTime(&st, &m_psp->ftStart);
		}
		else if (m_psp->nDateType == 4) {
			// 過去？年
			WORD w = (WORD)min(m_psp->nLimitDate, (int)st.wYear);
			st.wYear -= w;
			SystemTimeToFileTime(&st, &m_psp->ftStart);
		}
	}

	// サイズ制限
	m_psp->nSizeType = m_comboSizetype.GetCurSel();
	if (m_psp->nSizeType > 0)
		m_psp->nLimitSize = m_spinSize.GetPos32();

	// ヒストリの記録
	SaveHistory();

	EndDialog(IDOK);
}

void CSelFFDlg::OnHistoryChange()
{
	int sel = m_comboFilter.GetCurSel();
	if (sel != CB_ERR) {
		*m_psp = m_history[sel];
		ApplyCondition();
	}
}

void CSelFFDlg::OnNotifyTreeChanged(NMHDR*, LRESULT*)
{
	DWORD attr;
	LPITEMIDLIST pidl = m_tree.GetSelItem(&attr);
	if (pidl) {
		TCHAR buf[MAX_PATH * 2];
		if (!SHGetPathFromIDList(pidl, buf))
			buf[0] = 0;
		SetDlgItemText(IDC_PATH, buf);
	}
}

void CSelFFDlg::OnFindType() 
{
	BOOL bEnable = (m_comboFindmode.GetCurSel() > 0);
	GetDlgItem(IDC_DATERANGE).EnableWindow(bEnable);
	GetDlgItem(IDC_PAST_DATE).EnableWindow(bEnable);
	OnDateRange();
}

void CSelFFDlg::OnDateRange() 
{
	BOOL bEnable;
	bEnable = (GetDlgItem(IDC_DATERANGE).IsWindowEnabled() &&
				IsDlgButtonChecked(IDC_DATERANGE));
	m_dtFrom.EnableWindow(bEnable);
	m_dtTo.EnableWindow(bEnable);

	bEnable = (GetDlgItem(IDC_PAST_DATE).IsWindowEnabled() && IsDlgButtonChecked(IDC_PAST_DATE));
	GetDlgItem(IDC_PAST_PERIOD).EnableWindow(bEnable);
	m_spinPast.EnableWindow(bEnable);
	m_comboPast.EnableWindow(bEnable);
}

void CSelFFDlg::SelectTab()
{
	if (m_tab.GetCurSel() == 0) {
		m_list.ShowWindow(SW_HIDE);
		m_tree.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PATH).ShowWindow(SW_SHOW);
	}
	else {
		m_tree.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PATH).ShowWindow(SW_HIDE);
		m_list.ShowWindow(SW_SHOW);
	}
}

void CSelFFDlg::OnHelp()
{
#ifndef _WIN64
	ResHelp(m_hWnd, IDR_HTML_SEARCH);
#else
	ShowHtmlHelp(m_hWnd, IDD);
#endif
}

void CSelFFDlg::OnClear() 
{
	if (AppMessage(m_hWnd, IDS_CLEAR_HISTORY, NULL, MB_QYESNO) == IDYES) {
		CAppReg reg;
		reg.ClearMruHistory(m_comboFilter, IDS_SEC_SETTING, IDS_KEY_FIND);
	}
}

void CSelFFDlg::LoadHistory()
{
	void* pv = malloc(2048);
	if (!pv) return;

	CAppReg reg;
	CString strKey = reg.KeyCombine(IDS_SEC_SETTING, IDS_KEY_FIND);

	CMRUProc mru;
	if (mru.Create(HKEY_CURRENT_USER, strKey, 16, TRUE)) {
		int n = mru.GetCount();
		for (int i = 0; i < n; i++) {
			int size = mru.Enum(i, NULL, 0);
			if (size > 0) {
				mru.Enum(i, pv, size);

				IStream* pstrm = SHCreateMemStream((BYTE*)pv, size);
				if (pstrm) {
					CString strMask;
					CSearchParam sp;
					if (sp.Load(pstrm)) {
						m_history.Add(sp);
						CString str;
						str.Format(_T("%s - %s"), sp.strSearchMask.GetString(), sp.strStartPath.GetString());
						m_comboFilter.AddString(str);
					}
					pstrm->Release();
				}
			}
		}
		mru.Close();
	}
	free(pv);
}

void CSelFFDlg::SaveHistory()
{
	CAppReg reg;
	CString strKey = reg.KeyCombine(IDS_SEC_SETTING, IDS_KEY_FIND);

	CMRUProc mru;
	if (mru.Create(HKEY_CURRENT_USER, strKey, 16, TRUE)) {
		IStream* pstrm;
		if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
			m_psp->Save(pstrm);

			LARGE_INTEGER mov;
			ULARGE_INTEGER pos;
			mov.QuadPart = 0;
			pstrm->Seek(mov, STREAM_SEEK_CUR, &pos);
			if (pos.QuadPart > 0) {
				HGLOBAL hmem;
				if (GetHGlobalFromStream(pstrm, &hmem) == S_OK) {
					void* pv = GlobalLock(hmem);
					mru.AddBinary(pv, (int)pos.QuadPart);
					GlobalUnlock(hmem);
				}
			}
		}
		mru.Close();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWalkDlg dialog


/////////////////////////////////////////////////////////////////////////////
// CDirWalker

void CDirWalker::Initialize(HWND hOwner, CSearchParam* psp)
{
	m_hOwner = hOwner;
	m_psp = psp;
	m_bCanceled = FALSE;
	ResetItems();
}

void CDirWalker::ResetItems()
{
	int cnt = (int)m_array.GetCount();
	for (int i = 0; i < cnt; i++) {
		CFileInfo* pf = (CFileInfo*)m_array[i];
		if (pf) delete pf;
	}
	m_array.RemoveAll();
}

void CDirWalker::Start()
{
	m_bCanceled = FALSE;
	if (CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (LPVOID*)&m_progress) == S_OK) {
		CString str;
		str.LoadString(IDS_CAPTION_SEARCHING);
		m_progress->SetTitle(str);
		m_progress->SetAnimation(GetModuleHandle(DLL_FOPSRV), 150);
		m_progress->StartProgressDialog(m_hOwner, NULL, PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR, NULL);
	}

	// 検索経過表示
	CSimpleArray<CString> arrPath;
	int argc = SeparateString(m_psp->strStartPath, ';', arrPath);
	m_serial = 0;
	for (int i = 0; i < argc; i++) {
		if (!SearchTree(arrPath[i]))
			break;
	}

	if (m_progress) {
		m_progress->StopProgressDialog();
		m_progress->Release();
		m_progress = NULL;
	}
}

BOOL CDirWalker::SearchTree(LPCTSTR pszPath)
{
	if (m_progress) {
		if (m_progress->HasUserCancelled()) {
			m_bCanceled = TRUE;
			return FALSE;
		}
		m_progress->SetLine(2, pszPath, TRUE, NULL);
	}

	CFileArray aarr;		// 全ファイル＆フォルダ用アレイ
	CFileArray farr;		// フォルダ用アレイ
	CFileInfo* pf;
//@	aarr.BrowseFiles(pszPath, NULL, FBR_BOTH);
	aarr.Browse(pszPath);

	CString mask = m_psp->strSearchMask;
	if (!m_psp->bRegExp && mask.FindOneOf(_T("?*")) < 0) {
		// 正規表現ではなくて且つワイルドカードが指定されていない場合
		// *pat;pat*;*pat*;
		CString tmp;
		tmp.Format(_T("*%s;%s*;*%s*;"), (LPCTSTR)mask, (LPCTSTR)mask, (LPCTSTR)mask);
		mask = tmp;
	}

	CMatchSpec ms(mask, m_psp->bRegExp, m_psp->bCaseSense);

	CFileArray marr;		// 条件一致した項目用アレイ
	int i;
	for (i = 0; i < (int)aarr.GetCount(); i++) {
		pf = (CFileInfo*)aarr[i];

		// フォルダは複製してリストアップ
		if (pf->IsFolder()) {
			CFileInfo* pff = new CFileInfo(*pf);
			farr.Add(pff);
		}

		// ワイルドカード
		if (!ms.Match(pf->m_strName))
			continue;

		// サイズ指定あり
		if (!m_psp->MatchFileSize(pf->m_dlSize))
			continue;

		// 属性指定あり
		if (!m_psp->MatchFileAttributes(pf->m_dwAttr))
			continue;

		// 時刻期限指定あり
		if (!m_psp->MatchFileTime(&pf->m_ftCreationTime, &pf->m_ftLastWriteTime))
			continue;

		marr.Add(pf);
		aarr[i] = NULL;
	}

	if (!m_psp->bFilterExclude)
		aarr.DeleteAll();
	else {
		marr.DeleteAll();
		marr.MoveArray(&aarr);
	}

	int nFound = (int)marr.GetCount();
	for (i = 0; i < nFound; i++) {
		pf = marr[i];
		pf->m_nSerial = m_serial++;
		m_array.Add(pf);
		marr[i] = NULL;
	}
	marr.DeleteAll();

	m_nFound += nFound;
	if (nFound && m_progress) {
		CString fmt;
		fmt.LoadString(IDS_FOUND_PROGRESS);
		CString str;
		str.Format(fmt, m_nFound);
		m_progress->SetLine(1, str, FALSE, NULL);
	}

	BOOL bOK = TRUE;
	TCHAR buf[MAX_PATH * 2];
	for (i = 0; i < (int)farr.GetCount(); i++) {
		pf = farr[i];
		pf->GetRealName(buf);
		if (!SearchTree(buf)) {
			bOK = FALSE;
			break;
		}
	}
	farr.DeleteAll();
	return bOK;
}

void CDirWalker::MoveItemTo(CAtlArray<CFileInfo*>& moveto, size_t iStart)
{
	if (m_array.GetCount() > 0) {
		moveto.InsertArrayAt(iStart, &m_array);
		m_array.RemoveAll();
	}
}

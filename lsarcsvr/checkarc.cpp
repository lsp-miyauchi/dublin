// CheckArcDlg.cpp : CCheckArcDlg の実装

#include "stdafx.h"
#include "ArcManager.h"
#include "common.h"
#include "arcres.h"
#include "checkarc.h"
#include "stringseparator.h"

#define	CHECKARCHIVE_FULLCRC	2	/* 完全(格納ファイルの CRC チェック) */

static void domessage()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

// CCheckArcDlg

BOOL CCheckArcDlg::OnInitDialog(LPARAM lParam)
{
	if (!m_pArcManager || !m_hdrop) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	DlgResize_Init();
	CenterWindow();
	m_list = GetDlgItem(IDC_LIST);

	memset(&m_ca, 0, sizeof(CHKARC));

	// イメージリストの設定
	HIMAGELIST him = ImageList_LoadImage(ATL::_AtlBaseModule.GetResourceInstance(), _MKIR(IDB_CHECK),
							20, 0, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);
	m_list.SetImageList(him, LVSIL_SMALL);

	// カラム設定
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 80;

	CString str;
	str.LoadString(IDS_CHECKARC_HEADER);
	CStringSeparator ss;
	int i, numcolumn = ss.Separate(str, _T('|'));
	for (i = 0; i < numcolumn; i++) {
		lvc.pszText = (LPTSTR)(LPCTSTR)ss[i];
		m_list.InsertColumn(i, &lvc);
	}

	// 書庫の列挙
	TCHAR buf[MAX_PATH * 2];
	int cnt = DragQueryFile(m_hdrop, 0xffffffff, NULL, 0);
	int n = 0;

	WIN32_FILE_ATTRIBUTE_DATA wa;
	LVITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.iImage = 0;
	for (i = 0; i < cnt; i++) {
		DragQueryFile(m_hdrop, i, buf, ARRAYSIZE(buf));

		if (GetFileAttributesEx(buf, GetFileExInfoStandard, &wa) &&
			!(wa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			m_array.Add(buf);

			lvi.iItem = n++;
			lvi.pszText = PathFindFileName(buf);
			lvi.lParam = 0;
			int index = m_list.InsertItem(&lvi);

			int sub = 1;
			// サイズ
			TCHAR tmp[MAX_PATH];
			LARGE_INTEGER li;
			li.LowPart = wa.nFileSizeLow;
			li.HighPart = wa.nFileSizeHigh;
			StrFormatByteSize64(li.QuadPart, tmp, ARRAYSIZE(tmp));
			m_list.SetItemText(index, sub++, tmp);

			// 日付
			FormatFileDateTime(&wa.ftLastWriteTime, tmp, ARRAYSIZE(tmp));
			m_list.SetItemText(index, sub++, tmp);

			// パス
			PathRemoveFileSpec(buf);
			m_list.SetItemText(index, sub++, buf);
		}
	}

	for (i = 0; i < numcolumn; i++)
		m_list.SetColumnWidth(i, LVSCW_AUTOSIZE);

	if (n > 0)
		m_list.SetItemState(0, LVIS_SETSELECT, LVIS_SETSELECT);
	else
		EndDialog(IDCANCEL);

	GetDlgItem(IDC_COPY).EnableWindow(FALSE);
	return TRUE;
}

void CCheckArcDlg::OnStart()
{
	m_ca.bChecking = TRUE;
	::EnableWindow(GetDlgItem(IDC_START), FALSE);
	::EnableWindow(GetDlgItem(IDC_COPY), FALSE);

	ULONG sizeResult = 0x8000;
	char* pResult = new char[sizeResult];

	LVITEM lvi;
	int index = 0;
	while (!m_ca.bCanceled && index < m_array.GetSize()) {
		memset(&lvi, 0, sizeof(lvi));
		lvi.mask =  LVIF_IMAGE;
		lvi.iItem = index;
		lvi.iImage = 4;
		m_list.SetItem(&lvi);
		m_list.SetItemState(index, LVIS_SETSELECT, LVIS_SETSELECT);
		m_list.EnsureVisible(index, FALSE);

		domessage();

		void* pv = (void*)m_list.GetItemData(index);
		if (pv)
			free(pv);

		*pResult = 0;
		int r = DoCheck(m_array[index], pResult, sizeResult);
		lvi.mask = LVIF_IMAGE | LVIF_PARAM;
		lvi.lParam = 0;
		if (r == CHKARC_OK)
			lvi.iImage = 1;
		else if (r == CHKARC_NG) {
			lvi.iImage = 2;
			lvi.lParam = (*pResult != 0) ? (LPARAM)_strdup(pResult) : NULL;
		}
		else
			lvi.iImage = 3;
		m_list.SetItem(&lvi);
		index++;
	}

	delete [] pResult;
	GetDlgItem(IDC_START).EnableWindow(TRUE);
	GetDlgItem(IDC_COPY).EnableWindow(TRUE);
	m_ca.bChecking = FALSE;
	GetDlgItem(IDCANCEL).SetFocus();
}


void CCheckArcDlg::OnCopy()
{
	int cnt = m_list.GetItemCount();
	if (cnt < 1) return;

	LVITEM lvi;
	CString str;
	int i;
	for (i = 0; i < cnt; i++) {
		memset(&lvi, 0, sizeof(lvi));
		lvi.mask =  LVIF_IMAGE;
		lvi.iItem = i;
		m_list.GetItem(&lvi);
		switch (lvi.iImage) {
		case 0:
			str += _T("??: ");
			break;
		case 1:
			str += _T("OK: ");
			break;
		case 2:
			str += _T("NG: ");
			break;
		default:
			str += _T("--: ");
			break;
		}

		str += m_array[i];
		str += _T("\x0d\x0a");
	}
	CopyTextToClipboard(m_hWnd, str);
}

void CCheckArcDlg::OnNotifyDblClick(NMHDR* pHdr, LRESULT* pResult)
{
	*pResult = 0;

	int nIndex = m_list.GetTopIndex();
	m_list.GetNextItem(nIndex - 1, LVNI_SELECTED);
	if (nIndex != -1) {
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = nIndex;
		lvi.iSubItem = 0;
		m_list.GetItem(&lvi);
		if (lvi.lParam) {
			CArcResDlg dlg;
			dlg.SetResult((LPCSTR)lvi.lParam);
			dlg.DoModal();
		}
	}
}

void CCheckArcDlg::OnNotifyDeleteItem(NMHDR* pHdr, LRESULT* pResult)
{
	*pResult = 0;

	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pHdr;
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = pnmlv->iItem;
	lvi.iSubItem = 0;
	m_list.GetItem(&lvi);
	if (lvi.lParam)
		free((void*)lvi.lParam);
}

int CCheckArcDlg::DoCheck(LPCTSTR pszName, CHAR* pResult, ULONG sizeResult)
{
	CString cmd;

	int type = SpecifyArchiveTypeByFileHeader(pszName);
	HRESULT hr = E_FAIL;
	LONG res;
	if (type == ARC_LZH) {
		cmd.Format(_T("t -n -gm \"%s\""), pszName);
		hr = m_pArcManager->Archive(m_hWnd, type, cmd, pResult, sizeResult, &res);
	}
	else if (type == ARC_ZIP) {
		cmd.Format(_T("-t \"%s\""), pszName);
		hr = m_pArcManager->Archive(m_hWnd, type, cmd, pResult, sizeResult, &res);
	}
	else if (type == ARC_CAB) {
		cmd.Format(_T("t \"%s\""), pszName);
		hr = m_pArcManager->Archive(m_hWnd, type, cmd, pResult, sizeResult, &res);
	}
	else if (type == ARC_BGA) {
		cmd.Format(_T("t \"%s\""), pszName);
		hr = m_pArcManager->Archive(m_hWnd, type, cmd, pResult, sizeResult, &res);
	}
	else if (type == ARC_TAR) {
/* @@@
		if (m_pArcManager->asInitialize(m_hWnd, type) == S_OK) {
			hr = m_pArcManager->asCheckArchive(pszName, CHECKARCHIVE_FULLCRC);
			m_pArcManager->asTerminate();
		}
@@@ */
	}
	else if (type == ARC_7ZIP) {
		cmd.Format(_T("t \"%s\""), pszName);
		hr = m_pArcManager->Archive(m_hWnd, type, cmd, pResult, sizeResult, &res);
	}
	else if (type == ARC_ERROR) {
		// まずLZH形式
		cmd.Format(_T("t -n -gm \"%s\""), pszName);
		hr = m_pArcManager->Archive(m_hWnd, ARC_LZH, cmd, pResult, sizeResult, &res);
		if (FAILED(hr)) {
			// 次にCAB形式
			cmd.Format(_T("t \"%s\""), pszName);
			hr = m_pArcManager->Archive(m_hWnd, ARC_CAB, cmd, pResult, sizeResult, &res);
			if (FAILED(hr))
				return CHKARC_UNK;
		}
	}
	return (FAILED(hr) ? CHKARC_NG : CHKARC_OK);
}


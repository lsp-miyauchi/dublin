// ArcView.cpp : implementation file
//

#include "stdafx.h"
#include "common.h"
#include "mainfrm.h"
#include "drgdrp.h"
#include "newarc.h"
//#include "waiting.h"
#include "animationimagelist.h"

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

void CMainFrame::GetSortType(int& seq, int& type)
{
	seq = m_nSortSeq;
	type = m_nSortType;
}

void CMainFrame::OnInitialUpdate()
{
	if (m_nSortType != SORT_NONE) {
		int index;
		if      (m_nSortType == SORT_SIZE)	index = 1;
		else if (m_nSortType == SORT_EXT)	index = 2;
		else if (m_nSortType == SORT_TIME)	index = 4;
		else if (m_nSortType == SORT_ATTR)	index = 6;
		else if (m_nSortType == SORT_PATH)	index = 7;
		else                                index = 0;
		ShowBoundary(index, TRUE);
	}
}

BOOL CMainFrame::OpenArcFile(LPCTSTR pszName)
{
	m_parse.Wep();
	if (pszName && *pszName) {
		if (!PerseArchive(pszName)) {
			PostMessage(WM_CLOSE);
			return FALSE;
		}
	}
	else {
		TCHAR buf[MAX_PATH * 2];
		if (!SetNewArchive(buf)) {
			PostMessage(WM_CLOSE);
			return FALSE;
		}
	}

	get_fileinfo(m_parse.m_strFile, &m_wfd);
	return TRUE;
}

BOOL CMainFrame::SetNewArchive(LPTSTR pName)
{
	CNewArcDlg dlg;

	lstrcpy(dlg.m_nai.name, pName);
	if (dlg.DoModal() != IDOK)
		return FALSE;

	lstrcpy(pName, dlg.m_nai.name);
	LPTSTR p = PathFindExtension(pName);
	if (*p != '.')
		lstrcat(pName, dlg.m_nai.arctype == ARC_LZH ? gszLzhExt : gszZipExt);
	else if (dlg.m_nai.arctype == ARC_LZH && lstrcmpi(p, gszLzhExt))
		lstrcpy(p, gszLzhExt);
	else if (dlg.m_nai.arctype == ARC_ZIP && lstrcmpi(p, gszZipExt))
		lstrcpy(p, gszZipExt);

	m_parse.Wep();
	m_parse.m_strFile = pName;
	m_parse.m_nType = dlg.m_nai.arctype;

	// 決め打ちじゃいかんのだが。
	if (m_parse.m_nType == ARC_LZH || m_parse.m_nType == ARC_CAB)
		m_parse.m_bUnicodeSupported = TRUE;

	return TRUE;
}

void CMainFrame::ListupAgain(bool bAdjustColumnWidth)
{
	m_list.SetRedraw(FALSE);
	m_list.DeleteAllItems();

	// 暗号化ファイル用アイコン
	m_list.SetImageList(m_parse.m_bHasEncryptedItems ? m_himLocked : NULL, LVSIL_STATE);
	m_statusbar.SetPaneIcon(ID_INDICATOR_MARKER, m_parse.m_bHasEncryptedItems ? m_hicCrypted : NULL);

	CSimpleArray<int> cols;
	cols.Add(m_list.GetStringWidth(_T("samplefile.doc")));			// 0. 名前
	cols.Add(m_list.GetStringWidth(_T("999,9999")));				// 1. サイズ
	cols.Add(m_list.GetStringWidth(_T("9999/99/99 99:99:99")));		// 2. 日時
	cols.Add(m_list.GetStringWidth(_T("File Document")));			// 3. 種類
	cols.Add(m_list.GetStringWidth(_T("100%")));					// 4. レート
	cols.Add(m_list.GetStringWidth(_T("Stored")));					// 5. 圧縮
	cols.Add(m_list.GetStringWidth(_T("DSHRA")));					// 6. 属性
	cols.Add(m_list.GetStringWidth(_T("AnyFolder/SubFolder")));		// 7. 場所

	DWORD size = 999999;
	CString str;
	int i, index, cx, cnt = m_array.GetSize();
	for (i = 0; i < cnt; i++) {
		CArcItem *pai = (CArcItem*)m_array[i];

		// 名前
		str = pai->m_strName;
		if (m_nStat & ATS_LOWER)
			str.MakeLower();
		index = m_list.InsertItem(i, str, pai->m_iImage);
		m_list.SetItemData(index, (DWORD_PTR)pai);
		cx = m_list.GetStringWidth(str);
		if (cx > cols[0]) cols[0] = 0;

		// サイズ
		pai->FormatSize(str, m_nStat & ATS_SIZEKB);
		m_list.SetItemText(index, 1, str);
		// 日時
		pai->FormatDate(str);
		m_list.SetItemText(index, 2, str);
		// 種類
		if (!pai->m_strType.IsEmpty()) {
			m_list.SetItemText(index, 3, pai->m_strType);
			cols[3] = 0;
		}
		// 率
		pai->FormatRatio(str);
		m_list.SetItemText(index, 4, str);
		// 圧縮モード
		m_list.SetItemText(index, 5, pai->m_strMode);
		// 属性
		m_list.SetItemText(index, 6, pai->m_strAttr);
		// 場所
		if (!pai->m_strPath.IsEmpty())
			m_list.SetItemText(index, 7, (LPCTSTR)pai->m_strPath);
		// 暗号化
		if (pai->m_attr & FILE_ATTRIBUTE_ENCRYPTED)
			m_list.SetItemState(index, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);

		size = max(pai->m_dwSize, size);
	}

	// カラム幅自動調整
	if (bAdjustColumnWidth) {
		for (i = 0; i < MAXHEADITEM; i++)
			m_list.SetColumnWidth(i, cols[i] ? cols[i] + 16 : LVSCW_AUTOSIZE);
	}

	m_list.SetRedraw(TRUE);
	m_list.Invalidate();
}

void CMainFrame::ListSettingChange()
{
	CAppReg reg;
	m_nStat = (DWORD)reg.GetValue(IDS_SEC_OPERATION, IDS_KEY_LISTSTATUS, m_nStat);

	// 拡張スタイル
	DWORD style = m_list.GetExtendedListViewStyle();
	style &= ~(LVS_EX_GRIDLINES | LVS_EX_HOTTRACK);
	if (m_nStat & ATS_GRID)
		style |= LVS_EX_GRIDLINES;
#ifdef MOUSE_HOTTRACKING
	if (m_nStat & ATS_HOTTRACK)
		style |= LVS_EX_HOTTRACK;
#endif
	m_list.SetExtendedListViewStyle(style);

	m_list.SetTextColor(gColors[colText]);
	m_list.SetTextBkColor(gColors[colBack]);
	m_list.SetBkColor(gColors[colBack]);

	ListupAgain();
}

BOOL CMainFrame::MakeRespFile(CString& resp)
{
	int cnt = m_list.GetSelectedCount();
	if (cnt < 1)
		return FALSE;

	LPTSTR pszResponse = resp.GetBuffer(MAX_PATH * 2);
	CSimpleArray<CArcItem*> arr;
	int i, index = -1;
	CArcItem* pai;
	DWORD dw;
	while (1) {
		index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
		if (index < 0)
			break;

		pai = (CArcItem*)m_list.GetItemData(index);
		if (pai->IsFolder()) {
			if (m_parse.m_nType != ARC_LZH) {
				CString flt = pai->m_strRealName.GetString();
				flt.Replace(_T('\\'), _T('/'));
				flt.TrimRight(_T('/'));
				flt += _T("/*");
				CSimpleArray<CArcItem*> aw;
				cnt = m_parse.PackByFilter(aw, flt);
				for (i = 0; i < cnt; i++)
					arr.Add(aw[i]);
				aw.RemoveAll();
			}
			else
				arr.Add(pai);
		}
		else
			arr.Add(pai);
	}

	PathCombine(pszResponse, m_szTempFolder, _T("~gsfsd.tmp"));
	GetShortPathName(pszResponse, pszResponse, MAX_PATH * 2);
	resp.ReleaseBuffer();

	if (arr.GetSize() > 0) {
		HANDLE hf = CreateFile(resp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			SortArcItemArray(arr);

			if (m_parse.m_bUnicodeSupported) {
				WORD bom = 0xfeff;
				WriteFile(hf, &bom, sizeof(bom), &dw, NULL);
			}

			CString str;
			cnt = arr.GetSize();
			for (i = 0; i < cnt; i++) {
				pai = arr[i];

				if (pai->IsFolder() && m_parse.m_nType == ARC_LZH)
					str.Format(_T("\"%s*.*\""), (LPCTSTR)pai->m_strRealName);
				else
					str.Format(_T("\"%s\""), (LPCTSTR)pai->m_strRealName);
				str += _T("\x0d\x0a");

				if (m_parse.m_bUnicodeSupported)
					WriteFile(hf, str, str.GetLength() * sizeof(WCHAR), &dw, NULL);
				else {
					char sz[MAX_PATH * 2];
					WideCharToMultiByte(m_parse.m_bUtf8Supported ? CP_UTF8 : CP_ACP, 0, str, -1, sz, ARRAYSIZE(sz), NULL, NULL);
					WriteFile(hf, sz, lstrlenA(sz) * sizeof(char), &dw, NULL);
				}
			}
			CloseHandle(hf);
			return TRUE;
		}
	}
	return FALSE;
}

DWORD CMainFrame::MakeRespFileByFilter(CString& resp, LPCTSTR pszFilter)
{
	CSimpleArray<CArcItem*> arr;
	if (m_parse.PackByFilter(arr, pszFilter) < 1)
		return (DWORD)-1;

	DWORD total = (DWORD)-1;
	LPTSTR pszResponse = resp.GetBuffer(MAX_PATH * 2);
	PathCombine(pszResponse, m_szTempFolder, _T("~gsfsd.tmp"));
	GetShortPathName(pszResponse, pszResponse, MAX_PATH * 2);
	HANDLE hf = CreateFile(pszResponse, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		DWORD dw;
		if (m_parse.m_bUnicodeSupported) {
			WORD bom = 0xfeff;
			WriteFile(hf, &bom, sizeof(bom), &dw, NULL);
		}

		total = 0;
		CArcItem* pai;
		CString str;
		int cnt = arr.GetSize();
		for (int i = 0; i < cnt; i++) {
			pai = arr[i];
			total += pai->m_dwSize;

			if (m_parse.m_nType == ARC_LZH) {
				if (!pai->IsFolder()) continue;

				str.Format(_T("\"%s*.*\"\x0d\x0a"), (LPCTSTR)pai->m_strRealName);
			}
			else
				str.Format(_T("\"%s\"\x0d\x0a"), (LPCTSTR)pai->m_strRealName);

			if (m_parse.m_bUnicodeSupported)
				WriteFile(hf, str, str.GetLength() * sizeof(TCHAR), &dw, NULL);
			else {
				char sz[MAX_PATH * 2];
				WideCharToMultiByte(m_parse.m_bUtf8Supported ? CP_UTF8 : CP_ACP, 0, str, -1, sz, ARRAYSIZE(sz), NULL, NULL);
				WriteFile(hf, sz, lstrlenA(sz) * sizeof(char), &dw, NULL);
			}
		}
		CloseHandle(hf);
	}
	resp.ReleaseBuffer();
	return total;
}

BOOL CMainFrame::OnGetDragImage(SHDRAGIMAGE* image)
{
	memset(image, 0, sizeof(SHDRAGIMAGE));
	int num = m_list.GetSelectedCount();
	if (num > 0) {
		num = min(4, num);

		CArcItem* pai;
		CSimpleArray<CArcItem*> sel;

		int i, index = -1;
		for (i = 0; i < num; i++) {
			index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
			if (index < 0) break;

			pai = (CArcItem*)m_list.GetItemData(index);
			sel.Add(pai);
		}

		int xDelta = 3;
		int yDelta = 3;

		image->crColorKey = GetSysColor(COLOR_WINDOW);
		image->sizeDragImage.cx = m_sizeLarge.cx + xDelta * (num - 1) + 6;
		image->sizeDragImage.cy = m_sizeLarge.cy + yDelta * (num - 1) + 6;
		image->ptOffset.x = image->sizeDragImage.cx - (m_sizeLarge.cx / 2);
		image->ptOffset.y = image->sizeDragImage.cy * 3 / 4;

		CClientDC dc(NULL);
		HBITMAP hbm = CreateCompatibleBitmap(dc, image->sizeDragImage.cx, image->sizeDragImage.cy);
		if (hbm) {
			CDC dcw; dcw.CreateCompatibleDC(dc);
			HBITMAP hbmOld = dcw.SelectBitmap(hbm);
			CRect rc(0, 0, image->sizeDragImage.cx, image->sizeDragImage.cy);
			dcw.FillSolidRect(&rc, image->crColorKey);
			lccDrawSelectionFrame(dcw, &rc, TRUE, -1);

			int x = 3;
			int y = 3;
			for (i = num - 1; i >= 0; i--) {
				pai = sel[i];
				ImageList_Draw(m_himl, pai->m_iImage, dcw, x, y, ILD_TRANSPARENT);
				x += xDelta;
				y += yDelta;
			}
			image->hbmpDragImage = dcw.SelectBitmap(hbmOld);
		}
	}

	return (image->hbmpDragImage != NULL);
}

DWORD CMainFrame::OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	if (IsDragMyself() || !IsDropFormatsHDROP()) return DROPEFFECT_NONE;

	return OnDragOver(pdo, keyState, dropEffect, point);
}

DWORD CMainFrame::OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	return (!IsDragMyself() && IsDropFormatsHDROP()) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
}

DWORD CMainFrame::OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	HDROP hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);
	if (!hdrop) return DROPEFFECT_NONE;

	// ドロップされるファイル数を取得
	int num = (int)DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	if (!num) {
		DragFinish(hdrop);
		return DROPEFFECT_NONE;
	}

	TCHAR buf[MAX_PATH * 2];
	if (num == 1) {
		DragQueryFile(hdrop, 0, buf, ARRAYSIZE(buf));
		int nType = SpecifyArchiveType(buf);
		if (nType != ARC_ERROR) {
			if (AppMessage(m_hWnd, IDS_NEWARCHIVE, NULL, MB_QYESNO) == IDYES)
				OpenArcFile(buf);
			DragFinish(hdrop);
			return DROPEFFECT_NONE;
		}
	}

	if (GetForegroundWindow() != m_hWnd)
		SetForegroundWindow(m_hWnd);

	AppendFiles(hdrop);
	DragFinish(hdrop);

	lstrcpy(buf, m_parse.m_strFile);
	OpenArcFile(buf);

	return DROPEFFECT_COPY;
}

void CMainFrame::OnLVNotifyBeginDrag(NMHDR* /*pNmhdr*/, LRESULT* /*pResult*/)
{
	// アーカイバタイプでＤ＆Ｄ解凍可能かチェック
	if (!m_parse.IsTypeDragSource())
		return;

	CPoint pt;
	GetCursorPos(&pt);

	// ドラッグ＆ドロップ処理
	int cnt = m_list.GetSelectedCount();
	if (cnt < 1) return;

	// D&D直前に作業フォルダへ対象ファイルを解凍する為のレスポンスファイルを作成する
	CString resp;
	if (!MakeRespFile(resp)) return;

	int index = -1;
	CSimpleArray<void*> array;
	CArcItem* pai;
	while (1) {
		index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
		if (index < 0)
			break;

		pai = (CArcItem*)m_list.GetItemData(index);
		array.Add(pai);
	}

	if (lccBeginTrack(m_hWnd, pt)) {
		CDataObject* pdo = new CDataObject(m_parse.m_strFile, m_parse.m_nType, &array, m_szTempFolder, resp);
		DWORD eff = DROPEFFECT_COPY;
		DragStart(m_hWnd, pdo, &eff);
		pdo->Release();
	}
	DeleteFile(resp);
}

void CMainFrame::OnLVNotifyItemChanged(NMHDR* /*pNmhdr*/, LRESULT* /*pResult*/)
{
	m_dwSelect = GetSelectedFileSize();
}

void CMainFrame::OnLVNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMLVKEYDOWN* pkd = (NMLVKEYDOWN*)pNmhdr;

	NMKEY nk;
	nk.hdr.hwndFrom = m_hWnd;
	nk.hdr.idFrom = GetDlgCtrlID();
	nk.hdr.code = NM_KEYDOWN;
	nk.nVKey = pkd->wVKey;
	nk.uFlags = pkd->flags;
	if (KeyTranslate(&nk)) {
		*pResult = TRUE;
		return;
	}

	*pResult = TRUE;
	switch (pkd->wVKey) {
	case VK_BACK:
		SelectParent();
		break;
	case VK_TAB:
		if (m_tree.IsWindowVisible())
			m_split.SetActivePane(0);
		break;
	case 'B':
		PostMessage(WM_COMMAND, IDM_BDUMP);
		break;
	case 'E':
		PostMessage(WM_COMMAND, IDM_OPEN);
		break;
	case 'V':
		PostMessage(WM_COMMAND, IDM_VIEW);
		break;
	case 'U':
		PostMessage(WM_COMMAND, IDM_EXTRACT);
		break;
	default:
		*pResult = FALSE;
		break;
	}
}

void CMainFrame::OnLVNotifyDblClick(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	NMLISTVIEW* pnmlv = (NMLISTVIEW*)pNmhdr;
	if (pnmlv->iItem >= 0) {
		WPARAM wp = IDM_OPEN;
		if (GetKeyState(VK_SHIFT) & 0x8000)
			wp = IDM_VIEW;
		else if (GetKeyState(VK_MENU) & 0x8000)
			wp = IDM_EXTRACT;
		PostMessage(WM_COMMAND, wp);
	}
}

void CMainFrame::OnLVNotifyColumnClick(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	NMLISTVIEW* pnmlv = (NMLISTVIEW*)pNmhdr;
	int nType = -1;
	if (pnmlv->iSubItem == 0)		nType = SORT_NAME;
	else if (pnmlv->iSubItem == 1)	nType = SORT_SIZE;
	else if (pnmlv->iSubItem == 2)	nType = SORT_TIME;
	else if (pnmlv->iSubItem == 3)	nType = SORT_EXT;
	else if (pnmlv->iSubItem == 6)	nType = SORT_ATTR;
	else if (pnmlv->iSubItem == 7)	nType = SORT_PATH;

	if (nType != -1) {
		int nSeq = (m_nSortType == nType) ? m_nSortSeq ^ TRUE : m_nSortSeq;
		SortList(nType, nSeq);
	}
}

void CMainFrame::OnLVNotifyGetInfoTip(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	NMLVGETINFOTIP* pgit = (NMLVGETINFOTIP*)pNmhdr;
	CArcItem* pai = m_array[pgit->iItem];
	if (pai && !pai->IsFolder()) {
		CString str;
		if (pai->m_strTips.IsEmpty()) {
			TCHAR buf[32];
			if (!pai->m_strType.IsEmpty()) {
				str.Format(_T("種類: %s"), (LPCTSTR)pai->m_strType);
				pai->m_strTips = str;
			}

			StrFormatByteSize(pai->m_dwSize, buf, ARRAYSIZE(buf));
			CString ratio;
			pai->FormatRatio(ratio);
			if (!pai->m_strTips.IsEmpty())
				pai->m_strTips += _T('\n');
			str.Format(_T("サイズ: %s(%s)"), buf, ratio.GetString());
			pai->m_strTips += str;

			if (!pai->m_strPath.IsEmpty()) {
				if (!pai->m_strTips.IsEmpty())
					pai->m_strTips += _T('\n');
				str.Format(_T("場所: %s"), pai->m_strPath.GetString());
				pai->m_strTips += str;
			}
		}
		str.Empty();
		if (pgit->dwFlags)
			str.Format(_T("%s\n"), pai->m_strName.GetString());
		str += pai->m_strTips;
		lstrcpyn(pgit->pszText, str, pgit->cchTextMax);
	}
}

BOOL CMainFrame::IsRowSelected(int iRow)
{
	return (m_list.GetItemState(iRow, LVIS_SELECTED) & LVIS_SELECTED);
}

void CMainFrame::UseOurHighlighting(NMLVCUSTOMDRAW* pcd, BOOL bUseOurs)
{
	m_list.SetItemState((int)pcd->nmcd.dwItemSpec, bUseOurs? 0: 0xff, LVIS_SELECTED);
	if (bUseOurs) {
		// Use our highlight colors
		pcd->clrText   = gColors[colRText];
		pcd->clrTextBk = gColors[colRBack];
	}
}

static COLORREF calc_mixed_color(COLORREF clrA, COLORREF clrB, int per)
{
	// (11a + 1b)/12
	int r = (per * GetRValue(clrA) + (100 - per) * GetRValue(clrB)) / 100;
	int g = (per * GetGValue(clrA) + (100 - per) * GetGValue(clrB)) / 100;
	int b = (per * GetBValue(clrA) + (100 - per) * GetBValue(clrB)) / 100;
	return RGB(r, g, b);
}

void CMainFrame::OnLVCustomDraw(NMHDR *pHdr, LRESULT *pResult)
{
	NMLVCUSTOMDRAW *lvcd = (NMLVCUSTOMDRAW *)pHdr;
	int iRow = (int)lvcd->nmcd.dwItemSpec;

	switch (lvcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		m_bRowHighlighted = (IsRowSelected(iRow) && ::GetFocus() == m_list.m_hWnd);
		if (m_bRowHighlighted)
			UseOurHighlighting(lvcd, TRUE);
		else {
			DWORD style = m_list.GetStyle() & LVS_TYPEMASK;

			lvcd->clrText = gColors[colText];
			if (style == LVS_REPORT && m_nStat & ATS_REPCHECKER && ((iRow & 1) == 0))
				lvcd->clrTextBk = calc_mixed_color(gColors[colBack], gColors[colRBack], 90);
			else
				lvcd->clrTextBk = gColors[colBack];
		}
		*pResult = CDRF_NOTIFYPOSTPAINT;
		break;

	case CDDS_ITEMPOSTPAINT:
		if (m_bRowHighlighted) {
			UseOurHighlighting(lvcd, FALSE);

			CRect rc;
			m_list.GetItemRect((int)lvcd->nmcd.dwItemSpec, &rc, LVIR_ICON);

			CArcItem* pai = (CArcItem*)m_list.GetItemData((int)lvcd->nmcd.dwItemSpec);

			DWORD style = m_list.GetStyle() & LVS_TYPEMASK;
			HIMAGELIST him;
			if (style == LVS_ICON) {
				him = m_himl;
				rc.left += (rc.Width() - m_sizeLarge.cx) / 2;
				rc.top += (rc.Height() - m_sizeLarge.cy) / 2;
			}
			else {
				him = m_himl;
				rc.left += (rc.Width() - m_sizeSmall.cx) / 2;
				rc.top += (rc.Height() - m_sizeSmall.cy) / 2;
			}
			ImageList_DrawEx((style == LVS_ICON) ? m_himl : m_hims,
						pai->m_iImage, lvcd->nmcd.hdc, rc.left, rc.top, 0, 0,
						CLR_NONE, gColors[colRBack], ILD_BLEND50 | ILD_TRANSPARENT);
		}
		*pResult = CDRF_DODEFAULT;
		break;

	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

DWORD CMainFrame::GetSelectedFileSize()
{
	DWORD dwSelect = 0;
	int cnt = m_list.GetSelectedCount();
	if (cnt > 0) {
		CArcItem *pai;
		int index = -1;
		while (1) {
			index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
			if (index < 0)
				break;
			pai = (CArcItem*)m_list.GetItemData(index);
			dwSelect += pai->m_dwSize;
		}
	}
	return dwSelect;
}

void CMainFrame::ShowTotalInfo()
{
	TCHAR buf[MAX_PATH], fmt[MAX_PATH], info[MAX_PATH];
	StrFormatByteSize64(m_parse.m_dlTotal, buf, ARRAYSIZE(buf));
	LoadString(RESINST, IDS_INDICATOR_TOTAL, fmt, ARRAYSIZE(fmt));
	wsprintf(info, fmt, m_list.GetItemCount(), buf);

	m_statusbar.GetPaneText(ID_INDICATOR_TOTAL, buf);
	if (StrCmp(buf, info))
		m_statusbar.SetPaneText(ID_INDICATOR_TOTAL, info);
}

void CMainFrame::ShowSelInfo()
{
	TCHAR txt[MAX_PATH], fmt[MAX_PATH], tot[MAX_PATH];
	StrFormatByteSize(m_dwSelect, tot, ARRAYSIZE(tot));
	LoadString(RESINST, IDS_INDICATOR_SELECT, fmt, ARRAYSIZE(fmt));
	wsprintf(txt, fmt, m_list.GetSelectedCount(), tot);

	m_statusbar.GetPaneText(ID_INDICATOR_SELECT, tot);
	if (StrCmp(tot, txt))
		m_statusbar.SetPaneText(ID_INDICATOR_SELECT, txt);
}

void CMainFrame::OnViewItem()
{
	TCHAR name[MAX_PATH * 2], dir[MAX_PATH * 2];
	if (TemporaryExtract(name, dir))
		ViewFile(m_hWnd, name, dir);
}

void CMainFrame::OnBDumpItem()
{
	TCHAR name[MAX_PATH * 2], dir[MAX_PATH * 2];
	if (TemporaryExtract(name, dir)) {
		PathAppend(dir, name);
		RunHexEditor(m_hWnd, dir);
	}
}

void CMainFrame::OnExecItem()
{
	int n = m_list.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
	if (n >= 0) {
		CArcItem* pai = (CArcItem*)m_list.GetItemData(n);
		if (!IsShowAll() && pai->IsFolder()) {
			SelectFolder(pai->m_strRealName);
			return;
		}
	}

	TCHAR name[MAX_PATH * 2], dir[MAX_PATH * 2];
	if (TemporaryExtract(name, dir))
		myShellExecute(m_hWnd, name, dir);
}

BOOL CMainFrame::TemporaryExtract(LPTSTR pszFile, LPTSTR pszDir)
{
	if (!pszFile || !pszDir || m_list.GetItemCount() <= 0)
		return FALSE;

	int n = m_list.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
	if (n < 0)
		return FALSE;

	CArcItem* pai = (CArcItem*)m_list.GetItemData(n);
	if (pai->IsFolder())
		return FALSE;

	lstrcpy(pszDir, m_szTempFolder);
	PathAddBackslash(pszDir);

	// ディレクトリを有効にしないで解凍する
	TCHAR name[MAX_PATH * 2], arc[MAX_PATH * 2], tmp[MAX_PATH * 2];
	lstrcpy(arc, m_parse.m_strFile);
	lstrcpy(name, pai->m_strRealName);
	lstrcpy(tmp, pszDir);
	PathQuoteSpaces(arc);
	PathQuoteSpaces(name);
	PathQuoteSpaces(tmp);

	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (FAILED(hr))
		return FALSE;

	CString cmd;
	switch (m_parse.m_nType) {
	case ARC_LZH:
		// 解凍状況を抑止する "-n"
		cmd.Format(_T("e -c1r2x0a2m1p1s0n1y1 -ga1 -+ %s %s %s"), arc, tmp, name);
		cmd.Replace('/', '\\');
		break;
	case ARC_ZIP:
		// 解凍状況を抑止（--i）し、常に上書き（-o）する
		cmd.Format(_T("-x -j --i -o %s %s %s"), arc, tmp, name);
		break;
	case ARC_ARJ:
		// 解凍状況抑止 "-i"   すべての質問にyesと仮定 "-y"
		cmd.Format(_T("e -hk -i -y %s %s %s"), arc, tmp, name);
		cmd.Replace('/', '\\');
		break;
	case ARC_CAB:
		// 解凍状況を抑止する "-i"
		cmd.Format(_T("-x -i -j -o %s %s %s"), arc, tmp, name);
		break;
	case ARC_TAR:
		// 解凍状況を抑止する --display-dialog=0
		cmd.Format(_T("--use-directory=0 --display-dialog=0 -xfo %s %s %s"), arc, tmp, name);
		break;
	case ARC_RAR:
// for unrar32.dll:		cmd.Format(_T("-e -o --i %s %s %s"), arc, tmp, name);
		cmd.Format(_T("x -o -i -j %s %s %s"), arc, tmp, name);
		break;
	case ARC_BGA:
		// 解凍状況を抑止する "-i"
		cmd.Format(_T("x -i -j -a -o %s %s %s"), arc, tmp, name);
		cmd.Replace('/', '\\');
		break;
	case ARC_YZ1:
		// 解凍状況を抑止する "-i"
		cmd.Format(_T("x -i %s %s %s"), arc, tmp, name);
		cmd.Replace('/', '\\');
		break;
	case ARC_GCA:
		// 解凍状況を抑止する "-i"
		cmd.Format(_T("-e -i -j %s %s %s"), arc, tmp, name);
		cmd.Replace('/', '\\');
		break;
	case ARC_7ZIP:
	case ARC_ISO:
	case ARC_RPM:
		// 解凍状況抑止 "-hide" 上書き "-aoa" 出力先 "-o<path>"
		cmd.Format(_T("e %s -hide -aoa -o%s %s"), arc, tmp, name);
		cmd.Replace('/', '\\');
		break;
	default:
		return FALSE;
	}

	CWaitCursor wc;
	hr = pam->ArchiveW(m_hWnd, m_parse.m_nType, cmd, NULL, 0, NULL);
	if (SUCCEEDED(hr)) {
		lstrcpy(pszFile, pai->m_strName);
		PathCombine(name, pszDir, pszFile);
		if (GetFileAttributes(name) != 0xFFFFFFFF) {
			m_tempfile.Add(name);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CMainFrame::ExtractItem(LPCTSTR pszExtractList)
{
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr))
		hr = pam->ExtractFromArchive(m_hWnd, m_parse.m_nType, m_parse.m_strFile, pszExtractList);
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

void CMainFrame::ExtractFolder(LPCTSTR pszFolder)
{
	if (!pszFolder || *pszFolder == 0)
		OnExtractAllItem();
	else {
		CString flt = pszFolder;
		flt.Replace(_T('\\'), _T('/'));
		flt.TrimRight(_T('/'));
		flt += _T("/*");

		CString str;
		if (MakeRespFileByFilter(str, flt) != (DWORD)-1) {
			str.Insert(0, _T('@'));
			ExtractItem(str);
		}
	}
}

void CMainFrame::OnExtractItem()
{
if (!user_ristriction()) return;

	if (m_split.GetActivePane() == 0) {
		LPCTSTR p = GetCurrentFolderPath();
		if (p)
			ExtractFolder(p);
	}
	else {
		int cnt = m_list.GetItemCount();
		if (cnt > 0) {
			CString str;
			if (MakeRespFile(str)) {
				str.Insert(0, _T('@'));
				ExtractItem(str);
			}
		}
	}
}

void CMainFrame::OnExtractAllItem()
{
if (!user_ristriction()) return;

	if (m_parse.GetItemCount() > 0) {
		CString flt = (m_parse.m_nType == ARC_7ZIP || m_parse.m_nType == ARC_ISO || m_parse.m_nType == ARC_RPM) ? _T("*") : _T("*.*");
		ExtractItem(flt);
	}
}

void CMainFrame::OnMakeSfx()
{
	m_parse.MakeSfx(m_hWnd);
}

void CMainFrame::DeleteFolder(LPCTSTR pszFolder)
{
	if (!pszFolder || *pszFolder == '\0' || !m_parse.CanDelete()) return;

	CString flt = pszFolder;
	flt.Replace(_T('\\'), _T('/'));
	flt.TrimRight(_T('/'));
	flt += _T("/*");
	CString str;
	if (MakeRespFileByFilter(str, flt) != (DWORD)-1) {
		if (AppMessage(m_hWnd, IDS_FOLDER_DELETE, pszFolder, MB_QEXCM) != IDOK)
			return;

		str.Insert(0, _T('@'));
		if (m_parse.DeleteItems(m_hWnd, str)) {
			// 削除後、書庫ファイルが存在しなければ終了する
			if (!PathFileExists(m_parse.m_strFile))
				PostMessage(WM_CLOSE);
			else {
				str = m_parse.m_strFile;
				OpenArcFile(str);
			}
		}
	}
}

void CMainFrame::OnDeleteItem()
{
	int cnt = m_list.GetSelectedCount();
	if (cnt < 1) {
		MessageBeep(0);
		return;
	}

	if (AppMessage(m_hWnd, IDS_FILE_DELETE, NULL, MB_QEXCM) != IDOK)
		return;

	CString str;
	if (!MakeRespFile(str))
		return;
	str.Insert(0, _T('@'));

	if (m_parse.DeleteItems(m_hWnd, str)) {
		// 削除後、書庫ファイルが存在しなければ終了する
		if (!PathFileExists(m_parse.m_strFile))
			PostMessage(WM_CLOSE);
		else {
			str = m_parse.m_strFile;
			OpenArcFile(str);
		}
	}
}

void CMainFrame::OnRenameItem()
{
	int cnt = m_list.GetSelectedCount();
	if (cnt < 1) {
		MessageBeep(0);
		return;
	}

	// コマンドライン引数のセットアップ
	CString str;
	if (!MakeRespFile(str))
		return;
	str.Insert(0, _T('@'));

	if (m_parse.RenameItems(m_hWnd, str)) {
		str = m_parse.m_strFile;
		OpenArcFile(str);
	}
}

void CMainFrame::OnClipItem()
{
	if (m_parse.ListToClipboard(m_hWnd))
		RunTextEditor(m_hWnd, _T("/clip"));
}

void CMainFrame::OnSelectAll()
{
	int cnt = m_list.GetItemCount();
	if (cnt < 1)
		return;

	m_list.SetRedraw(FALSE);
	for (int i = 0; i < cnt; i++)
		m_list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	m_list.SetRedraw(TRUE);
	m_list.Invalidate(FALSE);

	m_dwSelect = GetSelectedFileSize();
}

void CMainFrame::OnSelectRev()
{
	int cnt = m_list.GetItemCount();
	if (cnt < 1)
		return;

	m_list.SetRedraw(FALSE);
	for (int i = 0; i < cnt; i++) {
		if (m_list.GetItemState(i, LVIS_SELECTED))
			m_list.SetItemState(i, 0, LVIS_SELECTED);
		else
			m_list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}
	m_list.SetRedraw(TRUE);
	m_list.Invalidate(FALSE);

	m_dwSelect = GetSelectedFileSize();
}

void CMainFrame::ShowBoundary(int index, BOOL bShow)
{
	CHeaderCtrl header = m_list.GetHeader();

	HDITEM hd;
	hd.mask = HDI_FORMAT;
	header.GetItem(index, &hd);
	if (!bShow) {
		hd.mask = HDI_FORMAT;
		hd.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
	}
	else {
		hd.mask = HDI_FORMAT;
		hd.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
		hd.fmt |= (m_nSortSeq == SORT_UP) ? HDF_SORTUP : HDF_SORTDOWN;
	}
	header.SetItem(index, &hd);
}

void CMainFrame::SortList(int nSortType, int nSortSeq)
{
	int index = -1;
	if (m_nSortType != nSortType) {
		if      (m_nSortType == SORT_NAME)	index = 0;
		else if (m_nSortType == SORT_SIZE)	index = 1;
		else if (m_nSortType == SORT_TIME)	index = 2;
		else if (m_nSortType == SORT_EXT)	index = 3;
		else if (m_nSortType == SORT_ATTR)	index = 6;
		else if (m_nSortType == SORT_PATH)	index = 7;

		if (index != -1)
			ShowBoundary(index, FALSE);
	}
	m_nSortType = nSortType;
	m_nSortSeq = nSortSeq;

	index = -1;
	if (nSortType == SORT_NAME)			index = 0;
	else if (nSortType == SORT_SIZE)	index = 1;
	else if (nSortType == SORT_TIME)	index = 2;
	else if (nSortType == SORT_EXT)		index = 3;
	else if (nSortType == SORT_ATTR)	index = 6;
	else if (nSortType == SORT_PATH)	index = 7;

	if (index != -1)
		ShowBoundary(index, TRUE);

	CAppReg reg;
	reg.SetValue(IDS_SEC_OPERATION, IDS_KEY_SORTTYPE, m_nSortType);
	reg.SetValue(IDS_SEC_OPERATION, IDS_KEY_SORTSEQUENCE, m_nSortSeq);

	if (m_array.GetSize() > 1) {
		m_parse.SortItems(m_nSortType, m_nSortSeq);
		m_parse.CopyFolderGroup(m_array, m_curkey);
		ListupAgain();
		m_list.Invalidate(FALSE);
		m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
}

INT_PTR CALLBACK CMainFrame::EnumCallback(LPCTSTR pszName, LPARAM param)
{
	static int counter = 0;

	CMainFrame* me = (CMainFrame*)param;
	LPCTSTR p = NULL;
	if (pszName && *pszName) {
		p = StrRChr(pszName, NULL, _T('/'));
		if (!p)
			p = pszName;
		else
			p++;
	}
	if (p == NULL || counter > 1000 && (counter % 20) == 0)
		me->m_statusbar.SetPaneText(ID_DEFAULT_PANE, p);
	counter++;
	return 0;
}

BOOL CMainFrame::PerseArchive(LPCTSTR pszName)
{
	m_list.SetRedraw(FALSE);

	CRect rc;
	m_list.GetClientRect(&rc);

	// Loading animation
	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_WAITMARKER), 48, CLR_DEFAULT);
	AnimationImageList anim(him, 48);
	anim.InitAnimation(m_list);
	anim.SetPosition(rc.CenterPoint());
	anim.Start();

	if (!m_parse.EnumArchive(m_hWnd, pszName, EnumCallback, (LPARAM)this)) {
		anim.Stop();
		AppMessage(m_hWnd, IDS_ARC_NOTACCESS, pszName);
		return FALSE;
	}

	m_array.RemoveAll();
	m_list.DeleteAllItems();
	m_dwSelect = 0;

	InitFolder(PathFindFileName(pszName));

	int cnt = m_parse.m_folder.GetSize();
	for (int i = 0; i < cnt; i++) {
		DWORD_PTR key = m_parse.m_folder.GetValueAt(i);
		CString name = m_parse.m_folder.GetKeyAt(i);
		AddFolder(name, key);
	}

	anim.Stop();
	m_list.SetRedraw(TRUE);

	// リストボックスへの追加
	if (m_tree.IsWindowVisible())
		SelectGroup(0);
	else
		SelectGroup(0xFFFFFFFF);
	SetWindowText(pszName);

	InitFolderComplete();
	return TRUE;
}

LPCTSTR CMainFrame::GetCurrentFile()
{
	if (m_parse.m_strFile.IsEmpty())
		return NULL;
	return m_parse.m_strFile;
}

BOOL CMainFrame::AppendFiles(HDROP hDrop)
{
	BOOL bOK = m_parse.AppendFiles(m_hWnd, hDrop);
	if (bOK) {
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, m_parse.m_strFile);
		OpenArcFile(buf);
	}
	return bOK;
}

BOOL CMainFrame::IsShowAll()
{
	return m_curkey == 0xffffffff;
}

void CMainFrame::SelectGroup(DWORD_PTR folderid)
{
	m_curkey = folderid;
	m_array.RemoveAll();
	m_parse.SortItems(m_nSortType, m_nSortSeq);
	m_parse.CopyFolderGroup(m_array, m_curkey);
	ListupAgain(true);
	m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}


/////////////////////////////////////////////////////////////////////////////
// bookmark_view.cpp

#include "stdafx.h"
#include "fv7.h"
#include "mainfrm.h"
#include "shellexec.h"
#include "trackmenuwnd.h"


////////////////////////////////////////////////////////////////
//

LRESULT CBookmarkView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CMyPaneContainerImpl<CBookmarkView>::OnCreate(uMsg, wParam, lParam, bHandled);

	HICON hic = (HICON)LoadImage(RESINST, MAKEINTRESOURCE(IDI_STAR), IMAGE_ICON, 13, 13, LR_DEFAULTCOLOR);
	SetIcon(hic);

	CRect rc;
	GetClientRect(&rc);
	m_list.Create(m_hWnd, 0, NULL,
				WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_EDITLABELS |
				LVS_SHAREIMAGELISTS | LVS_NOCOLUMNHEADER,
				0, IDC_FAVORITELIST);
	SetClient(m_list);

	HIMAGELIST hl, hs;
	Shell_GetImageLists(&hl, &hs);
	m_list.SetImageList(hs, LVSIL_SMALL);
	m_list.InsertColumn(0, NULL, LVCFMT_LEFT, rc.Width(), 0);
	m_list.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE, LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE);
	UpdateProperty(DCM_CHANGEFORMAT);

	LoadList();

	// IMEを無効に
	ImmAssociateContext(m_hWnd, NULL);
	ImmAssociateContext(m_list, NULL);
	return 0;
}

LRESULT CBookmarkView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CMyPaneContainerImpl<CBookmarkView>::OnSize(uMsg, wParam, lParam, bHandled);
	CRect rc;
	m_list.GetClientRect(&rc);
	m_list.SetColumnWidth(0, rc.Width());
	return 0;
}

int CBookmarkView::OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message)
{
	LRESULT lr = DefWindowProc();
	if (lr != MA_NOACTIVATE && lr != MA_NOACTIVATEANDEAT) {		// frame does not want to activate
		if (nHitTest == HTCLIENT)
			fvApp.SetActiveView(m_hWnd);
	}
	return (int)lr;
}

void CBookmarkView::OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* ptvkd = (LV_KEYDOWN*)pNMHDR;
	*pResult = TRUE;
	int vkeyCtrl = _ISKEYDOWN(VK_CONTROL);
	int vkeyMenu = _ISKEYDOWN(VK_MENU);
	switch (ptvkd->wVKey) {
	case VK_UP:
		if (vkeyCtrl) {
			OnItemMoveUp();
			return;
		}
		break;
	case VK_DOWN:
		if (vkeyCtrl) {
			OnItemMoveDown();
			return;
		}
		break;
	case VK_DELETE:					// ファイル削除
		OnItemDelete();
		return;
	case VK_RETURN:					// 開く
		if (vkeyMenu)	OnItemOpenNew();
		else			OnItemOpen();
		return;
	}
	*pResult = FALSE;
}

void CBookmarkView::OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pNMHDR;
	CCmItem* pci = (CCmItem*)m_list.GetItemData(pnmlv->iItem);
	if (pci)
		delete pci;
	*pResult = 0;
}

void CBookmarkView::OnNotifyGetInfoTip(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	NMLVGETINFOTIP* pgit = (NMLVGETINFOTIP*)pNmhdr;
	CCmItem* pci = (CCmItem*)m_list.GetItemData(pgit->iItem);
	if (pci) {
		CString path;
		if (idl_GetPathFromIDList(pci->m_idl, path)) {
			CString str;
			str.Format(IDS_TIP_PLACE, path.GetString());
			lstrcpyn(pgit->pszText, str.GetString(), pgit->cchTextMax);
		}
	}
}

void CBookmarkView::OnNotifyDblClk(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	if (!(gnMosStat & MOS_FAVO_ONECLICK)) {
		if (_ISKEYDOWN(VK_MENU))
			OnItemOpenNew();
		else
			OnItemOpen();
	}
}

void CBookmarkView::OnNotifyBeginLabelEdit(NMHDR* /*pNmhdr*/, LRESULT* pResult)
{
	fvApp.EnableAccel(false);
	m_bLabelEditing = true;
	*pResult = FALSE;
}

void CBookmarkView::OnNotifyEndLabelEdit(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pdi = (LV_DISPINFO*)pNmhdr;
	if (pdi->item.pszText != NULL && lstrlen(pdi->item.pszText) > 0) {
		CCmItem* pci = (CCmItem*)m_list.GetItemData(pdi->item.iItem);
		pci->m_name = pdi->item.pszText;
		m_list.SetItemText(pdi->item.iItem, 0, pci->m_name);
		SaveList();
	}
	m_bLabelEditing = false;
	fvApp.EnableAccel(true);
}

void CBookmarkView::OnNotifyItemActivate(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
//@	NMITEMACTIVATE* pia = (NMITEMACTIVATE*)pNmhdr;
	if (gnMosStat & MOS_FAVO_ONECLICK) {
		if (_ISKEYDOWN(VK_MENU))
			OnItemOpenNew();
		else
			OnItemOpen();
	}
}

void CBookmarkView::OnNotifyCustomDraw(NMHDR* pHdr, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* lvcd = (NMLVCUSTOMDRAW*)pHdr;
	NMCUSTOMDRAW& nmcd = lvcd->nmcd;
	int iRow = (int)nmcd.dwItemSpec;

	switch (nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = (gnMosStat & MOS_HOTTRACK) ? CDRF_NOTIFYITEMDRAW : CDRF_DODEFAULT;
		break;

	case CDDS_ITEMPREPAINT:
		if (iRow == m_list.GetHotItem()) {
			lvcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
			lvcd->clrTextBk = mix_color(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 50);
		}
		*pResult = CDRF_NOTIFYPOSTPAINT;
		break;

	case CDDS_ITEMPOSTPAINT:
		if (m_insIndex == iRow) {
			CDCHandle dc = nmcd.hdc;
			CRect rc;
			m_list.GetItemRect(iRow, &rc, LVIR_BOUNDS);
			rc.bottom = rc.top + 2;
			rc.InflateRect(-3, 0);
			dc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), DSTINVERT);
		}
		*pResult = CDRF_DODEFAULT;
		break;

	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

void CBookmarkView::UpdateProperty(WPARAM wHint, LPARAM lHint)
{
	if (wHint == DCM_CHANGEFORMAT) {
		LOGFONT lf;
		if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_PANELFONT))
			_GCDATA.GetIconTitleLogFont(&lf);

		if (!m_font.IsNull()) m_font.DeleteObject();
		m_font.CreateFontIndirect(&lf);

		m_list.SetFont(m_font);
		m_list.SetTextColor(gColors[colNorm]);
		m_list.SetTextBkColor(gColors[colBack]);
		m_list.SetBkColor(gColors[colBack]);
		m_list.Invalidate(TRUE);
	}
	else if (wHint == DCM_UPDATEIMAGE) {
		int cnt = m_list.GetItemCount();
		for (int i = 0; i < cnt; i++) {
			CCmItem* pci = (CCmItem*)m_list.GetItemData(i);
			if (pci && pci->GetImageIndex() == lHint) {
				pci->SetImageIndex(-1);
				LVITEM lvi = { 0 };
				lvi.mask = LVIF_IMAGE;
				lvi.iItem = i;
				lvi.iImage = pci->GetImageIndex();
				m_list.SetItem(&lvi);
			}
		}
	}
}

void CBookmarkView::OnContextMenu(HWND /*hwnd*/, CPoint point)
{
	// クリック位置のインデクスを取得
	int index;
	if (MAKELONG(point.x, point.y) == (DWORD)-1)
		index = m_list.GetSelectedIndex();
	else {
		LVHITTESTINFO hit;
		hit.pt = point;
		m_list.ScreenToClient(&hit.pt);
		hit.flags = LVHT_ONITEM;
		index = m_list.HitTest(&hit);
	}
	if (index >= 0) {
		// 当該インデクスが未選択なら選択する
		if (index != m_list.GetSelectedIndex()) m_list.SelectItem(index);

		CRect rc;
		GetWindowRect(&rc);
		if (!rc.PtInRect(point)) {
			m_list.GetItemRect(index, &rc, LVIR_BOUNDS);
			point = rc.CenterPoint();
			ClientToScreen(&point);
		}
// 通常のコンテキストメニューはまずい
//		CCmItem* pci = (CCmItem*)m_list.GetItemData(index);
//		TrackContextMenuByIDList(m_hWnd, pci->m_pidlFq, &point);

		CTrackMenuWnd wnd;
		int cmd = wnd.DoTrack(m_hWnd, ID_MENU_FAVORITE, point);
		if (cmd > 0) PostMessage(WM_COMMAND, cmd);
	}
}

void CBookmarkView::OpenTabGroup(int index, CCmItem* pci)
{
	if (!pci->isFolder()) {
		// tabgroup
		CString str;
		if (pci->GetParseName(str)) {
			if (!_pathfileexists(str) || !fvApp.OpenTabGroup(str)) {
				if (AppMessage(IDS_INVALID_TABGROUP, pci->m_name, MB_QYESNO) == IDYES) {
					ItemDelete(index);
					fopRemoveFileNoConfirm(str);
				}
			}
		}
	}
}

void CBookmarkView::OnItemOpen()
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CCmItem* pci = (CCmItem*)m_list.GetItemData(index);
		if (pci) {
			if (pci->isFolder()) {
				// フォルダ
				if (_ISKEYDOWN(VK_CONTROL)) {
					// 非アクティブで開く。firefox, chrome 等がそのような動作
					fvApp.OpenTabFolder(pci->m_idl, FALSE);
				}
				else
					fvApp.OpenFolder(pci->m_idl);
			}
			else {
				// tabgroup
				OpenTabGroup(index, pci);
			}
		}
	}
}

void CBookmarkView::OnItemOpenNew()
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CCmItem* pci = (CCmItem*)m_list.GetItemData(index);
		if (pci)
			ExecNewFrame(pci->m_idl, _T("/force"));
	}
}

void CBookmarkView::OnItemTabNew()
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CCmItem* pci = (CCmItem*)m_list.GetItemData(index);
		if (pci) {
			if (pci->isFolder()) {
				// フォルダならアクティブで開く
				fvApp.OpenTabFolder(pci->m_idl, TRUE);
			}
			else {
				// tabgroup
				OpenTabGroup(index, pci);
			}
		}
	}
}

void CBookmarkView::OnItemRename()
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) m_list.EditLabel(index);
}

bool CBookmarkView::ItemDelete(int index)
{
	int numItems = m_list.GetItemCount();
	if (index < 0 || index >= numItems) return false;

	m_list.DeleteItem(index);
	index = min(m_list.GetItemCount() - 1, index);
	if (index >= 0)
		m_list.SetItemState(index, LVIS_FOCUSED, LVIS_FOCUSED);
	SaveList();
	return true;
}

void CBookmarkView::OnItemDelete()
{
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CCmItem* pci = (CCmItem*)m_list.GetItemData(index);
		if (AppMessage(IDS_QUERY_DELFAVOITEM, pci->m_name, MB_QYESNO) == IDYES)
			ItemDelete(index);
	}
}

void CBookmarkView::OnItemProperty()
{
	CWindow wndTop = GetTopLevelParent();
	int index = m_list.GetSelectedIndex();
	if (index >= 0) {
		CCmItem* pci = (CCmItem*)m_list.GetItemData(index);
		CComPtr<IContextMenu> pcm = (IContextMenu*)pci->GetUIObjectOf(wndTop, IID_IContextMenu);
		if (pcm.p != NULL) {
			ctx_Execute(wndTop, pcm, gszVerbProp);
		}
	}
}

void CBookmarkView::OnItemMoveUp()
{
	int sel = m_list.GetSelectedIndex();
	if (sel > 0) {
		CCmItem* pciSrc = (CCmItem*)m_list.GetItemData(sel);
		CCmItem* pciDst = (CCmItem*)m_list.GetItemData(sel - 1);
		m_list.SetItem(sel, 0, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,
						pciDst->m_name, pciDst->GetImageIndex(),
						0, 0,
						(LPARAM)pciDst);
		m_list.SetItem(sel - 1, 0, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE,
						pciSrc->m_name, pciSrc->GetImageIndex(),
						LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED,
						(LPARAM)pciSrc);
		SaveList();
	}
}

void CBookmarkView::OnItemMoveDown()
{
	int sel = m_list.GetSelectedIndex();
	if (sel >= 0 && sel < m_list.GetItemCount() - 1) {
		CCmItem* pciSrc = (CCmItem*)m_list.GetItemData(sel);
		CCmItem* pciDst = (CCmItem*)m_list.GetItemData(sel + 1);
		m_list.SetItem(sel, 0, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,
						pciDst->m_name, pciDst->GetImageIndex(),
						0, 0,
						(LPARAM)pciDst);
		m_list.SetItem(sel + 1, 0, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE,
						pciSrc->m_name, pciSrc->GetImageIndex(),
						LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED,
						(LPARAM)pciSrc);
		SaveList();
	}
}

static GUID bookmarkDefault[] = {
	FOLDERID_StartMenuAllPrograms,
	FOLDERID_Programs,
	FOLDERID_RoamingAppData,
	FOLDERID_LocalAppData,
	FOLDERID_LocalAppDataLow,
	FOLDERID_SendTo,
	FOLDERID_Favorites
};

void CBookmarkView::LoadList()
{
	m_list.SetRedraw(FALSE);
	m_list.DeleteAllItems();

	CFavoriteItemArray array;
	int num = array.Load();
	if (num > 0) {
		for (int i = 0; i < num; i++) {
			CCmItem* pci = array[i];
			int index = m_list.InsertItem(m_list.GetItemCount(), pci->m_name, pci->GetImageIndex());
			m_list.SetItemData(index, (DWORD_PTR)pci);
		}
		array.RemoveAll();
	} else if (num < 0) {
		// 初期アクセス
		CIDListPtr idl;
		if (idl.GetMyDocument()) {
			// まずMyDocumentを追加
			InsertItem(idl, 0);
			idl.Free();
		}

		num = ARRAYSIZE(bookmarkDefault);
		for (int i = 0; i < num; i++) {
			if (idl.GetKnownFolder(bookmarkDefault[i]) == S_OK) {
				InsertItem(idl, m_list.GetItemCount());
				idl.Free();
			}
		}
		num = m_list.GetItemCount();
		// 保存
		SaveList();
	}
	if (num > 0)
		m_list.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);
	m_list.SetRedraw(TRUE);
	m_list.Invalidate();
}

void CBookmarkView::SaveList()
{
	// ここでレジストリにリストを保存する
	CFavoriteItemArray array;
	int num = m_list.GetItemCount();
	for (int i = 0; i < num; i++) {
		CCmItem* pci = (CCmItem*)m_list.GetItemData(i);
		array.Add(pci);
	}
	array.Save();
	array.RemoveAll();

	::PostMessage(HWND_BROADCAST, nRM_FAVORITE, 0, (LPARAM)m_hWnd);
}

DWORD CBookmarkView::OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	SetDragScrollTimer(100);
	m_hotIndex = -1;
	m_insIndex = -1;
	return OnDragOver(pdo, keyState, dropEffect, point);
}

enum {
	SCRL_NONE = 0,
	SCRL_UP,
	SCRL_DOWN,
	SCRL_VISIBLE,
};

int CBookmarkView::CheckDragScroll(int index)
{
	int cnt = m_list.GetItemCount();
	if (cnt < 1) return SCRL_NONE;

	int top = m_list.GetTopIndex();
	int last = top + m_list.GetCountPerPage();

	if (index == top)	return (top > 0) ? SCRL_UP : SCRL_NONE;
	if (index < last)	return SCRL_NONE;

	return (index < cnt - 1) ? SCRL_DOWN : SCRL_VISIBLE;
}

void CBookmarkView::SetInsertionMark(int index)
{
	if ((m_insIndex == index) || (m_insIndex < 0 && index < 0))
		return;

	int tmp;
	if (m_insIndex >= 0) {
		if (index != m_insIndex) {
			tmp = m_insIndex;
			m_insIndex = -1;
			m_list.RedrawItems(tmp, tmp);
		}
	}

	m_insIndex = index;
	if (m_insIndex >= 0)
		m_list.RedrawItems(m_insIndex, m_insIndex);
}

DWORD CBookmarkView::OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	m_list.ScreenToClient(&point);

	UINT flags = 0;
	int index = m_list.HitTest(point, &flags);
	if (!(flags & (LVHT_ONITEMICON | LVHT_ONITEMLABEL)))
		index = -1;

	if (_ISKEYDOWN(VK_MENU)) {
		ShowDropHelper(FALSE);
		if (m_hotIndex >= 0) {
			m_list.SetItemState(m_hotIndex, 0, LVIS_DROPHILITED);
			m_hotIndex = -1;
		}

		SetInsertionMark(index);
		ShowDropHelper(TRUE);
		index = -1;
	}
	else {
		ShowDropHelper(FALSE);
		SetInsertionMark(-1);

		if (index != m_hotIndex) {
			if (m_hotIndex >= 0)
				m_list.SetItemState(m_hotIndex, 0, LVIS_DROPHILITED);
			if (index >= 0)
				m_list.SetItemState(index, LVIS_DROPHILITED, LVIS_DROPHILITED);
			m_hotIndex = index;
		}
		ShowDropHelper(TRUE);
	}

	if (IsTimeToDragScroll()) {
		ShowDropHelper(FALSE);
		switch (CheckDragScroll(index)) {
		case SCRL_UP:		m_list.SendMessage(WM_VSCROLL, SB_LINEUP, 0); break;
		case SCRL_DOWN:		m_list.SendMessage(WM_VSCROLL, SB_LINEDOWN, 0); break;
		case SCRL_VISIBLE:	m_list.EnsureVisible(index, FALSE); break;
		}
		ShowDropHelper(TRUE);
	}

	// VISTAのエアロD&Dテキスト
	CString strTargetDisplayName;

	CCmItem* pci;
	DROPEFFECT eff = DROPEFFECT_NONE;
	if (index >= 0) {
		pci = (CCmItem*)m_list.GetItemData(index);
		eff = idl_GetDropEffect(m_hWnd, pci->m_idl, pdo, keyState, dropEffect);
		strTargetDisplayName = pci->m_name;
	}
	else {
		eff = DROPEFFECT_LINK;
		strTargetDisplayName.LoadString(IDW_FAVORITE);
	}
	SetDropTargetDescription(pdo, strTargetDisplayName, eff);
	return eff;
}

void CBookmarkView::OnDragLeave()
{
	// ドロップ待機状態になっているパスを元に戻す
	if (m_hotIndex >= 0) {
		m_list.SetItemState(m_hotIndex, 0, LVIS_DROPHILITED);
		m_hotIndex = -1;
	}
	SetInsertionMark(-1);
}

DWORD CBookmarkView::OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	int index = m_hotIndex;
	TDropFiles df;
	df.Attach((HDROP)GetDataFromDataObject(pdo, CF_HDROP));

	CCmItem* pci;
	DROPEFFECT eff = DROPEFFECT_NONE;
	if (index >= 0) {
		CString path;
		pci = (CCmItem*)m_list.GetItemData(index);
		if ((IsDropFormatsSupported(SDO_HDROP|SDO_FILENAMEMAPA|SDO_FILENAMEMAPW) == SDO_HDROP) &&
			idl_GetPathFromIDList(pci->m_idl, path) &&
			(keyState & MK_LBUTTON) != 0)
		{
			if (df.GetCount() > 0) {
				BOOL bThreaded = FALSE;
				BOOL bAsAdmin = FALSE;
				if (QueryDrop(m_hWnd, pci->m_name, dropEffect, bThreaded, bAsAdmin)) {
					eff = dropEffect;
					AcceptFiles(df, eff, path, bThreaded);
				}
				return eff;
			}
		}
		eff = AcceptDataObject(m_hWnd, pci->m_idl, pdo, keyState, dropEffect, point);
	}
	else {
		// リストに追加
		index = (m_insIndex >= 0) ? m_insIndex : m_list.GetItemCount();
		SetInsertionMark(-1);

		CSimpleArray<PCIDLIST_ABSOLUTE> array;
		int cnt = GetIDListsFromDataObject(pdo, array);
		if (cnt > 0) {
			PCIDLIST_ABSOLUTE pidl;
			for (int i = 0; i < cnt; i++) {
				pidl = array[i];
				if (InsertItem(pidl, index))
					index++;
				CoTaskMemFree((void*)pidl);
			}
		}
		SaveList();
	}
	OnDragLeave();
	return DROPEFFECT_NONE;
}

bool CBookmarkView::InsertItem(LPCITEMIDLIST pidl, int iInsertAfter)
{
	if (pidl) {
		bool canadd = false;
		CCmItem* pci = new CCmItem(pidl, NULL);
		if (pci->isFolder())
			canadd = true;
		else if (pci->isFileSystem()) {
			// tabgroup
			CString str;
			if (pci->GetParseName(str) && !lstrcmpi(PathFindExtension(str), gszTabGroup)) {
				TCHAR buf[MAX_PATH];
				lstrcpy(buf, PathFindFileName(str));
				PathRemoveExtension(buf);
				pci->m_name = buf;
				canadd = true;
			}
		}
		if (canadd) {
			int index = m_list.InsertItem(iInsertAfter, pci->m_name, pci->GetImageIndex());
			if (index >= 0) {
				m_list.SetItemData(index, (DWORD_PTR)pci);
				m_list.EnsureVisible(index, FALSE);
				return true;
			}
		}
		delete pci;
	}
	return false;
}

LRESULT CBookmarkView::OnInsertItem(WPARAM wParam, LPARAM lParam)
{
	int iInsertAfter = ((int)wParam >= 0) ? (int)wParam  : m_list.GetItemCount();
	if (InsertItem((LPCITEMIDLIST)lParam, iInsertAfter)) {
		SaveList();
		return TRUE;
	}
	return FALSE;
}

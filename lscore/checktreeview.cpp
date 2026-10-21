// CheckTreeViewCtrl.cpp : implementation of the CCheckTreeViewCtrl class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "lscore.h"
#include "common.h"
#include "fnc.h"
#include "resource.h"
#include "checktreeview.h"

#define XIC				16
#define YIC				16

CCheckTreeViewCtrl::CCheckTreeViewCtrl()
	: m_hCurrentRootItem(NULL)
	, m_iSpacing(4)
{
	m_image = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_TREECHECK), XIC, CLR_NONE);
	// Theme
	SetThemeClassList(L"Button");
}

CCheckTreeViewCtrl::~CCheckTreeViewCtrl()
{
	m_array.DeleteAll();
}

int CCheckTreeViewCtrl::OnCreate(CREATESTRUCT* pcs)
{
	m_tree.Create(m_hWnd, NULL, NULL,
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
				TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | TVS_NOTOOLTIPS,
				(pcs->style & CTS_NOCLIENTEDGE) ? 0 : WS_EX_CLIENTEDGE,
				IDC_TREE, NULL);
	if (OsVISTA()) {
		DWORD exStyle = TVS_EX_DOUBLEBUFFER;
		TreeView_SetExtendedStyle(m_tree, exStyle, exStyle);
	}
	m_tree.SetImageList(m_image, TVSIL_NORMAL);

	ImmAssociateContext(m_hWnd, NULL);
	ImmAssociateContext(m_tree, NULL);
	return 0;
}

void CCheckTreeViewCtrl::OnDraw(HDC hdc)
{
	// 何もしない
}

void CCheckTreeViewCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	OnDraw(dc);
}

void CCheckTreeViewCtrl::OnPrintClient(HDC hdc, LPARAM flags)
{
	if (flags & PRF_ERASEBKGND)
		SendMessage(WM_ERASEBKGND, (WPARAM)hdc, 0);

	if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
		OnDraw(hdc);
}

void CCheckTreeViewCtrl::OnSize(UINT flag, int cx, int cy)
{
	if (flag != SIZE_MINIMIZED)
		m_tree.MoveWindow(0, 0, cx, cy, TRUE);
}

void CCheckTreeViewCtrl::OnSetFont(HFONT hFont, BOOL bRedraw)
{
	if (m_tree.IsWindow()) {
		if (!hFont) {
			hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
		}
		m_tree.SetFont(hFont, bRedraw);
		// 行間を設定
		ResetItemHeight(hFont);
	}
}

void CCheckTreeViewCtrl::ResetItemHeight(HFONT hFont)
{
	// 行間を設定
	CClientDC dc(nullptr);
	TEXTMETRIC tm;
	if (hFont == nullptr) {
		hFont = m_tree.GetFont();
	}
	dc.SelectFont(hFont);
	dc.GetTextMetrics(&tm);
	dc.SelectFont(hFont);

	int height = tm.tmHeight + tm.tmExternalLeading;
	if (!m_image.IsNull()) {
		int cx;
		int cy;
		m_image.GetIconSize(cx, cy);
		height = max(height, cy);
	}
	height += m_iSpacing;
	m_tree.SetItemHeight(height);
}


LRESULT CCheckTreeViewCtrl::OnPrePaint(LPNMTVCUSTOMDRAW ptvcd)
{
	return CDRF_NOTIFYITEMDRAW;
}

LRESULT CCheckTreeViewCtrl::OnItemPrePaint(LPNMTVCUSTOMDRAW ptvcd)
{
	HTREEITEM hItem = (HTREEITEM)ptvcd->nmcd.dwItemSpec;
	CCheckItem* pi = (CCheckItem*)m_tree.GetItemData(hItem);
	if (!pi) return CDRF_DODEFAULT;

	if (pi->isDisabled()) {
		ptvcd->clrText = GetSysColor(COLOR_GRAYTEXT);
		if (ptvcd->nmcd.uItemState & CDIS_SELECTED)
			ptvcd->clrTextBk = GetSysColor(COLOR_BTNFACE);
	}
	else {
		if (hItem == m_tree.GetHotItem()) {
			ptvcd->clrTextBk = mix_color(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_WINDOW), 50);
			ptvcd->clrText   = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
	}
	return CDRF_NOTIFYPOSTPAINT;
}

LRESULT CCheckTreeViewCtrl::OnItemPostPaint(LPNMTVCUSTOMDRAW ptvcd)
{
	HTREEITEM hItem = (HTREEITEM)ptvcd->nmcd.dwItemSpec;
	CCheckItem* pi = (CCheckItem*)m_tree.GetItemData(hItem);
	if (!pi) return CDRF_DODEFAULT;

//	bool bHot = (hItem == m_tree.GetHotItem());
	bool bHot = ((ptvcd->nmcd.uItemState & CDIS_HOT) != 0);

	CRect rcItem;
	m_tree.GetItemRect(hItem, &rcItem, TRUE);
	CRect rc = rcItem;
	rc.left -= (XIC + 3);
	rc.right = rc.left + XIC;
	CDCHandle dc = ptvcd->nmcd.hdc;

	int iImage;
	if (IsThemeNull()) {
		if (pi->isGroup())
			iImage = pi->isExpanded() ? 1 : 2;
		else if (pi->isEdit())
			iImage = 7;
		else {
			if (pi->isRadio())
				iImage = pi->isChecked() ? 6 : 5;
			else
				iImage = pi->isChecked() ? 4 : 3;
		}
		m_image.Draw(dc, iImage, rc.left, (rc.top + rc.bottom - YIC) / 2, ILD_TRANSPARENT);
	}
	else {
		if (pi->isGroup() || pi->isEdit()) {
			iImage = pi->isEdit() ? 7 : (pi->isExpanded() ? 1 : 2);
			m_image.Draw(dc, iImage, rc.left, (rc.top + rc.bottom - YIC) / 2, ILD_TRANSPARENT);
		}
		else {
			int part, state;
			if (pi->isRadio()) {
				part = BP_RADIOBUTTON;
				if (pi->isDisabled())
					state = pi->isChecked() ? RBS_CHECKEDDISABLED : RBS_UNCHECKEDDISABLED;
				else if (bHot)
					state = pi->isChecked() ? RBS_CHECKEDHOT : RBS_UNCHECKEDHOT;
				else
					state = pi->isChecked() ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;
			}
			else {
				part = BP_CHECKBOX;
				if (pi->isDisabled())
					state = pi->isChecked() ? CBS_CHECKEDDISABLED : CBS_UNCHECKEDDISABLED;
				else if (bHot)
					state = pi->isChecked() ? CBS_CHECKEDHOT : CBS_UNCHECKEDHOT;
				else
					state = pi->isChecked() ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
			}
			DrawThemeBackground(dc, part, state, &rc, NULL);
		}
	}

	// Draw the "follow" Text
	if (pi->m_tsFollow) {
		rc = rcItem;
		rc.left = rcItem.right;
		rc.right = rc.left + 200;
		int clr = dc.SetTextColor(RGB(0,0,224));
		dc.DrawText(pi->m_tsFollow, -1, &rc, DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);
		dc.SetTextColor(clr);
	}

	if (pi->isGroup()) {
		m_tree.GetClientRect(&rc);
		rc.left = rcItem.right;
		rc.top = rcItem.top;
		rc.bottom = rcItem.bottom;

		LOGBRUSH lb;
		lb.lbStyle = BS_SOLID;
		lb.lbColor = GetSysColor(COLOR_GRAYTEXT);
		lb.lbHatch = 0;
		HPEN hpen = dc.SelectPen(ExtCreatePen(PS_GEOMETRIC | PS_DASH, 1, &lb, 0, NULL));
		dc.MoveTo(rc.left, rc.top + rc.Height() / 2);
		dc.LineTo(rc.right, rc.top + rc.Height() / 2);
		DeleteObject(dc.SelectPen(hpen));
	}
	return CDRF_DODEFAULT;
}


void CCheckTreeViewCtrl::OnNotifyCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMTVCUSTOMDRAW ptvcd = (LPNMTVCUSTOMDRAW)pNMHDR;
	switch (ptvcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = OnPrePaint(ptvcd);
		break;
	case CDDS_ITEMPREPAINT:
		*pResult = OnItemPrePaint(ptvcd);
		break;
	case CDDS_ITEMPOSTPAINT:
		*pResult = OnItemPostPaint(ptvcd);
		break;
	default:
		*pResult = 0;
		break;
	}
}

int CCheckTreeViewCtrl::GetEditItemTitle(CCheckItem* pi, CString& str)
{
	str = _T("");
	if (!pi->isEdit()) return 0;

	if (pi->m_tsTitle.istextcallback()) {
		NMCTITEM nc;
		memset(&nc, 0, sizeof(nc));
		nc.hdr.hwndFrom = m_hWnd;
		nc.hdr.idFrom = GetDlgCtrlID();
		nc.hdr.code = CTN_GETDISPINFO;
		nc.item.id = pi->m_id;
		nc.item.state = pi->m_state;
		GetParent().SendMessage(WM_NOTIFY, nc.hdr.idFrom, (LPARAM)&nc);
		if (nc.item.pszText)
			str = nc.item.pszText;
	}
	else if (!pi->m_tsTitle.isempty())
		str = pi->m_tsTitle;

	if (!str.IsEmpty())
		str += _T(": ");

	return str.GetLength();
}

void CCheckTreeViewCtrl::EditItemText(CCheckItem* pi)
{
	CRect rc;
	m_tree.GetItemRect(pi->m_hItem, &rc, TRUE);

	CString str;
	if (GetEditItemTitle(pi, str) > 0) {
		CClientDC dc(m_hWnd);
		HFONT hfont = dc.SelectFont(m_tree.GetFont());
		CSize size;
		dc.GetTextExtent(str, str.GetLength(), &size);
		dc.SelectFont(hfont);
		rc.OffsetRect(size.cx, 0);
	}

	m_tree.ClientToScreen(&rc);
	TCHAR buf[MAX_PATH * 2];
	if (!pi->m_tsEdit.isempty())
		lstrcpy(buf, pi->m_tsEdit);
	else
		buf[0] = '\0';
	if (lccTipEdit(m_tree, &rc, buf)) {
		pi->SetText(buf);
		RedrawItem(pi->m_hItem);

		NMCTITEM nc;
		memset(&nc, 0, sizeof(nc));
		nc.hdr.hwndFrom = m_hWnd;
		nc.hdr.idFrom = GetDlgCtrlID();
		nc.hdr.code = CTN_EDITCHANGED;
		nc.item.id = pi->m_id;
		nc.item.state = pi->m_state;
		nc.item.pszText = (LPTSTR)(LPCTSTR)pi->m_tsEdit;
		GetParent().SendMessage(WM_NOTIFY, (WPARAM)nc.hdr.idFrom, (LPARAM)&nc);
	}
}

bool CCheckTreeViewCtrl::ToggleCheck(CCheckItem* pi)
{
	if (!pi || !pi->isButton() || pi->isRadio() || pi->isDisabled()) return false;

	pi->SetCheck(!pi->isChecked());
	RedrawItem(pi->m_hItem);

	NMCTITEM nc;
	memset(&nc, 0, sizeof(nc));
	nc.hdr.hwndFrom = m_hWnd;
	nc.hdr.idFrom = GetDlgCtrlID();
	nc.hdr.code = CTN_CHECKCHANGED;
	nc.item.id = pi->m_id;
	nc.item.state = pi->m_state;

	GetParent().SendMessage(WM_NOTIFY, (WPARAM)nc.hdr.idFrom, (LPARAM)&nc);
	return true;
}

bool CCheckTreeViewCtrl::RadioCheck(CCheckItem* pi)
{
	if (!pi || !pi->isButton() || !pi->isRadio() || pi->isDisabled()) return false;
	if (pi->isChecked()) return true;

	CCheckItem* piw = m_array.GetRadioGroupTopItem(pi->m_id);
	while (piw) {
		if (piw == pi) {
			piw->SetCheck(true);
			RedrawItem(piw->m_hItem);
		}
		else if (piw->isChecked()) {
			piw->SetCheck(false);
			RedrawItem(piw->m_hItem);
		}
		piw = m_array.GetRadioGroupNextItem(piw->m_id);
	}

	NMCTITEM nc;
	memset(&nc, 0, sizeof(nc));
	nc.hdr.hwndFrom = m_hWnd;
	nc.hdr.idFrom = GetDlgCtrlID();
	nc.hdr.code = CTN_CHECKCHANGED;
	nc.item.id = pi->m_id;
	nc.item.state = pi->m_state;

	GetParent().SendMessage(WM_NOTIFY, (WPARAM)nc.hdr.idFrom, (LPARAM)&nc);
	return true;
}

void CCheckTreeViewCtrl::RedrawItem(HTREEITEM hItem)
{
	RECT rc;
	if (m_tree.GetItemRect(hItem, &rc, FALSE))
		m_tree.InvalidateRect(&rc, TRUE);
}

void CCheckTreeViewCtrl::OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	TVHITTESTINFO ht;
	GetCursorPos(&ht.pt);
	ScreenToClient(&ht.pt);
	HTREEITEM hItem = m_tree.HitTest(&ht);
	if (hItem) {
		CCheckItem* pi = (CCheckItem*)m_tree.GetItemData(hItem);
		if (!pi) return;

		if (pi->isButton() && (TVHT_ONITEM & ht.flags)) {
			if (pi->isRadio())
				RadioCheck(pi);
			else
				ToggleCheck(pi);
		}
		else if (pi->isEdit() && m_tree.GetSelectedItem() == pi->m_hItem && (TVHT_ONITEMLABEL & ht.flags))
			EditItemText(pi);
	}
}

void CCheckTreeViewCtrl::OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_KEYDOWN* ptvkd = (TV_KEYDOWN*)pNMHDR;
	*pResult = FALSE;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if (hItem) {
		CCheckItem* pi = (CCheckItem*)m_tree.GetItemData(hItem);
		if (!pi || pi->isGroup()) return;

		if (pi->isEdit()) {
			if (ptvkd->wVKey != VK_SPACE && ptvkd->wVKey != VK_RIGHT) return;
			EditItemText(pi);
		}
		else {
			if (ptvkd->wVKey != VK_SPACE) return;

			if (pi->isRadio())
				RadioCheck(pi);
			else
				ToggleCheck(pi);
		}
	}
	*pResult = TRUE;
}

void CCheckTreeViewCtrl::OnNotifyItemExpanded(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pNMHDR;
	CCheckItem* pi = (CCheckItem*)m_tree.GetItemData(pnmtv->itemNew.hItem);
	if (pi) {
		pi->m_state = (pi->m_state & ~CTIS_EXPANDED) | ((pnmtv->action == TVE_COLLAPSE) ? CTIS_EXPANDED : 0);
		RedrawItem(pnmtv->itemNew.hItem);
	}
	*pResult = 0;
}

void CCheckTreeViewCtrl::OnNotifyGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFO* pd = (NMTVDISPINFO*)pNMHDR;
	if (pd->item.mask & TVIF_TEXT) {
		CCheckItem* pi = (CCheckItem*)m_tree.GetItemData(pd->item.hItem);
		if (pi) {
			if (pi->isEdit()) {
				CString str;
				GetEditItemTitle(pi, str);
				if (!pi->m_tsEdit.isempty())
					str += pi->m_tsEdit;
				lstrcpyn(pd->item.pszText, str, pd->item.cchTextMax);
			}
			else {
				NMCTITEM nc;
				memset(&nc, 0, sizeof(nc));
				nc.hdr.hwndFrom = m_hWnd;
				nc.hdr.idFrom = GetDlgCtrlID();
				nc.hdr.code = CTN_GETDISPINFO;
				nc.item.id = pi->m_id;
				nc.item.state = pi->m_state;
				GetParent().SendMessage(WM_NOTIFY, nc.hdr.idFrom, (LPARAM)&nc);
				if (nc.item.pszText) {
					pd->item.pszText = nc.item.pszText;
					pd->item.cchTextMax = lstrlen(nc.item.pszText);
				}
			}
		}
	}
	*pResult = 0;
}

LRESULT CCheckTreeViewCtrl::OnAddItem(WPARAM wParam, LPARAM lParam)
{
	CTITEM* pc = (CTITEM*)lParam;
	if (!pc) return (LRESULT)-1;

	if (!m_hCurrentRootItem)
		m_hCurrentRootItem = TVI_ROOT;

	TVINSERTSTRUCT is;
	memset(&is, 0, sizeof(is));
	is.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	is.item.iImage = is.item.iSelectedImage = 0;
	is.item.pszText = pc->pszText;
	is.hInsertAfter = TVI_LAST;
	if (pc->idGroup == CTGROUP_CURRENT)
		is.hParent = m_hCurrentRootItem;
	else if (pc->idGroup == CTGROUP_ROOT)
		is.hParent = TVI_ROOT;
	else if (pc->idGroup == CTGROUP_TOP)
		is.hParent = m_tree.GetRootItem();
	else if (pc->idGroup == CTGROUP_PARENT) {
		HTREEITEM hParent = m_tree.GetParentItem(m_hCurrentRootItem);
		is.hParent = (hParent == NULL) ? TVI_ROOT : hParent;
	}
	else {
		CCheckItem* piParent = m_array.Lookup(pc->idGroup);
		is.hParent = piParent ? piParent->m_hItem : m_tree.GetRootItem();
	}

	HTREEITEM hItem = NULL;
	if (pc->state & CTIS_GROUP) {
		is.item.mask |= TVIF_STATE;
		is.item.state = TVIS_BOLD;
		is.item.stateMask = TVIS_BOLD;
		hItem = m_tree.InsertItem(&is);
		m_hCurrentRootItem = hItem;
	}
	else {
		if (pc->state & CTIS_EDIT)
			is.item.pszText = LPSTR_TEXTCALLBACK;
		hItem = m_tree.InsertItem(&is);
		m_hCurrentRootItem = is.hParent;
	}

	CCheckItem* pi = new CCheckItem(hItem, pc->id, pc->state, pc->pszText);
	if (pi->m_tsFollow)
		m_tree.SetItemText(hItem, pi->m_tsTitle);
	m_array.Add(pi);
	m_tree.SetItemData(hItem, (DWORD_PTR)pi);
	m_tree.Expand(is.hParent, TVE_EXPAND);
	return (LRESULT)pi->m_index;
}

LRESULT CCheckTreeViewCtrl::OnGetItemState(WPARAM wParam, LPARAM lParam)
{
	UINT id = (UINT)wParam;
	UINT mask = (UINT)lParam;
	CCheckItem* pi = m_array.Lookup(id);
	return ((pi != NULL) ? (LRESULT)(mask & pi->m_state) : 0);
}

LRESULT CCheckTreeViewCtrl::OnSetItemState(WPARAM wParam, LPARAM lParam)
{
	CTITEM* pc = (CTITEM*)lParam;
	if (!pc) return FALSE;

	CCheckItem* pi = m_array.Lookup(pc->id);
	if (pi) {
		UINT newState = (pi->m_state & ~pc->stateMask) | pc->state;
		UINT chgState = newState ^ pi->m_state;
		if (chgState != 0) {
			if (chgState & CTIS_GROUP)
				m_tree.SetItemState(pi->m_hItem, (newState & CTIS_GROUP) ? TVIS_BOLD : 0, TVIS_BOLD);
			pi->m_state = newState;
			RedrawItem(pi->m_hItem);
		}
		return TRUE;
	}
	return FALSE;
}

LRESULT CCheckTreeViewCtrl::OnGetItemText(WPARAM wParam, LPARAM lParam)
{
	int len = -1;
	UINT id = (UINT)wParam;
	LPTSTR psz = (LPTSTR)lParam;
	CCheckItem* pi = m_array.Lookup(id);
	if (pi && pi->isEdit()) {
		len = (!pi->m_tsEdit.isempty()) ? lstrlen(pi->m_tsEdit) : 0;
		if (len && psz)
			lstrcpy(psz, pi->m_tsEdit);
	}
	return (LRESULT)len;
}

LRESULT CCheckTreeViewCtrl::OnSetItemText(WPARAM wParam, LPARAM lParam)
{
	UINT id = (UINT)wParam;
	LPTSTR psz = (LPTSTR)lParam;
	CCheckItem* pi = m_array.Lookup(id);
	if (pi && pi->isEdit()) {
		pi->SetText(psz);
		RedrawItem(pi->m_hItem);
		return TRUE;
	}
	return FALSE;
}

LRESULT CCheckTreeViewCtrl::OnSelectItem(WPARAM wParam, LPARAM lParam)
{
	UINT id = (UINT)wParam;
	CCheckItem* pi = m_array.Lookup(id);
	return (pi ? m_tree.SelectItem(pi->m_hItem) : FALSE);
}

LRESULT CCheckTreeViewCtrl::OnGetSpacing(WPARAM, LPARAM)
{
	return (LRESULT)m_iSpacing;
}

LRESULT CCheckTreeViewCtrl::OnSetSpacing(WPARAM wParam, LPARAM lParam)
{
	int prevSpacing = m_iSpacing;
	if ((int)lParam >= 0 && (int)lParam < 300) {
		m_iSpacing = (int)lParam;
		ResetItemHeight();
	}
	return (LPARAM)prevSpacing;
}


////////////////////////////////////////////////////////////////////////
// CCheckTreeViewWndFn WindowProc

extern "C"
LRESULT CALLBACK CheckTreeViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CCheckTreeViewCtrl* pWnd = new CCheckTreeViewCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}


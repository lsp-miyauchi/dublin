// drawlist.cpp : CDrawListView クラスの実装
//

#include "stdafx.h"
#include <limits.h>
#include "lscore.h"
#include "fnc.h"
#include "common.h"
#include "dlview.h"

#define MAX_URL_LENGTH			(2048 + 32 + 8)

#define WM_SYSKEYFIRST			WM_SYSKEYDOWN
#define WM_SYSKEYLAST			WM_SYSDEADCHAR
#define WM_NCMOUSEFIRST			WM_NCMOUSEMOVE
#define WM_NCMOUSELAST			WM_NCMBUTTONDBLCLK

#define _ISKEYDOWN(vkey)		(GetKeyState((vkey)) < 0)
#define _INRANGE(a,b,c)			((b) <= (a) && (a) <= (c))
#define _INRANGEEX(a,b,c)		((b) < (a) && (a) < (c))
#define _OUTRANGE(a,b,c)		((a) < (b) || (a) > (c))
#define ISVKEYALPHA(a)			(_INRANGE((a), 'A', 'Z'))
#define ISVKEYNUM(a)			(_INRANGE((a), '0', '9'))
#define ISVKEYNUMPAD(a)			(_INRANGE((a), VK_NUMPAD0, VK_NUMPAD9))
#define ISVKEYDIGIT(a)			(ISVKEYNUM((a)) || ISVKEYNUMPAD((a)))
#define ISVKEYALNUM(a)			(ISVKEYALPHA((a)) || ISVKEYDIGIT((a)))
#define REPORT_HSCROLL_DELTA	(GetSystemMetrics(SM_CXVSCROLL))


////////////////////////////////////////////////////////////////////////
// CDrawListView WindowProc

extern "C"
LRESULT CALLBACK DrawListViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CDrawListView * pWnd = new CDrawListView();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}


////////////////////////////////////////////////////////////////////////
// CDrawListView

CDrawListView::CDrawListView()
: m_col(0)
, m_row(0)
, m_selindex(0)
, m_ancindex(0)
, m_dropindex(-1)
, m_hoverindex(-1)
, m_prevhoverindex(-1)
, m_bInsideUpdate(false)
, m_hit(-1)
, m_exStyle(DLS_EX_CONTEXTMENU)
, m_listStyle(DLS_STYLE_ICON)
, m_bCaptured(false)
, m_bTrackMouseEvent(FALSE)
, m_horzExtent(0)
, m_clrBkgnd(INVALID_COLOR)
, m_numWheelScrollLines(0)
{
	m_sizeItem = CSize(64, 64);
	m_sizeSpacing = CSize(0, 0);
	m_sizeClient = CSize(0, 0);
	LOGFONT lf;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
	m_hfont = CreateFontIndirect(&lf);
	m_array.SetCount(0);

	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, sizeof(UINT), &m_numWheelScrollLines, FALSE);

	// Theme
	SetThemeClassList(L"Edit");
	SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE, 0);
}

CDrawListView::~CDrawListView()
{
	m_array.RemoveAll();
	DeleteObject(m_hfont);
}

// CDrawListView メッセージ ハンドラ

void CDrawListView::OnPaint()
{
	COLORREF clr = (m_clrBkgnd == INVALID_COLOR) ? GetSysColor(COLOR_WINDOW) : m_clrBkgnd;
	CPaintDC dc(m_hWnd);

	int cnt = GetCount();
	if (cnt < 1) {
		EraseBkgnd(dc);
		return;
	}

	// ビットマップ作成のためにアイテムサイズを求める
	CSize sizeItem = GetItemSize();

	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hbm = CreateCompatibleBitmap(dc, sizeItem.cx, sizeItem.cy);
	ATLASSERT(hbm != NULL);
	hbm = dcw.SelectBitmap(hbm);

	HFONT hf = dcw.SelectFont(m_hfont);

	DRAWITEMSTRUCT ds;
	memset(&ds, 0, sizeof(ds));
	ds.CtlID = GetDlgCtrlID();
	ds.CtlType = ODT_LISTBOX;
	ds.hDC = dcw;
	ds.hwndItem = m_hWnd;
	ds.itemAction = ODA_DRAWENTIRE;
	SetRect(&ds.rcItem, 0, 0, sizeItem.cx, sizeItem.cy);

	int cxItem = sizeItem.cx;
	int cyItem = sizeItem.cy;
	if (IsIconStyle()) {
		cxItem += m_sizeSpacing.cx * 2;
		cyItem += m_sizeSpacing.cy * 2;
	}

	CPoint topPos(0, 0);
	GetAbsPos(&topPos);

	CRect rc;
	int i = GetTopIndex();
	if (i < 0) i = 0;
	for (; i < cnt; i++) {
		if (IsListStyle()) {
			rc.left   = (i / m_row) * cxItem;
			rc.top    = (i % m_row) * cyItem;
		}
		else {
			rc.left   = (i % m_col) * cxItem;
			rc.top    = (i / m_col) * cyItem;
		}
		rc.right  = rc.left + cxItem;
		rc.bottom = rc.top + cyItem;
		rc.OffsetRect(-topPos.x, -topPos.y);

		if (dc.RectVisible(&rc)) {
			if (IsIconStyle())
				rc.DeflateRect(m_sizeSpacing.cx, m_sizeSpacing.cy);

			dcw.FillSolidRect(&ds.rcItem, clr);
			CDLItem& di = m_array[i];
			ds.itemID = (UINT)i;
			ds.itemState = 0;
			ds.itemData = (ULONG_PTR)di.m_data;
			ds.itemState = di.m_state;
			if (i == m_selindex)					ds.itemState |= ODS_FOCUS;
			if (i == m_dropindex)					ds.itemState |= ODS_SELECTED;
			if (IsExHover() && i == m_hoverindex)	ds.itemState |= ODS_HOTLIGHT;

			int odd = ((i / m_col) & 1) + ((i % m_col) & 1);
			if (odd == 1)							ds.itemState |= ODS_ODDPOSITION;

			int save = dcw.SaveDC();
			m_owner.SendMessage(WM_DRAWITEM, ds.CtlID, (LPARAM)&ds);
			dcw.RestoreDC(save);
			dc.BitBlt(rc.left, rc.top, sizeItem.cx, sizeItem.cy, dcw, 0, 0, SRCCOPY);
			dc.ExcludeClipRect(&rc);
		}
	}

	dcw.SelectFont(hf);
	DeleteObject(dcw.SelectBitmap(hbm));

	EraseBkgnd(dc);
}

int CDrawListView::OnCreate(LPCREATESTRUCT pcs)
{
	ImmAssociateContext(m_hWnd, NULL);
	m_owner = GetParent();
	return 0;
}

void CDrawListView::OnSize(UINT nType, int cx, int cy)
{
	m_sizeClient = CSize(cx, cy);
	CalcBars();
//@	EnsureVisible(m_selindex, false);
}

BOOL CDrawListView::EraseBkgnd(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);
	COLORREF clr = (m_clrBkgnd == INVALID_COLOR) ? GetSysColor(COLOR_WINDOW) : m_clrBkgnd;
	clr = ::SetBkColor(hdc, clr);
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	::SetBkColor(hdc, clr);
	return TRUE;
}

BOOL CDrawListView::OnEraseBkgnd(HDC hdc)
{
	return TRUE;
}

void CDrawListView::OnNotifyToolTipText(int id, NMHDR* phdr, LRESULT* pResult)
{
	static TCHAR text[MAX_URL_LENGTH];

	CDLItem* item = GetItem(m_hit);
	if (TTIsEnabled() && item) {
		NMTTDISPINFO* ptt = (NMTTDISPINFO*)phdr;
		text[0] = 0;

		if (item->m_tiptext.IsEmpty()) {
			NMDLGETINFOTIP git;
			git.hdr.hwndFrom = m_hWnd;
			git.hdr.idFrom = GetDlgCtrlID();
			git.hdr.code = DLN_GETINFOTIP;
			git.dwFlags = DLGIT_UNFOLDED;
			git.pszText = text;
			git.cchTextMax = ARRAYSIZE(text);
			git.iItem = m_hit;
			git.lParam = (LPARAM)item->m_data;
			m_owner.SendMessage(WM_NOTIFY, git.hdr.idFrom, (LPARAM)&git);
		}
		else
			lstrcpy(text, item->m_tiptext);

		if (text[0])
			ptt->lpszText = text;
	}
}

bool CDrawListView::SetInfoTip(NMDLSETINFOTIP* pInfoTipStruct)
{
	if (pInfoTipStruct && pInfoTipStruct->cbSize == sizeof(NMDLSETINFOTIP)) {
		CDLItem* item = GetItem(pInfoTipStruct->iItem);
		if (item) {
			item->SetTipText(pInfoTipStruct->pszText);
			return true;
		}
	}
	return false;
}

BOOL CDrawListView::MouseButtonDown(UINT nFlags, CPoint point)
{
	if (::GetFocus() != m_hWnd) ::SetFocus(m_hWnd);
	int index = ItemFromPoint(point);

	// オーナーウィンドウにnotify
	NMDLCLICK dlc;
	memset(&dlc, 0, sizeof(dlc));
	dlc.hdr.code = DLN_CLICK;
	dlc.hdr.hwndFrom = m_hWnd;
	dlc.hdr.idFrom = GetDlgCtrlID();
	dlc.iItem = index;
	dlc.typeClick = nFlags;
	dlc.ptAction = point;
	if (m_owner.SendMessage(WM_NOTIFY, dlc.hdr.idFrom, (LPARAM)&dlc))
		return TRUE;

	if ((nFlags & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) == MK_MBUTTON)
		return TRUE;

	CPoint pt = point;
	ClientToScreen(&pt);

	bool vkeyShift = _ISKEYDOWN(VK_SHIFT);
	bool vkeyCtrl = _ISKEYDOWN(VK_CONTROL);
	bool vkeyAlt = _ISKEYDOWN(VK_MENU);			// ALTキー併用であれば無条件に矩形選択モード
	if (IsSingleSel())
		vkeyShift = vkeyCtrl = vkeyAlt = false;
	else {
		// 右下隅の矩形をクリックしたらdrag-select開始
		CRect rc;
		GetItemRect(index, &rc);
		rc.top = rc.bottom - 8;
		rc.left = rc.right - 8;
		if (!vkeyAlt && rc.PtInRect(point))
			vkeyAlt = true;
	}

	bool bSelect;
	if (!vkeyAlt && index >= 0) {
		// インデクス項目が選択されていなければ先に選択
		bSelect = GetSel(index);
		if (!bSelect)
			SetSelections(index);
		SetCaretIndex(index, false);

		if (!vkeyShift)
			SetAnchorIndex(index);

		// DRAG & DROP
		if (lccBeginTrack(m_hWnd, pt)) {
			CDLItem& di = m_array[index];

			// 親ウィンドウへDRAG&DROP通知
			NMDLVIEW dlv;
			memset(&dlv, 0, sizeof(dlv));
			dlv.hdr.code = (nFlags & MK_LBUTTON) ? DLN_BEGINDRAG : DLN_BEGINRDRAG;
			dlv.hdr.hwndFrom = m_hWnd;
			dlv.hdr.idFrom = GetDlgCtrlID();
			dlv.iItem = index;
			dlv.uNewState = di.m_state;
			dlv.uOldState = di.m_state;
			dlv.lParam = (LPARAM)di.m_data;
			m_owner.SendMessage(WM_NOTIFY, dlv.hdr.idFrom, (LPARAM)&dlv);
			return TRUE;
		}

		if (nFlags & MK_LBUTTON) {
			if (!vkeyShift) {
				if (vkeyCtrl)
					SetSel(index, !bSelect);
				else
					SelectIndex(index);
			}
			else 
				SetSelections(index);
		}
	}
	else {
		if (IsSingleSel()) return FALSE;

		// 矩形選択の開始
		if (vkeyAlt && index >= 0) {
			// インデクス項目が選択されていなければ先に選択
			bSelect = GetSel(index);
			if (!bSelect)
				SetSelections(index);
			SetCaretIndex(index, false);

			if (!vkeyShift)
				SetAnchorIndex(index);
		}

		if (lccBeginTrack(m_hWnd, pt)) {
			// DRAG SELECT
			if (nFlags & MK_LBUTTON) {
				SetSel(-1, false);
				SetAnchorIndex(m_selindex);

				m_dragancindex = m_ancindex;
				m_dragbefindex = m_selindex;
				m_selindex = -1;
				m_bCaptured = true;
				SetCapture();
				m_ptSel = point;
				GetAbsPos(&m_ptSel);
				m_rcSel.SetRect(m_ptSel.x, m_ptSel.y, m_ptSel.x, m_ptSel.y);
				return TRUE;
			}
		}
		else {
			if (!vkeyAlt && nFlags & MK_LBUTTON) {
				index = m_selindex;
				SetSel(-1, false);
				SetAnchorIndex(index);
//@				SetCaretIndex(index, false);
				return TRUE;
			}
		}
	}

	if (IsExOneClickActivate() && nFlags & MK_LBUTTON &&
		index >= 0 && !vkeyShift && !vkeyCtrl) {
		StandardParentNotify(NM_DBLCLK);
	}
	return FALSE;
}

void CDrawListView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (!MouseButtonDown(nFlags, point) && IsExContextMenu()) {
		ClientToScreen(&point);
		m_owner.SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
	}
}

void CDrawListView::OnMButtonDown(UINT nFlags, CPoint point)
{
	MouseButtonDown(nFlags, point);
}

void CDrawListView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bCaptured)
		return;

	if (m_rcSel.Width() && m_rcSel.Height()) {
		CPoint pt(0, 0);
		GetAbsPos(&pt);

		CRect rc = m_rcSel;
		rc.OffsetRect(-pt.x, -pt.y);

		HDC hdc = GetDC();
		DrawFocusRect(hdc, &rc);
		ReleaseDC(hdc);
	}
	m_selindex = m_dragancindex;
	ReleaseCapture();
	m_bCaptured = false;
}

void CDrawListView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	StandardParentNotify(NM_DBLCLK);
}

void CDrawListView::OnMouseMove(UINT nFlags, CPoint point)
{
	int index = ItemFromPoint(point);
	if (index != m_hit) {
		TTActivate(FALSE);
		m_hit = index;
		if (index >= 0 && TTIsEnabled()) {
			TTActivate(TRUE);
		}
	}

	if (!m_bCaptured) {
		if (IsExHover()) {
			if (!m_bTrackMouseEvent) {
				TRACKMOUSEEVENT te;
				te.cbSize = sizeof(te);
				te.dwFlags = TME_HOVER | TME_LEAVE;
				te.hwndTrack = m_hWnd;
				te.dwHoverTime = HOVER_DEFAULT;
				m_bTrackMouseEvent = _TrackMouseEvent(&te);
			}

			if (m_bTrackMouseEvent) {
				if (m_hoverindex != index) {
					if (IsExHover()) {
						int oldhover = m_hoverindex;
						m_hoverindex = index;
						CRect rc;
						if (oldhover >= 0) {
							if (GetItemRect(oldhover, &rc) != -1)
								RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
						}
						if (index >= 0) {
							if (GetItemRect(index, &rc) != -1)
								RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
						}
					}
				}
			}
			m_hoverindex = index;
		}
		return;
	}

	CPoint ptScroll(0, 0);
	GetAbsPos(&ptScroll);

	CRect rcc(0, 0, m_sizeClient.cx, m_sizeClient.cy);
	CRect rc(m_ptSel.x, m_ptSel.y, point.x + ptScroll.x, point.y + ptScroll.y);

	// 選択方向
	bool bBottomDirection = IsListStyle() ? (rc.left <= rc.right) : (rc.top <= rc.bottom);
	rc.NormalizeRect();
	if (rc.left < 0) rc.left = 0;
	if (rc.top < 0) rc.top = 0;
	if (rc == m_rcSel) return;

	HDC hdc = GetDC();
	CRect rcw;
	if (m_rcSel.Width() && m_rcSel.Height()) {
		rcw = m_rcSel;
		rcw.OffsetRect(-ptScroll.x, -ptScroll.y);
		DrawFocusRect(hdc, &rcw);
	}

	// 選択項目の更新
	CSize sizeItem = GetItemSize();
	if (IsIconStyle()) {
		sizeItem.cx += m_sizeSpacing.cx * 2;
		sizeItem.cy += m_sizeSpacing.cy * 2;
	}

	int minCol = rc.left / sizeItem.cx;
	int maxCol = rc.right / sizeItem.cx;
	int minRow = rc.top / sizeItem.cy;
	int maxRow = rc.bottom / sizeItem.cy;

	int cnt = GetCount();
	int selMin = cnt;
	int selMax = -1;
	int x, y;
	for (int i = 0; i < cnt; i++) {
		if (IsListStyle()) {
			x = i / m_row;
			y = i % m_row;
		}
		else {
			x = i % m_col;
			y = i / m_col;
		}
		bool bSel = GetSel(i);
		if (minCol <= x && x <= maxCol && minRow <= y && y <= maxRow) {
			if (!bSel) {
				SetSel(i, true);
				bSel = true;
			}
		}
		else if (bSel) {
			SetSel(i, false);
			bSel = false;
		}

		if (bSel) {
			selMin = min(i, selMin);
			selMax = max(i, selMax);
		}
	}

	if (0 <= selMin && selMax < cnt) {
		SetAnchorIndex(bBottomDirection ? selMax : selMin);
	}
	m_dragancindex = m_ancindex;

	// 必要であればスクロール処理
	CSize size = GetItemSize();
	size.cx = max(3, size.cx / 10);
	size.cy = max(3, size.cy / 10);

	int cx = 0;
	int cy = 0;
	if (ptScroll.x > 0 && point.x < rcc.left)
		cx = -size.cx;
	else if (m_sizeTotal.cx > ptScroll.x + rcc.Width() && point.x > rcc.right)
		cx = size.cx;
	if (ptScroll.y > 0 && point.y < rcc.top)
		cy = -size.cy;
	else if (m_sizeTotal.cy > ptScroll.y + rcc.Height() && point.y > rcc.bottom)
		cy = size.cy;

	if (cx || cy) {
		OnScrollBy(CSize(cx, cy), true);
	}

	UpdateWindow();

	// スクロール位置情報再取得
	ptScroll.x = 0;
	ptScroll.y = 0;
	GetAbsPos(&ptScroll);
	if (rc.Width() && rc.Height()) {
		rcw = rc;
		rcw.OffsetRect(-ptScroll.x, -ptScroll.y);
		DrawFocusRect(hdc, &rcw);
	}
	m_rcSel = rc;

	if (!rcc.PtInRect(point)) {
		MSG msg;
		if (!::PeekMessage(&msg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE))
			::PostMessage(m_hWnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(point.x, point.y));
	}

	ReleaseDC(hdc);
}

LRESULT CDrawListView::OnHandlerMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UINT nFlags = GET_KEYSTATE_WPARAM(wParam);
	short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	CRect rc;
	GetWindowRect(&rc);
	if (!rc.PtInRect(pt)) {
		bHandled = FALSE;
		return 1;
	}

	CSize sizeItem = GetItemSize();
	CSize size;
	if (m_numWheelScrollLines > 0) {
		if (IsListStyle()) {
			size.cx = sizeItem.cx >> 1;
			if (zDelta > 0) size.cx *= -1;
			size.cy = 0;
		}
		else if (IsReportStyle()) {
			size.cx = 0;
			size.cy = sizeItem.cy * m_numWheelScrollLines;
			if (zDelta > 0) size.cy *= -1;
		}
		else if (IsIconStyle()) {
			size.cx = 0;
			size.cy = sizeItem.cy * (m_numWheelScrollLines / m_col);
			size.cy = max(size.cy, sizeItem.cy);
			if (zDelta > 0) size.cy *= -1;
		}
		OnScrollBy(size, true);
	}
	else if (m_numWheelScrollLines < 0) {
		GetClientRect(&rc);
		if (IsListStyle()) {
			size.cx = rc.Width() - 16;
			if (zDelta > 0) size.cx *= -1;
			size.cy = 0;
		}
		else {
			size.cx = 0;
			size.cy = rc.Height() - sizeItem.cy;
			if (zDelta > 0) size.cy *= -1;
		}
		OnScrollBy(size, true);
	}
	return 0;
}

LRESULT CDrawListView::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
	if (IsExOneClickActivate() && m_bTrackMouseEvent) {
		if (GetActiveWindow() == gettopparent(m_hWnd)) {
			if (::GetFocus() != m_hWnd) {
				::SetFocus(m_hWnd);
				StandardParentNotify(NM_SETFOCUS);
			}

			if (m_hoverindex >= 0 && m_prevhoverindex != m_hoverindex) {
				// 二重処理をしないため
				m_prevhoverindex = m_hoverindex;

				CPoint pt((long)(short)LOWORD(lParam), (long)(short)HIWORD(lParam));

				BOOL vkeyShift = _ISKEYDOWN(VK_SHIFT);
				BOOL vkeyCtrl = _ISKEYDOWN(VK_CONTROL);

				UINT nFlags = MK_LBUTTON;
				if (vkeyShift) nFlags |= MK_SHIFT;
				if (vkeyCtrl)  nFlags |= MK_CONTROL;

				// オーナーウィンドウにnotify
				NMDLCLICK dlc;
				memset(&dlc, 0, sizeof(dlc));
				dlc.hdr.code = DLN_CLICK;
				dlc.hdr.hwndFrom = m_hWnd;
				dlc.hdr.idFrom = GetDlgCtrlID();
				dlc.iItem = m_hoverindex;
				dlc.typeClick = nFlags;
				dlc.ptAction = pt;
				if (m_owner.SendMessage(WM_NOTIFY, dlc.hdr.idFrom, (LPARAM)&dlc)) return 0;

				int numsel = GetSelCount();

				// ホーバー項目の現在の選択状態
				bool bSelect = GetSel(m_hoverindex);
				if (IsSingleSel())
					vkeyShift = vkeyCtrl = FALSE;

				if (numsel > 1) {
					if (bSelect && !vkeyShift && !vkeyCtrl) {
						SetAnchorIndex(m_hoverindex);
						SetCaretIndex(m_hoverindex, false);
						m_bTrackMouseEvent = FALSE;
						return 0;
					}
				}

				// インデクス項目が選択されていなければ先に選択
				if (!bSelect || vkeyShift)
					SetSelections(m_hoverindex);
				SetCaretIndex(m_hoverindex, false);

				if (!vkeyShift)
					SetAnchorIndex(m_hoverindex);

				if (!vkeyShift) {
					if (vkeyCtrl)
						SetSel(m_hoverindex, !bSelect);
					else
						SelectIndex(m_hoverindex);
				}
				else
					SetSelections(m_hoverindex);
			}
		}
	}
	m_bTrackMouseEvent = FALSE;
	return 0;
}

LRESULT CDrawListView::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	int oldhover = m_hoverindex;
	m_hoverindex = -1;
	if (IsExHover() && oldhover >= 0) {
		CRect rc;
		if (GetItemRect(oldhover, &rc) != -1)
			RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	m_prevhoverindex = -1;
	m_bTrackMouseEvent = FALSE;
	return 0;
}

void CDrawListView::OnCancelMode()
{
	if (!m_bCaptured)
		return;

	if (m_rcSel.Width() && m_rcSel.Height()) {
		CPoint pt(0, 0);
		GetAbsPos(&pt);

		CRect rc = m_rcSel;
		rc.OffsetRect(-pt.x, -pt.y);

		HDC hdc = GetDC();
		DrawFocusRect(hdc, &rc);
		ReleaseDC(hdc);
	}
	m_selindex = m_dragancindex;
	ReleaseCapture();
	m_bCaptured = false;
}

void CDrawListView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_bCaptured) return;

	NMKEY key;
	memset(&key, 0, sizeof(key));
	key.hdr.hwndFrom = m_hWnd;
	key.hdr.idFrom = GetDlgCtrlID();
	key.hdr.code = NM_KEYDOWN;
	key.nVKey = nChar;
	key.uFlags = nFlags;
	if (m_owner.SendMessage(WM_NOTIFY, key.hdr.idFrom, (LPARAM)&key))
		return;

	DefWindowProc();
}

void CDrawListView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_bCaptured) return;

	NMKEY key;
	memset(&key, 0, sizeof(key));
	key.hdr.hwndFrom = m_hWnd;
	key.hdr.idFrom = GetDlgCtrlID();
	key.hdr.code = NM_KEYDOWN;
	key.nVKey = nChar;
	key.uFlags = nFlags;
	if (m_owner.SendMessage(WM_NOTIFY, key.hdr.idFrom, (LPARAM)&key))
		return;

	if (nChar == VK_CONTROL || nChar == VK_SHIFT) return;

	int cnt = GetCount();
	if (cnt < 1) return;

	bool singleSel = IsSingleSel();
	BOOL vkeyShift = _ISKEYDOWN(VK_SHIFT);
	BOOL vkeyCtrl = _ISKEYDOWN(VK_CONTROL);
	if (singleSel)
		vkeyShift = vkeyCtrl = FALSE;

	int index = m_selindex;
	int i;

	switch (nChar) {
	case VK_UP:
		if (IsListStyle() || IsReportStyle()) {
			index--;
			if (index < 0)
				index = IsExWrapCursor() ? cnt - 1 : 0;		// ラップアラウンドチェック
		}
		else {
			if (index - m_col >= 0) index -= m_col;
		}
		break;

	case VK_DOWN:
		if (IsListStyle() || IsReportStyle()) {
			index++;
			if (index >= cnt)
				index = IsExWrapCursor() ? 0 : cnt - 1;		// ラップアラウンドチェック
		}
		else {
			int newindex = index + m_col;
			if (newindex < cnt)
				index = newindex;
			else {
				newindex = min(newindex, cnt - 1);
				if ((newindex / m_col) > (index / m_col))
					index = newindex;
			}
		}
		break;

	case VK_LEFT:
		if (IsListStyle()) {
			if (index - m_row >= 0) index -= m_row;
		}
		else if (IsReportStyle()) {
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), NULL);
			return;
		}
		else {
			index--;
			if (index < 0)
				index = IsExWrapCursor() ? cnt - 1 : 0;		// ラップアラウンドチェック
		}
		break;

	case VK_RIGHT:
		if (IsListStyle()) {
			int newindex = index + m_row;
			if (newindex < cnt)
				index = newindex;
			else {
				newindex = min(newindex, cnt - 1);
				if ((newindex / m_row) > (index / m_row))
					index = newindex;
			}
		}
		else if (IsReportStyle()) {
			SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), NULL);
			return;
		}
		else {
			index++;
			if (index >= cnt)
				index = IsExWrapCursor() ? 0 : cnt - 1;		// ラップアラウンドチェック
		}
		break;

	case VK_HOME:
		index = 0;
		break;

	case VK_END:
		index = cnt - 1;
		break;

	case VK_NEXT:
		if (IsListStyle()) {
			for (i = 0; i < m_col; i++) {
				if (index + m_row < cnt) index += m_row;
				else break;
			}
		}
		else {
			for (i = 0; i < m_row; i++) {
				if (index + m_col < cnt) index += m_col;
				else break;
			}
		}
		if (index == m_selindex)	// ENDキーと同じ動作
			index = cnt - 1;
		break;

	case VK_PRIOR:
		if (IsListStyle()) {
			for (i = 0; i < m_col; i++) {
				if (index - m_row >= 0) index -= m_row;
				else break;
			}
		}
		else {
			for (i = 0; i < m_row; i++) {
				if (index - m_col >= 0) index -= m_col;
				else break;
			}
		}
		if (index == m_selindex)	// HOMEキーと同じ動作
			index = 0;
		break;

	case VK_SPACE:
		if (!singleSel) {

			TTActivate(FALSE);		// Tip表示OFF

			m_selindex = index + 1;
			if (m_selindex >= cnt)
				m_selindex = IsExWrapCursor() ? 0 : cnt - 1;

			if (vkeyCtrl) {
				// Ctrlキー併用なら、現在位置の選択状態を反転させて次のインデクスへ
				SetSel(index, !GetSel(index));
				RedrawItem(m_selindex);
				SetAnchorIndex(m_selindex);
			}
			else {
				// Ctrlキー併用でないなら、現在位置の選択状態はそのまま次のインデクスを反転させて選択
				RedrawItem(index);
				SetSel(m_selindex, !GetSel(m_selindex));
			}
			EnsureVisible(m_selindex, FALSE);
		}
		return;

	default:
		return;
	}

	TTActivate(FALSE);		// Tip表示OFF

	if (index != m_selindex) {
		if (!vkeyCtrl && !vkeyShift)
			SelectIndex(index, false);
		else {
			int oldindex = m_selindex;
			m_selindex = index;
			if (vkeyCtrl) {
				// 選択フォーカス位置が移動するのみ
				if (oldindex >= 0)
					RedrawItem(oldindex);
				RedrawItem(m_selindex);
			}
			else if (vkeyShift) {
				int vmin = min(index, m_ancindex);
				int vmax = max(index, m_ancindex);
				for (i = 0; i < cnt; i++) {
					bool bSel = GetSel(i);
					if (vmin <= i && i <= vmax) {
						if (!bSel)
							SetSel(i, true);
					}
					else {
						if (bSel)
							SetSel(i, false);
					}
				}
				CRect rc;
				if (GetItemRect(oldindex, &rc) != -1)
					InvalidateRect(&rc, false);
				if (GetItemRect(m_selindex, &rc) != -1)
					InvalidateRect(&rc, false);
			}
			EnsureVisible(m_selindex, false);
		}
	}
	else if (!vkeyCtrl && !vkeyShift && !GetSel(index)) {
		SetSel(index, true, true);
	}
}

void CDrawListView::StandardParentNotify(UINT code)
{
	NMHDR hdr;
	hdr.code = code;
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom = GetDlgCtrlID();
	m_owner.SendMessage(WM_NOTIFY, hdr.idFrom, (LPARAM)&hdr);
}

void CDrawListView::OnSetFocus(HWND hwndOld)
{
	DefWindowProc();

	RedrawSelItems();
	if (!GetSel(m_selindex))
		RedrawItem(m_selindex);

	StandardParentNotify(NM_SETFOCUS);
}

void CDrawListView::OnKillFocus(HWND hwndNew)
{
	DefWindowProc();

	if (m_bTrackMouseEvent) {
		m_bTrackMouseEvent = FALSE;

		TRACKMOUSEEVENT te;
		te.cbSize = sizeof(te);
		te.dwFlags = TME_HOVER | TME_LEAVE | TME_CANCEL;
		te.hwndTrack = m_hWnd;
		te.dwHoverTime = HOVER_DEFAULT;
		_TrackMouseEvent(&te);
	}

	RedrawSelItems();
	if (!GetSel(m_selindex))
		RedrawItem(m_selindex);

	StandardParentNotify(NM_KILLFOCUS);
}

void CDrawListView::OnDestroy()
{
	if (GetCount() > 0) OnResetContent(0, 0);
}

////////////////////////////////////////////////////////////////////////
// scroll operations

int CDrawListView::GetScrollLimit(int nBar)
{
	int nMin, nMax;
	GetScrollRange(nBar, &nMin, &nMax);
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_PAGE;
	if (GetScrollInfo(nBar, &info)) {
		nMax -= __max(info.nPage - 1, 0);
	}
	return nMax;
}

void CDrawListView::CalcBars(void)
{
	CRect rc;
	GetClientRect(&rc);
	int cnt = GetCount();

	CSize size = GetItemSize();
	int cxItem = size.cx;
	int cyItem = size.cy;
	if (IsIconStyle()) {
		cxItem += m_sizeSpacing.cx * 2;
		cyItem += m_sizeSpacing.cy * 2;
	}

	if (IsListStyle()) {
		m_row = rc.Height() / cyItem;
		m_col = rc.Width() / cxItem;
	}
	else {
		m_row = rc.Height() / cyItem;
		m_col = IsReportStyle() ? 1 : rc.Width() / cxItem;
	}

	if (m_col < 1) m_col = 1;
	if (m_row < 1) m_row = 1;

	m_sizePage.cx = m_col * cxItem;
	m_sizePage.cy = m_row * cyItem;

	if (IsListStyle()) {
		m_sizeTotal.cx = (cnt / m_row) * cxItem;
		m_sizeTotal.cy = m_row * cyItem;
		if (cnt % m_row)
			m_sizeTotal.cx += cxItem;
	}
	else {
		m_sizeTotal.cx = m_col * cxItem;
		m_sizeTotal.cy = (cnt / m_col) * cyItem;
		if (cnt % m_col)
			m_sizeTotal.cy += cyItem;
	}

	// Now update the scrollbars
	if (m_hWnd != NULL) {
		UpdateBars();
		Invalidate(TRUE); // Zoom scale changed, redraw all
	}
}

// スクロール・バーの状態を変更せずに、スクロール・バーの状態を返す
void CDrawListView::GetScrollBarState(CSize sizeClient, CSize& needSb, CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient)
{
	// スクロールバーサイズの取得
	CSize sizeSb;
	GetScrollBarSizes(sizeSb);

	// enough room to add scrollbars
	sizeRange = m_sizeTotal - sizeClient;
		// > 0 => need to scroll
	ptMove.x = 0;
	ptMove.y = 0;
	GetAbsPos(&ptMove);
		// point to move to (start at current scroll pos)

	BOOL bNeedH = sizeRange.cx > 0;
	if (!bNeedH)							// 水平方向のスクロールバーが必要ではない
		ptMove.x = 0;                       // jump back to origin
	else if (bInsideClient)
		sizeRange.cy += sizeSb.cy;          // need room for a scroll bar

	BOOL bNeedV = sizeRange.cy > 0;
	if (!bNeedV)							// 垂直方向のスクロールバーが必要ではない
		ptMove.y = 0;                       // jump back to origin
	else if (bInsideClient)
		sizeRange.cx += sizeSb.cx;          // need room for a scroll bar

	if (bNeedV && !bNeedH && sizeRange.cx > 0)
	{
		// need a horizontal scrollbar after all
		bNeedH = TRUE;
		sizeRange.cy += sizeSb.cy;
	}

	// 現在のスクロール位置が限界(スクロールする制限)を過ぎている場合
	if (sizeRange.cx > 0 && ptMove.x >= sizeRange.cx)
		ptMove.x = sizeRange.cx;
	if (sizeRange.cy > 0 && ptMove.y >= sizeRange.cy)
		ptMove.y = sizeRange.cy;

	// now update the bars as appropriate
	needSb.cx = bNeedH && !IsIconStyle();
	needSb.cy = bNeedV && !IsListStyle();

	// needSb, sizeRange, and ptMove area now all updated
}

void CDrawListView::GetScrollBarSizes(CSize& sizeSb)
{
	sizeSb.cx = sizeSb.cy = 0;
	DWORD dwStyle = GetStyle();

	// vert scrollbars will impact client area of this window
	sizeSb.cx = GetSystemMetrics(SM_CXVSCROLL);
	if (dwStyle & WS_BORDER)
		sizeSb.cx -= GetSystemMetrics(SM_CXBORDER);

	// horz scrollbars will impact client area of this window
	sizeSb.cy = GetSystemMetrics(SM_CYHSCROLL);
	if (dwStyle & WS_BORDER)
		sizeSb.cy -= GetSystemMetrics(SM_CYBORDER);
}

bool CDrawListView::GetTrueClientSize(CSize& size, CSize& sizeSb)
	// return TRUE if enough room to add scrollbars if needed
{
	CRect rect;
	GetClientRect(&rect);
	size.cx = rect.right;
	size.cy = rect.bottom;
	DWORD dwStyle = GetStyle();

	// first get the size of the scrollbars for this window
	GetScrollBarSizes(sizeSb);

	// first calculate the size of a potential scrollbar
	// (scroll bar controls do not get turned on/off)
	if (sizeSb.cx != 0 && (dwStyle & WS_VSCROLL)) {
		// vert scrollbars will impact client area of this window
		size.cx += sizeSb.cx;   // currently on - adjust now
	}
	if (sizeSb.cy != 0 && (dwStyle & WS_HSCROLL)) {
		// horz scrollbars will impact client area of this window
		size.cy += sizeSb.cy;   // currently on - adjust now
	}

	// return TRUE if enough room
	return (size.cx > sizeSb.cx && size.cy > sizeSb.cy);
}

void CDrawListView::UpdateBars()
{
	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_bInsideUpdate)
		return;         // Do not allow recursive calls

	// Lock out recursion
	m_bInsideUpdate = TRUE;

	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks

	CRect rectClient;

	CSize sizeClient;
	CSize sizeSb;

	// get client rect
	if (!GetTrueClientSize(sizeClient, sizeSb)) {
		// no room for scroll bars (common for zero sized elements)
		CRect rect;
		GetClientRect(&rect);
		if (rect.right > 0 && rect.bottom > 0) {
			// if entire client area is not invisible, assume we have
			//  control over our scrollbars
			ShowScrollBar(SB_HORZ, FALSE);
			ShowScrollBar(SB_VERT, FALSE);
		}
		m_bInsideUpdate = FALSE;
		return;
	}

	// enough room to add scrollbars
	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;

	// get the current scroll bar state given the true client area
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, TRUE);
	if (needSb.cx) sizeClient.cy -= sizeSb.cy;
	if (needSb.cy) sizeClient.cx -= sizeSb.cx;

	// first scroll the window as needed
	ScrollToDevicePosition(ptMove); // will set the scroll bar positions too

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_PAGE|SIF_RANGE;
	info.nMin = 0;

	// now update the bars as appropriate
	ShowScrollBar(SB_HORZ, (BOOL)needSb.cx);
	if (needSb.cx) {
		info.nPage = sizeClient.cx;
		info.nMax = m_sizeTotal.cx-1;
		SetScrollInfo(SB_HORZ, &info, TRUE);
	}
	ShowScrollBar(SB_VERT, (BOOL)needSb.cy);
	if (needSb.cy) {
		info.nPage = sizeClient.cy;
		info.nMax = m_sizeTotal.cy-1;
		SetScrollInfo(SB_VERT, &info, TRUE);
	}

	// remove recursion lockout
	m_bInsideUpdate = FALSE;
}

void CDrawListView::ScrollToPosition(POINT pt)    // logical coordinates
{
	// now in device coordinates - limit if out of range
	int xMax = GetScrollLimit(SB_HORZ);
	int yMax = GetScrollLimit(SB_VERT);
	xMax = min(pt.x , max(0, xMax));
	yMax = min(pt.y , max(0, yMax));
	ScrollToDevicePosition(pt);
}

void CDrawListView::ScrollToDevicePosition(POINT ptDev)
{
	// Note: ScrollToDevicePosition can and is used to scroll out-of-range
	//  areas as far as CZoomView is concerned -- specifically in
	//  the print-preview code.  Since OnScrollBy makes sure the range is
	//  valid, ScrollToDevicePosition does not vector through OnScrollBy.

	int xOrig = GetScrollPos(SB_HORZ);
	int yOrig = GetScrollPos(SB_VERT);
	SetScrollPos(SB_HORZ, ptDev.x, TRUE);
	SetScrollPos(SB_VERT, ptDev.y, TRUE);

	CRect rc;
	GetClientRect(&rc);
	ScrollWindow(xOrig - ptDev.x, yOrig - ptDev.y, &rc, &rc);
}

void CDrawListView::OnHScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar)
{
	OnScroll(MAKEWORD(nSBCode, -1), nPos);
}

void CDrawListView::OnVScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar)
{
	OnScroll(MAKEWORD(-1, nSBCode), nPos);
}

bool CDrawListView::OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll)
{
	int x = GetScrollPos(SB_HORZ);
	int y = GetScrollPos(SB_VERT);
	int xOrig = x;
	int yOrig = y;
	SCROLLINFO si;

	// calc new x position
	switch (LOBYTE(nScrollCode)) {
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		if (IsReportStyle())
			x -= REPORT_HSCROLL_DELTA;
		else
			x -= m_sizeItem.cx + (IsIconStyle() ? m_sizeSpacing.cx * 2 : 0);
		break;
	case SB_LINEDOWN:
		if (IsReportStyle())
			x += REPORT_HSCROLL_DELTA;
		else
			x += m_sizeItem.cx + (IsIconStyle() ? m_sizeSpacing.cx * 2 : 0);
		break;
	case SB_PAGEUP:
		x -= m_sizePage.cx;
		break;
	case SB_PAGEDOWN:
		x += m_sizePage.cx;
		break;
	case SB_THUMBTRACK:
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		if (GetScrollInfo(SB_HORZ, &si)) {
//--- 項目の幅単位でスクロールするにはココのコメントをはずす
//			if (IsListStyle() && GetScrollLimit(SB_HORZ) > si.nTrackPos)
//				x = ((si.nTrackPos + m_sizeItem.cx / 2) / m_sizeItem.cx) * m_sizeItem.cx;
//			else
				x = si.nTrackPos;
		}
		break;
	}

	// calc new y position
	switch (HIBYTE(nScrollCode)) {
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= m_sizeItem.cy + (IsIconStyle() ? m_sizeSpacing.cy * 2 : 0);
		break;
	case SB_LINEDOWN:
		y += m_sizeItem.cy + (IsIconStyle() ? m_sizeSpacing.cy * 2 : 0);
		break;
	case SB_PAGEUP:
		y -= m_sizePage.cy;
		break;
	case SB_PAGEDOWN:
		y += m_sizePage.cy;
		break;
	case SB_THUMBTRACK:
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		if (GetScrollInfo(SB_VERT, &si))
			y = si.nTrackPos;
		break;
	}

	bool bResult = OnScrollBy(CSize(x - xOrig, y - yOrig), bDoScroll);
	if (bResult && bDoScroll)
		UpdateWindow();

	return bResult;
}

bool CDrawListView::OnScrollBy(CSize sizeScroll, bool bDoScroll)
{
	int xOrig, x;
	int yOrig, y;

	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	DWORD dwStyle = GetStyle();
	if (!(dwStyle & WS_VSCROLL)) {
		// vertical scroll bar not enabled
		sizeScroll.cy = 0;
	}
	if (!(dwStyle & WS_HSCROLL)) {
		// horizontal scroll bar not enabled
		sizeScroll.cx = 0;
	}

	xOrig = x = GetScrollPos(SB_HORZ);
	yOrig = y = GetScrollPos(SB_VERT);
	int xMax = GetScrollLimit(SB_HORZ);
	int yMax = GetScrollLimit(SB_VERT);

	// adjust current x position
	x += sizeScroll.cx;
	x = max(0, min(x, xMax));

	// adjust current y position
	y += sizeScroll.cy;
	y = max(0, min(y, yMax));

	// did anything change?
	if (x == xOrig && y == yOrig)
		return false;

	if (bDoScroll) {
		// do scroll and update scroll positions
		CRect rc;
		GetClientRect(&rc);
		ScrollWindow(-(x-xOrig), -(y-yOrig), &rc, &rc);

		if (x != xOrig) {
			SetScrollPos(SB_HORZ, x, TRUE);
			if (IsReportStyle() && m_header.IsWindow()) {
				m_header.SetWindowPos(NULL, -x, 0, 0, 0, SWP_NOSIZE);
				m_header.RedrawWindow(NULL, NULL, RDW_UPDATENOW);
			}
		}
		if (y != yOrig)
			SetScrollPos(SB_VERT, y, TRUE);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////
// DrawList Original Message Handler
//

LRESULT CDrawListView::OnDeleteItem(WPARAM wParam, LPARAM lParam)
{
	CDLItem* item = GetItem((int)wParam);
	if (!item) return -1;

	item->Clear();

	// 親ウィンドウへ通知
	NMDLVIEW dlv;
	memset(&dlv, 0, sizeof(dlv));
	dlv.hdr.code = DLN_DELETEITEM;
	dlv.hdr.hwndFrom = m_hWnd;
	dlv.hdr.idFrom = GetDlgCtrlID();
	dlv.iItem = wParam;
	dlv.uNewState = item->m_state;
	dlv.uOldState = item->m_state;
	dlv.lParam = (LPARAM)item->m_data;
	m_array.RemoveAt(wParam);				// 先に削除してから

	int cnt = (int)m_array.GetCount();
	m_ancindex = min(cnt - 1, m_ancindex);
	m_selindex = min(cnt - 1, m_selindex);

	if (!(GetStyle() & DLS_NODATA))
		m_owner.SendMessage(WM_NOTIFY, dlv.hdr.idFrom, (LPARAM)&dlv);

	CalcBars();
	return 0;
}

LRESULT CDrawListView::OnResetContent(WPARAM, LPARAM)
{
	int cnt = (int)m_array.GetCount();
	if (cnt < 1) return 0;

	if (IsExToolTip()) {
		TTEnable(false);
		TTActivate(FALSE);
	}

	// データを作業アレイにコピーしてから処理
	CAtlArray<CDLItem> backup;
	backup.Copy(m_array);
	m_array.RemoveAll();

	m_selindex = 0;
	m_ancindex = 0;
	m_dropindex = -1;
	m_hit = -1;
	m_horzExtent = 0;

	NMDLVIEW dlv;
	memset(&dlv, 0, sizeof(dlv));
	dlv.hdr.code = DLN_DELETEITEM;
	dlv.hdr.hwndFrom = m_hWnd;
	dlv.hdr.idFrom = GetDlgCtrlID();

	cnt = (int)backup.GetCount();
	for (int i = 0; i < cnt; i++) {
		CDLItem& di = backup[i];
		if (!(GetStyle() & DLS_NODATA)) {
			dlv.iItem = i;
			dlv.uNewState = di.m_state;
			dlv.uOldState = di.m_state;
			dlv.lParam = (LPARAM)di.m_data;
			// 削除メッセージを親ウィンドウへ通知
			m_owner.SendMessage(WM_NOTIFY, dlv.hdr.idFrom, (LPARAM)&dlv);
		}
		di.Clear();
	}
	// 作業アレイを削除
	backup.RemoveAll();

m_bInsideUpdate = TRUE;
	ShowScrollBar(SB_BOTH, FALSE);
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.nPage = 100;
	info.fMask = SIF_PAGE | SIF_RANGE;
	info.nMin = 0;
	info.nMax = 1;
	SetScrollInfo(SB_HORZ, &info, TRUE);
	info.nPage = 100;
	SetScrollInfo(SB_VERT, &info, TRUE);
m_bInsideUpdate = FALSE;

	CalcBars();

	if (IsExToolTip())
		TTEnable(true);
	return 0;
}

LRESULT CDrawListView::OnSetItemData(WPARAM wParam, LPARAM lParam)
{
	CDLItem* item = GetItem((int)wParam);
	if (item) return -1;

	item->m_data = (void*)lParam;
	CRect rc;
	if (GetItemRect((int)wParam, &rc) != -1)
		InvalidateRect(&rc, false);
	return 0;
}

LRESULT CDrawListView::OnSetSpacing(WPARAM wParam, LPARAM lParam)
{
	CSize size((DWORD)lParam);
	bool bRedraw = (wParam != 0);
	if (size != m_sizeSpacing && size.cx >= 0 && size.cy >= 0) {
		m_sizeSpacing = size;
		if (IsIconStyle()) {
			CalcBars();
			if (bRedraw) {
				EnsureVisible(m_selindex, false);
				Invalidate(true);
				UpdateWindow();
			}
		}
	}
	return 0;
}

LRESULT CDrawListView::OnSetOwner(WPARAM wParam, LPARAM lParam)
{
	HWND hwndT = NULL;
	if (::IsWindow((HWND)lParam)) {
		hwndT = m_owner;
		m_owner = (HWND)lParam;
	}
	return (LRESULT)hwndT;
}

LRESULT CDrawListView::OnSetBkColor(WPARAM wParam, LPARAM lParam)
{
	COLORREF clrOld = m_clrBkgnd;
	if (m_clrBkgnd != (COLORREF)lParam) {
		m_clrBkgnd = (COLORREF)lParam;
		Invalidate(TRUE);
	}
	return (LRESULT)clrOld;
}

LRESULT CDrawListView::OnSetCount(WPARAM wParam, LPARAM)
{
	OnResetContent(0, 0);
	int cnt = (int)wParam;
	if (cnt > 0 && m_array.SetCount(cnt)) {
//@		void* p = m_array.GetData();
//@		memset(p, 0, sizeof(CDLItem) * cnt);
		CalcBars();
	}
	return cnt;
}

int CDrawListView::AddItem(void* data)
{
	CDLItem di;
	di.Init(0, data);
	int index = (int)m_array.Add(di);
	if (index >= 0)
		CalcBars();
	return index;
}

int CDrawListView::InsertItem(int index, void* data)
{
	int cnt = GetCount();
	if (index < 0 || index >= cnt)
		return AddItem(data);

	CDLItem di;
	di.Init(0, data);
	m_array.InsertAt(index, di);
	CalcBars();
	return index;
}

BOOL CDrawListView::GetAnchorPoint(POINT* ppt)
{
	if (m_ancindex != -1) {
		CRect rc;
		if (GetItemRect(m_ancindex, &rc) != -1) {
			ppt->x = rc.left + rc.Width() / 2;
			ppt->y = rc.top + rc.Height() / 2;
			return TRUE;
		}
	}
	return FALSE;
}

int CDrawListView::SelectIndex(int index, bool fPartialOK)
{
	int cnt = GetCount();
	if (cnt > 0) {
		SetSel(-1, false);
		if (index >= 0 && index < cnt) {
			SetSel(index, true);
			SetCaretIndex(index, fPartialOK);
			SetAnchorIndex(index);
		}
		return 0;
	}
	return -1;
}

void CDrawListView::SetSelections(int index)
{
	// if the shift key is down, keep the selection range
	BOOL vkeyShift = _ISKEYDOWN(VK_SHIFT);
	BOOL vkeyCtrl = _ISKEYDOWN(VK_CONTROL);
	if (IsSingleSel())
		vkeyShift = vkeyCtrl = FALSE;

	if (vkeyShift) {
		// 選択されている状態をすべてクリア
		if (!vkeyCtrl)
			SetSel(-1, false);

		if (m_ancindex == index)
			SetSel(index, true);
		else {
			int sSel = min(index, m_ancindex);
			int eSel = max(index, m_ancindex);
			SelItemRange(true, sSel, eSel);
		}
		SetCaretIndex(index, false);
	}
	else if (vkeyCtrl) {
		// コントロールキーが押下されていたら、選択状態を反転
		SetSel(index, !GetSel(index));
		SetCaretIndex(index, false);
	}
	else {
		if (!GetSel(index)) {
			SetSel(-1, false);
			SetSel(index, true);
		}
		else {
			SetCaretIndex(index, false);
			SetAnchorIndex(index);
		}
	}
	UpdateWindow();
}

void CDrawListView::RedrawItemRange(int iStart, int iEnd)
{
	int cnt = GetCount();
	if (cnt < 1) return;

	iStart = min(cnt - 1, max(iStart, 0));
	iEnd = min(cnt - 1, max(iEnd, 0));

	CSize sizeItem = GetItemSize();

	CClientDC dc(m_hWnd);
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hbm = CreateCompatibleBitmap(dc, sizeItem.cx, sizeItem.cy);
	ATLASSERT(hbm != NULL);
	hbm = dcw.SelectBitmap(hbm);

	HFONT hf = dcw.SelectFont(m_hfont);

	DRAWITEMSTRUCT ds;
	memset(&ds, 0, sizeof(ds));
	ds.CtlType = ODT_LISTBOX;
	ds.CtlID = GetDlgCtrlID();
	ds.itemAction = ODA_DRAWENTIRE;
	ds.hwndItem = m_hWnd;
	ds.hDC = dcw;
	SetRect(&ds.rcItem, 0, 0, sizeItem.cx, sizeItem.cy);

	COLORREF clr = (m_clrBkgnd == INVALID_COLOR) ? GetSysColor(COLOR_WINDOW) : m_clrBkgnd;
	for (int i = iStart; i <= iEnd; i++) {
		CDLItem* item = GetItem(i);
		if (!item) continue;

		CRect rc;
		if ((GetItemRect(i, &rc) < 0) || !dc.RectVisible(&rc))
			continue;

		ds.itemID = (UINT)i;
		ds.itemData = (UINT_PTR)item->m_data;
		ds.itemState = item->m_state;
		if (i == m_selindex)					ds.itemState |= ODS_FOCUS;
		if (i == m_dropindex)					ds.itemState |= ODS_SELECTED;
		if (IsExHover() && i == m_hoverindex)	ds.itemState |= ODS_HOTLIGHT;

		int odd = ((i / m_col) & 1) + ((i % m_col) & 1);
		if (odd == 1)							ds.itemState |= ODS_ODDPOSITION;

		dcw.FillSolidRect(&ds.rcItem, clr);
		m_owner.SendMessage(WM_DRAWITEM, (WPARAM)ds.CtlID, (LPARAM)&ds);
		dc.BitBlt(rc.left, rc.top, sizeItem.cx, sizeItem.cy, dcw, 0, 0, SRCCOPY);
	}
	dcw.SelectFont(hf);
	DeleteObject(dcw.SelectBitmap(hbm));
}

void CDrawListView::RedrawMaskedItems(UINT mask)
{
	int cnt = GetMaskedCount(mask);
	if (cnt < 1)
		return;

	int* array = new int[cnt];
	cnt = GetMaskedItems(mask, cnt, array);
	for (int i = 0; i < cnt; i++) {
		CRect rc;
		if (GetItemRect(array[i], &rc) != -1)
			InvalidateRect(&rc, false);
	}
	delete [] array;
}

void CDrawListView::SetAnchorIndex(int index)
{
	int cnt = GetCount();
	m_ancindex = min(cnt - 1, max(0, index));
}

void CDrawListView::SetDropTargetIndex(int index)
{
	if (IsBadIndex(index)) index = -1;
	if (m_dropindex == index) return;

	int indexOld = m_dropindex;
	m_dropindex = index;
	if (indexOld >= 0 && !GetSel(indexOld))
		RedrawItem(indexOld);

	if (m_dropindex >= 0 && !GetSel(m_dropindex))
		RedrawItem(m_dropindex);
}

void CDrawListView::GetAbsPos(POINT* ppt)
{
	ppt->x += GetScrollPos(SB_HORZ);
	ppt->y += GetScrollPos(SB_VERT);
}

int CDrawListView::ItemFromPoint(POINT pt)
{
	int cnt = GetCount();
	if (cnt < 1)
		return -1;

	CPoint pts = pt;
	ClientToScreen(&pts);
	if (SendMessage(WM_NCHITTEST, 0, MAKELPARAM(pts.x, pts.y)) != HTCLIENT)
		return -1;

	GetAbsPos(&pt);

	if (IsListStyle()) {
		if (pt.y > m_sizePage.cy)
			return -1;
	}
	else {
		if (pt.x > m_sizePage.cx)
			return -1;
	}

	CSize sizeItem = GetItemSize();
	if (IsIconStyle()) {
		sizeItem.cx += m_sizeSpacing.cx * 2;
		sizeItem.cy += m_sizeSpacing.cy * 2;
	}

	int index;
	if (IsListStyle())
		index = (pt.x / sizeItem.cx) * m_row + (pt.y / sizeItem.cy);
	else
		index = (pt.y / sizeItem.cy) * m_col + (pt.x / sizeItem.cx);
	if (index < cnt) {
		CRect rc;
		if (GetItemRectAbs(index, &rc) != -1)
			return (rc.PtInRect(pt) ? index : -1);
	}
	return -1;
}

int CDrawListView::GetTopIndex()
{
	return ItemFromPoint(CPoint(0, 0));
}

CDLItem* CDrawListView::GetItem(int index)
{
	return (IsBadIndex(index) ? NULL : &m_array[index]);
}

void* CDrawListView::GetItemData(int index)
{
	CDLItem* item = GetItem(index);
	return ((item == NULL) ? NULL : item->m_data);
}

CSize CDrawListView::GetItemSize()
{
	CSize size;
	size.cx = m_sizeItem.cx;
	size.cy = m_sizeItem.cy;
	if (IsReportStyle())
		size.cx = m_horzExtent;
	return size;
}

int CDrawListView::GetItemRectAbs(int index, RECT* prc)
{
	int cnt = GetCount();
	if (prc && cnt > 0 && index >= 0 && index < cnt) {
		CSize size = GetItemSize();
		int cxItem = size.cx;
		int cyItem = size.cy;
		if (IsIconStyle()) {
			cxItem += m_sizeSpacing.cx * 2;
			cyItem += m_sizeSpacing.cy * 2;
		}

		if (IsListStyle()) {
			prc->left = (long)(index / m_row) * cxItem;
			prc->top = (long)(index % m_row) * cyItem;
			prc->right = prc->left + cxItem;
			prc->bottom = prc->top + cyItem;
		}
		else if (IsReportStyle()) {
			prc->left = 0;
			prc->top = (long)index * cyItem;
			prc->bottom = prc->top + cyItem;
			prc->right = m_horzExtent;
		}
		else {
			prc->left = (long)(index % m_col) * cxItem;
			prc->top = (long)(index / m_col) * cyItem;
			prc->right = prc->left + cxItem;
			prc->bottom = prc->top + cyItem;
		}
		if (IsIconStyle())
			InflateRect(prc, -m_sizeSpacing.cx, -m_sizeSpacing.cy);
		return 0;
	}
	return -1;
}

int CDrawListView::GetItemRect(int index, RECT* prc)
{
	if (GetItemRectAbs(index, prc) != -1) {
		OffsetRect(prc, -GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT));
		return 0;
	}
	return -1;
}

bool CDrawListView::GetMasked(UINT mask, int index)
{
	return (IsBadIndex(index) ? false : (m_array[index].m_state & mask) != 0);
}

void CDrawListView::SetMasked(UINT mask, int index, bool on, bool bNotify)
{
	if (index < 0) {
		MaskItemRange(mask, on, 0, GetCount() - 1);
		return;
	}

	CDLItem* item = GetItem(index);
	if (!item) return;

	bool bUpdate = false;
	UINT state = item->m_state;
	if (on) {
		if (!(item->m_state & mask)) {
			item->m_state |= mask;
			bUpdate = true;
		}
	}
	else {
		if (item->m_state & mask) {
			item->m_state &= ~mask;
			bUpdate = true;
		}
	}

	if (bUpdate && bNotify) {
		CRect rc;
		if (GetItemRect(index, &rc) != -1)
			InvalidateRect(&rc, false);

		// 親ウィンドウへ変更通知
		NMDLVIEW dlv;
		memset(&dlv, 0, sizeof(dlv));
		dlv.hdr.code = DLN_ITEMCHANGED;
		dlv.hdr.hwndFrom = m_hWnd;
		dlv.hdr.idFrom = GetDlgCtrlID();
		dlv.iItem = index;
		dlv.uNewState = item->m_state;
		dlv.uOldState = state;
		dlv.lParam = (LPARAM)item->m_data;
		m_owner.SendMessage(WM_NOTIFY, dlv.hdr.idFrom, (LPARAM)&dlv);
	}
}

int CDrawListView::SetCaretIndex(int index, bool fPartialOK)
{
	if (IsBadIndex(index)) return -1;

	if (m_selindex != index) {
		int oldindex = m_selindex;
		m_selindex = index;
		RedrawItem(oldindex);
		RedrawItem(m_selindex);
	}
	EnsureVisible(index, fPartialOK);
	return 0;
}

int CDrawListView::MaskItemRange(UINT mask, bool on, int iStart, int iEnd)
{
	int cnt = GetCount();
	if (cnt < 1)
		return -1;

	iStart = min(cnt - 1, max(0, iStart));
	iEnd = min(cnt - 1, max(0, iEnd));

	CRect rc, rcu;
	rcu.SetRectEmpty();
	for (int i = iStart; i <= iEnd; i++) {
		bool onCur = GetMasked(mask, i);
		if (onCur != on)
			SetMasked(mask, i, on);
	}
	return 0;
}

int CDrawListView::GetMaskedCount(UINT mask)
{
	int cnt = GetCount();
	int num = 0;
	for (int i = 0; i < cnt; i++) {
		if (m_array[i].m_state & mask)
			num++;
	}
	return num;
}

int CDrawListView::GetMaskedItems(UINT mask, int arraysize, int* array)
{
	if (arraysize < 1 || !array)
		return -1;

	int cnt = GetCount();
	int num = 0;
	for (int i = 0; i < cnt; i++) {
		if (num >= arraysize)
			break;
		if (m_array[i].m_state & mask)
			array[num++] = i;
	}
	return num;
}

int CDrawListView::SetTopIndex(int index)
{
	int cnt = GetCount();
	if (cnt < 1)
		return -1;

	index = min(cnt - 1, max(0, index));
	CRect rc;
	GetItemRectAbs(index, &rc);
	CPoint pt(rc.left, rc.top);
	ScrollToPosition(pt);
	return index;
}

int CDrawListView::GetNextItem(int iStart, int flags)
{
	int cnt = GetCount();
	if (cnt <= iStart) return -1;
	if (iStart < 0) iStart = 0;

	int index = -1;
	UINT mask;
	switch (flags) {
	case DLNI_ABOVE:	index = IsListStyle() ? iStart - 1 : iStart - m_col;	break;
	case DLNI_BELOW:	index = IsListStyle() ? iStart + 1 : iStart + m_col;	break;
	case DLNI_TOLEFT:	index = IsListStyle() ? iStart - m_row : iStart - 1;	break;
	case DLNI_TORIGHT:	index = IsListStyle() ? iStart + m_row : iStart + 1;	break;

	case DLNI_SELECTED:
	case DLNI_MARKED:
		mask = (flags == DLNI_MARKED) ? ODS_MARKED : ODS_SELECTED;
		for (int i = iStart; i < cnt; i++) {
			if (m_array[i].m_state & mask) {
				index = i;
				break;
			}
		}
		break;

	case DLNI_FOCUSED:
		if (m_selindex >= iStart)
			index = m_selindex;
		break;
	}
	return (0 <= index && index < cnt) ? index : -1;
}

bool CDrawListView::SetItemSize(CSize sizeItem, bool bRedraw)
{
	if (sizeItem != m_sizeItem && sizeItem.cx > 0 && sizeItem.cy > 0) {
		m_sizeItem = sizeItem;
		if (IsListStyle() && m_sizeItem.cx & 1) {
			// 一覧スタイルの場合はアイテム幅を偶数に丸める
			// wheelスクロールでアイテム幅の1/2単位のスクロールをするため
			m_sizeItem.cx++;
		}
		CalcBars();
		if (bRedraw) {
			EnsureVisible(m_selindex, false);
			Invalidate(true);
			UpdateWindow();
		}
		return true;
	}
	return false;
}

bool CDrawListView::IsItemVisible(int index, bool fPartialOK)
{
	CRect rcs;
	if (GetItemRect(index, &rcs) != -1) {
		if (IsIconStyle())
			rcs.InflateRect(m_sizeSpacing.cx, m_sizeSpacing.cx);
		CRect rc, rca;
		GetClientRect(&rc);
		BOOL bVisible = rca.IntersectRect(&rc, &rcs);
		if (bVisible && fPartialOK)			// 部分的に見えている
			return true;

		if (!fPartialOK) {
			if (rca == rcs)		// 完全に見えている
				return true;
			if (bVisible && (rcs.Width() >= rc.Width() || rcs.Height() >= rc.Height()))
				return true;
		}
	}
	return false;
}

bool CDrawListView::EnsureVisible(int index, bool fPartialOK)
{
	CRect rcs;
	if (GetItemRect(index, &rcs) != -1) {
		if (IsIconStyle())
			rcs.InflateRect(m_sizeSpacing.cx, m_sizeSpacing.cx);
		CRect rc, rca;
		GetClientRect(&rc);
		BOOL bVisible = rca.IntersectRect(&rc, &rcs);
		if (bVisible && fPartialOK)			// 部分的に見えている
			return true;

		if (!fPartialOK) {
			if (rca == rcs)		// 完全に見えている
				return true;
			if (bVisible && (rcs.Width() >= rc.Width() || rcs.Height() >= rc.Height()))
				return true;
		}

		int cxItem = m_sizeItem.cx;
		int cyItem = m_sizeItem.cy;
		if (IsIconStyle()) {
			cxItem += m_sizeSpacing.cx * 2;
			cyItem += m_sizeSpacing.cy * 2;
		}
		int xScroll = 0;
		int yScroll = 0;

		if (IsListStyle()) {
			int x = GetScrollPos(SB_HORZ);
			if (bVisible) {
				if (rca.left == rc.left)
					xScroll = -(cxItem - rca.Width());
				else
					xScroll = cxItem - rca.Width();
			}
			else {
				if (rc.right <= rcs.left) {
					xScroll = rcs.right - rc.right;
					if (rcs.left - xScroll < 0)
						xScroll = rcs.left;
				}
				else if (rc.left >= rcs.right)
					xScroll = -(rc.left - rcs.left);
			}
		}
		else {
			if (bVisible) {
				if (rca.top == rc.top)
					yScroll = -(cyItem - rca.Height());
				else
					yScroll = cyItem - rca.Height();
			}
			else {
				if (rc.bottom <= rcs.top)
					yScroll = rcs.bottom - rc.bottom;
				else if (rc.top >= rcs.bottom)
					yScroll = -(rc.top - rcs.top);
			}
		}

		return OnScrollBy(CSize(xScroll, yScroll), true);
	}
	return false;
}

bool CDrawListView::StartDragDrop()
{
	// ドラッグ＆ドロップ処理
	CPoint pt;
	GetCursorPos(&pt);
	bool bOK = FALSE;
	if (lccBeginTrack(m_hWnd, pt)) {
		bOK = true;
	}
	return bOK;
}

void CDrawListView::SetExtendedStyle(DWORD newStyle, DWORD mask)
{
	DWORD style = (m_exStyle & ~mask) | (newStyle & mask);
	DWORD chgStyle = m_exStyle ^ style;
	if (chgStyle) {
		m_exStyle = style;
		if (chgStyle & DLS_EX_TOOLTIP)
			TTEnable(IsExToolTip());
#ifdef __USE_HEADERINALLVIEWS
		if (chgStyle & DLS_EX_HEADERINALLVIEWS)
			UpdateLayout();
#endif
	}
}

void CDrawListView::UpdateLayout()
{
	bool bHeaderChanged = false;
	if (m_header.IsWindow()) {
		if (IsReportStyle()
#ifdef __USE_HEADERINALLVIEWS
			|| IsExHeaderInAllView()
#endif
			) {
			if (!m_header.IsWindowVisible()) {
				m_header.ShowWindow(SW_SHOWNOACTIVATE);
				bHeaderChanged = true;
			}
		}
		else {
			if (m_header.IsWindowVisible()) {
				m_header.ShowWindow(SW_HIDE);
				bHeaderChanged = true;
			}
		}
	}

	CRect rc;
	GetClientRect(&rc);

m_bInsideUpdate = TRUE;
	ShowScrollBar(SB_BOTH, FALSE);
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.nPage = rc.Width();
	info.fMask = SIF_PAGE | SIF_RANGE;
	info.nMin = 0;
	info.nMax = 1;
	SetScrollInfo(SB_HORZ, &info, TRUE);
	info.nPage = rc.Height();
	SetScrollInfo(SB_VERT, &info, TRUE);
m_bInsideUpdate = FALSE;

	if (bHeaderChanged) {
		CRect rch;
		m_header.GetWindowRect(&rch);
		if (m_header.IsWindowVisible())
			rc.top += rch.Height();
		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}
	OnSize(SIZE_RESTORED, rc.Width(), rc.Height());
}

void CDrawListView::SetListStyle(int newStyle)
{
	if (newStyle != m_listStyle) {
		if (newStyle < 0 || newStyle >= DLS_STYLE_LAST)
			return;
		m_listStyle = newStyle;

		UpdateLayout();
	}
}

void CDrawListView::SetHorizontalExtent(int horzExtent)
{
	if (m_horzExtent != horzExtent) {
		m_horzExtent = horzExtent;
		if (IsReportStyle()) {
			CalcBars();

			if (m_header.IsWindow() && m_header.IsWindowVisible()) {
				CRect rch;
				m_header.GetWindowRect(&rch);

				CRect rc;
				GetWindowRect(&rc);
				int width = max(rc.Width(), horzExtent) + GetSystemMetrics(SM_CXVSCROLL);
				m_header.SetWindowPos(NULL, 0, 0, width, rch.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
	}
}

void CDrawListView::OnSetHeaderCtrl(WPARAM, LPARAM lParam)
{
	m_header = (HWND)lParam;
	if (m_header.IsWindow()) {
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
		m_fontHeader.CreateFontIndirect(&lf);
		m_header.SetFont(m_fontHeader, FALSE);

		CClientDC dc(nullptr);
		HFONT hfont = dc.SelectFont(m_fontHeader);
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		dc.SelectFont(hfont);

		int height = tm.tmHeight + tm.tmExternalLeading + 7;
		m_header.SetWindowPos(NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		if (IsReportStyle()
#ifdef __USE_HEADERINALLVIEWS
		 || IsExHeaderInAllView()
#endif
		 ) {
			if (!m_header.IsWindowVisible())
				m_header.ShowWindow(SW_SHOWNOACTIVATE);
		}
		else {
			if (m_header.IsWindowVisible())
				m_header.ShowWindow(SW_HIDE);
		}
	}
}

void CDrawListView::OnLayout(WPARAM wParam, LPARAM lParam)
{
	if (!lParam) return;

	CRect rc = *(RECT*)lParam;
	if (!m_header.IsWindow()) {
		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		Invalidate();
	}
	else {
		CRect rcl, rch;
		rcl = rc;
		rch = rc;
		if (IsReportStyle()) {
			m_header.GetWindowRect(&rc);
			rcl.top += rc.Height();
			int col = m_header.GetItemCount();
			int width = 0;
			HDITEM hd;
			for (int i = 0; i < col; i++) {
				hd.mask = HDI_WIDTH;
				if (m_header.GetItem(i, &hd))
					width += hd.cxy;
			}
			rch.right = rch.left + max(width, GetSystemMetrics(SM_CXSCREEN));
			rch.bottom = rcl.top;

			rch.OffsetRect(-GetScrollPos(SB_HORZ), 0);
			m_header.SetWindowPos(NULL, rch.left, rch.top, rch.Width(), rch.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
			m_header.Invalidate();
		}
#ifdef __USE_HEADERINALLVIEWS
		else if (IsExHeaderInAllView()) {
			m_header.GetWindowRect(&rc);
			rcl.top += rc.Height();
		}
#endif
		SetWindowPos(NULL, rcl.left, rcl.top, rcl.Width(), rcl.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		Invalidate();
	}
}

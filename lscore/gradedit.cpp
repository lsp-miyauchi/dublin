/////////////////////////////////////////////////////////////////////////////
// CGradEdit construction/destruction

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdafx.h"
#include <limits.h>
#include "resource.h"
#include "lscore.h"
#include "gradedit.h"
#include "common.h"
#include "resource.h"

CGradItem::CGradItem()
{
	m_next = NULL;
	m_cr = RGB(0, 0, 0);
	m_pos = 0;
	m_hide = FALSE;
}

static int compitem(const void *arg1, const void *arg2)
{
	CGradItem* p1 = *(CGradItem**)arg1;
	CGradItem* p2 = *(CGradItem**)arg2;

	return p1->m_pos - p2->m_pos;
}

static void _sortitems(CGradItem** pList)
{
	CGradItem* arr[64];

	CGradItem* ps = *pList;
	int cnt = 0;
	while (ps) {
		arr[cnt++] = ps;
		ps = ps->m_next;
	}

	qsort(arr, cnt, sizeof(CGradItem*), compitem);

	*pList = arr[0];
	ps = *pList;
	for (int i = 1; i < cnt; i++) {
		ps->m_next = arr[i];
		ps = ps->m_next;
	}
	ps->m_next = NULL;
}

extern "C"
LRESULT CALLBACK GradEditWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CGradEdit* pView;
	switch (msg) {
		case WM_NCCREATE:
			pView = new CGradEdit();
			pView->Attach(hwnd);
			pView->m_parent = ((LPCREATESTRUCT)lParam)->hwndParent;
			SetWindowLongPtr(hwnd, 0, (LONG_PTR)pView);
			return TRUE;

		case WM_NCDESTROY:
			pView = (CGradEdit*)GetWindowLongPtr(hwnd, 0);
			SetWindowLongPtr(hwnd, 0, 0);
			delete pView;
			return DefWindowProc(hwnd, msg, wParam, lParam);

		case WM_GETDLGCODE:
			return DLGC_WANTARROWS | DLGC_WANTCHARS;
	}

	pView = (CGradEdit*)GetWindowLongPtr(hwnd, 0);
	if (pView)
		return pView->WndProc(hwnd, msg, wParam, lParam);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//

CGradEdit::CGradEdit()
{
	m_list = NULL;
	m_him = NULL;
	m_bCaptured = FALSE;
	m_pDrag = NULL;
	m_numlimits = 0;
	m_crRef = RGB(0, 0, 0);
}

CGradEdit::~CGradEdit()
{
	CGradItem* ps = m_list;
	while (ps)
	{
		CGradItem* pw = ps->m_next;
		delete ps;
		ps = pw;
	}
	if (m_him)
		ImageList_Destroy(m_him);
}

LRESULT CGradEdit::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_msg.hwnd = hwnd;
	m_msg.message = msg;
	m_msg.wParam = wParam;
	m_msg.lParam = lParam;
	m_msg.time = GetMessageTime();
	m_msg.pt = CPoint(GetMessagePos());

	switch (msg)
	{
		case WM_CREATE:
			return OnCreate((LPCREATESTRUCT)lParam);

		case WM_SETCURSOR:
			if (OnSetCursor((HWND)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)))
				return TRUE;
			break;

		case WM_KILLFOCUS:
		case WM_SETFOCUS:
		{
			Invalidate();

			NMHDR hdr;
			hdr.hwndFrom = m_hWnd;
			hdr.idFrom = GetDlgCtrlID();
			hdr.code = (msg == WM_KILLFOCUS) ? NM_KILLFOCUS : NM_SETFOCUS;
			m_parent.SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
			return 0;
		}

		case WM_ERASEBKGND:
			return TRUE;

		case WM_SIZE:
			OnSize((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_PAINT:
			OnPaint();
			return 0;

		case WM_KEYDOWN:
			OnKeyDown((UINT)wParam, (UINT)LOWORD(lParam), (UINT)lParam);
			return 0;

		case WM_LBUTTONDOWN:
			OnLButtonDown((UINT)wParam, CPoint((DWORD)lParam));
			return 0;

		case WM_LBUTTONUP:
			OnLButtonUp((UINT)wParam, CPoint((DWORD)lParam));
			return 0;

		case WM_RBUTTONUP:
			OnRButtonUp((UINT)wParam, CPoint((DWORD)lParam));
			return 0;

		case WM_LBUTTONDBLCLK:
			OnLButtonDblClk((UINT)wParam, CPoint((DWORD)lParam));
			return 0;

		case WM_MOUSEMOVE:
			OnMouseMove((UINT)wParam, CPoint((DWORD)lParam));
			return 0;

		case WM_DESTROY:
			OnDestroy();
			break;

		case GEM_GETNUMCOLORS:
			return OnGetNumColors();

		case GEM_GETCOLORS:
			return (LRESULT)OnGetColors((GRAD_COLORS*)lParam);

		case GEM_SETCOLORS:
			return (LRESULT)OnSetColors((int)wParam, (GRAD_COLORS*)lParam);

		case GEM_SETNUMLIMIT:
			if (wParam >= 2)
			{
				m_numlimits = (int)wParam;
				return 1;
			}
			return 0;

		case GEM_GETNUMLIMIT:
			return m_numlimits;

		case GEM_REVERSEMAP:
			OnReverseMap();
			return 0;

		case WM_NOTIFY:
		{
			NMHDR* phdr = (NMHDR*)lParam;
			if (phdr->code == TTN_GETDISPINFO)
			{
				TCHAR buf[128];
				if (m_pDrag && !m_pDrag->m_hide)
				{
					int s = m_pDrag->m_pos / 10;
					int a = m_pDrag->m_pos % 10;
					wsprintf(buf, _T("%d.%d %%"), s, a);
				}
				else
					lstrcpy(buf, _T("---"));

				NMTTDISPINFO* pttd = (NMTTDISPINFO*)phdr;
				pttd->lpszText = buf;
				return 0;
			}
			break;
		}

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CGradEdit::Default()
{
	return DefWindowProc(m_msg.hwnd, m_msg.message, m_msg.wParam, m_msg.lParam);
}

int CGradEdit::OnCreate(LPCREATESTRUCT lpcs)
{
	CGradItem* ps = new CGradItem();
	m_list = ps;
	ps->m_pos = 0;
	CGradItem* pe = new CGradItem();
	pe->m_cr = RGB(255, 255, 255);
	pe->m_pos = 1000;
	ps->m_next = pe;

	m_him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_GRADITEM), GRADITEM_WIDTH, RGB(255, 0, 255));
	return 0;
}

BOOL CGradEdit::OnSetCursor(HWND hwnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT && hwnd == m_hWnd) {
		CPoint point(GetMessagePos());
		ScreenToClient(&point);
		int hit = HitTest(point);
		if (hit == htGradSpace) {
			if (!m_numlimits || (m_numlimits > OnGetNumColors())) {
				::SetCursor(LoadCursor(NULL, IDC_HAND));
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CGradEdit::OnDestroy()
{
	if (m_tip.IsWindow()) {
		m_tip.Activate(FALSE);
		m_tip.DestroyWindow();
	}
}

void CGradEdit::OnSize(UINT nType, int cx, int cy)
{
	m_scale = (double)(cx - GRADITEM_WIDTH) / 1000;
}

void CGradEdit::OnPaint()
{
	CPaintDC dc(m_hWnd);

	CRect rc;
	GetClientRect(&rc);
	HBITMAP hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());

	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	hbm = dcw.SelectBitmap(hbm);
	dcw.BitBlt(0, 0, rc.Width(), rc.Height(), dc, rc.left, rc.top, SRCCOPY);
	OnDraw(dcw);
	dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
	DeleteObject(dcw.SelectBitmap(hbm));
}

void CGradEdit::OnDraw(HDC hdc)
{
	CDCHandle dc = hdc;
	CRect rc;
	GetClientRect(&rc);

	CRect rcg = rc;
	rcg.left += (GRADITEM_WIDTH >> 1);
	rcg.right = rcg.left + rc.Width() - GRADITEM_WIDTH;
	rcg.bottom = rc.bottom - GRADITEM_HEIGHT;
	rcg.InflateRect(2, 0);
	draw_rect(dc, &rcg, GetSysColor(COLOR_3DSHADOW),
		(GetFocus() == m_hWnd) ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_WINDOW));
	rcg.DeflateRect(2, 2);

	CGradItem* ps = m_list;
	while (ps->m_hide)
		ps = ps->m_next;

	int posFrom = 0;
	COLORREF crFrom = ps->m_cr;

	CRect rci = rcg;
	while (ps) {
		if (!ps->m_hide) {
			rci.left = rcg.left + (int)(m_scale * posFrom);
			rci.right = rcg.left + (int)(m_scale * ps->m_pos);
			if (rci.Width() > 0) {
				if (crFrom != ps->m_cr)
					FillShadeRect(dc, rci, crFrom, ps->m_cr, FALSE);
				else
					dc.FillSolidRect(&rci, crFrom);
			}
			posFrom = ps->m_pos;
			crFrom = ps->m_cr;
		}
		ps = ps->m_next;
	}

	if (posFrom < 1000) {
		rci.left = rcg.left + (int)(m_scale * posFrom);
		rci.right = rcg.right;
		dc.FillSolidRect(&rci, crFrom);
	}

	rc.top = rc.bottom - GRADITEM_HEIGHT;
	dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));

	rcg.InflateRect(0, 2);
	rcg.top = rcg.bottom;
	rcg.bottom = rc.bottom;
	ps = m_list;
	while (ps) {
		if (!ps->m_hide) {
			rci = rcg;
			rci.left = rcg.left + (int)(m_scale * ps->m_pos) - (GRADITEM_WIDTH >> 1);
			rci.right = rci.left + GRADITEM_WIDTH;
			ImageList_Draw(m_him, 0, dc, rci.left, rci.top, ILD_TRANSPARENT);
			rci.top += (GRADITEM_HEIGHT - GRADITEM_WIDTH);
			rci.DeflateRect(2, 2);
			dc.FillSolidRect(&rci, ps->m_cr);
		}
		ps = ps->m_next;
	}
}

int CGradEdit::GetItemFromPoint(CPoint point, CGradItem** ppItem)
{
	CRect rc;
	GetClientRect(&rc);

	int width = rc.Width();
	rc.top = rc.bottom - GRADITEM_HEIGHT;
	rc.left += (GRADITEM_WIDTH >> 1);
	rc.right = rc.left + width - GRADITEM_WIDTH;

	int index = 0;
	CGradItem* ps = m_list;
	while (ps)
	{
		CRect rci = rc;
		rci.left = rc.left + (int)(m_scale * ps->m_pos) - (GRADITEM_WIDTH >> 1);
		rci.right = rci.left + GRADITEM_WIDTH;
		if (rci.PtInRect(point))
		{
			if (ppItem)
				*ppItem = ps;
			return index;
		}
		ps = ps->m_next;
		index++;
	}
	return -1;
}

int CGradEdit::HitTest(CPoint point)
{
	CRect rc;
	GetClientRect(&rc);

	if (!rc.PtInRect(point))
		return htError;

	CRect rcg = rc;
	rcg.bottom = rc.bottom - GRADITEM_HEIGHT;

	if (rcg.PtInRect(point))
	{
		rcg.left += (GRADITEM_WIDTH >> 1);
		rcg.right = rcg.left + rc.Width() - GRADITEM_WIDTH;
		rcg.DeflateRect(0, 2);
		return rcg.PtInRect(point) ? htGradBar : htClient;
	}

	rcg.top = rcg.bottom;
	rcg.bottom = rc.bottom;
	rcg.left += (GRADITEM_WIDTH >> 1);
	rcg.right = rcg.left + rc.Width() - GRADITEM_WIDTH;

	CGradItem* ps = m_list;
	while (ps)
	{
		CRect rci = rcg;
		rci.left = rcg.left + (int)(m_scale * ps->m_pos) - (GRADITEM_WIDTH >> 1);
		rci.right = rci.left + GRADITEM_WIDTH;
		if (rci.PtInRect(point))
			return htGradItem;
		ps = ps->m_next;
	}
	return rcg.PtInRect(point) ? htGradSpace : htClient;
}

void CGradEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	NMKEY key;
	key.hdr.hwndFrom = m_hWnd;
	key.hdr.idFrom = GetDlgCtrlID();
	key.hdr.code = NM_KEYDOWN;
	key.nVKey = nChar;
	key.uFlags = nFlags;
	if (m_parent.SendMessage(WM_NOTIFY, (WPARAM)key.hdr.idFrom, (LPARAM)&key))
		return;

	Default();
}

void CGradEdit::CreateToolTip()
{
	// Create the ToolTip control.
	m_tip = ::CreateWindow(TOOLTIPS_CLASS, TEXT(""),
							WS_POPUP,
							CW_USEDEFAULT, CW_USEDEFAULT,
							CW_USEDEFAULT, CW_USEDEFAULT,
							NULL, (HMENU)NULL, MODINST, NULL);
	if (!m_tip.IsWindow())
		return;

	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	ti.hwnd   = m_hWnd;
	ti.uId    = (UINT_PTR)m_hWnd;
	ti.hinst  = RESINST;
	ti.lpszText  = LPSTR_TEXTCALLBACK;
	ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0; 

    // Add the tool to the control, displaying an error if needed.
	if (!m_tip.AddTool(&ti)) {
		m_tip.DestroyWindow();
		return;
	}

	m_tip.TrackActivate(&ti, TRUE);
}

void CGradEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	Default();

	BOOL bStartCapture = FALSE;
	CGradItem* ps = NULL;
	int hit = HitTest(point);
	if (hit == htGradSpace) {
		if (m_numlimits > 0 && OnGetNumColors() >= m_numlimits)
			return;

		ps = new CGradItem();
		ps->m_cr = m_crRef;
		int pos = (int)((point.x - (GRADITEM_WIDTH >> 1)) / m_scale);
		ps->m_pos = min(1000, max(0, pos));

		ps->m_next = m_list->m_next;
		m_list->m_next = ps;
		_sortitems(&m_list);
		Invalidate();

		bStartCapture = TRUE;
	}
	else if (hit == htGradItem) {
		if (GetItemFromPoint(point, &ps) >= 0) {
			// ALTキー押下時は複製
			if (GetKeyState(VK_MENU) < 0) {
				CGradItem* psc = new CGradItem();
				psc->m_cr = ps->m_cr;
				psc->m_pos = ps->m_pos;
				psc->m_next = ps->m_next;
				ps->m_next = psc;
				Invalidate();
			}
			bStartCapture = TRUE;
		}
	}

	if (bStartCapture) {
		SetCapture();
		m_bCaptured = TRUE;
		m_ptBase = point;
		m_pDrag = ps;
		CreateToolTip();
	}
}

void CGradEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
	Default();
	if (!m_bCaptured)
		return;

	ReleaseCapture();
	m_bCaptured = FALSE;

	CGradItem* arr[64];
	CGradItem* ps = m_list;
	CGradItem* pw;
	int cnt = 0;
	while (ps) {
		pw = ps->m_next;
		if (!ps->m_hide)
			arr[cnt++] = ps;
		else
			delete ps;
		ps = pw;
	}

	m_list = ps = arr[0];
	for (int i = 1; i < cnt; i++) {
		ps->m_next = arr[i];
		ps = ps->m_next;
	}
	ps->m_next = NULL;

	m_parent.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), GEN_COLORCHANGE), (LPARAM)m_hWnd);

	if (m_tip.IsWindow()) {
		m_tip.Activate(FALSE);
		m_tip.DestroyWindow();
	}
}

void CGradEdit::OnRButtonUp(UINT nFlags, CPoint point)
{
	Default();

	if (m_bCaptured) {
		ReleaseCapture();
		m_bCaptured = FALSE;
		Invalidate();

		m_parent.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), GEN_COLORCHANGE), (LPARAM)m_hWnd);

		if (m_tip.IsWindow()) {
			m_tip.Activate(FALSE);
			m_tip.DestroyWindow();
		}
		return;
	}

	CGradItem* ps;
	if (GetItemFromPoint(point, &ps) >= 0) {
		ClientToScreen(&point);
		COLORREF cr = ps->m_cr;
		if (lccColorPickerEx(m_hWnd, point, &cr, _MKIR(4)) && cr != ps->m_cr) {
			m_crRef = ps->m_cr = cr;
			Invalidate();

			m_parent.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), GEN_COLORCHANGE), (LPARAM)m_hWnd);
		}
	}
}

void CGradEdit::OnMouseMove(UINT nFlags, CPoint point)
{
	Default();
	if (!m_bCaptured)
		return;

	CRect rc;
	GetClientRect(&rc);
	rc.top = rc.bottom - GRADITEM_HEIGHT;
	if (OnGetNumColors() > 1 && (point.y < rc.top || point.y > rc.bottom)) {
		if (!m_pDrag->m_hide)
			m_pDrag->m_hide = TRUE;
	}
	else {
		if (m_pDrag->m_hide)
			m_pDrag->m_hide = FALSE;

		int pos = (int)((point.x - (GRADITEM_WIDTH >> 1)) / m_scale);
		pos = min(1000, max(0, pos));
		m_pDrag->m_pos = pos;
		Invalidate();
	}
	_sortitems(&m_list);
	Invalidate();

	if (m_tip.IsWindow()) {
		ClientToScreen(&point);
		m_tip.TrackPosition(point.x + 15, point.y + 15);
	}
 }

void CGradEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	Default();

	CGradItem* ps;
	if (GetItemFromPoint(point, &ps) >= 0) {
		COLORREF cr;
		if (lccSelectCustomColor(m_hWnd, ps->m_cr, &cr) && cr != ps->m_cr) {
			m_crRef = ps->m_cr = cr;
			Invalidate();

			m_parent.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), GEN_COLORCHANGE), (LPARAM)m_hWnd);
		}
	}
}

BOOL CGradEdit::OnSetColors(int numcolors, GRAD_COLORS* pColors)
{
	if (numcolors < 1 || IsBadReadPtr(pColors, sizeof(GRAD_COLORS) * numcolors))
		return FALSE;

	if (m_numlimits > 0)
		numcolors = min(numcolors, m_numlimits);

	CGradItem* ps = m_list;
	while (ps) {
		CGradItem* pw = ps->m_next;
		delete ps;
		ps = pw;
	}

	CGradItem* pa = NULL;
	for (int i = 0; i < numcolors; i++) {
		ps = new CGradItem();
		ps->m_cr = pColors->cr;
		if (!pa)
			m_list = ps;
		else
			pa->m_next = ps;
		ps->m_pos = min(1000, max(0, pColors->pos));
		pa = ps;
		pColors++;
	}
	Invalidate();
	return TRUE;
}

int CGradEdit::OnGetColors(GRAD_COLORS* pColors)
{
	int cnt = 0;
	CGradItem* ps = m_list;
	while (ps) {
		if (IsBadWritePtr(pColors, sizeof(GRAD_COLORS)))
			return -1;

		pColors->cr = ps->m_cr;
		pColors->pos = ps->m_pos;
		pColors++;
		ps = ps->m_next;
		cnt++;
	}
	return cnt;
}

int CGradEdit::OnGetNumColors()
{
	int cnt = 0;
	CGradItem* ps = m_list;
	while (ps) {
		cnt++;
		ps = ps->m_next;
	}
	return cnt;
}

void CGradEdit::OnReverseMap()
{
	CGradItem* ps = m_list;
	while (ps) {
		ps->m_pos = 1000 - ps->m_pos;
		ps = ps->m_next;
	}
	_sortitems(&m_list);
	Invalidate();

	m_parent.SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), GEN_COLORCHANGE), (LPARAM)m_hWnd);
}

//////////////////////////////////////////////////////////////////////
// slider.h degree control
//

#pragma once
#ifndef __SLIDER_INCLUDE
#define __SLIDER_INCLUDE

class CSlider : public CWindowImpl<CSlider>
{
private:
	CToolTipCtrl m_tip;
	int m_min;
	int m_max;
	int m_pos;
	int m_step;
	int m_page;
	int m_freq;
	int m_buddyBase;
	MSG m_msg;
	HFONT m_hfont;
	CSize m_sizeThumb;
	HIMAGELIST m_himThumb;
	bool m_bCaptured : 1;

public:
	CSlider() :	m_min(0),
				m_max(100),
				m_pos(0),
				m_step(1),
				m_page(10),
				m_freq(1),
				m_buddyBase(0),
				m_himThumb(NULL),
				m_bCaptured(false)
	{
		m_hfont = GetStockFont(DEFAULT_GUI_FONT);
	}
	~CSlider()
	{
		if (m_himThumb)	ImageList_Destroy(m_himThumb);
	}


BEGIN_MSG_MAP(CSlider)
	ON_WM_CREATE()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_ENABLE()
	ON_WM_SETFONT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_PRINTCLIENT()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_DESTROY()
	ON_NOTIFY_CODE(TTN_GETDISPINFO, OnToolTipGetDispInfo)
	ON_MESSAGE(TBM_SETRANGE, OnSetRange)
	ON_MESSAGE(TBM_SETPOS, OnSetPos)
	ON_MESSAGE(TBM_SETPAGESIZE, OnSetPageSize)
	ON_MESSAGE(TBM_GETPOS, OnGetPos)
	ON_MESSAGE(TBM_SETBUDDYBASE, OnSetBuddyBase)
END_MSG_MAP()

protected:
	int OnCreate(LPCREATESTRUCT pcs)
	{
		m_sizeThumb = CSize(12, 16);
		m_himThumb = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_SLIDER_THUMB), m_sizeThumb.cx, RGB(255, 0, 255));
		return 0;
	}

	__inline UINT OnGetDlgCode()
	{
		return DLGC_WANTARROWS;
	}

	__inline void OnSize(UINT flags, int cx, int cy)
	{
	}

	__inline void OnSetFocus(HWND hOldWnd)
	{
		ChangeFocus(TRUE);
	}

	__inline void OnKillFocus(HWND hNewWnd)
	{
		ChangeFocus(FALSE);
	}

	__inline void OnDestroy()
	{
		EndToolTip();
	}

	void OnPrintClient(HDC hdc, LPARAM flags)
	{
		if (flags & PRF_ERASEBKGND)
			return;
		if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			OnDraw(hdc);
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		OnDraw(dc);
	}

	__inline BOOL OnEraseBkgnd(HDC hdc)
	{
		return TRUE;
	}

	void OnSetFont(HFONT hFont, BOOL bRedraw)
	{
		if (!hFont)	m_hfont = GetStockFont(DEFAULT_GUI_FONT);
		else		m_hfont = hFont;

		if (bRedraw) Invalidate();
	}

	__inline void OnEnable(BOOL bEnable)
	{
		Invalidate();
	}

	void OnLButtonDown(UINT nFlags, CPoint pt)
	{
		if (GetFocus() != m_hWnd)
			SetFocus();

		CRect rc;
		GetClientRect(&rc);
		rc.DeflateRect(m_sizeThumb.cx >> 1, 0);
		rc.right--;

		int range = m_max - m_min;
		int x = ((m_pos - m_min) * rc.Width()) / range;
		if (x - 3 > pt.x || pt.x > x + 3) {
			int pos = m_min + MulDivRN(pt.x, range, rc.Width());
			if (pos != m_pos) {
				SetPos(pos, TRUE);
				GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, m_pos), (LPARAM)m_hWnd);
			}
		}

		SetCapture();
		m_bCaptured = true;

		CClientDC dc(m_hWnd);
		DrawThumb(dc);

		InitToolTip();
	}

	void OnLButtonUp(UINT nFlags, CPoint pt)
	{
		if (!m_bCaptured)
			return;

		ReleaseCapture();
		m_bCaptured = false;

		CClientDC dc(m_hWnd);
		DrawThumb(dc);
		GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBPOSITION, m_pos), (LPARAM)m_hWnd);

		EndToolTip();
	}

	void OnMouseMove(UINT flags, CPoint pt)
	{
		if (m_bCaptured) {
			CRect rc;
			GetClientRect(&rc);
			rc.DeflateRect(m_sizeThumb.cx >> 1, 0);
			rc.right--;

			int x = max(rc.left, min(rc.right, pt.x));
			int range = m_max - m_min;
			int pos = m_min + MulDivRN(x, range, rc.Width());
			if (m_pos != pos) {
				SetPos(pos, TRUE);
				GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, m_pos), (LPARAM)m_hWnd);
			}

			if (m_tip.IsWindow()) {
				CPoint point = pt;
				ClientToScreen(&point);
				m_tip.TrackPosition(point.x + 15, point.y + 15);
			}
		}
	}

	BOOL OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		int pos = m_pos;
		UINT code;
		if (nChar == VK_LEFT || nChar == VK_DOWN) {
			pos = max(m_min, pos - m_step);
			code = TB_LINEDOWN;
		}
		else if (nChar == VK_RIGHT || nChar == VK_UP) {
			pos = min(m_max, pos + m_step);
			code = TB_LINEUP;
		}
		else if (nChar == VK_PRIOR) {
			pos = max(m_min, pos - m_page);
			code = TB_PAGEUP;
		}
		else if (nChar == VK_NEXT) {
			pos = min(m_max, pos + m_page);
			code = TB_PAGEDOWN;
		}
		else
			return FALSE;

		if (pos != m_pos) {
			m_pos = pos;
			Invalidate();
			UpdateWindow();
			GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(code, pos), (LPARAM)m_hWnd);
		}
		return TRUE;
	}

	BOOL OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		switch (nChar) {
		case VK_LEFT:
		case VK_DOWN:
		case VK_RIGHT:
		case VK_UP:
		case VK_PRIOR:
		case VK_NEXT:
			GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_ENDTRACK, m_pos), (LPARAM)m_hWnd);
			return TRUE;
		default:
			break;
		}
		return FALSE;
	}

	void OnCancelMode()
	{
		if (!m_bCaptured)
			return;

		ReleaseCapture();
		m_bCaptured = false;

		EndToolTip();
	}

	void OnToolTipGetDispInfo(int idCtrl, NMHDR* pHdr, LRESULT* pResult)
	{
		*pResult = 0;
		TCHAR buf[128];
		wsprintf(buf, _T("%d"), m_pos);

		NMTTDISPINFO* pttd = (NMTTDISPINFO*)pHdr;
		pttd->lpszText = buf;
	}

	LRESULT OnSetRange(WPARAM wParam, LPARAM lParam)
	{
#define YBAR_OFFSET			7

		int nMin = (int)(short)LOWORD(lParam);
		int nMax = (int)(short)HIWORD(lParam);
		BOOL bRedraw = (BOOL)wParam;

		if (nMin == nMax)
			return 0;

		if (nMin > nMax) {
			int n = nMin;
			nMin = nMax;
			nMax = n;
		}
		m_min = nMin;
		m_max = nMax;

		int pos = m_pos;
		if (pos >= nMin)
			pos = nMin;
		else if (pos > nMax)
			pos = nMax;

		SetPos(pos, bRedraw);
		return 0;
	}

	__inline LRESULT OnSetPos(WPARAM wParam, LPARAM lParam)
	{
		SetPos((int)lParam, (BOOL)wParam);
		return 0;
	}

	LRESULT OnSetPageSize(WPARAM wPara, LPARAM lParam)
	{
		int page = (int)lParam;
		int pageo = m_page;
		if (0 < page && page < (m_max - m_min))
			m_page = page;
		return (LRESULT)pageo;
	}

	LRESULT OnSetBuddyBase(WPARAM wParam, LPARAM lParam)
	{
		m_buddyBase = (int)wParam;
		Invalidate();
		return 0;
	}

	__inline LRESULT OnGetPos(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_pos;
	}

	void ChangeFocus(BOOL bHaveFocus)
	{
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

		NMHDR hdr;
		hdr.hwndFrom = m_hWnd;
		hdr.idFrom = GetDlgCtrlID();
		hdr.code = bHaveFocus ? NM_SETFOCUS : NM_KILLFOCUS;
		GetParent().SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
	}

	void SetPos(int pos, BOOL bRedraw)
	{
		if (pos < m_min)
			pos = m_min;
		else if (pos > m_max)
			pos = m_max;
		if (pos != m_pos) {
			m_pos = pos;
			if (bRedraw) {
				Invalidate();
				UpdateWindow();
			}
		}
	}

	__inline void OnDraw(HDC hdc)
	{
		DrawThumb(hdc);
	}

	void DrawThumb(HDC hdc)
	{
		CRect rc;
		GetClientRect(&rc);

		CRect rcw(0, 0, rc.Width(), rc.Height());
		HBITMAP hbm = CreateCompatibleBitmap(hdc, rcw.Width(), rcw.Height());
		CDC dcw = CreateCompatibleDC(hdc);
		hbm = dcw.SelectBitmap(hbm);

		HBRUSH hbr = CtlColor_Static(GetParent(), (WPARAM)(HDC)dcw, m_hWnd);
		if (!hbr)
			hbr = GetSysColorBrush(COLOR_BTNFACE);
		dcw.FillRect(&rcw, hbr);

		BOOL bEnabled = IsWindowEnabled();
		BOOL bFocus = (::GetFocus() == m_hWnd);

		COLORREF clrTopLeft = GetSysColor(COLOR_3DSHADOW);
		COLORREF clrBottomRight = GetSysColor(COLOR_3DHILIGHT);

		rcw.top += YBAR_OFFSET;
		rcw.bottom = rcw.top + 4;

		HPEN hpen = dcw.SelectPen(CreatePen(PS_SOLID, 0, clrTopLeft));
		dcw.MoveTo(rcw.left,      rcw.bottom - 1);
		dcw.LineTo(rcw.left,      rcw.top);
		dcw.MoveTo(rcw.left + 1,  rcw.top);
		dcw.LineTo(rcw.right - 1, rcw.top);

		if (clrTopLeft != clrBottomRight) {
			DeleteObject(dcw.SelectPen(hpen));
			hpen = dcw.SelectPen(CreatePen(PS_SOLID, 0, clrBottomRight));
		}

		dcw.MoveTo(rcw.right - 1, rcw.top + 1);
		dcw.LineTo(rcw.right - 1, rcw.bottom);
		dcw.MoveTo(rcw.right - 2, rcw.bottom);
		dcw.LineTo(rcw.left,      rcw.bottom);
		DeleteObject(dcw.SelectPen(hpen));

		if (bFocus) {
			COLORREF __fastcall mix_color(COLORREF clr1, COLORREF clr2, int clr1_per);

			rcw.DeflateRect(1, 1);
			COLORREF clr = mix_color(GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_HIGHLIGHT), 80);
			dcw.FillSolidRect(&rcw, clr);

			rcw.InflateRect(1, 1);
		}

		rcw.SetRect(0, 0, rc.Width(), rc.Height());
		rcw.DeflateRect(m_sizeThumb.cx >> 1, 0);
		rcw.right--;
		int d;
		int pos;
		int range = m_max - m_min;
		if (!(GetStyle() & TBS_NOTICKS)) {
			hpen = dcw.SelectPen(CreatePen(PS_SOLID, 0, clrTopLeft));
			double delta = (double)(rcw.Width() * m_step) / (double)range;
			pos = 1;
			for (int i = 0; i <= range; i += m_step) {
				if ((pos % m_freq) == 0) {
					d = rcw.left + (int)(delta * (double)i);
					dcw.MoveTo(d, rcw.top);
					dcw.LineTo(d, rcw.top + 2);
				}
				pos++;
			}
			DeleteObject(dcw.SelectPen(hpen));
		}

		if (m_himThumb) {
			d = ((m_pos - m_min) * rcw.Width()) / range;
			ImageList_Draw(m_himThumb,
						bEnabled ? ((bFocus || m_bCaptured) ? 1 : 0) : 2, dcw,
						d, (rcw.Height() - m_sizeThumb.cy + 1) / 2, ILD_TRANSPARENT);
		}
		BitBlt(hdc, rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
		DeleteObject(dcw.SelectBitmap(hbm));
	}

	void InitToolTip()
	{
		if (!(GetStyle() & TBS_TOOLTIPS))
			return;

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

	void EndToolTip()
	{
		if (m_tip.IsWindow()) {
			m_tip.Activate(FALSE);
			m_tip.DestroyWindow();
		}
	}
};


#endif		// __SLIDER_INCLUDEE

/////////////////////////////////////////////////////////////////////////
// simpleslider.h

#pragma once
#ifndef __LCC_SIMPLESLIDER_H__
#define __LCC_SIMPLESLIDER_H__

#include "..\lscore\muldiv32.h"

#define THUMBWIDTH				8

template <class T, class TBase = CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CSimpleSliderCtrlImpl : public ATL::CWindowImpl< T, TBase, TWinTraits>
{
private:
	int m_min;
	int m_max;
	int m_pos;
	int m_xofs;
	int m_step;
	int m_page;
	CBitmap m_bmMarker;
	CSize m_sizeMarker;
	COLORREF m_clrBkgnd;
	COLORREF m_clrMarker;
	CRect m_area;
	CWindow m_buddy;
	int m_buddyBase;
	bool m_bCaptured:1;

public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	CSimpleSliderCtrlImpl()
		: m_min(0)
		, m_max(100)
		, m_pos(0)
		, m_xofs(1)
		, m_step(1)
		, m_page(10)
		, m_bCaptured(false)
		, m_buddyBase(0)
		, m_clrBkgnd(GetSysColor(COLOR_3DFACE))
		, m_clrMarker(RGB(88, 88, 88))
	{
	}

	~CSimpleSliderCtrlImpl()
	{
	}

BEGIN_MSG_MAP(CSimpleSliderCtrlImpl)
	ON_WM_CREATE()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_MESSAGE_VOID(WM_ENABLE, OnEnable)
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
	ON_MESSAGE_VOID(TBM_SETRANGE, OnHandlerSetRange)
	ON_MESSAGE_VOID(TBM_SETPOS, OnHandlerSetPos)
	ON_MESSAGE_VOID(TBM_SETPAGESIZE, OnHandlerSetPageSize)
	ON_MESSAGE_VOID(TBM_SETBUDDYBASE, OnHandlerSetBuddyBase)
	ON_MESSAGE(TBM_SETBUDDY, OnHandlerSetBuddy)
	ON_MESSAGE(TBM_GETPOS, OnHandlerGetPos)
END_MSG_MAP()


protected:
	int OnCreate(LPCREATESTRUCT pcs)
	{
		CreateMarker();
		return 0;
	}

	inline UINT OnGetDlgCode() { return DLGC_WANTARROWS; }

	void OnSize(UINT flags, int cx, int cy)
	{
		m_area.SetRect(0, 0, cx, cy);
		m_area.DeflateRect(THUMBWIDTH >> 1, 0);
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		CDCHandle dc = hdc;
		CRect rc; GetClientRect(&rc);
		dc.FillSolidRect(&rc, m_clrBkgnd);
		return TRUE;
	}

	inline void OnEnable(WPARAM, LPARAM) { Invalidate(); }
	inline void OnSetFocus(HWND) { ChangeFocus(TRUE); }
	inline void OnKillFocus(HWND) { ChangeFocus(FALSE); }

	void OnPrintClient(HDC hdc, LPARAM flags)
	{
		if (flags & PRF_ERASEBKGND)
			OnEraseBkgnd(hdc);
		if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			OnDraw(hdc);
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		OnDraw(dc);
	}

	void OnDraw(HDC hdc)
	{
		CDCHandle dc = hdc;
		CRect rc = m_area;
		rc.top += (rc.Height() - (m_sizeMarker.cy + 1)) / 2;
		rc.bottom = rc.top + 1;
		rc.OffsetRect(0, 1);
		dc.FillSolidRect(&rc, m_clrMarker);
		rc.OffsetRect(0, 1);

		int range = m_max - m_min;
		int pos = m_pos - m_min;
		rc.left = m_area.left + ((pos * m_area.Width()) / range) - (THUMBWIDTH >> 1);
		rc.right = rc.left + THUMBWIDTH;
		rc.bottom = m_area.bottom;

		CDC dcw; dcw.CreateCompatibleDC(hdc);
		HBITMAP hbm = dcw.SelectBitmap(m_bmMarker);
		dc.SetBkColor(m_clrBkgnd);
		dc.SetTextColor(m_clrMarker);
		dc.BitBlt(rc.left, rc.top, m_sizeMarker.cx, m_sizeMarker.cy, dcw, 0, 0, SRCCOPY);
	}

	void OnLButtonDown(UINT nFlags, CPoint pt)
	{
		if (GetFocus() != m_hWnd)
			SetFocus();

		int range = m_max - m_min;
		int pos = m_pos - m_min;
		int x = m_area.left + ((pos * m_area.Width()) / range);
		if (x - (THUMBWIDTH >> 1) <= pt.x && pt.x <= x + (THUMBWIDTH >> 1))
			m_xofs = pt.x - x;
		else {
			if (pt.x <= m_area.left)
				pos = m_min;
			else if (pt.x >= m_area.right)
				pos = m_max;
			else {
				double delta =(double)(pt.x - m_area.left) / (double)m_area.Width();
				pos = m_min + (int)((double)range * delta);
			}
			SetPos(pos, TRUE);
			m_xofs = 0;
			GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, m_pos), (LPARAM)m_hWnd);
		}
		SetCapture();
		m_bCaptured = true;

		CClientDC dc(m_hWnd);
		OnDraw(dc);
	}

	void OnLButtonUp(UINT nFlags, CPoint pt)
	{
		if (!m_bCaptured)
			return;

		ReleaseCapture();
		m_bCaptured = false;

		CClientDC dc(m_hWnd);
		OnDraw(dc);

		GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBPOSITION, m_pos), (LPARAM)m_hWnd);
	}

	void OnMouseMove(UINT flags, CPoint pt)
	{
		if (m_bCaptured) {
			pt.x -= m_xofs;
			int x = max(m_area.left, min(m_area.right, pt.x)) - m_area.left;
			int range = m_max - m_min;
			int pos = m_min + MulDivRN(x, range, m_area.Width());
			if (m_pos != pos) {
				SetPos(pos, TRUE);
				GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, m_pos), (LPARAM)m_hWnd);
			}
		}
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		int pos = m_pos;
		UINT code;
		switch (nChar) {
		case VK_LEFT:
		case VK_DOWN:
			pos = max(m_min, pos - m_step);
			code = TB_LINEDOWN;
			break;
		case VK_RIGHT:
		case VK_UP:
			pos = min(m_max, pos + m_step);
			code = TB_LINEUP;
			break;
		case VK_PRIOR:
			pos = max(m_min, pos - m_page);
			code = TB_PAGEUP;
			break;
		case VK_NEXT:
			pos = min(m_max, pos + m_page);
			code = TB_PAGEDOWN;
			break;
		default:
			DefWindowProc();
			return;
		}

		if (pos != m_pos) {
			SetPos(pos, TRUE);
			GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(code, pos), (LPARAM)m_hWnd);
		}
	}

	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		switch (nChar) {
		case VK_LEFT:
		case VK_DOWN:
		case VK_RIGHT:
		case VK_UP:
		case VK_PRIOR:
		case VK_NEXT:
			GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_ENDTRACK, m_pos), (LPARAM)m_hWnd);
			return;
		default:
			break;
		}
		DefWindowProc();
	}

	void OnCancelMode()
	{
		if (!m_bCaptured) return;

		ReleaseCapture();
		m_bCaptured = FALSE;
	}

	inline void OnHandlerSetRange(WPARAM wParam, LPARAM lParam) { SetRange((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (BOOL)wParam); }
	inline void OnHandlerSetPos(WPARAM wParam, LPARAM lParam) { SetPos((int)lParam, (BOOL)wParam); }
	inline void OnHandlerSetPageSize(WPARAM, LPARAM lParam) { SetPageSize((int)lParam); }
	inline void OnHandlerSetBuddyBase(WPARAM wParam, LPARAM) { SetBuddyBase((int)wParam); }
	inline LRESULT OnHandlerSetBuddy(WPARAM, LPARAM lParam) { return (LRESULT)SetBuddy((HWND)lParam); }
	inline LRESULT OnHandlerGetPos(WPARAM, LPARAM) { return (LRESULT)m_pos; }

	void ChangeFocus(BOOL bHaveFocus)
	{
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

		NMHDR hdr;
		hdr.hwndFrom = m_hWnd;
		hdr.idFrom = GetDlgCtrlID();
		hdr.code = bHaveFocus ? NM_SETFOCUS : NM_KILLFOCUS;
		GetParent().SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
	}

	void CSimpleSliderCtrlImpl::SetRange(int nMin, int nMax, BOOL bRedraw)
	{
		if (nMin == nMax)
			return;

		if (nMin > nMax) {
			int n = nMin;
			nMin = nMax;
			nMax = n;
		}
		m_min = nMin;
		m_max = nMax;

		int pos = max(m_min, min(m_pos, m_max));
		SetPos(pos, bRedraw);
	}

	void SetPos(int pos, BOOL bRedraw)
	{
		pos = max(m_min, min(pos, m_max));
		if (pos != m_pos) {
			m_pos = pos;
			if (bRedraw && ::IsWindow(m_hWnd)) {
				Invalidate();
				UpdateWindow();
			}
			SetValueToBuddy();
		}
	}

	int SetPageSize(int page)
	{
		int pageo = m_page;
		if (0 < page && page < (m_max - m_min))
			m_page = page;
		return pageo;
	}

	void SetBuddyBase(int buddyBase)
	{
		m_buddyBase = buddyBase;
		SetValueToBuddy();
	}

	HWND SetBuddy(HWND hBuddy)
	{
		HWND hOldBuddy = m_buddy;
		if (::IsWindow(hBuddy)) {
			m_buddy = hBuddy;
			SetValueToBuddy();
		}
		return hOldBuddy;
	}

	void SetValueToBuddy()
	{
		#define ABS(a)	((a)>=0?(a):-(a))

		if (m_buddy.IsWindow()) {
			int pos = ABS(m_pos);

			CString str;
			if      (m_buddyBase == 1)	str.Format(_T("%d.%d"), pos / 10, pos % 10);
			else if (m_buddyBase == 2)	str.Format(_T("%d.%02d"), pos / 100, pos % 100);
			else						str.Format(_T("%d"), pos);
			if (m_pos < 0)
				str.Insert(0, _T('-'));

			m_buddy.SetWindowText(str);
			m_buddy.UpdateWindow();
		}
	}

	void CreateMarker()
	{
		// ---@@---		0xe7		0x18
		// --@@@@--		0xc3		0x3c
		// -@@@@@@-		0x81		0x7e
		// @@@@@@@@		0x00		0xff
		// @@@@@@@@		0x00		0xff

		CSimpleArray<short> bittbl;
		bittbl.Add(0xe7);
		bittbl.Add(0xc3);
		bittbl.Add(0x81);
		bittbl.Add(0x00);
		bittbl.Add(0x00);
		m_sizeMarker.cx = 8;
		m_sizeMarker.cy = bittbl.GetSize();
		m_bmMarker.CreateBitmap(m_sizeMarker.cx, m_sizeMarker.cy, 1, 1, bittbl.GetData());
	}

public:
	COLORREF SetBkgndColor(COLORREF clr)
	{
		COLORREF prev = m_clrBkgnd;
		if (m_clrBkgnd != clr) {
			m_clrBkgnd = clr;
			if (::IsWindow(m_hWnd))
				Invalidate();
		}
		return prev;
	}

	COLORREF SetMarkerColor(COLORREF clr)
	{
		COLORREF prev = m_clrMarker;
		if (m_clrMarker != clr) {
			m_clrMarker = clr;
			if (::IsWindow(m_hWnd))
				Invalidate();
		}
		return prev;
	}
};

class CSimpleSliderCtrl : public CSimpleSliderCtrlImpl<CSimpleSliderCtrl>
{
public:
	DECLARE_WND_CLASS_EX(_T("LSCSimpleSlider"), 0, -1)
};



#endif			// __LCC_SIMPLESLIDER_H__

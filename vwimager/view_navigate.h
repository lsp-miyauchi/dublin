///////////////////////////////////////////////////////////////////////
// view_navigate.h

#pragma once
#ifndef __VIEW_NAVIGATE_H__
#define __VIEW_NAVIGATE_H__

#include "container.h"
#include "simpleslider.h"

#define MIN_THUMBSIZE_X			128
#define MIN_THUMBSIZE_Y			128
#define MAX_THUMBSIZE_X			320
#define MAX_THUMBSIZE_Y			320

#define ROUND_INT(n)			((int)floor(0.5 + (n)))
#define FLOOR_INT(n)			((int)floor((n)))
#define CEIL_INT(n)				((int)ceil((n)))

#define NVN_CLICK				1
#define NVN_CLOSE				2
#define NVN_ZOOMIN				3
#define NVN_ZOOMOUT				4
#define NVN_SCALE				5

#define ID_ZOOM_SLIDER			999
#define SLIDER_Y_MARGIN			5
#define SLIDER_HEIGHT			12

typedef struct _NAVICLICK {
	NMHDR hdr;
	POINT pt;
	LPARAM param;
} NAVICLICK;

class CNavigateView : public CContainerImpl<CNavigateView>
{
private:
	Image* m_image;
	UINT m_id;
	CWindow m_owner;
	CSize m_sizeMinClient;
	CSize m_sizeMaxClient;
	CSize m_sizeImage;
	CRect m_rectOriginalViewport;		// 実サイズの表示枠領域
	CRect m_rectViewport;				// 縮尺後の表示枠領域
	CRect m_rectNaviPane;				// クライアント領域中のナビ画面領域
	CRect m_rectTrack;					// トラック中の矩形
	double m_scaleVisible;
	double m_scaleReal;
	BYTE m_cbLeyeredAlpha;
	CPoint m_ptTrackingOffset;
	bool m_bCaptured:1;
	CCursor m_cursor[3];
	CSimpleSliderCtrl m_slider;
	CAtlArray<double> m_arrScales;

public:
	CNavigateView()
		: m_image(NULL)
		, m_scaleVisible(0)
		, m_scaleReal(0)
		, m_id(0)
		, m_cbLeyeredAlpha(192)
		, m_bCaptured(false)
	{
		m_sizeMinClient = CSize(MIN_THUMBSIZE_X, MIN_THUMBSIZE_Y);
		m_sizeMaxClient = CSize(MAX_THUMBSIZE_X, MAX_THUMBSIZE_Y);
		m_sizeImage = CSize(0, 0);
		m_rectOriginalViewport.SetRectEmpty();
		m_rectNaviPane.SetRectEmpty();
		m_rectViewport.SetRectEmpty();

		SetBkColor(LCOLOR_IMAGEPANE);
		SetTitle(_T("Navigator"));
		SetPaneHeight(144 + SLIDER_HEIGHT + SLIDER_Y_MARGIN);

		// スケール値のリスト
		m_arrScales.Add(1.0);
		double d = 1.0;
		while (1) {
			d *= 0.96;
			if (d <= 0.1) {
				m_arrScales.InsertAt(0, 0.1);
				break;
			}
			m_arrScales.InsertAt(0, d);
		}
		d = 1.0;
		while (1) {
			d *= 1.04;
			if (d >= 8.0) {
				m_arrScales.Add(8.0);
				break;
			}
			m_arrScales.Add(d);
		}
	}

	~CNavigateView()
	{
		if (m_image) delete m_image;
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
	}

BEGIN_MSG_MAP(CNavigateView)
	CHAIN_MSG_MAP(CContainerImpl<CNavigateView>)
	ON_WM_CREATE()
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnHandlerMouseWheel)
	MESSAGE_HANDLER(WM_SETCURSOR, OnHandlerSetCursor)
	MESSAGE_HANDLER(WM_CLOSE, OnHandlerClose)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_KEYDOWN()
	ON_WM_HSCROLL()
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		m_cursor[0].LoadCursor(IC_HAND_POINT);
		m_cursor[1].LoadCursor(IC_HAND_OPEN);
		m_cursor[2].LoadCursor(IC_HAND_CLOSE);

		// zoom-slider
		m_slider.SetBkgndColor(LCOLOR_IMAGEPANE);
		m_slider.SetMarkerColor(RGB(255, 255, 255));
		m_slider.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, ID_ZOOM_SLIDER);
		m_slider.SendMessage(TBM_SETRANGE, FALSE, MAKELPARAM(0, (int)m_arrScales.GetCount() - 1));
		return 0;
	}

	void RecalcLayout(CRect& rc)
	{
		if (m_slider.IsWindow()) {
			if (rc.Height() > SLIDER_HEIGHT + SLIDER_Y_MARGIN) {
				CRect rcw = rc;
				rcw.top = rcw.bottom - (SLIDER_HEIGHT + SLIDER_Y_MARGIN);
				UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
				if (!m_slider.IsWindowVisible())
					flags |= SWP_SHOWWINDOW;
				m_slider.SetWindowPos(NULL, rc.left + (rc.Width() - 160) / 2, rcw.top, 160, SLIDER_HEIGHT, flags);
			}
			else
				m_slider.ShowWindow(SW_HIDE);
		}
		CalcNaviPosition(false);
	}

	void OnDraw(CDCHandle dc, CRect& rc)
	{
		GetClientRect(&rc);
		CDC dcm; dcm.CreateCompatibleDC(dc);
		CBitmap bmp; bmp.CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
		HBITMAP hbm = dcm.SelectBitmap(bmp);
		dcm.FillSolidRect(0, 0, rc.Width(), rc.Height(), LCOLOR_IMAGEPANE);
		if (m_image)
			Draw(dcm.m_hDC);
		dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcm, 0, 0, SRCCOPY);
		dcm.SelectBitmap(hbm);
	}

	void Draw(CDCHandle dc)
	{
		if (m_image && !m_rectNaviPane.IsRectEmpty()) {
			CRect rc = m_rectNaviPane;
			rc.InflateRect(1, 1);
			dc.FillSolidRect(&rc, RGB(255, 255, 255));

			Graphics g(dc);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);
			g.DrawImage(m_image, m_rectNaviPane.left, m_rectNaviPane.top, m_rectNaviPane.Width(), m_rectNaviPane.Height());

			if (!m_rectViewport.IsRectEmpty()) {
				// Create a SolidBrush object.
				SolidBrush br(Color(128, 200, 200, 255));
				g.FillRectangle(&br, m_rectViewport.left, m_rectViewport.top, m_rectViewport.Width(), m_rectViewport.Height());

				// Create a Pen object.
				Pen pen(Color(255, 255, 0,0), 1);
				g.DrawRectangle(&pen, m_rectViewport.left, m_rectViewport.top, m_rectViewport.Width(), m_rectViewport.Height());
			}
		}
	}

	LRESULT OnHandlerSetCursor(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HWND hwnd  = (HWND)wParam;
		UINT nHitTest = (UINT)LOWORD(lParam);
		if ((nHitTest == HTCLIENT) && (hwnd == m_hWnd)) {
			CPoint point(GetMessagePos());
			ScreenToClient(&point);
			if (m_rectNaviPane.PtInRect(point)) {
				if (m_rectViewport.PtInRect(point))
					::SetCursor(m_cursor[1]);
				else
					::SetCursor(m_cursor[0]);
				return TRUE;
			}
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnHandlerClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (IsWindowVisible()) {
			ShowWindow(SW_HIDE);
			SendNaviNotify(CPoint(0,0), NVN_CLOSE);
		}
		return 0;
	}

	LRESULT OnHandlerMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		UINT keyState = GET_KEYSTATE_WPARAM(wParam);
		if (keyState & (MK_CONTROL | MK_MBUTTON)) {
			short delta = GET_WHEEL_DELTA_WPARAM(wParam);
			SendNaviNotify(pt, (delta > 0) ? NVN_ZOOMIN : NVN_ZOOMOUT);
		}
		else
			bHandled = FALSE;
		return 0;
	}

	void OnLButtonDown(UINT fwKeys, CPoint point)
	{
		if (!m_owner.IsWindow() ||
			m_rectNaviPane.IsRectEmpty() ||
			!m_rectNaviPane.PtInRect(point)) return;

		if (!m_rectViewport.PtInRect(point)) {
			SendNaviNotify(point, NVN_CLICK);
		}

		m_bCaptured = true;
		SetCapture();
		::SetCursor(m_cursor[2]);

		m_rectTrack = m_rectViewport;
		m_ptTrackingOffset.x = point.x - m_rectTrack.left;
		m_ptTrackingOffset.y = point.y - m_rectTrack.top;
	}

	void OnLButtonUp(UINT /*nFlags*/, CPoint point)
	{
		if (!m_bCaptured) return;

		UpdateTrackingRect(point);

		m_bCaptured = false;
		ReleaseCapture();
		::SetCursor(m_cursor[0]);
	}

	void OnMouseMove(UINT /*nFlags*/, CPoint point)
	{
		if (!m_bCaptured) return;

		UpdateTrackingRect(point);
	}

	void OnCancelMode()
	{
		if (m_bCaptured) {
			m_bCaptured = false;
			ReleaseCapture();
			::SetCursor(m_cursor[0]);
		}
		DefWindowProc();
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		switch (nChar) {
		case VK_TAB:
			if (m_owner.IsWindow())
				m_owner.SetActiveWindow();
			break;
		}
	}

	void OnHScroll(UINT nSBCode, UINT nPos, HWND hCtrl)
	{
		if (hCtrl == m_slider) {
			int val;
			switch (nSBCode) {
			case TB_TOP:
			case TB_BOTTOM:
			case TB_LINEUP:
			case TB_LINEDOWN:
			case TB_PAGEUP:
			case TB_PAGEDOWN:
				val = (int)m_slider.SendMessage(TBM_GETPOS);
				break;
			case TB_THUMBTRACK:
				val = (int)nPos;
				break;
			default:
				return;
			}
			if (0 <= val && val < (int)m_arrScales.GetCount()) {
				double scale = m_arrScales[val];
				SendNaviNotify(CPoint(0, 0), NVN_SCALE, (LPARAM)(scale * 1000));
			}
		}
	}

	bool CalcNaviPosition(bool bRedraw)
	{
		int dx = m_sizeImage.cx;
		int dy = m_sizeImage.cy;
		if (dx > 0 && dy > 0) {
			CRect rc;
			GetPaneRect(&rc);
			if (m_slider.IsWindow() && m_slider.IsWindowVisible())
				rc.bottom -= (SLIDER_HEIGHT + SLIDER_Y_MARGIN);
			rc.InflateRect(-5, -5);		// マージン

			m_scaleVisible = CalcInnerImageSize(rc.Width(), rc.Height(), &dx, &dy);
			m_rectNaviPane.SetRect(0, 0, dx, dy);
			m_rectNaviPane.OffsetRect(rc.left + (rc.Width() - dx) / 2, rc.top + (rc.Height() - dy) / 2);
			if (!m_rectOriginalViewport.IsRectEmpty()) {
				m_rectViewport.left = FLOOR_INT(m_rectOriginalViewport.left * m_scaleVisible);
				m_rectViewport.right = m_rectViewport.left + ROUND_INT(m_rectOriginalViewport.Width() * m_scaleVisible);
				m_rectViewport.top = FLOOR_INT(m_rectOriginalViewport.top * m_scaleVisible);
				m_rectViewport.bottom = m_rectViewport.top + ROUND_INT(m_rectOriginalViewport.Height() * m_scaleVisible);
				m_rectViewport.OffsetRect(m_rectNaviPane.left, m_rectNaviPane.top);
			}

			if (bRedraw && IsWindowVisible()) {
				Invalidate(FALSE);
				if (m_bCaptured) {
					// 自分がキャプチャしてる場合はすぐに画面更新
					UpdateWindow();
				}
			}
			return true;
		}
		return false;
	}

	void UpdateTrackingRect(CPoint point)
	{
		CRect rc(0, 0, m_rectTrack.Width(), m_rectTrack.Height());
		rc.OffsetRect(point.x - m_ptTrackingOffset.x, point.y - m_ptTrackingOffset.y);

		if (rc.left < m_rectNaviPane.left)		rc.OffsetRect(m_rectNaviPane.left - rc.left, 0);
		if (rc.top < m_rectNaviPane.top)		rc.OffsetRect(0, m_rectNaviPane.top - rc.top);
		if (rc.right > m_rectNaviPane.right)	rc.OffsetRect(m_rectNaviPane.right - rc.right, 0);
		if (rc.bottom > m_rectNaviPane.bottom)	rc.OffsetRect(0, m_rectNaviPane.bottom - rc.bottom);

		if (rc != m_rectTrack) {
			m_rectTrack = rc;
			SendNaviNotify(m_rectTrack.CenterPoint(), NVN_CLICK);
		}
	}

	void UpdateVisiblePosition(RECT* prc)
	{
		if (prc) {
			m_rectOriginalViewport = *prc;
			if (::IsWindow(m_hWnd))
				CalcNaviPosition(true);
		}
	}

	int ScaleToSliderIndex(double scale)
	{
		// スケール値のリストから近似の値のインデクスを取得する
		int cnt = (int)m_arrScales.GetCount();
		for (int i = 0; i < cnt; i++) {
			if (scale <= m_arrScales[i]) {
				int index = i;
				if (i > 0) {
					if (scale < (m_arrScales[i - 1] + m_arrScales[i]) / 2.0)
						index = i - 1;
				}
				return index;
			}
		}
		return cnt - 1;
	}

	void UpdateScale(double* scaleptr)
	{
		double scale = *scaleptr;
		if (m_scaleReal != scale) {
			m_scaleReal = scale;
			if (m_slider.IsWindow()) {
				int index = ScaleToSliderIndex(m_scaleReal);
				m_slider.SendMessage(TBM_SETPOS, TRUE, index);
			}
		}
	}

	void SetImage(HDIB hdib)
	{
		if (m_image) {
			delete m_image;
			m_image = NULL;
		}

		Bitmap* bitmap = GdipGetBitmapFromDIB(hdib);
		if (bitmap) {
			m_sizeImage.cx = bitmap->GetWidth();
			m_sizeImage.cy = bitmap->GetHeight();

			int cx = m_sizeImage.cx;
			int cy = m_sizeImage.cy;
			CalcInnerImageSize(m_sizeMaxClient.cx, m_sizeMaxClient.cy, &cx, &cy);

			m_image = bitmap->GetThumbnailImage(cx, cy, NULL, NULL);
			if (IsWindow())
				CalcNaviPosition(true);

			delete bitmap;
		}
	}

	void SendNaviNotify(POINT point, UINT code, LPARAM param = 0)
	{
		NAVICLICK nc;
		nc.hdr.hwndFrom = m_hWnd;
		nc.hdr.idFrom = m_id;
		nc.hdr.code = code;
		nc.pt.x = ROUND_INT((point.x - m_rectNaviPane.left) / m_scaleVisible);
		nc.pt.y = ROUND_INT((point.y - m_rectNaviPane.top) / m_scaleVisible);
		nc.param = param;
		m_owner.SendMessage(WM_NOTIFY, (WPARAM)nc.hdr.idFrom, (LPARAM)&nc);
	}

	void SetOwnerInfo(HWND hwndOwner, ULONG id)
	{
		m_owner = ::IsWindow(hwndOwner) ? hwndOwner : NULL;
		m_id = (UINT)id;
	}
};


#endif	// __VIEW_NAVIGATE_H__

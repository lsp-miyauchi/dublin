////////////////////////////////////////////////////////////////////////
// imagebuttonctrl.h

#pragma once
#ifndef __IMAGEBUTTONCTRL_H__
#define __IMAGEBUTTONCTRL_H__

#include "resource.h"

////////////////////////////////////////////////////////////////////////
//

inline int Point2Logical(HDC hdc, int nPointSize)
{
	return ::MulDiv(nPointSize, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
}

////////////////////////////////////////////////////////////////////////
//

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CBtnCtrlWndImpl : public CWindowImpl< T, TBase, TWinTraits>
{
protected:
	bool m_in:1;
	bool m_pushed:1;
	bool m_captured:1;
	bool m_trackmouse:1;

public:
	CBtnCtrlWndImpl()
		: m_in(false)
		, m_pushed(false)
		, m_captured(false)
		, m_trackmouse(false)
	{
	}
	~CBtnCtrlWndImpl()
	{
	}

	DECLARE_WND_CLASS(NULL)

typedef CBtnCtrlWndImpl< T, TBase, TWinTraits > thisClass;
BEGIN_MSG_MAP(this)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnHandlerEraseBkgnd)
	MESSAGE_HANDLER(WM_PAINT, OnHandlerPaint)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnHandlerLButtonDown)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnHandlerMouseMove)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnHandlerMouseLeave)
END_MSG_MAP()

	LRESULT OnHandlerEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)
	{
		return TRUE;
	}

	LRESULT OnHandlerPaint(UINT, WPARAM, LPARAM, BOOL&)
	{
		CPaintDC dc(m_hWnd);
		T* pT = static_cast<T*>(this);
		pT->OnDraw(dc);
		return 0;
	}

	virtual void OnDraw(HDC hdc)
	{
	}

	LRESULT OnHandlerLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		if (m_captured) return 0;

		CRect rc;
		GetClientRect(&rc);

		if (OnTrackMouse(rc))
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
		return 0;
	}

	bool OnTrackMouse(const CRect& rcr)
	{
		SetCapture();
		m_captured = true;
		m_pushed = true;

		CClientDC dc(m_hWnd);

		T* pT = static_cast<T*>(this);
		pT->OnDraw(dc);

		bool bCont = true;
		while (bCont) {
			MSG msg;
			while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
				if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
					CPoint pt(LOWORD(msg.lParam), HIWORD(msg.lParam));
					m_in = (rcr.PtInRect(pt) != FALSE);

					if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP) {
						bCont = false;
						break;
					}
				}
				else {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		m_pushed = false;
		pT->OnDraw(dc);

		m_captured = false;
		ReleaseCapture();

		return m_in;
	}

	LRESULT OnHandlerMouseMove(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		if (m_captured) return 0;

		if (!m_trackmouse) {
			CRect rc;
			GetClientRect(&rc);
			m_in = (rc.PtInRect(pt) != FALSE);

			TRACKMOUSEEVENT te;
			te.cbSize = sizeof(te);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = m_hWnd;
			te.dwHoverTime = HOVER_DEFAULT;
			m_trackmouse = (_TrackMouseEvent(&te) != FALSE);

			if (m_trackmouse) {
				Invalidate(FALSE);
				UpdateWindow();
			}
		}
		return 0;
	}

	LRESULT OnHandlerMouseLeave(UINT, WPARAM, LPARAM, BOOL&)
	{
		if (m_trackmouse) {
			m_trackmouse = false;
			m_in = false;
			Invalidate(FALSE);
			UpdateWindow();
		}
		return 0;
	}
};


////////////////////////////////////////////////////////////////////////
//

#define STARTBUTTON_CLASS			_T("VWSTART_BUTTON")

class CStartButtonWnd : public CBtnCtrlWndImpl<CStartButtonWnd>
{
private:
	HIMAGELIST m_himStart;
	HIMAGELIST m_himPause;
	CSize m_sizeImage;
	bool m_paused:1;

public:
	DECLARE_WND_CLASS_EX(STARTBUTTON_CLASS, CS_PARENTDC | CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

	CStartButtonWnd()
		: m_paused(true)
		, m_himStart(NULL)
		, m_himPause(NULL)
		, m_sizeImage(CSize(32, 32))
	{
	}

	~CStartButtonWnd()
	{
		if (m_himStart) ImageList_Destroy(m_himStart);
		if (m_himPause) ImageList_Destroy(m_himPause);
	}

BEGIN_MSG_MAP(CStartButtonWnd)
	CHAIN_MSG_MAP(CBtnCtrlWndImpl<CStartButtonWnd>)
	ON_WM_CREATE()
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		// イメージリスト
		m_himStart = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_SLIDESHOW_START), m_sizeImage.cx, RGB(255, 0, 255));
		m_himPause = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_SLIDESHOW_PAUSE), m_sizeImage.cx, RGB(255, 0, 255));
		return 0;
	}

	void OnDraw(HDC hdc)
	{
		DrawButton(hdc);
	}

	void DrawButton(HDC hdc)
	{
		// CS_PARENTDC を指定しているので、背景は親で既に描かれている（はず）

		CRect rc;
		GetClientRect(&rc);

		// ボタン
		HIMAGELIST him = m_paused ? m_himStart : m_himPause;
		if (him) {
			int index = m_pushed ? 2 : (m_in ? 1 : 0);
			ImageList_Draw(him, index, hdc,
						rc.left + (rc.Width() - m_sizeImage.cx) / 2,
						rc.top + (rc.Height() - m_sizeImage.cy) / 2, ILD_TRANSPARENT);
		}
	}

	bool IsPaused()
	{
		return m_paused;
	}

	bool SetPause(bool pause)
	{
		bool old = m_paused;
		m_paused = pause;
		if (m_paused != old) {
			Invalidate();
			UpdateWindow();
		}
		return old;
	}
};


////////////////////////////////////////////////////////////////////////
//

#define IMAGEBUTTON_CLASS				_T("VWIMAGE_BUTTON")

class CImageButtonWnd : public CBtnCtrlWndImpl<CImageButtonWnd>
{
private:
	UINT m_idResource;
	HIMAGELIST m_him;
	CSize m_sizeImage;

public:
	DECLARE_WND_CLASS_EX(IMAGEBUTTON_CLASS, CS_PARENTDC | CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

	CImageButtonWnd()
		: m_idResource(0)
		, m_him(NULL)
		, m_sizeImage(CSize(0, 0))
	{
	}

	~CImageButtonWnd()
	{
		if (m_him) ImageList_Destroy(m_him);
	}

	void SetImageResource(UINT idResource, SIZE sizeImage)
	{
		m_idResource = idResource;
		m_sizeImage = sizeImage;
	}

BEGIN_MSG_MAP(CImageButtonWnd)
	CHAIN_MSG_MAP(CBtnCtrlWndImpl<CImageButtonWnd>)
	ON_WM_CREATE()
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		// イメージリスト
		if (m_idResource) {
			m_him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(m_idResource), m_sizeImage.cx, RGB(255, 0, 255));
		}
		return 0;
	}

	void OnDraw(HDC hdc)
	{
		DrawButton(hdc);
	}

	void DrawButton(HDC hdc)
	{
		// CS_PARENTDC を指定しているので、背景は親で既に描かれている（はず）
		CRect rc;
		GetClientRect(&rc);

		// ボタン
		if (m_him) {
			int index = m_pushed ? 2 : (m_in ? 1 : 0);
			ImageList_Draw(m_him, index, hdc,
						rc.left + (rc.Width() - m_sizeImage.cx) / 2,
						rc.top + (rc.Height() - m_sizeImage.cy) / 2, ILD_TRANSPARENT);
		}
	}
};


#endif		// __IMAGEBUTTONCTRL_H__

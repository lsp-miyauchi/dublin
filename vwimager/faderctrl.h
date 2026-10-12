////////////////////////////////////////////////////////////////////////
// faderctrl.h

#pragma once
#ifndef __FADERCTRL_H__
#define __FADERCTRL_H__

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CFaderCtrlImpl : public CWindowImpl< T, TBase, TWinTraits>
{
private:
	COLORREF m_clrBkgnd;
	bool m_bFading:1;

public:
	DECLARE_WND_CLASS(NULL)

	CFaderCtrlImpl()
		: m_clrBkgnd(RGB(0, 0, 0))
		, m_bFading(false)
	{
	}

	~CFaderCtrlImpl()
	{
	}

BEGIN_MSG_MAP(this)
	MESSAGE_HANDLER(WM_CREATE, OnHandlerCreate)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnHandlerEraseBkgnd)
END_MSG_MAP()

	LRESULT OnHandlerCreate(UINT, WPARAM, LPARAM, BOOL&)
	{
		return 0;
	}

	LRESULT OnHandlerEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
	{
		CRect rc;
		GetClientRect(&rc);
		CDCHandle dc((HDC)wParam);
		dc.FillSolidRect(&rc, m_clrBkgnd);
		return TRUE;
	}

	bool IsTransparent()
	{
		return ((::GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) != 0);
	}

	void SetTransparent(int opaque)
	{
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		::SetLayeredWindowAttributes(m_hWnd, 0, opaque, LWA_ALPHA);
	}

	void ResetTransparent()
	{
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
	}

	void Fade()
	{
		AnimateLayer();
	}

protected:
	void ZapTimer(DWORD zapcount)
	{
		DWORD timeout = GetTickCount() + zapcount;
		MSG msg;
		while (timeout > GetTickCount()) {
			if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	void AnimateLayer()
	{
		if (!IsTransparent()) SetTransparent(255);

		m_bFading = true;
		int i = 0;
		while (m_bFading && i < 256) {
			::SetLayeredWindowAttributes(m_hWnd, 0, 255 - i, LWA_ALPHA);
			UpdateWindow();
			ZapTimer(60);
			i += 16;
		}
		m_bFading = false;

		ResetTransparent();
	}

};

class CFaderCtrl : public CFaderCtrlImpl<CFaderCtrl>
{
public:
};

#endif		// __FADERCTRL_H__

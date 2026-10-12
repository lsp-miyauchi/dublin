////////////////////////////////////////////////////////////////////////
// staticctrl.h

#pragma once
#ifndef __STATICCTRL_H__
#define __STATICCTRL_H__

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CDefStaticCtrlImpl : public CWindowImpl< T, TBase, TWinTraits>
{
protected:
	HFONT m_hFont;

public:
	CDefStaticCtrlImpl()
	{
		m_hFont = GetStockFont(DEFAULT_GUI_FONT);
	}

	~CDefStaticCtrlImpl()
	{
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

typedef CDefStaticCtrlImpl< T, TBase, TWinTraits >	thisClass;
BEGIN_MSG_MAP(this)
	MESSAGE_HANDLER(WM_GETDLGCODE, OnHandlerGetDlgCode)
	MESSAGE_HANDLER(WM_CREATE, OnHandlerCreate)
	MESSAGE_HANDLER(WM_SETFONT, OnHandlerSetFont)
	MESSAGE_HANDLER(WM_SETTEXT, OnHandlerSetText)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnHandlerEraseBkgnd)
	MESSAGE_HANDLER(WM_PAINT, OnHandlerPaint)
	MESSAGE_HANDLER(WM_PRINTCLIENT, OnHandlerPrintClient)
END_MSG_MAP()

	LRESULT OnHandlerCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->OnCreateClient((LPCREATESTRUCT)lParam);
	}

	LRESULT OnHandlerGetDlgCode(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnGetDlgCode();
	}

	LRESULT OnHandlerSetFont(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lr = DefWindowProc();
		m_hFont = wParam ? (HFONT)wParam : GetStockFont(DEFAULT_GUI_FONT);
		if (lParam)
			Invalidate(TRUE);
		return lr;
	}

	LRESULT OnHandlerSetText(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lr = DefWindowProc();
		Invalidate(TRUE);
		return lr;
	}

	LRESULT OnHandlerEraseBkgnd(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->OnEraseBkgnd((HDC)wParam);
	}

	LRESULT OnHandlerPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPaintDC dc(m_hWnd);
		T* pT = static_cast<T*>(this);
		pT->OnDraw(dc);
		return 0;
	}

	LRESULT OnHandlerPrintClient(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		if (lParam & PRF_ERASEBKGND)
			pT->OnEraseBkgnd((HDC)wParam);
		if (lParam & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			pT->OnDraw((HDC)wParam);
		return 0;
	}

	virtual int OnCreateClient(LPCREATESTRUCT pcs)
	{
		return 0;
	}

	virtual LRESULT OnGetDlgCode()
	{
		return DLGC_STATIC;
	}

	virtual BOOL OnEraseBkgnd(HDC hdc)
	{
		CRect rc;
		GetClientRect(&rc);
		HBRUSH hbr = CtlColor_Static(GetParent(), (WPARAM)hdc, m_hWnd);
		if (!hbr) hbr = GetSysColorBrush(COLOR_BTNFACE);
		FillRect(hdc, &rc, hbr);
		return TRUE;
	}

	virtual void OnDraw(HDC hdc)
	{
	}
};



#endif				// __STATICCTRL_H__


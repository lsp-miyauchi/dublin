/////////////////////////////////////////////////////////////////////////////
// CRectCtrl

#pragma once
#ifndef __lcc_include_rectctrl_h__
#define __lcc_include_rectctrl_h__


template <class TBase>
class CRectCtrlT : public TBase
{
public:
// Constructors
	CRectCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CRectCtrlT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return CWindow::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_LSC_RECT;
	}

	COLORREF GetTextColor()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RM_GETTEXTCOLOR, 0, 0);
	}

	COLORREF GetBkColor()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RM_GETBKCOLOR, 0, 0);
	}

	COLORREF GetEdgeColor()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RM_GETEDGECOLOR, 0, 0);
	}

	COLORREF SetTextColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RM_SETTEXTCOLOR, 0, (LPARAM)clr);
	}

	COLORREF SetBkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RM_SETBKCOLOR, 0, (LPARAM)clr);
	}

	COLORREF SetEdgeColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RM_SETEDGECOLOR, 0, (LPARAM)clr);
	}

	void SetIcon(HICON hIcon)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RM_SETICON, 0, (LPARAM)hIcon);
	}
};

typedef CRectCtrlT<CWindow>	CRectCtrl;


#endif			// _lcc_include_rectctrl_h__

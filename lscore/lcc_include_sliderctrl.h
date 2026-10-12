/////////////////////////////////////////////////////////////////////////////
// CLccSliderCtrl

#pragma once
#ifndef __lcc_include_sliderctrl_h__
#define __lcc_include_sliderctrl_h__


template <class TBase>
class CLccSliderCtrlT : public TBase
{
public:
// Constructors
	CLccSliderCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CLccSliderCtrlT< TBase >& operator=(HWND hWnd)
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
		return WC_LSC_SLIDER;
	}

	void SetPos(int nPos, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETPOS, bRedraw, nPos);
	}

	void SetRange(int nLower, int nUpper, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETRANGE, bRedraw, MAKELPARAM(nUpper, nLower));
	}

	void SetPageSize(int nPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETPAGESIZE, 0, nPage);
	}

	int GetPos()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETPOS, 0, 0);
	}

	void SetBuddyBase(int buddyBase)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETBUDDYBASE, (WPARAM)buddyBase, 0);
	}

};

typedef CLccSliderCtrlT<CWindow>	CLccSliderCtrl;


#endif			// __lcc_include_sliderctrl_h__

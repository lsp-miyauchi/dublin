/////////////////////////////////////////////////////////////////////////////
// CTrackCtrl

#pragma once
#ifndef __lcc_include_trackctrl_h__
#define __lcc_include_trackctrl_h__

template <class TBase>
class CTrackCtrlT : public TBase
{
public:
// Constructors
	CTrackCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CTrackCtrlT< TBase >& operator=(HWND hWnd)
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
		return WC_LSC_TRACK;
	}

	void SetBuddy(HWND hWndBuddy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETBUDDY, 0, (LPARAM)hWndBuddy);
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

	void SetSelection(int nLower, int nUpper, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETSEL, bRedraw, MAKELPARAM(nUpper, nLower));
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

	void SetTicFreq(int nFreq)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETTICFREQ, (WPARAM)nFreq, 0);
	}

	HIMAGELIST SetThumbImage(HIMAGELIST him)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)::SendMessage(m_hWnd, TBM_SETTHUMBIMAGE, 0, (LPARAM)him);
	}

	HIMAGELIST GetThumbImage()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)::SendMessage(m_hWnd, TBM_GETTHUMBIMAGE, 0, 0);
	}

	void SetSuffix(LPCTSTR pszSuffix)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETSUFFIX, 0, (LPARAM)pszSuffix);
	}
};

typedef CTrackCtrlT<CWindow>	CTrackCtrl;

#endif			// __lcc_include_trackctrl_h__

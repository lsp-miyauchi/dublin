/////////////////////////////////////////////////////////////////////////////
// CAnimationImageCtrl

#pragma once
#ifndef __lcc_include_animationimagectrl_h__
#define __lcc_include_animationimagectrl_h__


template <class TBase>
class CAnimationImageCtrlT : public TBase
{
public:
// Constructors
	CAnimationImageCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CAnimationImageCtrlT< TBase >& operator=(HWND hWnd)
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
		return WC_LSC_ANIIMAGE;
	}

	void Start()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, AIM_START, 0, 0);
	}

	void Stop()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, AIM_STOP, 0, 0);
	}

	void SetAnimationImage(HIMAGELIST him, int interval)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, AIM_SETANIMATION, (WPARAM)interval, (LPARAM)him);
	}

	void SetBkColor(COLORREF clrBkgnd)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, AIM_SETBKCOLOR, 0, (LPARAM)clrBkgnd);
	}
};

typedef CAnimationImageCtrlT<CWindow>	CAnimationImageCtrl;


#endif			// __lcc_include_animationimagectrl_h__

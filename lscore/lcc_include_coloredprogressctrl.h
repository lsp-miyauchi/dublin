/////////////////////////////////////////////////////////////////////////////
// CColoredProgressCtrl

#pragma once
#ifndef __lcc_include_coloredprogressctrl_h__
#define __lcc_include_coloredprogressctrl_h__


template <class TBase>
class CColoredProgressCtrlT : public TBase
{
public:
// Constructors
	CColoredProgressCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CColoredProgressCtrlT< TBase >& operator=(HWND hWnd)
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
		return WC_LSC_PROGRESS;
	}

	LRESULT SetRange(int nMin, int nMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SendMessage(m_hWnd, PBM_SETRANGE, 0, MAKELPARAM(nMin, nMax));
	}

	LRESULT SetRange32(int nMin, int nMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SendMessage(m_hWnd, PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);
	}

	int GetRange(BOOL fWhichLimit, PBRANGE* ppbr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_GETRANGE, (WPARAM)fWhichLimit, (LPARAM)ppbr);
	}

	int SetPos(int pos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_SETPOS, (WPARAM)pos, 0);
	}

	int GetPos()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_GETPOS, 0, 0);
	}

	int SetStep(int step)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_SETSTEP, (WPARAM)step, 0);
	}

	int DeltaPos(int delta)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_DELTAPOS, (WPARAM)delta, 0);
	}

	int StepIt()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_STEPIT, 0, 0);
	}

	COLORREF SetBarColor(COLORREF crFrom, COLORREF crTo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, PBM_SETBARCOLOR, (WPARAM)crTo, (LPARAM)crFrom);
	}
};

typedef CColoredProgressCtrlT<CWindow>	CColoredProgressCtrl;


#endif		// __lcc_include_coloredprogressctrl_h__

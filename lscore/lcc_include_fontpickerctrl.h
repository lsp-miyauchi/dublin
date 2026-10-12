/////////////////////////////////////////////////////////////////////////////
// CFontPickerCtrl

#pragma once
#ifndef __lcc_include_fontpickerctrl_h__
#define __lcc_include_fontpickerctrl_h__


template <class TBase>
class CFontPickerCtrlT : public TBase
{
public:
// Constructors
	CFontPickerCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CFontPickerCtrlT< TBase >& operator=(HWND hWnd)
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
		return WC_LSC_FONTPICK;
	}

	void SetLogFont(const LOGFONT* pLogFont)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FPM_SETLOGFONT, 0, (LPARAM)pLogFont);
	}
	void GetLogFont(LOGFONT* pLogFont)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FPM_GETLOGFONT, 0, (LPARAM)pLogFont);
	}
};

typedef CFontPickerCtrlT<CWindow>	CFontPickerCtrl;


#endif		// __lcc_include_fontpickerctrl_h__

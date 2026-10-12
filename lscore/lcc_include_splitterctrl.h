/////////////////////////////////////////////////////////////////////////////
// CSplitterCtrl

#pragma once
#ifndef __lcc_include_splitterctrl_h__
#define __lcc_include_splitterctrl_h__


template <class TBase>
class CSplitterCtrlT : public TBase
{
public:
// Constructors
	CSplitterCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CSplitterCtrlT< TBase >& operator=(HWND hWnd)
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
		return WC_LSC_SPLITTER;
	}

	void SetSplitterRect(LPRECT lpRect = NULL, bool bUpdate = true)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SPM_SETSPLITTERRECT, (WPARAM)bUpdate, (LPARAM)lpRect);
	}
	void GetSplitterRect(LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SPM_GETSPLITTERRECT, 0, (LPARAM)lpRect);
	}
	bool SetSplitterPos(int xyPos = -1, bool bUpdate = true)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (::SendMessage(m_hWnd, SPM_SETSPLITTERPOS, (WPARAM)bUpdate, (LPARAM)xyPos) != 0);
	}
	int GetSplitterPos()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SPM_GETSPLITTERPOS, 0, 0);
	}
	bool SetSinglePaneMode(int nPane = SPLIT_PANE_NONE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (::SendMessage(m_hWnd, SPM_SETSINGLEPANEMODE, (WPARAM)nPane, 0) != 0);
	}
	int GetSinglePaneMode()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SPM_GETSINGLEPANEMODE, 0, 0);
	}
	DWORD GetSplitterExtendedStyle()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, SPM_GETEXTENDEDSTYLE, 0, 0);
	}
	DWORD SetSplitterExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, SPM_SETEXTENDEDSTYLE, (WPARAM)dwMask, (LPARAM)dwExtendedStyle);
	}
	void SetSplitterPanes(HWND hWndLeftTop, HWND hWndRightBottom, bool bUpdate = true)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SPLITTERPANEINFO info;
		info.hWndLeftTop = hWndLeftTop;
		info.hWndRightBottom = hWndRightBottom;
		::SendMessage(m_hWnd, SPM_SETPANES, (WPARAM)bUpdate, (LPARAM)&info);
	}
	bool SetSplitterPane(int nPane, HWND hWnd, bool bUpdate = true)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (::SendMessage(m_hWnd, SPM_SETSPLITTERPANE, MAKELONG(nPane, bUpdate), (LPARAM)hWnd) != 0);
	}
	HWND GetSplitterPane(int nPane)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, SPM_GETSPLITTERPANE, (WPARAM)nPane, 0);
	}
	bool SetActivePane(int nPane)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (::SendMessage(m_hWnd, SPM_SETACTIVEPANE, (WPARAM)nPane, 0) != 0);
	}
	int GetActivePane()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SPM_GETACTIVEPANE, 0, 0);
	}
	bool ActivateNextPane(bool bNext = true)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (::SendMessage(m_hWnd, SPM_ACTIVATENEXTPANE, (WPARAM)bNext, 0) != 0);
	}
	bool SetDefaultActivePane(int nPane)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (::SendMessage(m_hWnd, SPM_SETDEFAULTACTIVEPANE, 1, (LPARAM)nPane) != 0);
	}
	bool SetDefaultActivePane(HWND hWnd)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (::SendMessage(m_hWnd, SPM_SETDEFAULTACTIVEPANE, 0, (LPARAM)hWnd) != 0);
	}
	int GetDefaultActivePane()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SPM_GETDEFAULTACTIVEPANE, 0, 0);
	}
	void SetSplitterMinSize(int cxyMin)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SPM_SETSPLITTERMINSIZE, (WPARAM)cxyMin, 0);
	}
	int GetSplitterMinSize()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		(int)::SendMessage(m_hWnd, SPM_GETSPLITTERMINSIZE, 0, 0);
	}
	void SetSplitterDragMode(bool bFullDrag)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SPM_SETSPLITTERDRAGMODE, (WPARAM)bFullDrag, 0);
	}
	bool GetSplitterDragMode()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (::SendMessage(m_hWnd, SPM_GETSPLITTERDRAGMODE, 0, 0) != 0);
	}
};

typedef CSplitterCtrlT<CWindow>	CSplitterCtrl;


#endif			// __lcc_include_splitterctrl_h__

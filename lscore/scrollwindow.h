// Windows Template Library - WTL version 8.0
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// This file is a part of the Windows Template Library.
// The use and distribution terms for this software are covered by the
// Common Public License 1.0 (http://opensource.org/osi3.0/licenses/cpl1.0.php)
// which can be found in the file CPL.TXT at the root of this distribution.
// By using this software in any fashion, you are agreeing to be bound by
// the terms of this license. You must not remove this notice, or
// any other, from this software.

#ifndef __ATLSCRL_H__
#define __ATLSCRL_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlscrl.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error atlscrl.h requires atlwin.h to be included first
#endif

#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
  #include <zmouse.h>
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)

#ifndef GET_WHEEL_DELTA_WPARAM
  #define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#endif

#ifndef WM_MOUSEHWHEEL
  #define WM_MOUSEHWHEEL                  0x020E
#endif


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CScrollImpl<T>
// CScrollWindowImpl<T, TBase, TWinTraits>

namespace WTL
{

///////////////////////////////////////////////////////////////////////////////
// CScrollImpl - Provides scrolling support to any window

// Scroll extended styles
#define SCRL_SCROLLCHILDREN	0x00000001
#define SCRL_ERASEBACKGROUND	0x00000002
#define SCRL_NOTHUMBTRACKING	0x00000004
#if (WINVER >= 0x0500)
#define SCRL_SMOOTHSCROLL	0x00000008
#endif // (WINVER >= 0x0500)
#define SCRL_DISABLENOSCROLLV	0x00000010
#define SCRL_DISABLENOSCROLLH	0x00000020
#define SCRL_DISABLENOSCROLL	(SCRL_DISABLENOSCROLLV | SCRL_DISABLENOSCROLLH)


template <class T>
class CScrollImpl
{
public:
	enum { uSCROLL_FLAGS = SW_INVALIDATE };

	POINT m_ptOffset;
	SIZE m_sizeAll;
	SIZE m_sizeLine;
	SIZE m_sizePage;
	SIZE m_sizeClient;
	int m_zDelta;              // current wheel value
	int m_nWheelLines;         // number of lines to scroll on wheel
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
	// Note that this message must be forwarded from a top level window
	UINT m_uMsgMouseWheel;     // MSH_MOUSEWHEEL
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
	int m_zHDelta;              // current horizontal wheel value
	int m_nHWheelChars;         // number of chars to scroll on horizontal wheel
	UINT m_uScrollFlags;
	DWORD m_dwExtendedStyle;   // scroll specific extended styles

// Constructor
	CScrollImpl() : m_zDelta(0), m_nWheelLines(3), 
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
			m_uMsgMouseWheel(0U), 
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
			m_zHDelta(0), m_nHWheelChars(3), 
			m_uScrollFlags(0U), m_dwExtendedStyle(0)
	{
		m_ptOffset.x = 0;
		m_ptOffset.y = 0;
		m_sizeAll.cx = 0;
		m_sizeAll.cy = 0;
		m_sizePage.cx = 0;
		m_sizePage.cy = 0;
		m_sizeLine.cx = 0;
		m_sizeLine.cy = 0;
		m_sizeClient.cx = 0;
		m_sizeClient.cy = 0;

		SetScrollExtendedStyle(SCRL_SCROLLCHILDREN | SCRL_ERASEBACKGROUND);
	}

// Attributes & Operations
	DWORD GetScrollExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetScrollExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if(dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
		// cache scroll flags
		T* pT = static_cast<T*>(this);
		pT;   // avoid level 4 warning
		m_uScrollFlags = pT->uSCROLL_FLAGS | (IsScrollingChildren() ? SW_SCROLLCHILDREN : 0) | (IsErasingBackground() ? SW_ERASE : 0);
#if (WINVER >= 0x0500) && !defined(_WIN32_WCE)
		m_uScrollFlags |= (IsSmoothScroll() ? SW_SMOOTHSCROLL : 0);
#endif // (WINVER >= 0x0500) && !defined(_WIN32_WCE)
		return dwPrevStyle;
	}

	// offset operations
	void SetScrollOffset(int x, int y, BOOL bRedraw = TRUE)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		pT->AdjustScrollOffset(x, y);

		int dx = m_ptOffset.x - x;
		int dy = m_ptOffset.y - y;
		m_ptOffset.x = x;
		m_ptOffset.y = y;

		// block: set horizontal scroll bar
		{
			SCROLLINFO si = { sizeof(SCROLLINFO) };
			si.fMask = SIF_POS;
			if((m_dwExtendedStyle & SCRL_DISABLENOSCROLLH) != 0)
				si.fMask |= SIF_DISABLENOSCROLL;
			si.nPos = m_ptOffset.x;
			pT->SetScrollInfo(SB_HORZ, &si, bRedraw);
		}

		// block: set vertical scroll bar
		{
			SCROLLINFO si = { sizeof(SCROLLINFO) };
			si.fMask = SIF_POS;
			if((m_dwExtendedStyle & SCRL_DISABLENOSCROLLV) != 0)
				si.fMask |= SIF_DISABLENOSCROLL;
			si.nPos = m_ptOffset.y;
			pT->SetScrollInfo(SB_VERT, &si, bRedraw);
		}

		// Move all children if needed
		if(IsScrollingChildren() && (dx != 0 || dy != 0))
		{
			for(HWND hWndChild = ::GetWindow(pT->m_hWnd, GW_CHILD); hWndChild != NULL; hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT))
			{
				RECT rect = { 0 };
				::GetWindowRect(hWndChild, &rect);
				::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rect, 1);
				::SetWindowPos(hWndChild, NULL, rect.left + dx, rect.top + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}

		if(bRedraw)
			pT->Invalidate();
	}

	void SetScrollOffset(POINT ptOffset, BOOL bRedraw = TRUE)
	{
		SetScrollOffset(ptOffset.x, ptOffset.y, bRedraw);
	}

	void GetScrollOffset(POINT& ptOffset) const
	{
		ptOffset = m_ptOffset;
	}

	// size operations
	void SetScrollSize(int cx, int cy, BOOL bRedraw = TRUE, bool bResetOffset = true)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		m_sizeAll.cx = cx;
		m_sizeAll.cy = cy;

		int x = 0;
		int y = 0;
		if(!bResetOffset)
		{
			x = m_ptOffset.x;
			y = m_ptOffset.y;
			pT->AdjustScrollOffset(x, y);
		}

		int dx = m_ptOffset.x - x;
		int dy = m_ptOffset.y - y;
		m_ptOffset.x = x;
		m_ptOffset.y = y;

// ----- Add K.Miyauchi -----------
CSize sizeWorkArea = m_sizeClient;
DWORD style = pT->GetStyle();
if (style & WS_VSCROLL) sizeWorkArea.cx += GetSystemMetrics(SM_CXVSCROLL);
if (style & WS_HSCROLL) sizeWorkArea.cy += GetSystemMetrics(SM_CYHSCROLL);
// <------ add end
		// block: set horizontal scroll bar
		{
			SCROLLINFO si = { sizeof(SCROLLINFO) };
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			if((m_dwExtendedStyle & SCRL_DISABLENOSCROLLH) != 0)
				si.fMask |= SIF_DISABLENOSCROLL;
			si.nMin = 0;
			si.nMax = m_sizeAll.cx - 1;
// ----- replace K.Miyauchi -----------
//@			si.nPage = m_sizeClient.cx;
			si.nPage = sizeWorkArea.cx;
// <----
			si.nPos = m_ptOffset.x;
			pT->SetScrollInfo(SB_HORZ, &si, bRedraw);
		}

		// block: set vertical scroll bar
		{
			SCROLLINFO si = { sizeof(SCROLLINFO) };
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			if((m_dwExtendedStyle & SCRL_DISABLENOSCROLLV) != 0)
				si.fMask |= SIF_DISABLENOSCROLL;
			si.nMin = 0;
			si.nMax = m_sizeAll.cy - 1;
// ----- replace K.Miyauchi -----------
//@			si.nPage = m_sizeClient.cy;
			si.nPage = sizeWorkArea.cy;
// <----
			si.nPos = m_ptOffset.y;
			pT->SetScrollInfo(SB_VERT, &si, bRedraw);
		}

		// Move all children if needed
		if(IsScrollingChildren() && (dx != 0 || dy != 0))
		{
			for(HWND hWndChild = ::GetWindow(pT->m_hWnd, GW_CHILD); hWndChild != NULL; hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT))
			{
				RECT rect = { 0 };
				::GetWindowRect(hWndChild, &rect);
				::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rect, 1);
				::SetWindowPos(hWndChild, NULL, rect.left + dx, rect.top + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}

		SetScrollLine(0, 0);
		SetScrollPage(0, 0);

		if(bRedraw)
			pT->Invalidate();
	}

	void SetScrollSize(SIZE size, BOOL bRedraw = TRUE, bool bResetOffset = true)
	{
		SetScrollSize(size.cx, size.cy, bRedraw, bResetOffset);
	}

	void GetScrollSize(SIZE& sizeWnd) const
	{
		sizeWnd = m_sizeAll;
	}

	// line operations
	void SetScrollLine(int cxLine, int cyLine)
	{
		ATLASSERT(cxLine >= 0 && cyLine >= 0);
		ATLASSERT(m_sizeAll.cx != 0 && m_sizeAll.cy != 0);

		m_sizeLine.cx = T::CalcLineOrPage(cxLine, m_sizeAll.cx, 100);
		m_sizeLine.cy = T::CalcLineOrPage(cyLine, m_sizeAll.cy, 100);
	}

	void SetScrollLine(SIZE sizeLine)
	{
		SetScrollLine(sizeLine.cx, sizeLine.cy);
	}

	void GetScrollLine(SIZE& sizeLine) const
	{
		sizeLine = m_sizeLine;
	}

	// page operations
	void SetScrollPage(int cxPage, int cyPage)
	{
		ATLASSERT(cxPage >= 0 && cyPage >= 0);
		ATLASSERT(m_sizeAll.cx != 0 && m_sizeAll.cy != 0);

		m_sizePage.cx = T::CalcLineOrPage(cxPage, m_sizeAll.cx, 10);
		m_sizePage.cy = T::CalcLineOrPage(cyPage, m_sizeAll.cy, 10);
	}

	void SetScrollPage(SIZE sizePage)
	{
		SetScrollPage(sizePage.cx, sizePage.cy);
	}

	void GetScrollPage(SIZE& sizePage) const
	{
		sizePage = m_sizePage;
	}

	// commands
	void ScrollLineDown()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_LINEDOWN, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollLineUp()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_LINEUP, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollPageDown()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_PAGEDOWN, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollPageUp()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_PAGEUP, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollTop()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_TOP, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollBottom()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_BOTTOM, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollLineRight()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_LINEDOWN, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollLineLeft()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_LINEUP, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollPageRight()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_PAGEDOWN, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollPageLeft()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_PAGEUP, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollAllLeft()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_TOP, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollAllRight()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_BOTTOM, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	// scroll to make point/view/window visible
	void ScrollToView(POINT pt)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		RECT rect = { pt.x, pt.y, pt.x, pt.y };
		pT->ScrollToView(rect);
	}

	void ScrollToView(RECT& rect)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		RECT rcClient = { 0 };
		pT->GetClientRect(&rcClient);

		int x = m_ptOffset.x;
		if(rect.left < m_ptOffset.x)
			x = rect.left;
		else if(rect.right > (m_ptOffset.x + rcClient.right))
			x = rect.right - rcClient.right;

		int y = m_ptOffset.y;
		if(rect.top < m_ptOffset.y)
			y = rect.top;
		else if(rect.bottom > (m_ptOffset.y + rcClient.bottom))
			y = rect.bottom - rcClient.bottom;

		SetScrollOffset(x, y);
	}

	void ScrollToView(HWND hWnd)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		RECT rect = { 0 };
		::GetWindowRect(hWnd, &rect);
		::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rect, 2);
		ScrollToView(rect);
	}

	BEGIN_MSG_MAP(CScrollImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
		MESSAGE_HANDLER(m_uMsgMouseWheel, OnMouseWheel)
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
		MESSAGE_HANDLER(WM_MOUSEHWHEEL, OnMouseHWheel)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
#endif // !_WIN32_WCE
	// standard scroll commands
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_SCROLL_UP, OnScrollUp)
		COMMAND_ID_HANDLER(ID_SCROLL_DOWN, OnScrollDown)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_UP, OnScrollPageUp)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_DOWN, OnScrollPageDown)
		COMMAND_ID_HANDLER(ID_SCROLL_TOP, OnScrollTop)
		COMMAND_ID_HANDLER(ID_SCROLL_BOTTOM, OnScrollBottom)
		COMMAND_ID_HANDLER(ID_SCROLL_LEFT, OnScrollLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_RIGHT, OnScrollRight)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_LEFT, OnScrollPageLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_RIGHT, OnScrollPageRight)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_LEFT, OnScrollAllLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_RIGHT, OnScrollAllRight)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		GetSystemSettings();
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, (int)(short)LOWORD(wParam), (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
		return 0;
	}

	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, (int)(short)LOWORD(wParam), (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
		return 0;
	}

// ----- K.Miyauchi ----------->
//	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
// <---------------
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		// if shift key pressed, then scroll horizontally
		if (GetKeyState(VK_SHIFT) < 0) {
			return OnMouseHWheel(uMsg, wParam, lParam, bHandled);
		}

// ----- K.Miyauchi ----------->
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		CRect rc;
		pT->GetWindowRect(&rc);
		if (!rc.PtInRect(pt)) {
			bHandled = FALSE;
			return 0;
		}
// <----- end of K.Miyauchi

#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400) || defined(_WIN32_WCE)
		uMsg;
		int zDelta = (int)GET_WHEEL_DELTA_WPARAM(wParam);
#else
		int zDelta = (uMsg == WM_MOUSEWHEEL) ? (int)GET_WHEEL_DELTA_WPARAM(wParam) : (int)wParam;
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400) || defined(_WIN32_WCE))
		int nScrollCode = (m_nWheelLines == WHEEL_PAGESCROLL) ? ((zDelta > 0) ? SB_PAGEUP : SB_PAGEDOWN) : ((zDelta > 0) ? SB_LINEUP : SB_LINEDOWN);
		m_zDelta += zDelta;   // cumulative
		int zTotal = (m_nWheelLines == WHEEL_PAGESCROLL) ? abs(m_zDelta) : abs(m_zDelta) * m_nWheelLines;
// ----- supress K.Miyauchi ----------->
#if 0
		if(m_sizeAll.cy > m_sizeClient.cy)
		{
			for(int i = 0; i < zTotal; i += WHEEL_DELTA)
			{
				pT->DoScroll(SB_VERT, nScrollCode, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
				pT->UpdateWindow();
			}
		}
		else		// can't scroll vertically, scroll horizontally
		{
			for(int i = 0; i < zTotal; i += WHEEL_DELTA)
			{
				pT->DoScroll(SB_HORZ, nScrollCode, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
				pT->UpdateWindow();
			}
		}
#else
		int scrLine = 0, scrPage = 0;
		if (m_sizeAll.cy > m_sizeClient.cy) {
			if (nScrollCode == SB_PAGEUP || nScrollCode == SB_PAGEDOWN) {
				scrPage = m_sizePage.cy * (zTotal / WHEEL_DELTA);
				scrLine = m_sizeLine.cy;
			}
			else {
				scrPage = m_sizePage.cy;
				scrLine = m_sizeLine.cy * (zTotal / WHEEL_DELTA);
			}
			pT->DoScroll(SB_VERT, nScrollCode, (int&)m_ptOffset.y, m_sizeAll.cy, scrPage, scrLine);
			pT->UpdateWindow();
		}
		else {		// can't scroll vertically, scroll horizontally
			if (nScrollCode == SB_PAGEUP || nScrollCode == SB_PAGEDOWN) {
				scrPage = m_sizePage.cx * (zTotal / WHEEL_DELTA);
				scrLine = m_sizeLine.cx;
			}
			else {
				scrPage = m_sizePage.cx;
				scrLine = m_sizeLine.cx * (zTotal / WHEEL_DELTA);
			}
			pT->DoScroll(SB_HORZ, nScrollCode, (int&)m_ptOffset.x, m_sizeAll.cx, scrPage, scrLine);
			pT->UpdateWindow();
		}
#endif
		m_zDelta %= WHEEL_DELTA;
// <----- end of K.Miyauchi

		return 0;
	}

// ----- K.Miyauchi ----------->
//	LRESULT OnMouseHWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	LRESULT OnMouseHWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

// ----- K.Miyauchi ----------->
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		CRect rc;
		pT->GetWindowRect(&rc);
		if (!rc.PtInRect(pt)) {
			bHandled = FALSE;
			return 0;
		}
// <----- end of K.Miyauchi

		int zDelta = (int)GET_WHEEL_DELTA_WPARAM(wParam);
		int nScrollCode = (m_nHWheelChars == WHEEL_PAGESCROLL) ? ((zDelta > 0) ? SB_PAGERIGHT : SB_PAGELEFT) : ((zDelta > 0) ? SB_LINERIGHT : SB_LINELEFT);
		m_zHDelta += zDelta;   // cumulative
		int zTotal = (m_nHWheelChars == WHEEL_PAGESCROLL) ? abs(m_zHDelta) : abs(m_zHDelta) * m_nHWheelChars;
// ----- supress K.Miyauchi ----------->
#if 0
		if(m_sizeAll.cx > m_sizeClient.cx)
		{
			for(int i = 0; i < zTotal; i += WHEEL_DELTA)
			{
				pT->DoScroll(SB_HORZ, nScrollCode, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
				pT->UpdateWindow();
			}
		}
#else
		int scrLine = 0, scrPage = 0;
		if (m_sizeAll.cx > m_sizeClient.cx) {
			if (nScrollCode == SB_PAGEUP || nScrollCode == SB_PAGEDOWN) {
				scrPage = m_sizePage.cx * (zTotal / WHEEL_DELTA);
				scrLine = m_sizeLine.cx;
			}
			else {
				scrPage = m_sizePage.cx;
				scrLine = m_sizeLine.cx * (zTotal / WHEEL_DELTA);
			}
			pT->DoScroll(SB_HORZ, nScrollCode, (int&)m_ptOffset.x, m_sizeAll.cx, scrPage, scrLine);
			pT->UpdateWindow();
		}
#endif
// <----- end of K.Miyauchi
		m_zHDelta %= WHEEL_DELTA;

		return 0;
	}

	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		GetSystemSettings();
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		m_sizeClient.cx = GET_X_LPARAM(lParam);
		m_sizeClient.cy = GET_Y_LPARAM(lParam);

		// block: set horizontal scroll bar
		{
			SCROLLINFO si = { sizeof(SCROLLINFO) };
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			si.nMin = 0;
			si.nMax = m_sizeAll.cx - 1;
			if((m_dwExtendedStyle & SCRL_DISABLENOSCROLLH) != 0)
				si.fMask |= SIF_DISABLENOSCROLL;
			si.nPage = m_sizeClient.cx;
			si.nPos = m_ptOffset.x;
			pT->SetScrollInfo(SB_HORZ, &si, TRUE);
		}

		// block: set vertical scroll bar
		{
			SCROLLINFO si = { sizeof(SCROLLINFO) };
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			si.nMin = 0;
			si.nMax = m_sizeAll.cy - 1;
			if((m_dwExtendedStyle & SCRL_DISABLENOSCROLLV) != 0)
				si.fMask |= SIF_DISABLENOSCROLL;
			si.nPage = m_sizeClient.cy;
			si.nPos = m_ptOffset.y;
			pT->SetScrollInfo(SB_VERT, &si, TRUE);
		}

		int x = m_ptOffset.x;
		int y = m_ptOffset.y;
		if(pT->AdjustScrollOffset(x, y))
		{
			// Children will be moved in SetScrollOffset, if needed
			pT->ScrollWindowEx(m_ptOffset.x - x, m_ptOffset.y - y, (m_uScrollFlags & ~SCRL_SCROLLCHILDREN));
			SetScrollOffset(x, y, FALSE);
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		if(wParam != NULL)
		{
			CDCHandle dc = (HDC)wParam;
			dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);
			pT->DoPaint(dc);
		}
		else
		{
			CPaintDC dc(pT->m_hWnd);
			dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);
			pT->DoPaint(dc.m_hDC);
		}
		return 0;
	}

// --> no use K.Miyauchi
#if 0
	// scrolling handlers
	LRESULT OnScrollUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollLineUp();
		return 0;
	}

	LRESULT OnScrollDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollLineDown();
		return 0;
	}

	LRESULT OnScrollPageUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollPageUp();
		return 0;
	}

	LRESULT OnScrollPageDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollPageDown();
		return 0;
	}

	LRESULT OnScrollTop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollTop();
		return 0;
	}

	LRESULT OnScrollBottom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollBottom();
		return 0;
	}

	LRESULT OnScrollLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollLineLeft();
		return 0;
	}

	LRESULT OnScrollRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollLineRight();
		return 0;
	}

	LRESULT OnScrollPageLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollPageLeft();
		return 0;
	}

	LRESULT OnScrollPageRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollPageRight();
		return 0;
	}

	LRESULT OnScrollAllLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollAllLeft();
		return 0;
	}

	LRESULT OnScrollAllRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ScrollAllRight();
		return 0;
	}
#endif		// 0
// <-- no use

// Overrideables
	void DoPaint(CDCHandle /*dc*/)
	{
		// must be implemented in a derived class
		ATLASSERT(FALSE);
	}

// Implementation
	void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
	{
		T* pT = static_cast<T*>(this);
		RECT rect = { 0 };
		pT->GetClientRect(&rect);
		int cxyClient = (nType == SB_VERT) ? rect.bottom : rect.right;
		int cxyMax = cxySizeAll - cxyClient;

		if(cxyMax < 0)   // can't scroll, client area is bigger
			return;

		bool bUpdate = true;
		int cxyScroll = 0;

		switch(nScrollCode)
		{
		case SB_TOP:		// top or all left
			cxyScroll = cxyOffset;
			cxyOffset = 0;
			break;
		case SB_BOTTOM:		// bottom or all right
			cxyScroll = cxyOffset - cxyMax;
			cxyOffset = cxyMax;
			break;
		case SB_LINEUP:		// line up or line left
			if(cxyOffset >= cxySizeLine)
			{
				cxyScroll = cxySizeLine;
				cxyOffset -= cxySizeLine;
			}
			else
			{
				cxyScroll = cxyOffset;
				cxyOffset = 0;
			}
			break;
		case SB_LINEDOWN:	// line down or line right
			if(cxyOffset < cxyMax - cxySizeLine)
			{
				cxyScroll = -cxySizeLine;
				cxyOffset += cxySizeLine;
			}
			else
			{
				cxyScroll = cxyOffset - cxyMax;
				cxyOffset = cxyMax;
			}
			break;
		case SB_PAGEUP:		// page up or page left
			if(cxyOffset >= cxySizePage)
			{
				cxyScroll = cxySizePage;
				cxyOffset -= cxySizePage;
			}
			else
			{
				cxyScroll = cxyOffset;
				cxyOffset = 0;
			}
			break;
		case SB_PAGEDOWN:	// page down or page right
			if(cxyOffset < cxyMax - cxySizePage)
			{
				cxyScroll = -cxySizePage;
				cxyOffset += cxySizePage;
			}
			else
			{
				cxyScroll = cxyOffset - cxyMax;
				cxyOffset = cxyMax;
			}
			break;
		case SB_THUMBTRACK:
			if(IsNoThumbTracking())
				break;
			// else fall through
		case SB_THUMBPOSITION:
			{
				SCROLLINFO si = { sizeof(SCROLLINFO), SIF_TRACKPOS };
				if(pT->GetScrollInfo(nType, &si))
				{
					cxyScroll = cxyOffset - si.nTrackPos;
					cxyOffset = si.nTrackPos;
				}
			}
			break;
		case SB_ENDSCROLL:
		default:
			bUpdate = false;
			break;
		}

		if(bUpdate && cxyScroll != 0)
		{
			pT->SetScrollPos(nType, cxyOffset, TRUE);
			if(nType == SB_VERT)
				pT->ScrollWindowEx(0, cxyScroll, m_uScrollFlags);
			else
				pT->ScrollWindowEx(cxyScroll, 0, m_uScrollFlags);
		}
	}

	static int CalcLineOrPage(int nVal, int nMax, int nDiv)
	{
		if(nVal == 0)
		{
			nVal = nMax / nDiv;
			if(nVal < 1)
				nVal = 1;
		}
		else if(nVal > nMax)
		{
			nVal = nMax;
		}

		return nVal;
	}

	bool AdjustScrollOffset(int& x, int& y)
	{
		int xOld = x;
		int yOld = y;

		int cxMax = m_sizeAll.cx - m_sizeClient.cx;
		if(x > cxMax)
			x = (cxMax >= 0) ? cxMax : 0;
		else if(x < 0)
			x = 0;

		int cyMax = m_sizeAll.cy - m_sizeClient.cy;
		if(y > cyMax)
			y = (cyMax >= 0) ? cyMax : 0;
		else if(y < 0)
			y = 0;

		return (x != xOld || y != yOld);
	}

	void GetSystemSettings()
	{
#ifndef _WIN32_WCE
#ifndef SPI_GETWHEELSCROLLLINES
		const UINT SPI_GETWHEELSCROLLLINES = 104;
#endif // !SPI_GETWHEELSCROLLLINES
		::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &m_nWheelLines, 0);

#ifndef SPI_GETWHEELSCROLLCHARS
		const UINT SPI_GETWHEELSCROLLCHARS = 0x006C;
#endif // !SPI_GETWHEELSCROLLCHARS
		::SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &m_nHWheelChars, 0);

#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
		if(m_uMsgMouseWheel != 0)
			m_uMsgMouseWheel = ::RegisterWindowMessage(MSH_MOUSEWHEEL);

		HWND hWndWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
		if(::IsWindow(hWndWheel))
		{
			UINT uMsgScrollLines = ::RegisterWindowMessage(MSH_SCROLL_LINES);
			if(uMsgScrollLines != 0)
				m_nWheelLines = (int)::SendMessage(hWndWheel, uMsgScrollLines, 0, 0L);
		}
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
#endif // !_WIN32_WCE
	}

	bool IsScrollingChildren() const
	{
		return (m_dwExtendedStyle & SCRL_SCROLLCHILDREN) != 0;
	}

	bool IsErasingBackground() const
	{
		return (m_dwExtendedStyle & SCRL_ERASEBACKGROUND) != 0;
	}

	bool IsNoThumbTracking() const
	{
		return (m_dwExtendedStyle & SCRL_NOTHUMBTRACKING) != 0;
	}

#if (WINVER >= 0x0500)
	bool IsSmoothScroll() const
	{
		return (m_dwExtendedStyle & SCRL_SMOOTHSCROLL) != 0;
	}
#endif // (WINVER >= 0x0500)
};


///////////////////////////////////////////////////////////////////////////////
// CScrollWindowImpl - Implements a scrollable window

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CScrollWindowImpl : public ATL::CWindowImpl<T, TBase, TWinTraits>, public CScrollImpl< T >
{
public:
	BEGIN_MSG_MAP(CScrollWindowImpl)
		MESSAGE_HANDLER(WM_VSCROLL, CScrollImpl< T >::OnVScroll)
		MESSAGE_HANDLER(WM_HSCROLL, CScrollImpl< T >::OnHScroll)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, CScrollImpl< T >::OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
		MESSAGE_HANDLER(m_uMsgMouseWheel, CScrollImpl< T >::OnMouseWheel)
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)) && !defined(_WIN32_WCE)
		MESSAGE_HANDLER(WM_MOUSEHWHEEL, CScrollImpl< T >::OnMouseHWheel)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, CScrollImpl< T >::OnSettingChange)
		MESSAGE_HANDLER(WM_SIZE, CScrollImpl< T >::OnSize)
		MESSAGE_HANDLER(WM_PAINT, CScrollImpl< T >::OnPaint)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_PRINTCLIENT, CScrollImpl< T >::OnPaint)
#endif // !_WIN32_WCE
// --> no use K.Miyauchi
#if 0
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_SCROLL_UP, CScrollImpl< T >::OnScrollUp)
		COMMAND_ID_HANDLER(ID_SCROLL_DOWN, CScrollImpl< T >::OnScrollDown)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_UP, CScrollImpl< T >::OnScrollPageUp)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_DOWN, CScrollImpl< T >::OnScrollPageDown)
		COMMAND_ID_HANDLER(ID_SCROLL_TOP, CScrollImpl< T >::OnScrollTop)
		COMMAND_ID_HANDLER(ID_SCROLL_BOTTOM, CScrollImpl< T >::OnScrollBottom)
		COMMAND_ID_HANDLER(ID_SCROLL_LEFT, CScrollImpl< T >::OnScrollLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_RIGHT, CScrollImpl< T >::OnScrollRight)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_LEFT, CScrollImpl< T >::OnScrollPageLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_RIGHT, CScrollImpl< T >::OnScrollPageRight)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_LEFT, CScrollImpl< T >::OnScrollAllLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_RIGHT, CScrollImpl< T >::OnScrollAllRight)
#endif
// <-- no use
	END_MSG_MAP()
};


}; // namespace WTL

#endif // __ATLSCRL_H__

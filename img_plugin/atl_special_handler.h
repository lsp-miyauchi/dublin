//////////////////////////////////////////////////////////////////////
// ATL_SPECIAL_HANDLER 

#pragma once
#ifndef __ATL_SPECIAL_HANDLER_H__
#define __ATL_SPECIAL_HANDLER_H__



/* ----------
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnClose();
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void OnActivate(UINT nState, HWND hWndOther, BOOL bMinimized);
	void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	LRESULT OnActivateTopLevel(WPARAM, LPARAM);
	void OnCancelMode();
	void OnChildActivate();
	void OnContextMenu(HWND hWnd, int xScreenPos, int yScreenPos);
	BOOL OnCopyData(HWND hWnd, COPYDATASTRUCT* pCopyDataStruct);

	HBRUSH OnCtlColor(HDC hDC, HWND hWnd, UINT nCtlColor);

	void OnDestroy();
	void OnEnable(BOOL bEnable);
	void OnEndSession(BOOL bEnding);
	void OnEnterIdle(UINT nWhy, HWND hWho);
	BOOL OnEraseBkgnd(HDC hdc);
	void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	BOOL OnHelpInfo(HELPINFO* lpHelpInfo);
	void OnIconEraseBkgnd(HDC hdc);
	void OnKillFocus(HWND hNewWnd);
	LRESULT OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu);
	void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	void OnMove(int x, int y);
	void OnPaint();
	void OnPrintClient(HDC hdc, DWORD flags);
	void OnParentNotify(UINT message, LPARAM lParam);
	HCURSOR OnQueryDragIcon();
	BOOL OnQueryEndSession();
	BOOL OnQueryNewPalette();
	BOOL OnQueryOpen();
	void OnSetFocus(HWND hOldWnd);
	void OnShowWindow(BOOL bShow, UINT nStatus);
	void OnSize(UINT nType, int cx, int cy);
	void OnTCard(UINT idAction, DWORD dwActionData);
	void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	void OnWindowPosChanged(WINDOWPOS* lpwndpos);

	void OnChangeUIState(UINT nAction, UINT nUIElement);
	void OnUpdateUIState(UINT nAction, UINT nUIElement);
	UINT OnQueryUIState();

// Nonclient-Area message handler member functions
	BOOL OnNcActivate(BOOL bActive);
	void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	void OnNcDestroy();
	UINT OnNcHitTest(int xPos, int yPos);
	void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	void OnNcLButtonDown(UINT nHitTest, CPoint point);
	void OnNcLButtonUp(UINT nHitTest, CPoint point);
	void OnNcMButtonDblClk(UINT nHitTest, CPoint point);
	void OnNcMButtonDown(UINT nHitTest, CPoint point);
	void OnNcMButtonUp(UINT nHitTest, CPoint point);
	void OnNcMouseMove(UINT nHitTest, CPoint point);
	void OnNcPaint();
	void OnNcRButtonDblClk(UINT nHitTest, CPoint point);
	void OnNcRButtonDown(UINT nHitTest, CPoint point);
	void OnNcRButtonUp(UINT nHitTest, CPoint point);

// System message handler member functions
	void OnDropFiles(HDROP hDropInfo);
	void OnPaletteIsChanging(HWND hRealizeWnd);
	void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSysCommand(UINT nID, LPARAM lParam);
	void OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnCompacting(UINT nCpuTime);
	void OnDevModeChange(LPTSTR lpDeviceName);
	void OnFontChange();
	void OnPaletteChanged(HWND hFocusWnd);
	void OnSpoolerStatus(UINT nStatus, UINT nJobs);
	void OnSysColorChange();
	void OnTimeChange();
	void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	void OnWinIniChange(LPCTSTR lpszSection);

// Input message handler member functions
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	int OnVKeyToItem(UINT nKey, HWND hListBox, UINT nIndex);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMButtonDblClk(UINT nFlags, CPoint point);
	void OnMButtonDown(UINT nFlags, CPoint point);
	void OnMButtonUp(UINT nFlags, CPoint point);
	int OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message);
	void OnMouseMove(UINT nFlags, CPoint point);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	LRESULT OnRegisteredMouseWheel(WPARAM wParam, LPARAM lParam);
	void OnRButtonDblClk(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnRButtonUp(UINT nFlags, CPoint point);
	BOOL OnSetCursor(HWND hWnd, UINT nHitTest, UINT message);
	void OnTimer(UINT_PTR nIDEvent);

// Initialization message handler member functions
	void OnInitMenu(HMENU hMenu);
	void OnInitMenuPopup(HMENU hPopupMenu, UINT nIndex, BOOL bSysMenu);

// Control message handler member functions
	int OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	UINT OnGetDlgCode();
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	int OnCharToItem(UINT nChar, HWND hListBox, UINT nIndex);
	int OnVKeyToItem(UINT nKey, HWND hListBox, UINT nIndex);
------------- */

#define BEGIN_MSG_EX_MAP(theClass) \
public: \
	MSG m_MsgCur; \
	LRESULT Default() \
	{ \
		return ::DefWindowProc(m_hWnd, m_MsgCur.message, m_MsgCur.wParam, m_MsgCur.lParam); \
	} \
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0) \
	{ \
		BOOL bHandled = TRUE; \
		(bHandled); \
		m_MsgCur.hwnd = hWnd; \
		m_MsgCur.message = uMsg; \
		m_MsgCur.wParam = wParam; \
		m_MsgCur.lParam = lParam; \
		m_MsgCur.time = GetMessageTime(); \
		m_MsgCur.pt = CPoint(GetMessagePos()); \
		switch(dwMsgMapID) \
		{ \
		case 0:

#define COMMAND_SIMPLE_HANDLER(id, code, func) \
	if(uMsg == WM_COMMAND && id == LOWORD(wParam) && code == HIWORD(wParam)) { \
		func(); \
		return TRUE; \
	}
#define COMMAND_SIMPLE_ID_HANDLER(id, func) \
	if (uMsg == WM_COMMAND && id == LOWORD(wParam)) { \
		func(); \
		lResult = 0; \
		return TRUE; \
	}
#define COMMAND_SIMPLE_RANGE_HANDLER(idFirst, idLast, func) \
	if(uMsg == WM_COMMAND && LOWORD(wParam) >= idFirst  && LOWORD(wParam) <= idLast) { \
		func((UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define MESSAGE_SIMPLE_HANDLER(id, func) \
	if (uMsg == id) { \
		lResult = func(wParam, lParam); \
		return TRUE; \
	}
#define MESSAGE_SIMPLE_VOID_HANDLER(id, func) \
	if (uMsg == id) { \
		func(wParam, lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_INITDIALOG_HANDLER() \
	if (uMsg == WM_INITDIALOG) { \
		lResult = (LRESULT)OnInitDialog(lParam); \
		return TRUE; \
	}
#define WM_NCCREATE_HANDLER() \
	if (uMsg == WM_NCCREATE) { \
		lResult = (LRESULT)OnNcCreate((LPCREATESTRUCT)lParam); \
		return TRUE; \
	}
#define WM_CREATE_HANDLER() \
	if (uMsg == WM_CREATE) { \
		lResult = (LRESULT)OnCreate((LPCREATESTRUCT)lParam); \
		return TRUE; \
	}
#define WM_DROPFILES_HANDLER() \
	if (uMsg == WM_DROPFILES) { \
		OnDropFiles((HDROP)wParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_DRAWITEM_HANDLER() \
	if (uMsg == WM_DRAWITEM) { \
		OnDrawItem((int)wParam, (LPDRAWITEMSTRUCT)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_MEASUREITEM_HANDLER() \
	if (uMsg == WM_MEASUREITEM) { \
		OnMeasureItem((int)wParam, (LPMEASUREITEMSTRUCT)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_DELETEITEM_HANDLER() \
	if (uMsg == WM_DELETEITEM) { \
		OnDeleteItem((int)wParam, (LPDELETEITEMSTRUCT)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_CHAR_HANDLER() \
	if (uMsg == WM_CHAR) { \
		OnChar((UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_CLOSE_HANDLER() \
	if (uMsg == WM_CLOSE) { \
		bHandled = FALSE; \
		OnClose(bHandled); \
		lResult = 0; \
		if (bHandled) \
			return TRUE; \
	}
#define WM_KEYDOWN_HANDLER() \
	if (uMsg == WM_KEYDOWN) { \
		OnKeyDown((UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_KEYUP_HANDLER() \
	if (uMsg == WM_KEYUP) { \
		OnKeyUp((UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_VKEYTOITEM_HANDLER() \
	if (uMsg == WM_VKEYTOITEM) { \
		lResult = (LRESULT)OnVKeyToItem((UINT)LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam)); \
		return TRUE; \
	}
#define WM_SIZE_HANDLER() \
	if (uMsg == WM_SIZE) { \
		OnSize((UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_COMMAND_HANDLER() \
	if (uMsg == WM_COMMAND) { \
		lResult = (LRESULT)OnCommand((UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HWND)lParam); \
		if (lResult) \
			return TRUE; \
	}
#define WM_MOUSEACTIVATE_HANDLER() \
	if (uMsg == WM_MOUSEACTIVATE) { \
		lResult = (LRESULT)OnMouseActivate((HWND)wParam, (UINT)(short)LOWORD(lParam), (UINT)HIWORD(lParam)); \
		return TRUE; \
	}
#define WM_MOUSEMOVE_HANDLER() \
	if (uMsg == WM_MOUSEMOVE) { \
		OnMouseMove((UINT)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_MOUSEHOVER_HANDLER() \
	if (uMsg == WM_MOUSEHOVER) { \
		OnMouseHover((UINT)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_MOUSELEAVE_HANDLER() \
	if (uMsg == WM_MOUSELEAVE) { \
		OnMouseLeave(); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_LBUTTONDOWN_HANDLER() \
	if (uMsg == WM_LBUTTONDOWN) { \
		OnLButtonDown((UINT)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_LBUTTONUP_HANDLER() \
	if (uMsg == WM_LBUTTONUP) { \
		OnLButtonUp((UINT)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_LBUTTONDBLCLK_HANDLER() \
	if (uMsg == WM_LBUTTONDBLCLK) { \
		OnLButtonDblClk((UINT)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_RBUTTONDOWN_HANDLER() \
	if (uMsg == WM_RBUTTONDOWN) { \
		OnRButtonDown((UINT)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_RBUTTONUP_HANDLER() \
	if (uMsg == WM_RBUTTONUP) { \
		OnRButtonUp((UINT)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_RBUTTONDBLCLK_HANDLER() \
	if (uMsg == WM_RBUTTONDBLCLK) { \
		OnRButtonDblClk((UINT)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_VSCROLL_HANDLER() \
	if (uMsg == WM_VSCROLL) { \
		OnVScroll((UINT)(short)LOWORD(wParam), (UINT)(short)HIWORD(wParam), (HWND)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_HSCROLL_HANDLER() \
	if (uMsg == WM_HSCROLL) { \
		OnHScroll((UINT)(short)LOWORD(wParam), (UINT)(short)HIWORD(wParam), (HWND)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_SETFOCUS_HANDLER() \
	if (uMsg == WM_SETFOCUS) { \
		OnSetFocus((HWND)wParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_KILLFOCUS_HANDLER() \
	if (uMsg == WM_KILLFOCUS) { \
		OnKillFocus((HWND)wParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_DESTROY_HANDLER() \
	if (uMsg == WM_DESTROY) { \
		OnDestroy(); \
		lResult = 0; \
	}
#define WM_NCDESTROY_HANDLER() \
	if (uMsg == WM_NCDESTROY) { \
		OnNcDestroy(); \
		lResult = 0; \
	}
#define WM_SETCURSOR_HANDLER() \
	if (uMsg == WM_SETCURSOR) { \
		lResult = (LRESULT)OnSetCursor((HWND)wParam, (UINT)(short)LOWORD(lParam), (UINT)HIWORD(lParam)); \
		return TRUE; \
	}
#define WM_CONTEXTMENU_HANDLER() \
	if (uMsg == WM_CONTEXTMENU) { \
		OnContextMenu((HWND)wParam, CPoint((DWORD)lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_ERASEBKGND_HANDLER() \
	if (uMsg == WM_ERASEBKGND) { \
		lResult = (LRESULT)OnEraseBkgnd((HDC)wParam); \
		return TRUE; \
	}
#define WM_MOUSEWHEEL_HANDLER() \
	if (uMsg == WM_MOUSEWHEEL) { \
		lResult = (LRESULT)OnMouseWheel((UINT)HIWORD(wParam), (SHORT)LOWORD(wParam), CPoint((DWORD)lParam)); \
		return TRUE; \
	}
#define WM_PAINT_HANDLER() \
	if (uMsg == WM_PAINT) { \
		OnPaint(); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_NCPAINT_HANDLER() \
	if (uMsg == WM_NCPAINT) { \
		bHandled = TRUE; \
		OnNcPaint((HRGN)wParam, bHandled); \
		lResult = 0; \
		if (bHandled) \
			return TRUE; \
	}
#define WM_PRINTCLIENT_HANDLER() \
	if (uMsg == WM_PRINTCLIENT) { \
		OnPrintClient((HDC)wParam, (DWORD)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_ACTIVATE_HANDLER() \
	if (uMsg == WM_ACTIVATE) { \
		OnActivate((UINT)LOWORD(lParam), (HWND)lParam, (BOOL)HIWORD(wParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_ACTIVATE_HANDLER() \
	if (uMsg == WM_ACTIVATE) { \
		OnActivate((UINT)LOWORD(lParam), (HWND)lParam, (BOOL)HIWORD(wParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_TIMER_HANDLER() \
	if (uMsg == WM_TIMER) { \
		OnTimer((UINT_PTR)wParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_GETMINMAXINFO_HANDLER() \
	if (uMsg == WM_GETMINMAXINFO) { \
		OnGetMinMaxInfo((MINMAXINFO*)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_SHOWWINDOW_HANDLER() \
	if (uMsg == WM_SHOWWINDOW) { \
		OnShowWindow((BOOL)wParam, (UINT)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_INITMENUPOPUP_HANDLER() \
	if (uMsg == WM_INITMENUPOPUP) { \
		OnInitMenuPopup((HMENU)wParam, (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_INITMENU_HANDLER() \
	if (uMsg == WM_INITMENU) { \
		OnInitMenu((HMENU)wParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_MENUCHAR_HANDLER() \
	if (uMsg == WM_MENUCHAR) { \
		lResult = OnMenuChar((UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HMENU)lParam); \
		return TRUE; \
	}
#define WM_SYSKEYUP_HANDLER() \
	if (uMsg == WM_SYSKEYUP) { \
		OnSysKeyUp((UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_SYSKEYDOWN_HANDLER() \
	if (uMsg == WM_SYSKEYDOWN) { \
		OnSysKeyDown((UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_HELP_HANDLER() \
	if (uMsg == WM_HELP) { \
		lResult = OnHelpInfo((HELPINFO*)lParam); \
		return TRUE; \
	}
#define WM_ENABLE_HANDLER() \
	if (uMsg == WM_ENABLE) { \
		OnEnable((BOOL)wParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_SETFONT_HANDLER() \
	if (uMsg == WM_SETFONT) { \
		OnSetFont((HFONT)wParam, (BOOL)LOWORD(lParam)); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_GETDLGCODE_HANDLER() \
	if (uMsg == WM_GETDLGCODE) { \
		lResult = OnGetDlgCode(); \
		return TRUE; \
	}
#define WM_SETTEXT_HANDLER() \
	if (uMsg == WM_SETTEXT) { \
		bHandled = TRUE; \
		OnSetText((LPCTSTR)lParam, bHandled); \
		lResult = 0; \
		if (bHandled) \
			return TRUE; \
	}
#define WM_COPYDATA_HANDLER() \
	if (uMsg == WM_COPYDATA) { \
		OnCopyData((HWND)wParam, (COPYDATASTRUCT*)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_QUERYENDSESSION_HANDLER() \
	if (uMsg == WM_QUERYENDSESSION) { \
		lResult = (LRESULT)OnQueryEndSession((UINT)wParam); \
		return TRUE; \
	}
#define WM_ENDSESSION_HANDLER() \
	if (uMsg == WM_ENDSESSION) { \
		OnEndSession((BOOL)wParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_WININICHANGE_HANDLER() \
	if (uMsg == WM_WININICHANGE) { \
		OnWinIniChange((LPCTSTR)lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_CANCELMODE_HANDLER() \
	if (uMsg == WM_CANCELMODE) { \
		OnCancelMode(); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_PARENTNOTIFY_HANDLER() \
	if (uMsg == WM_PARENTNOTIFY) { \
		OnParentNotify((UINT)wParam, lParam); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_THEMECHANGED_HANDLER() \
	if (uMsg == WM_THEMECHANGED) { \
		OnThemeChanged(); \
		lResult = 0; \
		return TRUE; \
	}
#define WM_NOTIFY_HANDLER(cd, id, func) \
	if (uMsg == WM_NOTIFY && ((NMHDR*)lParam)->idFrom == id && ((NMHDR*)lParam)->code == cd) { \
		lResult = 0; \
		func((NMHDR*)lParam, &lResult); \
		return TRUE; \
	}
#define WM_NOTIFY_RANGE_HANDLER(cd, id, idLast, func) \
	if (uMsg == WM_NOTIFY && ((NMHDR*)lParam)->idFrom >= id && ((NMHDR*)lParam)->idFrom <= idLast && ((NMHDR*)lParam)->code == cd) { \
		lResult = 0; \
		func((UINT)((NMHDR*)lParam)->idFrom, (NMHDR*)lParam, &lResult); \
		return TRUE; \
	}
#define WM_NOTIFY_CODE_HANDLER(cd, func) \
	if (uMsg == WM_NOTIFY && cd == ((LPNMHDR)lParam)->code) { \
		lResult = 0; \
		func((int)wParam, (LPNMHDR)lParam, &lResult); \
		return TRUE; \
	}


////////////////////////////////////////////////////////////////
//

#define ON_COMMAND(id, func)			COMMAND_SIMPLE_ID_HANDLER(id, func)
#define ON_COMMAND_RANGE(st, ed, func)	COMMAND_SIMPLE_RANGE_HANDLER(st, ed, func)
#define ON_MESSAGE(id, func)			MESSAGE_SIMPLE_HANDLER(id, func)
#define ON_MESSAGE_VOID(id, func)		MESSAGE_SIMPLE_VOID_HANDLER(id, func)
#define ON_REGISTERD_MESSAGE(id, func)	MESSAGE_SIMPLE_HANDLER(id, func)
#define ON_WM_INITDIALOG()				WM_INITDIALOG_HANDLER()
#define ON_WM_NCCREATE()				WM_NCCREATE_HANDLER()
#define ON_WM_CREATE()					WM_CREATE_HANDLER()
#define ON_WM_DROPFILES()				WM_DROPFILES_HANDLER()
#define ON_WM_DRAWITEM()				WM_DRAWITEM_HANDLER()
#define ON_WM_MEASUREITEM()				WM_MEASUREITEM_HANDLER()
#define ON_WM_DELETEITEM()				WM_DELETEITEM_HANDLER()
#define ON_WM_CHAR()					WM_CHAR_HANDLER()
#define ON_WM_CLOSE()					WM_CLOSE_HANDLER()
#define ON_WM_KEYDOWN()					WM_KEYDOWN_HANDLER()
#define ON_WM_KEYUP()					WM_KEYUP_HANDLER()
#define ON_WM_VKEYTOITEM()				WM_VKEYTOITEM_HANDLER()
#define ON_WM_SIZE()					WM_SIZE_HANDLER()
#define ON_WM_COMMAND()					WM_COMMAND_HANDLER()
#define ON_WM_MOUSEACTIVATE()			WM_MOUSEACTIVATE_HANDLER()
#define ON_WM_MOUSEMOVE()				WM_MOUSEMOVE_HANDLER()
#define ON_WM_MOUSEHOVER()				WM_MOUSEHOVER_HANDLER()
#define ON_WM_MOUSELEAVE()				WM_MOUSELEAVE_HANDLER()
#define ON_WM_LBUTTONDOWN()				WM_LBUTTONDOWN_HANDLER()
#define ON_WM_LBUTTONUP()				WM_LBUTTONUP_HANDLER()
#define ON_WM_LBUTTONDBLCLK()			WM_LBUTTONDBLCLK_HANDLER()
#define ON_WM_RBUTTONDOWN()				WM_RBUTTONDOWN_HANDLER()
#define ON_WM_RBUTTONUP()				WM_RBUTTONUP_HANDLER()
#define ON_WM_RBUTTONDBLCLK()			WM_RBUTTONDBLCLK_HANDLER()
#define ON_WM_VSCROLL()					WM_VSCROLL_HANDLER()
#define ON_WM_HSCROLL()					WM_HSCROLL_HANDLER()
#define ON_WM_SETFOCUS()				WM_SETFOCUS_HANDLER()
#define ON_WM_KILLFOCUS()				WM_KILLFOCUS_HANDLER()
#define ON_WM_DESTROY()					WM_DESTROY_HANDLER()
#define ON_WM_NCDESTROY()				WM_NCDESTROY_HANDLER()
#define ON_WM_SETCURSOR()				WM_SETCURSOR_HANDLER()
#define ON_WM_CONTEXTMENU()				WM_CONTEXTMENU_HANDLER()
#define ON_WM_ERASEBKGND()				WM_ERASEBKGND_HANDLER()
#define ON_WM_MOUSEWHEEL()				WM_MOUSEWHEEL_HANDLER()
#define ON_WM_PAINT()					WM_PAINT_HANDLER()
#define ON_WM_NCPAINT()					WM_NCPAINT_HANDLER()
#define ON_WM_PRINTCLIENT()				WM_PRINTCLIENT_HANDLER()
#define ON_WM_ACTIVATE()				WM_ACTIVATE_HANDLER()
#define ON_WM_TIMER()					WM_TIMER_HANDLER()
#define ON_WM_GETMINMAXINFO()			WM_GETMINMAXINFO_HANDLER()
#define ON_WM_SHOWWINDOW()				WM_SHOWWINDOW_HANDLER()
#define ON_WM_INITMENU()				WM_INITMENU_HANDLER()
#define ON_WM_INITMENUPOPUP()			WM_INITMENUPOPUP_HANDLER()
#define ON_WM_MENUCHAR()				WM_MENUCHAR_HANDLER()
#define ON_WM_SYSKEYUP()				WM_SYSKEYUP_HANDLER()
#define ON_WM_SYSKEYDOWN()				WM_SYSKEYDOWN_HANDLER()
#define ON_WM_HELPINFO()				WM_HELP_HANDLER()
#define ON_WM_ENABLE()					WM_ENABLE_HANDLER()
#define ON_WM_SETFONT()					WM_SETFONT_HANDLER()
#define ON_WM_GETDLGCODE()				WM_GETDLGCODE_HANDLER()
#define ON_WM_SETTEXT()					WM_SETTEXT_HANDLER()
#define ON_WM_COPYDATA()				WM_COPYDATA_HANDLER()
#define ON_WM_QUERYENDSESSION()			WM_QUERYENDSESSION_HANDLER()
#define ON_WM_ENDSESSION()				WM_ENDSESSION_HANDLER()
#define ON_WM_WININICHANGE()			WM_WININICHANGE_HANDLER()
#define ON_WM_CANCELMODE()				WM_CANCELMODE_HANDLER()
#define ON_WM_PARENTNOTIFY()			WM_PARENTNOTIFY_HANDLER()
#define ON_WM_THEMECHANGED()			WM_THEMECHANGED_HANDLER()
#define ON_NOTIFY(code, id, func)		WM_NOTIFY_HANDLER(code, id, func)
#define ON_NOTIFY_RANGE(code, id, idLast, func)		WM_NOTIFY_RANGE_HANDLER(code, id, idLast, func)
#define ON_NOTIFY_CODE(code, func)		WM_NOTIFY_CODE_HANDLER(code, func)

#define ON_CONTROL(code, id, func)		COMMAND_SIMPLE_HANDLER(id, code, func)
#define ON_BN_CLICKED(id, func)			COMMAND_SIMPLE_HANDLER(id, BN_CLICKED, func)
#define ON_BN_PAINT(id, func)			COMMAND_SIMPLE_HANDLER(id, BN_PAINT, func)
#define ON_BN_PUSHED(id, func)			COMMAND_SIMPLE_HANDLER(id, BN_PUSHED, func)
#define ON_BN_UNPUSHED(id, func)		COMMAND_SIMPLE_HANDLER(id, BN_UNPUSHED, func)
#define ON_BN_DISABLE(id, func)			COMMAND_SIMPLE_HANDLER(id, BN_DISABLE, func)
#define ON_BN_DBLCLK(id, func)			COMMAND_SIMPLE_HANDLER(id, BN_DBLCLK, func)
#define ON_BN_SETFOCUS(id, func)		COMMAND_SIMPLE_HANDLER(id, BN_SETFOCUS, func)
#define ON_BN_KILLFOCUS(id, func)		COMMAND_SIMPLE_HANDLER(id, BN_KILLFOCUS, func)

#define ON_CBN_SELCHANGE(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_SELCHANGE, func)
#define ON_CBN_SETFOCUS(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_SETFOCUS, func)
#define ON_CBN_KILLFOCUS(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_KILLFOCUS, func)
#define ON_CBN_ERRSPACE(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_ERRSPACE, func)
#define ON_CBN_DBLCLK(id, func)			COMMAND_SIMPLE_HANDLER(id, CBN_DBLCLK, func)
#define ON_CBN_EDITCHANGE(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_EDITCHANGE, func)
#define ON_CBN_EDITUPDATE(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_EDITUPDATE, func)
#define ON_CBN_DROPDOWN(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_DROPDOWN, func)
#define ON_CBN_CLOSEUP(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_CLOSEUP, func)
#define ON_CBN_SELENDOK(id, func)		COMMAND_SIMPLE_HANDLER(id, CBN_SELENDOK, func)
#define ON_CBN_SELENDCANCEL(id, func)	COMMAND_SIMPLE_HANDLER(id, CBN_SELENDCANCEL, func)

#define ON_LBN_SELCHANGE(id, func)		COMMAND_SIMPLE_HANDLER(id, LBN_SELCHANGE, func)
#define ON_LBN_DBLCLK(id, func)			COMMAND_SIMPLE_HANDLER(id, LBN_DBLCLK, func)
#define ON_LBN_SETFOCUS(id, func)		COMMAND_SIMPLE_HANDLER(id, LBN_SETFOCUS, func)
#define ON_LBN_KILLFOCUS(id, func)		COMMAND_SIMPLE_HANDLER(id, LBN_KILLFOCUS, func)
#define ON_LBN_ERRSPACE(id, func)		COMMAND_SIMPLE_HANDLER(id, LBN_ERRSPACE, func)
#define ON_LBN_SELCANCEL(id, func)		COMMAND_SIMPLE_HANDLER(id, LBN_SELCANCEL, func)

#define ON_EN_SETFOCUS(id, func)		COMMAND_SIMPLE_HANDLER(id, EN_SETFOCUS, func)
#define ON_EN_KILLFOCUS(id, func)		COMMAND_SIMPLE_HANDLER(id, EN_KILLFOCUS, func)
#define ON_EN_CHANGE(id, func)			COMMAND_SIMPLE_HANDLER(id, EN_CHANGE, func)
#define ON_EN_UPDATE(id, func)			COMMAND_SIMPLE_HANDLER(id, EN_UPDATE, func)
#define ON_EN_ERRSPACE(id, func)		COMMAND_SIMPLE_HANDLER(id, EN_ERRSPACE, func)
#define ON_EN_MAXTEXT(id, func)			COMMAND_SIMPLE_HANDLER(id, EN_MAXTEXT, func)
#define ON_EN_HSCROLL(id, func)			COMMAND_SIMPLE_HANDLER(id, EN_HSCROLL, func)
#define ON_EN_VSCROLL(id, func)			COMMAND_SIMPLE_HANDLER(id, EN_VSCROLL, func)


#endif					// __ATL_SPECIAL_HANDLER_H__

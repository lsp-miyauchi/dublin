/////////////////////////////////////////////////////////////////////
// xwin.h     windowsx.h �̊ȈՔ�

#pragma once
#ifndef __XWIN_H__
#define __XWIN_H__

#ifdef _INC_WINDOWSX
	#error xwin.h is parts of windowsx.h
#endif


#define INVALID_COLOR		((COLORREF)-1)
#define COLOR_NONE			((COLORREF)0xFFFFFFFFL)
#define COLOR_DEFAULT		((COLORREF)0xFF000000L)

#ifndef SNDMSG
	#ifdef __cplusplus
		#define SNDMSG ::SendMessage
	#else   /* __cplusplus */
		#define SNDMSG SendMessage
	#endif
#endif

#define GetWindowInstance(hwnd) ((HMODULE)::GetWindowLongPtr(hwnd, GWLP_HINSTANCE))
#define GetWindowStyle(hwnd)    ((DWORD)::GetWindowLong(hwnd, GWL_STYLE))
#define GetWindowExStyle(hwnd)  ((DWORD)::GetWindowLong(hwnd, GWL_EXSTYLE))
#define GetWindowOwner(hwnd)    ::GetWindow(hwnd, GW_OWNER)
#define GetWindowID(hwnd)       ::GetDlgCtrlID(hwnd)
#define SetWindowFont(hwnd, hfont, fRedraw) SNDMSG((hwnd), WM_SETFONT, (WPARAM)(hfont), (LPARAM)(fRedraw))
#define GetWindowFont(hwnd)     (HFONT)SNDMSG((hwnd), WM_GETFONT, 0, 0)
#define GetWindowID(hwnd)       ::GetDlgCtrlID(hwnd)

#define CtlColor_Static(a,b,c) 	(HBRUSH)SNDMSG((a),WM_CTLCOLORSTATIC,(WPARAM)(b),(LPARAM)(c))
#define CtlColor_Button(a,b,c) 	(HBRUSH)SNDMSG((a),WM_CTLCOLORBTN,(WPARAM)(b),(LPARAM)(c))

#define GetStockFont(i)         ((HFONT)GetStockObject(i))
#define GetStockPen(i)          ((HPEN)GetStockObject(i))
#define GetStockBrush(i)        ((HBRUSH)GetStockObject(i))


#define GlobalPtrHandle(lp)     ((HGLOBAL)GlobalHandle(lp))
#define GlobalLockPtr(lp)       ((BOOL)GlobalLock(GlobalPtrHandle(lp)))
#define GlobalUnlockPtr(lp)     GlobalUnlock(GlobalPtrHandle(lp))
#define GlobalAllocPtr(flg, cb) (GlobalLock(GlobalAlloc((flg), (cb))))
#define GlobalReAllocPtr(lp, cbNew, flags)       \
                                (GlobalUnlockPtr(lp), GlobalLock(GlobalReAlloc(GlobalPtrHandle(lp) , (cbNew), (flags))))
#define GlobalFreePtr(lp)       (GlobalUnlockPtr(lp), (BOOL)(ULONG_PTR)GlobalFree(GlobalPtrHandle(lp)))






#endif			// __XWIN_H__

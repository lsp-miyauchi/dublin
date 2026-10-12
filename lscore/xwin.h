/////////////////////////////////////////////////////////////////////
// xwin.h     windowsx.h の簡易版

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

////////////////////////////////////////////////////////////////////////
// HIMAGELIST の構造体情報 --- 出典 WINE imagelist.h より


#ifndef _WIN64		// i86のみ

#pragma pack(1)
typedef struct tagIMAGELISTSTRUCT_V1 {
	DWORDLONG magic1;				// 0x00: magic = 0x5ab80870
	DWORDLONG magic2;				// 0x08: magic???
	DWORD unused_dummy1;			// 0x10:
	DWORD unused_dummy2;			// 0x14:
	DWORD unused_dummy3;			// 0x18:
	int numCurrentImageCount;		// 0x1C: ImageCount
	int numMaxImageCount;			// 0x20: maximages
	int numGrow;					// 0x24: numGrow
	int cx;							// 0x28: cx
	int cy;							// 0x2C: cy
	DWORD unused_dummy5;			// 0x30:
	DWORD unused_dummy6;			// 0x34:
	COLORREF clrForeground;			// 0x38: foreground color
	COLORREF clrBackground;			// 0x3C: backgournd color
	DWORD unused_dummy7;			// 0x40:
	HBITMAP hbmImage;				// 0x44: images Bitmap
} IMAGELISTSTRUCT_V1;

typedef struct tagIMAGELISTSTRUCT_V2 {
	DWORDLONG magic1;				// 0x00: magic = 0x77162170
	DWORDLONG magic2;				// 0x08: magic???
	DWORD unused_dummy1;			// 0x10:
	DWORD unused_dummy2;			// 0x14:
	DWORD unused_dummy3;			// 0x18:
	DWORD unused_dummy4;			// 0x1C:
	int numCurrentImageCount;		// 0x20: ImageCount
	int numMaxImageCount;			// 0x24: maximages
	int numGrow;					// 0x28: numGrow
	int cx;							// 0x2C: cx
	int cy;							// 0x30: cy
	DWORD unused_dummy5;			// 0x34:
	DWORD unused_dummy6;			// 0x38:
	COLORREF clrForeground;			// 0x3C: foreground color
	COLORREF clrBackground;			// 0x40: backgournd color
	DWORD unused_dummy7;			// 0x44:
	HBITMAP hbmImage;				// 0x48: images Bitmap
} IMAGELISTSTRUCT_V2;

typedef struct tagIMAGELISTSTRUCT_V3 {
	DWORDLONG magic1;				// 0x00: magic = 0x76EB26E8 or 0x76EB4270
	DWORDLONG magic2;				// 0x08: magic???
	DWORD unused_dummy1;			// 0x10:
	DWORD unused_dummy2;			// 0x14:
	DWORD unused_dummy3;			// 0x18:
	DWORD unused_dummy4;			// 0x1C:
	DWORD unused_dummy5;			// 0x20:
	DWORD unused_dummy6;			// 0x24:
	DWORD unused_dummy7;			// 0x28:
	DWORD unused_dummy8;			// 0x2C:
	DWORD unused_dummy9;			// 0x30:
	DWORD unused_dummy10;			// 0x34:
	DWORD unused_dummy11;			// 0x38:
	int numCurrentImageCount;		// 0x3C: ImageCount
	int numMaxImageCount;			// 0x40: maximages
	int numGrow;					// 0x44: numGrow
	int cx;							// 0x48: cx
	int cy;							// 0x4C: cy
	DWORD unused_dummy12;			// 0x50:
	COLORREF clrForeground;			// 0x54: foreground color
	COLORREF clrBackground;			// 0x58: backgournd color
	DWORD unused_dummy13;			// 0x5C:
	HBITMAP hbmImage;				// 0x60: images Bitmap
} IMAGELISTSTRUCT_V3;

#define IMAGELIST_MAGIC_V1				0x5ab80870
#define IMAGELIST_MAGIC_V2				0x77162170
#define IMAGELIST_MAGIC_V3				0x76EB26E8
#define IMAGELIST_MAGIC_V3_2			0x76EB4270

#pragma pack()

#endif		// !_WIN64

////////////////////////////////////////////////////////////////////////
//

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

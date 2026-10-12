//////////////////////////////////////////////////////////////////////
// Local Application Messages Definition

#pragma once

#define WM_FILENOTIFY			(WM_USER + 390)
#define WM_IDLISTNOTIFY			(WM_USER + 391)
#define WM_ACTIVATENEXTVIEW		(WM_USER + 393)		// WPARAM=boundary
#define WM_SETACTIVEVIEW		(WM_USER + 395)		// WPARAM=setfocus, LPARAM=HWND
#define WM_ENABLEACCEL			(WM_USER + 396)		// WPARAM=bEnable
#define WM_QUERYIMAGELIST		(WM_USER + 397)		// WPARAM=idCommand, LPARAM=(int*)iImage, return HIMAGELIST
#define WM_MBMTAKEFOCUS			(WM_USER + 398)
#define WM_JUMPFILE				(WM_USER + 0xF21)	// LPARAM=�t���p�X
#define WM_OPENFOLDER			(WM_USER + 0xF22)	// lParam = idlist
#define WM_OPENTABFOLDER		(WM_USER + 0xF23)	// wParam = true then tab is activate, else tab is inactive, lParam = idlist
#define WM_SELECTFOLDERITEM		(WM_USER + 0xF24)	// wParam = N/A, lParam = idlist
#define WM_GETTABSFOLDERIDLIST	(WM_USER + 0xF25)	// wParam = index, lParam N/A
#define WM_CLEARFOLDERHISTORY	(WM_USER + 0xF26)	// wParam = N/A, lParam N/A
#define WM_ADDFAVORITE			(WM_USER + 0xF27)	// idlist
#define WM_OPENTABGROUP			(WM_USER + 0xF28)	// wParam = N/A, lParam = tabgroup-filename

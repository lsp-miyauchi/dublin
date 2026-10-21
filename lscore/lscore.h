///////////////////////////////////////////////////////
// lscore.h

#pragma once
#ifndef _LSCORE_H_
#define _LSCORE_H_

#define WC_LSC_STATIC			_T("LSCStatic")
#define	WC_LSC_COLOR			_T("LSCColor")
#define WC_LSC_METER			_T("LSCMeter")
#define WC_LSC_POPVIEW			_T("LSCPopView")
#define WC_LSC_RECT				_T("LSCRect")
#define WC_LSC_CHECK			_T("LSCCheck")
#ifdef USE_PRGLIST
	#define WC_LSC_PRGLIST		_T("LSCPrgList")
#endif
#define WC_LSC_SHADE			_T("LSCShade")
#define WC_LSC_LEDCLOCK			_T("LSCLedClock")
#define WC_LSC_ANIIMAGE			_T("LSCAniImage")
#define WC_LSC_TASKLINK			_T("LSCTaskLink")
#define WC_LSC_URL				_T("LSCUrl")
#define WC_LSC_PAL				_T("LSCPal")
#define WC_LSC_COLORPICK		_T("LSCColorPick")
#define WC_LSC_COLORPOP			_T("LSCColorPop")
#define WC_LSC_FONTPICK			_T("LSCFontPick")
#define WC_LSC_DEGREE			_T("LSCDegree")
#define WC_LSC_TRACK			_T("LSCTrack")
#define WC_LSC_SLIDER			_T("LSCSlider")
#define WC_LSC_LOG				_T("LSCLog")
#ifdef USE_LISTCONTROL
	#define WC_LSC_LIST			_T("LSCList")
#endif
#define WC_LSC_PROGRESS			_T("LSCProgress")
#define WC_LSC_INFOBAR			_T("LSCInfoBar")
#define WC_LSC_FLATBUTTON		_T("LSCFlatButton")
#define WC_LSC_ZOOMVIEW			_T("LSCZoomView")
#define WC_LSC_GRADEDIT			_T("LSCGradEdit")
#define WC_LSC_SIGNAL			_T("LSCSignal")
#define WC_LSC_LISTVIEW			_T("LSCListView")
#define WC_LSC_LISTCTRL			_T("LSCListCtrl")
#define WC_LSC_TIPEDIT			_T("LSCTipEdit")
#define WC_LSC_FOLDERTREE		_T("LSCFolderTree")
#define WC_LSC_IDLISTPICK		_T("LSCIDListPick")
#define WC_LSC_SPLITTER			_T("LSCSplitter")
#define WC_LSC_CHECKTREEVIEW	_T("LSCCheckTreeView")
#define WC_LSC_TAB				_T("LSCTab")
#define WC_LSC_TAB_EX			_T("LSCTabEx")
#define WC_LSC_HEADER			_T("LSCHeader")
#define WC_LSC_SEARCHPATH		_T("LSCSearchPath")

///////////////////////////////////////////////////////
// useful macros

#define _MKIR(a)				(MAKEINTRESOURCE((a)))
#define _INVALID_HIMAGELIST		((HIMAGELIST)(LONG_PTR)-1)


///////////////////////////////////////////////////////
// lccDrawMenuButton style

enum {
	bmenuDefault,
	bmenuXP,
	bmenuWindows,
};

#define COLOR_VISTA_MENUBKGND		(RGB(0xfa, 0xfa, 0xf8))


///////////////////////////////////////////////////////
// RECT control style
//
#define RS_POP			0x00000001L             // Rect Style
#define RS_LEFT			0x00000002L				//
#define RS_RIGHT		0x00000004L				//
#define RS_ICON			0x00000008L				//
#define RS_HORZBORDER	0x00000010L				//
#define RS_VERTBORDER	0x00000020L				//
#define RS_NOEDGE		0x00000040L             //
#define RS_ENDELLIPSIS	0x00000080L             //
#define RS_OWNERDRAW    0x00000100L             //


///////////////////////////////////////////////////////
// METER control style
//
#define MS_BLOCKBAR     0x00000001L             // Meter Style
#define MS_FRAMERECT    0x00000010L             //  "
#define MS_LARGEBLOCK   0x00000002L				//  "


///////////////////////////////////////////////////////
// SPIN control style
//
#define SPNS_WRAP       0x00000001L


///////////////////////////////////////////////////////
// METER control command message
//
#define MM_SETSIZE				(WM_USER + 0x110)
#define MM_GETSIZE				(WM_USER + 0x111)
#define MM_SETCOMPLETE			(WM_USER + 0x112)
#define MM_GETCOMPLETE			(WM_USER + 0x113)
#define MM_SETTEXTCOLOR			(WM_USER + 0x114)
#define MM_GETTEXTCOLOR			(WM_USER + 0x115)
#define MM_SETBKCOLOR			(WM_USER + 0x116)
#define MM_GETBKCOLOR			(WM_USER + 0x117)


///////////////////////////////////////////////////////
// SPIN control command message
//
#define SM_SETRANGE				(WM_USER + 0x120)
#define SM_GETRANGE				(WM_USER + 0x121)
#define SM_SETCURVAL			(WM_USER + 0x122)
#define SM_GETCURVAL			(WM_USER + 0x123)
#define SM_SCROLLVALUE			(WM_USER + 0x124)
#define SN_VALUECHANGE			(1)


///////////////////////////////////////////////////////
// TAB control command message
//
#define TBM_SETTITLE    		(WM_USER + 0x130)
#define TBM_GETFOCUS    		(WM_USER + 0x131)
#define TBM_SETFOCUS    		(WM_USER + 0x132)
#define TBM_SETBUDDYBASE		(WM_USER + 0x133)	// wParam: 0=%d, 1= %.1f, 2=%2.f


///////////////////////////////////////////////////////
// COLOR control command message
//
#define CM_GETCOLORREF          (WM_USER + 0x140)
#define CM_GETNEARESTCOLORREF   (WM_USER + 0x141)
#define CM_SETCOLORREF          (WM_USER + 0x142)
#define CN_SELCOLORREF          (1)
#define CN_SELNEARESTCOLORREF   (2)


///////////////////////////////////////////////////////
// RECT control command message
//
#define RM_GETTEXTCOLOR			(WM_USER + 0x150)
#define RM_GETBKCOLOR			(WM_USER + 0x151)
#define RM_SETTEXTCOLOR			(WM_USER + 0x152)
#define RM_SETBKCOLOR			(WM_USER + 0x153)
#define RM_SETICON				(WM_USER + 0x154)
#define RM_GETEDGECOLOR			(WM_USER + 0x155)
#define RM_SETEDGECOLOR			(WM_USER + 0x156)

#define RectCtrl_GetTextColor(hwnd)				(COLORREF)SNDMSG((hwnd), RM_GETTEXTCOLOR, 0, 0)
#define RectCtrl_GetBkColor(hwnd)				(COLORREF)SNDMSG((hwnd), RM_GETBKCOLOR, 0, 0)
#define RectCtrl_GetEdgeColor(hwnd)				(COLORREF)SNDMSG((hwnd), RM_GETEDGECOLOR, 0, 0)
#define RectCtrl_SetTextColor(hwnd, clr)		SNDMSG((hwnd), RM_SETTEXTCOLOR, 0, (LPARAM)(clr))
#define RectCtrl_SetBkColor(hwnd, clr)			SNDMSG((hwnd), RM_SETBKCOLOR, 0, (LPARAM)(clr))
#define RectCtrl_SetEdgeColor(hwnd, clr)		SNDMSG((hwnd), RM_SETEDGECOLOR, 0, (LPARAM)(clr))
#define RectCtrl_SetIcon(hwnd, hic)				SNDMSG((hwnd), RM_SETICON, 0, (LPARAM)(hic))


///////////////////////////////////////////////////////
// GUID control command message
//
#define GM_GETCOLOR				(WM_USER + 0x170)
#define GM_SETCOLOR				(WM_USER + 0x171)


///////////////////////////////////////////////////////
// BROWSE control command message
//

#define PLM_ADD					(WM_USER + 0x190)
#define PLM_DELETE				(WM_USER + 0x191)
#define PLM_SETSTATE			(WM_USER + 0x192)

enum {
	PLITEM_NONE,
	PLITEM_CONT,
	PLITEM_DONE,
	PLITEM_ERR
};


///////////////////////////////////////////////////////
// SHADE control command message
//

#define SDM_SETCOLORS			(WM_USER + 0x200)


///////////////////////////////////////////////////////
// LED Clock control command message
//

#define LCM_START				(WM_USER + 0x210)
	#define WLC_CLOCK			0		// 時計モード
	#define WLC_TIMER			1		// タイマモード
	#define WLC_COUNTDOWN		2		// カウントダウンモード
										// LPARAM = カウント開始値
#define LCM_STOP				(WM_USER + 0x211)
#define LCM_SETBKCOLOR			(WM_USER + 0x212)
#define LCM_RESET				(WM_USER + 0x213)

#define LCN_TIMEOUT				(1)

#define LCS_CLOCK				0x00000000
#define LCS_TIMER				0x00000001
#define LCS_COUNTDOWN			0x00000002
#define LCS_MODEMASK			0x0000000F
#define LCS_LARGE				0x00000010
#define LCS_AUTOSTART			0x00000020

#define LedClock_Start(hwnd)				SNDMSG((hwnd), LCM_START, 0, 0)
#define LedClock_Stop(hwnd)					SNDMSG((hwnd), LCM_STOP, 0, 0)
#define LedClock_Reset(hwnd)				SNDMSG((hwnd), LCM_RESET, 0, 0)


///////////////////////////////////////////////////////
// Animation_Image control command message
//

#define AIM_START				(WM_USER + 0x220)
#define AIM_STOP				(WM_USER + 0x221)
#define AIM_RESET				(WM_USER + 0x222)
#define AIM_SETANIMATION		(WM_USER + 0x223)
#define AIM_SETBKCOLOR			(WM_USER + 0x224)


///////////////////////////////////////////////////////
// ColorPick control command message
//

#define CPM_GETTEXTCOLOR        (WM_USER + 0x230)
#define CPM_GETBKCOLOR          (WM_USER + 0x231)
#define CPM_SETTEXTCOLOR        (WM_USER + 0x232)
#define CPM_SETBKCOLOR          (WM_USER + 0x233)

#define CPS_STYLE_MASK			0x0000000FL
#define CPS_GRADATION			0x00000001L
#define CPS_WEBSAFE				0x00000002L
#define CPS_IE4					0x00000003L

#define CPPAL_NORMAL			MAKEINTRESOURCE(0)
#define CPPAL_GRADIANT			MAKEINTRESOURCE(1)
#define CPPAL_WEBSAFE			MAKEINTRESOURCE(2)
#define CPPAL_IE4				MAKEINTRESOURCE(3)

#define ColorPicker_GetTextColor(hwnd)			(COLORREF)SNDMSG((hwnd), CPM_GETTEXTCOLOR, 0, 0)
#define ColorPicker_GetBkColor(hwnd)			(COLORREF)SNDMSG((hwnd), CPM_GETBKCOLOR, 0, 0)
#define ColorPicker_SetTextColor(hwnd, clr)		SNDMSG((hwnd), CPM_SETTEXTCOLOR, 0, (LPARAM)(clr))
#define ColorPicker_SetBkColor(hwnd, clr)		SNDMSG((hwnd), CPM_SETBKCOLOR, 0, (LPARAM)(clr))



///////////////////////////////////////////////////////
// FontPick control command message
//

#define FPM_GETLOGFONT          (WM_USER + 0x240)
#define FPM_SETLOGFONT          (WM_USER + 0x241)

#define FPS_SCREENFONTS			0x00000020L
#define FPS_TTONLY					0x00000001L
#define FPS_PRINTERFONTS		0x00000002L
#define FPS_BOTH            (FPS_SCREENFONTS | FPS_PRINTERFONTS)
#define FPS_FIXEDPITCHONLY	0x00000004L
#define FPS_ANSIONLY				0x00000008L
#define FPS_NOVERTFONTS			0x00000010L

#define FontPicker_GetLogFont(hwnd, plf)		SNDMSG((hwnd), FPM_GETLOGFONT, 0, (LPARAM)(plf))
#define FontPicker_SetLogFont(hwnd, plf)		SNDMSG((hwnd), FPM_SETLOGFONT, 0, (LPARAM)(plf))



///////////////////////////////////////////////////////
// Degree control command message
//

#define DGM_SETRANGE            (WM_USER + 0x250)
#define DGM_GETRANGE            (WM_USER + 0x251)
#define DGM_SETPOS              (WM_USER + 0x252)
#define DGM_GETPOS              (WM_USER + 0x253)


///////////////////////////////////////////////////////
// Trackbar control notification
//

typedef struct tagTRACKBARVALUE {
	NMHDR hdr;
	int pos;
	TCHAR szValue[80];
} TRACKBARVALUE;

#define TRBN_GETVALUE			1



///////////////////////////////////////////////////////
// ColorPop control command message
//

typedef struct tagCP_NOTIFY {
	NMHDR hdr;
	COLORREF rgb;
} CP_NOTIFY, *LPCP_NOTIFY;

enum {
	CPN_SELENDOK,
	CPN_SELENDCANCEL,
	CPN_SELENDCUSTOM
};


///////////////////////////////////////////////////////
// Palette control command message
//

#define PLTM_SETPALETTE			(WM_USER + 0x230)
#define PLTM_GETCOUNT			(WM_USER + 0x231)
#define PLTM_GETSELCOUNT		(WM_USER + 0x232)
#define PLTM_SETSELITEMS		(WM_USER + 0x233)
#define PLTM_GETSELITEMS		(WM_USER + 0x234)
#define PLTM_SETDIBPALETTE		(WM_USER + 0x235)
#define PLTM_GETCOLOR			(WM_USER + 0x236)
#define PLTM_SETCOLOR			(WM_USER + 0x237)
#define PLTM_GETCURSEL			(WM_USER + 0x238)
#define PLTM_SETCURSEL			(WM_USER + 0x239)

#define PLTS_ALLOWEDIT			0x00000001L
#define PLTS_SELMULTIPLE		0x00000002L


typedef struct tagPALNOTIFY {
	NMHDR hdr;
	int nIndex;
	COLORREF rgb;
} PALNOTIFY, *LPPALNOTIFY;

#define PLTN_CLICK				1
#define PLTN_DBLCLICK			2
#define PLTN_PALETTECHANGED		3
#define PLTN_SELCHANGED			4


///////////////////////////////////////////////////////
// Log control command message
//

#define LGM_ADDLOG				(WM_USER + 0x240)
#define LGM_CLEARLOG			(WM_USER + 0x241)


///////////////////////////////////////////////////////
// List control command message
//

#define OLM_ADDDATA				(WM_USER + 0x250)
#define OLM_DELETEDATA			(WM_USER + 0x251)
#define OLM_INSERTDATA			(WM_USER + 0x252)
#define OLM_GETITEMDATA			(WM_USER + 0x253)
#define OLM_SETITEMDATA			(WM_USER + 0x254)
#define OLM_GETCOUNT			(WM_USER + 0x255)
#define OLM_SETCURSEL			(WM_USER + 0x256)
#define OLM_GETCURSEL			(WM_USER + 0x257)
#define OLM_GETTOPINDEX			(WM_USER + 0x258)
#define OLM_GETITEMRECT			(WM_USER + 0x259)
#define OLM_RESETCONTENT		(WM_USER + 0x25a)


///////////////////////////////////////////////////////
// InfoBar control command message
//

#define IBM_SETBKCOLOR			(WM_USER + 0x260)
#define IBM_SETLEFTTEXT			(WM_USER + 0x261)
#define IBM_SETCENTERTEXT		(WM_USER + 0x262)
#define IBM_SETRIGHTTEXT		(WM_USER + 0x263)

#define InfoBar_SetBkColor(hwnd, crf, crt)			SNDMSG((hwnd), IBM_SETBKCOLOR, (WPARAM)(crf), (LPARAM)(crt))
#define InfoBar_SetLeftText(hwnd, cr, str)			SNDMSG((hwnd), IBM_SETLEFTTEXT, (WPARAM)(cr), (LPARAM)(str))
#define InfoBar_SetCenterText(hwnd, cr, str)		SNDMSG((hwnd), IBM_SETCENTERTEXT, (WPARAM)(cr), (LPARAM)(str))
#define InfoBar_SetRightText(hwnd, cr, str)			SNDMSG((hwnd), IBM_SETRIGHTTEXT, (WPARAM)(cr), (LPARAM)(str))


///////////////////////////////////////////////////////
// Degree / Track control command message
//

#define TBM_SETBKCOLORS         (WM_USER + 0x270)
#define TBM_SETBKCOLORSEX       (WM_USER + 0x271)		// wParam:#colors, lParam COLORREF*
#define TBM_SETTHUMBIMAGE		(WM_USER + 0x272)		// lParam:ImageList
#define TBM_GETTHUMBIMAGE		(WM_USER + 0x273)		// return ImageList
#define TBM_SETSUFFIX			(WM_USER + 0x274)		// wParam:n/a lParam: suffx_string_ptr


#define Degree_SetBuddy(hctl, hBuddy)					\
		(::SendMessage(hctl, TBM_SETBUDDY, 0, (LPARAM)hBuddy))
#define Degree_SetPos(hctl, nPos, bRedraw)				\
		(::SendMessage(hctl, TBM_SETPOS, bRedraw, nPos))
#define Degree_SetRange(hctl, nLower, nUpper, bRedraw)	\
		(::SendMessage(hctl, TBM_SETRANGE, bRedraw, MAKELPARAM(nUpper,nLower)))
#define Degree_SetPageSize(hctl, nPage)					\
		(::SendMessage(hctl, TBM_SETPAGESIZE,0,nPage))
#define Degree_GetPos(hctl)								\
		(int)(::SendMessage(hctl,TBM_GETPOS,0,0))
#define Degree_SetBkColors(hctl, crFrom, crTo)			\
		(::SendMessage(hctl, TBM_SETBKCOLORS, (WPARAM)crTo, (LPARAM)crFrom))
#define Degree_SetBkColorsEx(hctl, numcolors, pcrArray)	\
		(::SendMessage(hctl, TBM_SETBKCOLORSEX, (WPARAM)numcolors, (LPARAM)pcrArray))
#define Degree_SetBuddyBase(hctl, buddyBase)			\
		(::SendMessage(hctl, TBM_SETBUDDYBASE, (WPARAM)buddyBase, 0))
#define Degree_SetSelection(hctl, nLower, nUpper, bRedraw)	\
		(::SendMessage(hctl, TBM_SETSEL, bRedraw, MAKELPARAM(nUpper,nLower)))
#define Degree_SetTicFreq(hctl, nNumFreq)				\
		(::SendMessage(hctl, TBM_SETTICFREQ, (WPARAM)nNumFreq, 0))



///////////////////////////////////////////////////////
// Flat Button control style
//
#define FBS_FRAMEBORDER			0x00000010L
#define FBS_SHAREIMAGELIST		0x00000040L
#define FBS_COLOREDFOCUS		0x00000080L
#define FBS_DROPDOWN			0x00000100L
#define FBS_NOTEXT				0x00000200L
#define FBS_WHOLEDROPDOWN		0x00000400L
#define FBS_PUSHBUTTON			0x00008000L

#define FBM_SETIMAGELIST		(WM_USER + 0x280)
#define FBM_SETBKCOLOR			(WM_USER + 0x281)
#define FBM_SETFOCUSCOLOR		(WM_USER + 0x282)
#define FBM_LOADIMAGELIST		(WM_USER + 0x283)
#define FBM_SETSHIELD			(WM_USER + 0x284)
#define FBM_SETTIPTEXT			(WM_USER + 0x285)

#define FBITYPE_MASK			0x0000000FL
#define FBITYPE_FOLDER			1
#define FBITYPE_HELP			2
#define FBITYPE_COLOR			3
#define FBITYPE_HINT			4
#define FBITYPE_BROWSE			5

#define FlatButton_LoadImageList(hwnd, inst, id)				SNDMSG((hwnd), FBM_LOADIMAGELIST, (WPARAM)(inst), (LPARAM)(id))
#define FlatButton_SetImageList(hwnd, him)						SNDMSG((hwnd), FBM_SETIMAGELIST, 0, (LPARAM)(him))
#define FlatButton_SetBkColor(hwnd, clr)						SNDMSG((hwnd), FBM_SETBKCOLOR, 0, (LPARAM)(clr))
#define FlatButton_SetElevationRequiredState(hwnd, fRequired)	SNDMSG((hwnd), FBM_SETSHIELD, 0, (LPARAM)(fRequired))
#define FlatButton_SetTipText(hwnd, pszTipText)					SNDMSG((hwnd), FBM_SETTIPTEXT, 0, (LPARAM)(LPCTSTR)(pszTipText))


///////////////////////////////////////////////////////
// TaskLink Button control style
//
#define TLM_SETCOLORS			(WM_USER + 0x290)
#define TLM_SETBKCOLOR			(WM_USER + 0x291)
#define TLM_SETIMAGE			(WM_USER + 0x292)
#define TLM_GETIMAGE			(WM_USER + 0x293)
#define TLM_SETICON				(WM_USER + 0x294)
#define TLM_GETICON				(WM_USER + 0x295)

#define TLS_LEFT				0x00000000L
#define TLS_CENTER				0x00000001L
#define TLS_RIGHT				0x00000002L
#define TLS_TOP					0x00000000L
#define TLS_VCENTER				0x00000004L
#define TLS_BOTTOM				0x00000008L
#define TLS_SINGLELINE			0x00000020L
#define TLS_PATH_ELLIPSIS		0x00000100L
#define TLS_END_ELLIPSIS		0x00000200L
#define TLS_UNDERLINE			0x00001000L

typedef struct tagTASKLINKCOLORS {
	COLORREF clrText;
	COLORREF clrFly;
	COLORREF clrClicked;
} TASKLINKCOLORS, *PTASKLINKCOLORS;

///////////////////////////////////////////////////////
// Url Button control style
//
#define UM_SETCOLORS			TLM_SETCOLORS
#define UM_SETBKCOLOR			TLM_SETBKCOLOR


///////////////////////////////////////////////////////
// PopView control style
//
#define PVS_CAPTION				0x00000001
#define PVS_CLOSEBUTTON			0x00000002


///////////////////////////////////////////////////////
// ZoomView control command message
//

#define ZMVM_SETIMAGE			(WM_USER + 0x2a0)	// wParam:redarw_flg lParam:hdib
#define ZMVM_GETIMAGESIZE		(WM_USER + 0x2a1)	// wParam:na lParam:SIZE*
#define ZMVM_GETSCALE			(WM_USER + 0x2a2)
#define ZMVM_ZOOMIN				(WM_USER + 0x2a3)	// wParam:mode
#define ZMVM_SETMODE			(WM_USER + 0x2a4)	// wParam:mode
#define ZMVM_GETMODE			(WM_USER + 0x2a5)
#define ZMVM_SETBKCOLOR			(WM_USER + 0x2a6)	// wParam:redraw_flg lParam:color
#define ZMVM_GETBKCOLOR			(WM_USER + 0x2a7)
#define ZMVM_SETCLIPRECT		(WM_USER + 0x2a8)	// lParam:rect_pointer
#define ZMVM_GETCLIPRECT		(WM_USER + 0x2a9)	// lParam:rect_pointer
#define ZMVM_SCALEMENU			(WM_USER + 0x2aa)	// lParam:point_pointer
#define ZMVM_SETGRID			(WM_USER + 0x2ab)	// wParam:TRUE is show grid
													// lParam:0 > grid_size
#define ZMVM_GETGRID			(WM_USER + 0x2ac)
#define ZMVM_SETINFO			(WM_USER + 0x2ad)	// lParam:ZOOMVIEWINFO struct
#define ZMVM_GETINFO			(WM_USER + 0x2ae)	// lParam:ZOOMVIEWINFO struct
#define ZMVM_GETIMAGE			(WM_USER + 0x2af)	//
#define ZMVM_CLIENTTODEVICE		(WM_USER + 0x2b1)	// wParam:# of points lParam:client LPPOINT array
													// ret true if OK
#define ZMVM_DEVICETOCLIENT		(WM_USER + 0x2b2)	// wParam:# of points lParam:device LPPOINT array
													// ret true if OK
#define ZMVM_TRACKIMAGE			(WM_USER + 0x2b3)	// wParam: HIMAGELIST, lParam:LPPOINT [OUT] track_end position
													// ret true if TRACK OK
#define ZMVM_SETSCALE			(WM_USER + 0x2b4)	// wParam: scale * 1000, i.e. 120% is 1200
													// lParam: POINT*, center point, if NULL auto centered
#define ZMVM_GETSCALEFACTORLIST	(WM_USER + 0x2b5)	// wParam: max array, lParam: double array ptr
#define ZMVM_CENTERPOSITION		(WM_USER + 0x2b6)	// wParam: n/a, lParam: n/a
#define ZMVM_SETDRAWMODE		(WM_USER + 0x2b7)	// wParam: 0 = use BITMAP, 1 = use GDI+, 2 = use DIB
#define ZMVM_USEGDIPLUS			ZMVM_SETDRAWMODE
#define ZMVM_GETDRAWMODE		(WM_USER + 0x2b8)
#define ZMVM_CANZOOM			(WM_USER + 0x2b9)	// LOWORD(ret_code) : CanZoomIn, HIWORD(ret_code) : CanZoomOut
#define ZMVM_GETPIXELCOLOR		(WM_USER + 0x2ba)	// lParam = MAKELONG(X_POS, Y_POS), ret = COLORREF
#define ZMVM_UPDATEIMAGE		(WM_USER + 0x2bb)	// wParam:redarw_flg lParam:hdib

#define ZMVS_STYLE_NONE			0x00000000
#define ZMVS_STYLE_ZOOM			0x00000001
#define ZMVS_STYLE_GRAB			0x00000002
#define ZMVS_STYLE_SEL			0x00000003
#define ZMVS_STYLE_MASK			0x0000000F
#define ZMVS_SHAREIMAGE			0x00000010
#define ZMVS_GRID				0x00000020

typedef struct tagZOOMVIEWINFO_V1	{
	UINT cbSize;
	UINT fMask;
	SIZE grid;
	BOOL showGrid;
	COLORREF crGrid;
	COLORREF crBack;
	int mode;
	RECT rcClip;
	int scale;										// scale * 1000
} ZOOMVIEWINFO_V1;

typedef struct tagZOOMVIEWINFO	{
	UINT cbSize;
	UINT fMask;
	SIZE grid;
	BOOL showGrid;
	COLORREF crGrid;
	COLORREF crBack;
	int mode;
	RECT rcClip;
	int scale;										// scale * 1000
	POINT ptCenterPosition;
} ZOOMVIEWINFO;

// define for ZOOMVIEWINFO's fMask value
#define ZIF_GRID				0x00000001
#define ZIF_GRIDCOLOR			0x00000002
#define ZIF_MODE				0x00000004
#define ZIF_CLIP				0x00000008
#define ZIF_SCALE				0x00000010
#define ZIF_BKCOLOR				0x00000020
#define ZIF_SHOWGRID			0x00000040
#define ZIF_CENTERPOSITION		0x00000080


enum {					// ZMVM_ZOOMIN
	DOZOOM_IN,
	DOZOOM_OUT,
	DOZOOM_FULL,
	DOZOOM_DEFAULT,
	DOZOOM_FITHORZ,
	DOZOOM_FITVERT,
};

enum {					// ZMVM_SETMODE
	ZOOMMODE_NONE,
	ZOOMMODE_ZOOM,
	ZOOMMODE_GRAB,
	ZOOMMODE_SEL,
};

typedef struct tagZOOM_SCALE {
	NMHDR hdr;
	double scale;
} ZOOM_SCALE;

typedef struct tagZOOM_MOUSEPOINT {
	NMHDR hdr;
	POINT pt;					// マウスカーソル位置
	RECT rcClip;				// 選択範囲の矩形
	BOOL bInDrag;				// ドラッグ中か否か
} ZOOM_MOUSEPOINT;

typedef struct tagZOOM_CLIP {
	NMHDR hdr;
	RECT rcClip;
} ZOOM_CLIP;

typedef struct tagZOOM_GETDISPINFO {
	NMHDR hdr;
	DWORD mask;
	HBITMAP hbm;
	RECT rect;
} ZOOM_GETDISPINFO;

typedef struct tagZOOM_VISIBLEINFO {
	NMHDR hdr;
	SIZE sizeImage;				// 実イメージサイズ
	RECT rectVisible;			// 実イメージの可視矩形
} ZOOM_VISIBLEINFO;


#define ZMVF_SIZE				0x00000001L
#define ZMVF_HBITMAP			0x00000002L
#define ZMVF_DI_SETITEM			0x00001000L

#define ZMVN_SCALECHANGED		1
#define ZMVN_MOUSEPOINT			2
#define ZMVN_CLIPCHANGED		3
#define ZMVN_GETDISPINFO		4
#define ZMVN_VISIBLECHANGED		5

#ifndef HBITMAP_CALLBACK
#define HBITMAP_CALLBACK		((HBITMAP)-1L)
#endif

enum {
	ZVDM_BITMAP = 0,
	ZVDM_GDIPLUS,
	ZVDM_DIB,
	ZVDM_DIRECT2D,
};


///////////////////////////////////////////////////////
// GradEdit control command message
//

#define GEM_SETCOLORS			(WM_USER + 0x2b0)	// wParam:num entries lParam:struct
#define GEM_GETCOLORS			(WM_USER + 0x2b1)
#define GEM_GETNUMCOLORS		(WM_USER + 0x2b2)
#define GEM_GETNUMLIMIT			(WM_USER + 0x2b3)
#define GEM_SETNUMLIMIT			(WM_USER + 0x2b4)	// wParam:num_limits
#define GEM_REVERSEMAP			(WM_USER + 0x2b5)

#define GEN_COLORCHANGE			1
#define GEN_SETFOCUS			2
#define GEN_KILLFOCUS			3

typedef struct tagGRAD_COLORS {
	COLORREF cr;
	int pos;
} GRAD_COLORS;

#define GradEdit_SetColors(hwnd, n, cr)			SNDMSG((hwnd), GEM_SETCOLORS, (WPARAM)(n), (LPARAM)(cr))
#define GradEdit_GetColors(hwnd, cr)			(int)SNDMSG((hwnd), GEM_GETCOLORS, 0, (LPARAM)(cr))
#define GradEdit_GetNumColors(hwnd)				(int)SNDMSG((hwnd), GEM_GETNUMCOLORS, 0, 0)
#define GradEdit_GetNumLimit(hwnd)				(int)SNDMSG((hwnd), GEM_GETNUMLIMIT, 0, 0)
#define GradEdit_SetNumLimit(hwnd, n)			SNDMSG((hwnd), GEM_SETNUMLIMIT, (WPARAM)(n), 0)
#define GradEdit_ReverseMap(hwnd)				(int)SNDMSG((hwnd), GEM_REVERSEMAP, 0, 0)


///////////////////////////////////////////////////////
// Signal control command message
//

#define SGM_SETSIGNAL			(WM_USER + 0x2c0)
#define SGM_GETSIGNAL			(WM_USER + 0x2c1)
#define SGM_SETIMAGELIST		(WM_USER + 0x2c2)

#define Signal_SetBusy(hwnd, n)					SNDMSG((hwnd), SGM_SETSIGNAL, (WPARAM)(n), 0)
#define Signal_GetBusy(hwnd)					(BOOL)SNDMSG((hwnd), SGM_GETSIGNAL, 0, 0)
#define Signal_SetImageList(hwnd, him)			(BOOL)SNDMSG((hwnd), SGM_SETIMAGELIST, 0, (LPARAM)(him))


//////////////////////////////////////////////////
// FolderTree
// window styles
#define FTWS_NOCONTEXTMENU		0x00000001
#define FTWS_FILESYSTEMONLY		0x00000002
#define FTWS_ACCEPTFILES		0x00000004
#define FTWS_FULLROWSELECT		0x00000008
#define FTWS_NOBOLDSELECT		0x00000010				// 選択項目を強調表示しない
#define FTWS_NOFUSE				0x00000020				// 導火線を表示しない

// notify codes
#define FTWN_SELCHANGED			1
#define FTWN_TABPRESSED			2
#define FTWN_QUERYHIDDEN		3						// 隠し属性のフォルダの取り扱いを問い合わせる
														// 戻り値 0=すべて表示 1=すべて非表示 2=隠しシステムファイルを非表示
enum {
	FTH_DEFAULT = 0,
	FTH_HIDDEN = 1,
	FTH_SYSHIDDEN = 2,
};

// window messages
#define FTWM_SETCURSEL			(WM_USER + 0x2d0)
#define FTWM_SELECTPARENT		(WM_USER + 0x2d1)
#define FTWM_SELECTCHILD		(WM_USER + 0x2d2)
#define FTWM_GETCURSEL			(WM_USER + 0x2d3)
#define FTWM_REFRESH			(WM_USER + 0x2d4)
#define FTWM_SETROOT			(WM_USER + 0x2d5)
#define FTWM_SETCOLORS			(WM_USER + 0x2d6)
#define FTWM_SETOWNER			(WM_USER + 0x2d7)
#define FTWM_QUERYDESCENDANT	(WM_USER + 0x2d8)
#define FTWM_GETTREECTRLSTYLE	(WM_USER + 0x2d9)
#define FTWM_SETTREECTRLSTYLE	(WM_USER + 0x2da)

enum {
	FT_BYPATH = 0,
	FT_BYIDLIST,
	FT_BYCSIDL,
	FT_BYFOLDERID
};

//////////////////////////////////////////////////
// IDListPick control command message
//

#define IPM_GETIDLIST			(WM_USER + 0x2e0)
#define IPM_SETIDLIST			(WM_USER + 0x2e1)

#define IDListPicker_GetIDList(hwnd)			(LPITEMIDLIST)SNDMSG((hwnd), IPM_GETIDLIST, 0, 0)
#define IDListPicker_SetIDList(hwnd, pidl)		SNDMSG((hwnd), IPM_SETIDLIST, 0, (LPARAM)(pidl))


//////////////////////////////////////////////////
// LSCCheckTreeView control command message
//

// comtrol style

#define CTS_NOCLIENTEDGE	0x0001

// message code
#define CTM_BASE				(WM_USER + 0x2f0)
#define CTM_ADDITEM				(CTM_BASE + 0x00)
#define CTM_GETITEMSTATE		(CTM_BASE + 0x01)
#define CTM_SETITEMSTATE		(CTM_BASE + 0x02)
#define CTM_GETITEMTEXT			(CTM_BASE + 0x03)
#define CTM_SETITEMTEXT			(CTM_BASE + 0x04)
#define CTM_SELECTITEM			(CTM_BASE + 0x05)
#define CTM_GETSPACING			(CTM_BASE + 0x06)
#define CTM_SETSPACING			(CTM_BASE + 0x07)

// item mask flags
#define CTIF_TEXT				0x0001
#define CTIF_GROUP				0x0004
#define CTIF_STATE				0x0008

// item state flags
#define CTIS_CHECKED			0x00000001
#define CTIS_DISABLED			0x00000010
#define CTIS_RADIO				0x00000100
#define CTIS_EDIT				0x00000200
#define CTIS_EXPANDED			0x10000000
#define CTIS_GROUP				0x80000000

// notify code
#define CTN_CHECKCHANGED		1
#define CTN_GETDISPINFO			2
#define CTN_EDITCHANGED			3

// GROUP definition
#define CTGROUP_CURRENT			-1
#define CTGROUP_ROOT			-2
#define CTGROUP_TOP				-3
#define CTGROUP_PARENT			-4

typedef struct tagCTITEM {
	int id;
	int idGroup;
	UINT state;
	UINT stateMask;
	LPTSTR pszText;
} CTITEM;

typedef struct tagNMCTITEM {
	NMHDR hdr;
	CTITEM item;
} NMCTITEM;


//////////////////////////////////////////////////
// DrawListView control command message

/*
 * MESSAGES
 */
#define DLM_BASE                 (WM_USER + 0x400)
#define DLM_ADDITEM              (DLM_BASE + 0x00)
#define DLM_INSERTITEM           (DLM_BASE + 0x01)
#define DLM_DELETEITEM           (DLM_BASE + 0x02)
#define DLM_SELITEMRANGEEX       (DLM_BASE + 0x03)
#define DLM_RESETCONTENT         (DLM_BASE + 0x04)
#define DLM_DELETEALLITEMS       DLM_RESETCONTENT
#define DLM_SETSEL               (DLM_BASE + 0x05)
#define DLM_SETCURSEL            (DLM_BASE + 0x06)
#define DLM_GETSEL               (DLM_BASE + 0x07)
#define DLM_GETCURSEL            (DLM_BASE + 0x08)
#define DLM_ISITEMVISIBLE        (DLM_BASE + 0x09)
#define DLM_GETCOUNT             (DLM_BASE + 0x0B)
#define DLM_GETTOPINDEX          (DLM_BASE + 0x0E)
#define DLM_GETSELCOUNT          (DLM_BASE + 0x10)
#define DLM_GETSELITEMS          (DLM_BASE + 0x11)
#define DLM_SETTOPINDEX          (DLM_BASE + 0x17)
#define DLM_GETITEMRECT          (DLM_BASE + 0x18)
#define DLM_GETITEMDATA          (DLM_BASE + 0x19)
#define DLM_SETITEMDATA          (DLM_BASE + 0x1A)
#define DLM_SELITEMRANGE         (DLM_BASE + 0x1B)
#define DLM_SETANCHORINDEX       (DLM_BASE + 0x1C)
#define DLM_GETANCHORINDEX       (DLM_BASE + 0x1D)
#define DLM_SETCARETINDEX        (DLM_BASE + 0x1E)
#define DLM_GETCARETINDEX        (DLM_BASE + 0x1F)
#define DLM_SETHORIZONTALEXTENT  (DLM_BASE + 0x20)
#define DLM_GETHORIZONTALEXTENT  (DLM_BASE + 0x21)
#define DLM_ITEMFROMPOINT        (DLM_BASE + 0x22)
#define DLM_GETITEMSIZE          (DLM_BASE + 0x23)
#define DLM_SETITEMSIZE          (DLM_BASE + 0x24)
#define DLM_GETNEXTITEM          (DLM_BASE + 0x25)
#define DLM_ENSUREVISIBLE        (DLM_BASE + 0x26)
#define DLM_REDRAWITEMS          (DLM_BASE + 0x27)
#define DLM_GETSPACING           (DLM_BASE + 0x28)
#define DLM_SETSPACING           (DLM_BASE + 0x29)
#define DLM_SETOWNER             (DLM_BASE + 0x2a)
#define DLM_SETEXTENDEDSTYLE     (DLM_BASE + 0x2b)
#define DLM_GETEXTENDEDSTYLE     (DLM_BASE + 0x2c)
#define DLM_GETBKCOLOR           (DLM_BASE + 0x2d)
#define DLM_SETBKCOLOR           (DLM_BASE + 0x2e)
#define DLM_SETCOUNT             (DLM_BASE + 0x2f)
#define DLM_GETANCHORPOINT       (DLM_BASE + 0x30)
#define DLM_GETDROPTARGETINDEX   (DLM_BASE + 0x31)
#define DLM_SETDROPTARGETINDEX   (DLM_BASE + 0x32)
#define DLM_SETLISTSTYLE         (DLM_BASE + 0x33)
#define DLM_GETLISTSTYLE         (DLM_BASE + 0x34)
#define DLM_SETHEADER            (DLM_BASE + 0x35)
#define DLM_GETHEADER            (DLM_BASE + 0x36)
#define DLM_LAYOUT               (DLM_BASE + 0x37)
#define DLM_SETINFOTIP           (DLM_BASE + 0x38)
#define DLM_REDRAWSELITEMS       (DLM_BASE + 0x39)
#define DLM_SETMARK              (DLM_BASE + 0x3a)
#define DLM_GETMARK              (DLM_BASE + 0x3b)
#define DLM_GETMARKCOUNT         (DLM_BASE + 0x3c)
#define DLM_GETMARKITEMS         (DLM_BASE + 0x3d)
#define DLM_REDRAWMARKITEMS      (DLM_BASE + 0x3e)
#define DLM_MARKITEMRANGE        (DLM_BASE + 0x3f)

/*
 * STYLES
 */
#define DLS_NOTIFY					0x0001L
#define DLS_SORT					0x0002L
#define DLS_SINGLESEL				0x0004L
#define DLS_NODATA					0x2000L

/*
 * EXTENDED STYLES
 */
#define DLS_EX_TOOLTIP				0x0001L				// default off
#define DLS_EX_CONTEXTMENU			0x0002L				// default on
#define DLS_EX_ONECLICKACTIVATE		0x0004L				// one-click activate
#define DLS_EX_HOTTRACK				0x0008L				// hot-tracking
#define DLS_EX_WRAPCURSOR			0x0010L				// wraparound cursor
#define DLS_EX_HEADERINALLVIEWS		0x0020L				// カラムヘッダを常時表示

/*
 * LIST STYLES
 */
enum {
	DLS_STYLE_ICON = 0,
	DLS_STYLE_LIST,
	DLS_STYLE_REPORT,
	DLS_STYLE_LAST,
};

/*
 * NOTIFICATION CODES
 */
#define DLN_ERRSPACE				(-2)
#define DLN_SELCHANGE				1
#define DLN_DBLCLK					2
#define DLN_SELCANCEL				3
#define DLN_SETFOCUS				4
#define DLN_KILLFOCUS				5
#define DLN_BEGINDRAG				6
#define DLN_BEGINRDRAG				7
#define DLN_DELETEITEM				8
#define DLN_CLICK					9

/*
 * 
 */
#define DLNI_ALL                 0x0000
#define DLNI_FOCUSED             0x0001
#define DLNI_SELECTED            0x0002
#define DLNI_CUT                 0x0004
#define DLNI_DROPHILITED         0x0008
#define DLNI_MARKED              0x0010
#define DLNI_ABOVE               0x0100
#define DLNI_BELOW               0x0200
#define DLNI_TOLEFT              0x0400
#define DLNI_TORIGHT             0x0800

#pragma pack(1)

typedef struct tagNMDLVIEW {
    NMHDR hdr;
	INT_PTR iItem;
    UINT uNewState;
    UINT uOldState;
    UINT uChanged;
    POINT ptAction;
    LPARAM lParam;
} NMDLVIEW;

typedef struct tagNMDLGETINFOTIPA {
    NMHDR hdr;
    DWORD dwFlags;
    LPSTR pszText;
    int cchTextMax;
    INT_PTR iItem;
    LPARAM lParam;
} NMDLGETINFOTIPA;

typedef struct tagNMDLGETINFOTIPW {
    NMHDR hdr;
    DWORD dwFlags;
    LPWSTR pszText;
    int cchTextMax;
    INT_PTR iItem;
    LPARAM lParam;
} NMDLGETINFOTIPW;

typedef struct tagNMDLCLICK {
    NMHDR hdr;
	INT_PTR iItem;
	UINT typeClick;
    POINT ptAction;
} NMDLCLICK;

typedef struct tagNMDLSETINFOTIP {
	UINT cbSize;
	DWORD dwFlags;
	LPWSTR pszText;
	int iItem;
	int iSubItem;
	HBITMAP hbmp;		// large preview in tooltip
} NMDLSETINFOTIP, *PNMDLSETINFOTIP;

#define DLSIT_BITMAP			0x0001

#pragma pack()

// NMLVGETINFOTIPA.dwFlag values

#define DLGIT_UNFOLDED				0x0001

#define DLN_FIRST					200
#define DLN_GETINFOTIPA				(DLN_FIRST+ 0)
#define DLN_GETINFOTIPW				(DLN_FIRST+ 1)
#define DLN_ITEMCHANGED				(DLN_FIRST+ 2)

#ifdef UNICODE
#define DLN_GETINFOTIP				DLN_GETINFOTIPW
#define NMDLGETINFOTIP				NMLVGETINFOTIPW
#else
#define DLN_GETINFOTIP				DLN_GETINFOTIPA
#define NMDLGETINFOTIP				NMLVGETINFOTIPA
#endif

#define ODS_MARKED					ODS_CHECKED
#define ODS_ODDPOSITION				0x8000


//////////////////////////////////////////////////
// Splitter

/*
 * MESSAGES
 */
#define SPM_BASE                 (WM_USER + 0x480)
#define SPM_SETSPLITTERRECT      (SPM_BASE + 0x00)	// wParam = bUpdate, lParam = LPRECT
#define SPM_GETSPLITTERRECT      (SPM_BASE + 0x01)	// wParam = NA, lParam = LPRECT
#define SPM_SETSPLITTERPOS       (SPM_BASE + 0x02)	// wParam = bUpdate, lParam = xyPos
#define SPM_GETSPLITTERPOS       (SPM_BASE + 0x03)	// wParam = NA, lParam = NA
#define SPM_SETSINGLEPANEMODE    (SPM_BASE + 0x04)	// wParam = paneMode, lParam = NA
#define SPM_GETSINGLEPANEMODE    (SPM_BASE + 0x05)	// wParam = NA, lParam = NA
#define SPM_GETEXTENDEDSTYLE     (SPM_BASE + 0x06)	// wParam = NA, lParam = NA
#define SPM_SETEXTENDEDSTYLE     (SPM_BASE + 0x07)	// wParam = styleMask, lParam = style
#define SPM_SETPANES             (SPM_BASE + 0x08)	// wParam = bUpdate, lParam = struct SPLITTERPANEINFO*
#define SPM_SETSPLITTERPANE      (SPM_BASE + 0x09)	// wParam = MAKELONG(idPane, bUpdate), lParam = hwndPane
#define SPM_GETSPLITTERPANE      (SPM_BASE + 0x0a)	// wParam = idPane, lParam = NA
#define SPM_SETACTIVEPANE        (SPM_BASE + 0x0b)	// wParam = idPane, lParam = NA
#define SPM_GETACTIVEPANE        (SPM_BASE + 0x0c)	// wParam = NA, lParam = NA
#define SPM_ACTIVATENEXTPANE     (SPM_BASE + 0x0d)	// wParam = bNext, lParam = NA
#define SPM_SETDEFAULTACTIVEPANE (SPM_BASE + 0x0e)	// wParam != 0 ? lParam = idPane : lParam = hwndPane
#define SPM_GETDEFAULTACTIVEPANE (SPM_BASE + 0x0f)	// wParam = NA, lParam = NA
#define SPM_SETSPLITTERMINSIZE   (SPM_BASE + 0x10)	// wParam = cxyMin, lParam = NA
#define SPM_GETSPLITTERMINSIZE   (SPM_BASE + 0x11)	// wParam = NA, lParam = NA
#define SPM_SETSPLITTERDRAGMODE  (SPM_BASE + 0x12)	// wParam = nFullDrag, lParam = NA
#define SPM_GETSPLITTERDRAGMODE  (SPM_BASE + 0x13)	// wParam = NA, lParam = NA


/*
 * STYLES
 */
#define SPS_SPLITTERVERTICAL      0x0000L
#define SPS_SPLITTERHORIZONTAL    0x0001L


typedef struct tagSPLITTERPANEINFO {
	HWND hWndLeftTop;
	HWND hWndRightBottom;
} SPLITTERPANEINFO;

#ifndef __ATLSPLIT_H__
	// Splitter panes constants
	#define SPLIT_PANE_LEFT			0
	#define SPLIT_PANE_RIGHT		1
	#define SPLIT_PANE_TOP			SPLIT_PANE_LEFT
	#define SPLIT_PANE_BOTTOM		SPLIT_PANE_RIGHT
	#define SPLIT_PANE_NONE			-1

	// Splitter extended styles
	#define SPLIT_PROPORTIONAL		0x00000001
	#define SPLIT_NONINTERACTIVE	0x00000002
	#define SPLIT_RIGHTALIGNED		0x00000004
	#define SPLIT_BOTTOMALIGNED		SPLIT_RIGHTALIGNED
#endif


///////////////////////////////////////////////////////
// Tab controls

#ifndef TCM_LAST
#define TCM_LAST					(TCM_FIRST + 0xff)
#endif

#define TCM_SETPARENT				(TCM_LAST - 1)
#define TCM_GETPANECOLOR			(TCM_LAST - 2)
#define TCM_SETPANECOLOR			(TCM_LAST - 3)
#define TCM_CREATEDRAGIMAGE			(TCM_LAST - 4)
#define TCM_GETBARMETRICS			(TCM_LAST - 5)
#define TCM_SETBARMETRICS			(TCM_LAST - 6)
#define TCM_GETBKGNDIMAGE			(TCM_LAST - 7)
#define TCM_SETBKGNDIMAGE			(TCM_LAST - 8)
#define TCM_TABMOVETO				(TCM_LAST - 9)
#define TCM_TABEXCHANGE				(TCM_LAST - 10)

#define TCS_EX_CLOSEBUTTON			0x01000000			// extended-style クローズボタン
#define TCS_EX_BOTTOMEDGE			0x02000000
#define TCS_EX_DRAGDROP				0x04000000
#define TCS_EX_HOVERSELECT			0x08000000
#define TCS_EX_CLOSEBUTTON_RA		0x00100000			// クローズボタン右寄せ

#define TCN_BEGINDRAG				(TCN_LAST + 1)
#define TCN_ENDDRAG					(TCN_LAST + 2)
#define TCN_GETINFOTIPA				(TCN_LAST + 3)
#define TCN_GETINFOTIPW				(TCN_LAST + 4)
#define TCN_CLOSEBUTTONCLICKED		(TCN_LAST + 5)
#define TCN_DELETEITEM				(TCN_LAST + 6)
#define TCN_ADDBUTTONCLICKED		(TCN_LAST + 7)

typedef struct tagNMTCGETINFOTIPA {
    NMHDR hdr;
    LPSTR pszText;
    int cchTextMax;
    int iItem;
    LPARAM lParam;
} NMTCGETINFOTIPA;

typedef struct tagNMTCGETINFOTIPW {
    NMHDR hdr;
    LPWSTR pszText;
    int cchTextMax;
    int iItem;
    LPARAM lParam;
} NMTCGETINFOTIPW;

typedef struct tagNMTAB {
    NMHDR   hdr;
    int     iItem;
    LPARAM  lParam;
} NMTAB;

typedef struct tagNMTCSELCHANGE {
    NMHDR   hdr;
    int     iItemOld;
    int     iItemNew;
} NMTCSELCHANGE;

typedef struct tagNMTCMETRICS {
    NMHDR   hdr;
	int cxBarPaddingLeft;
	int cxBarPaddingRight;
} NMTCMETRICS;


#ifdef UNICODE
#define TCN_GETINFOTIP				TCN_GETINFOTIPW
#define NMTCGETINFOTIP				NMTCGETINFOTIPW
#else
#define TCN_GETINFOTIP				TCN_GETINFOTIPA
#define NMTCGETINFOTIP				NMTCGETINFOTIPA
#endif

//#define TCHT_NOWHERE				0x0001
//#define TCHT_ONITEMICON			0x0002
//#define TCHT_ONITEMLABEL			0x0004
//#define TCHT_ONITEM				(TCHT_ONITEMICON | TCHT_ONITEMLABEL)
#define TCHT_ONITEMCLOSE			0x8000				// 「閉じる」ボタン上
#define TCHT_ONADDBUTTON			0x4000				// 追加ボタン上

// item states
#define TCIS_MARKED					0x8000
#define TCIS_LOCKED					0x4000


//////////////////////////////////////////////////
// SearchPath

/*
 * MESSAGES
 */
#define SEM_OK						(0)
#define SEM_ERR						(-1)

#define SEM_BASE					(WM_USER + 0x490)
#define SEM_ADDSTRING				(SEM_BASE + 0x00)
#define SEM_DELETESTRING			(SEM_BASE + 0x01)
#define SEM_RESETCONTENT			(SEM_BASE + 0x02)
#define SEM_SETCURSEL				(SEM_BASE + 0x03)
#define SEM_GETCURSEL				(SEM_BASE + 0x04)
#define SEM_GETCOUNT				(SEM_BASE + 0x05)
#define SEM_SELECTSTRING			(SEM_BASE + 0x06)
#define SEM_FINDSTRING				(SEM_BASE + 0x07)
#define SEM_LIMITTEXT				(SEM_BASE + 0x08)
#define SEM_SHOWDROPDOWN			(SEM_BASE + 0x09)
#define SEM_INSERTSTRING			(SEM_BASE + 0x0a)
#define SEM_GETEDITSEL				(SEM_BASE + 0x0b)
#define SEM_SETEDITSEL				(SEM_BASE + 0x0c)
#define SEM_SETREADONLY				(SEM_BASE + 0x0d)
#define SEM_SETIMAGELIST			(SEM_BASE + 0x0e)
#define SEM_GETIMAGELIST			(SEM_BASE + 0x0f)
#define SEM_GETITEMIMAGE			(SEM_BASE + 0x10)
#define SEM_SETITEMIMAGE			(SEM_BASE + 0x11)
#define SEM_SELECTSTRINGEXACT		(SEM_BASE + 0x12)
#define SEM_FINDSTRINGEXACT			(SEM_BASE + 0x13)

/*
 * STYLES
 */
#define SES_READONLY				0x0001L
#define SES_DROPDOWN				0x0002L

/*
 * NOTIFY
 */
#define SEN_FIRST					300
#define SEN_GETTEXTA				(SEN_FIRST + 1)
#define SEN_GETTEXTW				(SEN_FIRST + 2)
#define SEN_DROPDOWNA				(SEN_FIRST + 3)
#define SEN_DROPDOWNW				(SEN_FIRST + 4)
#define SEN_EDITUPDATE				(SEN_FIRST + 5)					// NMHDRのみ
#define SEN_EDITCHANGE				(SEN_FIRST + 6)					// NMHDRのみ

typedef struct tagNMSEGETTEXTA {
    NMHDR hdr;
    LPSTR pszText;
    int cchTextMax;
    LPARAM lParam;
} NMSEGETTEXTA;

typedef struct tagNMSEGETTEXTW {
    NMHDR hdr;
    LPWSTR pszText;
    int cchTextMax;
    LPARAM lParam;
} NMSEGETTEXTW;

typedef struct tagNMSEDROPDOWNA {
    NMHDR   hdr;
    LPSTR   pszText;
    int     cchText;
    RECT    rcButton;
} NMSEDROPDOWNA;

typedef struct tagNMSEDROPDOWNW {
    NMHDR   hdr;
    LPWSTR  pszText;
    int     cchText;
    RECT    rcButton;
} NMSEDROPDOWNW;


#ifdef UNICODE

#define SEN_GETTEXT					SEN_GETTEXTW
#define SEN_DROPDOWN				SEN_DROPDOWNW
#define NMSEGETTEXT					NMSEGETTEXTW
#define NMSEDROPDOWN				NMSEDROPDOWNW

#else

#define SEN_GETTEXT					SEN_GETTEXTA
#define SEN_DROPDOWN				SEN_DROPDOWNA
#define NMSEGETTEXT					NMSEGETTEXTA
#define NMSEDROPDOWN				NMSEDROPDOWNA

#endif


///////////////////////////////////////////////////////
// VersionInformation for ver.dll
//
enum {
	KV_COMMENTS,
	KV_COMPANYNAME,
	KV_FILEDESCRIPTION,
	KV_FILEVERSION,
	KV_INTERNALNAME,
	KV_LEGALCOPYRIGHT,
	KV_LEGALTRADEMARKS,
	KV_ORIGINALFILENAME,
	KV_PRIVATEBUILD,
	KV_PRODUCTNAME,
	KV_PRODUCTVERSION,
	KV_SPECIALBUILD
};


///////////////////////////////////////////////////////
// OS TYPE
//
enum {
	OST_32S,
	OST_NT350,
	OST_NT351,
	OST_95,
	OST_NT400
};


///////////////////////////////////////////////////////
// Reg Default Icon Type
//
enum {
	DEFICON_NONE,
	DEFICON_MYSELF,
	DEFICON_MAPPABLE,
	DEFICON_HANDLER
};

///////////////////////////////////////////////////////
// AboutDlgFn
//

typedef struct tagAPPABOUTINFO {
	HINSTANCE hInst;
	TCHAR szUser[MAX_PATH];
	TCHAR szCompany[MAX_PATH];
} APPABOUTINFO, *PAPPABOUTINFO, FAR *LPAPPABOUTINFO;

#define MAGIC_APPABOUTINFO_V2	(HINSTANCE)(IntToPtr(2))

typedef struct tagAPPABOUTINFO_V2 {
	HINSTANCE magic;
	HINSTANCE hInst;
	TCHAR szUser[MAX_PATH];
	TCHAR szCompany[MAX_PATH];
	TCHAR szUrl[MAX_PATH];
} APPABOUTINFO_V2, *PAPPABOUTINFO_V2, FAR *LPAPPABOUTINFO_V2;

typedef struct tagKMAPPINFO {
	HINSTANCE hInst;
	TCHAR szAppInfo[MAX_PATH * 2];
	DWORD code;
} KMAPPINFO;


///////////////////////////////////////////////////////
// TOOLBARCUSTOMIZE
//

#pragma pack(1)

typedef struct tagTBCDATA {
	HWND hctl;
	HINSTANCE hInst;
	UINT id;
	SIZE sizeButton;
	SIZE sizeBitmap;
} TBCDATA, *LPTBCDATA;

typedef struct _LSCTOOLBARDATA {
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]
	inline WORD* items() { return (WORD*)(this + 1); }
	inline size_t dataSize() { return sizeof(struct _LSCTOOLBARDATA) + sizeof(WORD) * (size_t)wItemCount; }
} LSCTOOLBARDATA;

#pragma pack()



///////////////////////////////////////////////////////
// inline macros
//

#ifdef _INC_SHLWAPI

_inline BOOL PathGetParent(LPCWSTR src, LPWSTR dst)
	{ lstrcpyW(dst, src);
	  return PathRemoveFileSpecW(dst); }

#endif

///////////////////////////////////////////////////////
// API entries
//
#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

WORD WINAPI lccGetVersion(void);
void WINAPI lccSetVersion(WORD ver);
int WINAPI lccInsertControls(HWND hwnd, LPRECT prcArea, HWND hwndAfter, HINSTANCE hResoursce, UINT idTemplate, HWND FAR *lpHwnd);
void WINAPI lccDrawBitmap(HDC hdc, HBITMAP hbm, int x, int y, int bx, int by);
void WINAPI lccScreenToClientRect(HWND hwnd, LPRECT lprc);
void WINAPI lccClientToScreenRect(HWND hwnd, LPRECT lprc);
void WINAPI lccCenterWindow(HWND hwnd, HWND hwndParent);

HGLOBAL WINAPI lccVerOpen(LPCWSTR lpszName);
void WINAPI lccVerClose(HANDLE hMemVer);
BOOL WINAPI lccVerGetVersion(HANDLE hMemVer, int nVersionID, LPWSTR lpszVersion);
DWORD WINAPI lccVerGetFileOSFlags(HANDLE hMemVer);
BOOL WINAPI lccVerGetFileOS(HANDLE hMemVer, LPWSTR lpszFileOS);
BOOL WINAPI lccVerGetFileType(HANDLE hMemVer, LPWSTR lpszType);
BOOL WINAPI lccVerQueryValue(HANDLE hMemVer, int nValueID, LPWSTR lpszValue);
BOOL WINAPI lccVerQueryValueEx(HANDLE hMemVer, LPCTSTR pszValueName, LPTSTR pszText, int cchMax);
BOOL WINAPI lccVerGetVersionEx(LPCWSTR pszName, LPDWORD pdwMajor, LPDWORD pdwMinor);
void WINAPI lccPopView(HWND hwnd, LPCWSTR pCaption, LPCWSTR pText, DWORD flags);
void WINAPI lccPopViewEx(HWND hwnd, LPCTSTR pCaption, LPCTSTR pText, HICON hIcon, DWORD style);
//@ int WINAPI lccAssignDrive(HWND hwnd, LPWSTR lpLocal, LPWSTR lpRemote, BOOL fProf);
//@ int WINAPI lccGetLocalDrive(LPWSTR lpRemote);
//@ BOOL WINAPI lccLogonNetwork(HWND hwndOwner, LPWSTR lpLocal, LPWSTR lpRemote, DWORD dwConn);
//@ BOOL WINAPI lccRestoreConnection(int nDrive);
//@ BOOL WINAPI lccValidateConnection(HWND hwnd, LPCWSTR pszFolder);
BOOL WINAPI lccGetNetworkPath(int nDrive, LPWSTR pszName);

void WINAPI lccAddChar(LPWSTR pszPathName, TCHAR nChar);
void WINAPI lccReplaceChar(LPWSTR pszText, TCHAR nSrcChar, TCHAR nDstChar);
BOOL WINAPI lccGetValues(LPWSTR psz, LPINT v, int cnt);
BOOL WINAPI lccIsDigitStr(LPCWSTR psz);
int WINAPI lccIsPathInclude(LPCWSTR pszSrc, LPCWSTR pszDst);
LPWSTR WINAPI lccNumForm(DWORD lNum, LPWSTR pszFormat);
LPWSTR WINAPI lccFormatByteSize64(LONGLONG llNum, LPWSTR pszFormat, UINT cchMax);
LPWSTR WINAPI lccFormatFileSize(DWORD lNum, LPWSTR pszFormat);
BOOL WINAPI lccSetFileDate(LPCWSTR pszName, LPFILETIME lpftCreate, LPFILETIME lpftUpdate);
int WINAPI lccComparePath(LPCWSTR pszSrc, LPCWSTR pszDst);
HWND WINAPI lccFindWindowByClassName(LPCWSTR pszClassName);
BOOL WINAPI lccUniqueFilter(LPWSTR pszFilter);
void WINAPI lccFormatFileTime(LPWSTR pszFormat, FILETIME& ft);
BOOL WINAPI lccGetBinaryType(LPCWSTR lpName, LPLONG lpType);
void WINAPI lccFileAttrToStr(DWORD dwAttr, LPWSTR lpBuf);
void WINAPI lccPatB(HDC hdc, int x, int y, int dx, int dy, COLORREF rgb);
BOOL WINAPI lccPresent(LPCWSTR pList, LPCWSTR pToken);
BOOL WINAPI lccSetStringToClipboard(HWND hwnd, LPCWSTR pszText);
BOOL WINAPI lccWriteTextToFile(HWND hwnd, LPCWSTR lpFile, LPCWSTR lpText);

BOOL WINAPI lccRegCheckShellVerb(LPCWSTR pszFile);
BOOL WINAPI lccRegDeleteKeyAll(HKEY hKeyRoot, LPCWSTR pszSubKey);
BOOL WINAPI lccRegDeleteValueAll(HKEY hRootKey, LPCWSTR pszSubKey);
BOOL WINAPI lccRegDeleteValue(HKEY hRootKey, LPCWSTR pszSubKey, LPCWSTR pszValue);
int WINAPI lccRegGetBinary(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, LPBYTE pData, int nLen);
//@ LPBYTE WINAPI lccRegGetBinaryEx(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, LPINT pnLen);
BOOL WINAPI lccRegSetBinary(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, LPBYTE pData, int nLen);
int WINAPI lccRegGetString(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, LPWSTR pStr, int nLen);
int WINAPI lccRegGetStrJum(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, char* pStr, int nLen);
BOOL WINAPI lccRegSetString(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, LPCWSTR pStr);
BOOL WINAPI lccRegSetStrJum(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, const char* pStr);
BOOL WINAPI lccRegGetInt(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, LPINT pData);
BOOL WINAPI lccRegSetInt(LPCWSTR pApp, LPCWSTR pSec, LPCWSTR pKey, int nData);
int WINAPI lccRegGetEnumKey(LPCWSTR pApp, LPCWSTR pSec, LPWSTR pKey, DWORD dwRegType, int nLen);

BOOL WINAPI lccRegGetLibraryPath(LPWSTR pszPath);
BOOL WINAPI lccRegGetAppPath(LPCTSTR pszAppName, LPTSTR pszAppPath, int cchMax);
BOOL WINAPI lccRegGetPerceivedType(LPCTSTR pszFile, LPTSTR typeStr, int cchMax);

BOOL WINAPI SetBrowseButton(HWND hwnd);

BOOL WINAPI lccExecApp(HWND hwnd, LPCWSTR pszCmdLine, LPCWSTR pszDir, UINT nExecMode);
//@ void* WINAPI lccMemAlloc(DWORD dwSize);
//@ void WINAPI lccMemFree(void* lpMem);
//@ void* WINAPI lccMemRealloc(void* pMem, DWORD dwSize);
void WINAPI lccGetErrorString(int nErrorCode, LPWSTR pszText);
int WINAPI lccErrorBox(HWND hwnd, int nErrorCode, LPCWSTR pszTitle, LPCWSTR pszText, int nTypeAction);
BOOL WINAPI lccMakeTree(LPCWSTR pszPath);
BOOL WINAPI lccPathBuild(LPCWSTR pszSrc, LPCWSTR pszBase, LPWSTR pszDst);
HICON WINAPI lccCreateUpdirIcon();
//@ int WINAPI lccCustomizeToolBar(HWND hParent, LPTBCDATA ptbc);
BOOL WINAPI lccBeginTrack(HWND hwnd, POINT pt);
BOOL WINAPI lccBeginTrack2(HWND hwnd, POINT pt, DWORD waittime);
HRESULT WINAPI lccGetShell32Version(LPDWORD pdwMajor, LPDWORD pdwMinor);
HRESULT WINAPI lccDllGetVersion(LPCWSTR pszDll, LPDWORD pdwMajor, LPDWORD pdwMinor);
BOOL WINAPI lccGetTempFileName(LPCWSTR pszPath, LPCWSTR pszPrefix, LPWSTR pszName);
//@ BOOL WINAPI lccIsPathShared(LPCWSTR pszPathName);
HGLOBAL WINAPI lccDuplicateMemoryHandle(HGLOBAL hmem, UINT flag);
//@ void WINAPI lccGetLongPathName(LPCWSTR pszShort, LPWSTR pszLong);
BOOL WINAPI lccGetFileCreateOwner(LPCWSTR pszFile, LPWSTR pszCreateOwner);
BOOL WINAPI lccGetSharedPointDisplayName(LPCWSTR pszSharedPoint, LPWSTR pszName);
BOOL WINAPI lccColorPicker(HWND hParent, POINT p, COLORREF* pcrColor, BOOL bGradation);
BOOL WINAPI lccColorPickerEx(HWND hParent, POINT p, COLORREF* pcrColor, LPCWSTR pszPalFile);
BOOL WINAPI lccSelectCustomColor(HWND hOwner, COLORREF rgb, COLORREF* rgbNew);
BOOL WINAPI lccAddTextToListbox(HWND hlist, LPCWSTR pText);
void WINAPI lccAddPathEnvironment(LPCTSTR pszAdd);

void WINAPI lccDrawEmbossedImage(HDC hdc, HIMAGELIST hil, int iImage, POINT pt);
//@ void WINAPI lccDrawEmbossedImageEx(HDC hdc, HIMAGELIST hil, int iImage, POINT pt, BOOL bAlphaImage);
void WINAPI lccDrawButtonMenu(int menuStyle, DRAWITEMSTRUCT* pdi, LPCWSTR pText, HIMAGELIST hImage, int iImage, SIZE sizeButton, UINT txflg);
//@ void WINAPI lccDrawButtonMenuEx(int menuStyle, DRAWITEMSTRUCT* pdi, LPCWSTR pText, HIMAGELIST hImage, int iImage, SIZE sizeButton, UINT txflg, BOOL bAlphaImage);
void WINAPI lccDrawCustomMenu(DRAWITEMSTRUCT* pdi, LPCTSTR pText, HIMAGELIST hImage, int iImage, UINT txflg);
void WINAPI lccDrawSelectionFrame(HDC hdc, RECT* prc, BOOL bActive, int alpha);
void WINAPI lccDrawSelectionFrameEx(HDC hdc, RECT* prc, COLORREF* pclrTable, int alpha);
void WINAPI lccDrawSelectionRoundFrame(HDC hdc, RECT* prc, BOOL bActive, int alpha);
void WINAPI lccDrawSelectionRoundFrameEx(HDC hdc, RECT* prc, COLORREF* pclrTable, int alpha);

BOOL WINAPI lccGetModuleFilePath(HINSTANCE hInst, LPWSTR pszPath);

//@ BOOL WINAPI lccDoContextMenu(HWND hwnd, LPCONTEXTMENU pcm, LPPOINT ppt);
void WINAPI lccCompactPath(HWND hwnd, LPCWSTR pszText);
DWORD WINAPI lccCirRed_ul(DWORD crc, BYTE* pdata, int len);
WORD WINAPI lccCirRed_us(WORD crc, BYTE* pdata, int len);
DWORD WINAPI lccCalcCrc32(BYTE* pdata, int len);
WORD WINAPI lccCalcCrc16(BYTE* pdata, int len);
BOOL WINAPI lccCalcMD5(const BYTE* pdata, unsigned len, BYTE* digest);
BOOL WINAPI lccCalcSHA1(const BYTE* dataptr, unsigned len, BYTE* digest);
BOOL WINAPI lccCalcSHA256(const BYTE* dataptr, unsigned len, BYTE* digest);

//@ void WINAPI lccRunView(HWND hParent, HWND hView);
void WINAPI lccShowAppAbout(HWND hwndOwner, APPABOUTINFO* pai);
int WINAPI lccShowAppInfo(HWND hwndOwner, KMAPPINFO* pai);
//@ int WINAPI lccDlgModal(HINSTANCE hInst, HWND hwnd, DLGPROC pfn, int id, void* pv);
//@ HWND WINAPI lccDlgModeless(HINSTANCE hInst, HWND hwnd, DLGPROC pfn, int id, void* pv);
void WINAPI lccCommandPrompt(LPCWSTR pszInitialPath);

HANDLE WINAPI lccMRUCreateList(HKEY hKey, LPCWSTR pszSubKey, DWORD dwMaxItem);
void WINAPI lccMRUFreeList(HANDLE hList);
int WINAPI lccMRUAddString(HANDLE hList, LPCWSTR pszString);
BOOL WINAPI lccMRUDelString(HANDLE hList, int nItemPos);
int WINAPI lccMRUEnumList(HANDLE hList, int nItemPos, LPVOID pBuffer, DWORD nSize);
int WINAPI lccMRUFindString(HANDLE hList, LPCWSTR pszString, LPINT pRegNum);
int WINAPI lccMRUGetCount(HANDLE hList);

BOOL WINAPI lccMemSaveToTempFile(HGLOBAL hmem, LPWSTR pszFile);
HGLOBAL WINAPI lccMemLoadFromTempFile(LPWSTR pszFile);
BOOL WINAPI lccRunDllEx(HWND hwnd, LPCTSTR pszDll, LPCTSTR pszFunc, LPCTSTR pszFile, BOOL bAsAdmin);
BOOL WINAPI lccRunDll(HWND hwnd, LPCWSTR pszDll, LPCWSTR pszFunc, LPCWSTR pszFile);
BOOL WINAPI lccDllExecute(HWND hwnd, LPCTSTR pszDll, LPCTSTR pszFunc, LPCTSTR pszFile, LPCTSTR pszIcon, BOOL bAsAdmin);

HBITMAP WINAPI lccLoadGrayedBitmap(HINSTANCE hInst, UINT id, COLORREF crMask);
HBITMAP WINAPI lccLoadGIFImage(HINSTANCE hInst, LPCWSTR typeResource, UINT idResource, COLORREF* pcrMask);
HIMAGELIST WINAPI lccLoadGIFImageList(HINSTANCE hInst, LPCWSTR typeResource, UINT idResource, int itemWidth);
int WINAPI lccDrawShadowText(HDC hdc, LPCWSTR pszText, UINT cch, const RECT *pRect, DWORD dwFlags, COLORREF crText, COLORREF crShadow, int ixOffset, int iyOffset);

BOOL WINAPI lccGetBitmapResourceInfo(HINSTANCE hInst, LPCWSTR pszResource, BITMAPINFOHEADER* pBitmapInfoHeader);
BOOL WINAPI lccIsAlphaBitmapResource(HINSTANCE hInst, LPCWSTR pszResource);
HIMAGELIST WINAPI lccCreateImageList(HINSTANCE hInst, UINT idResource, int cxIcon, COLORREF crMask);
const LSCTOOLBARDATA* WINAPI lccGetToobarResourceData(HINSTANCE hInst, LPCTSTR nameResource);
/* lccGetToobarResourceData: 戻り値のメモリは CoTaskMemFreeで開放すること */
HIMAGELIST WINAPI lccAddToolBarDisabledImageList(HWND, HINSTANCE, LPCTSTR);
HIMAGELIST WINAPI lccAddToolBarHotImageList(HWND, HINSTANCE, LPCTSTR);

HANDLE WINAPI lccRexInit(LPCTSTR pszExp);
BOOL WINAPI lccRexMatch(HANDLE h, LPCTSTR pszTarget, int* posStart, int* posEnd);
void WINAPI lccRexExit(HANDLE h);

BOOL WINAPI lccTipEdit(HWND hParent, LPRECT prc, LPTSTR pszText);
BOOL WINAPI lccDisableCharInput(HWND hwnd);
void WINAPI lccApplyAccelMenu(HMENU hMenu, ACCEL* pa, int numAccl);
BOOL WINAPI lccGetAccelString(UINT idCommand, ACCEL* pa, int numAccl, LPTSTR psz, int cchMax);

void WINAPI lccSetCoolStyle(BOOL bCool);
BOOL WINAPI lccGetCoolStyle();

void WINAPI lccMapRectToProgmanPane(RECT* prc);
void WINAPI lccUnmapRectFromProgmanPane(RECT* prc);

//////////////////////////
// visual-style theme supports

BOOL WINAPI lccThIsThemeActive();
BOOL WINAPI lccThIsAppThemed();

BOOL WINAPI lccTrackContextMenu(HWND hwnd, IContextMenu* pcm, POINT* ppt, UINT flag);
BOOL WINAPI lccContextMenuByIDList(HWND hwnd, LPITEMIDLIST pidl, LPPOINT ppt);
BOOL WINAPI lccContextMenuByFile(HWND hwnd, LPCWSTR pszFile, LPPOINT ppt);
BOOL WINAPI lccContextMenuByShellItem(HWND hwnd, IShellItem* psi, LPPOINT ppt);

//////////////////////////
// resource loader

HBITMAP WINAPI lccLoadImage(HINSTANCE hInst, LPCTSTR pszResourceType, LPCTSTR pszResourceName);
HIMAGELIST WINAPI lccLoadImageList(HINSTANCE hInst, LPCTSTR pszResourceType, LPCTSTR pszResourceName, int cxWidth, COLORREF clrMask);
HIMAGELIST WINAPI lccLoadAlphaImageList(HINSTANCE hInst, LPCTSTR pszResourceType, LPCTSTR pszResourceName, int cxWidth, BOOL bInverse);
HWND lccCreateToolBar(HWND hWndParent, HINSTANCE hInst, LPCTSTR nameResource, DWORD dwStyle, UINT idCtrl);
void WINAPI lccReplaceToolBarImageList(HWND hbar, HINSTANCE hInst, LPCTSTR nameResource);

// ---- とりあえずサポートしておかない
//	HIMAGELIST WINAPI lccLIMRoundMarker();
//	HIMAGELIST WINAPI lccLIMSmallRoundMarker();
//	HIMAGELIST WINAPI lccLIMHorzMarker();


#ifdef __cplusplus
}
#endif




#endif	// _LSCORE_H_

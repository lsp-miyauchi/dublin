// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "resource.h"
#include "common.h"
#include "shellnotify.h"

#define USE_VISUALSTYLE
#define USE_VISTA_AERO


////////////////////////////////////////////////////////////////

#define FRAMECLASS				_T("FV7Launch")

#define MAXDRIVE				26
#define DRVLX					32
#define DRVLY					25
#define SYSBOX_X				11
#define SYSBOX_Y				11
#define CX_MARGIN				3					// ボタンと左右のウィンドウ矩形の間隔
#define CY_MARGIN				4					// ボタンと上下のウィンドウ矩形の間隔
#define CXY_BLOCKSPACER			1					// ツールボタンのボタン間隔の値
#define EDGE_SPACER				1					// 
#define CXY_GRIPPER				3					// グリップの幅
#define TIMERID_STOPALPHA		1

enum {
	SYSBOX_INACTIVE = 0,
	SYSBOX_ACTIVE,
	SYSBOX_ONMENU
};

#define SYSBOXCOLOR_INACTIVE	RGB(128, 0, 0)
#define SYSBOXCOLOR_ACTIVE		RGB(255, 0, 0)
#define SYSBOXCOLOR_ONMENU		RGB(8, 255, 0)

/////////////////////////////////////////////////////////////////

#define STS_VERT				0x0001
#define STS_TOPMOST				0x0002
#define STS_SMALLICON   		0x0004
#define STS_HIDEWINDOW			0x0008
#define STS_SHADING				0x0010
#define STS_ALPHA				0x0020


////////////////////////////////////////////////////////////////
// drive infomation

#define PTS_PRESSED				0x00000001
#define PTS_HOT					0x00000002

class CDriveItem
{
public:
	CString m_strRoot;
	CString m_strDisplayName;
	int m_iImage;
	int m_iOverlayImage;
	DWORD m_stat;

public:
	CDriveItem()
		: m_iImage(-1)
		, m_iOverlayImage(-1)
		, m_stat(0)
	{
	}
	CDriveItem(LPCTSTR pszRoot)
		: m_iOverlayImage(-1)
		, m_stat(0)
	{
		m_strRoot = pszRoot;
		Parse();
	}

	~CDriveItem()
	{
	}
	inline bool IsPressed() { return ((m_stat & PTS_PRESSED) != 0); }
	inline bool IsHot() { return ((m_stat & PTS_HOT) != 0); }
	void CopyFrom(const CDriveItem& src)
	{
		if (&src != this) {
			m_strRoot = src.m_strRoot;
			m_strDisplayName = src.m_strDisplayName;
			m_iImage = src.m_iImage;
			m_iOverlayImage = src.m_iOverlayImage;
			m_stat = src.m_stat;
		}
	}
	inline CDriveItem& operator=(const CDriveItem& src)
	{
		CopyFrom(src);
		return *this;
	}
	inline BOOL operator ==(const CDriveItem& src) const
	{
		return (m_strRoot.CompareNoCase(src.m_strRoot) == 0);
	}
	void Parse()
	{
		SHFILEINFO sfi;
		SHGetFileInfo(m_strRoot, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME);
		m_iImage = sfi.iIcon;
		m_strDisplayName = sfi.szDisplayName;
	}

};

////////////////////////////////////////////////////////////////
//

class CMainFrame
			: public CFrameWindowImpl<CMainFrame>
			, public CUpdateUI<CMainFrame>
			, public CMessageFilter
			, public CIdleHandler
#ifdef USE_VISUALSTYLE
			, public CThemeImpl<CMainFrame>
#endif
{
public:
	CSimpleArray<CDriveItem*> m_drives;
	UINT m_nRM_NOTIFYICON;
	UINT m_nRM_LAUNCH;
	int m_style;
	int m_xBlock;
	int m_yBlock;
	int m_xDrive;
	int m_yDrive;
	BOOL m_bTrackMouseEvent;
	int m_hotIndex;
	UINT_PTR m_timerid;
	HFONT m_hMenuFont;
	HFONT m_hIconFont;
	HIMAGELIST m_himDrive;
	HIMAGELIST m_himDot;
	CShellNotify m_ShellNotify;
	gdiplus_using m_gu;
#ifdef USE_VISTA_AERO
	CDwmApiDll m_DwmManager;
#endif
	int m_nCmdShow;
	bool m_bCaptured:1;
	bool m_bInLayered:1;
	bool m_bFadeOuting:1;


public:
	DECLARE_FRAME_WND_CLASS(FRAMECLASS, IDR_MAINFRAME)

	CMainFrame()
		: m_style(STS_SMALLICON | STS_SHADING)
		, m_xBlock(0)
		, m_yBlock(0)
		, m_xDrive(0)
		, m_yDrive(0)
		, m_bTrackMouseEvent(FALSE)
		, m_bCaptured(false)
		, m_hotIndex(-1)
		, m_bInLayered(false)
		, m_bFadeOuting(false)
		, m_nCmdShow(SW_SHOWDEFAULT)
		, m_timerid(0)
		, m_hMenuFont(NULL)
		, m_hIconFont(NULL)
		, m_himDrive(NULL)
		, m_himDot(NULL)
		, m_nRM_NOTIFYICON(RegisterWindowMessage(_T("fvlch7_notify")))
		, m_nRM_LAUNCH(RegisterWindowMessage(_T("fvlch7_launch")))
	{
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UpdateUIAll();
		return FALSE;
	}

	bool IsTheming() const
	{
		return m_hTheme != 0;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(IDM_SHADING,				UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_BAR_VERT,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_TOPMOST,				UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_ICON_SMALL,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_USELAYERED,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_HIDEWINDOW,			UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		ON_WM_CREATE()
		ON_WM_CLOSE()
		ON_WM_DESTROY()
		ON_WM_QUERYENDSESSION()
		ON_WM_ACTIVATE()
		ON_WM_MOVING()
		ON_WM_TIMER()
		ON_WM_MOUSEMOVE()
		ON_WM_MOUSELEAVE()
		ON_WM_LBUTTONDOWN()
		ON_WM_CONTEXTMENU()
		MESSAGE_HANDLER(WM_NCMOUSEMOVE, OnHandlerNcMouseMove)
		ON_WM_NCHITTEST()
		ON_WM_NCLBUTTONDOWN()
		ON_WM_NCLBUTTONDBLCLK()
		ON_WM_TIMER()
		ON_WM_ERASEBKGND()
		ON_WM_PAINT()
		ON_WM_MEASUREITEM()
		ON_WM_DRAWITEM()
		ON_COMMAND(IDM_CLOSE, OnCommandClose)
		ON_COMMAND(IDM_ABOUT, OnCommandAbout)
		ON_COMMAND(IDM_BAR_VERT, OnCommandBarVert)
		ON_COMMAND(IDM_TOPMOST, OnCommandTopMost)
		ON_COMMAND(IDM_REFRESH, OnCommandRefresh)
		ON_COMMAND(IDM_ICON_SMALL, OnCommandIconSmall)
		ON_COMMAND(IDM_NOTIFYICON, OnCommandNotifyIcon)
		ON_COMMAND(IDM_HIDEWINDOW, OnCommandHideWindow)
		ON_COMMAND(IDM_SHADING, OnCommandShading)
		ON_COMMAND(IDM_USELAYERED, OnCommandLayered)
		ON_REGISTERD_MESSAGE(nRM_SHNOTIFY, OnRegisterShellNotify)
		ON_REGISTERD_MESSAGE(m_nRM_NOTIFYICON, OnRegisterNotifyIcon)
		ON_REGISTERD_MESSAGE(m_nRM_LAUNCH, OnLaunch)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs);
	void OnClose(BOOL& bHandled);
	void OnDestroy();
	BOOL OnQueryEndSession(UINT nSession);
	void OnActivate(UINT fActive, HWND hWndOther, BOOL fMinimized);
	BOOL OnMoving(UINT nSide, LPRECT prc);
	void OnTimer(UINT_PTR idEvent);
	void OnMouseMove(UINT fwKeys, CPoint pt);
	void OnMouseLeave();
	void OnLButtonDown(UINT fwKeys, CPoint pt);
	void OnContextMenu(HWND hWnd, CPoint point);
	UINT OnNcHitTest(int xPos, int yPos);
	void OnNcLButtonDown(UINT nHitTest, CPoint point);
	void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	LRESULT OnHandlerNcMouseMove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnEraseBkgnd(HDC hdc);
	void OnPaint();
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT pmi);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT pdi);

	void OnCommandClose();
	void OnCommandAbout();
	void OnCommandBarVert();
	void OnCommandTopMost();
	void OnCommandRefresh();
	void OnCommandIconSmall();
	void OnCommandNotifyIcon();
	void OnCommandHideWindow();
	void OnCommandShading();
	void OnCommandLayered();

	LRESULT OnRegisterShellNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnRegisterNotifyIcon(WPARAM wParam, LPARAM lParam);
	LRESULT OnLaunch(WPARAM wParam, LPARAM lParam) { Launch((int)wParam); return 0; }

	void UpdateUIAll();
	void Launch(int index);
	void SaveProfile();
	void ZapTimer(DWORD zapcount);
	BOOL TrackInRect(int index);
	UINT HitTest(POINT pt);
	void DrawCaption(HDC hdc, int clrIndex);
	void DrawDrive(HDC hdc, int index);
	void ResetLayeredWindow();
	void CleanUp();
	void TrackDriveMenu();
	BOOL TrayMessage(BOOL bAdd);
	void DriveListup();
	void DriveUpdate();
	void RecalcLayout(CRect& rc);
	BOOL GetItemRect(int index, CRect& rc);
	int GetIndexFromPoint(POINT pt, CRect& rc);
	void ShowSysMenu(POINT pt);
	void ProcessShellNotify(DWORD id, LPITEMIDLIST* pidls);

};

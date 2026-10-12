// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "filter.h"
#include "resource.h"

#include "..\lscore\menubar.h"
#include "..\lscore\xrebar.h"
#include "..\lscore\tbb_customdraw.h"
#include "..\lscore\taskbarlist.h"

#include "undo.h"
#include "printinfo.h"
#include "imagetype.h"
#include "exifinfo.h"
#include "navigator.h"
#include "recentopenlist.h"

class CFilterGroupMenu
{
public:
	CString m_str;
	HMENU m_hpop;
	CFilterGroupMenu(LPCTSTR pszName = NULL, HMENU hpop = NULL) : m_str(pszName), m_hpop(hpop) { }
};

#define EDITFRAME_CLASS			_T("VWIMAGEEDIT_FRAME")


/////////////////////////////////////////////////////////////////////////////
//

class CMainFrame :
		public CFrameWindowImpl<CMainFrame>,
		public CUpdateUI<CMainFrame>,
		public CMessageFilter,
		public CIdleHandler,
		public CPrintJobInfo,
		public CTaskbarListImpl<CMainFrame>,
		public CTBarCustomDrawImpl<CMainFrame>
{
public:
	DECLARE_FRAME_WND_CLASS(EDITFRAME_CLASS, IDR_FRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	virtual void TaskbarButtonCreated(ITaskbarList3* pTaskbarList);

	BEGIN_MSG_MAP(CMainFrame)
		CHAIN_MSG_MAP(CTaskbarListImpl<CMainFrame>)
		CHAIN_MSG_MAP(CTBarCustomDrawImpl<CMainFrame>)
		ON_WM_CREATE()
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		ON_WM_CLOSE()
		ON_WM_DESTROY()
		ON_WM_DROPFILES()
		ON_WM_CONTEXTMENU()
		ON_WM_APPCOMMAND()
		ON_WM_ERASEBKGND()
		ON_WM_GETMINMAXINFO()
//@		ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
		ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
		ON_COMMAND(IDM_FILE_NEW, OnFileNew)
		ON_COMMAND(IDM_FILE_OPENCLIPBOARD, OnFileOpenClipboard)
		ON_COMMAND(IDM_FILE_SAVE, OnFileSave)
		ON_COMMAND(IDM_FILE_SAVEAS, OnFileSaveAs)
		ON_COMMAND(IDM_FILE_WALLPAPER, OnFileWallPaper)
		ON_COMMAND(IDM_FILE_CLOSE, OnFileClose)
		ON_COMMAND(IDM_FILE_PRINT, OnPrint)
		ON_COMMAND(IDM_FILE_TWAIN, OnFileTwain)
		ON_COMMAND(IDM_EDIT_COPY, OnEditCopy)
		ON_COMMAND(IDM_EDIT_PASTE, OnEditPaste)
		ON_COMMAND(IDM_EDIT_SELALL, OnEditSelAll)
		ON_COMMAND(IDM_EDIT_SELCANCEL, OnEditSelCancel)
		ON_COMMAND(IDM_EDIT_SIZECLIP, OnEditSizeClip)
		ON_COMMAND(IDM_EDIT_UNDO, OnEditUndo)
		ON_COMMAND(IDM_COLOR_PALETTE, OnColorPalette)
		ON_COMMAND(IDM_COLOR_FULLCOLOR, OnColorFullColor)
		ON_COMMAND(IDM_COLOR_QUANTIZE, OnColorQuantize)
		ON_COMMAND(IDM_COLOR_COUNT, OnColorCount)
		ON_COMMAND(IDM_RESIZE_ZOOM, OnResizeZoom)
		ON_COMMAND(IDM_RESIZE_CANVAS, OnResizeCanvas)
		ON_COMMAND(IDM_RESIZE_TRIM, OnResizeTrim)
		ON_COMMAND(IDM_TOOL_TBSMALL, OnToolSmallToolbar)
		ON_COMMAND(IDM_TOOL_NAVIGATOR, OnToolNavigator)
		ON_COMMAND(IDM_ROTATE_LEFT, OnRotateLeft)
		ON_COMMAND(IDM_ROTATE_RIGHT, OnRotateRight)
		ON_COMMAND(IDM_ROTATE_FLIP, OnRotateFlip)
		ON_COMMAND(IDM_ROTATE_MIRROR, OnRotateMirror)
		ON_COMMAND(IDM_OPTION_SETUP, OnOptionSetup)
		ON_COMMAND(IDM_OPTION_GRID, OnShowGrid)
		ON_COMMAND(IDM_ZOOM, OnSetZoomMode)
		ON_COMMAND(IDM_ZOOM_IN, OnZoomIn)
		ON_COMMAND(IDM_ZOOM_OUT, OnZoomOut)
		ON_COMMAND(IDM_ZOOM_NONE, OnZoomNone)
		ON_COMMAND(IDM_ZOOM_FIT, OnZoomFit)
		ON_COMMAND(IDM_ZOOM_FITHORZ, OnZoomFitHorz)
		ON_COMMAND(IDM_ZOOM_FITVERT, OnZoomFitVert)
		ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		ON_COMMAND(ID_HELP, OnAppHelp)
		ON_COMMAND_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
		ON_COMMAND_RANGE(IDM_PLUGIN_FIRST, IDM_PLUGIN_LAST, OnFilterPlugin)
		ON_NOTIFY(ZMVN_MOUSEPOINT, IDW_ZOOMVIEW, OnNotifyMousePoint)
		ON_NOTIFY(ZMVN_CLIPCHANGED, IDW_ZOOMVIEW, OnNotifyClip)
		ON_NOTIFY(ZMVN_VISIBLECHANGED, IDW_ZOOMVIEW, OnNotifyVisibleChanged)
		ON_NOTIFY(ZMVN_SCALECHANGED, IDW_ZOOMVIEW, OnNotifyScaleChanged)
		ON_NOTIFY_ID(IDW_NAVI, OnNotifyNavigator)
		ON_NOTIFY_RANGE(NM_KEYDOWN, 0, 0xFFFF, OnNotifyKeyDown)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(IDM_FILE_SAVE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_FILE_OPENCLIPBOARD,	UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_EDIT_UNDO,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_ZOOM_IN,				UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_ZOOM_OUT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_COLOR_QUANTIZE,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_COLOR_FULLCOLOR,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_COLOR_PALETTE,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_EDIT_COPY,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_RESIZE_TRIM,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_EDIT_PASTE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_TOOL_TBSMALL,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_TOOL_NAVIGATOR,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_OPTION_GRID,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_DRAWBY_BITMAP,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_DRAWBY_GDIPLUS,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_DRAWBY_DIB,			UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	// メッセージハンドラ
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnActivate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnClose(BOOL& bHandled);
	void OnDestroy();
	void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	void OnDropFiles(HDROP hDropInfo);
	void OnContextMenu(HWND hWnd, CPoint pt);
	BOOL OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState);
//@	LRESULT OnCommandHelp(WPARAM, LPARAM);
	void OnFileOpen();
	void OnFileNew();
	void OnFileOpenClipboard();
	void OnFileSave();
	void OnFileSaveAs();
	void OnFileWallPaper();
	void OnFileClose();
	void OnPrint();
	void OnFilePrev();
	void OnFileNext();
	void OnFileTwain();
	void OnEditCopy();
	void OnEditPaste();
	void OnEditSelAll();
	void OnEditSelCancel();
	void OnEditSizeClip();
	void OnEditUndo();
	void OnColorPalette();
	void OnColorFullColor();
	void OnColorQuantize();
	void OnColorCount();
	void OnShowHistogram();
	void OnResizeZoom();
	void OnResizeCanvas();
	void OnResizeTrim();
	void OnRotateLeft();
	void OnRotateRight();
	void OnRotateFlip();
	void OnRotateMirror();
	void OnOptionSetup();
	BOOL OnEraseBkgnd(HDC hdc) { return TRUE; }
	void OnNotifyMousePoint(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyClip(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyHeightChange(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyChevronPushed(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyVisibleChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyScaleChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyNavigator(NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarGetButtonStyle(NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolSmallToolbar();
	void OnToolNavigator();
	void OnZoomIn() { m_pane.ZoomIn(); }
	void OnZoomOut() { m_pane.ZoomOut(); }
	void OnZoomNone() { m_pane.ZoomDefault(); }
	void OnZoomFit() { m_pane.ZoomFull(); }
	void OnZoomFitHorz() { m_pane.ZoomFitHorz(); }
	void OnZoomFitVert() { m_pane.ZoomFitVert(); }
	void OnSetZoomMode();
	void OnShowGrid();
	void OnOpenRecentFile(UINT nID, UINT code, HWND hwnd);
	void OnFilterPlugin(UINT nID, UINT code, HWND hwnd);
	void OnNotifyKeyDown(UINT id, NMHDR* pNmhdr, LRESULT* pResult);
	void OnAppAbout();
	void OnAppHelp();

	int DoToolBarDropDown(TBNOTIFY* ptbn) { return 0; }			// CTBarCustomDrawImpl から呼び出される。ここでは何もしない

private:
	void Construct();
	void UpdateUIAll();
	HMENU GetSubMenuByPosition(int n1stPos, int n2ndPos);

protected:
	HWND CreateToolbar();
	BOOL LoadFile(LPCTSTR pszName);
	void EmptyUndo();
	HDIB PopUndo(CRect& rrc);
	void PushUndo(UINT nIDCommand, HDIB hDIB);
	void SetDIB(HDIB hdib, BOOL bRedraw);
	void ConvertColor(int nBits);
	void SetDIBInformation(HDIB hDIB);
	void DoFilter(UINT nFlag);
	void UpdateDIB(HDIB hdib, UINT nID, BOOL bRedraw, BOOL bKeepRect);

	void InitializePluginFilters();
	void EnumPluginFilters(LPCTSTR pszDll, UINT& idStart, CSimpleMap<UINT, CFilterGroupMenu*>& arrNewMenu);

	static BOOL CALLBACK ProgressCallback(int msg, int param);
	static BOOL CALLBACK TrackImage(HDIB hdibTrack, POINT* ppt, COLORREF clrMask);
	static BOOL CALLBACK GradationSelector(HWND hwndParent, GRADEDITINFO* pgsi);

public:
	BOOL OpenFile(LPCTSTR pszName);
	void SetCaptionTitle(LPCTSTR pszTitle);
//@	void ResizeFrame();

	virtual bool PrintPage(UINT nPage, HDC hDC);

private:
	// フレーム用データメンバ
#ifdef CUSTOM_REBER_USING
	CXRebarCtrl m_rebar;						// リバー
#else
	CReBarCtrl m_rebar;						// リバー
#endif
	CZoomViewCtrl m_pane;
	CMenuBarCtrl m_menubar;						// コマンドバー
	CMultiPaneStatusBarCtrl m_statusbar;		// ステータスバー
	CToolBarCtrl m_toolbar;						// ツールバー
	CColoredProgressCtrl m_progress;

	// 印刷
	CPrinterT<true> m_printer;
	CDevModeT<true> m_devmode;
	bool m_print_selrect;

	CString m_strName;
	CSize m_sizeDib;
	UINT m_nBitCount;
	HICON m_hicScale;
	HICON m_hicSizeGrip;
	HICON m_hicRgbGrip;
	HWND m_hdlgPrintReady;

	CAtlList<CUndo*> m_Undo;
	int m_nFileType;

	DWORD m_dwStat;

	CSimpleMap<CString, UINT>  m_mapFilterGroup;
	CSimpleMap<UINT, HMENU>    m_mapPluginMenu;
	CSimpleMap<UINT, CFilter*> m_mapPlugin;

#if _WTL_CMDBAR_VISTA_MENUS
	CRecentOpenFileList m_mru;
#else
	CRecentDocumentList m_mru;
#endif	// _WTL_CMDBAR_VISTA_MENUS

	CNaviWindow m_navi;					// ナビゲータ
};

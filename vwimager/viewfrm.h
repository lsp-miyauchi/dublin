// MainFrm.h : interface of the CViewFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlsync.h>
#include "..\lscore\atl_special_split.h"

#include "filter.h"
#include "resource.h"

#include "..\lscore\menubar.h"
#include "..\lscore\xrebar.h"
#include "..\lscore\tbb_customdraw.h"
#include "..\lscore\taskbarlist.h"

#include "undo.h"
#include "printinfo.h"
#include "imagetype.h"
#include "view_information.h"
#include "filelist.h"
#include "recentopenlist.h"
#include "loaderthread.h"
#include "gifanimation_ctrl.h"


/////////////////////////////////////////////////////////////////////////////
//

template <bool t_bVertical = true>
class CVWSplitterWindowT : public CSplitterWindowImpl<CVWSplitterWindowT<t_bVertical>, t_bVertical>
{
private:
	COLORREF m_clrBar;

public:
	DECLARE_WND_CLASS_EX(_T("VWSplitter"), CS_DBLCLKS, COLOR_WINDOW)

	CVWSplitterWindowT()
		: m_clrBar(GetSysColor(COLOR_3DFACE))
	{
	}

	void DrawSplitterBar(CDCHandle dc)
	{
		CRect rc;
		if (GetSplitterBarRect(&rc)) {
			dc.FillSolidRect(&rc, m_clrBar);
		}
	}

	COLORREF SetBarColor(COLORREF clr, BOOL bRedraw)
	{
		COLORREF prev = m_clrBar;
		if (m_clrBar != clr) {
			m_clrBar = clr;
			if (bRedraw && IsWindow()) {
				CRect rc;
				if (GetSplitterBarRect(&rc)) {
					InvalidateRect(&rc);
				}
			}
		}
		return  prev;
	}
};

typedef CVWSplitterWindowT<true>	CVWSplitterWindow;
typedef CVWSplitterWindowT<false>	CVWHorSplitterWindow;


#define VIEWFRAME_CLASS				_T("VWIMAGEVIEW_FRAME")


/////////////////////////////////////////////////////////////////////////////
//

class CViewFrame :
		public CFrameWindowImpl<CViewFrame>,
		public CUpdateUI<CViewFrame>,
		public CMessageFilter,
		public CIdleHandler,
		public CPrintJobInfo,
		public CTaskbarListImpl<CViewFrame>,
		public CTBarCustomDrawImpl<CViewFrame>
{
private:
	UINT m_nRM_LOADIMAGECOMPLETED;

	// フレーム用データメンバ
#ifdef CUSTOM_REBER_USING
	CXRebarCtrl m_rebar;						// リバー
#else
	CReBarCtrl m_rebar;						// リバー
#endif
	CMenuBarCtrl m_menubar;						// コマンドバー
	CToolBarCtrl m_toolbar;						// ツールバー
	CMultiPaneStatusBarCtrl m_statusbar;		// ステータスバー

	CVWSplitterWindow m_splitApp;				// スプリターウィンドウ
	CVWHorSplitterWindow m_splitImageHorz;
	CVWSplitterWindow m_splitImageVert;
	CZoomViewCtrl m_pane;						// イメージ表示ペイン
	CInformationView m_infoview;				// 情報ウィンドウ
	CThumbnaiListView m_thumb;					// サムネイルリスト
	AnimationImageList* m_anim;					// Loading animation
	CLoaderThread m_loader;
	DWORD m_magic;

	CString m_strName;							// ファイル名
	CSize m_sizeDib;							// DIB画像サイズ
	BOOL m_bGps;								// GPS
	HICON m_hicScale;							// 虫眼鏡アイコン
	HICON m_hicSizeGrip;						// 十字アイコン
	HICON m_hicRgbGrip;

	HWND m_hdlgPrintReady;						// 印刷ダイアログウィンドウハンドル

	DWORD m_dwStat;
	bool m_bMouseWheelProcessing:1;

	// 最近開いたファイルのリスト
#if _WTL_CMDBAR_VISTA_MENUS
	CRecentOpenFileList m_mru;
#else
	CRecentDocumentList m_mru;
#endif	// _WTL_CMDBAR_VISTA_MENUS

	// 印刷関連
	CPrinterT<true> m_printer;
	CDevModeT<true> m_devmode;
	bool m_print_selrect;

public:
	DECLARE_FRAME_WND_CLASS(VIEWFRAME_CLASS, IDR_VIEW)

	CViewFrame() : m_nRM_LOADIMAGECOMPLETED(::RegisterWindowMessage(_T("wm_gmil_loadimagecompleted")))
	{
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	virtual void TaskbarButtonCreated(ITaskbarList3* pTaskbarList);

	void UpdateLayout(BOOL bResizeBars = TRUE);

	BEGIN_MSG_MAP(CViewFrame)
		CHAIN_MSG_MAP(CTaskbarListImpl<CViewFrame>)
		CHAIN_MSG_MAP(CTBarCustomDrawImpl<CViewFrame>)
		ON_WM_CREATE()
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		ON_WM_CLOSE()
		ON_WM_DESTROY()
		ON_WM_DROPFILES()
		ON_WM_CONTEXTMENU()
		ON_WM_ERASEBKGND()
		ON_WM_GETMINMAXINFO()
		ON_WM_APPCOMMAND()
//@		ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
		ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
		ON_COMMAND(IDM_FILE_CLOSE, OnFileClose)
		ON_COMMAND(IDM_FILE_PRINT, OnPrint)
		ON_COMMAND(IDM_FILE_PREV, OnFilePrev)
		ON_COMMAND(IDM_FILE_NEXT, OnFileNext)
		ON_COMMAND(IDM_FILE_DELETE, OnFileDelete)
		ON_COMMAND(IDM_FILE_CONTEXTMENU, OnFileContextMenu)
		ON_COMMAND(IDM_EDIT_COPY, OnEditCopy)
		ON_COMMAND(IDM_EDIT_SELALL, OnEditSelAll)
		ON_COMMAND(IDM_EDIT_SELCANCEL, OnEditSelCancel)
		ON_COMMAND(IDM_EDIT_SIZECLIP, OnEditSizeClip)
		ON_COMMAND(IDM_ROTATE_LEFT, OnRotateLeft)
		ON_COMMAND(IDM_ROTATE_RIGHT, OnRotateRight)
		ON_COMMAND(IDM_ZOOM, OnSetZoomMode)
		ON_COMMAND(IDM_ZOOM_IN, OnZoomIn)
		ON_COMMAND(IDM_ZOOM_OUT, OnZoomOut)
		ON_COMMAND(IDM_ZOOM_NONE, OnZoomNone)
		ON_COMMAND(IDM_ZOOM_FIT, OnZoomFit)
		ON_COMMAND(IDM_ZOOM_FITHORZ, OnZoomFitHorz)
		ON_COMMAND(IDM_ZOOM_FITVERT, OnZoomFitVert)
		ON_COMMAND(IDM_TOOL_EXIFINFO, OnToolExifInfo)
		ON_COMMAND(IDM_TOOL_THUMBLIST, OnToolThumbnailList)
		ON_COMMAND(IDM_TOOL_THUMBLIST_LAYOUT, OnToggleThumbnailListLayout)
		ON_COMMAND(IDM_TOOL_TBSMALL, OnToolSmallToolbar)
		ON_COMMAND(IDM_TOOL_GPS, OnToolGpsAccess)
		ON_COMMAND(IDM_MODE_GRAB, OnModeGrab)
		ON_COMMAND(IDM_MODE_SEL, OnModeSel)
		ON_COMMAND(IDM_EXEC_EDITOR, OnExecEditor)
		ON_COMMAND(IDM_EXEC_SLIDESHOW, OnExecSlideShow)
		ON_COMMAND(IDM_PARSE_INFORMATION, OnParseInformation)
		ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		ON_COMMAND(ID_HELP, OnAppHelp)
		ON_COMMAND_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
		ON_NOTIFY_ID(IDW_THUMBLIST, OnNotifyThumbnailList)
		ON_NOTIFY(ZMVN_MOUSEPOINT, IDW_ZOOMVIEW, OnNotifyMousePoint)
		ON_NOTIFY(ZMVN_CLIPCHANGED, IDW_ZOOMVIEW, OnNotifyClip)
		ON_NOTIFY(ZMVN_VISIBLECHANGED, IDW_ZOOMVIEW, OnNotifyVisibleChanged)
		ON_NOTIFY(ZMVN_SCALECHANGED, IDW_ZOOMVIEW, OnNotifyScaleChanged)
		ON_NOTIFY_RANGE(NM_KEYDOWN, 0, 0xFFFF, OnNotifyKeyDown)
		ON_NOTIFY_ID(IDR_INFOVIEW, OnNotifyNavigator)
		ON_REGISTERD_MESSAGE(m_nRM_LOADIMAGECOMPLETED, OnLoadImageCompleted)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnHandlerMouseWheel)
		CHAIN_MSG_MAP(CUpdateUI<CViewFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CViewFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CViewFrame)
		UPDATE_ELEMENT(IDM_FILE_PREV,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_FILE_NEXT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_FILE_DELETE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_ZOOM_IN,				UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_ZOOM_OUT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_TOOL_EXIFINFO,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_TOOL_THUMBLIST,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_TOOL_TBSMALL,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_TOOL_GPS,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_MODE_GRAB,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_MODE_SEL,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
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
	void OnFileClose();
	void OnPrint();
	void OnFilePrev();
	void OnFileNext();
	void OnFileDelete();
	void OnFileContextMenu();
	void OnEditCopy();
	void OnEditSelAll();
	void OnEditSelCancel();
	void OnEditSizeClip();
	void OnExecEditor();
	void OnExecSlideShow();
	BOOL OnEraseBkgnd(HDC hdc) { return TRUE; }
	void OnNotifyThumbnailList(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyMousePoint(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyClip(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyVisibleChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyScaleChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyHeightChange(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyChevronPushed(NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarGetButtonStyle(NMHDR* pNmhdr, LRESULT* pResult);
	void OnRotateLeft();
	void OnRotateRight();
	void OnZoomIn() { m_pane.ZoomIn(); }
	void OnZoomOut() { m_pane.ZoomOut(); }
	void OnZoomNone() { m_pane.ZoomDefault(); }
	void OnZoomFit() { m_pane.ZoomFull(); }
	void OnZoomFitHorz() { m_pane.ZoomFitHorz(); }
	void OnZoomFitVert() { m_pane.ZoomFitVert(); }
	void OnToolExifInfo();
	void OnToolThumbnailList();
	void OnToolSmallToolbar();
	void OnToolGpsAccess();
	void OnToggleThumbnailListLayout();
	void OnModeGrab();
	void OnModeSel();
	void OnSetZoomMode();
	void OnShowGrid();
	void OnOpenRecentFile(UINT nID, UINT code, HWND hwnd);
	void OnNotifyKeyDown(UINT id, NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyNavigator(NMHDR* pNmhdr, LRESULT* pResult);
	LRESULT OnLoadImageCompleted(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnParseInformation();
	void OnAppAbout();
	void OnAppHelp();

	int DoToolBarDropDown(TBNOTIFY* ptbn) { return 0; }			// CTBarCustomDrawImpl から呼び出される。ここでは何もしない

private:
	void Construct();
	void UpdateUIAll();
	HMENU GetSubMenuByPosition(int n1stPos, int n2ndPos);

protected:
	HWND CreateToolbar();
	void ShowDIB(HDIB hdib);
	void SetDIB(HDIB hdib, BOOL bRedraw);
	void SetDIBInformation(HDIB hDIB);

public:
	BOOL OpenFile(LPCTSTR pszName, bool bInitSlide = false);
	void SetCaptionTitle(LPCTSTR pszTitle);
	LPCTSTR GetCurrentFileName()
	{
		return m_strName;
	}
	virtual bool PrintPage(UINT nPage, HDC hDC);
};

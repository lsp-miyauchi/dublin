/////////////////////////////////////////////////////////////////////////////
// mainfrm.h : interface of the CMainFrame class
//

#pragma once

#include "..\lscore\atl_special_split.h"
#include "..\lscore\taskbarlist.h"
#include "mousewheeler.h"
#include "folderview.h"
#include "file.h"
#include "bookmark_view.h"
#include "history.h"
#include "navibar.h"
#include "idlbar.h"
#include "funckeybar.h"

#include "shellnotify.h"
#include "..\lscore\tbb_customdraw.h"
#include "..\lscore\xrebar.h"
#include "tabbarctrl.h"
#include "shellitemsearch.h"
#include "shell_new_item.h"
#include "view_switcher.h"
#include "statusbar_sup.h"

/////////////////////////////////////////////////////////////////////////////
//

#define IDW_SPLITTER_APP			ATL_IDW_PANE_FIRST
#define IDW_SPLITTER_VIEW			ATL_IDW_PANE_FIRST+1
#define IDW_SPLITTER_TREE			ATL_IDW_PANE_FIRST+2
#define IDW_FOLDERTREE				ATL_IDW_PANE_FIRST+3
#define IDW_FILEBROWSER				ATL_IDW_PANE_FIRST+4

enum {
	DCM_CHANGEFORMAT,
	DCM_VIEWSTYLE,
	DCM_UPDATEIMAGE,
};

typedef int (CALLBACK* TRACKMENUCUSTOMPROC)(HMENU hmenu);


/////////////////////////////////////////////////////////////////////////////
// タイマ関連

#define TIMERID_CHECKREGIST		4
#define TIMERID_COMBINE			5
#define TIM_COMBINEWAIT			400L				// 0.4 sec
#define TIM_CHECKREGIST			7200000L			// 120 min


/////////////////////////////////////////////////////////////////////////////
// マルチドキュメント
//

class CDocument
{
public:
	// ファイルペイン（実体）… new で作成、delete はCFileBrowseView のOnFinalMessage で delete this
	CFileBrowseView* m_pWndBrowser;
	CHistory m_History;
	CFilterSetting m_Filter;					// ワイルドカード
	CIDListPtr m_idlRoot;							// ツリーのルートフォルダ（デスクトップがルートでないモードで有効）
	DWORD m_serial;
	DWORD m_lastTimeReference;

public:
	CDocument() : m_pWndBrowser(NULL), m_serial(GetTickCount()), m_lastTimeReference(0)
	{
		m_Filter.m_bUseRegExp = ((gnFopStat & FOS_REXFINDER) != 0);
	}
	~CDocument()
	{
		m_History.Clear();

		// CFileBrowser は AutoDelete
		CFileBrowseView* pTemp = m_pWndBrowser;
		m_pWndBrowser = NULL;
		if (pTemp && pTemp->IsWindow())
			pTemp->SendMessage(WM_CLOSE);
	}
	HWND CreateBrowser(HWND hwndParent, DWORD style, DWORD exStyle, UINT id)
	{
		m_pWndBrowser = new CFileBrowseView;
		return m_pWndBrowser->Create(hwndParent, 0, NULL, style, exStyle, id, (void*)this);
	}

	inline CHistory* GetHistory() { return &m_History; }
	inline CFileBrowseWindow GetFileBrowser() const
	{
		ATLASSERT(m_pWndBrowser != NULL);
		__try {
			if (m_pWndBrowser && !IsBadReadPtr(m_pWndBrowser, sizeof(CFileBrowseWindow)) && ::IsWindow(m_pWndBrowser->m_hWnd))
				return CFileBrowseWindow(m_pWndBrowser->m_hWnd);
		} __finally { }
		return CFileBrowseWindow((HWND)NULL);
	}
	inline CFilterSetting* GetFilter() { return &m_Filter; }

	LPCITEMIDLIST GetDocumentFolder()
	{
		CFileBrowseWindow browser = GetFileBrowser();
		return browser.GetCurrentFolder();
	}

	inline void SetRootFolder(LPCITEMIDLIST pidlRoot) { m_idlRoot.Clone(pidlRoot); }
	inline LPCITEMIDLIST GetRootFolder() { return m_idlRoot; }

	void SetTimeReference()
	{
		m_lastTimeReference = SystemTimeToDWord();
	}
	inline DWORD GetTimeReference() { return m_lastTimeReference; }
};


/// <summary>
/// ToolbarCtrlのイメージリストに別のイメージリストを追加するクラス
/// </summary>
class CMIToolBarCtrl : public CToolBarCtrl
{
public:
	void AppendImageList(HIMAGELIST him)
	{
		HIMAGELIST himBase = GetImageList();
		if (!himBase) {
			SetImageList(him);
			return;
		}
		int cnt = ImageList_GetImageCount(him);
		for (int i = 0; i < cnt; i++) {
			HICON hic = ImageList_GetIcon(him, i, ILD_TRANSPARENT);
			ImageList_ReplaceIcon(himBase, -1, hic);
			DestroyIcon(hic);
		}
	}
};




/////////////////////////////////////////////////////////////////////////////
// CMainFrame

class CMainFrame
		: public CFrameWindowImpl<CMainFrame>
		, public CUpdateUI<CMainFrame>
		, public CMessageFilter
		, public CIdleHandler
		, public CTaskbarListImpl<CMainFrame>
		, public CTBarCustomDrawImpl<CMainFrame>
		, public CMouseWheelServerImpl<CMainFrame>
		, public CShellChangeNotifyImpl<CMainFrame>
{
protected:  // control bar embedded members
	CDocument* m_pCurrentDocument;
	CFolderTreeView m_baseTree;					// フォルダツリー（実体）
	CBookmarkView m_viewFavo;
	CMenuBarCtrl m_menubar;						// メニューバー

#ifdef CUSTOM_REBER_USING
	CXRebarCtrl m_rebar;						// リバー（標準）
	CXRebarCtrl m_vtrebar;						// リバー（ドライブ縦置き用）
#else
	CReBarCtrl m_rebar;							// リバー（標準）
	CReBarCtrl m_vtrebar;						// リバー（ドライブ縦置き用）
#endif
	CLspStatusBarCtrl m_statbar;			// ステータスバー
	CToolBarCtrl m_propbar;						// プロパティバー
	CToolBarCtrl m_sortbar;						// ソートバー
	CAppBar m_appbar;							// オブジェクトパネル
	CDriveBar m_drive;							// ドライブバー
	CNaviBar m_navibar;							// ナビバー
	CIdlBar m_smartbar;							// スマートバー
	CTabBarCtrl m_tabbar;						// タブバー
	CFuncKeyBar m_functionbar;					// 機能キー
	CSplitterWindow m_splitApp;
	CSplitterWindow m_splitView;
	CHorSplitterWindow m_splitTree;
	CViewSwitcher m_ViewSwitch;
	CCmItemArray m_arrSysFolder;
	CCmItemArray m_arrSSync;
	CCmItemArray m_arrTabGroup;
	CTabReopenArray m_arrTabReopen;				// 開いたタブの履歴
	CListStyleMap m_mapListStyle;				// 表示したフォルダの履歴
	CAccessHistoryArray m_arrAccessHistory;		// “何かした”（例えばコピーや移動、参照など）フォルダの履歴
	CSimpleArray<CShellNewItem*> m_arrShellNew;
	CSimpleArray<CToolBarCtrl*> m_arrToolbar;
	CImageList m_statbarImageList;
	CShortcutList m_scutFile;					// ファイルペイン用ショートカットメニューデータ
	CShortcutList m_scutTree;					// フォルダツリー用ショートカットメニューデータ
	UINT_PTR m_timerid_combine;
	UINT m_wm_frameisidle;
	UINT m_wm_notifyicon;
	bool m_bEnableAccel;
	bool m_bInitCompleted:1;
	bool m_bIgnoreFolderBrowsing:1;
	CShellItemSearch m_ShellItemSearch;

public:
	DECLARE_FRAME_WND_CLASS(APP_FRAMECLASS, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	virtual void TaskbarButtonCreated(ITaskbarList3* pTaskbarList);
	virtual void OnShellChangeNotify(long lEvent, PIDLIST_ABSOLUTE* pidls);

	CMainFrame();
	~CMainFrame();

	void UpdateLayout(BOOL bResizeBars = TRUE);

BEGIN_MSG_MAP(CMainFrame)
	CHAIN_MSG_MAP(CTaskbarListImpl<CMainFrame>)
	CHAIN_MSG_MAP(CTBarCustomDrawImpl<CMainFrame>)
	CHAIN_MSG_MAP(CMouseWheelServerImpl<CMainFrame>)
	CHAIN_MSG_MAP(CShellChangeNotifyImpl<CMainFrame>)
	MESSAGE_HANDLER(WM_NCPAINT, OnHandlerNcPaint)
	MESSAGE_HANDLER(WM_COMMAND, OnHandlerCommand)
	MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
	ON_WM_CREATE()
	MESSAGE_HANDLER(WM_SIZE, OnMySize)
	MESSAGE_HANDLER(WM_MENUSELECT, OnHandlerMenuSelect)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_ENDSESSION()
	ON_WM_CONTEXTMENU()
	ON_WM_WININICHANGE()
	ON_WM_DROPFILES()
	ON_WM_DEVICECHANGE()
	ON_WM_APPCOMMAND()
	ON_WM_DRAWITEM()
	ON_NOTIFY_RANGE(TBN_BEGINADJUST, IDW_TBFIRST, IDW_TBLAST, OnToolbarBeginAdjust)
	ON_NOTIFY_RANGE(TBN_ENDADJUST, IDW_TBFIRST, IDW_TBLAST, OnToolbarEndAdjust)
	ON_NOTIFY_RANGE(TBN_TOOLBARCHANGE, IDW_TBFIRST, IDW_TBLAST, OnToolbarChange)
	ON_NOTIFY_RANGE(TBN_QUERYINSERT, IDW_TBFIRST, IDW_TBLAST, OnToolbarQueryInsert)
	ON_NOTIFY_RANGE(TBN_QUERYDELETE, IDW_TBFIRST, IDW_TBLAST, OnToolbarQueryDelete)
	ON_NOTIFY_RANGE(TBN_GETBUTTONINFO, IDW_TBFIRST, IDW_TBLAST, OnToolbarGetButtonInfo)
	ON_NOTIFY_RANGE(TBN_CUSTHELP, IDW_TBFIRST, IDW_TBLAST, OnToolbarCustomHelp)
	ON_NOTIFY_RANGE(TBN_RESET, IDW_TBFIRST, IDW_TBLAST, OnToolbarReset)
	ON_NOTIFY_RANGE(TBN_BEGINDRAG, IDW_TOOLBAR_GROUP_FIRST, IDW_TOOLBAR_GROUP_LAST, OnToolbarBeginDrag)
	ON_NOTIFY(DBN_LBUTTONDOWN, IDW_DRIVEBAR, OnDrivebarLButtonDown)
	ON_NOTIFY(DBN_NEWWINDOW, IDW_DRIVEBAR, OnDrivebarNewWindow)
	ON_NOTIFY(DBN_NEWTAB, IDW_DRIVEBAR, OnDrivebarNewTab)
	ON_NOTIFY(DBN_ADDFAVORITE, IDW_DRIVEBAR, OnDrivebarAddFavorite)
	ON_NOTIFY(TCN_GETINFOTIP, IDW_TABBAR, OnTabbarNotifyGetInfotip)
	ON_NOTIFY(TCN_SELCHANGE, IDW_TABBAR, OnTabbarNotifySelChange)
	ON_NOTIFY(TCN_BEGINDRAG, IDW_TABBAR, OnTabbarNotifyBeginDrag)
	ON_NOTIFY(TCN_CLOSEBUTTONCLICKED, IDW_TABBAR, OnTabbarNotifyCloseButtonClicked)
	ON_NOTIFY(TCN_DELETEITEM, IDW_TABBAR, OnTabbarNotifyDeleteItem)
	ON_NOTIFY(TCN_ADDBUTTONCLICKED, IDW_TABBAR, OnTabbarNotifyAddButtonClicked)
	ON_NOTIFY(NM_DBLCLK, IDW_TABBAR, OnTabbarNotifyDblClk)
	ON_NOTIFY(NM_DBLCLK, IDW_FAVORITE, OnFavoviewNotifyDblClk)
	ON_NOTIFY(TFN_SELCHANGED, IDW_FOLDERTREE, OnFolderTreeNotifySelChanged)
	ON_NOTIFY(TFN_ENTERFILEOPERATION, IDW_FOLDERTREE, OnFolderTreeNotifyEnterFileOperation)
	ON_NOTIFY(NM_CLICK, ATL_IDW_STATUS_BAR, OnStatusbarNotifyClick)
	ON_MESSAGE_VOID(WM_MBMTAKEFOCUS, OnMenubarTakeFocus)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	ON_MESSAGE(WM_JUMPFILE, OnJump)
	ON_MESSAGE(WM_OPENFOLDER, OnOpenFolder)
	ON_MESSAGE(WM_OPENTABFOLDER, OnOpenTabFolder)
	ON_MESSAGE(WM_OPENTABGROUP, OnOpenTabGroup)
	ON_MESSAGE_VOID(WM_ADDFAVORITE, OnAddFavorite)
	ON_MESSAGE(WM_SELECTFOLDERITEM, OnSelectFolderItem)
	ON_MESSAGE(WM_GETTABSFOLDERIDLIST, OnGetTabsFolderIDList)
	ON_MESSAGE_VOID(WM_CLEARFOLDERHISTORY, OnClearDocumentHistoryAll)
	ON_MESSAGE_VOID(WM_ACTIVATENEXTVIEW, OnActivateNextView)
	ON_MESSAGE_VOID(WM_SETACTIVEVIEW, OnSetActiveView)
	ON_MESSAGE_VOID(WM_ENABLEACCEL, OnEnableAccel)
	ON_MESSAGE(WM_QUERYIMAGELIST, OnQueryImageList)
	ON_REGISTERD_MESSAGE(nRM_REGTHANKS, OnRegThanks)
	ON_REGISTERD_MESSAGE(nRM_REGOK, OnRegOK)
	ON_REGISTERD_MESSAGE(nRM_FAVORITE, OnUpdateFavorite)
	ON_REGISTERD_MESSAGE(nRM_SAVEAPPPROP, OnSaveAppProp)
	ON_REGISTERD_MESSAGE(m_wm_frameisidle, OnFrameIsIdle)
	ON_REGISTERD_MESSAGE(m_ShellItemSearch.m_wm_shellitemsearch, OnShellItemSearch)
	CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
	CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
END_MSG_MAP()

BEGIN_UPDATE_UI_MAP(CMainFrame)
	UPDATE_ELEMENT(IDM_VIEW_DESKTOPTREE,	UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT,				UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_NAME,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_TYPE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_SIZE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_TIME,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_ATTR,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_DOS,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_PATH,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_COL0,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_COL1,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_COL2,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_COL3,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_COL4,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_COL5,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_COL6,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_COL7,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_COL8,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_SORT_UP,				UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_SORT_DOWN,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_ICON_NORMAL,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_ICON_OUTLINE,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_ICON_REPORT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_ICON_IMAGE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_ICON_ICON,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_ICON_ENLARGE,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_ICON_RELAX,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_IMAGE_072,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_IMAGE_096,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_IMAGE_128,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_IMAGE_160,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_IMAGE_192,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_EDIT_PASTE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_EDIT_PASTESHORTCUT,	UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_VIEW_SIZE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_VIEW_DATE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_VIEW_ATTRIB,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_VIEW_NAMEELLIPSIS,	UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_TYPE_FILEHIDDEN,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TYPE_FILESUPERHIDDEN,UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TYPE_EXCFOLDER,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_DISK_LABEL,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_NET_CONNECT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_NET_DISCONNECT,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_NET_ADDPLACE,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_BAR_MENU,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_PROP,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_SORT,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_APPBAR,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_STAT,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_FUNCTION,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_DRIVE,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_DRIVE_HORZ,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_DRIVE_VERTLEFT,	UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_DRIVE_VERTRIGHT,	UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_SMART,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_LARGEICON,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_FAVO,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_NAVI,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_REPOSITION,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_BAR_LOCKREBAR,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 0,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 1,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 2,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 3,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 4,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 5,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 6,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 7,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 8,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 9,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 10,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDW_TBFIRST + 11,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_DIR_TRACE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_DIR_HISTORY,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_DIR_PREV,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_DIR_NEXT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	UPDATE_ELEMENT(IDM_WINDOW_PREV,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_WINDOW_NEXT,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_NEW,				UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_CLOSE,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_PREV,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_NEXT,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_CLOSE_ANYOTHER,	UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_CLOSE_LEFTSIDE,	UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_CLOSE_RIGHTSIDE,	UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_REOPEN_RECENT,	UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_EXPORT,			UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_SORTDEFAULT,		UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_SORTREFERENCE,	UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_TAB_LOCK,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR | UPDUI_MENUBAR)
	UPDATE_ELEMENT(IDM_HISTORY_START + 0,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 1,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 2,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 3,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 4,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 5,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 6,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 7,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 8,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 9,   UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 10,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 11,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 12,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 13,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 14,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 15,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 16,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 17,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 18,  UPDUI_MENUPOPUP)
	UPDATE_ELEMENT(IDM_HISTORY_START + 20,  UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()


public:
	void InitialUpdate();
	void SetInitialFolder(LPCTSTR pszInitialFolder);
	void SetInitCompleted()
	{
		m_bInitCompleted = true;
	}
//@	int ListupOther(CSimpleArray<HWND>& array);
	void ShowApp(CSimpleArray<HWND>& array, UINT nCmdShow);
	BOOL SelectFolderItem(LPCITEMIDLIST pidlFullyQual);
	BOOL OpenFolder(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlRoot = NULL, bool bSelectTreeNode = true);
	BOOL OpenChildFolder(LPCITEMIDLIST pidl);

	void UpdateTitle();
	BOOL ChangeFolder(LPCITEMIDLIST pidlFullyQual);
	HMENU GetSubSequenceMenu(int iMain, int iSub);
	void EnableAccel(bool bEnable);
	DWORDLONG GetProcessCreationTime();
	DWORDLONG GetProcessActiveTime();
	void GetSelectionCenterPoint(LPPOINT ppt);
	void RecordAccessFolder(LPCITEMIDLIST pidl);

// Implementation
public:
	void UpdateFileView(LPCITEMIDLIST pidlFolder);
	void ClearDocumentHistoryAll();
	void ResetFolderSettings();
	void SaveRebarSettings();
	void BrowserListStyleChanged(const ICONLISTSTYLE* pls);

protected:
	LRESULT RerootCurrentMessage(HWND hwnd, BOOL bUseSendMessage = TRUE);
	void UpdateUIAll();
	void UpdateTabMenu(int indexTarget);
	void UpdateStatusBarText();
	void __UpdateMenuAccelerator(HMENU hmenu, bool bForce);
	void UpdateMenuAccelerator(bool bForce = false);
	BOOL CreateClient();
	BOOL CreatePropBar();
	HWND CreateDefaultTooBar(int* parrButtons, int numButtons, UINT idBar, UINT idResource);
	BOOL CreateToolBar();
	BOOL CreateSortBar();
	BOOL CreateNaviBar();
	BOOL CreateSmartBar();
	BOOL CreateStatusBar();
	BOOL CreateFunctionBar();
	BOOL CreateDriveBar();
	void ResetCustomDrawToolbar(HWND hwnd, UINT resid);
	void ResetToolbarImage();
	BOOL CloseApp();

	void RegistChecker();
	void TileWindows(WORD wMode);
	void CascadeWindows();
	void SaveProfile();
	void CleanUp();
	int ListupShellNewItems();
	int InitShellNewMenu(CMenuHandle pop);
	HWND SearchSiblings(BOOL bBound);
	void RepositionFrames(BOOL bNext);
	void SetDropDownButton(HWND hBar);
	void SaveToolBarState(HWND hBar, int nEntry);
	void RestoreToolBarState(HWND hBar, int nEntry);
	void BuildButtons(HWND hBar, LPBYTE pcbBtnSeg, int len);
	void SaveToolBarStateAll();

	void RefuseToFolder(LPITEMIDLIST pidlTarget, LPITEMIDLIST pidlRefuse);
	void QuickAccessMenu(const CPoint& pt);
	void TabbarShowAny();

	LPCITEMIDLIST GetTabFolder(int index, CFileBrowseWindow& browser);
	bool TabNew(LPITEMIDLIST pidl, bool bActivate = true, bool bAppend = false, bool bForce = false, ICONLISTSTYLE* pStyle = NULL);
	void TabClose(int index);
	void TabCloseAnyOther(int index);
	void TabCloseLeftSide(int index);
	void TabCloseRightSide(int index);
	void TabValidate(LPCITEMIDLIST pidlSource);
	void TabRefresh(int index);
	void TabExport(int index);
	void ToggleTabLock(int index);
	void SelectTab(int index);

	void UpdateFileBrowserListStyle(const CIconListStyle& ils);

	void BarSettings(POINT point);
	void LockBars(bool bLock);

	void FileFilterList(const CPoint& pt);
	void MaskSelectList(const CPoint& pt);

public:
	bool TabGroupSave(LPCTSTR pszTabGroupName);
	int TabGroupLoad(LPCTSTR pszTabGroupName);

public:
	CSimpleArray<CToolBarCtrl*>& GetToolBarArray();

	CToolBarCtrl* CreateEmptyToolBar(LPCTSTR pszTitle);
	BOOL DeleteToolBar(int index);
	void RecalcSmartBar();
	void RecalcDriveBar();
	void SetupDriveBar();
	void ShowBar(HWND hwndBar, BOOL bShow);
	void SetFileFilter(const CFilterSetting* pfs);

	inline void SetStatusbarIcon(int idPane, HICON hIcon) { m_statbar.SetPaneIcon(idPane, hIcon); }

	void TrackMenuForFileBrowser(CPoint pt);
	void TrackMenuForFolderTree(CPoint pt);
	BOOL TrackMenu(HMENU hmenu, UINT fuFlags, CPoint pt, bool bApplyAccel = true, TRACKMENUCUSTOMPROC pfn = NULL);
	BOOL TrackMenu(UINT idPopup, UINT fuFlags, CPoint pt, bool bApplyAccel = true, TRACKMENUCUSTOMPROC pfn = NULL);

	int DoToolBarDropDown(TBNOTIFY* ptbn);

	CFolderTreeWindow GetFolderTree() { ATLASSERT(m_baseTree.IsWindow()); return CFolderTreeWindow(m_baseTree); }

	CDocument* GetDocument()
	{
		if (m_pCurrentDocument == NULL) {
			int index = m_tabbar.GetCurSel();
			ATLASSERT(index >= 0);
			m_pCurrentDocument = (CDocument*)m_tabbar.GetItemData(index);
		}
		return m_pCurrentDocument;
	}
	void SwitchDocument(int indexOld, int indexNew);
	int GetDocumentIndex(CDocument* pdoc);

#ifndef DEBUG
	inline CFileBrowseWindow GetCurrentFileBrowser() { return GetDocument()->GetFileBrowser(); }
	inline CHistory* GetHistory() { return GetDocument()->GetHistory(); }
	inline CFilterSetting* GetFilter() { return GetDocument()->GetFilter(); }
#else
	CFileBrowseWindow GetCurrentFileBrowser()
	{
		CDocument* pDoc = GetDocument();
		ATLASSERT(pDoc != NULL);
		CFileBrowseWindow browser = pDoc->GetFileBrowser();
		ATLASSERT(browser);
		return browser;
	}
	CHistory* GetHistory()
	{
		CDocument* pDoc = GetDocument();
		ATLASSERT(pDoc != NULL);
		return pDoc->GetHistory();
	}
	CFilterSetting* GetFilter()
	{
		CDocument* pDoc = GetDocument();
		ATLASSERT(pDoc != NULL);
		return pDoc->GetFilter();
	}
#endif
	LPCITEMIDLIST GetCurrentTabFolder()
	{
		CFileBrowseWindow browser;
		return GetTabFolder(m_tabbar.GetCurSel(), browser);
	}
	void GetBrowserPaneRect(RECT* prc);

	void UpdateStatusbarImageSize(const CIconListStyle& ls);

// Generated message map functions
protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnMySize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnDestroy();
	BOOL OnEraseBkgnd(HDC hdc) { return TRUE; }
	LRESULT OnInitMenuPopup(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnTimer(UINT_PTR nEventID);
	LRESULT OnHandlerNcPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHandlerCommand(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHandlerMenuSelect(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnDropFiles(HDROP hDropInfo);
	BOOL OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState);
	void OnSetFocus(HWND hOldWnd);
	void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT lpdi);
	void OnExecMsdos();
    void OnExecShell();
	void OnExecOwn();
	void OnTabGroupSave();
	void OnTabGroupLoad();
	void OnWindowCombineSave();
	void OnWindowCombineRestore();
	void OnWindowCascade();
	void OnWindowFileStack();
	void OnWindowTileHorz();
	void OnWindowTileVert();
	void OnWindowMaxmize();
	void OnWindowMinimize();
	void OnClose(BOOL& bHandled);
	void OnEditPaste();
	void OnEditPasteShortcut();
#ifdef SUPPORT_FINDURL
	void OnToolFindUrl();
#endif
	void OnIconEnlarge();
	void OnViewDriveInfo();
	void OnViewIcon();
	void OnViewSize();
	void OnViewDate();
	void OnViewAttrib();
	void OnViewNameEllipsis();
	void OnViewSetDefaultStyle();
	void OnViewSort();
	void OnViewDesktopTree();
	void OnIconRelax();
	void OnTypeFileSuperHidden();
	void OnTypeFileHidden();
	void OnTypeFolder();
	void OnSetFilter();
	void OnReleaseFileFilter();
	void OnFileFilterList();
	void OnShortcutSetup();
	void OnViewKeyMap();
	void OnDiskSelect();
//@	void OnDiskCopy();
//@	void OnDiskFormat();
//@	void OnDiskLabel();
//@	void OnDiskMakeImage();
//@	void OnDiskRestoreImage();
//@	void OnDiskCheck();
	void OnHistoryBack();
	void OnHistoryForward();
	void OnDirAddFavorite();
	void OnQuickAccess();
	void OnMakeDir();
	void OnEndSession(BOOL bEnding);
	void OnNextWindow();
	void OnPrevWindow();
	void OnTabNew();
	void OnTabPrev();
	void OnTabNext();
	void OnTabClose();
	void OnTabCloseAnyOther();
	void OnTabCloseLeftSide();
	void OnTabCloseRightSide();
	void OnTabExport();
	void OnTabLock();
	void OnTabSortRange(UINT nID, UINT code, HWND hwnd);
	void OnAppExit();
	void OnAppExitAll();
	void OnRefresh();
	void OnNetConnect();
	void OnNetDisconnect();
	void OnNetAddPlace();
	void OnBarMenu();
	void OnBarProp();
	void OnBarSort();
	void OnBarSmart();
	void OnBarApp();
	void OnBarAppSideChange();
	void OnBarStat();
	void OnBarFunction();
	void OnBarDrive();
	void OnBarDriveVertRange(UINT id, UINT code, HWND hwnd);
	void OnBarNavi();
	void OnBarFavo();
	void OnBarLargeIcon();
	void OnBarAutoReposition();
	void OnBarLock();
	void OnBarSettings();
	void OnToolBarMenu(UINT nID, UINT code, HWND hwnd);
	void OnDirUp();
	void OnDirScriptSync();
	void OnOpenFolderSyncDataFolder();
	void OnHistoryTrace();
	void OnDirHistory();
	void OnHistoryClear();
	void OnDirHome();
	void OnDirEmptyRecycleBin();
	void OnDirMakeBriefcase();
//@	void OnDirLinkCheck();
	void OnDirRoot();
	void OnDirGoto();
	void OnFolderSync();
	void OnFileSearch();
	void OnFileShellNew();
	void OnFileNewFile();
	void OnFileRenExt();
	void OnSetSelect();
	void OnRevSelect();
	void OnMaskSelect();
	void OnMaskSelectList();
	void OnDoScriptSync(UINT nID, UINT code, HWND hwnd);
	void OnHotKeyRange(UINT nID, UINT code, HWND hwnd);
	void OnContextMenu(HWND hWnd, CPoint point);
	void OnOptionShellExt();
	void OnOptionEnv(UINT idd = NULL);
	void OnEditKeyMap();
	void OnPopupHotkeyMenu();
	void OnAppAbout();
	void OnCommandTabReopenRecent();
	void OnCommandSpecialFolder(UINT nID, UINT code, HWND hwnd);
	void OnCommandShellNew(UINT nID, UINT code, HWND hwnd);
	void OnCommandDriveAccess(UINT nID, UINT code, HWND hwnd);
	void OnCommandTabReopenRange(UINT nID, UINT code, HWND hwnd);
	void OnCommandTabGroupRange(UINT nID, UINT code, HWND hwnd);
	void OnDriveBarCommandRange(UINT nID, UINT code, HWND hwnd);
	void OnToolGrep();
//@	void OnToolSlideShow();
	void OnHelpShareware() { fvApp.Help(HB_CMD, IDM_HELP_SHAREWARE); }
	void OnWebFeedBack() { WebCall(m_hWnd, IDS_WEB_FEEDBACK); }
	void OnWebDownload() { WebCall(m_hWnd, IDS_WEB_DOWNLOAD); }
	void OnWebHomePage() { WebCall(m_hWnd, IDS_WEB_HOMEPAGE); }
	void OnDrivebarLButtonDown(NMHDR* pNmhdr, LRESULT* pResult);
	void OnDrivebarNewWindow(NMHDR* pNmhdr, LRESULT* pResult);
	void OnDrivebarNewTab(NMHDR* pNmhdr, LRESULT* pResult);
	void OnDrivebarAddFavorite(NMHDR* pNmhdr, LRESULT* pResult);
	void OnTabbarNotifyGetInfotip(NMHDR* pNmhdr, LRESULT* pResult);
	void OnTabbarNotifySelChange(NMHDR* pNmhdr, LRESULT* pResult);
	void OnTabbarNotifyBeginDrag(NMHDR* pNmhdr, LRESULT* pResult);
	void OnTabbarNotifyCloseButtonClicked(NMHDR* pNmhdr, LRESULT* pResult);
	void OnTabbarNotifyAddButtonClicked(NMHDR* pNmhdr, LRESULT* pResult);
	void OnTabbarNotifyDeleteItem(NMHDR* pNmhdr, LRESULT* pResult);
	void OnTabbarNotifyDblClk(NMHDR* pNmhdr, LRESULT* pResult);
	void OnFavoviewNotifyDblClk(NMHDR* pNmhdr, LRESULT* pResult);
	void OnFolderTreeNotifySelChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnFolderTreeNotifyEnterFileOperation(NMHDR* pNmhdr, LRESULT* pResult);
	void OnStatusbarNotifyClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnWinIniChange(LPCTSTR lpszSection);
	void OnToolbarBeginAdjust(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarEndAdjust(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarChange(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarQueryInsert(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarQueryDelete(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarGetButtonInfo(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarCustomHelp(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarReset(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnToolbarBeginDrag(UINT nID, NMHDR* pNmhdr, LRESULT* pResult);
	void OnMenubarTakeFocus(WPARAM wParam, LPARAM lParam);
	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
	LRESULT OnJump(WPARAM wParam, LPARAM lParam);
	LRESULT OnOpenFolder(WPARAM wParam, LPARAM lParam);
	LRESULT OnOpenTabFolder(WPARAM, LPARAM lParam);
	__inline LRESULT OnOpenTabGroup(WPARAM, LPARAM lParam) { return (TabGroupLoad((LPCTSTR)lParam) > 0); }
	__inline void OnAddFavorite(WPARAM wParam, LPARAM lParam) { m_viewFavo.InsertFavoriteItem((LPCITEMIDLIST)lParam); }
	__inline LRESULT OnSelectFolderItem(WPARAM wParam, LPARAM lParam) { return (LRESULT)SelectFolderItem((LPCITEMIDLIST)lParam); }
	LRESULT OnGetTabsFolderIDList(WPARAM wParam, LPARAM lParam);
	__inline void OnClearDocumentHistoryAll(WPARAM wParam, LPARAM) { ClearDocumentHistoryAll(); }
	LRESULT OnQueryImageList(WPARAM wParam, LPARAM lParam);
	__inline void OnActivateNextView(WPARAM wParam, LPARAM) { m_ViewSwitch.ActivateNextView((BOOL)wParam); }
	__inline void OnSetActiveView(WPARAM wParam, LPARAM lParam) { m_ViewSwitch.SetActiveView((HWND)lParam, (bool)(wParam != 0)); }
	__inline void OnEnableAccel(WPARAM wParam, LPARAM) { EnableAccel((bool)(wParam != 0)); }
	LRESULT OnRegThanks(WPARAM wParam, LPARAM lParam);
	LRESULT OnRegOK(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdateFavorite(WPARAM wParam, LPARAM lParam);
	LRESULT OnSaveAppProp(WPARAM wParam, LPARAM lParam);
	LRESULT OnFrameIsIdle(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_bInitCompleted; }
	LRESULT OnShellItemSearch(WPARAM wParam, LPARAM lParam);
	BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

	void OnSortRange(UINT id, UINT code, HWND hwnd);
	void OnIconModeRange(UINT id, UINT code, HWND hwnd);
	void OnHotIconModeRange(UINT id, UINT code, HWND hwnd);
	void OnThumbnailSizeRange(UINT id, UINT code, HWND hwnd);
	void OnThumbnailZoomIn();
	void OnThumbnailZoomOut();

	void OnNaviBarSetFocus();
	void OnNaviBarClicked();

	static int CALLBACK ApplyMaskFilter(const CFilterSetting* pfs);
	static int __cdecl __doc_sort(const void* arg1, const void* arg2, const void* param);
};


/////////////////////////////////////////////////////////////////////////////

extern CMainFrame* gpMainFrame;

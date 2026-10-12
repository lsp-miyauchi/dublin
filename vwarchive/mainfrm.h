#if !defined(AFX_EDFRAME_H__80DFFB1E_B73F_4C81_A240_31D47F4891B5__INCLUDED_)
#define AFX_EDFRAME_H__80DFFB1E_B73F_4C81_A240_31D47F4891B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// mainfrm.h : ヘッダー ファイル
//

#include "..\lscore\atl_special_split.h"
#include "..\lscore\xrebar.h"
#include "..\lscore\tbb_customdraw.h"
#include "..\lscore\taskbarlist.h"
#include "mypanecontainer_ex.h"

#define MY_COMMANDBAR
#ifndef MY_COMMANDBAR
	#include <atlctrlw.h>
	#define CMenuBarCtrl		CCommandBarCtrl
#else
	#include "..\lscore\menubar.h"
#endif

#include "resource.h"
#include "wrapper.h"
#include "arcitem.h"
#include "dropper.h"
#include "mousewheeler.h"

#define FRAMECLASS				_T("vwarcFrame")
#define IDW_TREE				1001
#define IDW_LIST				1002
#define MAXHEADITEM				8
#define LVS_EX_HOTTRACK			(LVS_EX_TWOCLICKACTIVATE | LVS_EX_UNDERLINEHOT)


typedef struct tagLISTHEAD {
	LPTSTR pszText;
	int fmt;
	int cx;
} LISTHEAD;



/////////////////////////////////////////////////////////////////////////////
// CMainFrame フレーム

class CMainFrame
		: public CFrameWindowImpl<CMainFrame>
		, public CDropperImpl<CMainFrame>
		, public CUpdateUI<CMainFrame>
		, public CMessageFilter
		, public CIdleHandler
		, public CTaskbarListImpl<CMainFrame>
		, public CTBarCustomDrawImpl<CMainFrame>
		, public CMouseWheelServerImpl<CMainFrame>
{
public:
	DECLARE_FRAME_WND_CLASS(FRAMECLASS, IDR_ARCVIEW)

	CMainFrame()
	{
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	virtual void TaskbarButtonCreated(ITaskbarList3* pTaskbarList);

	BEGIN_MSG_MAP(CMainFrame)
		CHAIN_MSG_MAP(CTaskbarListImpl<CMainFrame>)
		CHAIN_MSG_MAP(CDropperImpl<CMainFrame>)
		CHAIN_MSG_MAP(CTBarCustomDrawImpl<CMainFrame>)
		CHAIN_MSG_MAP(CMouseWheelServerImpl<CMainFrame>)
		ON_WM_CREATE()
		ON_WM_ERASEBKGND()
		ON_WM_CLOSE()
		ON_WM_CONTEXTMENU()
		ON_WM_APPCOMMAND()
		ON_WM_DESTROY()
		ON_WM_HELPINFO()
		ON_COMMAND(ID_PANE_CLOSE, OnPaneClose)
		ON_COMMAND(ID_APP_EXIT, OnAppExit)
		ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		ON_COMMAND(IDM_CLOSE, OnArcClose)
		ON_COMMAND(IDM_NEW, OnArcNew)
		ON_COMMAND(IDM_SETUP, OnOptionSetup)
		ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
		ON_COMMAND(IDM_VIEW_FOLDER, OnFolderTree)
		ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		ON_COMMAND(ID_HELP, OnAppHelp)
		ON_COMMAND_RANGE(IDM_SORT_UP, IDM_SORT_DOWN, OnSortSeqRange)
		ON_COMMAND_RANGE(IDM_SORT_NONE, IDM_SORT_PATH, OnSortTypeRange)
		ON_COMMAND_RANGE(IDM_ICON_LARGE, IDM_ICON_REPORT, OnIconRange)

		// リストビュー
		ON_NOTIFY(LVN_BEGINDRAG, IDW_LIST, OnLVNotifyBeginDrag)
		ON_NOTIFY(LVN_ITEMCHANGED, IDW_LIST, OnLVNotifyItemChanged)
		ON_NOTIFY(LVN_KEYDOWN, IDW_LIST, OnLVNotifyKeyDown)
		ON_NOTIFY(NM_DBLCLK, IDW_LIST, OnLVNotifyDblClick)
		ON_NOTIFY(NM_CUSTOMDRAW, IDW_LIST, OnLVCustomDraw)
		ON_NOTIFY(LVN_COLUMNCLICK, IDW_LIST, OnLVNotifyColumnClick)
//@		ON_NOTIFY(LVN_GETDISPINFO, IDW_LIST, OnLVNotifyGetDispInfo)
		ON_NOTIFY(LVN_GETINFOTIP, IDW_LIST, OnLVNotifyGetInfoTip)
		ON_COMMAND(IDM_CLIP, OnClipItem)
		ON_COMMAND(IDM_OPEN, OnExecItem)
		ON_COMMAND(IDM_VIEW, OnViewItem)
		ON_COMMAND(IDM_BDUMP, OnBDumpItem)
		ON_COMMAND(IDM_EXTRACT, OnExtractItem)
		ON_COMMAND(IDM_EXTRACTALL, OnExtractAllItem)
		ON_COMMAND(IDM_MAKESFX, OnMakeSfx)
		ON_COMMAND(IDM_DELETE, OnDeleteItem)
		ON_COMMAND(IDM_RENAME, OnRenameItem)
		ON_COMMAND(IDM_SELALL, OnSelectAll)
		ON_COMMAND(IDM_SELREV, OnSelectRev)

		// ツリービュー
		ON_NOTIFY(TVN_DELETEITEM, IDW_TREE, OnTVDeleteItem)
		ON_NOTIFY(TVN_KEYDOWN, IDW_TREE, OnTVKeyDown)
		ON_NOTIFY(TVN_SELCHANGED, IDW_TREE, OnTVSelChanged)
		ON_NOTIFY(NM_CUSTOMDRAW, IDW_TREE, OnTVCustomDraw)
		ON_WM_TIMER()

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(IDM_RENAME,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_DELETE,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_MAKESFX,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_SORT_UP,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_DOWN,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_NAME,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_EXT,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_TIME,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_SIZE,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_ATTR,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_PATH,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_NONE,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_VIEW_FOLDER,	UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_ICON_LARGE,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_ICON_LIST,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_ICON_REPORT,	UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	int DoToolBarDropDown(TBNOTIFY* ptbn);

private:
	// メッセージハンドラ
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnAppExit();
	void OnDestroy();
	void OnClose(BOOL& bHandled);
	BOOL OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState);
	void OnContextMenu(HWND hWnd, CPoint point);
	BOOL OnEraseBkgnd(HDC hdc);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	LRESULT OnHandlerMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnPaneClose();
	void OnArcNew();
	void OnArcClose();
	void OnOptionSetup();
	void OnFileOpen();
	void OnSortSeqRange(UINT nID, UINT code, HWND hwnd);
	void OnSortTypeRange(UINT nID, UINT code, HWND hwnd);
	void OnIconRange(UINT nID, UINT code, HWND hwnd);
	void OnFolderTree();
	void OnAppAbout();
	void OnAppHelp();

	// リストビュー メッセーハンドラ
	void OnLVNotifyBeginDrag(NMHDR* pNmhdr, LRESULT* pResult);
	void OnLVNotifyItemChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnLVNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult);
	void OnLVNotifyDblClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnLVNotifyColumnClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnLVCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
//@	void OnLVNotifyGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	void OnLVNotifyGetInfoTip(NMHDR* pNmhdr, LRESULT* pResult);
	void OnClipItem();
	void OnExecItem();
	void OnViewItem();
	void OnBDumpItem();
	void OnExtractItem();
	void OnExtractAllItem();
	void OnMakeSfx();
	void OnDeleteItem();
	void OnRenameItem();
	void OnSelectAll();
	void OnSelectRev();

	// ツリービューメッセージハンドラ
	void OnTVDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	void OnTVKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	void OnTVSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void OnTVDeleteItem();
	void OnTVCustomDraw(NMHDR* pHdr, LRESULT* pResult);
	void OnTimer(UINT_PTR nIDEvent);

	// CMainFrameメンバ
	BOOL KeyTranslate(NMKEY* pKey);
	void Construct();
	void Destruct();
	void UpdateUIAll();
	HMENU GetSubMenuByPosition(int n1stPos, int n2ndPos);
	void TrackBarMenu(int n1stPos, int n2ndPos, POINT point);
	void UpdateTitle(LPCTSTR pszFile);
	HWND CreateToolbar();

	// リストビューメンバ
public:
	BOOL OnGetDragImage(SHDRAGIMAGE* imageptr);
	DWORD OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
private:
	void ListupAgain(bool bAdjustColumnWidth = false);
	DWORD GetSelectedFileSize();
	void ShowTotalInfo();
	void ShowSelInfo();
	void SortList(int nSortType, int nSortSeq);
	int CheckFileType(LPCTSTR pszFileName);
	LPCTSTR GetCurrentFile();
	void GetSortType(int& seq, int& type);
	BOOL IsShowAll();
	BOOL OpenArcFile(LPCTSTR pszName);
	BOOL AppendFiles(HDROP hDrop);
	BOOL SetNewArchive(LPTSTR lpNew);
	void ListSettingChange();

	void OnInitialUpdate();

	static INT_PTR CALLBACK EnumCallback(LPCTSTR pszName, LPARAM param);
	BOOL PerseArchive(LPCTSTR pszName);
	BOOL TemporaryExtract(LPTSTR lpFile, LPTSTR lpDir);

	void CalcFields();
	BOOL MakeRespFile(CString& resp);
	DWORD MakeRespFileByFilter(CString& resp, LPCTSTR pszFilter);
	BOOL ExtractItem(LPCTSTR pszExtractList);
	void SelectGroup(DWORD_PTR folderid);
	void ExtractFolder(LPCTSTR pszFolder);
	void DeleteFolder(LPCTSTR pszFolder);

	void ShowBoundary(int index, BOOL bShow);

	BOOL IsRowSelected(int iRow);
	void UseOurHighlighting(NMLVCUSTOMDRAW* pcd, BOOL bUseOurs);

	// ツリービューメンバ
	HTREEITEM FindTree(HTREEITEM hStart, LPCTSTR pszDir);
	HTREEITEM FindChild(HTREEITEM hParent, LPCTSTR pszDir);
	void InitializeTreeView();
	void InitFolder(LPCTSTR pszRoot);
	void InitFolderComplete();
	void TreeSettingChange();
	void AddFolder(LPCTSTR pszDir, DWORD_PTR key);
	HTREEITEM SelectFolder(LPCTSTR pszDir);
	HTREEITEM SelectParent();
	LPCTSTR GetCurrentFolderPath();
	DWORD_PTR GetCurrentFolderID();

public:
	BOOL FileOpen(LPCTSTR pszFile);

protected:
#ifdef CUSTOM_REBER_USING
	CXRebarCtrl m_rebar;						// リバー（標準）
#else
	CReBarCtrl m_rebar;							// リバー（標準）
#endif
	CMenuBarCtrl m_menubar;						// コマンドバー
	CMultiPaneStatusBarCtrl m_statusbar;		// ステータスバー
	CToolBarCtrl m_toolbar;						// ツールバー
	CSplitterWindow m_split;
	CMyPaneContainer m_container;
	CMWTreeViewCtrl m_tree;
	CMWListViewCtrl m_list;
	bool m_bMouseWheelProcessing;
	HICON m_hicCrypted;

	// リストビュー関連
	UINT m_nRM_GETDRAGIMAGE;
	CArcParse m_parse;
	DWORD m_dwSelect;
	DWORD m_nStat;
	CSimpleArray<CArcItem*> m_array;
	CSimpleArray<CString> m_tempfile;
	DWORD_PTR m_curkey;
	int m_nSortType;
	int m_nSortSeq;
	TCHAR m_szTempFolder[MAX_PATH * 2];
	WIN32_FIND_DATA m_wfd;
	HIMAGELIST m_himLocked;
	HIMAGELIST m_himl;
	HIMAGELIST m_hims;
	HBITMAP m_hbmUp;
	HBITMAP m_hbmDown;
	BOOL m_bRowHighlighted;
	CSize m_sizeLarge;
	CSize m_sizeSmall;

	// ツリービュー関連
	UINT_PTR m_timerid;
	int m_iSelImage;
	int m_iImage;
	HIMAGELIST m_himTree;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_EDFRAME_H__80DFFB1E_B73F_4C81_A240_31D47F4891B5__INCLUDED_)

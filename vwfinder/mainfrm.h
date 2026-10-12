// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\lscore\menubar.h"
#include "..\lscore\xrebar.h"
#include "fileitem.h"
#include "..\lscore\tbb_customdraw.h"
#include "..\lscore\taskbarlist.h"

#define IDW_LIST			1001


class CMainFrame :
		public CFrameWindowImpl<CMainFrame>,
		public CUpdateUI<CMainFrame>,
		public CMessageFilter,
		public CIdleHandler,
		public CTaskbarListImpl<CMainFrame>,
		public CTBarCustomDrawImpl<CMainFrame>
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_FRAME)

#ifdef CUSTOM_REBER_USING
	CXRebarCtrl m_rebar;
#else
	CReBarCtrl m_rebar;
#endif
	CListViewCtrl m_list;
	CToolBarCtrl m_toolbar;
	CMenuBarCtrl m_menubar;
	CMultiPaneStatusBarCtrl m_statusbar;
	CString m_strCmdLine;
	CString m_csPathName;

	int m_nListStat;
	int m_nSortType;
	int m_nSortSeq;
	int m_nMaxName;
	int m_nMaxExt;
	int m_nMaxSize;
	int m_nMaxDate;
	int m_nMaxAttr;
	int m_nMaxPath;
	int m_nMaxType;
	DWORDLONG m_dlTotal;
	DWORDLONG m_dlSelect;
	HIMAGELIST m_hImage;
	CSize m_sizeIcon;
	CBitmap m_bmUp;
	CBitmap m_bmDown;
	CAtlArray<CFileInfo*> m_stack;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	virtual void TaskbarButtonCreated(ITaskbarList3* pTaskbarList);

	void Construct();
	void CreateClient();
	HWND CreateToolbar();

	BEGIN_MSG_MAP(CMainFrame)
		CHAIN_MSG_MAP(CTaskbarListImpl<CMainFrame>)
		CHAIN_MSG_MAP(CTBarCustomDrawImpl<CMainFrame>)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_DRAWITEM()
		ON_WM_APPCOMMAND()
		ON_COMMAND(IDM_EXEC_FIND, OnExecFind)
		ON_COMMAND(ID_APP_EXIT, OnAppExit)
		ON_COMMAND_RANGE(IDM_SORT_NONE, IDM_SORT_DOWN, OnSortRange)
		ON_COMMAND(IDM_FILE_COPY, OnFileCopy)
		ON_COMMAND(IDM_FILE_MOVE, OnFileMove)
		ON_COMMAND(IDM_FILE_SELALL, OnFileSelAll)
		ON_COMMAND(IDM_FILE_SELREV, OnFileSelRev)
		ON_COMMAND(IDM_FILE_JUMP, OnFileJump)
		ON_COMMAND(IDM_FILE_CLEAR, OnFileClear)
		ON_COMMAND(IDM_FILE_RENAME, OnFileRename)
		ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
		ON_COMMAND(IDM_FILE_DELETE, OnFileDelete)
		ON_COMMAND(IDM_EDIT_COPY, OnEditCopy)
		ON_COMMAND(IDM_EDIT_CUT, OnEditCut)
		ON_COMMAND_RANGE(IDM_COPY_NAMEONLY, IDM_COPY_SHORTNAME, OnClipCopyRange)
		ON_COMMAND(IDM_TOOL_VIEWER, OnFileView)
		ON_COMMAND(IDM_TOOL_BDUMP, OnFileBinDump)
		ON_COMMAND(IDM_VIEW_FILEINFO, OnFileInfo)
		ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
		ON_COMMAND(IDM_TOOL_SETUP, OnToolSetup)
		ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		ON_COMMAND(ID_HELP, OnAppHelp)
		ON_NOTIFY(LVN_COLUMNCLICK, IDW_LIST, OnNotifyColumnClick)
		ON_NOTIFY(NM_RCLICK, IDW_LIST, OnNotifyRClick)
		ON_NOTIFY(NM_DBLCLK, IDW_LIST, OnNotifyDblClick)
		ON_NOTIFY(LVN_KEYDOWN, IDW_LIST, OnNotifyKeyDown)
		ON_NOTIFY(LVN_GETINFOTIP, IDW_LIST, OnNotifyGetInfoTip)
		ON_NOTIFY(LVN_BEGINDRAG, IDW_LIST, OnNotifyBeginDrag)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
//@		UPDATE_ELEMENT(ID_FILE_SAVE,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_EDIT_CUT,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_EDIT_COPY,		UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(IDM_SORT_NONE,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_NAME,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_EXT,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_TIME,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_SIZE,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_ATTR,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_TYPE,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_PATH,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_UP,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDM_SORT_DOWN,		UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	int OnCreate(LPCREATESTRUCT pcs);
	LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnDestroy();
	BOOL OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState);
	void OnExecFind();
	void OnAppExit() { PostMessage(WM_CLOSE); }
	void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi);
	void OnNotifyColumnClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyRClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyDblClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyGetInfoTip(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyBeginDrag(NMHDR* pNmhdr, LRESULT* pResult);
	void OnSortRange(UINT id, UINT code, HWND hwnd);
	void OnFileCopy() { DoCopyMove(TRUE); }
	void OnFileMove() { DoCopyMove(FALSE); }
	void OnFileSelAll();
	void OnFileSelRev();
	void OnFileJump();
	void OnFileDelete();
	void OnFileClear();
	void OnFileView();
	void OnFileBinDump();
	void OnFileInfo();
	void OnFileOpen();
	void OnFileRename();
	void OnEditCopy();
	void OnEditCut();
	void OnToolSetup();
	void OnAppAbout();
	void OnAppHelp();
	void OnClipCopyRange(UINT id, UINT code, HWND hwnd);

	void UpdateUIAll();

	int DoToolBarDropDown(TBNOTIFY* ptbn);

	HMENU GetSubMenuByPosition(int n1stPos, int n2ndPos);
	void TrackBarMenu(int n1stPos, int n2ndPos, POINT point);
	void ShowBoundary(int index, BOOL bShow);
	void ListupAgain();
	void CalcFields();
	void SortListup();
	void DoSort(int type, int seq);
	void ShowSelInfo();
	void ShowTotalInfo();
	int GetSelItems(int** ppSel);
	HDROP MakeDropFiles();
	IDataObject* MakeDataObject();
	void DoCopyMove(BOOL bCopy);
	void DoObjectCopy(bool bCut);
	void ValidateFileList();
	void ListToStack();
	void UpdateSelItems();


public:
	void SetCommandLine(LPCTSTR pszCmdLine) { m_strCmdLine = pszCmdLine; }
};

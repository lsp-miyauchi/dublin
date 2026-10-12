// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
//

#pragma once

#include "..\lscore\menubar.h"
#include "..\lscore\xrebar.h"
#include "..\lscore\tbb_customdraw.h"
#include "..\lscore\taskbarlist.h"
#include "recentopenlist.h"

#define STS_TBSMALL			0x00000001

#define HEXEDIT_FRAME		_T("VWHEXEDIT_FRAME")

/////////////////////////////////////////////////////////////////////////////
//

class CMainFrame :
		public CFrameWindowImpl<CMainFrame>,
		public CUpdateUI<CMainFrame>,
		public CMessageFilter,
		public CIdleHandler,
		public CTaskbarListImpl<CMainFrame>,
		public CTBarCustomDrawImpl<CMainFrame>
{
public:
	DECLARE_FRAME_WND_CLASS(HEXEDIT_FRAME, IDR_BJAIL)

	CBeView m_view;
#ifdef CUSTOM_REBER_USING
	CXRebarCtrl m_rebar;
#else
	CReBarCtrl m_rebar;
#endif
	CMenuBarCtrl m_menubar;
	CMultiPaneStatusBarCtrl m_statusbar;
	CToolBarCtrl m_toolbar;						// ツールバー
	CString m_strCmdLine;
	CString m_csPathName;
	BOOL m_bModified;
	CColoredProgressCtrl m_progress;
#if _WTL_CMDBAR_VISTA_MENUS
	CRecentOpenFileList m_mru;
#else
	CRecentDocumentList m_mru;
#endif	// _WTL_CMDBAR_VISTA_MENUS
	DWORD m_nStat;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	virtual void TaskbarButtonCreated(ITaskbarList3* pTaskbarList);

	BEGIN_MSG_MAP(CMainFrame)
		CHAIN_MSG_MAP(CTaskbarListImpl<CMainFrame>)
		CHAIN_MSG_MAP(CTBarCustomDrawImpl<CMainFrame>)
		ON_WM_CREATE()
		MESSAGE_HANDLER(WM_CLOSE, OnClose)		// ハンドルしたかどうかを返す必要有り
		ON_WM_DESTROY()
		ON_WM_DROPFILES()
		ON_MESSAGE(WM_COPYDATA, OnCopyData)
		ON_WM_APPCOMMAND()
		ON_WM_COMMAND()
		ON_REGISTERD_MESSAGE(nRM_PROCREAD, OnProcessParallelReading)
		ON_REGISTERD_MESSAGE(nRM_DATAREQUEST, OnProcessDataRequest)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_FILE_SAVE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_FILE_SAVE_AS,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_UNDO,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_CUT,				UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_COPY,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_TEXTCOPY,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_PASTE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_FIND,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_SELSAVENEW,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_BYNEWEDITOR,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_SELTO_PREVMARK,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_SELTO_NEXTMARK,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_FIND_PREV,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_FIND_NEXT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MARK_CLEARALL,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_MARK_PREV,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MARK_NEXT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_JUMP_FILETOP,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_JUMP_FILEEND,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_JUMP_LASTEDIT,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TEXT_ASCII,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TEXT_SJIS,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TOOL_CHECKSUM,		UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	int OnCreate(LPCREATESTRUCT pcs);
	HWND CreateToolbar();
	LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnDestroy();
	void OnDropFiles(HDROP hdrop);
	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
	BOOL OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState);
	BOOL OnCommand(UINT idCtrl, UINT code, HWND hwnd);
	void OnFileRecent(UINT idCtrl, UINT code, HWND hWnd);
	void OnFileNew();
	void OnFileOpen();
	void OnFileExit();
	void OnFileSave();
	void OnFileSaveAs();
	void OnAppAbout();
	void OnAppHelp();
	void OnToolbarSmall();
	LRESULT OnProcessParallelReading(WPARAM code, LPARAM param);
	LRESULT OnProcessDataRequest(WPARAM code, LPARAM param);

	void UpdateUIAll();
	LRESULT OnRefreshStatusBar();

public:
	void SetCommandLine(LPCTSTR pszCmdLine);
 	void Modified(BOOL bModified);
	void SetAppFrameTitle();

	BOOL CheckFileSave();
	BOOL OpenFile(LPCTSTR pszName, BOOL bVolatile = FALSE);

};

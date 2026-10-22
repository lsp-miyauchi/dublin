// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\lscore\menubar.h"
#include "..\lscore\xrebar.h"
#include "..\lscore\tbb_customdraw.h"
#include "..\lscore\taskbarlist.h"

#include "common.h"
#include "padview.h"
#include "filebrowse.h"
#include "grep.h"
#include "findstring.h"
#include "reg.h"
#include "recentopenlist.h"

/////////////////////////////////////////////////////////////////////////////
//

typedef struct tagFINDREPLACEINFO {
	FINDREPLACE fr;
	TCHAR szFindWhat[MAX_PATH];
	TCHAR szReplaceWith[MAX_PATH];
} FINDREPLACEINFO;

#define FRAMECLASS		_T("vwtxtFrame")
#define ISDOT(a)		((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))

#define STS_TBSMALL					0x00000001

/////////////////////////////////////////////////////////////////////
// CEnterFolder

class CEnterFolder
{
public:
	explicit CEnterFolder(LPCTSTR lpNewPath)
	{
		m_dwLastError = 0;
		int len = MAX_PATH * 2;
		GetCurrentDirectory(len, m_oldPath.GetBuffer(len));
		if (!SetCurrentDirectory(lpNewPath))
			m_dwLastError = GetLastError();
	}
	~CEnterFolder()
	{
		Exit();
	}

	BOOL IsFolderChanged() const
	{
		return (m_dwLastError == 0) ? TRUE : FALSE;
	}

	void Exit() const
	{
		if (!m_dwLastError)
			SetCurrentDirectory(m_oldPath);
	}

	CString m_oldPath;
	DWORD m_dwLastError;
};

/////////////////////////////////////////////////////////////////////////////
//

class CMainFrame :	public CFrameWindowImpl<CMainFrame>,
					public CUpdateUI<CMainFrame>,
					public CMessageFilter,
					public CIdleHandler,
					public CDocumentEventHandler,
					public CCursorEventHandler,
					public CTaskbarListImpl<CMainFrame>,
					public CTBarCustomDrawImpl<CMainFrame>
{
private:
	CPadView m_view;
#ifdef CUSTOM_REBER_USING
	CXRebarCtrl m_rebar;						// リバー
#else
	CReBarCtrl m_rebar;							// リバー
#endif
	CMenuBarCtrl m_menubar;						// コマンドバー
	CToolBarCtrl m_toolbar;						// ツールバー
	CMultiPaneStatusBarCtrl m_statusbar;		// ステータスバー
	HACCEL m_hLocalAccel;
	HICON m_hicRO;								// 編集不可アイコン
	DWORD m_nStat;

	VConfig m_config;							// 設定情報

	CFind m_Finder;
	CReplaceProg m_FindReplaceDlg;
	FINDREPLACEINFO m_fri;

	CString m_strFileName;
	CString m_strTitle;
	CCharSetList m_CharSetList;
	int m_csi;
	int m_lb;

	bool m_bCountCharByUnicode;
	bool m_bOpenSameWindow;
	bool m_bUseAutoDetect;
	bool m_bDocumentUpdated;

#if _WTL_CMDBAR_VISTA_MENUS
	CRecentOpenFileList m_mru;
#else
	CRecentFileList m_mru;
#endif	// _WTL_CMDBAR_VISTA_MENUS

public:
	DECLARE_FRAME_WND_CLASS(FRAMECLASS, IDR_FRAME)

	CMainFrame();
	void InitialUpdate();
	bool StartEdit(LPCTSTR pszFile, int cs = CS_AUTODETECT, int line_number = -1, bool rom = false);

	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnIdle() override;
	void TaskbarButtonCreated(ITaskbarList3* pTaskbarList) override;

	void UpdateUIAll();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_FILE_SAVE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_FILE_OPENCLIPBOARD,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_REDO, 			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_UNDO,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_PASTE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_COPY,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_COPY_FOLLOW,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_COPY_APPEND,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_CUT,				UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_DELETE_CHAR,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_DELETE_LINE,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_DELETE_LEFT,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_DELETE_RIGHT,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_UPPERCASE,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_LOWERCASE,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_READONLY,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_FIND_NEXT,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_FIND_PREV,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_JUMP_LASTEDITPOS,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_ENCODE_AUTO,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TOOLBAR_SMALL,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_LINEBREAK_CRLF,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_LINEBREAK_CR,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_LINEBREAK_LF,			UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		CHAIN_MSG_MAP(CTaskbarListImpl<CMainFrame>)
		CHAIN_MSG_MAP(CTBarCustomDrawImpl<CMainFrame>)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_CLOSE()
		ON_WM_ACTIVATE()
		ON_WM_DROPFILES()
		ON_WM_CONTEXTMENU()
		ON_COMMAND(ID_APP_EXIT, OnFileExit)
		ON_COMMAND(ID_FILE_NEW, OnFileNew)
		ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
		ON_COMMAND(ID_FILE_OPENCLIPBOARD, OnFileOpenClipboard)
		ON_COMMAND(ID_FILE_SAVE, OnFileSave)
		ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
		ON_COMMAND(ID_APP_ESCAPE, OnAppEscape)
		ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		ON_COMMAND(ID_HELP, OnAppHelp)
		ON_COMMAND(ID_EDIT_CUT, OnEditCut)
		ON_COMMAND(ID_EDIT_CUT_APPEND, OnEditCutAppend)
		ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
		ON_COMMAND(ID_EDIT_COPY_APPEND, OnEditCopyAppend)
		ON_COMMAND(ID_EDIT_COPY_FOLLOW, OnEditCopyFollow)
		ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
		ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
		ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
		ON_COMMAND(ID_EDIT_DELETE_CHAR, OnEditDelete)
		ON_COMMAND(ID_EDIT_DELETE_LINE, OnEditDeleteLine)
		ON_COMMAND(ID_EDIT_DELETE_LEFT, OnEditDeleteLeft)
		ON_COMMAND(ID_EDIT_DELETE_RIGHT, OnEditDeleteRight)
		ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
		ON_COMMAND(ID_EDIT_UPPERCASE, OnEditUpperCase)
		ON_COMMAND(ID_EDIT_LOWERCASE, OnEditLowerCase)
		ON_COMMAND(ID_EDIT_DUPLICATELINE, OnEditDuplicateLine)
		ON_COMMAND(ID_EDIT_READONLY, OnEditReadOnly)
		ON_COMMAND(ID_FIND_SEARCH, OnFindSearch)
		ON_COMMAND(ID_FIND_SEARCH_BACK, OnFindSearchBack)
		ON_COMMAND(ID_FIND_PREV, OnFindPrev)
		ON_COMMAND(ID_FIND_NEXT, OnFindNext)
		ON_COMMAND(ID_FIND_REPLACE, OnFindReplace)
		ON_COMMAND(ID_FIND_GREP, OnFindGrep)
		ON_COMMAND(ID_FIND_FUNCTION, OnFindFunction)
		ON_COMMAND(ID_JUMP_LASTEDITPOS, OnJumpLastEditPos)
		ON_COMMAND(ID_JUMP_FILETOP, OnJumpFileTop)
		ON_COMMAND(ID_JUMP_FILEEND, OnJumpFileEnd)
		ON_COMMAND(ID_JUMP_TAG, OnJumpTag)
		ON_COMMAND(ID_JUMP_LINE, OnJumpToGoLine)
		ON_COMMAND(ID_PAGE_PREV, OnPageUp)
		ON_COMMAND(ID_PAGE_NEXT, OnPageDown)
		ON_COMMAND(ID_PAGE_PREV_HALF, OnPageHalfUp)
		ON_COMMAND(ID_PAGE_NEXT_HALF, OnPageHalfDown)
		ON_COMMAND(ID_VIEW_CHARCODE, OnShowCurCharCode)
		ON_COMMAND(ID_TOOLBAR_SMALL, OnToolbarSmall)
		ON_COMMAND(ID_OPTION_ENV, OnSetupProperty)
		ON_COMMAND(ID_OPTION_KEYMAP, OnSetupKeymap)
		ON_COMMAND_RANGE(ID_ENCODE_TYPE1, ID_ENCODE_LAST, OnEncodeRange)
		ON_COMMAND_RANGE(ID_LINEBREAK_CRLF, ID_LINEBREAK_LF, OnLineBreakRange)
		ON_COMMAND_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
		ON_REGISTERD_MESSAGE(nRM_FINDNOTIFY, OnFindNotify)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

private:
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	int OnCreate(LPCREATESTRUCT pcs);
	void OnClose(BOOL& bHandled);
	void OnDestroy();
	void OnActivate(UINT nState, HWND hWndOther, BOOL bMinimized);
	void OnDropFiles(HDROP hDropInfo);
	void OnContextMenu(HWND hWnd, CPoint pt);

	void OnFileOpenClipboard();				// クリップボードを開く
	void OnFileExit();
	void OnFileNew();
	void OnFileOpen();
	void OnFileSave();
	void OnFileSaveAs();
	void OnAppEscape();
	void OnAppAbout();
	void OnAppHelp();
	void OnEditCut();						// 切り取り
	void OnEditCutAppend();					// 追加して切り取り
	void OnEditCopy();						// コピ－
	void OnEditCopyFollow();				// 引用付きコピー
	void OnEditCopyAppend();				// 追加コピー
	void OnEditPaste();						// 貼り付け
	void OnEditUndo();						// やり直し
	void OnEditRedo();						// やり直しのやり直し
	void OnEditDelete();					// 削除
	void OnEditDeleteLine();				// 1行削除
	void OnEditDeleteLeft();				// 左削除
	void OnEditDeleteRight();				// 右削除
	void OnEditSelectAll();					// すべて選択
	void OnEditUpperCase();					// 大文字へ変換
	void OnEditLowerCase();					// 小文字へ変換
	void OnEditDuplicateLine();				// 1行複写
	void OnEditReadOnly();					// 書き込み禁止
	void OnFindSearch();					// 検索
	void OnFindSearchBack();				// 検索（後方）
	void OnFindPrev();						// 前を検索
	void OnFindNext();						// 次を検索
	void OnFindFunction();					// 関数検索
	void OnFindGrep();						// ファイルから検索
	void OnFindReplace();					// 置換
	LRESULT OnFindNotify(WPARAM wParam, LPARAM lParam);
	void OnJumpFileTop();					// 文書の先頭へ
	void OnJumpFileEnd();					// 文書の最後へ
	void OnJumpLastEditPos();				// 最後に編集した場所
	void OnJumpTag();						// タグジャンプ
	void OnJumpToGoLine();					// 指定行へ
	void OnPageUp();						// ページUp
	void OnPageDown();						// ページDown
	void OnPageHalfUp();					// 半ページUp
	void OnPageHalfDown();					// 半ページDown
	void OnShowCurCharCode();
	void OnSetupKeymap();
	void OnSetupProperty();
	void OnOpenRecentFile(UINT nID, UINT code, HWND hWnd);
	void OnEncodeRange(UINT id, UINT code, HWND hWnd);
	void OnLineBreakRange(UINT id, UINT code, HWND hWnd);
	void OnToolbarSmall();

	HWND CreateToolbar();

	// 継承イベントハンドラ
	void OnDocumentEventDirtyFlagChanged(bool dirty) override;
	void OnCursorEventMove( const DPos& c, const DPos& s ) override;
	void OnCursorEventMode( int modeType, bool mode) override;

	void JumpToLine(ULONG line_number);		// 指定行へ移動

	void ChangeTitle(LPCTSTR pszTitle = nullptr);
	void UpdateTitleNames();
	inline bool IsUntitled() const { return (m_strFileName.IsEmpty() != FALSE); }

	void ReloadConfig(bool noSetDocType = false);

	bool SaveAs();
	bool SaveIfNeeded();
	bool AskToSave();
	bool Save();
	bool Open(LPCTSTR pszFile, int cs);
	bool OpenByMyself(LPCTSTR pszFile, int cs, bool needReConf = true);

	void FindSearch(bool bForward = true);

private:
	HMENU GetSubMenuByPosition(int n1stPos, int n2ndPos);
	void InitEncodeMenu();
	void UIEncodeMenu();

	void MakeLocalAccelTable();
	void UpdateMenuAccelerator();

	void ProcessGrep();
	BOOL GrepTree(CGrep* pGrep, LPCTSTR pszPath, CGrepProg* pgpg);
	BOOL GrepFile(LPCTSTR pszFile, CGrep* pGrep, CGrepProg* pgpg);


	void ZapMessage();
};

// EdFrame.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "common.h"
#include "mainfrm.h"
#include "setup.h"

static int indicators[] =
{
	ID_DEFAULT_PANE,
	ID_INDICATOR_MARKER,
	ID_INDICATOR_SELECT,
	ID_INDICATOR_TOTAL
};

static LISTHEAD lhEntry[MAXHEADITEM] = {
	{ _T("名前"),	LVCFMT_LEFT,	20 }, 
	{ _T("サイズ"),	LVCFMT_RIGHT,	16 },
	{ _T("日時"),	LVCFMT_LEFT,	22 },
	{ _T("種類"),	LVCFMT_LEFT,	20 }, 
	{ _T("率"),		LVCFMT_RIGHT,	6  },
	{ _T("圧縮"),	LVCFMT_LEFT,	16 },
	{ _T("属性"),	LVCFMT_LEFT,	16 },
	{ _T("場所"),	LVCFMT_LEFT,	20 }
};

/////////////////////////////////////////////////////////////////////////////

static int CountSiblings(HWND hwndOwn)
{
	TCHAR cname[128], buf[128];
	::GetClassName(hwndOwn, cname, ARRAYSIZE(cname));

	int cnt = 0;
	HWND hwnd = ::GetTopWindow(::GetDesktopWindow());
	for (; hwnd; hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT)) {
		if (hwnd == hwndOwn) continue;
		if (::GetClassName(hwnd, buf, ARRAYSIZE(buf)) && !lstrcmp(cname, buf))
			cnt++;
	}
	return cnt;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UpdateUIAll();
	return FALSE;
}

void CMainFrame::TaskbarButtonCreated(ITaskbarList3* pTaskbarList)
{
/*
	if (pTaskbarList != NULL) {
		HICON hIcon = LoadIcon(RESINST, MAKEINTRESOURCE(IDR_ARCVIEW));
		if (hIcon) {
			pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);
			DestroyIcon(hIcon);
		}
	}
*/
}

void CMainFrame::UpdateUIAll()
{
	UIEnable(IDM_RENAME, m_parse.CanRename());
	UIEnable(IDM_DELETE, m_parse.CanDelete());
	UIEnable(IDM_MAKESFX, m_parse.CanMakeSfx());

	int type, seq;
	GetSortType(seq, type);
	UISetCheck(IDM_SORT_UP, seq == SORT_UP);
	UISetCheck(IDM_SORT_DOWN, seq == SORT_DOWN);
	UISetCheck(IDM_SORT_NAME, type == SORT_NAME);
	UISetCheck(IDM_SORT_EXT, type == SORT_EXT);
	UISetCheck(IDM_SORT_TIME, type == SORT_TIME);
	UISetCheck(IDM_SORT_SIZE, type == SORT_SIZE);
	UISetCheck(IDM_SORT_PATH, type == SORT_PATH);
	UISetCheck(IDM_SORT_ATTR, type == SORT_ATTR);
	UISetCheck(IDM_SORT_NONE, type == SORT_NONE);

	DWORD style = m_list.GetStyle() & LVS_TYPEMASK;
	UISetCheck(IDM_ICON_LARGE, style == LVS_ICON);
	UISetCheck(IDM_ICON_LIST, style == LVS_LIST);
	UISetCheck(IDM_ICON_REPORT, style == LVS_REPORT);

	UISetCheck(IDM_VIEW_FOLDER, m_tree.IsWindowVisible());

	// ツールバー
	UIUpdateToolBar();

	// ステータスバー
	ShowTotalInfo();
	ShowSelInfo();
}

BOOL CMainFrame::KeyTranslate(NMKEY* pKey)
{
	if (m_hAccel) {
		MSG msg;
		msg.hwnd = m_hWnd;
		msg.message = WM_KEYDOWN;
		msg.wParam = pKey->nVKey;
		msg.lParam = MAKELPARAM(0, pKey->uFlags);
		msg.time = GetMessageTime();
		msg.pt = CPoint(GetMessagePos());
		if (::TranslateAccelerator(m_hWnd, m_hAccel, &msg))
			return TRUE;
	}
	return FALSE;
}

void CMainFrame::Construct()
{
	m_bMouseWheelProcessing = false;

	m_hicCrypted = NULL;

	// リストビュー
	m_dwSelect = 0;
	m_himLocked = NULL;
	m_hbmUp = NULL;
	m_hbmDown = NULL;
	m_curkey = 0xFFFFFFFF;

	CAppReg reg;
	memset(&m_wfd, 0, sizeof(m_wfd));

	DWORD def = ATS_GRID | ATS_REPCHECKER;
	m_nStat = (DWORD)reg.GetValue(IDS_SEC_OPERATION, IDS_KEY_LISTSTATUS, def);
	m_nSortType = reg.GetValue(IDS_SEC_OPERATION, IDS_KEY_SORTTYPE, SORT_NAME);
	m_nSortSeq = reg.GetValue(IDS_SEC_OPERATION, IDS_KEY_SORTSEQUENCE, SORT_UP);
	m_bRowHighlighted = FALSE;
	m_himl = m_hims = NULL;

	// ツリービュー
	m_timerid = 0;
	m_himTree = NULL;
}

void CMainFrame::Destruct()
{
	// リストビュー
	if (m_himLocked)		ImageList_Destroy(m_himLocked);
	if (m_hbmUp)			DeleteObject(m_hbmUp);
	if (m_hbmDown)			DeleteObject(m_hbmDown);

	m_parse.Wep();
	int cnt = m_tempfile.GetSize();
	for (int i = 0; i < cnt; i++) {
		CString str = m_tempfile[i];
		if (PathFileExists(str)) {
			SetFileAttributes(str, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(str);
		}
	}

	// ツリービュー
	if (m_himTree) ImageList_Destroy(m_himTree);
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	Construct();

	HINSTANCE hInst = RESINST;

	// Rebar
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
#ifdef CUSTOM_REBER_USING
	m_rebar.SubclassWindow(m_hWndToolBar);
#else
	m_rebar = m_hWndToolBar;
#endif

	// メニューバー
	m_menubar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_menubar.AttachMenu(GetMenu());
	m_menubar.SetImageMaskColor(RGB(255, 0, 255));
	m_menubar.LoadMultipleImages(IDB_TOOLBAR_S);
	m_menubar.LoadMultipleImages(IDB_APPICON_S);
	SetMenu(NULL);
#ifdef CUSTOM_REBER_USING
	m_rebar.SetMenuBar(m_menubar);						// メニューバーを明示的に登録
#endif
	AddSimpleReBarBand(m_menubar, NULL, FALSE, 0, FALSE);

	// ツールバー作成
	HWND hWndToolBar = CreateToolbar();
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE, 0, FALSE);

	// lock rebar
	m_rebar.LockBands(true);


	// statusbar
	CreateSimpleStatusBar();
	m_statusbar.SubclassWindow(m_hWndStatusBar);
	m_statusbar.SetPanes(indicators, sizeof(indicators) / sizeof(int), false);
	m_hicCrypted = (HICON)LoadImage(RESINST, _MKIR(IDI_CRYPTED), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	CAppReg reg;

	// Spritter
	CRect rc;
	GetClientRect(&rc);
	m_split.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_split.SetSplitterExtendedStyle(0, SPLIT_RIGHTALIGNED | SPLIT_PROPORTIONAL);
	m_split.SetSplitterPos(reg.GetValue(IDS_SEC_MAIN, IDS_KEY_TREEWIDTH, 280));

	// リストビュー
	DWORD style = WS_CHILD | WS_VISIBLE | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS | LVS_REPORT;
	m_list.Create(m_split.m_hWnd, rcDefault, NULL, style, 0, IDW_LIST);
	m_split.SetSplitterPane(1, m_list.m_hWnd);

	// イメージリストのアサイン
	Shell_GetImageLists(&m_himl, &m_hims);

	ImageList_GetIconSize(m_himl, (int*)&m_sizeLarge.cx, (int*)&m_sizeLarge.cy);
	ImageList_GetIconSize(m_hims, (int*)&m_sizeSmall.cx, (int*)&m_sizeSmall.cy);
	m_list.SetImageList(m_himl, LVSIL_NORMAL);
	m_list.SetImageList(m_hims, LVSIL_SMALL);

	// 暗号化ファイル用ステートアイコンイメージリスト
	m_himLocked = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_LOCKED), 16, CLR_DEFAULT);

	m_list.SetTextColor(gColors[colText]);
	m_list.SetTextBkColor(gColors[colBack]);
	m_list.SetBkColor(gColors[colBack]);

	// カラム挿入とヘッダカスタマイズ
	TEXTMETRIC tm;
	CClientDC dc(m_list);
	HFONT hfont = dc.SelectFont(m_list.GetFont());
	dc.GetTextMetrics(&tm);
	dc.SelectFont(hfont);

	LVCOLUMN lvc;
	for (int i = 0; i < MAXHEADITEM; i++) {
		lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvc.fmt = lhEntry[i].fmt;
		lvc.cx = lhEntry[i].cx * tm.tmAveCharWidth;
		lvc.pszText = lhEntry[i].pszText;
		lvc.iSubItem = i;
		m_list.InsertColumn(i, &lvc);
	}

	// 拡張スタイル
	style = m_list.GetExtendedListViewStyle() | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	style &= ~(LVS_EX_GRIDLINES | LVS_EX_HOTTRACK);
	if (m_nStat & ATS_GRID)
		style |= LVS_EX_GRIDLINES;
#ifdef MOUSE_HOTTRACKING
	if (m_nStat & ATS_HOTTRACK)
		style |= LVS_EX_HOTTRACK;
#endif
	m_list.SetExtendedListViewStyle(style);

	// 前回のスタイルを適用
	style = reg.GetValue(IDS_SEC_MAIN, IDS_KEY_LISTSTYLE, LVS_REPORT);
	if (style != LVS_REPORT)
		m_list.SetViewType(style);

	// ツリービュー
	m_container.Create(m_split.m_hWnd, (LPCTSTR)NULL);
	style = WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS;
	m_tree.Create(m_container.m_hWnd, rcDefault, NULL, style, 0, IDW_TREE);
	m_container.SetClient(m_tree.m_hWnd);
	m_split.SetSplitterPane(0, m_container.m_hWnd);
	InitializeTreeView();

	// クライアント
	m_hWndClient = m_split.m_hWnd;
	m_split.SetActivePane(1);

	UIAddToolBar(hWndToolBar);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// 前回のウィンドウ位置
	WINDOWPLACEMENT wpm;
	if (reg.GetBinary(IDS_SEC_MAIN, IDS_KEY_WNDPOS, &wpm, sizeof(wpm)) == sizeof(wpm)) {
		CRect rc = wpm.rcNormalPosition;
		if (CountSiblings(m_hWnd) > 0) {
			GetWindowPlacement(&wpm);
			rc.OffsetRect(wpm.rcNormalPosition.left - rc.left, 0);
		}
		lccMapRectToProgmanPane(&rc);
		MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), FALSE);
	}

	// 作業フォルダの作成
	TCHAR buf[MAX_PATH * 2];
	GetTempPath(_COUNTOF(buf), buf);
	lccGetTempFileName(buf, _T("~am"), m_szTempFolder);
	CreateDirectory(m_szTempFolder, NULL);

	ImmAssociateContext(m_hWnd, NULL);
	ImmAssociateContext(m_tree, NULL);
	ImmAssociateContext(m_list, NULL);

	// 初期更新処理
	OnInitialUpdate();

	return 0;
}

HWND CMainFrame::CreateToolbar()
{
	if (m_toolbar.IsWindow())
		RemoveCustomDrawBar(m_toolbar.m_hWnd);

	HWND hWndToolBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(IDB_TOOLBAR_S), ATL_SIMPLE_TOOLBAR_PANE_STYLE, ATL_IDW_TOOLBAR);
	m_toolbar = hWndToolBar;

	// ドロップダウンスタイル
	TBBUTTONINFO tbi;
	m_toolbar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);		// | TBSTYLE_EX_HIDECLIPPEDBUTTONS
	tbi.cbSize = sizeof(tbi);
	tbi.dwMask = TBIF_STYLE;
	tbi.fsStyle = BTNS_WHOLEDROPDOWN;
	m_toolbar.SetButtonInfo(IDM_ICON_RANGE, &tbi);
	m_toolbar.SetButtonInfo(IDM_SORT_RANGE, &tbi);

	AddCustomDrawBar(hWndToolBar);
	return hWndToolBar;
}

void CMainFrame::OnAppExit()
{
	PostMessage(WM_CLOSE);
}

void CMainFrame::OnDestroy()
{
	// 作業フォルダの削除
	if (PathFileExists(m_szTempFolder)) {
		TCHAR dir[MAX_PATH * 2], windir[MAX_PATH * 2];
		GetCurrentDirectory(_COUNTOF(dir), dir);
		GetWindowsDirectory(windir, _COUNTOF(windir));
		SetCurrentDirectory(windir);

		fopRemoveTreeNoConfirm(m_szTempFolder);

		if (PathFileExists(dir))
			SetCurrentDirectory(dir);
	}

	// リストビュー
	m_parse.Wep();
	DWORD style = m_list.GetViewType();
	CAppReg reg;
	reg.SetValue(IDS_SEC_MAIN, IDS_KEY_LISTSTYLE, style);

	// ツリービュー
	if (m_timerid)
		KillTimer(m_timerid);
	m_tree.DeleteAllItems();

	// アイコン
	if (m_hicCrypted) DestroyIcon(m_hicCrypted);

	Destruct();

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
}

void CMainFrame::UpdateTitle(LPCTSTR pszFile)
{
	CString tit;
	tit.Format(_T("書庫 - %s"), PathFindFileName(pszFile));
	SetWindowText(tit);
}

void CMainFrame::OnClose(BOOL& bHandled) 
{
	// 分割位置記憶
	CAppReg reg;
	if (m_tree.IsWindowVisible()) {
		CRect rc;
		m_container.GetWindowRect(&rc);
		reg.SetValue(IDS_SEC_MAIN, IDS_KEY_TREEWIDTH, rc.Width());
	}

	// ウィンドウ位置
	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(wpm);
	if (GetWindowPlacement(&wpm))
		reg.SetBinary(IDS_SEC_MAIN, IDS_KEY_WNDPOS, &wpm, sizeof(wpm));
}

BOOL CMainFrame::OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState)
{
	UINT idCommand = 0;
	switch (appCommand) {
	case APPCOMMAND_CLOSE:				idCommand = ID_APP_EXIT;		break;
	}
	if (idCommand) {
		PostMessage(WM_COMMAND, idCommand, 0);
		return TRUE;
	}
	return FALSE;
}

HMENU CMainFrame::GetSubMenuByPosition(int n1stPos, int n2ndPos)
{
	HMENU hmenu = m_menubar.GetMenu();
	if (hmenu) {
		hmenu = ::GetSubMenu(hmenu, n1stPos);
		if (hmenu) {
			if (n2ndPos >= 0)
				hmenu = ::GetSubMenu(hmenu, n2ndPos);
			return hmenu;
		}
	}
	return NULL;
}

void CMainFrame::OnContextMenu(HWND hWnd, CPoint pt) 
{
	CRect rc;
	m_list.GetWindowRect(&rc);
	if (rc.PtInRect(pt)) {
		CPoint ptw = pt;
		m_list.ScreenToClient(&ptw);
		UINT flg;
		int index = m_list.HitTest(ptw, &flg);
		if (index >= 0 && flg & LVHT_ONITEM) {
			HMENU hmenu = GetSubMenuByPosition(1, -1);
			if (hmenu)
				m_menubar.TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, pt.x, pt.y);
		}
	}
}

BOOL CMainFrame::OnEraseBkgnd(HDC hdc)
{
	return TRUE;
}

BOOL CMainFrame::OnHelpInfo(HELPINFO* pHelpInfo) 
{
/* @@
	if (pHelpInfo->iContextType == HELPINFO_WINDOW) {
		WinHelp(HID_BASE_RESOURCE + IDR_ARCVIEW);
		return TRUE;
	}
@@ */
	return FALSE;
}

void CMainFrame::TrackBarMenu(int n1stPos, int n2ndPos, POINT point)
{
	HMENU hmenu = GetSubMenuByPosition(n1stPos, n2ndPos);
	if (hmenu)
		m_menubar.TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y);
}

int CMainFrame::DoToolBarDropDown(TBNOTIFY* ptbn)
{
	CPoint pt(ptbn->rcButton.left, ptbn->rcButton.bottom - 1);
	::ClientToScreen(ptbn->hdr.hwndFrom, &pt);

	switch (ptbn->iItem) {
	case IDM_ICON_RANGE:
		TrackBarMenu(2, 2, pt);
		break;
	case IDM_SORT_RANGE:
		TrackBarMenu(2, 3, pt);
		break;
	default:
		return TBDDRET_NODEFAULT;
	}
	return TBDDRET_DEFAULT;
}

BOOL CMainFrame::FileOpen(LPCTSTR pszFile)
{
	BOOL bOK = OpenArcFile(pszFile);
	if (bOK) {
		UpdateTitle(pszFile);
	}
	return bOK;
}

void CMainFrame::OnFileOpen() 
{
	TCHAR name[MAX_PATH * 2];
	name[0] = 0;
	if (FileBrowse(TRUE, m_hWnd, IDS_CAPTION_OPEN, IDS_FILTER_ARCHIVE, name))
		FileOpen(name);
}

void CMainFrame::OnArcClose()
{
	PostMessage(WM_CLOSE);
}

void CMainFrame::OnArcNew()
{
	TCHAR buf[MAX_PATH * 2];
	if (!SetNewArchive(buf)) return;

	SetWindowText(buf);
}

void CMainFrame::OnOptionSetup()
{
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	BOOL bServer = SUCCEEDED(hr);

	CAppReg reg;
	CSetupDlg dlg;
	dlg.m_ai.stat = (WORD)reg.GetValue(IDS_SEC_OPERATION, IDS_KEY_LISTSTATUS, ATS_GRID);
	if (!bServer || FAILED(pam->GetDefaultFolder(dlg.m_ai.path)))
		dlg.m_ai.path[0] = '\0';
	CopyMemory(dlg.m_ai.colors, gColors, sizeof(dlg.m_ai.colors));

	if (dlg.DoModal() == IDOK) {
		m_nStat = dlg.m_ai.stat;
		CAppReg reg;
		reg.SetValue(IDS_SEC_OPERATION, IDS_KEY_LISTSTATUS, m_nStat);

		if (memcmp(dlg.m_ai.colors, gColors, sizeof(dlg.m_ai.colors))) {
			CopyMemory(gColors, dlg.m_ai.colors, sizeof(dlg.m_ai.colors));
			reg.SetBinary(IDS_SEC_OPERATION, IDS_KEY_COLORS, dlg.m_ai.colors, sizeof(dlg.m_ai.colors));
		}
		if (bServer)
			pam->SetDefaultFolder(dlg.m_ai.path);
		TreeSettingChange();
		ListSettingChange();
	}
}

void CMainFrame::OnIconRange(UINT idCommand, UINT code, HWND hwnd)
{
	DWORD style = (DWORD)-1;
	switch (idCommand) {
	case IDM_ICON_REPORT: style = LVS_REPORT; break;
	case IDM_ICON_LARGE: style = LVS_ICON; break;
	case IDM_ICON_LIST: style = LVS_LIST; break;
	}
	if (style != (DWORD)-1) {
		m_list.SetViewType(style);

		int i;
		switch (style) {
		case LVS_REPORT:
			for (i = 0; i < MAXHEADITEM; i++)
				m_list.SetColumnWidth(i, LVSCW_AUTOSIZE);
			break;
		case LVS_LIST:
			m_list.SetColumnWidth(-1, LVSCW_AUTOSIZE);
			break;
		}
	}
}

void CMainFrame::OnSortSeqRange(UINT nID, UINT code, HWND hwnd)
{
	int type, seq;
	GetSortType(seq, type);
	int seqNew = (int)(nID - IDM_SORT_UP);
	if (seqNew != seq)
		SortList(type, seqNew);
}

void CMainFrame::OnSortTypeRange(UINT nID, UINT code, HWND hwnd)
{
	int type, seq;
	GetSortType(seq, type);
	int typeNew = (int)(nID - IDM_SORT_NONE);
	if (typeNew != type)
		SortList(typeNew, seq);
}

void CMainFrame::OnFolderTree()
{
	if (m_tree.IsWindowVisible()) {
		OnPaneClose();
	}
	else {
		SelectGroup(GetCurrentFolderID());
		m_split.SetSinglePaneMode(SPLIT_PANE_NONE);
	}
}

void CMainFrame::OnPaneClose()
{
	SelectGroup(0xFFFFFFFF);
	m_split.SetActivePane(1);
	m_split.SetSinglePaneMode(1);
}

void CMainFrame::OnAppAbout()
{
	APPABOUTINFO_V2 ai = { 0 };
	ai.magic = MAGIC_APPABOUTINFO_V2;
	ai.hInst = RESINST;
	ai.szUser[0] = ' ';
	ai.szCompany[0] = ' ';
	::LoadString(RESINST, IDS_WEB_HOMEPAGE, ai.szUrl, ARRAYSIZE(ai.szUrl));
	lccShowAppAbout(m_hWnd, (LPAPPABOUTINFO)&ai);
}

void CMainFrame::OnAppHelp()
{
	CString url;
	url.LoadString(IDS_WEB_HELP);
	ShellExecute(NULL, _T("open"), url.GetString(), NULL, NULL, SW_SHOW);
}


// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include "resource.h"

#include "common.h"
#include "MainFrm.h"
#include "save.h"
#include "zoom.h"
#include "setup.h"
#include "gradlist.h"
#include "quantize.h"
#include "paledit.h"
#include "resize.h"
#include "paste.h"
#include "sizeclip.h"

///////////////////////////////////////////////////////////////////////
// functions

HDIB create_newimage(HWND hwnd);

///////////////////////////////////////////////////////////////////////
// defines

#ifndef SB_SETICON
	#define SB_SETICON              (WM_USER+15)
#endif

#define CY_SCREEN					(GetSystemMetrics(SM_CYSCREEN))
#define CY_MENU						(GetSystemMetrics(SM_CYMENU))

static int indicators[] =
{
	ID_DEFAULT_PANE,
	ID_INDICATOR_CURSOR,
	ID_INDICATOR_SCALE,
	ID_INDICATOR_SELECT,
	ID_INDICATOR_SQUARE,
	ID_INDICATOR_COLOR,
};

const TCHAR _FileSection[] = _T("Recent File List");
const TCHAR _FileEntry[] = _T("File%d");
const TCHAR sszUseClip[] = _T("/clip");

typedef struct tagTBARINFO {
	int nCol;
	int nRow;
	int nWidth;
} TBARINFO, *LPTBARINFO;

static CMainFrame* spMainFrame = NULL;

const SIZE sizePluginIcon = { 20, 20 };

/////////////////////////////////////////////////////////////////////////////
//

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
		HICON hIcon = LoadIcon(RESINST, MAKEINTRESOURCE(IDR_FRAME));
		if (hIcon) {
			pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);
			DestroyIcon(hIcon);
		}
	}
*/
}

void CMainFrame::UpdateUIAll()
{
	BOOL bUndo = (m_Undo.GetCount() > 0);
	UIEnable(IDM_FILE_SAVE, bUndo);
	UIEnable(IDM_EDIT_UNDO, bUndo);

	BOOL bPaste = ImageInClipboard(m_hWnd);
	UIEnable(IDM_FILE_OPENCLIPBOARD, bPaste);
	UIEnable(IDM_EDIT_PASTE, bPaste);

	BOOL bFullColor = (m_nBitCount >= 24);
	UIEnable(IDM_COLOR_QUANTIZE, bFullColor);

	BOOL bPalColor = (m_nBitCount <= 8);
	UIEnable(IDM_COLOR_FULLCOLOR, !bFullColor);
	UIEnable(IDM_COLOR_PALETTE, !bFullColor);

	CRect rc;
	BOOL bClipped = (m_pane.GetClipRect(&rc) && !rc.IsRectEmpty());
	UIEnable(IDM_EDIT_COPY, bClipped);
	UIEnable(IDM_RESIZE_TRIM, bClipped);

	BOOL bGrip = ((m_pane.GetStyle() & ZMVS_GRID) != 0);
	UISetCheck(IDM_OPTION_GRID, bGrip);

	UISetCheck(IDM_TOOL_TBSMALL, 	(m_dwStat & BST_TBSMALL) != 0);
	UISetCheck(IDM_TOOL_NAVIGATOR,	m_navi.IsWindowVisible());

	DWORD dw = m_pane.CanZoom();
	UIEnable(IDM_ZOOM_IN, LOWORD(dw) != 0);
	UIEnable(IDM_ZOOM_OUT, HIWORD(dw) != 0);

	CString cs;
	int iScale = m_pane.GetScale();
	int s = iScale / 10;
	int m = iScale % 10;
	if (!m)	cs.Format(_T("%d%%"), s);
	else 	cs.Format(_T("%d.%d%%"), s, m);
	m_statusbar.SetPaneText(ID_INDICATOR_SCALE, cs);

	UIUpdateToolBar();
}

void CMainFrame::Construct()
{
	CAppReg reg;
	m_strName.Empty();

	DWORD def = BST_AUTOSCALE | BST_NAVIGATOR;
	if (GetSystemMetrics(SM_CYFULLSCREEN) < 800) def |= BST_TBSMALL;
	m_dwStat = reg.GetValue(IDS_SEC_MAIN_EDITOR, IDS_KEY_DISPSTAT, def);
	m_hicScale = NULL;
	m_hicSizeGrip = NULL;
	m_hicRgbGrip = NULL;

	m_hdlgPrintReady = NULL;
}

void CMainFrame::SetDIBInformation(HDIB hdib)
{
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	m_sizeDib = CSize(pbh->biWidth, pbh->biHeight);
	m_nBitCount = pbh->biBitCount;
	GlobalUnlock(hdib);

	CString str;
	str.Format(_T("%d x %d"), m_sizeDib.cx, m_sizeDib.cy);
	m_statusbar.SetPaneText(ID_INDICATOR_SQUARE, str);
	str.Format(_T("%dbpp"), m_nBitCount);
	m_statusbar.SetPaneText(ID_INDICATOR_COLOR, str);
}

void CMainFrame::SetDIB(HDIB hdib, BOOL bRedraw)
{
	if (m_pane.IsWindow()) {
		CRect rc(0, 0, 0, 0);
		m_pane.SetClipRect(&rc);
	}

	HDIB hdibPrev = m_pane.UpdateImage(hdib, TRUE);
	if (hdibPrev) GlobalFree(hdibPrev);

	m_navi.OnUpdate(NFV_IMAGE_CHANGED, (LPARAM)hdib);
}

void CMainFrame::UpdateDIB(HDIB hdib, UINT nID, BOOL bRedraw, BOOL bKeepRect)
{
	CRect rc;
	if (!m_pane.GetClipRect(&rc))
		bKeepRect = FALSE;

	HDIB hdibCur = m_pane.GetImage();
	if (nID && hdibCur)
		PushUndo(nID, (HDIB)copyhandle(hdibCur));
	SetDIBInformation(hdib);
	SetDIB(hdib, bRedraw);

	if (bKeepRect)
		m_pane.SetClipRect(&rc);
}

void CMainFrame::EmptyUndo()
{
	while (!m_Undo.IsEmpty()) {
		CUndo* pUndo = m_Undo.RemoveHead();
		if (pUndo) delete pUndo;
	}
	m_Undo.RemoveAll();
}

HDIB CMainFrame::PopUndo(CRect& rrc)
{
	HDIB hdib = NULL;
	if (!m_Undo.IsEmpty()) {
		CUndo* pUndo = m_Undo.RemoveHead();
		if (pUndo) {
			hdib = pUndo->DetachDIB();
			rrc = pUndo->m_rc;
			delete pUndo;
		}
	}
	return hdib;
}

void CMainFrame::PushUndo(UINT idCommand, HDIB hdib)
{
	if (hdib) {
		CUndo* pUndo = new CUndo(idCommand, hdib);
		m_pane.GetClipRect(&pUndo->m_rc);
		m_Undo.AddHead(pUndo);
		while (m_Undo.GetCount() > MAXUNDO) {
			pUndo = m_Undo.RemoveTail();
			if (pUndo) delete pUndo;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////


int CMainFrame::OnCreate(LPCREATESTRUCT /*pcs*/)
{
	spMainFrame = this;
	Construct();

	// 各種フィルタ用にメニューをカスタマイズ - コマンドバー作成前におこなう
	HMENU hMainMenu = GetMenu();
	HMENU hpop = ::CreatePopupMenu();
	::InsertMenu(hMainMenu, 3, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hpop, _T("フィルタ(&P)"));

	// Rebar
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
#ifdef CUSTOM_REBER_USING
	m_rebar.SubclassWindow(m_hWndToolBar);
#else
	m_rebar = m_hWndToolBar;
#endif

	// コマンドバー作成
	m_menubar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_menubar.AttachMenu(hMainMenu);
	m_menubar.SetImageMaskColor(RGB(255, 0, 255));
	m_menubar.LoadMultipleImages(IDB_S_TOOLBAR);
	m_menubar.LoadMultipleImages(IDB_S_EDIT_APPICON);
	SetMenu(NULL);
#ifdef CUSTOM_REBER_USING
	m_rebar.SetMenuBar(m_menubar);						// メニューバーを明示的に登録
#endif
	AddSimpleReBarBand(m_menubar, NULL, FALSE, 0, FALSE);

	HWND hWndToolBar = CreateToolbar();
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE, 0, FALSE);

	// lock rebar
	m_rebar.LockBands(true);

	CreateSimpleStatusBar();
	m_statusbar.SubclassWindow(m_hWndStatusBar);
	m_statusbar.SetPanes(indicators, sizeof(indicators) / sizeof(int), false);

	m_hicScale = (HICON)LoadImage(RESINST, _MKIR(IDI_SCALE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_statusbar.SetPaneIcon(ID_INDICATOR_SCALE, m_hicScale);
	m_hicSizeGrip = (HICON)LoadImage(RESINST, _MKIR(IDI_SIZEGRIP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_statusbar.SetPaneIcon(ID_INDICATOR_SQUARE, m_hicSizeGrip);
	m_hicRgbGrip = (HICON)LoadImage(RESINST, _MKIR(IDI_RGB), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_statusbar.SetPaneIcon(ID_INDICATOR_COLOR, m_hicRgbGrip);

	// クライアント作成
	m_hWndClient = m_pane.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ZMVS_STYLE_SEL, WS_EX_CLIENTEDGE, IDW_ZOOMVIEW);
	m_pane.SetDrawMode(ZVDM_BITMAP);

	ZOOMVIEWINFO zi;
	CAppReg reg;
	if (reg.GetBinary(IDS_SEC_MAIN_EDITOR, IDS_KEY_ZOOMVIEWINFO, &zi, sizeof(zi)) == sizeof(zi)) {
		zi.cbSize = sizeof(zi);
		zi.fMask = ZIF_BKCOLOR | ZIF_GRID | ZIF_GRIDCOLOR | ZIF_SHOWGRID;
		m_pane.SetZoomViewInfo(&zi);
	}

	// ツールバー更新
	UIAddToolBar(hWndToolBar);

	// recent file list
	HMENU hMenu = GetSubMenuByPosition(0, 11);
	m_mru.SetMenuHandle(hMenu);
	CString str = reg.GetRootName();
	m_mru.ReadFromRegistry(str);

	// plug-in filter
	InitializePluginFilters();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// 前回のウィンドウ位置
	WINDOWPLACEMENT wpm;
	if (reg.GetBinary(IDS_SEC_MAIN_EDITOR, IDS_KEY_WNDPOS, &wpm, sizeof(wpm)) == sizeof(wpm)) {
		CRect rc = wpm.rcNormalPosition;
		if (CountSiblings(m_hWnd) > 0) {
			GetWindowPlacement(&wpm);
			rc.OffsetRect(wpm.rcNormalPosition.left - rc.left, wpm.rcNormalPosition.top - rc.top);
		}
		lccMapRectToProgmanPane(&rc);
		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	DragAcceptFiles(TRUE);
	m_pane.SetFocus();

	// ナビ --- デフォルトではビュアの右上隅に位置させる
	CRect rc;
	m_pane.GetClientRect(&rc);
	m_pane.ClientToScreen(&rc);
	rc.right -= GetSystemMetrics(SM_CXVSCROLL);
	CRect rcw(0, 0, 192, 160);
	AdjustWindowRectEx(&rcw, WS_CAPTION | WS_SIZEBOX, FALSE, WS_EX_TOOLWINDOW);
	rc.left = rc.right - rcw.Width();
	rc.bottom = rc.top + rcw.Height();
	NAVIPOSINFO npi;
	if (reg.GetBinary(IDS_SEC_MAIN_EDITOR, IDS_KEY_NAVIPOS, &npi, sizeof(npi)) == sizeof(npi)) {
		rc = npi.wpm.rcNormalPosition;
	}
	lccMapRectToProgmanPane(&rc);

	m_navi.Create(m_hWnd, rc, NULL,
					WS_SYSMENU | WS_CAPTION | WS_SIZEBOX | WS_CLIPCHILDREN,
					WS_EX_TOOLWINDOW, 0U, (void*)IDW_NAVI);
	if (m_dwStat & BST_NAVIGATOR)
		m_navi.ShowWindow(SW_SHOWNOACTIVATE);

	return 0;
}

HWND CMainFrame::CreateToolbar()
{
	if (m_toolbar.IsWindow())
		RemoveCustomDrawBar(m_toolbar.m_hWnd);

	UINT id = (m_dwStat & BST_TBSMALL) ? IDB_S_TOOLBAR : IDB_M_TOOLBAR;
	HWND hWndToolBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(id), ATL_SIMPLE_TOOLBAR_PANE_STYLE, ATL_IDW_TOOLBAR);
	m_toolbar = hWndToolBar;

	AddCustomDrawBar(hWndToolBar);
	return hWndToolBar;
}

typedef struct _meueItemImage {
	UINT id;
	int type;
	union {
		HBITMAP hbm;
		HICON hic;
	} image;
} menuItemImage;

void CMainFrame::InitializePluginFilters()
{
	// 書き出しグループは既存なので事前に追加
	HMENU hmenu = GetSubMenuByPosition(0, 6);
	::DeleteMenu(hmenu, 0, MF_BYPOSITION);				// ダミー用セパレータを削除
	m_mapPluginMenu.Add(gidWriteout, hmenu);
	// 色調の変更グループは既存なので事前に追加
	hmenu = GetSubMenuByPosition(2, 1);
	::DeleteMenu(hmenu, 0, MF_BYPOSITION);				// ダミー用セパレータを削除
	m_mapPluginMenu.Add(gidTone, hmenu);

	// 新規メニュー追加用マップ
	CSimpleMap<UINT, CFilterGroupMenu*> arrNewMenu;

	TCHAR buf[MAX_PATH], path[MAX_PATH];
	GetModuleFileName(MODINST, path, ARRAYSIZE(path));
	PathRemoveFileSpec(path);
	PathAppend(path, PLUGIN_SECTION);
	PathCombine(buf, path, PLUGIN_EXTENSION);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(buf, &wfd);
	UINT nID = IDM_PLUGIN_FIRST;
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			PathCombine(buf, path, wfd.cFileName);
			EnumPluginFilters(buf, nID, arrNewMenu);
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}

	int i;
	hmenu = GetSubMenuByPosition(3, -1);
	for (i = 0; i < 80; i++) {
		CFilterGroupMenu* pgm = arrNewMenu.Lookup((UINT)i);
		if (pgm) {
			::AppendMenu(hmenu, MF_POPUP, (UINT_PTR)pgm->m_hpop, pgm->m_str);
			delete pgm;
		}
	}
	// 指定のあったメニュー項目アイコンを登録
	int numFilters = m_mapPlugin.GetSize();
	for (i = 0; i < numFilters; i++) {
		UINT id = m_mapPlugin.GetKeyAt(i);
		CFilter* pf = m_mapPlugin.GetValueAt(i);
		switch (pf->m_imageStyle) {
			case 0:
				m_menubar.AddIcon(pf->m_hicImage, id);
				break;
			case 1:
				m_menubar.AddBitmap(pf->m_hbmImage, id);
				break;
		}
	}

}

void CMainFrame::EnumPluginFilters(LPCTSTR pszDll, UINT& idStart, CSimpleMap<UINT, CFilterGroupMenu*>& arrNewMenu)
{
	HMODULE hdll = LoadLibrary(pszDll);
	if (!hdll) return;

	PLUGINITPROC pfnInit;
	FILTERINFOPROC pfnInfo;
	FILTERPROC pfnFilter;

	(FARPROC&)pfnInit = GetProcAddress(hdll, PLUGIN_FILTER_INIT);
	(FARPROC&)pfnInfo = GetProcAddress(hdll, PLUGIN_FILTER_INFO);
	(FARPROC&)pfnFilter = GetProcAddress(hdll, PLUGIN_FILTER_EXEC);
	if (pfnInit && pfnInfo && pfnFilter) {
		PLUGIN_INIT pi;
		memset(&pi, 0, sizeof(pi));
		if (pfnInit(&pi)) {
			for (int i = 0; i < pi.numFilters; i++) {
				FILTER_INFO info;
				memset(&info, 0, sizeof(info));
				info.magic = sizeof(FILTER_INFO);
				if (pfnInfo(i, &info)) {
					CFilter* filter = new CFilter();
					filter->m_strDllName = pszDll;
					filter->m_iFilterNumber = i;
					filter->m_nFlags = info.flags;
					// アイコン
					if (info.idIcon) {
						filter->m_imageStyle = info.style;
						if (info.style == 0) {
							HICON hic = ::LoadIcon(hdll, MAKEINTRESOURCE(info.idIcon));
							if (hic) {
								m_menubar.AddIcon(hic, idStart);
								::DestroyIcon(hic);
							}
						} else {
							HBITMAP hbm = lccLoadImage(hdll, RT_RCDATA, MAKEINTRESOURCE(info.idIcon));
							if (hbm) {
								m_menubar.AddBitmap(hbm, idStart);
								::DeleteObject(hbm);
							}
						}
					}
					filter->m_strFilterName = info.strFilterName;
					CString str = _T("");
					if (IS_INTRESOURCE(info.groupName)) {
						// 既定グループ
						filter->m_menugroup = (UINT)LOWORD(info.groupName);
						if (filter->m_menugroup > gidOther)
							filter->m_menugroup = gidOther;
						str.LoadString(RESINST, IDS_PLUGIN_GROUP + filter->m_menugroup);
					} else {
						// ユーザ定義グループ
						UINT groupid = m_mapFilterGroup.Lookup(info.groupName);
						if (!groupid) {
							groupid = gidOther + m_mapFilterGroup.GetSize() + 1;
							m_mapFilterGroup.Add(info.groupName, groupid);
						}
						filter->m_menugroup = groupid;
						str = info.groupName;
					}

					HMENU hpop = m_mapPluginMenu.Lookup(filter->m_menugroup);
					if (!hpop) {
						hpop = ::CreatePopupMenu();
						m_mapPluginMenu.Add(filter->m_menugroup, hpop);

						CFilterGroupMenu* pgm = new CFilterGroupMenu(str, hpop);
						arrNewMenu.Add(filter->m_menugroup, pgm);
					}

					filter->m_idCommand = idStart;
					::AppendMenu(hpop, 0, idStart, filter->m_strFilterName);
					m_mapPlugin.Add(idStart, filter);
					idStart++;
				}
			}
		}
	}
	FreeLibrary(hdll);
}

LRESULT CMainFrame::OnActivate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (LOWORD(wParam) != WA_INACTIVE) {
		m_pane.SetFocus();
	}
	bHandled = FALSE;
	return 1;
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

#if 0
void CMainFrame::ResizeFrame()
{
	CRect rc;
	GetClientRect(&rc);
	int cyHeight = rc.Height();
	UpdateBarsPosition(rc, FALSE);
	cyHeight -= rc.Height();

	// ビットマップの大きさにウィンドウサイズを調整する
	double scale = (double)m_pane.GetScale() / 1000;
	int xDev = (int)ceil(m_sizeDib.cx * scale);
	int yDev = (int)ceil(m_sizeDib.cy * scale);

	rc.SetRect(0, 0, xDev, yDev);
	AdjustWindowRectEx(&rc, m_pane.GetStyle(), FALSE, m_pane.GetExStyle());
	rc.bottom += cyHeight;

	AdjustWindowRect(&rc, GetStyle(), (GetMenu() != NULL));

	CRect rcWin;
	GetWindowRect(&rcWin);
	int x = rcWin.left;
	int y = rcWin.top;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWin, 0);
	if (rcWin.Width() < rc.Width())		rc.right = rc.left + rcWin.Width();
	if (rcWin.Height() < rc.Height())	rc.bottom = rc.top + rcWin.Height();
	if (x + rc.Width() > rcWin.right)	x = rc.Width() - rcWin.right;
	if (y + rc.Height() > rcWin.bottom)	y = rc.Height() - rcWin.bottom;

	if (x < 0) x = 0;
	if (y < 0) y = 0;

	SetWindowPos(NULL, x, y, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}
#endif

void CMainFrame::SetCaptionTitle(LPCTSTR pszTitle)
{
	// タイトルの設定
	LPCTSTR p = StrRChr(pszTitle, NULL, '\\');
	if (!p)
		p = pszTitle;
	else
		p++;

	CString str; str.LoadString(RESINST, IDR_VIEW);
	CString tit; tit.Format(_T("%s - %s"), p, (LPCTSTR)str);
	SetWindowText(tit);
	SetWindowText(tit);
}

void CMainFrame::OnContextMenu(HWND hWnd, CPoint pt)
{
	if (hWnd == m_pane.m_hWnd) {
		HMENU hmenu = GetSubMenuByPosition(3, -1);
		if (hmenu) {
			if (pt.x < 0 && pt.y < 0) {
				pt = CPoint(0, 0);
				m_pane.ClientToScreen(&pt);
			}
			m_menubar.TrackPopupMenu(hmenu, TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, NULL);
		}
	}
}

void CMainFrame::OnDropFiles(HDROP hDrop)
{
	// ドロップされるファイル数を取得
	if (DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0) > 0) {
		TCHAR buf[MAX_PATH * 2];
		DragQueryFile(hDrop, 0, buf, MAX_PATH * 2);
		if (imgCheckImageType(buf) != IMG_NOTSUPPORTED)
			OpenFile(buf);
	}
	DragFinish(hDrop);
}

BOOL CMainFrame::OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState)
{
	UINT idCommand = 0;
	switch (appCommand) {
	case APPCOMMAND_CLOSE:				idCommand = IDM_FILE_CLOSE;	break;
	}
	if (idCommand) {
		PostMessage(WM_COMMAND, idCommand, 0);
		return TRUE;
	}
	return FALSE;
}

void CMainFrame::OnDestroy()
{
	EmptyUndo();
	if (m_hicScale)			DestroyIcon(m_hicScale);
	if (m_hicSizeGrip)		DestroyIcon(m_hicSizeGrip);
	if (m_hicRgbGrip)		DestroyIcon(m_hicRgbGrip);

	CAppReg reg;
	CString str = reg.GetRootName();
	m_mru.WriteToRegistry(str);

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
}

void CMainFrame::OnClose(BOOL& bHandled)
{
	if (!m_Undo.IsEmpty()) {
		int n = AppMessage(IDR_FRAME, IDS_QUERY_UPDATESAVE, NULL, MB_QUESTION);
		if (n == IDCANCEL) {
			bHandled = TRUE;
			return;
		}
		if (n == IDYES)
			OnFileSave();
		EmptyUndo();
	}

	CAppReg reg;
	ZOOMVIEWINFO zi;
	zi.cbSize = sizeof(zi);
	zi.fMask = ZIF_BKCOLOR | ZIF_SCALE | ZIF_MODE | ZIF_GRID | ZIF_GRIDCOLOR | ZIF_SHOWGRID;
	m_pane.GetZoomViewInfo(&zi);
	reg.SetBinary(IDS_SEC_MAIN_EDITOR, IDS_KEY_ZOOMVIEWINFO, &zi, sizeof(zi));

	// ウィンドウ位置
	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(wpm);
	if (GetWindowPlacement(&wpm))
		reg.SetBinary(IDS_SEC_MAIN_EDITOR, IDS_KEY_WNDPOS, &wpm, sizeof(wpm));

	// ナビ
	m_dwStat &= ~BST_NAVIGATOR;
	if (m_navi.IsWindow() && m_navi.IsWindowVisible()) {
		m_dwStat |= BST_NAVIGATOR;
		// ウィンドウ位置
		NAVIPOSINFO npi;
		npi.wpm.length = sizeof(npi.wpm);
		if (m_navi.GetWindowPlacement(&npi.wpm)) {
			reg.SetBinary(IDS_SEC_MAIN_EDITOR, IDS_KEY_NAVIPOS, &npi, sizeof(npi));
		}
		m_navi.DestroyWindow();
	}
	reg.SetValue(IDS_SEC_MAIN_EDITOR, IDS_KEY_DISPSTAT, m_dwStat);
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* pMMI) 
{
	pMMI->ptMinTrackSize.x = 320;
	pMMI->ptMinTrackSize.y = 200;
}

void CMainFrame::OnFileSave()
{
	if (m_strName.IsEmpty())
		OnFileSaveAs();
	else if (!m_Undo.IsEmpty()) {
		HDIB hdib = m_pane.GetImage();
		if (hdib == NULL) return;

		if (SaveGraphic(hdib, (LPTSTR)(LPCTSTR)m_strName, m_nFileType))
			EmptyUndo();
		else
			AppMessage(IDR_FRAME, IDS_ERR_CANNOTSAVE, m_strName, MB_ICONSTOP);
	}
}

void CMainFrame::OnFileSaveAs()
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	int type;
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, m_strName);
	_pathdecorate(buf);
	if (SaveFileBrowse(m_hWnd, buf, &type)) {
		if (PathFileExists(buf)) {
			if (AppMessage(IDR_FRAME, IDS_ALREADYEXIST, buf, MB_QYESNO) != IDYES)
				return;
		}
		if (SaveGraphic(hdib, buf, type)) {
			EmptyUndo();
			m_nFileType = type;
			m_strName = buf;
			SetCaptionTitle(buf);
		}
	}
}

void CMainFrame::OnFileWallPaper()
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	BOOL bActiveDesktop = FALSE;
	IActiveDesktop* pad = GetActiveDesktop();
	if (pad) {
		pad->Release();
		bActiveDesktop = TRUE;
	}

	TCHAR buf[MAX_PATH * 2];
	int filterIndex;
	if (FileBrowse(m_hWnd, IDS_CAPTION_SAVEBMP,
			bActiveDesktop ? IDS_FILTER_WALLPAPER : IDS_FILTER_BMP,
			buf, FALSE, &filterIndex))
	{
		HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
		if (!bActiveDesktop)
			filterIndex = 1;
		PathRenameExtension(buf, img_types[filterIndex].ext);

		if (SaveGraphic(hdib, buf, imgSpecifyImageType(buf)))
			SetWallpaper(buf);
		SetCursor(hcur);
	}
}

void CMainFrame::OnFileClose()
{
	PostMessage(WM_CLOSE);
}

void CMainFrame::OnFileTwain()
{
	if (!m_Undo.IsEmpty()) {
		if (AppMessage(IDR_FRAME, IDS_QUERY_UPDATESAVE, NULL, MB_QYESNO) == IDYES)
			OnFileSave();
		EmptyUndo();
	}

	HMODULE hmod = LoadLibrary(_T("lsptwain.dll"));
	if (hmod) {
		HDIB (WINAPI* pfn)(void);
		(FARPROC&)pfn = GetProcAddress(hmod, "TwainGetImage");
		if (pfn) {
			HDIB hdib = pfn();
			if (hdib)
				UpdateDIB(hdib, IDM_FILE_TWAIN, TRUE, FALSE);
		}
		FreeLibrary(hmod);
	}
}

BOOL CMainFrame::LoadFile(LPCTSTR pszName)
{
	HDIB hdib = NULL;
/* @@
	int nType = imgCheckImageType(pszName);
	if (nType == IMG_MRW)
		hdib = imgDCRawLoad(pszName);
	else
@@ */
		hdib = imgLoadImage(pszName);

	if (!hdib) {
		MessageBeep(0);
		return FALSE;
	}
	SetCaptionTitle(pszName);

	// 減色が必要ならおこなう
	CClientDC dc(NULL);
	int nBitsPixel = dc.GetDeviceCaps(BITSPIXEL);
	if (nBitsPixel <= 8 && nBitsPixel < (int)m_nBitCount) {
		SetWindowText(_T("減色中(256色)..."));

		HDIB h = quantize(hdib, ditherFS, 256);
		if (h) {
			GlobalFree(hdib);
			hdib = h;
		}
	}
	SetDIBInformation(hdib);

	if (!IsWindowVisible()) {
		ShowWindow(SW_SHOWNORMAL);
		UpdateWindow();
	}

	SetDIB(NULL, FALSE);
	m_pane.SetScale(1000);
	SetDIB(hdib, FALSE);
	if (m_dwStat & BST_SCALE_MASK) {
		CRect rc;
		m_pane.GetClientRect(&rc);
		if (m_dwStat & BST_AUTOSCALE) {
			if (rc.Width() < m_sizeDib.cx || rc.Height() < m_sizeDib.cy)
				m_pane.ZoomFull();
		}
		else {
			// 前回終了時の倍率
			CAppReg reg;
			ZOOMVIEWINFO zi;
			if (reg.GetBinary(IDS_SEC_MAIN_EDITOR, IDS_KEY_ZOOMVIEWINFO, &zi, sizeof(zi)) == sizeof(zi)) {
				zi.cbSize = sizeof(zi);
				m_pane.SetScale(zi.scale);
			}
		}
	}
	return TRUE;
}

BOOL CMainFrame::OpenFile(LPCTSTR pszName)
{
	if (!m_Undo.IsEmpty()) {
		int n = AppMessage(IDR_FRAME, IDS_QUERY_UPDATESAVE, NULL, MB_QUESTION);
		if (n == IDCANCEL) return FALSE;
		else if (n == IDYES) OnFileSave();
		EmptyUndo();
	}

	m_strName.Empty();
	if (!pszName || *pszName == 0) return FALSE;

	TCHAR file[MAX_PATH * 2];
	lstrcpy(file, pszName);
	PathUnquoteSpaces(file);

	if (!StrCmpI(file, sszUseClip)) {
		if (ImageInClipboard(m_hWnd)) {
			ShowWindow(SW_SHOWNORMAL);
			UpdateWindow();
			m_nFileType = IMG_BMP;
			OnEditPaste();
			SetCaptionTitle(_T("クリップボード"));
		}
		else
			return FALSE;
	}
	else if (PathFileExists(file) && !PathIsDirectory(file)) {
		int nType = imgCheckImageType(file);
		if (nType == IMG_NOTSUPPORTED || !LoadFile(file))
			return FALSE;

		m_nFileType = nType;
		m_strName = file;
		m_mru.AddToList(file);
	}
	else
		return FALSE;

	return TRUE;
}

void CMainFrame::OnFileOpen() 
{
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, m_strName);
	if (FileBrowse(m_hWnd, IDS_CAPTION_LOADBMP, IDS_FILTER_PICTURE, buf, TRUE, NULL)) {
		OpenFile(buf);
	}
}

void CMainFrame::OnFileNew() 
{
	if (!m_Undo.IsEmpty()) {
		int n = AppMessage(IDR_FRAME, IDS_QUERY_UPDATESAVE, NULL, MB_QUESTION);
		if (n == IDCANCEL) return;
		else if (n == IDYES) OnFileSave();
		EmptyUndo();
	}

	HDIB hdib = create_newimage(m_hWnd);
	if (hdib)
		UpdateDIB(hdib, IDM_FILE_NEW, TRUE, FALSE);
}

void CMainFrame::OnFileOpenClipboard()
{
	if (ImageInClipboard(m_hWnd)) {
		if (m_pane.GetImage() != NULL)
			lccDllExecute(m_hWnd, DLL_GMIL, _T("pxfRun"), sszUseClip, _T("/image"), FALSE);
		else
			OpenFile(sszUseClip);
	}
}

void CMainFrame::OnEditCopy()
{
	if (!user_ristriction()) return;

	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	CRect rc;
	m_pane.GetClipRect(&rc);
	mif image(hdib);
	if (image.crop(&rc)) {
		hdib = image.createdib();
		if (OpenClipboard()) {
			EmptyClipboard();

			HPALETTE hpal;
			HBITMAP hbm;
			imgCreateBitmapFromDIB(hdib, &hbm, &hpal);

			SetClipboardData(CF_DIB, hdib);
			SetClipboardData(CF_PALETTE, hpal);
			SetClipboardData(CF_BITMAP, hbm);
			CloseClipboard();
		}
		else
			GlobalFree(hdib);
	}
}

void CMainFrame::OnEditPaste()
{
	if (OpenClipboard()) {
		HANDLE hf;
		HDIB hdibClip = NULL;
		if (hf = GetClipboardData(CF_DIB))
			hdibClip = (HDIB)copyhandle(hf);
		else if (hf = GetClipboardData(CF_BITMAP)) {
			HPALETTE hpal = (HPALETTE)GetClipboardData(CF_PALETTE);
			hdibClip = imgCreateDIBFromBitmap((HBITMAP)hf, hpal);
		}
		CloseClipboard();

		int width, height, bits;
		dib_getinfo(hdibClip, &width, &height, NULL, &bits);
		if (bits > 24 || bits == 16) {
			HDIB h = imgChangeDIBFormat(hdibClip, 24, BI_RGB);
			if (h) {
				GlobalFree(hdibClip);
				hdibClip = h;
			}
		}

		HDIB hdib = m_pane.GetImage();
		if (hdib) {
			CPasteDlg dlg;
			if (dlg.DoModal() != IDOK) {
				GlobalFree(hdibClip);
				return;
			}

			CPoint pt;
			if (dlg.m_pp.noalign) {
				if (!TrackImage(hdibClip, &pt, RGB(255, 255, 255))) {
					GlobalFree(hdibClip);
					return;
				}
			}
			else {
				pt = CPoint(0, 0);
				switch (dlg.m_pp.anchor / 3) {
					case 1:	pt.y = (m_sizeDib.cy - height) / 2;	break;
					case 2:	pt.y = m_sizeDib.cy - height;		break;
					default:	break;
				}
				switch (dlg.m_pp.anchor % 3) {
					case 1:	pt.x = (m_sizeDib.cx - width) / 2;	break;
					case 2:	pt.x = m_sizeDib.cx - width;		break;
					default:	break;
				}
				pt += dlg.m_pp.ptOffset;
			}

			CWaitCursor wc;
			HDIB w = PasteImage(hdib, hdibClip, pt.x, pt.y, &dlg.m_pp.ai);
			GlobalFree(hdibClip);
			hdibClip = w;
		}

		if (hdibClip)
			UpdateDIB(hdibClip, IDM_EDIT_PASTE, TRUE, FALSE);
	}
}

void CMainFrame::OnEditSelAll()
{
	if (!m_pane.GetImage()) return;

	CRect rc(0, 0, m_sizeDib.cx, m_sizeDib.cy);
	m_pane.SetClipRect(&rc);
}

void CMainFrame::OnEditSelCancel()
{
	CRect rc;
	if (m_pane.GetClipRect(&rc) && !rc.IsRectEmpty()) {
		rc.SetRectEmpty();
		m_pane.SetClipRect(&rc);
	}
}

void CMainFrame::OnEditSizeClip()
{
	HDIB hdib = m_pane.GetImage();
	if (!hdib) return;

	CSizeClipDlg dlg;

	CRect rc;
	m_pane.GetClipRect(&rc);
	bool bRectPresented = !rc.IsRectEmpty();
	dlg.m_sizeClip = CSize(rc.Width(), rc.Height());

	if (dlg.DoModal() == IDOK) {
		CRect rcc(0, 0, dlg.m_sizeClip.cx, dlg.m_sizeClip.cy);
		if (bRectPresented)
			rcc.OffsetRect(rc.left, rc.top);
		else
			rcc.OffsetRect((m_sizeDib.cx - dlg.m_sizeClip.cx) / 2, (m_sizeDib.cy - dlg.m_sizeClip.cy) / 2);
		m_pane.SetClipRect(&rcc);
	}
}

void CMainFrame::OnResizeZoom() 
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	CZoomDlg dlg;
	dlg.m_sizeSrc = m_sizeDib;
	dlg.m_nBitCount = m_nBitCount;
	if (dlg.DoModal() != IDOK || m_sizeDib == dlg.m_sizeDst)
		return;

	mif image;
	if (image.attachdib(hdib)) {
		HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
		if (dlg.m_resample == 2)
			image.zoom(dlg.m_sizeDst.cx, dlg.m_sizeDst.cy, dlg.m_nType, FALSE);
		else if (dlg.m_resample == 1)
			image.zoom_linear(dlg.m_sizeDst.cx, dlg.m_sizeDst.cy);
		else
			image.zoom_near(dlg.m_sizeDst.cx, dlg.m_sizeDst.cy);
		hdib = image.createdib();
		SetCursor(hcur);
	}
	else
		hdib = NULL;

	if (hdib)
		UpdateDIB(hdib, IDM_RESIZE_ZOOM, FALSE, FALSE);
	else
		MessageBeep(0);
}

void CMainFrame::OnResizeCanvas() 
{
	HDIB hdib = resize_canvas(m_hWnd, m_pane.GetImage());
	if (hdib)
		UpdateDIB(hdib, IDM_RESIZE_CANVAS, FALSE, FALSE);
	else
		MessageBeep(0);
}


void CMainFrame::OnColorFullColor()
{
	HDIB hdib = m_pane.GetImage();
	if (!hdib || m_nBitCount >= 24) return;

	mif image;
	if (image.attachdib(hdib)) {
		image.m_bits = 24;
		hdib = image.createdib();
		if (hdib)
			UpdateDIB(hdib, IDM_COLOR_FULLCOLOR, TRUE, FALSE);
	}
}

void CMainFrame::OnColorQuantize()
{
	HDIB hdib = m_pane.GetImage();
	if (!hdib || m_nBitCount <= 8) return;

	CQuantizeDlg dlg;
	dlg.m_hdib = hdib;
	if (dlg.DoModal() == IDOK) {
		hdib = quantize(hdib, dlg.m_method, dlg.m_colors);
		if (hdib)
			UpdateDIB(hdib, IDM_COLOR_QUANTIZE, TRUE, FALSE);
	}
}

void CMainFrame::OnEditUndo() 
{
	CRect rc;
	HDIB hdib = PopUndo(rc);
	if (hdib) {
		CSize sizeOld = m_sizeDib;
		UpdateDIB(hdib, 0, TRUE, FALSE);
		m_pane.SetClipRect(&rc);
	}
}


void CMainFrame::OnColorPalette() 
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	hdib = edit_palette(m_hWnd, hdib);
	if (hdib)
		UpdateDIB(hdib, IDM_COLOR_PALETTE, TRUE, FALSE);
}

void CMainFrame::OnResizeTrim() 
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	CRect rc;
	m_pane.GetClipRect(&rc);

	mif image(hdib);
	image.crop(&rc);
	hdib = image.createdib();
	if (hdib)
		UpdateDIB(hdib, IDM_RESIZE_TRIM, FALSE, FALSE);
}

void CMainFrame::OnColorCount() 
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	mif image(hdib);
	HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	CString cs;
	cs.Format(_T("%d"), image.countcolors());
	SetCursor(hcur);
	AppMessage(IDR_FRAME, IDS_NUMOFCOLORS, cs, MB_OK | MB_ICONINFORMATION);
}

void CMainFrame::OnRotateLeft() 
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	mif image(hdib);
	image.rotate(TRUE);
	hdib = image.createdib();
	if (hdib)
		UpdateDIB(hdib, IDM_ROTATE_LEFT, FALSE, FALSE);
}

void CMainFrame::OnRotateRight() 
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	mif image(hdib);
	image.rotate(FALSE);
	hdib = image.createdib();
	if (hdib)
		UpdateDIB(hdib, IDM_ROTATE_RIGHT, FALSE, FALSE);
}

void CMainFrame::OnRotateFlip() 
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	CRect rc;
	m_pane.GetClipRect(&rc);

	mif image(hdib);
	image.flip(rc.left, rc.top, rc.Width(), rc.Height());
	hdib = image.createdib();
	if (hdib)
		UpdateDIB(hdib, IDM_ROTATE_FLIP, TRUE, TRUE);
}

void CMainFrame::OnRotateMirror() 
{
	HDIB hdib = m_pane.GetImage();
	if (hdib == NULL) return;

	CRect rc;
	m_pane.GetClipRect(&rc);

	mif image(hdib);
	image.mirror(rc.left, rc.top, rc.Width(), rc.Height());
	hdib = image.createdib();
	if (hdib)
		UpdateDIB(hdib, IDM_ROTATE_MIRROR, TRUE, TRUE);
}

void CMainFrame::OnShowGrid()
{
	m_pane.ToggleGrid();
}

void CMainFrame::OnSetZoomMode()
{
	int mode = (m_pane.GetMode() == ZOOMMODE_ZOOM) ? ZOOMMODE_SEL : ZOOMMODE_ZOOM;
	m_pane.SetMode(mode);
}

void CMainFrame::OnNotifyKeyDown(UINT id, NMHDR* pNmhdr, LRESULT* pResult)
{
	NMKEY* pk = (NMKEY*)pNmhdr;
	if (m_hAccel) {
		MSG msg;
		msg.hwnd = m_hWnd;
		msg.message = WM_KEYDOWN;
		msg.wParam = pk->nVKey;
		msg.lParam = MAKELPARAM(0, pk->uFlags);
		msg.time = GetMessageTime();
		msg.pt = CPoint(GetMessagePos());
		if (::TranslateAccelerator(m_hWnd, m_hAccel, &msg)) {
			*pResult = TRUE;
			return;
		}
	}
	*pResult = FALSE;
}

void CMainFrame::OnNotifyMousePoint(NMHDR* pNmhdr, LRESULT* pResult)
{
	ZOOM_MOUSEPOINT* pzm = (ZOOM_MOUSEPOINT*)pNmhdr;
	TCHAR buf[MAX_PATH];
	buf[0] = '\0';
	if (pzm->bInDrag)
		wsprintf(buf, _T("X:%d Y:%d W:%d H:%d"),
				pzm->rcClip.left, pzm->rcClip.top,
				pzm->rcClip.right - pzm->rcClip.left, pzm->rcClip.bottom - pzm->rcClip.top);
	else if (pzm->pt.x >= 0 && pzm->pt.y >= 0 &&
		pzm->pt.x < m_sizeDib.cx && pzm->pt.y < m_sizeDib.cy)
		wsprintf(buf, _T("X:%d Y:%d"), pzm->pt.x, pzm->pt.y);
	m_statusbar.SetPaneText(ID_INDICATOR_CURSOR, buf);
	*pResult = TRUE;
}

void CMainFrame::OnNotifyVisibleChanged(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	ZOOM_VISIBLEINFO* pzv = (ZOOM_VISIBLEINFO*)pNmhdr;
	m_navi.OnUpdate(NFV_VISIBLEPOSITION_CHANGED, (LPARAM)&pzv->rectVisible);
}

void CMainFrame::OnNotifyScaleChanged(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	ZOOM_SCALE* pzs = (ZOOM_SCALE*)pNmhdr;
	double scale = pzs->scale;
	m_navi.OnUpdate(NFV_SCALE_CHANGED, (LPARAM)&scale);
}

void CMainFrame::OnNotifyNavigator(NMHDR* pNmhdr, LRESULT* pResult)
{
	NAVICLICK* pnc = (NAVICLICK*)pNmhdr;
	if (pNmhdr->code == NVN_CLICK) {
		ZOOMVIEWINFO zvi;
		zvi.cbSize = sizeof(zvi);
		zvi.fMask = ZIF_CENTERPOSITION;
		zvi.ptCenterPosition = pnc->pt;
		m_pane.SetZoomViewInfo(&zvi);
	}
	else if (pNmhdr->code == NVN_SCALE)
		m_pane.SetScale((int)pnc->param);
}

void CMainFrame::OnNotifyClip(NMHDR* pNmhdr, LRESULT* pResult)
{
	TCHAR buf[MAX_PATH];
	ZOOM_CLIP* pbc = (ZOOM_CLIP*)pNmhdr;

	if (!pbc->rcClip.left && !pbc->rcClip.top &&
		!pbc->rcClip.right && !pbc->rcClip.bottom)
		buf[0] = '\0';
	else {
		wsprintf(buf, _T("選択 %d x %d"), pbc->rcClip.right - pbc->rcClip.left, pbc->rcClip.bottom - pbc->rcClip.top);
	}
	m_statusbar.SetPaneText(ID_INDICATOR_SELECT, buf);
	*pResult = TRUE;
}

void CMainFrame::OnOpenRecentFile(UINT nID, UINT code, HWND hwnd)
{
	TCHAR sFile[MAX_PATH];
#if (_WTL_VER < 0x0800)
	if (m_mru.GetFromList(nID, sFile)) {
#else
	if (m_mru.GetFromList(nID, sFile, ARRAYSIZE(sFile))) {
#endif
			if (PathFileExists(sFile)) {
			m_mru.MoveToTop(nID);
			OpenFile(sFile);
		}
		else {
			m_mru.RemoveFromList(nID);
			MessageBeep(0);
		}
	}
}

void CMainFrame::OnToolSmallToolbar()
{
	if (m_dwStat & BST_TBSMALL) m_dwStat &= ~BST_TBSMALL;
	else m_dwStat |= BST_TBSMALL;

	CReBarCtrl rebar = m_hWndToolBar;
	rebar.DeleteBand(1);
	m_toolbar.DestroyWindow();
	HWND hWndToolBar = CreateToolbar();
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE, 0, FALSE);
	// ツールバー更新
	UIAddToolBar(hWndToolBar);

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void CMainFrame::OnToolNavigator()
{
	if (m_navi.IsWindow()) {
		if (m_navi.IsWindowVisible())
			m_navi.ShowWindow(SW_HIDE);
		else
			m_navi.ShowWindow(SW_SHOWNOACTIVATE);
	}
}

void CMainFrame::OnOptionSetup() 
{
	ZOOMVIEWINFO zi;
	zi.cbSize = sizeof(zi);
	zi.fMask = ZIF_BKCOLOR | ZIF_GRID | ZIF_GRIDCOLOR | ZIF_SHOWGRID;
	m_pane.GetZoomViewInfo(&zi);

	CSetupDlg dlg;
	dlg.m_startup_scale = (int)HIWORD(m_dwStat & BST_SCALE_MASK);
	dlg.m_showGrid = zi.showGrid;
	dlg.m_sizeGrid = zi.grid;
	dlg.m_crGrid = zi.crGrid;
	dlg.m_crBack = zi.crBack;
	if (dlg.DoModal() == IDOK) {
		zi.showGrid = dlg.m_showGrid;
		zi.grid = dlg.m_sizeGrid;
		zi.crGrid = dlg.m_crGrid;
		zi.crBack = dlg.m_crBack;
		m_pane.SetZoomViewInfo(&zi);

		m_dwStat = (m_dwStat & BST_TBSMALL) | (dlg.m_startup_scale << 16);
	}
}

void CMainFrame::OnFilterPlugin(UINT nID, UINT code, HWND hwnd)
{
	HDIB hdib = m_pane.GetImage();
	CFilter* filter = m_mapPlugin.Lookup(nID);
	if (hdib && filter) {
		FILTER_EXEC fe;
		memset(&fe, 0, sizeof(fe));
		fe.progress = &CMainFrame::ProgressCallback;
		fe.tracker = &CMainFrame::TrackImage;
		fe.gradeditor = &CMainFrame::GradationSelector;
		CAppReg reg;
		fe.hkey = reg.GetKey(PLUGIN_SECTION);
		fe.hparent = m_hWnd;
		fe.name = m_strName;
		fe.scale = m_pane.GetScale();
		TCHAR dir[MAX_PATH * 2];
		GetDataFolder(dir);
		fe.userdir = dir;

		CRect rc;
		m_pane.GetClipRect(&rc);
		LPRECT prc = rc.IsRectEmpty() ? NULL : &rc;
		hdib = filter->Execute(hdib, prc, &fe);
		RegCloseKey(fe.hkey);
		if (hdib) {
			if (user_ristriction())
				UpdateDIB(hdib, nID, TRUE, (prc != NULL));
			else
				GlobalFree(hdib);
		}
	}
}

bool CMainFrame::PrintPage(UINT nPage, HDC hdc)
{
	HDIB hdib = m_pane.GetImage();
	if (!hdib) return false;
	if (nPage >= 1) return false;

	// 選択範囲
	bool needfree = false;
	mif image(hdib);
	if (m_print_selrect) {
		CRect rc;
		if (m_pane.GetClipRect(&rc) && !rc.IsRectEmpty()) {
			image.crop(&rc);
			hdib = image.createdib();
			if (!hdib) return false;

			needfree = true;
		}
	}

	Graphics g(hdc);
	g.SetPageUnit(UnitPixel);			// ユニットベースはピクセル（印刷時この指定は重要）
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetSmoothingMode(SmoothingModeHighQuality);
	g.SetPixelOffsetMode(PixelOffsetModeHighQuality);

	// 印刷サイズ
	int x_printer = ::GetDeviceCaps(hdc, HORZRES);
	int y_printer = ::GetDeviceCaps(hdc, VERTRES);

	CRect rectDraw;
	rectDraw.SetRect(0, 0, ::GetDeviceCaps(hdc, HORZRES), ::GetDeviceCaps(hdc, VERTRES));

	// イメージ情報
	BITMAPINFO* pbi = (BITMAPINFO*)GlobalLock(hdib);
	BYTE* pbits = dib_findbits(pbi);
	int width = pbi->bmiHeader.biWidth;
	int height = pbi->bmiHeader.biHeight;
	double per_width = (double)x_printer / (double)width;
	double per_height = (double)y_printer / (double)height;

	int scale_width, scale_height;
	double scale_factor = (per_width < per_height) ? per_width : per_height;
	scale_width = (int)((double)width * scale_factor);
	scale_height = (int)((double)height * scale_factor);

	// センタリング
	CPoint pt(0, 0);
	if (rectDraw.Width() > scale_width)
		pt.x = (rectDraw.Width() - scale_width) / 2;
	if (rectDraw.Height() > scale_height)
		pt.y = (rectDraw.Height() - scale_height) / 2;

	Bitmap* b = new Bitmap(pbi, pbits);
	g.DrawImage(b, pt.x, pt.y, scale_width, scale_height);
	delete b;
	GlobalUnlock(hdib);

	if (needfree) GlobalFree(hdib);

	return true;
}

void CMainFrame::OnPrint()
{
	if (m_printer.IsNull()) {
		// 印刷用
		m_printer.OpenDefaultPrinter();
		m_devmode.CopyFromPrinter(m_printer);
	}

	CPrintDialog dlg(FALSE);
	dlg.m_pd.hDevMode = m_devmode.CopyToHDEVMODE();
	dlg.m_pd.hDevNames = m_printer.CopyToHDEVNAMES();
	dlg.m_pd.nMinPage = 1;
	dlg.m_pd.nMaxPage = 1;
	dlg.m_pd.nFromPage = 1;
	dlg.m_pd.nToPage = 1;

	// 選択部分の印刷の有効化
	CRect rc;
	if (m_pane.GetClipRect(&rc) && !rc.IsRectEmpty())
		dlg.m_pd.Flags &= ~PD_NOSELECTION;

	if (dlg.DoModal() == IDOK) {
		m_print_selrect = ((dlg.m_pd.Flags & PD_SELECTION) != 0);

		m_devmode.CopyFromHDEVMODE(dlg.m_pd.hDevMode);
		m_printer.ClosePrinter();
		m_printer.OpenPrinter(dlg.m_pd.hDevNames, m_devmode.m_pDevMode);

		CPrintJob job;
		int nMin=0;
		int nMax = 0;
		job.StartPrintJob(false, m_printer, m_devmode.m_pDevMode, this, _T("VWIMG Document"), nMin, nMax);
	}
	GlobalFree(dlg.m_pd.hDevMode);
	GlobalFree(dlg.m_pd.hDevNames);
}

BOOL CALLBACK CMainFrame::ProgressCallback(int msg, int param)
{
	switch (msg) {
		case PROG_INITIALIZE:
			if (!spMainFrame->m_progress.IsWindow()) {
				CRect rc;
				spMainFrame->m_statusbar.GetPaneRect(ID_DEFAULT_PANE, &rc);
				rc.right = min(200, rc.right);
				rc.InflateRect(-3, -3);
				spMainFrame->m_progress.Create(spMainFrame->m_statusbar, rc, NULL, WS_VISIBLE | WS_CHILD);
				spMainFrame->m_progress.SetBarColor(RGB(64, 0, 128), RGB(0, 128, 255));
			}
			// fall down to...

		case PROG_RESET:
			if (spMainFrame->m_progress.IsWindow()) {
				spMainFrame->m_progress.SetRange32(0, param);
				spMainFrame->m_progress.SetPos(0);
			}
			break;

		case PROG_CONTINUE:
			if (spMainFrame->m_progress.IsWindow())
				spMainFrame->m_progress.StepIt();
			break;

		case PROG_DONE:
		default:
			if (spMainFrame->m_progress.IsWindow())
				spMainFrame->m_progress.DestroyWindow();
			break;
	}
	return TRUE;
}

BOOL CALLBACK CMainFrame::TrackImage(HDIB hdibTrack, POINT* ppt, COLORREF clrMask)
{
	mif image;
	if (!hdibTrack || !ppt || !image.attachdib(hdibTrack)) return FALSE;

	int scale = spMainFrame->m_pane.GetScale();
	if (scale != 1000)
		image.zoom_near(MulDiv(image.m_width, scale, 1000), MulDiv(image.m_height, scale, 1000));

	BOOL bOK = FALSE;
	HBITMAP hbm = image.createbitmap(NULL);
	image.destroy();
	if (hbm) {
		BITMAP bm;
		GetObject(hbm, sizeof(BITMAP), &bm);
		HIMAGELIST him = ImageList_Create(bm.bmWidth, bm.bmHeight, ILC_COLORDDB | ILC_MASK, 1, 0);
		ImageList_AddMasked(him, hbm, clrMask);
		DeleteObject(hbm);

		bOK = spMainFrame->m_pane.TrackImage(him, ppt);
		ImageList_Destroy(him);
	}
	return bOK;
}

BOOL CALLBACK CMainFrame::GradationSelector(HWND hwndParent, GRADEDITINFO* pgsi)
{
	return GradationEdit(hwndParent, pgsi);
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
	url.LoadString(IDS_WEB_HELP_EDIT);
	ShellExecute(NULL, _T("open"), url.GetString(), NULL, NULL, SW_SHOW);
}


// ViewFrm.cpp : implmentation of the CViewFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include "resource.h"

#include "common.h"
#include "viewFrm.h"
#include "slidemenu.h"
#include "dib_sup.h"
#include "qsort.h"
#include "select_folder.h"
#include "sizeclip.h"


///////////////////////////////////////////////////////////////////////
// defines

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

typedef struct tagTBARINFO {
	int nCol;
	int nRow;
	int nWidth;
} TBARINFO, *LPTBARINFO;

typedef struct tagIMAGEINFOSETTING {
	BOOL visible_info;
	BOOL visible_thumb;
	BOOL thumb_horz;
	int width;
	int cxySplit;
	DWORD stat;
} IMAGEINFOSETTING;

static CViewFrame* spViewFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
//

BOOL CViewFrame::PreTranslateMessage(MSG* pMsg)
{
	return CFrameWindowImpl<CViewFrame>::PreTranslateMessage(pMsg);
}

BOOL CViewFrame::OnIdle()
{
	UpdateUIAll();
	return FALSE;
}

void CViewFrame::TaskbarButtonCreated(ITaskbarList3* pTaskbarList)
{
/*
	if (pTaskbarList != NULL) {
		HICON hIcon = LoadIcon(RESINST, MAKEINTRESOURCE(IDR_VIEW));
		if (hIcon) {
			pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);
			DestroyIcon(hIcon);
		}
	}
*/
}

void CViewFrame::UpdateUIAll()
{
	int cnt = m_thumb.GetItemCount();
	int pos = m_thumb.GetSelectedIndex();
	UIEnable(IDM_FILE_PREV, cnt > 1 && pos > 0);
	UIEnable(IDM_FILE_NEXT, cnt > 1 && pos < cnt - 1);

	UIEnable(IDM_FILE_DELETE, PathFileExists(m_strName));

	CRect rc;
	BOOL b = (m_pane.GetClipRect(&rc) && !rc.IsRectEmpty());
	UIEnable(IDM_EDIT_COPY, b);

	UISetCheck(IDM_TOOL_EXIFINFO, m_infoview.IsWindowVisible());
	UISetCheck(IDM_TOOL_THUMBLIST, m_thumb.IsWindowVisible());
	b = ((m_dwStat & BST_GRAB) != 0);
	UISetCheck(IDM_MODE_GRAB, b);
	UISetCheck(IDM_MODE_SEL, !b);
	UISetCheck(IDM_TOOL_TBSMALL, (m_dwStat & BST_TBSMALL) != 0);

	DWORD dw = m_pane.CanZoom();
	UIEnable(IDM_ZOOM_IN, LOWORD(dw) != 0);
	UIEnable(IDM_ZOOM_OUT, HIWORD(dw) != 0);

	UIEnable(IDM_TOOL_GPS, m_bGps);

	CString cs;
	int iScale = m_pane.GetScale();
	int s = iScale / 10;
	int m = iScale % 10;
	if (!m)	cs.Format(_T("%d%%"), s);
	else 	cs.Format(_T("%d.%d%%"), s, m);
	m_statusbar.SetPaneText(ID_INDICATOR_SCALE, cs);

	UIUpdateToolBar();
}

void CViewFrame::Construct()
{
	CAppReg reg;
	m_strName.Empty();
	m_bGps = FALSE;
	m_hicScale = NULL;
	m_hicSizeGrip = NULL;
	m_hicRgbGrip = NULL;
	m_hdlgPrintReady = NULL;
	m_sizeDib = CSize(0, 0);
	m_bMouseWheelProcessing = false;
	m_anim = NULL;
	m_magic = 0;
}

void CViewFrame::SetDIBInformation(HDIB hdib)
{
	int bits = 0;
	if (hdib) {
		LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
		m_sizeDib = CSize(pbh->biWidth, pbh->biHeight);
		bits = pbh->biBitCount;
		GlobalUnlock(hdib);
	}
	else
		m_sizeDib = CSize(0, 0);

	CString str;
	str.Format(_T("%d x %d"), m_sizeDib.cx, m_sizeDib.cy);
	m_statusbar.SetPaneText(ID_INDICATOR_SQUARE, str);
	str.Format(_T("%dbpp"), bits);
	m_statusbar.SetPaneText(ID_INDICATOR_COLOR, str);
}

void CViewFrame::SetDIB(HDIB hdib, BOOL bRedraw)
{
	if (m_pane.IsWindow()) {
		CRect rc(0, 0, 0, 0);
		m_pane.SetClipRect(&rc);
	}

	hdib = m_pane.SetImage(hdib, TRUE);
	if (hdib) GlobalFree(hdib);
}

void CViewFrame::ShowDIB(HDIB hdib)
{
	CRect rc;
	m_pane.GetClientRect(&rc);

	// 直前のイメージサイズを保存
	CSize sizePrevDib = m_sizeDib;

	// 新しいイメージをセット
	SetDIBInformation(hdib);

	// 以前の情報
	// イメージサイズに変更がなければ、直前のスケール・中心位置を保持して次のイメージを表示
	ZOOMVIEWINFO zvi;
	zvi.cbSize = sizeof(zvi);
	zvi.fMask = ZIF_SCALE | ZIF_CENTERPOSITION;
	m_pane.GetZoomViewInfo(&zvi);

	if (!IsWindowVisible()) {
		ShowWindow(SW_SHOWNORMAL);
		UpdateWindow();
	}

	// イメージをリセット
	SetDIB(NULL, FALSE);

	// 新しいイメージが直前のイメージとサイズが異なれば、スケールをリセット
	if (m_sizeDib != sizePrevDib)
		m_pane.SetScale(1000);

	// イメージをセット
	SetDIB(hdib, FALSE);

	if (m_sizeDib != sizePrevDib) {
		if (rc.Width() < m_sizeDib.cx || rc.Height() < m_sizeDib.cy)
			m_pane.ZoomFull();
		else
			m_pane.ZoomDefault();
	}
	else
		m_pane.SetZoomViewInfo(&zvi);
}


/////////////////////////////////////////////////////////////////////////////


int CViewFrame::OnCreate(LPCREATESTRUCT /*pcs*/)
{
	spViewFrame = this;
	Construct();

	CAppReg reg;
	IMAGEINFOSETTING iis;
	if (reg.GetBinary(IDS_SEC_MAIN_VIEWER, IDS_KEY_IMAGEINFO, &iis, sizeof(iis)) != sizeof(iis)) {
		iis.visible_info = TRUE;
		iis.visible_thumb = TRUE;
		iis.thumb_horz = FALSE;
		iis.width = GetSystemMetrics(SM_CXVSCROLL) + 104;
		iis.cxySplit = -1;
		iis.stat = (GetSystemMetrics(SM_CYFULLSCREEN) < 800) ? BST_TBSMALL : 0;
	}
	m_dwStat = iis.stat;

	HMENU hMainMenu = GetMenu();

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
	m_menubar.LoadMultipleImages(IDB_S_VIEWBAR);
	m_menubar.LoadMultipleImages(IDB_S_VIEW_APPICON);
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

	// （イメージペイン + サムネイルリスト） + 諸情報のスプリタ
	DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rc;
	GetClientRect(&rc);

	m_splitApp.SetBarColor(LCOLOR_FACE, FALSE);
	m_splitApp.Create(m_hWnd, rc, NULL, style, 0, ATL_IDW_PANE_FIRST);
	m_splitApp.SetSplitterExtendedStyle(SPLIT_RIGHTALIGNED);
	m_splitApp.GetSystemSettings(false);
	m_splitApp.m_cxyMinTopLeft = 320;
	m_splitApp.m_cxyMinBottomRight = 180;
	m_hWndClient = m_splitApp;

	// 水平分割イメージペイン + サムネイルリストのスプリタ
	m_splitImageHorz.SetBarColor(LCOLOR_FACE, FALSE);
	m_splitImageHorz.Create(m_splitApp, rcDefault, NULL, style, 0, ATL_IDW_PANE_FIRST + 1);
	m_splitImageHorz.SetSplitterExtendedStyle(SPLIT_RIGHTALIGNED);
	m_splitImageHorz.GetSystemSettings(false);
	m_splitImageHorz.m_cxyMinTopLeft = 320;
	m_splitImageHorz.m_cxyMinBottomRight = 80;

	// 垂直分割イメージペイン + サムネイルリストのスプリタ
	m_splitImageVert.SetBarColor(LCOLOR_FACE, FALSE);
	m_splitImageVert.Create(m_splitApp, rcDefault, NULL, style, 0, ATL_IDW_PANE_FIRST + 1);
	m_splitImageVert.SetSplitterExtendedStyle(SPLIT_RIGHTALIGNED);
	m_splitImageVert.GetSystemSettings(false);
	m_splitImageVert.m_cxyMinTopLeft = 320;
	m_splitImageVert.m_cxyMinBottomRight = 80 + GetSystemMetrics(SM_CXVSCROLL);

	HWND hwndSubpane = iis.thumb_horz ? m_splitImageHorz.m_hWnd : m_splitImageVert.m_hWnd;

	// 情報ウィンドウ
	m_infoview.Create(m_splitApp, rcDefault, NULL, style, 0, IDR_INFOVIEW);
	m_splitApp.SetSplitterPanes(hwndSubpane, m_infoview);
	m_splitApp.SetSplitterPos(rc.Width() - iis.width);
	if (!iis.visible_info) m_splitApp.SetSinglePaneMode(SPLIT_PANE_LEFT);

	// イメージ表示画面
	m_pane.Create(hwndSubpane, rcDefault, NULL, style | ZMVS_STYLE_SEL, 0, IDW_ZOOMVIEW);
	m_pane.SetBkColor(LCOLOR_IMAGEPANE, FALSE);
	m_pane.SetDrawMode(ZVDM_BITMAP);
	m_pane.SetMode((m_dwStat & BST_GRAB) ? ZOOMMODE_GRAB : ZOOMMODE_SEL);

	// サムネイルリスト
	m_thumb.Create(hwndSubpane, rcDefault, NULL, style | LVS_NOLABELWRAP | LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_ALIGNTOP | LVS_ICON | LVS_SHOWSELALWAYS , 0, IDW_THUMBLIST);
	m_thumb.SetExtendedListViewStyle(m_thumb.GetExtendedListViewStyle() | LVS_EX_DOUBLEBUFFER);
	m_thumb.InitialUpdate();
	if (iis.cxySplit < 0) {
		// サムネイルリストの幅／高さが未定義の場合
		DWORD sizeItem = m_thumb.ApproximateViewRect(-1, -1, 1);
		iis.cxySplit = iis.thumb_horz ? (HIWORD(sizeItem) + GetSystemMetrics(SM_CYHSCROLL)) : (LOWORD(sizeItem) + GetSystemMetrics(SM_CXVSCROLL));
	}

	if (iis.thumb_horz) {
		m_splitImageVert.ShowWindow(SW_HIDE);
		m_splitImageHorz.SetSplitterPanes(m_pane, m_thumb);
		m_splitImageHorz.SetSplitterPos(rc.Height() - iis.cxySplit);
		if (!iis.visible_thumb) m_splitImageHorz.SetSinglePaneMode(SPLIT_PANE_TOP);
	}
	else {
		m_splitImageHorz.ShowWindow(SW_HIDE);
		m_splitImageVert.SetSplitterPanes(m_pane, m_thumb);
		m_splitImageVert.SetSplitterPos(rc.Width() - (iis.visible_info ? iis.width : 0) - iis.cxySplit);
		if (!iis.visible_thumb) m_splitImageHorz.SetSinglePaneMode(SPLIT_PANE_LEFT);
	}

	// Loading animation
	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_LOADER_WHITE_48X48), 48, CLR_NONE);
	m_anim = new AnimationImageList(him, 48);
	if (m_anim) m_anim->InitAnimation(m_pane);

	// loader thread
	m_loader.Initialize(m_hWnd, m_nRM_LOADIMAGECOMPLETED);

	// ツールバー更新
	UIAddToolBar(hWndToolBar);

	// recent file list
	HMENU hMenu = GetSubMenuByPosition(0, 5);
	m_mru.SetMenuHandle(hMenu);
	CString str = reg.GetRootName();
	m_mru.ReadFromRegistry(str);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// 前回のウィンドウ位置
	WINDOWPLACEMENT wpm;
	if (reg.GetBinary(IDS_SEC_MAIN_VIEWER, IDS_KEY_WNDPOS, &wpm, sizeof(wpm)) == sizeof(wpm)) {
		CRect rc = wpm.rcNormalPosition;
		if (CountSiblings(m_hWnd) > 0) {
			GetWindowPlacement(&wpm);
			rc.OffsetRect(wpm.rcNormalPosition.left - rc.left, wpm.rcNormalPosition.top - rc.top);
		}
		lccMapRectToProgmanPane(&rc);
		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	DragAcceptFiles(TRUE);
	m_splitApp.SetActivePane(SPLIT_PANE_LEFT);
	return 0;
}

HWND CViewFrame::CreateToolbar()
{
	if (m_toolbar.IsWindow())
		RemoveCustomDrawBar(m_toolbar.m_hWnd);

	UINT id = (m_dwStat & BST_TBSMALL) ? IDB_S_VIEWBAR : IDB_M_VIEWBAR;
	HWND hWndToolBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(id), ATL_SIMPLE_TOOLBAR_PANE_STYLE, ATL_IDW_TOOLBAR);
	m_toolbar = hWndToolBar;

	AddCustomDrawBar(hWndToolBar);
	return hWndToolBar;
}

LRESULT CViewFrame::OnActivate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (LOWORD(wParam) != WA_INACTIVE)
		m_splitApp.SetActivePane(SPLIT_PANE_LEFT);
	bHandled = FALSE;
	return 1;
}

void CViewFrame::OnDestroy()
{
	if (m_anim) {			delete m_anim; m_anim = NULL; }
	if (m_hicScale)			DestroyIcon(m_hicScale);
	if (m_hicSizeGrip)		DestroyIcon(m_hicSizeGrip);
	if (m_hicRgbGrip)		DestroyIcon(m_hicRgbGrip);

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
}

void CViewFrame::UpdateLayout(BOOL bResizeBars)
{
	CFrameWindowImpl::UpdateLayout(bResizeBars);
}

HMENU CViewFrame::GetSubMenuByPosition(int n1stPos, int n2ndPos)
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

void CViewFrame::SetCaptionTitle(LPCTSTR pszTitle)
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
}

void CViewFrame::OnContextMenu(HWND hWnd, CPoint pt)
{
	if ((m_pane.m_hWnd == WindowFromPoint(pt)) && PathFileExists(m_strName)) {
		lccContextMenuByFile(m_hWnd, m_strName, &pt);
	}
}

BOOL CViewFrame::OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState)
{
	UINT idCommand = 0;
	switch (appCommand) {
	case APPCOMMAND_BROWSER_BACKWARD:	idCommand = IDM_FILE_PREV;	break;
	case APPCOMMAND_BROWSER_FORWARD:	idCommand = IDM_FILE_NEXT;	break;
	case APPCOMMAND_CLOSE:				idCommand = IDM_FILE_CLOSE;	break;
	}
	if (idCommand) {
		PostMessage(WM_COMMAND, idCommand, 0);
		return TRUE;
	}
	return FALSE;
}

void CViewFrame::OnDropFiles(HDROP hDrop)
{
	// ドロップされるファイル数を取得
	if (DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0) > 0) {
		TCHAR buf[MAX_PATH * 2];
		DragQueryFile(hDrop, 0, buf, _COUNTOF(buf));
		if (imgCheckImageType(buf) != IMG_NOTSUPPORTED) {
			OpenFile(buf, true);
		}
	}
	DragFinish(hDrop);
}

void CViewFrame::OnClose(BOOL& bHandled)
{
	// イメージ情報ペイン位置
	IMAGEINFOSETTING iis;

	CRect rc;
	m_splitApp.GetWindowRect(&rc);
	iis.width = rc.Width() - m_splitApp.GetSplitterPos();

	if (m_splitImageHorz.IsWindowVisible()) {
		iis.thumb_horz = TRUE;
		m_splitImageHorz.GetWindowRect(&rc);
		iis.cxySplit = rc.Height() - m_splitImageHorz.GetSplitterPos();
	}
	else {
		iis.thumb_horz = FALSE;
		m_splitImageVert.GetWindowRect(&rc);
		iis.cxySplit = rc.Width() - m_splitImageVert.GetSplitterPos();
	}
	iis.visible_info = m_infoview.IsWindowVisible();
	iis.visible_thumb = m_thumb.IsWindowVisible();
	iis.stat = m_dwStat;
	CAppReg reg;
	reg.SetBinary(IDS_SEC_MAIN_VIEWER, IDS_KEY_IMAGEINFO, &iis, sizeof(iis));

	// ウィンドウ位置
	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(wpm);
	if (GetWindowPlacement(&wpm))
		reg.SetBinary(IDS_SEC_MAIN_VIEWER, IDS_KEY_WNDPOS, &wpm, sizeof(wpm));

	CString str = reg.GetRootName();
	m_mru.WriteToRegistry(str);
}

void CViewFrame::OnGetMinMaxInfo(MINMAXINFO* pMMI) 
{
	pMMI->ptMinTrackSize.x = 320;
	pMMI->ptMinTrackSize.y = 200;
}

void CViewFrame::OnFileClose()
{
	PostMessage(WM_CLOSE);
}

BOOL CViewFrame::OpenFile(LPCTSTR pszName, bool bEnumListup)
{
	m_strName.Empty();
	m_magic = 0;
	m_bGps = FALSE;
	if (!PathFileExists(pszName) || PathIsDirectory(pszName)) return FALSE;
	if (imgCheckImageType(pszName) == IMG_NOTSUPPORTED) return FALSE;

	if (m_loader.IsRunning()) {
		MessageBeep(0);
		return FALSE;
	}

	if (m_anim) {
		// 都度ペインの位置を確認
		CRect rc;
		m_pane.GetClientRect(&rc);
		m_anim->SetPosition(rc.CenterPoint());
		m_anim->Start();
	}

	m_strName = pszName;
	m_magic = GetTickCount();
	m_loader.Start(m_magic, (LPARAM)m_strName.GetString());

	if (m_thumb.SelectFileItem(m_strName) < 0 && bEnumListup)
		m_thumb.Listup(m_strName);
	return TRUE;
}

LRESULT CViewFrame::OnLoadImageCompleted(WPARAM wParam, LPARAM lParam)
{
	HDIB hdib = (HDIB)lParam;
	DWORD magic = m_magic;
	m_magic = 0;

	if (hdib) {
		if ((DWORD)wParam == magic) {
			m_mru.AddToList(m_strName);
			SetCaptionTitle(m_strName);
			ShowDIB(hdib);
			PostMessage(WM_COMMAND, IDM_PARSE_INFORMATION, 0);
		}
		else
			GlobalFree(hdib);
	}
	else
		ShowDIB(NULL);

	if (m_anim)
		m_anim->Stop();
	UpdateWindow();

	m_loader.Stop();
	return 0;
}

LRESULT CViewFrame::OnHandlerMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lr = 0;
	if (!m_bMouseWheelProcessing) {
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		CWindow wnd = WindowFromPoint(pt);
		if (IsChild(wnd)) {
			m_bMouseWheelProcessing = true;
			lr = wnd.SendMessage(msg, wParam, lParam);
			m_bMouseWheelProcessing = false;
		}
	}
	return lr;
}

static int compname(const void *arg1, const void *arg2)
{
	LPCTSTR p1 = *(TCHAR**)arg1;
	LPCTSTR p2 = *(TCHAR**)arg2;
	return StrCmpLogicalW(PathFindFileName(p1), PathFindFileName(p2));
}

void CViewFrame::OnFileNext()
{
	TCHAR buf[MAX_PATH * 2];
	int cnt = m_thumb.GetItemCount();
	int index = m_thumb.GetSelectedIndex() + 1;
	if (cnt > 1) {
		if (index >= cnt)
			index = 0;
		if (m_thumb.GetItemFileName(index, buf, ARRAYSIZE(buf)))
			OpenFile(buf, false);
	}
}

void CViewFrame::OnFilePrev()
{
	TCHAR buf[MAX_PATH * 2];
	int cnt = m_thumb.GetItemCount();
	int index = m_thumb.GetSelectedIndex() - 1;
	if (cnt > 1) {
		if (index < 0)
			index = cnt - 1;
		if (m_thumb.GetItemFileName(index, buf, ARRAYSIZE(buf)))
			OpenFile(buf, false);
	}
}

void CViewFrame::OnFileOpen() 
{
/*
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, m_strName);
	if (FileBrowse(m_hWnd, IDS_CAPTION_LOADBMP, IDS_FILTER_PICTURE, buf, TRUE, NULL)) {
		OpenFile(buf, true);
	}
*/

	TCHAR path[MAX_PATH * 2];
	LPITEMIDLIST pidlBase;
	if (PathFileExists(m_strName)) {
		lstrcpy(path, m_strName);
		PathRemoveFileSpec(path);
		pidlBase = ILCreateFromPath(path);
	}
	else
		SHGetSpecialFolderLocation(m_hWnd, CSIDL_MYPICTURES, &pidlBase);

	CSelectFolderDlg sfd;
	LPITEMIDLIST pidlTarget = sfd.SelectFolder(m_hWnd, pidlBase);
	if (pidlTarget) {
		if (SHGetPathFromIDList(pidlTarget, path)) {
			// 面倒なけど一応フォルダを検索して、名前順にソートして、最初のファイル名を取得する
			TCHAR name[MAX_PATH * 2];
			PathCombine(name, path, _T("*.*"));

			CSimpleArray<LPTSTR> array;
			WIN32_FIND_DATA wfd;
			HANDLE hFind = FindFirstFile(name, &wfd);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
					if (imgSpecifyImageType(wfd.cFileName) == IMG_NOTSUPPORTED) continue;

					array.Add(StrDup(wfd.cFileName));
				} while (FindNextFile(hFind, &wfd));
				FindClose(hFind);
			}

			int entries = array.GetSize();
			if (entries > 0) {
				if (entries > 1) {
					CQSort qs;
					qs.sort(array.GetData(), entries, sizeof(LPTSTR), NULL, NULL);
				}

				PathCombine(name, path, array[0]);
				for (int i = 0; i < entries; i++)
					LocalFree(array[i]);

				OpenFile(name, true);
			}
		}
		CoTaskMemFree(pidlTarget);
	}
	CoTaskMemFree(pidlBase);
}

void CViewFrame::OnFileDelete()
{
	if (PathFileExists(m_strName)) {
		if (AppMessage(IDR_VIEW, IDS_QUERY_DELETE_FILE, PathFindFileName(m_strName), MB_QYESNO) == IDYES) {
			TCHAR buf[MAX_PATH * 2];
			if (GetKeyState(VK_SHIFT) < 0) {
				lstrcpy(buf, m_strName);
				buf[lstrlen(buf) + 1] = 0;
				fopTrashFile(m_hWnd, buf, FOF_ALLOWUNDO);
			}
			else
				fopRemoveFileNoConfirm(m_strName);

			if (!PathFileExists(m_strName)) {
				int cnt = m_thumb.GetItemCount();
				if (cnt == 1)
					OnFileClose();
				else {
					m_pane.Invalidate(FALSE);
					m_pane.UpdateWindow();
					int index = m_thumb.GetSelectedIndex();
					if (m_thumb.DeleteFileItem(index)) {
						if (index == cnt - 1)
							index--;
						if (m_thumb.GetItemFileName(index, buf, ARRAYSIZE(buf)))
							OpenFile(buf, false);
					}
				}
			}
		}
	}
}

void CViewFrame::OnFileContextMenu()
{
	CRect rc;
	m_pane.GetWindowRect(&rc);
	CPoint pt(rc.left + rc.Width() / 2, rc.top + rc.Height() / 4);
	OnContextMenu(m_hWnd, pt);
}

void CViewFrame::OnEditCopy()
{
	HDIB hdib = m_pane.GetImage();
	if (!hdib) return;

	CRect rc;
	m_pane.GetClipRect(&rc);
	mif image(hdib);
	if (image.crop(&rc)) {
		HDIB hdib = image.createdib();
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

void CViewFrame::OnEditSelAll()
{
	if (!m_pane.GetImage()) return;

	CRect rc(0, 0, m_sizeDib.cx, m_sizeDib.cy);
	m_pane.SetClipRect(&rc);
}

void CViewFrame::OnEditSelCancel()
{
	CRect rc;
	if (m_pane.GetClipRect(&rc) && !rc.IsRectEmpty()) {
		rc.SetRectEmpty();
		m_pane.SetClipRect(&rc);
	}
}

void CViewFrame::OnEditSizeClip()
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

void CViewFrame::OnRotateLeft() 
{
	HDIB hdib = m_pane.GetImage();
	if (!hdib) return;

	mif image(hdib);
	image.rotate(TRUE);
	hdib = image.createdib();
	if (hdib) {
		// スケール・中心位置を固定してイメージを表示
		ZOOMVIEWINFO zvi;
		memset(&zvi, 0, sizeof(zvi));
		zvi.cbSize = sizeof(zvi);
		zvi.fMask = ZIF_SCALE | ZIF_CENTERPOSITION;
		m_pane.GetZoomViewInfo(&zvi);

		// 回転したイメージをセットする前にポジションを計算しておく
		long tmp = zvi.ptCenterPosition.x;
		zvi.ptCenterPosition.x = zvi.ptCenterPosition.y;
		zvi.ptCenterPosition.y = m_sizeDib.cx - tmp;

		SetDIBInformation(hdib);
		SetDIB(NULL, FALSE);
		SetDIB(hdib, FALSE);
		m_infoview.OnUpdate(IFV_IMAGE_CHANGED, (LPARAM)hdib);
		m_pane.SetZoomViewInfo(&zvi);
	}
}

void CViewFrame::OnRotateRight() 
{
	HDIB hdib = m_pane.GetImage();
	if (!hdib) return;

	mif image(hdib);
	image.rotate(FALSE);
	hdib = image.createdib();
	if (hdib) {
		// スケール・中心位置を固定してイメージを表示
		ZOOMVIEWINFO zvi;
		memset(&zvi, 0, sizeof(zvi));
		zvi.cbSize = sizeof(zvi);
		zvi.fMask = ZIF_SCALE | ZIF_CENTERPOSITION;
		m_pane.GetZoomViewInfo(&zvi);

		// 回転したイメージをセットする前にポジションを計算しておく
		long tmp = zvi.ptCenterPosition.x;
		zvi.ptCenterPosition.x = m_sizeDib.cy - zvi.ptCenterPosition.y;
		zvi.ptCenterPosition.y = tmp;

		SetDIBInformation(hdib);
		SetDIB(NULL, FALSE);
		SetDIB(hdib, FALSE);
		m_infoview.OnUpdate(IFV_IMAGE_CHANGED, (LPARAM)hdib);
		m_pane.SetZoomViewInfo(&zvi);
	}
}

void CViewFrame::OnExecEditor()
{
	if (PathFileExists(m_strName))
		lccDllExecute(m_hWnd, DLL_GMIL, _T("pxfRun"), m_strName, _T("/image"), FALSE);
}

void CViewFrame::OnExecSlideShow()
{
	if (PathFileExists(m_strName)) {
		lccDllExecute(m_hWnd, DLL_GMIL, _T("slideRun"), m_strName, _T("/image"), FALSE);
		PostMessage(WM_CLOSE);
	}
}

void CViewFrame::OnSetZoomMode()
{
	int mode = (m_pane.GetMode() == ZOOMMODE_ZOOM) ? ZOOMMODE_SEL : ZOOMMODE_ZOOM;
	m_pane.SetMode(mode);
}

void CViewFrame::OnNotifyKeyDown(UINT id, NMHDR* pNmhdr, LRESULT* pResult)
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

void CViewFrame::OnNotifyThumbnailList(NMHDR* pNmhdr, LRESULT* pResult)
{
	switch (pNmhdr->code) {
	case NM_CUSTOMDRAW:
		m_thumb.ProcessCustomDraw((NMLVCUSTOMDRAW*)pNmhdr, pResult);
		break;
	case NM_DBLCLK:
	case NM_RCLICK:
	{
		int index = m_thumb.GetSelectedIndex();
		if (index >= 0) {
			TCHAR buf[MAX_PATH * 2];
			if (m_thumb.GetItemFileName(index, buf, ARRAYSIZE(buf))) {
				if (pNmhdr->code == NM_DBLCLK)
					OpenFile(buf, false);
				else {
					CPoint pt;
					GetCursorPos(&pt);
					lccContextMenuByFile(m_thumb, buf, &pt);
				}
			}
		}
		break;
	}
	case LVN_BEGINDRAG:
		m_thumb.DragStart(((NMLISTVIEW*)pNmhdr)->iItem);
		break;
	default:
		break;
	}
}

void CViewFrame::OnNotifyMousePoint(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	ZOOM_MOUSEPOINT* pzm = (ZOOM_MOUSEPOINT*)pNmhdr;
	TCHAR buf[MAX_PATH];
	buf[0] = 0;
	if (pzm->bInDrag)
		wsprintf(buf, _T("X:%d Y:%d W:%d H:%d"),
				pzm->rcClip.left, pzm->rcClip.top,
				pzm->rcClip.right - pzm->rcClip.left, pzm->rcClip.bottom - pzm->rcClip.top);
	else if (pzm->pt.x >= 0 && pzm->pt.y >= 0 &&
			 pzm->pt.x < m_sizeDib.cx && pzm->pt.y < m_sizeDib.cy)
		wsprintf(buf, _T("X:%d Y:%d"), pzm->pt.x, pzm->pt.y);
	m_statusbar.SetPaneText(ID_INDICATOR_CURSOR, buf);
}

void CViewFrame::OnNotifyVisibleChanged(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	ZOOM_VISIBLEINFO* pzv = (ZOOM_VISIBLEINFO*)pNmhdr;
	m_infoview.OnUpdate(IFV_VISIBLEPOSITION_CHANGED, (LPARAM)&pzv->rectVisible);
}

void CViewFrame::OnNotifyScaleChanged(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	ZOOM_SCALE* pzs = (ZOOM_SCALE*)pNmhdr;
	double scale = pzs->scale;
	m_infoview.OnUpdate(IFV_SCALE_CHANGED, (LPARAM)&scale);		// doubleのアドレス渡し
}

void CViewFrame::OnNotifyNavigator(NMHDR* pNmhdr, LRESULT* pResult)
{
	NAVICLICK* pnc = (NAVICLICK*)pNmhdr;
	if (pNmhdr->code == NVN_CLICK) {
		ZOOMVIEWINFO zvi;
		zvi.cbSize = sizeof(zvi);
		zvi.fMask = ZIF_CENTERPOSITION;
		zvi.ptCenterPosition = pnc->pt;
		m_pane.SetZoomViewInfo(&zvi);
	}
	else if (pNmhdr->code == NVN_ZOOMIN)
		m_pane.ZoomIn();
	else if (pNmhdr->code == NVN_ZOOMOUT)
		m_pane.ZoomOut();
	else if (pNmhdr->code == NVN_SCALE)
		m_pane.SetScale((int)pnc->param);
}

void CViewFrame::OnNotifyClip(NMHDR* pNmhdr, LRESULT* pResult)
{
	TCHAR buf[MAX_PATH];
	ZOOM_CLIP* pbc = (ZOOM_CLIP*)pNmhdr;

	if (!pbc->rcClip.left && !pbc->rcClip.top &&
		!pbc->rcClip.right && !pbc->rcClip.bottom)
		buf[0] = 0;
	else {
		wsprintf(buf, _T("選択 %d x %d"), pbc->rcClip.right - pbc->rcClip.left, pbc->rcClip.bottom - pbc->rcClip.top);
	}
	m_statusbar.SetPaneText(ID_INDICATOR_SELECT, buf);
	*pResult = TRUE;
}

void CViewFrame::OnOpenRecentFile(UINT nID, UINT code, HWND hwnd)
{
	TCHAR sFile[MAX_PATH];
#if (_WTL_VER < 0x0800)
	if (m_mru.GetFromList(nID, sFile)) {
#else
	if (m_mru.GetFromList(nID, sFile, _COUNTOF(sFile))) {
#endif
		if (PathFileExists(sFile)) {
			m_mru.MoveToTop(nID);
			OpenFile(sFile, true);
		}
		else {
			m_mru.RemoveFromList(nID);
			MessageBeep(0);
		}
	}
}

void CViewFrame::OnToolExifInfo()
{
	m_splitApp.SetSinglePaneMode(m_infoview.IsWindowVisible() ? SPLIT_PANE_LEFT : SPLIT_PANE_NONE);
	m_pane.ZoomFull();
}

void CViewFrame::OnToolThumbnailList()
{
	BOOL bVisible = m_thumb.IsWindowVisible();
	if (m_splitImageHorz.IsWindowVisible())
		m_splitImageHorz.SetSinglePaneMode(bVisible ? SPLIT_PANE_LEFT : SPLIT_PANE_NONE);
	else
		m_splitImageVert.SetSinglePaneMode(bVisible ? SPLIT_PANE_TOP : SPLIT_PANE_NONE);

	if (!bVisible) {
		// 現在選択中のアイテムを可視化
		int index = m_thumb.GetSelectedIndex();
		if (index >= 0)
			m_thumb.EnsureVisible(index, FALSE);
	}
	m_pane.ZoomFull();
}

void CViewFrame::OnToggleThumbnailListLayout()
{
	// サムネイルリストの幅／高さの基本
	DWORD sizeItem = m_thumb.ApproximateViewRect(-1, -1, 1);
	BOOL visible_thumb = m_thumb.IsWindowVisible();
	if (m_splitImageHorz.IsWindowVisible()) {
		m_splitImageHorz.ShowWindow(SW_HIDE);
		m_splitImageHorz.UpdateWindow();
		m_pane.SetParent(m_splitImageVert);
		m_thumb.SetParent(m_splitImageVert);
		m_splitImageVert.SetSplitterPanes(m_pane, m_thumb);

		m_splitApp.SetSplitterPane(SPLIT_PANE_LEFT, m_splitImageVert);
		m_splitImageVert.ShowWindow(SW_SHOW);
		if (!visible_thumb) m_splitImageVert.SetSinglePaneMode(SPLIT_PANE_LEFT);
	} else {
		m_splitImageVert.ShowWindow(SW_HIDE);
		m_splitImageVert.UpdateWindow();
		m_pane.SetParent(m_splitImageHorz);
		m_thumb.SetParent(m_splitImageHorz);
		m_splitImageHorz.SetSplitterPanes(m_pane, m_thumb);

		m_splitApp.SetSplitterPane(SPLIT_PANE_LEFT, m_splitImageHorz);
		m_splitImageHorz.ShowWindow(SW_SHOW);
		if (!visible_thumb) m_splitImageHorz.SetSinglePaneMode(SPLIT_PANE_TOP);
	}
}

void CViewFrame::OnToolSmallToolbar()
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

void CViewFrame::OnToolGpsAccess()
{
	if (PathFileExists(m_strName) && m_bGps) {
		fopGpsAccess(m_hWnd, m_strName);
	}
}

void CViewFrame::OnModeGrab()
{
	m_dwStat |= BST_GRAB;
	m_pane.SetMode(ZOOMMODE_GRAB);
}

void CViewFrame::OnModeSel()
{
	m_dwStat &= ~BST_GRAB;
	m_pane.SetMode(ZOOMMODE_SEL);
}

bool CViewFrame::PrintPage(UINT nPage, HDC hdc)
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

void CViewFrame::OnPrint()
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

void CViewFrame::OnParseInformation()
{
	m_bGps = FALSE;
	if (PathFileExists(m_strName)) {
		m_infoview.ParseInformation(m_strName, m_pane.GetImage());
		// GPS
		m_bGps = fopHasGpsData(m_strName);
	}

	int state = m_toolbar.GetState(IDM_TOOL_GPS);
	if (state != -1) {
		if (m_bGps) {
			state &= ~TBSTATE_HIDDEN;
		} else {
			state |= TBSTATE_HIDDEN;
		}
		m_toolbar.SetState(IDM_TOOL_GPS, (UINT)state);
	}
}

void CViewFrame::OnAppAbout()
{
	APPABOUTINFO_V2 ai = { 0 };
	ai.magic = MAGIC_APPABOUTINFO_V2;
	ai.hInst = RESINST;
	ai.szUser[0] = ' ';
	ai.szCompany[0] = ' ';
	::LoadString(RESINST, IDS_WEB_HOMEPAGE, ai.szUrl, ARRAYSIZE(ai.szUrl));
	lccShowAppAbout(m_hWnd, (LPAPPABOUTINFO)&ai);
}

void CViewFrame::OnAppHelp()
{
	CString url;
	url.LoadString(IDS_WEB_HELP_VIEW);
	ShellExecute(NULL, _T("open"), url.GetString(), NULL, NULL, SW_SHOW);
}


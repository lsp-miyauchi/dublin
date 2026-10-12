// MainFrm.cpp : implmentation of the CMainFrame class
//

#include "stdafx.h"
#include "resource.h"
#include "beview.h"
#include "MainFrm.h"

extern BOOL gbInsert;

static const TCHAR sszTitleNone[] = _T("(無題)");

/////////////////////////////////////////////////////////////////////////////
//

static int ParseCommandLine(LPCTSTR pszCmdLine, CSimpleArray<CString>& array)
{
	array.RemoveAll();
	if (!pszCmdLine || *pszCmdLine == 0) return 0;

	TCHAR* cmdline = new TCHAR[lstrlen(pszCmdLine) + 2];
	lstrcpy(cmdline, pszCmdLine);

	TCHAR* p = cmdline;
	TCHAR* q;
	while (p && *p) {
		while (*p == _T(' ')) *p++;
		if (*p == 0) break;

		if (*p == _T('"')) {
			q = StrChr(p + 1, _T('"'));
			if (q) {
				++q;
				*q++ = 0;
			}
		}
		else {
			q = StrChr(p + 1, _T(' '));
			if (q)
				*q++ = 0;
		}
		array.Add(p);
		p = q;
	}

	delete [] cmdline;
	return array.GetSize();
}

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
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	m_view.OnIdle();		// ワーカースレッドで並行読み込みしているときは、この呼び出しで読み込んだデータが更新される

	UpdateUIAll();
	return FALSE;
}

void CMainFrame::TaskbarButtonCreated(ITaskbarList3* pTaskbarList)
{
	if (pTaskbarList != NULL) {
		HICON hIcon = LoadIcon(RESINST, MAKEINTRESOURCE(IDR_BJAIL));
		if (hIcon) {
			pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);
			DestroyIcon(hIcon);
		}
	}
}

void CMainFrame::UpdateUIAll()
{
	OnRefreshStatusBar();

	BOOL bCanUndo = m_view.CanUndo();
	BOOL bCanRedo = m_view.IsReadOnly();
	BOOL bCanPaste = m_view.CanPaste();
	BOOL bCanCopy = m_view.CanCopy();
	BOOL bCanCut = m_view.CanCut();
	BOOL bCanFind = m_view.CanContinueFind();
	BOOL bHasMark = m_view.HasMark();

	UIEnable(ID_FILE_SAVE, m_bModified);
	UIEnable(ID_FILE_SAVE_AS, TRUE);
	UIEnable(ID_EDIT_UNDO, bCanUndo);
	UIEnable(ID_EDIT_PASTE, bCanPaste);
	UIEnable(ID_EDIT_CUT, bCanCut);
	UIEnable(ID_EDIT_COPY, bCanCopy);
	UIEnable(ID_EDIT_TEXTCOPY, bCanCopy);
	UIEnable(ID_EDIT_SELSAVENEW, bCanCopy);
	UIEnable(ID_EDIT_BYNEWEDITOR, bCanCopy);
	UIEnable(ID_EDIT_SELTO_PREVMARK, bHasMark);
	UIEnable(ID_EDIT_SELTO_NEXTMARK, bHasMark);
	UIEnable(ID_FIND_NEXT, bCanFind);
	UIEnable(ID_FIND_PREV, bCanFind);
	UIEnable(ID_TOOL_CHECKSUM, bCanCopy);
	UISetCheck(ID_TEXT_ASCII, m_view.IsTextTypeAscii());
	UISetCheck(ID_TEXT_SJIS, m_view.IsTextTypeSjis());
	UIEnable(ID_MARK_CLEARALL, bHasMark);
	UIEnable(ID_MARK_PREV, bHasMark);
	UIEnable(ID_MARK_NEXT, bHasMark);
	UIEnable(ID_JUMP_LASTEDIT, bCanUndo);

	UIUpdateToolBar();
//@	UpdateStatusBar();
}

void CMainFrame::SetCommandLine(LPCTSTR pszCmdLine)
{
	m_strCmdLine = pszCmdLine;
}


LRESULT CMainFrame::OnRefreshStatusBar()
{
	size_t total;
	UINT_PTR curpos = m_view.GetCurrentAddress(&total);

	TCHAR buf[MAX_PATH];
	if (total & 0xff000000)			wsprintf(buf, _T("%X:%07X"), curpos, total);
	else if (total & 0xfff00000)	wsprintf(buf, _T("%X:%06X"), curpos, total);
	else if (total & 0xffff0000)	wsprintf(buf, _T("%X:%05X"), curpos, total);
	else if (total & 0xfffff000)	wsprintf(buf, _T("%X:%04X"), curpos, total);
	else							wsprintf(buf, _T("%X:%X"),   curpos, total);
	m_statusbar.SetPaneText(ID_INDICATOR_POS, buf);

	CString str;
	if (m_view.CanCopy()) {
		CRange sel = m_view.GetSelRange();
		sel.Normalize();
		INT_PTR cnt = sel.Range() + 1;
		str.Format(IDS_SELECTLEN, cnt, cnt);
	}
	m_statusbar.SetPaneText(ID_INDICATOR_LEN, str);

	m_statusbar.SetPaneText(ID_INDICATOR_INST, gbInsert ? _T("挿入") : _T("上書"));
	return 0;
}

LRESULT CMainFrame::OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = !CheckFileSave();
	return 0;
	return 0;
}

void CMainFrame::OnDestroy()
{
	CAppReg reg;
	reg.SetValue(IDS_SEC_STATUS, IDS_KEY_SETTING, m_nStat);

	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(wpm);
	if (GetWindowPlacement(&wpm))
		reg.SetBinary(IDS_SEC_STATUS, IDS_KEY_WNDPOS, &wpm, sizeof(wpm));

	m_view.DestroyWindow();
	CString str = reg.GetRootName();
	m_mru.WriteToRegistry(str);

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
}

int CMainFrame::OnCreate(LPCREATESTRUCT /*pcs*/)
{
	ModifyStyleEx(0, WS_EX_ACCEPTFILES);

	m_bModified = FALSE;
	m_csPathName = _T("");
	CAppReg reg;

	m_nStat = reg.GetValue(IDS_SEC_STATUS, IDS_KEY_SETTING, STS_TBSMALL);

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

	HWND hWndToolBar = CreateToolbar();
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE, 0, FALSE);

	// lock rebar
	m_rebar.LockBands(true);

	// statusbar
	CreateSimpleStatusBar();
	m_statusbar.SubclassWindow(m_hWndStatusBar);
	int arrParts[] = {
		ID_DEFAULT_PANE,
		ID_INDICATOR_POS,
		ID_INDICATOR_LEN,
		ID_INDICATOR_INST
	};
	m_statusbar.SetPanes(arrParts, sizeof(arrParts) / sizeof(int), false);

	// クライアント
	// WS_BORDER を指定して、CBeViewでWM_NCPAINTをキャッチし、BORDERを独自に描く
	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, 0);
	m_view.m_pFrame = this;

	UIAddToolBar(hWndToolBar);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// recent file list
	HMENU hMenu = m_menubar.GetMenu();
	HMENU hFileMenu = ::GetSubMenu(hMenu, 0);
	HMENU hMruMenu = ::GetSubMenu(hFileMenu, 3);
	m_mru.SetMenuHandle(hMruMenu);
	CString str = reg.GetRootName();
	m_mru.ReadFromRegistry(str);

	// 前回のウィンドウ位置
	CRect rc;
	WINDOWPLACEMENT wpm;
	if (reg.GetBinary(IDS_SEC_STATUS, IDS_KEY_WNDPOS, &wpm, sizeof(wpm)) == sizeof(wpm))
		rc = wpm.rcNormalPosition;
	else {
		CClientDC dc(NULL);
		HFONT hfont = dc.SelectFont(m_view.GetTextFont());
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		rc.left = 0;
		rc.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rc.top = 0;
		rc.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
		int xDelta = (GetSystemMetrics(SM_CXFULLSCREEN) - tm.tmAveCharWidth * 78 - GetSystemMetrics(SM_CXVSCROLL)) / 2;
		int yDelta = tm.tmHeight * 2;
		rc.DeflateRect(xDelta, yDelta);
		dc.SelectFont(hfont);
	}
	if (CountSiblings(m_hWnd) > 0) {
		GetWindowPlacement(&wpm);
		rc.OffsetRect(wpm.rcNormalPosition.left - rc.left, 0);
	}
	lccMapRectToProgmanPane(&rc);
	MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), FALSE);

	BOOL bOpenDocument = FALSE;
	CSimpleArray<CString> cmd;
	int cnt = ParseCommandLine(m_strCmdLine, cmd);
	if (cnt > 0) {
		bool bVolatile = false;
		CString strFile;
		for (int i = 0; i < cnt; i++) {
			LPCTSTR p;
			CString str = cmd[i];
			str.Trim(_T('"'));
			switch (str[0]) {
			case _T('/'):
				str.TrimLeft(_T("/"));
				p = str;
				switch (*p) {
				case _T('v'):
				case _T('V'):
					// 起動元から選択範囲のデータを送信してもらうよう要求する
					if (*++p == _T(':')) {
						CString tmp;
						tmp.Format(_T("0x%s"), ++p);
						int h;
						if (StrToIntEx(tmp, STIF_SUPPORT_HEX, &h)) {
							HWND hwnd = (HWND)(LONG_PTR)h;
							if (::IsWindow(hwnd)) {
								::PostMessage(hwnd, nRM_DATAREQUEST, 0, (LPARAM)m_hWnd);
								bOpenDocument = TRUE;
							}
						}
					}
					break;
				}
				break;
			default:
				if (strFile.IsEmpty())
					strFile = str;
				break;
			}
		}

		if (!strFile.IsEmpty())
			bOpenDocument = OpenFile(strFile, bVolatile);
	}

	if (!bOpenDocument)
		SetAppFrameTitle();			// （無題）を表示

	return 0;
}

HWND CMainFrame::CreateToolbar()
{
	if (m_toolbar.IsWindow())
		RemoveCustomDrawBar(m_toolbar.m_hWnd);

	HWND hWndToolBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(IDB_TOOLBAR_S), ATL_SIMPLE_TOOLBAR_PANE_STYLE, ATL_IDW_TOOLBAR);
	m_toolbar = hWndToolBar;

	AddCustomDrawBar(hWndToolBar);
	return hWndToolBar;
}

LRESULT CMainFrame::OnProcessParallelReading(WPARAM code, LPARAM param)
{
	switch (code) {
	case 0:
		// インジケータ用プログレスバー
		if (!m_progress.IsWindow()) {
			CRect rc;
			m_statusbar.GetPaneRect(0, &rc);
			rc.right = min(120, rc.right);
			rc.InflateRect(-1, -2);
			m_progress.Create(m_statusbar, rc, NULL, WS_VISIBLE | WS_CHILD);
		}
		m_progress.SetRange32(0, (int)param);
		m_progress.SetPos(0);
		break;
	case 1:
		if (m_progress.IsWindow()) {
			int pos = m_progress.GetPos() + (int)param;
			m_progress.SetPos(pos);
		}
		break;
	case 2:
		if (m_progress.IsWindow())
			m_progress.DestroyWindow();
		break;
	}
	return 0;
}

LRESULT CMainFrame::OnProcessDataRequest(WPARAM code, LPARAM param)
{
	// 要求のあったウィンドウに、現在選択している範囲のデータを送る
	HWND hwndFrom = (HWND)param;
	if (::IsWindow(hwndFrom)) {
		m_view.SendDataToWindow(hwndFrom);
	}
	return 0;
}

LRESULT CMainFrame::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	// 送られてきたデータを貼り付ける
	HWND hwndFrom = (HWND)wParam;						// 送り手側のウィンドウハンドル
	COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;		// データを格納する構造体へのポインタ
	if (pcds) {
		if (pcds->dwData == 100) {
			m_view.PasteData(pcds->lpData, pcds->cbData);
			SetAppFrameTitle();
		}
	}
	return TRUE;
}

BOOL CMainFrame::OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState)
{
	UINT idCommand = 0;
	switch (appCommand) {
		case APPCOMMAND_FIND:
		case APPCOMMAND_BROWSER_SEARCH:		idCommand = ID_EDIT_FIND;	break;
		case APPCOMMAND_CLOSE:				idCommand = ID_APP_EXIT;	break;
	}
	if (idCommand) {
		PostMessage(WM_COMMAND, idCommand, 0);
		return TRUE;
	}
	return FALSE;
}

BOOL CMainFrame::OnCommand(UINT idCtrl, UINT code, HWND hwnd)
{
	switch (idCtrl) {
		case ID_APP_EXIT:			OnFileExit(); break;
		case ID_FILE_NEW:			OnFileNew(); break;
		case ID_FILE_OPEN:		OnFileOpen(); break;
		case ID_FILE_SAVE:		OnFileSave(); break;
		case ID_FILE_SAVE_AS:	OnFileSaveAs(); break;
		case ID_APP_ABOUT:		OnAppAbout(); break;
		case ID_HELP:					OnAppHelp(); break;
		default:
			if (idCtrl >= ID_FILE_MRU_FIRST && idCtrl <= ID_FILE_MRU_LAST)
				OnFileRecent(idCtrl, code, hwnd);
			else if (m_view.IsWindow())
				m_view.SendMessage(WM_COMMAND, MAKEWPARAM(idCtrl, code), (LPARAM)hwnd);
			break;
	}
	return TRUE;
}

void CMainFrame::OnFileExit()
{
	PostMessage(WM_CLOSE);
}

void CMainFrame::OnFileNew()
{
	BootNewProcess(m_hWnd, NULL);
}

void CMainFrame::OnFileOpen()
{
	TCHAR name[MAX_PATH * 2], dir[MAX_PATH * 2];
	name[0] = 0;
	if (m_csPathName.IsEmpty())
		dir[0] = 0;
	else {
		lstrcpy(dir, m_csPathName);
		PathRemoveFileSpec(dir);
	}

	if (ShowOpenSaveFileDialog(m_hWnd, true, MAKEINTRESOURCE(IDS_FILEOPEN), dir, name, ARRAYSIZE(name))) {
		if (PathFileExists(name)) {
			if (PathIsDirectory(name)) {
				AppMessage(IDS_FILEISFOLDER, name, MB_OK | MB_ICONSTOP);
				return;
			}
		}

		PathQuoteSpaces(name);
		BootNewProcess(m_hWnd, name);
	}
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

#if 0
void CMainFrame::OnFileNewWindow()
{
	::PostThreadMessage(_Module.m_dwMainThreadID, WM_USER, 0, 0L);
}
#endif

#if 0
void CMainFrame::OnViewToolBar()
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
}
#endif

#if 0
void CMainFrame::OnViewStatusBar()
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
}
#endif

void CMainFrame::Modified(BOOL bModified)
{
	if (bModified != m_bModified) {
		m_bModified = bModified;
		SetAppFrameTitle();
	}
}

void CMainFrame::SetAppFrameTitle()
{
	CString str = m_csPathName;
	if (str.IsEmpty())
		str = sszTitleNone;

	if (m_bModified)
		str += _T("(更新)");
	str += _T(" - ");
	CString tit;
	tit.LoadString(IDS_APP_TITLE);
	str += tit;

	SetWindowText(str);
}

void CMainFrame::OnFileRecent(UINT idCtrl, UINT code, HWND hWnd)
{
	TCHAR sFile[MAX_PATH];
#if (_WTL_VER < 0x0800)
	if (m_mru.GetFromList(idCtrl, sFile)) {
#else
	if (m_mru.GetFromList(idCtrl, sFile, ARRAYSIZE(sFile))) {
#endif
		if (PathFileExists(sFile)) {
			m_mru.MoveToTop(idCtrl);
			BootNewProcess(m_hWnd, sFile);
		}
		else
			m_mru.RemoveFromList(idCtrl);
	}
}

BOOL CMainFrame::CheckFileSave()
{
	if (m_bModified) {
		int n = AppMessage(IDS_QUERY_UPDATESAVE, NULL, MB_QUESTION);
		switch (n) {
		case IDYES:
			return m_view.DoSave(m_csPathName);
		case IDNO:
			return TRUE;
		}
		return FALSE;
	}
	m_bModified = FALSE;
	SetAppFrameTitle();
	return TRUE;
}

void CMainFrame::OnDropFiles(HDROP hdrop)
{
	TCHAR file[MAX_PATH * 2];
	DragQueryFile(hdrop, 0, file, ARRAYSIZE(file));
	if (m_csPathName.CompareNoCase(file))
		OpenFile(file);
	DragFinish(hdrop);
}

BOOL CMainFrame::OpenFile(LPCTSTR pszName, BOOL bVolatile)
{
	BOOL bOK = m_view.OpenFile(pszName);
	if (bOK) {
		if (bVolatile) {
			m_csPathName.Empty();
			m_bModified = TRUE;
		}
		else {
			m_csPathName = pszName;
			m_mru.AddToList(pszName);
		}
		SetAppFrameTitle();
	}
	return bOK;
}

void CMainFrame::OnFileSave() 
{
	if (m_bModified) {
		if (m_csPathName.IsEmpty())
			OnFileSaveAs();
		else {
			if (m_view.DoSave(m_csPathName)) {
				m_bModified = FALSE;
				SetAppFrameTitle();
			}
		}
	}
}

void CMainFrame::OnFileSaveAs() 
{
	TCHAR name[MAX_PATH * 2];
	if (m_csPathName.IsEmpty())
		lstrcpy(name, sszTitleNone);
	else {
		lstrcpy(name, m_csPathName);
		if (PathFileExists(name))
			PathDecorate(name);
	}
	if (ShowOpenSaveFileDialog(m_hWnd, false, MAKEINTRESOURCE(IDS_SAVEAS), NULL, name, ARRAYSIZE(name))) {
		if (PathFileExists(name)) {
			if (PathIsDirectory(name)) {
				AppMessage(IDS_FILEISFOLDER, name, MB_OK | MB_ICONSTOP);
				return;
			}
		}

		if (m_view.DoSave(name)) {
			m_csPathName = name;
			m_bModified = FALSE;
			SetAppFrameTitle();
		}
	}
}

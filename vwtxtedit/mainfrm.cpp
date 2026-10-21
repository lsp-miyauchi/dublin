// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "mainfrm.h"
#include "jump.h"
#include "setup.h"

/////////////////////////////////////////////////////////////////////////////
//

static int indicators[] =
{
	ID_DEFAULT_PANE,
	ID_INDICATOR_CODE,
	ID_INDICATOR_RDONLY,
	ID_INDICATOR_OVR,
};

#define FRD_ID_SRCSTRING		0x480
#define FRD_ID_DSTSTRING		0x481
#define COPYDATA_PATH			0x98654411


/////////////////////////////////////////////////////////////////////////////
//

void BootNewProcess(HWND hwnd, LPCTSTR cmd)
{
#ifdef _USRDLL
	lccDllExecute(hwnd, DLL_MIL, MIL_EDIT, cmd, _T("/edit"), FALSE);
#else
	TCHAR name[MAX_PATH * 2];
	GetModuleFileName(NULL, name, ARRAYSIZE(name));
	TCHAR path[MAX_PATH * 2];
	lstrcpy(path, name);
	PathRemoveFileSpec(path);

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_UNICODE;
	sei.hwnd = hwnd;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpVerb = NULL;
	sei.lpFile = name;
	sei.lpParameters = cmd;
	sei.lpDirectory = path;
	ShellExecuteEx(&sei);
#endif
}


/////////////////////////////////////////////////////////////////////////////
//

extern "C" void WINAPI RunEdit(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow);

extern "C"
void WINAPI RunGrepW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	CFind finder;
	CGrep grep;
	grep.m_pFinder = &finder;
	lstrcpy(grep.m_path, lpCmdLine);

	CAppReg reg;
	reg.GetText(IDS_SEC_GREP, IDS_KEY_FINDFILTER, grep.m_filter, ARRAYSIZE(grep.m_filter));
	DWORD stat = reg.GetValue(IDS_SEC_GREP, IDS_KEY_STAT, FIS_SUBDIR);
	grep.m_bSubdir = (stat & FIS_SUBDIR) != 0;

	CGrepDlg dlg;
	dlg.m_pGrep = &grep;
	if (dlg.DoModal() == IDOK) {
		reg.SetText(IDS_SEC_GREP, IDS_KEY_SEARCHSTRING, finder.m_strText);
		reg.SetText(IDS_SEC_GREP, IDS_KEY_FINDFILTER, grep.m_filter);
		reg.SetText(IDS_SEC_GREP, IDS_KEY_FOLDER, grep.m_path);
		stat = finder.m_stat;
		stat |= grep.m_bSubdir ? FIS_SUBDIR : 0;
		reg.SetValue(IDS_SEC_GREP, IDS_KEY_STAT, stat);
		BootNewProcess(hwnd, _T("#grep"));
	}
}

extern "C"
void WINAPI RunGrep(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR wz[MAX_PATH * 2];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, wz, ARRAYSIZE(wz));
	RunGrepW(hwnd, hInst, wz, nCmdShow);
}

////////////////////////////////////////////////////////////////////////////

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

CMainFrame::CMainFrame()
	: m_csi(0)
	, m_lb(CRLF)
	, m_hLocalAccel(NULL)
	, m_bCountCharByUnicode(false)
	, m_bOpenSameWindow(true)
	, m_bDocumentUpdated(false)
{
	CAppReg reg;
	DWORD def = (GetSystemMetrics(SM_CYFULLSCREEN) < 800) ? STS_TBSMALL : 0;
	m_nStat = reg.GetValue(IDS_SEC_SETTING, IDS_KEY_STAT, def);
	m_bUseAutoDetect = (reg.GetValue(IDS_SEC_SETTING, IDS_KEY_AUTODETECT, TRUE) != FALSE);
	m_csi = reg.GetValue(IDS_SEC_SETTING, IDS_KEY_CHARSET, 0);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// 仮想キーコードの処理
	if (pMsg->message == WM_KEYDOWN) {
		bool ok = false;
		switch (pMsg->wParam) {
		case VK_ESCAPE:						// Escapeキー
			if (m_view.cursor()->CanCopy()) {
				m_view.cursor()->ResetSelection();
				ok = true;
			}
			break;
		default:
			break;
		}

		if (ok)
			return TRUE;
	}

	if (m_hLocalAccel && ::TranslateAccelerator(m_hWnd, m_hLocalAccel, pMsg))
		return TRUE;
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

void CMainFrame::InitialUpdate()
{
	// フレームウィンドウ作成完了後に呼び出される
	// ユーザ定義のアクセラレータ
	MakeLocalAccelTable();
	UpdateMenuAccelerator();
}

BOOL CMainFrame::OnIdle()
{
	UpdateUIAll();
	return FALSE;
}

void CMainFrame::TaskbarButtonCreated(ITaskbarList3* pTaskbarList)
{
	if (pTaskbarList != NULL) {
		HICON hIcon = LoadIcon(RESINST, MAKEINTRESOURCE(IDR_FRAME));
		if (hIcon) {
			pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);
			DestroyIcon(hIcon);
		}
	}
}

void CMainFrame::UpdateUIAll()
{
	UIEnable(ID_EDIT_REDO, m_view.document()->CanRedo());

	bool bCanUndo = m_view.document()->CanUndo();
	UIEnable(ID_FILE_SAVE, bCanUndo);
	UIEnable(ID_EDIT_UNDO, bCanUndo);
	UIEnable(ID_JUMP_LASTEDITPOS, bCanUndo);
	if (m_bDocumentUpdated != bCanUndo) {
		m_bDocumentUpdated = bCanUndo;
		UpdateTitleNames();
	}
	bool bCanPaste = m_view.cursor()->CanPaste();
	bool bROMode = m_view.cursor()->isROMode();
	bool bSelected = m_view.cursor()->isSelected();
	bool bEmpty = m_view.document()->IsEmpty();

	UIEnable(ID_EDIT_PASTE, (bCanPaste && !bROMode));

	UIEnable(ID_EDIT_COPY, bSelected);
	UIEnable(ID_EDIT_COPY_FOLLOW, bSelected);
	UIEnable(ID_EDIT_COPY_APPEND, bSelected);

	UIEnable(ID_EDIT_CUT, !bROMode && bSelected);
	UIEnable(ID_EDIT_DELETE_CHAR, !bROMode && bSelected);
	UIEnable(ID_EDIT_UPPERCASE, !bROMode && bSelected);
	UIEnable(ID_EDIT_LOWERCASE, !bROMode && bSelected);

	UIEnable(ID_EDIT_DELETE_LINE, !bROMode && !bEmpty);
	UIEnable(ID_EDIT_DELETE_LEFT, !bROMode && !bEmpty);
	UIEnable(ID_EDIT_DELETE_RIGHT, !bROMode && !bEmpty);
	UISetCheck(ID_EDIT_READONLY, bROMode);

	bool b = !m_Finder.m_strText.IsEmpty() && !m_FindReplaceDlg.IsWindow();
	UIEnable(ID_FIND_NEXT, b);
	UIEnable(ID_FIND_PREV, b);

	UIEnable(ID_FILE_OPENCLIPBOARD, bCanPaste);

	// エンコードの種類
	UISetCheck(ID_ENCODE_AUTO, m_bUseAutoDetect);

	UISetCheck(ID_LINEBREAK_CRLF, (m_lb == CRLF));
	UISetCheck(ID_LINEBREAK_CR,   (m_lb == CR));
	UISetCheck(ID_LINEBREAK_LF,   (m_lb == LF));

	//
	UISetCheck(ID_TOOLBAR_SMALL, ((m_nStat & STS_TBSMALL) != 0));

	// ツールバー
	UIUpdateToolBar();
}

int CMainFrame::OnCreate(LPCREATESTRUCT pcs)
{
	// アイコン
	m_hicRO = (HICON)LoadImage(RESINST, _MKIR(IDI_READONLY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

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
	m_menubar.LoadMultipleImages(IDB_S_TOOLBAR);
	m_menubar.LoadMultipleImages(IDB_S_APPICON);
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
	m_statusbar.SetPanes(indicators, sizeof(indicators) / sizeof(int), false);

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, 0);

	// CPadView初期設定
	VIEWCONFIG vc;
	CAppReg reg;
	if (reg.GetBinary(IDS_SEC_SETTING, IDS_KEY_CONFIG, &vc, sizeof(VIEWCONFIG)) == sizeof(VIEWCONFIG))
		m_config = vc;
	m_view.SetConfig(m_config);						// 色・フォント

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// 文字セットメニュー
	InitEncodeMenu();

	// Ｄ＆Ｄサポート
	ModifyStyleEx(0, WS_EX_ACCEPTFILES);

	// recent file list
	HMENU hMruMenu = GetSubMenuByPosition(0, 6);
	m_mru.SetMenuHandle(hMruMenu);
	CString str = reg.GetRootName();
	m_mru.ReadFromRegistry(str);

	// 前回のウィンドウ位置
	CRect rc;
	WINDOWPLACEMENT wpm;
	if (reg.GetBinary(IDS_SEC_SETTING, IDS_KEY_WNDPOS, &wpm, sizeof(wpm)) == sizeof(wpm)) {
		rc = wpm.rcNormalPosition;
		if (CountSiblings(m_hWnd) > 0) {
			GetWindowPlacement(&wpm);
			rc.OffsetRect(wpm.rcNormalPosition.left - rc.left, 0);
		}
	} else {
		CClientDC dc(NULL);
		HFONT hfont = dc.SelectFont(::CreateFontIndirect(&m_config.m_logfont));
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		rc.left = 0;
		rc.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rc.top = 0;
		rc.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
		int xDelta = (GetSystemMetrics(SM_CXFULLSCREEN) - tm.tmAveCharWidth * 140 - GetSystemMetrics(SM_CXVSCROLL)) / 2;
		int yDelta = tm.tmHeight * 2;
		rc.DeflateRect(xDelta, yDelta);
		::DeleteObject(dc.SelectFont(hfont));
	}
	lccMapRectToProgmanPane(&rc);
	MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), FALSE);

	// イベントハンドラ登録
	m_view.document()->AddHandler( this );
	m_view.cursor()->AddHandler( this );

	return 0;
}

HWND CMainFrame::CreateToolbar()
{
	if (m_toolbar.IsWindow())
		RemoveCustomDrawBar(m_toolbar.m_hWnd);

	UINT id = (m_nStat & STS_TBSMALL) ? IDB_S_TOOLBAR : IDB_M_TOOLBAR;
	HWND hWndToolBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(id), ATL_SIMPLE_TOOLBAR_PANE_STYLE, ATL_IDW_TOOLBAR);
//	lccAddToolBarDisabledImageList(hWndToolBar, RESINST, MAKEINTRESOURCE(id + 1));
	m_toolbar = hWndToolBar;
	AddCustomDrawBar(hWndToolBar);
	return hWndToolBar;
}

void CMainFrame::OnClose(BOOL& bHandled) 
{
	if (!AskToSave()) {
		bHandled = TRUE;
		return;
	}

	// ウィンドウ位置
	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(wpm);
	if (GetWindowPlacement(&wpm)) {
		CAppReg reg;
		reg.SetBinary(IDS_SEC_SETTING, IDS_KEY_WNDPOS, &wpm, sizeof(wpm));
	}
}

void CMainFrame::OnDestroy()
{
	if (m_view.IsWindow())
		m_view.DestroyWindow();

	CAppReg reg;
	reg.SetValue(IDS_SEC_SETTING, IDS_KEY_AUTODETECT, m_bUseAutoDetect);
	reg.SetValue(IDS_SEC_SETTING, IDS_KEY_CHARSET, m_csi);
	reg.SetValue(IDS_SEC_SETTING, IDS_KEY_STAT, m_nStat);

	if (m_hLocalAccel) DestroyAcceleratorTable(m_hLocalAccel);
	if (m_hicRO) DestroyIcon(m_hicRO);

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
}

void CMainFrame::OnActivate(UINT nState, HWND hWndOther, BOOL bMinimized)
{
	if (nState != WA_INACTIVE)
		m_view.SetFocus();
}

void CMainFrame::OnDropFiles(HDROP hdrop) 
{
	::SetForegroundWindow(m_hWnd);

	TCHAR file[MAX_PATH * 2];
	int s = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < s; i++) {
		DragQueryFile(hdrop, i, file, ARRAYSIZE(file));
		if (PathFileExists(file) && !PathIsDirectory(file)) {
			if (i == 0 && IsUntitled() && !m_view.document()->IsModified()) {
				// 無題で無変更だったら自分で開く
				OpenByMyself(file, CS_AUTODETECT);
			}
			else {
				PathQuoteSpaces(file);
				BootNewProcess(m_hWnd, file);
			}
		}
	}
	DragFinish(hdrop);
}

void CMainFrame::OnContextMenu(HWND hWnd, CPoint pt) 
{
	CRect rc;
	m_view.GetWindowRect(&rc);
	if (rc.PtInRect(pt)) {
		HMENU hmenu = m_menubar.GetMenu();
		if (hmenu= ::GetSubMenu(hmenu, 1))
			m_menubar.TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, pt.x, pt.y);
	}
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

void CMainFrame::InitEncodeMenu()
{
	/* --- メニューの構成初期値
		自動選択
		セパレータ
		   <--- ここに挿入していく
		セパレータ
		改行(CR+LF)
		改行(CR)
		改行(LF)
	------------------------ */

	HMENU hmenu = GetSubMenuByPosition(3, 1);
	if (hmenu) {
		int i, cnt = ::GetMenuItemCount(hmenu);
		for (i = 0; i < cnt; i++) {
			if (::GetMenuItemID(hmenu, i) == ID_ENCODE_AUTO) {
				i += 2;			// 2 separators skip;
				break;
			}
		}
		int insertAfter = i;

		cnt = m_CharSetList.GetSize();
		for (i = 0; i < cnt; i++) {
			if (m_CharSetList[i].ID != CS_AUTODETECT && m_CharSetList[i].type & CSF_LOAD)
				::InsertMenu(hmenu, insertAfter++, MF_BYPOSITION | MF_STRING, i + ID_ENCODE_TYPE1, m_CharSetList[i].longName);
		}
	}
}

void CMainFrame::UIEncodeMenu()
{
	HMENU hmenu = GetSubMenuByPosition(3, 1);
	if (hmenu) {
		int cnt = m_CharSetList.GetSize();
		for (int i = 0; i < cnt; i++) {
			UINT idCommand = i + ID_ENCODE_TYPE1;
			if (::GetMenuState(hmenu, idCommand, MF_BYCOMMAND) != 0xFFFFFFFF) {
				UINT flag = MF_BYCOMMAND | ((i == m_csi) ? MF_CHECKED : MF_UNCHECKED);
				::CheckMenuItem(hmenu, idCommand, flag);
			}
		}
	}
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
	CFileOpenSaveDialog dlg(TRUE, CS_AUTODETECT, 0, NULL, _T("*.*\0*.*\0\0"));
	if (dlg.DoModal() == IDOK) {
		CString name;
		int charset;
		dlg.GetSelectOpenFileName(name, charset);

		Open(name, charset);
	}
}

void CMainFrame::OnFileSaveAs() 
{
	SaveAs();
}

void CMainFrame::OnFileSave() 
{
	SaveIfNeeded();
}

void CMainFrame::OnFileOpenClipboard()
{
	if (m_view.cursor()->CanPaste()) {
		CString cmd = _T("#clip");
		if (IsUntitled() && !m_view.document()->IsModified() && m_view.document()->IsEmpty()) {
			// 無題で無変更だったら自分で開く
			OpenByMyself(cmd, ::GetACP());
		}
		else {
			// 他へ回す
			BootNewProcess(m_hWnd, cmd);
		}
	}
}

void CMainFrame::OnEditCut()
{
	// 切り抜き
	m_view.cursor()->Cut();
}

void CMainFrame::OnEditCutAppend()
{
	// 追加して切り抜き
	m_view.cursor()->Cut(true);
}

void CMainFrame::OnEditDelete()
{
	// 削除
	m_view.cursor()->Del();
}

void CMainFrame::OnEditDeleteLine()
{
	// 1行削除
	m_view.cursor()->DelLine();
}

void CMainFrame::OnEditDeleteLeft()
{
	// カーソル左側削除
	m_view.cursor()->DelLeft();
}

void CMainFrame::OnEditDeleteRight()
{
	// カーソル右側削除
	m_view.cursor()->DelRight();
}

void CMainFrame::OnEditCopy()
{
	// コピー
	m_view.cursor()->Copy();
	// 選択範囲をクリア
	m_view.cursor()->ResetSelection();
}

void CMainFrame::OnEditCopyFollow()
{
	// 引用付きコピー
	m_view.cursor()->Copy(true);
}

void CMainFrame::OnEditCopyAppend()
{
	// 追加コピー
	m_view.cursor()->Copy(false, true);
}

void CMainFrame::OnEditPaste()
{
	// 貼り付け
	m_view.cursor()->Paste();
}

void CMainFrame::OnEditUndo()
{
	// 元に戻す
	m_view.document()->Undo();
}

void CMainFrame::OnEditRedo()
{
	// やり直し
	m_view.document()->Redo();
}

void CMainFrame::OnEditSelectAll()
{
	// すべて選択
	m_view.cursor()->End(true, false);
	m_view.cursor()->Home(true, true);
}

void CMainFrame::OnEditUpperCase()
{
	// 大文字へ変換
	CString str;
	if (m_view.cursor()->GetSelectedStr(str) > 0) {
		str.MakeUpper();
		m_view.cursor()->Input(str, str.GetLength());
	}
}

void CMainFrame::OnEditLowerCase()
{
	// 小文字へ変換
	CString str;
	if (m_view.cursor()->GetSelectedStr(str) > 0) {
		str.MakeLower();
		m_view.cursor()->Input(str, str.GetLength());
	}
}

void CMainFrame::OnEditDuplicateLine()
{
	// 1行複写
	m_view.cursor()->DupLine();
}

void CMainFrame::OnEditReadOnly()
{
	m_view.cursor()->SetROMode(!m_view.cursor()->isROMode());
}

void CMainFrame::FindSearch(bool bForward)
{
	if (m_FindReplaceDlg.IsWindow()) return;

//@	if (m_pickup.IsWindowVisible())
//@		OnPickupClose();

	CString str;
	if (m_view.cursor()->GetSelectedStr(str) > 0) {
		int pos = 0;
		CString token = str.Tokenize(_T("\r\n"), pos);
		if (!token.IsEmpty())
			str = token;
	}
	else
		m_view.cursor()->GetCurrentWord(str, false);

	CFindStringDlg dlg;
	dlg.m_pFinder = &m_Finder;
	dlg.m_bSearchForward = bForward;
	dlg.m_strSearchFor = str.Trim();		// 前後の空白（space,tab）を取り除く
	if (dlg.DoModal() == IDOK) {
		if (m_Finder.m_stat & FIS_JRE) {
			REParseError status = m_Finder.m_jre.Parse(m_Finder.m_strText);
			if (REPARSE_ERROR_OK != status) {
				m_Finder.m_stat &= ~FIS_JRE;
			}
		}

/* @@ まだ
		if (m_Finder.m_stat & FIS_PICKUP) {
			m_Finder.m_stat &= ~FIS_PICKUP;
			m_Finder.m_stat |= FIS_NEXT;

			m_pickup.StartGrep();

			CSimpleArray<CFindItem*> array;
			HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
			int found = m_Edit.FindPickup(&m_Finder, array);
			SetCursor(hcur);

			if (found > 0) {
				for (int i = 0; i < found; i++) {
					CFindItem* pf = array[i];
					m_pickup.AddList(pf->m_linenum, pf->m_from, pf->m_to, pf->m_text);
					array[i] = NULL;
					delete pf;
				}
				array.RemoveAll();
				m_pickup.EndGrep();
				if (!m_pickup.IsWindowVisible()) {
					m_split.SetSinglePaneMode(SPLIT_PANE_NONE);
				}
			}
			else {
				MessageBeep(0);
				if (m_pickup.IsWindowVisible())
					OnPickupClose();
				else
					m_pickup.Wep();
			}
		}
		else {
@@ */
			if (m_Finder.m_stat & FIS_NEXT)
				OnFindNext();
			else
				OnFindPrev();
//@		}
	}
}

void CMainFrame::OnFindSearch() 
{
	FindSearch(true);
}

void CMainFrame::OnFindSearchBack() 
{
	FindSearch(false);
}

void CMainFrame::OnFindNext() 
{
	if (!m_FindReplaceDlg.IsWindow() && !m_Finder.m_strText.IsEmpty()) {
		if (!m_view.FindNext(&m_Finder))
			MessageBeep(0);
	}
}

void CMainFrame::OnFindPrev() 
{
	if (!m_FindReplaceDlg.IsWindow() && !m_Finder.m_strText.IsEmpty()) {
		if (!m_view.FindPrev(&m_Finder))
			MessageBeep(0);
	}
}

// 関数を抽出する正規表現
static const TCHAR* sszFuncRegExp[] = {
	_T("^[_a-zA-Z][^\\(=;:]* [_a-zA-Z][_a-zA-Z0-9]*\\([^;]*$"),
	_T("^[_a-zA-Z][^\\(=;:]* operator[^\\(;]+\\([^;]*$"),
	_T("^[_a-zA-Z][^\\(=;:]*::[^\\(;]+\\([^;]*$"),
	_T("^[_a-zA-Z][_a-zA-Z0-9]*[a-z]+[_a-zA-Z0-9]* *\\([^;]*$"),
};

void CMainFrame::OnFindFunction()
{
	CSimpleArray<CString> finder;
	int i;
	for (i = 0; i < 4; i++)
		finder.Add(sszFuncRegExp[i]);

	CSimpleMap<ULONG, CString> array;
	int cnt = m_view.FindAll(array, finder);
	if (cnt > 0) {
		CFunctionListDlg dlg;

		VPos pos = m_view.cursor()->GetCurPos();
		dlg.m_curline = pos.tl;
		dlg.m_map = &array;
		if (dlg.DoModal() == IDOK)
			JumpToLine(dlg.m_curline + 1);
	}
}

void CMainFrame::ZapMessage()
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && m_FindReplaceDlg.IsWindow()) {
		if (!m_FindReplaceDlg.IsDialogMessage(&msg)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}

void CMainFrame::OnFindReplace() 
{
	if (m_FindReplaceDlg.IsWindow()) return;

//@	if (m_pickup.IsWindowVisible())
//@		OnPickupClose();

	CString str;
	if (m_view.cursor()->GetSelectedStr(str) > 0) {
		LPTSTR p = StrPBrk(str, _T("\r\n"));
		if (p)
			*p = '\0';
		m_Finder.SetText(str);
	}
	else if (m_view.cursor()->GetCurrentWord(str, false) > 0)
		m_Finder.SetText(str);

	CReplaceStringDlg dlg;
	dlg.m_pFinder = &m_Finder;
	if (dlg.DoModal() == IDOK) {
		if (m_Finder.m_stat & FIS_FROMTOP)
			JumpToLine(1);
		m_Finder.m_stat &= ~FIS_FROMTOP;

		if (m_Finder.m_stat & FIS_JRE) {
			REParseError status = m_Finder.m_jre.Parse(m_Finder.m_strText);
			if (REPARSE_ERROR_OK != status) {
				m_Finder.m_stat &= ~FIS_JRE;
			}
		}

		// 最初のマッチ対象を選択する
		if (!m_view.FindNext(&m_Finder))
			AppMessage(IDS_FIND_NOMATCH);
		else {
			if (m_FindReplaceDlg.Create(m_hWnd)) {
				m_FindReplaceDlg.ShowWindow(SW_SHOW);
				ZapMessage();
			}
		}
	}
}

LRESULT CMainFrame::OnFindNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == REP_SKIP) {
		if (!m_view.FindNext(&m_Finder)) {
			MessageBeep(0);
			m_FindReplaceDlg.DestroyWindow();
		}
	}
	else if (wParam == REP_REPLACE) {
		m_view.cursor()->ReplaceSel(m_Finder.m_strReplace);
		if (!m_view.FindNext(&m_Finder)) {
			MessageBeep(0);
			m_FindReplaceDlg.DestroyWindow();
		}
	}
	else if (wParam == REP_REPLACEALL) {
		int cnt = 0;
		do {
			m_view.cursor()->ReplaceSel(m_Finder.m_strReplace);
			cnt++;
		} while (m_view.FindNext(&m_Finder));
		if (cnt > 0) {
			TCHAR w[64];
			wsprintf(w, _T("%d"), cnt);
			AppMessage(IDS_NUMREPLACE, w, MB_OK);
		}
	}
	return 0;
}

void CMainFrame::OnFindGrep() 
{
	CString str;
	if (m_view.cursor()->GetSelectedStr(str) > 0) {
		LPTSTR p = StrPBrk(str, _T("\r\n"));
		if (p)
			*p = 0;
		m_Finder.SetText(str);
	}
	else if (m_view.cursor()->GetCurrentWord(str) > 0)
		m_Finder.SetText(str);

	CGrep grep;
	grep.m_pFinder = &m_Finder;

	CAppReg reg;
	reg.GetText(IDS_SEC_GREP, IDS_KEY_FINDFILTER, grep.m_filter, ARRAYSIZE(grep.m_filter));
	DWORD stat = reg.GetValue(IDS_SEC_GREP, IDS_KEY_STAT, FIS_SUBDIR);
	grep.m_bSubdir = (stat & FIS_SUBDIR) != 0;
	if (!m_strFileName.IsEmpty())
		PathGetParent(m_strFileName, grep.m_path);
	else
		grep.m_path[0] = 0;

	CGrepDlg dlg;
	dlg.m_pGrep = &grep;
	if (dlg.DoModal() == IDOK) {
		reg.SetText(IDS_SEC_GREP, IDS_KEY_SEARCHSTRING, grep.m_pFinder->m_strText);
		reg.SetText(IDS_SEC_GREP, IDS_KEY_FINDFILTER, grep.m_filter);
		reg.SetText(IDS_SEC_GREP, IDS_KEY_FOLDER, grep.m_path);
		stat = grep.m_pFinder->m_stat;
		stat |= grep.m_bSubdir ? FIS_SUBDIR : 0;
		reg.SetValue(IDS_SEC_GREP, IDS_KEY_STAT, stat);

		if (IsUntitled() && !m_view.document()->IsModified() && m_view.document()->IsEmpty()) {
			// 無題で無変更だったら自分で開く
			ProcessGrep();
		}
		else
			BootNewProcess(m_hWnd, _T("#grep"));
	}
}

void CMainFrame::ProcessGrep()
{
	m_strFileName.Empty();
	m_csi = 1;
	m_lb = CRLF;

	TCHAR buf[MAX_PATH * 2];
	CGrep grep;
	CAppReg reg;
	reg.GetText(IDS_SEC_GREP, IDS_KEY_FINDFILTER, grep.m_filter, ARRAYSIZE(grep.m_filter));
	reg.GetText(IDS_SEC_GREP, IDS_KEY_FOLDER, grep.m_path, ARRAYSIZE(grep.m_path));
	reg.GetText(IDS_SEC_GREP, IDS_KEY_SEARCHSTRING, buf, ARRAYSIZE(buf));
	m_Finder.SetText(buf);
	DWORD stat = reg.GetValue(IDS_SEC_GREP, IDS_KEY_STAT, FIS_SUBDIR);
	grep.m_bSubdir = (stat & FIS_SUBDIR) != 0;
	m_Finder.m_stat = (stat & ~FIS_SUBDIR);

	// 必要ならJREコンパイルをおこなう
	if (m_Finder.m_stat & FIS_JRE) {
		REParseError status = m_Finder.m_jre.Parse(m_Finder.m_strText);
		if (REPARSE_ERROR_OK != status) {
			m_Finder.m_stat &= ~FIS_JRE;
		}
	}
	grep.m_pFinder = &m_Finder;

	// 実行開始
	CGrepProg progress;
	progress.Create(m_hWnd);
	GrepTree(&grep, grep.m_path, &progress);

	// カーソルを先頭行へ
	JumpToLine(1);

	CString str;
	str.Format(_T("\"%s\" %s"), (LPCTSTR)m_Finder.m_strText, grep.m_filter);
	ChangeTitle(str);
	UpdateTitleNames();
}

BOOL CMainFrame::GrepTree(CGrep* pGrep, LPCTSTR pszPath, CGrepProg* pgpg)
{
	TCHAR wilds[MAX_PATH * 2];
	lstrcpy(wilds, pGrep->m_filter);
	CSimpleArray<CString> argv;
	int argc = SeparateString(wilds, ';', argv);

	CEnterFolder ef(pszPath);
	if (!ef.IsFolderChanged())
		return TRUE;

	pgpg->SetFolder(pszPath);

	CFindFile ff;
	if (ff.FindFile(_T("*.*"))) {
		do {
			if (pgpg->IsCanceled()) {
				ff.Close();
				return FALSE;
			}

			TCHAR path[MAX_PATH * 2];
			PathCombine(path, pszPath, ff.m_fd.cFileName);

			if (ff.m_fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (pGrep->m_bSubdir && !ISDOT(ff.m_fd.cFileName)) {
					if (!GrepTree(pGrep, path, pgpg)) {
						ff.Close();
						return FALSE;
					}
				}
			}
			else {
				int r = 0;
				for (int i = 0; i < argc; i++) {
					r = match(ff.m_fd.cFileName, argv[i]);
					if (r == 1)
						break;
				}

				if (r == 1) {
					if (!GrepFile(path, pGrep, pgpg)) {
						ff.Close();
						return FALSE;
					}
				}
			}
		} while (ff.FindNextFile());
		ff.Close();
	}
	return TRUE;
}

BOOL CMainFrame::GrepFile(LPCTSTR pszFile, CGrep* pGrep, CGrepProg* pgpg)
{
	TextFileR tf(CS_AUTODETECT);
	if (!tf.Open(pszFile))
		return FALSE;

	pgpg->SetFile(PathFindFileName(pszFile));

	BOOL bOK = TRUE;
	TCHAR buf[1024 + 2];
	int from, to, cnt = 0, line = 1;
	CString str, strFound;
	while (tf.state()) {
		if (pgpg->IsCanceled()) {
			bOK = FALSE;
			break;
		}

		size_t n = tf.ReadLine(buf, ARRAYSIZE(buf) - 2);
		if (n > 0) {
			buf[n] = 0;
			if (pGrep->m_pFinder->Find(buf, buf + n, from, to)) {
				pgpg->Countup();
				str.Format(_T("%s(%d): %s\n"), pszFile, line, buf);
				str.Replace(_T('\t'), _T(' '));
				strFound += str;

				if (++cnt > 32) {
					m_view.document()->AppendString(strFound);
					m_view.cursor()->End(true, false);
					strFound.Empty();
					cnt = 0;
				}
			}
		}
		line++;
	}
	tf.Close();

	if (!strFound.IsEmpty()) {
		m_view.document()->AppendString(strFound);
		m_view.cursor()->End(true, false);
	}
	return bOK;
}

void CMainFrame::OnJumpLastEditPos()
{
	// 最後に編集した場所
	m_view.cursor()->LastEditPos();
}

void CMainFrame::OnJumpFileTop()
{
	// 文頭へ
	bool bShift = (GetKeyState(VK_SHIFT) < 0);
	m_view.cursor()->Home(true, bShift);
}

void CMainFrame::OnJumpFileEnd()
{
	// 文末へ
	bool bShift = (GetKeyState(VK_SHIFT) < 0);
	m_view.cursor()->End(true, bShift);
}

void CMainFrame::OnJumpTag()
{
	bool bOK = false;
	LPCTSTR line = m_view.cursor()->GetCurrentText();
	if (line && *line) {
		int len = lstrlen(line);
		TCHAR* text = new TCHAR[len + 1];
		memcpy(text, line, len * sizeof(TCHAR));
		text[len] = 0;
		TCHAR* p = StrStr(text, _T("):"));
		if (p) {
			*p-- = 0;
			TCHAR* q = StrRChr(text, p, _T('('));
			if (q) {
				*q++ = 0;
				int num = StrToInt(q);

				CFindFile ff;
				if (ff.FindFile(text)) {
					ff.Close();

					if (!m_strFileName.CompareNoCase(text)) {
						// 現在開いているファイルと一致したら？
						// 指定の行へジャンプ
						JumpToLine(num);
					}
					else {
						// 他のウィンドウで開く
						CString cmd;
						cmd.Format(_T("/l%d \"%s\""), num, text);
						BootNewProcess(m_hWnd, cmd);
					}
					bOK = true;
				}
			}
		}
		delete [] text;
	}

	if (!bOK)
		MessageBeep(0);
}

void CMainFrame::OnJumpToGoLine()
{
	CJumpDlg dlg;
	if (dlg.DoModal() == IDOK)
		JumpToLine(dlg.m_line);
}

void CMainFrame::JumpToLine(ULONG line_number)
{
	m_view.cursor()->MoveCur(0, line_number - 1, false);
}

void CMainFrame::OnPageUp()
{
	m_view.cursor()->PageUp(false);
}

void CMainFrame::OnPageDown()
{
	m_view.cursor()->PageDown(false);
}

void CMainFrame::OnPageHalfUp() 
{
	m_view.cursor()->PageHalfUp(false);
}

void CMainFrame::OnPageHalfDown() 
{
	m_view.cursor()->PageHalfDown(false);
}

static TCHAR sszCharSet[] = _T("文字コード");

void CMainFrame::OnShowCurCharCode() 
{
	// 現在位置の文字コードを表示する
	WCHAR ch = (WCHAR)m_view.cursor()->GetCurrentChar();
	if (ch) {
		TCHAR buf[MAX_PATH];
		BYTE mb[4];
		int n = WideCharToMultiByte(932, 0, &ch, 1, (char*)mb, ARRAYSIZE(mb), NULL, NULL);
		if (n < 2)
			wsprintf(buf, _T("ShifJIS : 0x%02X\nUnicode : 0x%04X"), mb[0], ch);
		else
			wsprintf(buf, _T("ShifJIS : 0x%04X\nUnicode : 0x%04X"), MAKEWORD(mb[1], mb[0]), ch);
		MessageBox(buf, sszCharSet, MB_OK);
	}
}

void CMainFrame::OnSetupProperty()
{
	CSetupDlg dlg;
	dlg.m_vc = m_config;
	if (dlg.DoModal() == IDOK) {
		m_config = dlg.m_vc;

		// コンフィグ情報
		CAppReg reg;
		reg.SetBinary(IDS_SEC_SETTING, IDS_KEY_CONFIG, &m_config, sizeof(VIEWCONFIG));

		ReloadConfig();
	}
}

void CMainFrame::OnToolbarSmall()
{
	if (m_nStat & STS_TBSMALL) m_nStat &= ~STS_TBSMALL;
	else                       m_nStat |= STS_TBSMALL;

	CAppReg reg;
	reg.SetValue(IDS_SEC_SETTING, IDS_KEY_STAT, m_nStat);

	CReBarCtrl rebar = m_hWndToolBar;
	rebar.DeleteBand(1);
	m_toolbar.DestroyWindow();
	HWND hWndToolBar = CreateToolbar();
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE, 0, FALSE);
	// ツールバー更新
	UIAddToolBar(hWndToolBar);

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void CMainFrame::OnAppEscape()
{
	// ESCキー押下に対する処理
	PostMessage(WM_COMMAND, ID_APP_EXIT);
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

void CMainFrame::OnEncodeRange(UINT nID, UINT code, HWND hwnd)
{
	if (m_FindReplaceDlg.IsWindow() || m_strFileName.IsEmpty() || !PathFileExists(m_strFileName)) {
		MessageBeep(0);
		return;
	}

	if (m_view.document()->IsModified()) {
		if (AppMessage(IDS_CHANGESDISCARD, NULL, MB_QYESNO) != IDYES)
			return;
	}

//@	WaitThreadExit();

	CString name;
	name = m_strFileName;
	m_strFileName.Empty();

	int charset = m_CharSetList[nID - ID_ENCODE_TYPE1].ID;

	OpenByMyself(name, charset, true);
}

void CMainFrame::OnLineBreakRange(UINT id, UINT code, HWND hWnd)
{
	int lb;
	switch (id) {
	case ID_LINEBREAK_CR:	lb = CR;   break;
	case ID_LINEBREAK_LF:	lb = LF;   break;
	case ID_LINEBREAK_CRLF:
	default:				lb = CRLF; break;
	}

	if (m_lb != lb)
		m_lb = lb;
}

void CMainFrame::OnOpenRecentFile(UINT nID, UINT code, HWND hwnd)
{
	TCHAR sFile[MAX_PATH * 2];
	int charset = 0;
#if (_WTL_VER < 0x0800)
	if (m_mru.GetFromList(nID, sFile)) {
#else
	if (m_mru.GetFromList(nID, sFile, ARRAYSIZE(sFile), charset)) {
#endif
		if (!PathFileExists(sFile)) {
			// ファイルは存在しないので、レジストリからも削除して掃除
			MessageBeep(0);
			m_mru.RemoveFromList(nID);
			CAppReg reg;
			CString str = reg.GetRootName();
			m_mru.WriteToRegistry(str);
		}
		else {
			if (IsUntitled() && !m_view.document()->IsModified() && m_view.document()->IsEmpty()) {
				// 無題で無変更だったら自分で開く
				OpenByMyself(sFile, charset);
			}
			else {
				// 他へ回す
				CString str; str.Format(_T("-c%d %s"), charset, sFile);
				BootNewProcess(m_hWnd, str);
			}
		}
	}
}

void CMainFrame::OnCursorEventMode(int modeType, bool mode)
{
	CString str;
	switch (modeType) {
	case CEM_READONLYMODE:
		m_statusbar.SetPaneIcon(ID_INDICATOR_RDONLY, mode ? m_hicRO : NULL);
		UpdateTitleNames();
		break;
	case CEM_INSERTMODE:
		str.LoadString(mode ? ID_INDICATOR_INS : ID_INDICATOR_OVR);
		m_statusbar.SetPaneText(ID_INDICATOR_OVR, str);
		break;
	}
}

void CMainFrame::OnCursorEventMove( const DPos& c, const DPos& s )
{
	static int busy_cnt = 0;

	CPadDocument* doc = m_view.document();
	if (doc->isBusy() && ((++busy_cnt) & 0xff))
		return;

	ULONG cad = c.ad;
	if (!m_bCountCharByUnicode) {
		// ShiftJIS風のByte数カウント
		const WCHAR* cu = doc->tl(c.tl);
//@		const ULONG tab = m_config.vConfig().tabstep;
		const ULONG tab = 4;
		cad = 0;
		for (ULONG i = 0; i < c.ad; i++) {
			if (cu[i] == L'\t')
				cad = (cad / tab + 1) * tab;
			else if (cu[i] < 0x80 || 0xff60 <= cu[i] && cu[i] <= 0xff9f)
				cad = cad + 1;
			else
				cad = cad + 2;
		}
	}

	CString str;
	str.Format(_T("[%d: %d]"), c.tl + 1, cad + 1);
	if (c != s) {
		ULONG sad = s.ad;
		if (!m_bCountCharByUnicode) {
			// ShiftJIS風のByte数カウント
			const WCHAR* su = doc->tl(s.tl);
			sad = 0;
			for (ULONG i = 0; i < s.ad; ++i)
				sad += (su[i] < 0x80 || 0xff60 <= su[i] && su[i] <= 0xff9f ? 1 : 2);
		}
		str.AppendFormat(_T(" - [%d: %d]"), s.tl + 1, sad + 1);
	}
	m_statusbar.SetPaneText(ID_DEFAULT_PANE, str);
}

void CMainFrame::OnDocumentEventDirtyFlagChanged( bool )
{
	UpdateTitleNames();
}

void CMainFrame::ChangeTitle(LPCTSTR pszTitle)
{
	if (pszTitle && *pszTitle)
		m_strTitle = pszTitle;
	else {
		if (m_strFileName.IsEmpty())
			m_strTitle.LoadString(IDS_UNTITLED);
		else
			m_strTitle = PathFindFileName(m_strFileName);
	}
}

void CMainFrame::UpdateTitleNames()
{
	CString str = m_strTitle.GetString();
	if (!str.IsEmpty()) {
		CString s;
		if (m_bDocumentUpdated) {
			// 更新済み
			s.LoadString(ID_INDICATOR_UPD);
			str.AppendFormat(_T("(%s)"), (LPCTSTR)s);
		}
		if (m_view.cursor()->isROMode()) {
			// 参照中
			s.LoadString(ID_INDICATOR_RDONLY);
			str.AppendFormat(_T("(%s)"), (LPCTSTR)s);
		}
		s.LoadString(IDR_FRAME);
		str.AppendFormat(_T(" - %s"), (LPCTSTR)s);
	}
	else
		str.LoadString(IDR_FRAME);
	SetWindowText(str);

	// 文字セット種別
	m_statusbar.SetPaneText(ID_INDICATOR_CODE, m_CharSetList[m_csi].longName);
	UIEncodeMenu();

}

//-------------------------------------------------------------------------
// 設定更新処理
//-------------------------------------------------------------------------

#include "keyword_c.h"
#include "keyword_cs.h"
#include "keyword_vb.h"
#include "keyword_html.h"
#include "keyword_xml.h"
#include "keyword_java.h"
#include "keyword_perl.h"
#include "keyword_rc.h"
#include "keyword_actionscript.h"
#include "keyword_javascript.h"
#include "keyword_typescript.h"
#include "keyword_php.h"
#include "keyword_vue.h"

struct EXTENTIONTYPE {
	const WCHAR* kwd;
	LPCTSTR ext;
};

static EXTENTIONTYPE stExts[12] = {
	kwd_C,						_T("c;cc;cpp;cxx;h;hpp"),
	kwd_CS,						_T("cs"),
	kwd_HTML,					_T("htm;html;shtml;css"),
	kwd_XML,					_T("xml;mxml;vcproj;vcxproj;filters"),
	kwd_JAVA,					_T("java;kt"),
	kwd_JavaScript,		_T("js;jsp"),
	kwd_TypeScript,   _T("ts"),
	kwd_vue,          _T("vue"),
	kwd_ActionScript, _T("as"),
	kwd_PHP,          _T("php"),
	kwd_VB,           _T("vb"),
  kwd_RC,           _T("rc;rc2"),
};

void CMainFrame::ReloadConfig(bool /*noSetDocType*/)
{
	// コンフィグ
	m_view.SetConfig(m_config);

	// 拡張子マップ作成
	CSimpleMap<CString, CString> map;
  for (const auto& et : stExts) {
    CString tmp = et.ext;
    tmp.MakeLower();
    int pos = 0;
    CString token = tmp.Tokenize(_T(";"), pos);
    while (token != _T("")) {
      map.Add(token, et.kwd);
      token = tmp.Tokenize(_T(";"), pos);
    }
  }

	CString strKeywords;
	int len = 0;
	if (PathFileExists(m_strFileName)) {
		LPCTSTR ext = PathFindExtension(m_strFileName);
		if (*ext == '.') {
			CString str = ext + 1;
			str.MakeLower();
			int pos = map.FindKey(str);
			if (pos >= 0) {
				strKeywords = map.GetValueAt(pos);
				len = strKeywords.GetLength();
			}
		}
	}
	m_view.document()->SetKeyword(strKeywords, len);
}

bool CMainFrame::SaveAs()
{
	CString name;
	if (!m_strFileName.IsEmpty())
		name = m_strFileName;
	else
		name.LoadString(IDS_UNTITLED);

	int cs;
	int lb;
	CFileOpenSaveDialog dlg(FALSE, m_CharSetList[m_csi].ID, m_lb, name);
	if (dlg.DoModal() == IDOK) {
		dlg.GetSelectSaveFileName(name, cs, lb);
		if (PathFileExists(name)) {
			if (AppMessage(IDS_QUERY_OVERWRITE, name, MB_QYESNO) != IDYES)
				return false;
		}
		m_strFileName = name;
		m_csi = m_CharSetList.FindCharSetIndex(cs);
		m_lb = lb;

		if (!Save())
			return false;

		ChangeTitle();
	}
	return true;
}

bool CMainFrame::SaveIfNeeded()
{
	bool bWasUntitled = IsUntitled();

	// [Save][Cancel] ファイル名未定ならダイアログ表示
	if (IsUntitled()) {
		return SaveAs();
	}

	if (Save()) {
		if (bWasUntitled)
			ReloadConfig();			// 文書タイプに応じて表示を更新
		return true;
	}
	return false;
}

bool CMainFrame::AskToSave()
{
	// 変更されていたら、
	// [Yes][No][Cancel] 保存するかどうか尋ねる。
	// 保存するなら
	// [Save][Cancel]    ファイル名未定ならダイアログ表示

	if (m_view.document()->IsModified()) {
		int answer = AppMessage(IDS_QUERYSAVE, m_strTitle, MB_QUESTION);
		if (answer == IDYES) return SaveIfNeeded();
		if (answer == IDCANCEL) return false;
	}
	return true;
}

bool CMainFrame::Save()
{
	TextFileW tf(m_CharSetList[m_csi].ID, m_lb);

	if (tf.Open(m_strFileName)) {
		// 無事ファイルに保存できた場合
		m_view.document()->SaveFile(&tf);
		UpdateTitleNames();

		// 履歴に追加してレジストリを更新
		m_mru.AddToList(m_strFileName, m_CharSetList[m_csi].ID);
		CAppReg reg;
		CString str = reg.GetRootName();
		m_mru.WriteToRegistry(str);
		// todo 同時起動しているエディタへ[最近使ったファイル]が更新されたことを通知

		return true;
	}

	// Error!
	AppMessage(IDS_SAVEERROR, PathFindFileName(m_strFileName), MB_STOP);
	return false;
}

bool CMainFrame::Open(LPCTSTR pszFile, int cs)
{
	if (IsUntitled() && !m_view.document()->IsModified()) {
		// 無題で無変更だったら自分で開く
		return OpenByMyself(pszFile, cs);
	}
	else {
		// 同じ窓で開くモードならそうする
		if (m_bOpenSameWindow)
			return ( AskToSave() ? OpenByMyself(pszFile, cs) : true );

		// そうでなければ他へ回す
		CString cmd;
		cmd.Format(_T("-c%d \"%s\""), cs, pszFile);

		BootNewProcess(m_hWnd, cmd);
		return true;
	}
}

bool CMainFrame::OpenByMyself(LPCTSTR pszFile, int cs, bool bNeedReConf)
{
	if (pszFile && !StrCmpNI(pszFile, _T("#clip"), 5)) {
		m_strFileName.Empty();
		m_csi = 1;
		m_lb = CRLF;
		// クリップボードをチェック
		// 貼り付け
		CClipboard clp(m_hWnd, true);
		if (clp.isOpened()) {
			CString str;
			int len = clp.GetUnicodeText(str);
			if (len > 0)
				m_view.document()->AppendString(str);
		}
		m_view.cursor()->Home(true, false);

		ChangeTitle(_T("(clipboard)"));
		UpdateTitleNames();
		return true;
	}

	// ファイルを開けなかったらそこでおしまい。
	TextFileR tf(cs);
	if (!tf.Open(pszFile)) {
		// ERROR!
		AppMessage(IDS_OPENFAILED, PathFindFileName(pszFile), MB_STOP);
		return false;
	}

	// 自分内部の管理情報を更新
	if (*pszFile == _T('\\') || *(pszFile + 1) == _T(':'))
		m_strFileName = pszFile;
	else {
		TCHAR buf[MAX_PATH * 2];
		GetCurrentDirectory(ARRAYSIZE(buf), buf);
		PathAppend(buf, pszFile);
		m_strFileName = buf;
	}

	if (tf.size()) {
		m_csi = m_CharSetList.FindCharSetIndex(tf.codepage());
		if (tf.nolb_found()) {
//@			m_lb = m_config.GetNewFileLineBreak();
			m_lb = CRLF;
		}
		else
			m_lb = tf.linebreak();
	}
	else {
		// 空ファイルの場合は新規作成と同じ扱い
//@		m_csi = m_config.GetNewFileCharSetIndex();
		m_csi = 1;
//@		m_lb = m_config.GetNewFileLineBreak();
		m_lb = CRLF;
	}

	// 文書タイプに応じて表示を更新
	if (bNeedReConf)
		ReloadConfig();

	// 開く
	m_view.document()->ClearAll();
	m_view.document()->OpenFile(&tf);

	// タイトルバー更新
	ChangeTitle();
	UpdateTitleNames();

	// 履歴に追加してレジストリを更新
	m_mru.AddToList(m_strFileName, m_CharSetList[m_csi].ID);
	CAppReg reg;
	CString str = reg.GetRootName();
	m_mru.WriteToRegistry(str);

	// todo 同時起動しているエディタへ[最近使ったファイル]が更新されたことを通知

	return true;
}

bool CMainFrame::StartEdit(LPCTSTR pszFile, int cs, int line_number, bool rom)
{
	bool bDefaultStartup = false;
	if (!pszFile || *pszFile == 0)
		bDefaultStartup = true;
	else {
		if (!StrCmpNI(pszFile, _T("#clip"), 5)) {
			OnFileOpenClipboard();
		}
		// ファイルから検索をチェック
		else if (!StrCmpNI(pszFile, _T("#grep"), 5)) {
			ProcessGrep();
			return true;
		}
		else if (!PathFileExists(pszFile)) {
			bDefaultStartup = true;
		}
		else {
			if (cs == CS_AUTODETECT) {
				// 履歴から見つける
				int charset;
				if (m_mru.FindFromList(pszFile, charset) > 0)
					cs = charset;
			}
			if (!OpenByMyself(pszFile, cs)) {
				bDefaultStartup = true;
			}
		}
	}

	if (bDefaultStartup) {
		// ファイルを開か(け)なかった場合
		ReloadConfig(true);

		ChangeTitle();
		UpdateTitleNames();
	}
	else if (line_number != -1) {
		// 指定の行へジャンプ
		JumpToLine(line_number);
	}

	m_view.cursor()->SetROMode(rom);
	return true;
}

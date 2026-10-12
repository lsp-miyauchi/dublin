// MainFrm.cpp : implmentation of the CMainFrame class
//

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "pidl.h"
#include "MainFrm.h"
#include "selff.h"
#include "drgdrp.h"
#include "setup.h"

BOOL QueryFolder(HWND hwnd, LPTSTR pszPath, BOOL bCopy);

void __cdecl qsort_ex(void *base, size_t num, size_t width,
    int (__cdecl *comp)(const void*, const void*, const void*), const void* param);


///////////////////////////////////////////////////////////////////////
// ヘッダカラム定義
//

typedef struct tagLISTHEAD {
	TCHAR title[16];
	int fmt;
	int cxy;
} LISTHEAD;

#define MAXHEADITEM		6
static LISTHEAD lhEntry[MAXHEADITEM] = {
	{ _T(""), LVCFMT_LEFT,  10 },		// 名前
	{ _T(""), LVCFMT_LEFT,  10 },		// 場所
	{ _T(""), LVCFMT_RIGHT, 10 },		// サイズ
	{ _T(""), LVCFMT_LEFT,  10 },		// 更新日時
	{ _T(""), LVCFMT_LEFT,  10 },		// 種類
	{ _T(""), LVCFMT_RIGHT, 10 }		// 属性
};

#define STYLE_BAR			(CBRS_FLYBY | CBRS_TOOLTIPS | CBRS_GRIPPER)
#define DTEX_SINGLE			(DT_VCENTER | \
							 DT_NOPREFIX | \
							 DT_END_ELLIPSIS | \
							 DT_SINGLELINE)
#define DTPATH_SINGLE		(DT_VCENTER | \
							 DT_NOPREFIX | \
							 DT_PATH_ELLIPSIS | \
							 DT_SINGLELINE)

// FileVisorへ通知
enum {
	CPD_JUMP_PATHA = 1,
	CPD_JUMP_PATHW,
	CPD_JUMP_IDLIST,
	CPD_JUMP_FILEA,
	CPD_JUMP_FILEW,
	CPD_EXEC_IDLIST = 11,
	CPD_EXEC_FILEA,
	CPD_EXEC_FILEW,
	CPD_VIEW_IDLIST = 21,
	CPD_VIEW_FILEA,
	CPD_VIEW_FILEW,
};

#define CPD_COMMAND_MASK		0x0000ffff
#define CPD_NEW_TAB				0x80000000

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
	UINT id;
	for (id = IDM_SORT_NONE; id <= IDM_SORT_PATH; id++)
		UISetCheck(id, m_nSortType == (id - IDM_SORT_NONE));
	for (id = IDM_SORT_UP; id <= IDM_SORT_DOWN; id++)
		UISetCheck(id, m_nSortSeq == (id - IDM_SORT_UP));

	BOOL bEnable = (m_list.GetSelectedCount() > 0);
	UIEnable(IDM_EDIT_CUT, bEnable);
	UIEnable(IDM_EDIT_COPY, bEnable);

	UIUpdateToolBar();
	ShowSelInfo();
}

void CMainFrame::OnDestroy()
{
	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(wpm);
	if (GetWindowPlacement(&wpm)) {
		CAppReg reg;
		reg.SetBinary(IDS_SEC_SETTING, IDS_KEY_WNDPOS, &wpm, sizeof(wpm));
	}
}

void CMainFrame::Construct()
{
	CAppReg reg;
	m_nMaxName = 0;
	m_nMaxExt = 0;
	m_nMaxSize = 0;
	m_nMaxDate = 0;
	m_nMaxAttr = 0;
	m_dlTotal = 0;
	m_dlSelect = 0;
	m_nListStat = reg.GetValue(IDS_SEC_SETTING, IDS_KEY_LISTSTAT, LTS_ALIGNEXT | LTS_REPCHECKER);
	m_nSortType = reg.GetValue(IDS_SEC_SETTING, IDS_KEY_SORTTYPE, SORT_PATH);
	m_nSortSeq = reg.GetValue(IDS_SEC_SETTING, IDS_KEY_SORTSEQ, SORT_UP);

	COLORREF colors[16];
	int size = lccRegGetBinary(_T("CommonSetting"), _T("Colors"), _T("ColorMap"), (BYTE*)colors, sizeof(colors));
	if ((size / sizeof(COLORREF)) == gColors.GetSize())
		gColors.Set(colors, gColors.GetSize());
}

void CMainFrame::CreateClient()
{
	m_hWndClient = m_list.Create(m_hWnd, rcDefault, NULL, WS_VISIBLE | WS_CHILD | LVS_OWNERDRAWFIXED | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS, 0, IDW_LIST);

	HINSTANCE hInst = RESINST;

	// カラムヘッダ
	LoadString(hInst, IDS_COLUMN_NAME, lhEntry[0].title, 16);
	LoadString(hInst, IDS_COLUMN_PATH, lhEntry[1].title, 16);
	LoadString(hInst, IDS_COLUMN_SIZE, lhEntry[2].title, 16);
	LoadString(hInst, IDS_COLUMN_DATE, lhEntry[3].title, 16);
	LoadString(hInst, IDS_COLUMN_TYPE, lhEntry[4].title, 16);
	LoadString(hInst, IDS_COLUMN_ATTR, lhEntry[5].title, 16);

	// イメージリストのアサイン
	HIMAGELIST dumm;
	Shell_GetImageLists(&dumm, &m_hImage);
	ImageList_GetIconSize(m_hImage, (int*)&m_sizeIcon.cx, (int*)&m_sizeIcon.cy);

	m_list.SetImageList(m_hImage, LVSIL_SMALL);
	m_list.SetBkColor(gColors[colBack]);

	// 拡張情報設定
	DWORD exStyle = LVS_EX_DOUBLEBUFFER;
	if (m_nListStat & LTS_INFOTIP) {
		// チップ情報
		exStyle |= LVS_EX_INFOTIP;
	}
	exStyle |= m_list.GetExtendedListViewStyle();
	m_list.SetExtendedListViewStyle(exStyle);

	// カラム挿入とヘッダカスタマイズ
	LVCOLUMN lvc;
	for (int i = 0; i < MAXHEADITEM; i++) {
		lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvc.fmt = lhEntry[i].fmt;
		lvc.cx = lhEntry[i].cxy;;
		lvc.pszText = lhEntry[i].title;
		lvc.iSubItem = i;
		m_list.InsertColumn(i, &lvc);
		m_list.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
	}

	if (m_nSortType != SORT_NONE) {
		int index = -1;
		if (m_nSortType == SORT_NAME)		index = 0;
		else if (m_nSortType == SORT_PATH)	index = 1;
		else if (m_nSortType == SORT_SIZE)	index = 2;
		else if (m_nSortType == SORT_TIME)	index = 3;
		else if (m_nSortType == SORT_EXT)	index = 4;
		else if (m_nSortType == SORT_ATTR)	index = 4;

		if (index != -1)
			ShowBoundary(index, TRUE);
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT /*pcs*/)
{
	Construct();

	HMENU hmenu = GetMenu();

	// リバー
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
#ifdef CUSTOM_REBER_USING
	m_rebar.SubclassWindow(m_hWndToolBar);
#else
	m_rebar = m_hWndToolBar;
#endif

	// メニューバー
	m_menubar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_menubar.AttachMenu(hmenu);
	m_menubar.SetImageMaskColor(RGB(255, 0, 255));
	m_menubar.LoadMultipleImages(IDB_S_TOOLBAR);
	m_menubar.LoadMultipleImages(IDB_S_APPICON);
	SetMenu(NULL);
#ifdef CUSTOM_REBER_USING
	m_rebar.SetMenuBar(m_menubar);						// メニューバーを明示的に登録
#endif
	AddSimpleReBarBand(m_menubar, NULL, FALSE, 0, FALSE);

	// lock rebar
	m_rebar.LockBands(true);

	// ツールバー
	HWND hWndToolBar = CreateToolbar();
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE, 0, FALSE);

	// statusbar
	CreateSimpleStatusBar();
	m_statusbar.SubclassWindow(m_hWndStatusBar);
	int arrParts[] = 
	{
		ID_DEFAULT_PANE,
		ID_INDICATOR_SELECT,
		ID_INDICATOR_TOTAL
	};
	m_statusbar.SetPanes(arrParts, sizeof(arrParts) / sizeof(int), false);

	// クライアント
	CreateClient();

	UIAddToolBar(hWndToolBar);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// 前回のウィンドウ位置
	WINDOWPLACEMENT wpm;
	CAppReg reg;
	if (reg.GetBinary(IDS_SEC_SETTING, IDS_KEY_WNDPOS, &wpm, sizeof(wpm)) == sizeof(wpm)) {
		CRect rc = wpm.rcNormalPosition;
		lccMapRectToProgmanPane(&rc);
		MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), FALSE);
	}

	ImmAssociateContext(m_hWnd, NULL);
	ImmAssociateContext(m_list, NULL);

	CString str;
	str.LoadString(IDS_APP_TITLE);
	SetWindowText(str);

	PostMessage(WM_COMMAND, IDM_EXEC_FIND);
	return 0;
}

HWND CMainFrame::CreateToolbar()
{
	if (m_toolbar.IsWindow())
		RemoveCustomDrawBar(m_toolbar.m_hWnd);

	HWND hWndToolBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(IDB_S_TOOLBAR), ATL_SIMPLE_TOOLBAR_PANE_STYLE, ATL_IDW_TOOLBAR);
	m_toolbar = hWndToolBar;

	// ドロップダウンスタイル
	TBBUTTONINFO tbi;
	m_toolbar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	tbi.cbSize = sizeof(tbi);
	tbi.dwMask = TBIF_STYLE;
	tbi.fsStyle = BTNS_WHOLEDROPDOWN;
	m_toolbar.SetButtonInfo(IDM_SORT_RANGE, &tbi);

	AddCustomDrawBar(hWndToolBar);
	return hWndToolBar;
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

void CMainFrame::TrackBarMenu(int n1stPos, int n2ndPos, POINT pt)
{
	HMENU hmenu = GetSubMenuByPosition(n1stPos, n2ndPos);
	if (hmenu)
		m_menubar.TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y);
}

int CMainFrame::DoToolBarDropDown(TBNOTIFY* ptbn)
{
	CPoint pt(ptbn->rcButton.left, ptbn->rcButton.bottom - 1);
	::ClientToScreen(ptbn->hdr.hwndFrom, &pt);

	if (ptbn->iItem == IDM_SORT_RANGE)
		TrackBarMenu(2, 0, pt);

	return 0;
}

BOOL CMainFrame::OnAppCommand(HWND hwnd, int appCommand, UINT uDevice, UINT uKeyState)
{
	UINT idCommand = 0;
	switch (appCommand) {
	case APPCOMMAND_FIND:
	case APPCOMMAND_BROWSER_SEARCH:		idCommand = IDM_EXEC_FIND;	break;
	case APPCOMMAND_CLOSE:				idCommand = ID_APP_EXIT;	break;
	}
	if (idCommand) {
		PostMessage(WM_COMMAND, idCommand, 0);
		return TRUE;
	}
	return FALSE;
}

void CMainFrame::OnExecFind()
{
	TCHAR path[MAX_PATH * 2];
	if (!m_strCmdLine.IsEmpty() && PathIsDirectory(m_strCmdLine))
		lstrcpy(path, m_strCmdLine);
	else
		GetCurrentDirectory(ARRAYSIZE(path), path);

	CSelFFDlg setup;
	CSearchParam sp;
	sp.strStartPath = m_strCmdLine;
	setup.m_psp = &sp;

	CDirWalker walk;
	while (1) {
		if (setup.DoModal(m_hWnd) != IDOK) {
			PostMessage(WM_CLOSE);
			return;
		}

		walk.Initialize(m_hWnd, &sp);
		walk.Start();
		SetCurrentDirectory(path);

		// 何かしら検索できれば終了
		if (walk.GetItemCount() > 0) break;

		if (walk.IsCanceled() || AppMessage(NULL, IDS_FILEFIND_NODATA, NULL, MB_QYESNO) == IDNO) {
			PostMessage(WM_CLOSE);
			return;
		}
		setup.m_fSearchRetry = true;
	}

	CString tit;
	tit.LoadString(IDS_APP_TITLE);

	CString str;
	str.Format(_T("%s - %s"), walk.m_psp->strSearchMask.GetString(), tit.GetString());
	SetWindowText(str);

	walk.MoveItemTo(m_stack, 0);
	ListupAgain();

	if (!IsWindowVisible())
		ShowWindow(SW_SHOWNORMAL);
	else if (IsIconic())
		ShowWindow(SW_RESTORE);
	if (::GetForegroundWindow() != m_hWnd)
		::SetForegroundWindow(m_hWnd);
}

void CMainFrame::ShowBoundary(int index, BOOL bShow)
{
	CHeaderCtrl hdr = m_list.GetHeader();

	HDITEM hd;
	hd.mask = HDI_FORMAT;
	hdr.GetItem(index, &hd);
	if (!bShow) {
		hd.mask = HDI_FORMAT;
		hd.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
	}
	else {
		hd.mask = HDI_FORMAT;
		hd.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
		hd.fmt |= (m_nSortSeq == SORT_UP) ? HDF_SORTUP : HDF_SORTDOWN;
	}
	hdr.SetItem(index, &hd);
}

void CMainFrame::ListupAgain()
{
	m_dlTotal = 0;
	m_dlSelect = 0;

	int cnt = (int)m_stack.GetCount();
	for (int i = 0; i < cnt; i++) {
		CFileInfo* pf = m_stack[i];
		m_dlTotal += pf->m_dlSize;
	}
	m_list.SetRedraw(FALSE);
	m_list.DeleteAllItems();
	SortListup();

	CalcFields();
	m_list.SetRedraw(TRUE);
	ShowTotalInfo();
}

void CMainFrame::SortListup()
{
	int cnt = (int)m_stack.GetCount();
	if (cnt > 0) {
		if (cnt > 1)
			qsort_ex(m_stack.GetData(), cnt, sizeof(CFileInfo*),
					CompareFileInfo, ULongToPtr(MAKELONG(m_nSortType, m_nSortSeq)));

		register int i;
		register CFileInfo* pf;
		for (i = 0; i < cnt; i++) {
			pf = m_stack[i];
			m_list.InsertItem(i, pf->m_strName, 0);
		}
	}
}

void CMainFrame::CalcFields()
{
	CClientDC dc(m_hWnd);
	HFONT hfont = m_list.GetFont();
	hfont = (HFONT)::SelectObject(dc, hfont);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	m_nMaxName = tm.tmAveCharWidth * 8;
	m_nMaxExt = 0;
	m_nMaxType = tm.tmAveCharWidth * 16;
	m_nMaxDate = 0;
	m_nMaxAttr = tm.tmAveCharWidth * 8;
	m_nMaxPath = tm.tmAveCharWidth * 8;
	DWORDLONG dlSize = 999999;

	int nDnt;
	if (m_nListStat & LTS_LOWERCASE)
		nDnt = DNT_LOWER;
	else
		nDnt = DNT_NORMAL;

	TCHAR buf[MAX_PATH];
	CSize csz;
	CFileInfo *pf;
	int i;
	for (i = 0; i < (int)m_stack.GetCount(); i++) {
		pf = m_stack[i];
		lstrcpy(buf, pf->m_strName);
		if (nDnt == DNT_LOWER)
			CharLower(buf);
		int len = lstrlen(buf);

		// ファイル名長
		dc.GetTextExtent(buf, len, &csz);
		int dx = csz.cx;
		if (!pf->IsFolder() && m_nListStat & LTS_ALIGNEXT) {
			LPTSTR p = StrRChr(buf, NULL, '.');
			if (p) {
				dc.GetTextExtent(p, len - (int)(p - buf), &csz);
				dx -= csz.cx;
				m_nMaxExt = max(m_nMaxExt, csz.cx);
			}
		}
		m_nMaxName = max(m_nMaxName, dx);

		// タイプ名
		pf->GetTypeName();

		// パス名
		lstrcpy(buf, pf->m_strPath);
		if (m_nListStat & LTS_LOWERCASE)
			CharLower(buf);
		dc.GetTextExtent(buf, lstrlen(buf), &csz);
		m_nMaxPath = max(m_nMaxPath, csz.cx);

		// サイズ長
		dlSize = max(pf->m_dlSize, dlSize);
	}

	// サイズ長
	num_form(dlSize, buf);
	dc.GetTextExtent(buf, lstrlen(buf), &csz);
	m_nMaxSize = csz.cx;
	if (m_nListStat & LTS_FMTSIZE) {
		FormatFileSize(dlSize, buf, TRUE);
		dc.GetTextExtent(buf, lstrlen(buf), &csz);
		m_nMaxSize = max(m_nMaxSize, csz.cx);
	}

	// 日付時刻長
	LPTSTR pw = (m_nListStat & LTS_FMTSIZE) ? gszDateForm + 2 : gszDateForm;
	dc.GetTextExtent(pw, lstrlen(pw), &csz);
	m_nMaxDate = csz.cx;

	// パス長補正
	m_nMaxPath = min(m_nMaxPath, tm.tmAveCharWidth * 64);

	::SelectObject(dc, hfont);

	int cols[7];
	cols[0] = m_sizeIcon.cx + 4 + m_nMaxName + m_nMaxExt + 8;
	cols[1] = m_nMaxPath + 8;
	cols[2] = m_nMaxSize + 8;
	cols[3] = m_nMaxDate + 16;
	cols[4] = m_nMaxType + 8;
	cols[5] = m_nMaxAttr + 8;

	for (i = 0; i < MAXHEADITEM; i++)
		m_list.SetColumnWidth(i, cols[i]);
}

static COLORREF calc_mixed_color(COLORREF clrA, COLORREF clrB)
{
	// (11a + 1b)/12
	int r = (11 * GetRValue(clrA) + 1 * GetRValue(clrB)) / 12;
	int g = (11 * GetGValue(clrA) + 1 * GetGValue(clrB)) / 12;
	int b = (11 * GetBValue(clrA) + 1 * GetBValue(clrB)) / 12;
	return RGB(r, g, b);
}

void CMainFrame::OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi)
{
	CDCHandle dc = pdi->hDC;
	CFileInfo *pf = m_stack[pdi->itemID];

	COLORREF clrBack = gColors[colBack];
	if (m_nListStat & LTS_REPCHECKER && (pdi->itemID & 1) == 0)
		clrBack = calc_mixed_color(clrBack, gColors[colSelBack]);

	COLORREF clr;
	COLORREF clrFore = dc.SetTextColor(gColors[colNorm]);
	clrBack = dc.SetBkColor(clrBack);
	CRect rc(&pdi->rcItem), rcw;
	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		rcw = rc;
		rcw.right = rcw.left + m_sizeIcon.cx + 2;
		dc.ExtTextOut(0, 0, ETO_OPAQUE, &rcw, NULL, 0, NULL);

		COLORREF clrBlend = CLR_DEFAULT;
		UINT style = ILD_TRANSPARENT;
		if (pdi->itemState & ODS_SELECTED) {
			if      (pf->IsAttrib(FAT_SYSTEM))	clr = gColors[colSystem];
			else if (pf->IsAttrib(FAT_HIDDEN))	clr = gColors[colHidden];
			else if (pf->IsAttrib(FAT_RDONLY))	clr = gColors[colRdonly];
			else								clr = gColors[colSelBack];
			dc.SetTextColor(gColors[colSelect]);
			dc.SetBkColor(clr);

			clrBlend = clr;
			style |= ILD_SELECTED;
		}
		else {
			if      (pf->IsAttrib(FAT_SYSTEM))	clr = gColors[colSystem];
			else if (pf->IsAttrib(FAT_HIDDEN))	clr = gColors[colHidden];
			else if (pf->IsAttrib(FAT_RDONLY))	clr = gColors[colRdonly];
			else								clr = gColors[colNorm];
			dc.SetTextColor(clr);
		}
		rcw = rc;
		rcw.left += m_sizeIcon.cx + 2;
		dc.ExtTextOut(rcw.left, rcw.top, ETO_OPAQUE, &rcw, NULL, 0, NULL);

		// アイコン
		rcw = rc;
		int col = m_list.GetColumnWidth(0);
		rcw.right = rcw.left + col - 4;

		if (pf->m_iImage == -1) {
			TCHAR name[MAX_PATH * 2];
			pf->GetRealName(name);
			SHFILEINFO sfi;
			if (!SHGetFileInfo(name, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
				SHGetFileInfo(name, pf->m_dwAttr, &sfi, sizeof(sfi),
								SHGFI_USEFILEATTRIBUTES |
								SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
			pf->m_iImage = sfi.iIcon;
		}
		ImageList_DrawEx(m_hImage, pf->m_iImage, dc,
					rc.left + 2, (rc.top + rc.bottom - m_sizeIcon.cy) / 2,
					0, 0, CLR_NONE, clrBlend, style);
		rcw.left += m_sizeIcon.cx + 4;

		// ファイル名
		TCHAR buf[MAX_PATH * 2], ext[MAX_PATH];
		if (!pf->IsFolder() && m_nListStat & LTS_ALIGNEXT) {
			lstrcpy(buf, pf->m_strName);
			LPTSTR p = PathFindExtension(buf);
			if (*p == '.') {
				lstrcpy(ext, p);
				*p = 0;
			}

			CRect w = rcw;
			w.right = min(w.right, w.left + m_nMaxName);
			dc.DrawText(buf, -1, &w, DTEX_SINGLE | DT_LEFT);
			w = rcw;
			w.left += m_nMaxName;
			dc.DrawText(ext, -1, &w, DTEX_SINGLE | DT_LEFT);
		}
		else
			dc.DrawText(pf->m_strName, -1, &rcw, DTEX_SINGLE | DT_LEFT);

		// パス名
		m_list.GetSubItemRect(pdi->itemID, 1, LVIR_BOUNDS, rcw);
		rcw.InflateRect(-4, 0);
		lstrcpy(buf, pf->m_strPath);
		if (m_nListStat & LTS_LOWERCASE)
			CharLower(buf);
		dc.DrawText(buf, -1, &rcw, DTPATH_SINGLE | DT_LEFT);

		// サイズ
		m_list.GetSubItemRect(pdi->itemID, 2, LVIR_BOUNDS, rcw);
		rcw.InflateRect(-4, 0);
		FormatFileSize(pf->m_dlSize, buf, m_nListStat & LTS_FMTSIZE);
		dc.DrawText(buf, -1, &rcw, DTEX_SINGLE | DT_RIGHT);

		// 日付・時刻
		m_list.GetSubItemRect(pdi->itemID, 3, LVIR_BOUNDS, rcw);
		rcw.InflateRect(-4, 0);
		FormatFileTime(&pf->m_ftLastWriteTime, buf, m_nListStat & LTS_FMTDATE, m_nListStat & LTS_RELATIVEDATE);
		dc.DrawText(buf, -1, &rcw, DTEX_SINGLE | DT_LEFT);

		// タイプ名
		m_list.GetSubItemRect(pdi->itemID, 4, LVIR_BOUNDS, rcw);
		rcw.InflateRect(-4, 0);
		dc.DrawText(pf->GetTypeName(), -1, &rcw, DTEX_SINGLE | DT_LEFT);

		// 属性
		m_list.GetSubItemRect(pdi->itemID, 5, LVIR_BOUNDS, rcw);
		rcw.InflateRect(-4, 0);
		lccFileAttrToStr(pf->m_dwAttr, buf);
		if (m_nListStat & LTS_LOWERCASE)
			CharLower(buf);
		dc.DrawText(buf, -1, &rcw, DTEX_SINGLE | DT_RIGHT);
	}
	if (pdi->itemState & ODS_FOCUS) {
		rcw = rc;
		rcw.left += 18;
		dc.DrawFocusRect(&rcw);
	}
	dc.SetTextColor(clrFore);
	dc.SetBkColor(clrBack);
}

void CMainFrame::DoSort(int type, int seq)
{
	int index = -1;
	if (m_nSortType != type) {
		if (m_nSortType == SORT_NAME)		index = 0;
		else if (m_nSortType == SORT_PATH)	index = 1;
		else if (m_nSortType == SORT_SIZE)	index = 2;
		else if (m_nSortType == SORT_TIME)	index = 3;
		else if (m_nSortType == SORT_TYPE)	index = 4;
		else if (m_nSortType == SORT_ATTR)	index = 5;

		if (index >= 0)
			ShowBoundary(index, FALSE);
	}
	m_nSortType = type;
	m_nSortSeq = seq;

	index = -1;
	if (type == SORT_NAME)			index = 0;
	else if (type == SORT_PATH)		index = 1;
	else if (type == SORT_SIZE)		index = 2;
	else if (type == SORT_TIME)		index = 3;
	else if (type == SORT_TYPE)		index = 4;
	else if (type == SORT_ATTR)		index = 5;

	if (index != -1)
		ShowBoundary(index, TRUE);

	ListupAgain();
	m_list.Invalidate(FALSE);
}

void CMainFrame::OnNotifyColumnClick(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMLISTVIEW* pnmlv = (NMLISTVIEW*)pNmhdr;
	int nType = -1;
	if (pnmlv->iSubItem == 0)		nType = SORT_NAME;
	else if (pnmlv->iSubItem == 1)	nType = SORT_PATH;
	else if (pnmlv->iSubItem == 2)	nType = SORT_SIZE;
	else if (pnmlv->iSubItem == 3)	nType = SORT_TIME;
	else if (pnmlv->iSubItem == 4)	nType = SORT_TYPE;
	else if (pnmlv->iSubItem == 5)	nType = SORT_ATTR;

	if (nType != -1) {
		int nSeq = (m_nSortType == nType) ? m_nSortSeq ^ TRUE : m_nSortSeq;
		DoSort(nType, nSeq);
	}
}

void CMainFrame::OnNotifyRClick(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMLISTVIEW* pnmlv = (NMLISTVIEW*)pNmhdr;
	if (pnmlv->iItem >= 0) {
		if (m_list.GetSelectedCount() > 1) {
			int index = -1;
			while (1) {
				index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
				if (index < 0)
					break;
				if (index != pnmlv->iItem)
					m_list.SetItemState(index, 0, LVIS_SELECTED);
			}
		}

		CFileInfo* pf = m_stack[pnmlv->iItem];
		TCHAR name[MAX_PATH * 2];
		pf->GetRealName(name);
		CPoint pt = pnmlv->ptAction;
		m_list.ClientToScreen(&pt);
		lccContextMenuByFile(m_hWnd, name, &pt);
	}
}

void CMainFrame::OnNotifyDblClick(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMLISTVIEW* pnmlv = (NMLISTVIEW*)pNmhdr;
	if (pnmlv->iItem >= 0) PostMessage(WM_COMMAND, IDM_FILE_OPEN);
}

void CMainFrame::OnNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMLVKEYDOWN* pkd = (NMLVKEYDOWN*)pNmhdr;
	int vkeyCtrl = GetKeyState(VK_CONTROL) & 0x8000;

	*pResult = FALSE;
	UINT idCommand = 0;
	switch (pkd->wVKey) {
	case VK_ESCAPE:		idCommand = ID_APP_EXIT; break;
	case VK_RETURN:		idCommand = IDM_TOOL_VIEWER; break;
	case 'X':
		if (vkeyCtrl)	idCommand = IDM_EDIT_CUT;
		break;
	case VK_DELETE:
		if (vkeyCtrl)	idCommand = IDM_EDIT_CUT;
		else 			idCommand = IDM_FILE_DELETE;
		break;
	case 'C':
	case VK_INSERT:
		if (vkeyCtrl)	idCommand = IDM_EDIT_COPY;
		break;
	case 'A':
		if (vkeyCtrl)	idCommand = IDM_FILE_SELALL;
		break;
	case 'F':
		if (vkeyCtrl)	idCommand = IDM_EXEC_FIND;
		break;
	case 'J':
		if (vkeyCtrl)	idCommand = IDM_FILE_JUMP;
		break;
	default:
		break;
	}
	if (idCommand) {
		PostMessage(WM_COMMAND, idCommand);
		*pResult = TRUE;
	}
}

void CMainFrame::OnNotifyGetInfoTip(NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	NMLVGETINFOTIP* pgit = (NMLVGETINFOTIP*)pNmhdr;
	CFileInfo* pf = m_stack[pgit->iItem];
	if (pf) {
		TCHAR name[MAX_PATH];
		pf->GetRealName(name);
		LPITEMIDLIST pidl = idl_GetFromPath(name);
		if (pidl) {
			TCHAR buf[MAX_URL_LENGTH];
			if (idl_GetInfoTip(m_hWnd, pidl, buf, ARRAYSIZE(buf)) && buf[0]) {
				if (pgit->dwFlags)
					lstrcpy(pgit->pszText, pf->m_strName);
				lstrcat(pgit->pszText, _T("\n"));
				lstrcat(pgit->pszText, buf);
			}
			idl_Free(pidl);
		}
	}
}

void CMainFrame::OnNotifyBeginDrag(NMHDR* /*pNmhdr*/, LRESULT* /*pResult*/)
{
	HDROP hdrop = MakeDropFiles();
	if (hdrop) {
		DWORD dwEffect = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;
		DragStart(hdrop, &dwEffect);
		DragFinish(hdrop);

		UpdateSelItems();
	}
}

void CMainFrame::OnSortRange(UINT id, UINT code, HWND hwnd)
{
	if (IDM_SORT_NONE <= id && id <= IDM_SORT_PATH) {
		int type = id - IDM_SORT_NONE;
		DoSort((m_nSortType != type) ? type : SORT_NONE, m_nSortSeq);
	}
	else if (IDM_SORT_UP <= id && id <= IDM_SORT_DOWN) {
		int seq = id - IDM_SORT_UP;
		DoSort(m_nSortType, seq);
	}
}

void CMainFrame::ShowSelInfo()
{
	m_dlSelect = 0;
	int cnt = m_list.GetSelectedCount();
	if (cnt > 0) {
		CFileInfo* pf;
		int index = -1;
		while (1) {
			index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
			if (index < 0) break;

			pf = m_stack[index];
			m_dlSelect += pf->m_dlSize;
		}
	}

	TCHAR buf[MAX_PATH], tot[MAX_PATH];
	StrFormatByteSize64(m_dlSelect, tot, ARRAYSIZE(tot));

	CString fmt;
	fmt.LoadString(IDS_INDICATOR_SELECT);
	fmt += _T("(%s)");
	wsprintf(buf, fmt, cnt, tot);

	m_statusbar.SetPaneText(ID_INDICATOR_SELECT, buf);
}

void CMainFrame::ShowTotalInfo()
{
	TCHAR buf[MAX_PATH], tot[MAX_PATH];
	StrFormatByteSize64(m_dlTotal, tot, ARRAYSIZE(tot));

	CString fmt;
	fmt.LoadString(IDS_INDICATOR_TOTAL);
	fmt += _T("(%s)");
	wsprintf(buf, fmt, (int)m_stack.GetCount(), tot);
	m_statusbar.SetPaneText(ID_INDICATOR_TOTAL, buf);
}

int CMainFrame::GetSelItems(int** ppSel)
{
	*ppSel = NULL;
	int cnt = m_list.GetSelectedCount();
	if (cnt > 0) {
		int* ps = (int*)malloc(sizeof(int) * cnt);
		CFileInfo pf;
		int i = 0, index = -1;
		while (1) {
			index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
			if (index < 0)
				break;
			ps[i++] = index;
		}
		*ppSel = ps;
	}
	return cnt;
}

HDROP CMainFrame::MakeDropFiles()
{
	HDROP hdrop = NULL;
	int cnt = m_list.GetSelectedCount();
	if (cnt > 0) {
		CDropFiles df;
		int index = -1;
		while (1) {
			index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
			if (index < 0)
				break;

			CFileInfo* pf = m_stack[index];
			TCHAR name[MAX_PATH * 2];
			pf->GetRealName(name);
			if (PathFileExists(name))
				df.DragAppendFile(name);
		}

		if (df.GetCount() > 0)
			hdrop = df.Detach();
	}
	return hdrop;
}

IDataObject* CMainFrame::MakeDataObject()
{
	HRESULT (WINAPI* _func)(LPITEMIDLIST pidlFolder, UINT cidl, LPITEMIDLIST*_arr, IDataObject** ppdtobj);
	(FARPROC&)_func = GetProcAddress(GetModuleHandle(_T("shell32.dll")), "CIDLData_CreateFromIDArray");
	if (!_func)
		return NULL;

	IDataObject* pdo = NULL;
	int cnt = m_list.GetSelectedCount();
	if (cnt > 0) {
		LPITEMIDLIST pidl;
		LPITEMIDLIST* arr = new LPITEMIDLIST[cnt];
		cnt = 0;
		int index = -1;
		while (1) {
			index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
			if (index < 0)
				break;

			CFileInfo* pf = m_stack[index];
			TCHAR name[MAX_PATH * 2];
			pf->GetRealName(name);
			if (PathFileExists(name)) {
				pidl = idl_GetFromPath(name);
				if (pidl) {
					arr[cnt] = pidl;
					cnt++;
				}
			}
		}

		if (cnt > 0) {
			if (_func(NULL, cnt, arr, &pdo) != S_OK)
				pdo = NULL;

			for (int i = 0; i < cnt; i++)
				idl_Free(arr[i]);
		}
		delete [] arr;
	}
	return pdo;
}

void CMainFrame::ValidateFileList()
{
	CFileInfo* pf;
	TCHAR buf[MAX_PATH * 2];
	int nDel = 0;
	SIZE_T i = 0;
	while (i < m_stack.GetCount()) {
		pf = m_stack[i];
		pf->GetRealName(buf);
		if (!PathFileExists(buf)) {
			m_stack.RemoveAt(i);
			delete pf;
			nDel++;
		}
		else
			i++;
	}
	if (nDel > 0)
		ListupAgain();
}

void CMainFrame::ListToStack()
{
	CFileArray array;
	array.Append(m_stack);
	m_stack.RemoveAll();

	CFileInfo* pf;
	for (int i = 0; i < (int)array.GetCount(); i++) {
		pf = array[i];
		if (pf) m_stack.Add(pf);
	}
	array.RemoveAll();

	ListupAgain();
	m_list.Invalidate(FALSE);
}

void CMainFrame::UpdateSelItems()
{
	// 移動されたファイルはリストから消去
	int* sel;
	int cnt = GetSelItems(&sel);
	int del = 0;
	for (int i = 0; i < cnt; i++) {
		CFileInfo* pf = m_stack[sel[i]];
		TCHAR name[MAX_PATH * 2];
		pf->GetRealName(name);
		if (!PathFileExists(name)) {
			m_stack[sel[i]] = NULL;
			delete pf;
			del++;
		}
	}
	free(sel);
	if (del > 0) ListToStack();
}

void CMainFrame::DoCopyMove(BOOL bCopy)
{
	HDROP hdrop = MakeDropFiles();
	if (hdrop) {
		TCHAR dst[MAX_PATH * 2];
		dst[0] = '\0';
		if (QueryFolder(m_hWnd, dst, bCopy)) {
			// 安全なフォルダへの移動
			TCHAR windir[MAX_PATH];
			GetWindowsDirectory(windir, ARRAYSIZE(windir));
			CEnterFolder ef;
			ef.Enter(windir);

			DROPFILEOPERATION dfo;
			dfo.cbSize = sizeof(dfo);
			dfo.hwnd = m_hWnd;
			dfo.hDrop = hdrop;
			dfo.dwEffect = (bCopy ? DROPEFFECT_COPY : DROPEFFECT_MOVE);
			dfo.pszDropPath = dst;
			dfo.bAsAdmin = FALSE;
			dfo.bAsyncOperation = FALSE;
			dfo.bSaveFolderHierarchy = TRUE;
			fopDropFileOperation(&dfo);

			if (!bCopy)
				UpdateSelItems();
		}
		DragFinish(hdrop);
	}
}

void CMainFrame::OnFileSelAll()
{
	int cnt = m_list.GetItemCount();
	if (cnt < 1) return;

	m_list.SetRedraw(FALSE);

	int sel = m_list.GetSelectedCount();
	UINT newState = (cnt == sel) ? 0 : LVIS_SELECTED;
	for (int i = 0; i < cnt; i++)
		m_list.SetItemState(i, newState, LVIS_SELECTED);

	m_list.SetRedraw(TRUE);
	m_list.Invalidate(FALSE);
}

void CMainFrame::OnFileSelRev()
{
	int cnt = m_list.GetItemCount();
	if (cnt < 1) return;

	m_list.SetRedraw(FALSE);
	for (int i = 0; i < cnt; i++) {
		if (m_list.GetItemState(i, LVIS_SELECTED))
			m_list.SetItemState(i, 0, LVIS_SELECTED);
		else
			m_list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}
	m_list.SetRedraw(TRUE);
	m_list.Invalidate(FALSE);
}

void CMainFrame::OnFileJump()
{
	if (m_list.GetItemCount() > 0) {
		int sel = m_list.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
		if (sel >= 0) {
			TCHAR buf[MAX_PATH * 2];
			CFileInfo* pf = m_stack[sel];
			pf->GetRealName(buf);

			HWND hwnd = FindWindow(APP_FRAMECLASS, NULL);
			if (!hwnd) {
				TCHAR exe[MAX_PATH];
				if (!lccRegGetAppPath(APP_EXENAME, exe, ARRAYSIZE(exe))) return;

				SHELLEXECUTEINFO sei;
				memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.fMask = SEE_MASK_UNICODE;
				sei.hwnd = m_hWnd;
				sei.nShow = SW_SHOWNORMAL;
				sei.lpFile = exe;
				sei.lpParameters = buf;
				sei.lpDirectory = pf->m_strPath;
				ShellExecuteEx(&sei);
				return;
			}
			else {
				COPYDATASTRUCT cds;
				cds.dwData = CPD_JUMP_FILEW;
				cds.cbData = (lstrlen(buf) + 1) * sizeof(TCHAR);
				cds.lpData = buf;
				::SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM)&cds);
				SetForegroundWindow(hwnd);
			}
		}
	}
}

void CMainFrame::OnFileDelete()
{
	HDROP hdrop = MakeDropFiles();
	if (hdrop) {
		CDropFiles df;
		df.Attach(hdrop);

		if (GetKeyState(VK_SHIFT) < 0) {
			LPTSTR lp = df.MakeFileNames();
			if (lp) {
				fopTrashFile(m_hWnd, lp, FOF_ALLOWUNDO);
				free(lp);
			}
		}
		else
			fopDeleteFile(m_hWnd, df);
		ValidateFileList();
	}
}

void CMainFrame::OnFileClear()
{
	int* pSel;
	int cnt = GetSelItems(&pSel);
	if (cnt < 1) return;

	CFileInfo* pf;
	int nDel = 0;
	for (int i = 0; i < cnt; i++) {
		pf = m_stack[pSel[i]];
		m_stack[pSel[i]] = NULL;
		delete pf;
		nDel++;
	}
	free(pSel);

	if (nDel > 0) ListToStack();
}

void CMainFrame::OnFileView()
{
	int sel = m_list.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
	if (sel < 0) return;

	CFileInfo* pf = m_stack[sel];
	ViewFile(m_hWnd, pf->m_strName, pf->m_strPath);
}

void CMainFrame::OnFileBinDump()
{
	int sel = m_list.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
	if (sel < 0) return;

	CFileInfo* pf = m_stack[sel];
	if (pf) {
		TCHAR buf[MAX_PATH * 2];
		pf->GetRealName(buf);
		RunBinDump(m_hWnd, buf);
	}
}

void CMainFrame::OnFileInfo()
{
	HDROP hdrop = MakeDropFiles();
	if (hdrop) {
		fopShowFileInfo(m_hWnd, hdrop);
		DragFinish(hdrop);
	}
}

void CMainFrame::OnFileOpen()
{
	int sel = m_list.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
	if (sel < 0) return;

	CFileInfo* pf = m_stack[sel];
	if (pf->IsFolder()) {
		CDropFiles df;
		df.DragAppendFile(pf);
		fopShowFileInfo(m_hWnd, df);
	}
	else
		myShellExecute(m_hWnd, pf->m_strName, pf->m_strPath);
}

void CMainFrame::OnFileRename()
{
	int cnt = m_list.GetSelectedCount();
	if (cnt > 0) {
		int index = -1;
		while (1) {
			index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
			if (index < 0)
				break;

			CFileInfo* pf = m_stack[index];
			TCHAR name[MAX_PATH * 2];
			pf->GetRealName(name);
			if (PathFileExists(name)) {
				if (!fopRename(m_hWnd, name))
					break;

				pf->m_strName = PathFindFileName(name);
				m_list.RedrawItems(index, index);
			}
		}
	}
}

void CMainFrame::DoObjectCopy(bool bCut)
{
	HDROP hdrop = MakeDropFiles();
	if (hdrop) {
		CDataObject* pdo = new CDataObject(hdrop, bCut ? DROPEFFECT_MOVE : DROPEFFECT_COPY);
		OleFlushClipboard();
		OleSetClipboard(pdo);
		pdo->Release();
	}
}

void CMainFrame::OnEditCopy()
{
	DoObjectCopy(false);
}

void CMainFrame::OnEditCut()
{
	DoObjectCopy(true);
}

void CMainFrame::OnClipCopyRange(UINT id, UINT code, HWND hwnd)
{
	HDROP hdrop = MakeDropFiles();
	if (hdrop) {
		CDropFiles df;
		df.Attach(hdrop);
		switch (id) {
		case IDM_COPY_NAMEONLY:
			df.CopyFullPathToClipboard(m_hWnd, FALSE);
			break;

		case IDM_COPY_DETAIL:
			df.CopyToClipboard(m_hWnd);
			break;

		case IDM_COPY_FULLNAME:
			df.CopyFullPathToClipboard(m_hWnd);
			break;

		case IDM_COPY_SHORTNAME:
			df.CopyShortPathToClipboard(m_hWnd);
			break;
		}
	}
}

void CMainFrame::OnToolSetup()
{
	CSetupDlg dlg;
	dlg.m_status = m_nListStat;

	if (dlg.DoModal() == IDOK) {
		CAppReg reg;
		bool bRedraw = false;
		if (m_nListStat != dlg.m_status) {
			int changed_flags = (m_nListStat ^ dlg.m_status);
			m_nListStat = dlg.m_status;
			reg.SetValue(IDS_SEC_SETTING, IDS_KEY_LISTSTAT, m_nListStat);

			if (changed_flags & (LTS_FMTDATE | LTS_FMTSIZE | LTS_RELATIVEDATE | LTS_LOWERCASE))
				CalcFields();

			if (changed_flags & LTS_INFOTIP) {
				DWORD style = m_list.GetExtendedListViewStyle();
				if (m_nListStat & LTS_INFOTIP)
					style |= LVS_EX_INFOTIP;
				else
					style &= ~LVS_EX_INFOTIP;
				m_list.SetExtendedListViewStyle(style);
			}

			bRedraw = true;
		}

		if (bRedraw)
			m_list.Invalidate();
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

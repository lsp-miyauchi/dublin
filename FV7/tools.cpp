// tools.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"
#include "mainfrm.h"
#include "wildcard.h"
#include "rebarsetting.h"

/////////////////////////////////////////////////////////////////////////////
// LOCAL MACRO DEFINITION
//

#define SEPCODE					((BYTE)-1)
#define MARGIN_HORZ				6
#define MARGIN_VERT				1

//@ #define STYLE_BAR		(CBRS_FLYBY | CBRS_TOOLTIPS | CBRS_GRIPPER | CBRS_TOP)
#define STYLE_BAR					(CBRS_FLYBY | CBRS_TOOLTIPS | CBRS_TOP)



/////////////////////////////////////////////////////////////////////////////
// LOCAL DATA DEFINITION
//

static int siTbOperation[] = {
	IDM_FILE_COPY,
	IDM_FILE_MOVE,
	IDM_FILE_MKDIR,
	ID_SEPARATOR,
	IDM_FILE_DELETE,
	ID_SEPARATOR,
	IDM_FILE_RENAME,
//	IDM_FILE_SERIESNAME,
//	IDM_FILE_REPNAMES,
//	IDM_FILE_CHGCAPS,
	ID_SEPARATOR,
	IDM_FILE_ATTRIB,
	ID_SEPARATOR,
    IDM_FILE_PROPERTY,
//	IDM_VIEW_FILESIZE,
};

static int siTbTools[] = {
	IDM_DIR_SEARCH,
	IDM_DIR_SYNC,
	ID_SEPARATOR,
	IDM_TOOL_EDITOR,
	IDM_TOOL_VIEWER,
	IDM_TOOL_BDUMP,
//	IDM_TOOL_GREP,
	ID_SEPARATOR,
  IDM_TOOL_SHELL,
  ID_SEPARATOR,
	IDM_FILE_PACK,
	IDM_FILE_UNPACK,
};

// ツールバーに登録可能なボタンIDとインデクスを保持するマップ
static int* siToolbarGroupEntries;
static CSimpleMap<int, int> siTbMap;

/////////////////////////////////////////////////////////////////////////////
//
//

// structure of RT_TOOLBAR resource
struct TOOLBARDATA {
	WORD wVersion;		// version # should be 1
	WORD wWidth;		// width of one bitmap
	WORD wHeight;		// height of one bitmap
	WORD wItemCount;	// number of items
	WORD items[1];		// array of command IDs, actual size is wItemCount
};

static BOOL InitToolbarButtonInfo()
{
	// ツールバーの情報のロード
	HINSTANCE hInst = RESINST;
	HRSRC hr = FindResource(hInst, _MKIR(IDB_TOOLBAR), RT_TOOLBAR);
	if (!hr)
		return FALSE;
	TOOLBARDATA* ptbd = (TOOLBARDATA*)LoadResource(hInst, hr);
	if (!ptbd) return FALSE;

	CSimpleMap<int, int> map;
	for (int i = 0; i < ptbd->wItemCount; i++) {
		// 有効なコマンドIDだけを登録する
		if (ptbd->items[i] > 0 && ptbd->items[i] != IDM_INVALID) {
			siTbMap.Add(i, ptbd->items[i]);
		}
	}
	return TRUE;
}

typedef struct tagTFDROPDOWNINFO {
	UINT id;
	BYTE style;
	UINT label;
} TFDROPDOWNINFO;


static TFDROPDOWNINFO tfDropDown[] = {
	IDM_FILE_SHELLNEW,		BTNS_WHOLEDROPDOWN,	0,
//	IDM_DIR_PREV,			BTNS_DROPDOWN,		0,
//	IDM_DIR_NEXT,			BTNS_DROPDOWN,		0,
	IDM_DIR_QUICKACCESS,	BTNS_WHOLEDROPDOWN,	0,
	IDM_VIEW_HOTKEYMENU,	BTNS_WHOLEDROPDOWN,	0,
	IDM_VIEW_WILDCARD,		BTNS_DROPDOWN,		0,
	IDM_FILE_MASKSELECT,	BTNS_DROPDOWN,		0,
	IDM_SORT,				BTNS_WHOLEDROPDOWN,	IDS_SORT
};

void CMainFrame::SetDropDownButton(HWND hBar)
{
	CToolBarCtrl bar = hBar;
	bar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);
	TBBUTTONINFO tbi;
	tbi.cbSize = sizeof(tbi);
	for (int i = 0; i < sizeof(tfDropDown) / sizeof(TFDROPDOWNINFO); i++) {
		int index = bar.CommandToIndex(tfDropDown[i].id);
		if (index >= 0) {
			if (tfDropDown[i].style == 0) {
				bar.HideButton(tfDropDown[i].id);
			} else {
				tbi.dwMask = TBIF_STYLE;
				tbi.fsStyle = tfDropDown[i].style;
				// ボタンラベル
				if (tfDropDown[i].label) {
					CString s;
					s.LoadString(tfDropDown[i].label);
					int pos = s.Find(_T('\n'));
					if (pos > 0) {
						s = s.Mid(pos);
					}
					tbi.dwMask |= TBIF_TEXT;
					tbi.pszText = s.GetBuffer();
					tbi.fsStyle |= BTNS_SHOWTEXT | TBSTYLE_LIST;
				}
				bar.SetButtonInfo(tfDropDown[i].id, &tbi);
			}
		}
	}
}

HWND CMainFrame::CreateDefaultTooBar(int* parrButtons, int numButtons, UINT idBar, UINT idResource)
{
	bool q = ((gnBarStat & BTS_LARGEICON) != 0);
	HWND hBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(q ? IDB_M_TOOLBAR : IDB_TOOLBAR), TBSTYLE_FLAT, idBar);
	if (!hBar) {
		CannotCreateWindow(m_hWnd, _T("Default ToolBar"));
		return NULL;
	}

	CToolBarCtrl* pBar = new CToolBarCtrl(hBar);
	m_arrToolbar.Add(pBar);

	CString str;
	str.LoadString(idResource);
	pBar->SetWindowText(str);

	BYTE* arr = new BYTE[numButtons];
	BYTE* pa = arr;
	for (int i = 0; i < numButtons; i++) {
		int idCommand = *parrButtons++;
		if (idCommand == ID_SEPARATOR)
			*pa++ = SEPCODE;
		else {
			int index = siTbMap.FindVal(idCommand);
			if (index >= 0)
				*pa++ = (BYTE)siTbMap.GetKeyAt(index);
		}
	}
	BuildButtons(hBar, arr, numButtons);
	delete[] arr;

	AddCustomDrawBar(hBar);
	return hBar;
}

CSimpleArray<CToolBarCtrl*>& CMainFrame::GetToolBarArray()
{
	return m_arrToolbar;
}

BOOL CMainFrame::CreateToolBar()
{
	InitToolbarButtonInfo();

	bool q = ((gnBarStat & BTS_LARGEICON) != 0);
	HWND hBar;
	CRect rc(MARGIN_HORZ, MARGIN_VERT, MARGIN_HORZ, MARGIN_VERT);
	CToolBarCtrl* pBar;
	UINT id = IDW_TBFIRST;
	int cnt = GetRegValue(IDS_SEC_TOOLBAR, IDS_KEY_COUNT, -1);
	if (cnt > 0) {
		for (int i = 0; i < cnt; i++) {
			hBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(q ? IDB_M_TOOLBAR : IDB_TOOLBAR), TBSTYLE_FLAT, id++);
			if (!hBar)
				continue;

			pBar = new CToolBarCtrl(hBar);
			m_arrToolbar.Add(pBar);
			RestoreToolBarState(hBar, i);
			AddCustomDrawBar(hBar);
		}
	} else if (cnt < 0) {
		CreateDefaultTooBar(siTbOperation, ARRAYSIZE(siTbOperation), id++, IDW_OPERATIONBAR);
		CreateDefaultTooBar(siTbTools, ARRAYSIZE(siTbTools), id++, IDW_TOOLBAR);
	}
	return TRUE;
}

CToolBarCtrl* CMainFrame::CreateEmptyToolBar(LPCTSTR pszTitle)
{
	int num = m_arrToolbar.GetSize();
	if (num >= IDW_TBLAST - IDW_TBFIRST) return NULL;

	int nIDNew;
	for (nIDNew = 0; nIDNew < IDW_TBLAST - IDW_TBFIRST; nIDNew++) {
		BOOL bMatch = FALSE;
		for (int pos = 0; pos < num; pos++) {
			CToolBarCtrl* pbw = m_arrToolbar[pos];
			if (nIDNew + IDW_TBFIRST == pbw->GetDlgCtrlID()) {
				bMatch = TRUE;
				break;
			}
		}
		if (!bMatch)
			break;
	}
	nIDNew += IDW_TBFIRST;

	bool q = ((gnBarStat & BTS_LARGEICON) != 0);
	HWND hBar = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(q ? IDB_M_TOOLBAR : IDB_TOOLBAR), TBSTYLE_FLAT, nIDNew);
	if (!hBar)
		return NULL;

	CToolBarCtrl* pBar = new CToolBarCtrl(hBar);
	m_arrToolbar.Add(pBar);

	pBar->SetWindowText(pszTitle);

	// ボタン削除
	num = pBar->GetButtonCount();
	while (num-- > 0) pBar->DeleteButton(0);

	// いったんダミーのボタンを登録
	TBBUTTON tbb;
	memset(&tbb, 0, sizeof(tbb));
	tbb.idCommand = 0;
	tbb.iBitmap = -1;
	tbb.fsState = TBSTATE_ENABLED;
	tbb.fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
	pBar->AddButtons(1, &tbb);
	AddSimpleReBarBandCtrl(m_rebar.m_hWnd, hBar, nIDNew, NULL, TRUE, 0, FALSE);

	// そしてもう一度削除
	num = pBar->GetButtonCount();
	while (num-- > 0) pBar->DeleteButton(0);

	AddCustomDrawBar(hBar);
	return pBar;
}

BOOL CMainFrame::DeleteToolBar(int index)
{
	if (index < 0 || index >= m_arrToolbar.GetSize())
		return FALSE;

	CToolBarCtrl* pBar = m_arrToolbar[index];
	if (pBar->IsWindowVisible())
		ShowBar(*pBar, FALSE);

	m_arrToolbar.RemoveAt(index);
	RemoveCustomDrawBar(*pBar);

	CRebarSettings crs(m_rebar);
	index = crs.LookupIndex(*pBar);
	if (index >= 0)
		m_rebar.DeleteBand(index);
	delete pBar;

	// ツールバーのIDを連番にする
	for (int i = 0; i < m_arrToolbar.GetSize(); i++)
		m_arrToolbar[i]->SetDlgCtrlID(i + IDW_TBFIRST);

	SaveRebarSettings();
	return TRUE;
}

BOOL CMainFrame::CreatePropBar()
{
	bool q = ((gnBarStat & BTS_LARGEICON) != 0);
	HWND hwnd = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(q ? IDB_M_PROPBAR : IDB_PROPBAR), TBSTYLE_LIST | TBSTYLE_TRANSPARENT, IDW_PROPBAR);
	if (hwnd) {
		m_propbar = hwnd;
		SetDropDownButton(hwnd);
		AddCustomDrawBar(hwnd);
		return TRUE;
	}

	CannotCreateWindow(m_hWnd, _T("PropertyBar"));
	return FALSE;
}

BOOL CMainFrame::CreateSortBar()
{
	bool q = ((gnBarStat & BTS_LARGEICON) != 0);
	HWND hwnd = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(q ? IDB_M_SORTBAR : IDB_SORTBAR), TBSTYLE_FLAT, IDW_SORTBAR);
	if (hwnd) {
		m_sortbar = hwnd;
		SetDropDownButton(hwnd);
		AddCustomDrawBar(hwnd);
		return TRUE;
	}

	CannotCreateWindow(m_hWnd, _T("SortBar"));
	return FALSE;
}

BOOL CMainFrame::CreateNaviBar()
{
	bool q = ((gnBarStat & BTS_LARGEICON) != 0);
	HWND hwnd = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(q ? IDB_M_NAVIBAR : IDB_NAVIBAR), TBSTYLE_FLAT, IDW_NAVIGATIONBAR);
	if (hwnd) {
		SetDropDownButton(hwnd);
		m_navibar.SubclassWindow(hwnd);
		int index = m_navibar.CommandToIndex(IDM_DIR_QUICKACCESS);
		m_navibar.Initialize(index, 2114);

		AddCustomDrawBar(hwnd);
		return TRUE;
	}

	CannotCreateWindow(m_hWnd, _T("NaviBar"));
	return FALSE;
}

BOOL CMainFrame::CreateDriveBar()
{
	bool vert = ((gnBarStat & BTS_VERTDRIVE) != 0);
	m_drive.CreateBar(m_hWnd, IDW_DRIVEBAR, vert);
	if (!m_drive.IsWindow()) {
		CannotCreateWindow(m_hWnd, _T("DriveBar"));
		return FALSE;
	}

	SetupDriveBar();
	return TRUE;
}

BOOL CMainFrame::CreateSmartBar()
{
	m_smartbar.Create(m_hWnd, 0, NULL, ATL_SIMPLE_TOOLBAR_PANE_STYLE, 0, IDW_SMARTBAR, 0);
	if (!m_smartbar.IsWindow()) {
		CannotCreateWindow(m_hWnd, _T("SmartBar"));
		return FALSE;
	}

	// 以前の状態を復帰
	m_smartbar.LoadItems();
	return TRUE;
}

void CMainFrame::ResetCustomDrawToolbar(HWND hwnd, UINT resid)
{
	RemoveCustomDrawBar(hwnd);
	lccReplaceToolBarImageList(hwnd, RESINST, MAKEINTRESOURCE(resid));
	AddCustomDrawBar(hwnd);
}

void CMainFrame::ResetToolbarImage()
{
	bool q = ((gnBarStat & BTS_LARGEICON) != 0);

	ResetCustomDrawToolbar(m_propbar, q ? IDB_M_PROPBAR : IDB_PROPBAR);
	ResetCustomDrawToolbar(m_sortbar, q ? IDB_M_SORTBAR : IDB_SORTBAR);
	ResetCustomDrawToolbar(m_navibar, q ? IDB_M_NAVIBAR : IDB_NAVIBAR);

	int num = m_arrToolbar.GetSize();
	for (int i = 0; i < num; i++) {
		ResetCustomDrawToolbar(*m_arrToolbar[i], q ? IDB_M_TOOLBAR : IDB_TOOLBAR);
	}
}

void CMainFrame::OnToolbarBeginDrag(UINT /*nID*/, NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = 0;
/* ---- ボタンを1秒押し続けたら、ドロップダウンメニューを出す、という仕組み Google Chrome

	NMTOOLBAR* pnmtb = (NMTOOLBAR*)pNmhdr;
	CToolBarCtrl bar = pnmtb->hdr.hwndFrom;
	for (int i = 0; i < ARRAYSIZE(tfDropDown); i++) {
		if (pnmtb->iItem == (int)tfDropDown[i].id) {
			// ドロップダウンスタイル
			CPoint pt;
			GetCursorPos(&pt);
			if (lccBeginTrack2(m_hWnd, pt, 1000)) {
				NMTOOLBAR nmtb = *pnmtb;
				bar.GetItemRect(bar.CommandToIndex(pnmtb->iItem), &nmtb.rcButton);

				int state = bar.GetState(pnmtb->iItem) & ~TBSTATE_PRESSED;
				bar.SetState(pnmtb->iItem, state | TBSTATE_PRESSED);
				DoToolBarDropDown(&nmtb);
				bar.SetState(pnmtb->iItem, state);

				*pResult = 1;
			}
		}
	}
-------------------------------------------------------------------- */
}

void CMainFrame::OnToolbarBeginAdjust(UINT /*nID*/, NMHDR* /*pNmhdr*/, LRESULT* pResult)
{
	*pResult = 0;
}

void CMainFrame::OnToolbarEndAdjust(UINT /*nID*/, NMHDR* /*pNmhdr*/, LRESULT* pResult)
{
	*pResult = 0;
}

void CMainFrame::OnToolbarChange(UINT /*nID*/, NMHDR* pNmhdr, LRESULT* pResult)
{
	SetDropDownButton(pNmhdr->hwndFrom);
	SaveToolBarStateAll();
	*pResult = 0;
}

void CMainFrame::OnToolbarQueryInsert(UINT /*nID*/, NMHDR* /*pNmhdr*/, LRESULT* pResult)
{
	*pResult = TRUE;
}

void CMainFrame::OnToolbarQueryDelete(UINT /*nID*/, NMHDR* /*pNmhdr*/, LRESULT* pResult)
{
	*pResult = TRUE;
}

void CMainFrame::OnToolbarGetButtonInfo(UINT nID, NMHDR* pNmhdr, LRESULT* pResult)
{
	TBNOTIFY* ptbn = (TBNOTIFY*)pNmhdr;
	if (ptbn->iItem >= siTbMap.GetSize()) {
		*pResult = FALSE;
		return;
	}

	CSimpleArray<CString> arr;
	int idCommand = siTbMap.Lookup(ptbn->iItem);
	CString tip = _T("");
	if (LoadTokenizedString(idCommand, _T("\n"), arr) > 1) {
		tip = arr[1];
	}
	_tcsncpy_s(ptbn->pszText, ptbn->cchText, tip, _TRUNCATE);
	ptbn->tbButton.iBitmap = ptbn->iItem;
	ptbn->tbButton.idCommand = idCommand;
	ptbn->tbButton.fsState = TBSTATE_ENABLED;
	ptbn->tbButton.fsStyle = TBSTYLE_BUTTON;
	ptbn->tbButton.dwData = 0;
	ptbn->tbButton.iString = 0;
	*pResult = TRUE;
}

void CMainFrame::OnToolbarCustomHelp(UINT /*nID*/, NMHDR* /*pNmhdr*/, LRESULT* /*pResult*/)
{
}

void CMainFrame::OnToolbarReset(UINT /*nID*/, NMHDR* pNmhdr, LRESULT* /*pResult*/)
{
	int num = m_arrToolbar.GetSize();
	CToolBarCtrl* pBar;
	for (int i = 0; i < num; i++) {
		pBar = m_arrToolbar[i];
		if (pNmhdr->hwndFrom == *pBar) {
			RestoreToolBarState(*pBar, i);
			break;
		}
	}
}

int CMainFrame::DoToolBarDropDown(TBNOTIFY* ptbn)
{
	CPoint pt(ptbn->rcButton.left, ptbn->rcButton.bottom - 1);

	CToolBarCtrl bar = ptbn->hdr.hwndFrom;
	bar.ClientToScreen(&pt);

	CMenuHandle pop;
	CIDListPtr idl;

	CHistory* pHistory;
	switch (ptbn->iItem) {
	case IDM_DIR_SYSTEMFOLDER:
		pop = GetSubSequenceMenu(2, 5);
		TrackMenu(pop, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt);
		break;

	case IDM_FILE_SHELLNEW:
		pop = GetSubSequenceMenu(0, 3);
		TrackMenu(pop, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt);
		break;

	case IDM_DIR_PREV:
		pHistory = GetHistory();
		ATLASSERT(pHistory != NULL);
		if (idl.Clone(pHistory->BackwardHistoryMenu(m_hWnd, pt))) {
			pHistory->DisableUpdate(true);
			OpenFolder(idl);
		}
		break;

	case IDM_DIR_NEXT:
		pHistory = GetHistory();
		ATLASSERT(pHistory != NULL);
		if (idl.Clone(pHistory->ForwardHistoryMenu(m_hWnd, pt))) {
			pHistory->DisableUpdate(true);
			OpenFolder(idl);
		}
		break;

	case IDM_SORT:
		pop = GetSubSequenceMenu(3, 7);
		TrackMenu(pop, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt);
		break;

	case IDM_ICON_IMAGE:
		pop = GetSubSequenceMenu(3, 9);
		TrackMenu(pop, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt);
		break;

	case IDM_VIEW_HOTKEYMENU:
	{
		CHotKeyTrackWnd wnd;
		wnd.DoTrack(m_hWnd, &fvApp.menuHotKey, pt);
		break;
	}

	case IDM_DIR_QUICKACCESS:
		QuickAccessMenu(pt);
		break;

	case IDM_VIEW_WILDCARD:
		FileFilterList(pt);
		break;

	case IDM_FILE_MASKSELECT:
		MaskSelectList(pt);
		break;

	default:
		return TBDDRET_NODEFAULT;
	}

	return TBDDRET_DEFAULT;
}

void CMainFrame::SaveToolBarStateAll()
{
	int limit = IDW_TBLAST - IDW_TBFIRST;
	int cnt = m_arrToolbar.GetSize();
	SetRegValue(IDS_SEC_TOOLBAR, IDS_KEY_COUNT, cnt);

	CToolBarCtrl* pBar;
	int i;
	for (i = 0; i < cnt; i++) {
		pBar = m_arrToolbar[i];
		pBar->SetDlgCtrlID(IDW_TBFIRST + i);
		SaveToolBarState(*pBar, i);
	}

	// 残りのレジストリを掃除
	for (; i < limit; i++) {
		SetRegTextSeq(IDS_SEC_TOOLBAR, IDS_KEY_STATE, NULL, i);
		SetRegTextSeq(IDS_SEC_TOOLBAR, IDS_KEY_TITLE, NULL, i);
	}
}

void CMainFrame::SaveToolBarState(HWND hBar, int index)
{
	CToolBarCtrl bar = hBar;

	BYTE cb[MAX_PATH];
	int cnt = bar.GetButtonCount();
	TBBUTTON tbb;
	for (int i = 0; i < cnt; i++) {
		bar.GetButton(i, &tbb);
		if (tbb.fsStyle & TBSTYLE_SEP)
			cb[i] = SEPCODE;
		else
			cb[i] = (BYTE)tbb.iBitmap;
	}
	SetRegBinarySeq(IDS_SEC_TOOLBAR, IDS_KEY_STATE, index, cb, cnt);

	CString str;
	bar.GetWindowText(str);
	SetRegTextSeq(IDS_SEC_TOOLBAR, IDS_KEY_TITLE, str, index);
}

void CMainFrame::RestoreToolBarState(HWND hBar, int index)
{
	CToolBarCtrl bar = hBar;
	CString str;
	if (GetRegTextSeq(IDS_SEC_TOOLBAR, IDS_KEY_TITLE, str, index) < 1) {
		str.LoadString(IDW_TOOLBAR);
		str.AppendFormat(_T("%d"), index);
	}
	bar.SetWindowText(str);
	BYTE cb[MAX_PATH];
	int cnt = GetRegBinarySeq(IDS_SEC_TOOLBAR, IDS_KEY_STATE, index, cb, ARRAYSIZE(cb));
	if (cnt > 0) {
		BuildButtons(hBar, cb, cnt);
	} else {
		// ボタン全削除
		cnt = bar.GetButtonCount();
		while (cnt-- > 0)
			bar.DeleteButton(0);
	}
}

/** */
void CMainFrame::BuildButtons(HWND hBar, LPBYTE pcbBtnSeq, int len)
{
	CToolBarCtrl bar = hBar;

	// ボタン全削除
	int cnt = bar.GetButtonCount();
	while (cnt-- > 0)
		bar.DeleteButton(0);

	TBBUTTON tbb;
	memset(&tbb, 0, sizeof(tbb));
	tbb.fsState = TBSTATE_ENABLED;
	for (int i = 0; i < len; i++) {
		int index = *pcbBtnSeq++;
		if (index == SEPCODE) {
			tbb.idCommand = ID_SEPARATOR;
			tbb.iBitmap = 6;
			tbb.fsStyle = BTNS_SEP;
		}
		else {
			tbb.idCommand = siTbMap.Lookup(index);
			tbb.iBitmap = index;
			tbb.fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
		}
		bar.AddButtons(1, &tbb);
	}
	SetDropDownButton(hBar);
}

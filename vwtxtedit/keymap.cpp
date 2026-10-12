// keymap.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "mainfrm.h"
#include "keymap.h"

#define X_CHK			16
#define X_FNC			20
#define X_ABC			20
#define DUMMY_VAL		20

static TCHAR t_ctl[] = _T("Ctrl+");
static TCHAR t_sft[] = _T("Shift+");


static UINT def_abc_key[52] = {
	ID_EDIT_SELECT_ALL,				// Ctrl + A 全て選択
	ID_JUMP_FILEEND,				// Ctrl + B ファイルの最後へ
	ID_EDIT_COPY,					// Ctrl + C コピー
	0,								// Ctrl + D
	0,								// Ctrl + E
	ID_FIND_SEARCH,					// Ctrl + F 検索
	ID_FIND_GREP,					// Ctrl + G ファイルから検索
	ID_FIND_REPLACE,				// Ctrl + H 置換
	0,								// Ctrl + I
	ID_JUMP_LINE,					// Ctrl + J 指定行へ移動
	ID_EDIT_DELETE_RIGHT,			// Ctrl + K 行右削除
	ID_EDIT_DELETE_LINE,			// Ctrl + L １行削除
	0,								// Ctrl + M
	ID_FILE_NEW,					// Ctrl + N 新規作成
	ID_FILE_OPEN,					// Ctrl + O ファイルを開く
	0,								// Ctrl + P
	0,								// Ctrl + Q
	0,								// Ctrl + R
	0,								// Ctrl + S
	ID_JUMP_FILETOP,				// Ctrl + T ファイルの先頭へ
	ID_EDIT_DELETE_LEFT,			// Ctrl + U 行左削除
	ID_EDIT_PASTE,					// Ctrl + V 貼り付け
	ID_JUMP_LASTEDITPOS,			// Ctrl + W 最後に編集した場所
	ID_EDIT_CUT,					// Ctrl + X 切り取り
	0,								// Ctrl + Y
	ID_EDIT_UNDO,					// Ctrl + Z 元に戻す
	0,								// Ctrl + Shift + A
	0,								// Ctrl + Shift + B
	ID_EDIT_COPY_APPEND,			// Ctrl + Shift + C 追加してコピー
	0,								// Ctrl + Shift + D
	0,								// Ctrl + Shift + E
	ID_FIND_SEARCH_BACK,			// Ctrl + Shift + F 後方検索
	0,								// Ctrl + Shift + G
	ID_EDIT_COPY_FOLLOW,			// Ctrl + Shift + H 引用つきコピー
	0,								// Ctrl + Shift + I
	0,								// Ctrl + Shift + J
	0,								// Ctrl + Shift + K
	0,								// Ctrl + Shift + L
	0,								// Ctrl + Shift + M
	0,								// Ctrl + Shift + N
	0,								// Ctrl + Shift + O
	0,								// Ctrl + Shift + P
	0,								// Ctrl + Shift + Q
	0,								// Ctrl + Shift + R
	0,								// Ctrl + Shift + S
	0,								// Ctrl + Shift + T
	0,								// Ctrl + Shift + U
	0,								// Ctrl + Shift + V
	0,								// Ctrl + Shift + W
	0,								// Ctrl + Shift + X
	0,								// Ctrl + Shift + Y
	ID_EDIT_REDO,					// Ctrl + Shift + Z やり直す
};
#define ABCARRAYSIZE				(sizeof(def_abc_key))

static UINT def_fnc_key[48] = {
	0,								// F1
	0,								// F2
	ID_FIND_NEXT,					// F3 次を検索
	0,								// F4
	0,								// F5
	0,								// F6
	0,								// F7
	ID_VIEW_CHARCODE,				// F8 文字コードを表示
	ID_FIND_FUNCTION,				// F9 関数検索
	ID_JUMP_TAG,					// F10 タグジャンプ
	0,								// F11
	0,								// F12
	0,								// Shift + F1
	0,								// Shift + F2
	ID_FIND_PREV,					// Shift + F3 前を検索
	0,								// Shift + F4
	0,								// Shift + F5
	0,								// Shift + F6
	0,								// Shift + F7
	0,								// Shift + F8
	0,								// Shift + F9
	0,								// Shift + F10
	0,								// Shift + F11
	0,								// Shift + F12
	0,								// Ctrl + F1
	0,								// Ctrl + F2
	0,								// Ctrl + F3
	0,								// Ctrl + F4
	0,								// Ctrl + F5
	0,								// Ctrl + F6
	0,								// Ctrl + F7
	0,								// Ctrl + F8
	0,								// Ctrl + F9
	0,								// Ctrl + F10
	0,								// Ctrl + F11
	0,								// Ctrl + F12
	0,								// Shift + Ctrl + F1
	0,								// Shift + Ctrl + F2
	0,								// Shift + Ctrl + F3
	0,								// Shift + Ctrl + F4
	0,								// Shift + Ctrl + F5
	0,								// Shift + Ctrl + F6
	0,								// Shift + Ctrl + F7
	0,								// Shift + Ctrl + F8
	0,								// Shift + Ctrl + F9
	0,								// Shift + Ctrl + F10
	0,								// Shift + Ctrl + F11
	0								// Shift + Ctrl + F12
};
#define FNCARRAYSIZE				(sizeof(def_fnc_key))

static UINT nKeyMapList[] = {
	0,
	ID_FILE_NEW,
	ID_FILE_OPEN,
	ID_FILE_OPENCLIPBOARD,
	ID_FILE_SAVE,
	ID_FILE_SAVE_AS,
	ID_EDIT_UNDO,
	ID_EDIT_REDO,
	ID_EDIT_CUT,
	ID_EDIT_CUT_APPEND,
	ID_EDIT_PASTE,
	ID_EDIT_COPY,
	ID_EDIT_COPY_FOLLOW,
	ID_EDIT_COPY_APPEND,
	ID_EDIT_SELECT_ALL,
	ID_EDIT_DELETE_CHAR,
	ID_EDIT_UPPERCASE,
	ID_EDIT_LOWERCASE,
	ID_EDIT_DATETIME,
	ID_EDIT_DELETE_LINE,
	ID_EDIT_DELETE_LEFT,
	ID_EDIT_DELETE_RIGHT,
	ID_EDIT_SELECTLINE,
	ID_EDIT_DUPLICATELINE,
	ID_EDIT_READONLY,
	ID_FIND_SEARCH,
	ID_FIND_PREV,
	ID_FIND_NEXT,
	ID_FIND_FUNCTION,
	ID_JUMP_FILETOP,
	ID_JUMP_FILEEND,
	ID_JUMP_LINE,
	ID_FIND_REPLACE,
	ID_FIND_GREP,
	ID_JUMP_TAG,
	ID_JUMP_LINETOP,
	ID_JUMP_LINEEND,
	ID_JUMP_LASTEDITPOS,
    ID_PAGE_PREV,
    ID_PAGE_NEXT,
    ID_PAGE_PREV_HALF,
    ID_PAGE_NEXT_HALF,
	ID_VIEW_CHARCODE,
};

////////////////////////////////////////////////////////////////////////
//

#pragma pack(1)

struct __ToolBarData {
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]
	WORD* items() { return (WORD*)(this+1); }
};

#pragma pack()


int GetToolbarCommandFromResource(UINT idToolbar, CSimpleArray<UINT>& array)
{
	array.RemoveAll();
	HRSRC hRsrc = ::FindResource(RESINST, MAKEINTRESOURCE(idToolbar), RT_TOOLBAR);
	if (hRsrc) {
		HGLOBAL hGlobal = ::LoadResource(RESINST, hRsrc);
		if (hGlobal) {
			__ToolBarData* pData = (__ToolBarData*)::LockResource(hGlobal);
			if (pData) {
				ATLASSERT(pData->wVersion == 1);
				WORD* pItems = pData->items();
				for (WORD i = 0; i < pData->wItemCount; i++) {
					if (pItems[i] != 0)
						array.Add(pItems[i]);
				}
			}
		}
	}
	return array.GetSize();
}

void GetToolbarButtonSize(UINT idToolbar, CSize& size)
{
	size.cx = size.cy = 20;
	HRSRC hRsrc = ::FindResource(RESINST, MAKEINTRESOURCE(idToolbar), RT_TOOLBAR);
	if (hRsrc) {
		HGLOBAL hGlobal = ::LoadResource(RESINST, hRsrc);
		if (hGlobal) {
			__ToolBarData* pData = (__ToolBarData*)::LockResource(hGlobal);
			if (pData) {
				size.cx = pData->wWidth;
				size.cy = pData->wHeight;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//

void AddNormalTab(HWND hwnd, LPCTSTR lpTitle, HIMAGELIST hImageList)
{
	TCHAR buf[512];

	if (hImageList)
		::SendMessage(hwnd, TCM_SETIMAGELIST, 0, (LPARAM)hImageList);

	lstrcpy(buf, lpTitle);
	TCHAR *p = buf;
	TC_ITEM it;
	int nIndex = 0;
	while (p && *p) {
		TCHAR *q = StrChr(p, '|');
		if (q)
			*q++ = '\0';

		ZeroMemory(&it, sizeof(it));
		it.mask = TCIF_TEXT;
		it.iImage = -1;
		if (hImageList) {
			it.mask |= TCIF_IMAGE;
			it.iImage = nIndex;
		}
		it.pszText = p;
		it.cchTextMax = lstrlen(p);
		::SendMessage(hwnd, TCM_INSERTITEM, nIndex, (LPARAM)(const TC_ITEM*)(&it));
		nIndex++;
		p = q;
	}
}

void GetCmdName(UINT nKey, TCHAR *pszCmdName)
{
	*pszCmdName = 0;
	if (LoadString(ATL::_AtlBaseModule.GetResourceInstance(), nKey, pszCmdName, MAX_PATH) > 0) {
		TCHAR* p = StrRChr(pszCmdName, NULL, '\n');
		if (p) {
			memmove(pszCmdName, p + 1, sizeof(TCHAR) * (lstrlen(p + 1) + 1));
			*p = 0;
		}
	}
	if (*pszCmdName == 0)
		LoadString(ATL::_AtlBaseModule.GetResourceInstance(), IDS_UNDEFINED, pszCmdName, MAX_PATH);
}

/////////////////////////////////////////////////////////////////////////////
// CKeyPage プロパティ ページ

CKeyPage::CKeyPage()
{
	//{{AFX_DATA_INIT(CKeyPage)
	//}}AFX_DATA_INIT

	m_bUpdate = FALSE;
	m_bUseCtrl = FALSE;
	m_bShift = FALSE;
	m_bCtrl = FALSE;
	m_himKey = NULL;

	// ツールバーイメージリスト
	GetToolbarButtonSize(IDB_S_TOOLBAR, m_sizeBarButton);
	m_himTool = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_S_TOOLBAR), m_sizeBarButton.cx, RGB(255, 0, 255));
	GetToolbarCommandFromResource(IDB_S_TOOLBAR, m_arrToolbarCommand);
	// イメージリスト
	m_himChk = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_CHECK), X_CHK, RGB(255, 0, 255));
}

CKeyPage::~CKeyPage()
{
	if (m_himChk) ImageList_Destroy(m_himChk);
	if (m_himKey) ImageList_Destroy(m_himKey);
	if (m_himTool) ImageList_Destroy(m_himTool);
}

/////////////////////////////////////////////////////////////////////////////
// CKeyPage メッセージ ハンドラ

BOOL CKeyPage::OnInitDialog(LPARAM lParam) 
{
	m_keylist = GetDlgItem(IDC_KEYLIST);
	m_cmdlist = GetDlgItem(IDC_CMDLIST);

	// Ctrlキーの利用
	m_bCtrl = m_bUseCtrl;
	CheckDlgButton(IDC_USECTRL, m_bCtrl);
	GetDlgItem(IDC_USECTRL).EnableWindow(!m_bCtrl);

	// コマンドリスト列挙
	TCHAR buf[MAX_PATH * 2];
	int i;
	for (i = 0; i < sizeof(nKeyMapList) / sizeof(UINT); i++) {
		GetCmdName(nKeyMapList[i], buf);
		m_cmdlist.AddString(buf);
	}

	m_keylist.ResetContent();
	m_keylist.SetRedraw(FALSE);
	for (i = 0; i < m_numkeys; i++) {
		int index = m_keylist.AddString((LPCTSTR)DUMMY_VAL);
		m_keylist.SetItemData(index, (DWORD)i);
	}
	m_keylist.SetRedraw(TRUE);
	m_keylist.Invalidate();
	m_keylist.SetCurSel(0);
	SelectCommand();
	return TRUE;
}

void CKeyPage::OnMeasureItem(int idCtl, MEASUREITEMSTRUCT* pmi)
{
	pmi->itemHeight = (idCtl == IDC_KEYLIST) ? 
		max((UINT)m_sizeImage.cx + 2, pmi->itemHeight) :
		max((UINT)m_sizeBarButton.cy + 2, pmi->itemHeight);
}

void CKeyPage::OnDrawItem(int idCtl, DRAWITEMSTRUCT* pdi)
{
	CDCHandle dc = pdi->hDC;
	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		COLORREF clrFore, clrBack;
		if (pdi->itemState & ODS_SELECTED) {
			clrFore = GetSysColor(COLOR_HIGHLIGHTTEXT);
			clrBack = GetSysColor(COLOR_HIGHLIGHT);
		}
		else {
			clrFore = GetSysColor(COLOR_WINDOWTEXT);
			clrBack = GetSysColor(COLOR_WINDOW);
		}

		dc.SetTextColor(clrFore);
		dc.SetBkColor(clrBack);

		CRect rc(&pdi->rcItem);
		dc.ExtTextOut(rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc, NULL, 0, NULL);

		dc.SetBkMode(TRANSPARENT);
		TCHAR buf[MAX_PATH];
		if (idCtl == IDC_KEYLIST) {
			UINT cmd = GetCommand((int)pdi->itemData);
			if (cmd > 0)
				ImageList_Draw(m_himChk, 1, dc, rc.left + 2, (rc.top + rc.bottom - X_ABC) / 2, ILD_NORMAL);

			rc.left += X_CHK + 4;
			buf[0] = 0;
			if (m_bCtrl)
				lstrcat(buf, t_ctl);
			if (m_bShift)
				lstrcat(buf, t_sft);
			if (buf[0]) {
				dc.SetBkMode(TRANSPARENT);
				dc.DrawText(buf, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
			}
			CSize csz;
			dc.GetTextExtent(buf, lstrlen(buf), &csz);
			rc.left += csz.cx;
			ImageList_Draw(m_himKey, (int)pdi->itemData, dc, rc.left + 2, (rc.top + rc.bottom - m_sizeImage.cy) / 2, ILD_NORMAL);
			rc.left += m_sizeImage.cx + 4;

			if (cmd > 0) {
				GetCmdName(cmd, buf);
				dc.DrawText(buf, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
			}
		}
		else {
			rc.left += 2;

			UINT cmd = nKeyMapList[pdi->itemID];
			if (cmd > 0) {
				int index = m_arrToolbarCommand.Find(cmd);
				if (index >= 0) {
					ImageList_Draw(m_himTool, index, dc, rc.left, (rc.top + rc.bottom - m_sizeBarButton.cy) / 2, ILD_NORMAL);
				}
			}
			rc.left += m_sizeBarButton.cx + 4;

			m_cmdlist.GetText(pdi->itemID, buf);
			dc.DrawText(buf, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
		}
	}
	if (pdi->itemAction & ODA_FOCUS)
		dc.DrawFocusRect(&pdi->rcItem);
}

void CKeyPage::OnSelChangeKeyList()
{
	SelectCommand();
}

void CKeyPage::OnSelChangeCmdList()
{
	int index = m_keylist.GetCurSel();
	if (index != LB_ERR) {
		int sel = m_cmdlist.GetCurSel();
		if (sel != LB_ERR) {
			int key = index;
			if (m_bUseCtrl) {
				if (m_bShift)
					key += m_numkeys;
			}
			else {
				if (m_bShift)
					key += m_numkeys;
				if (m_bCtrl)
					key += m_numkeys * 2;
			}
			m_cmds[key] = nKeyMapList[sel];

			CRect rc;
			m_keylist.GetItemRect(index, &rc);
			m_keylist.InvalidateRect(&rc, TRUE);

			m_bUpdate = TRUE;
		}
	}
}

void CKeyPage::OnShiftCheck()
{
	m_bShift = IsDlgButtonChecked(IDC_USESHIFT);
	m_keylist.RedrawWindow();

	SelectCommand();
}

void CKeyPage::OnCtrlCheck()
{
	m_bCtrl = m_bUseCtrl ? TRUE : IsDlgButtonChecked(IDC_USECTRL);
	m_keylist.RedrawWindow();

	SelectCommand();
}

UINT CKeyPage::GetCommand(int nIndex)
{
	if (m_bUseCtrl) {
		if (m_bShift)
			nIndex += m_numkeys;
	}
	else {
		if (m_bShift)
			nIndex += m_numkeys;
		if (m_bCtrl)
			nIndex += m_numkeys * 2;
	}
	return m_cmds[nIndex];
}

void CKeyPage::SelectCommand()
{
	int nSel = m_keylist.GetCurSel();
	if (nSel == LB_ERR)
		return;

	UINT nCommand = GetCommand(nSel);
	int cnt = m_cmdlist.GetCount();

	for (int i = 0; i < sizeof(nKeyMapList) / sizeof(UINT); i++) {
		if (nKeyMapList[i] == nCommand) {
			m_cmdlist.SetCurSel(i);
			return;
		}
	}
	m_cmdlist.SetCurSel(0);
}

BOOL CKeyPage::OnApply()
{
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////

/* @@
void CMainFrame::MakeLocalAccelTable()
{
	UINT mabc[52], mfnc[48];
	if (GetRegBinary(IDS_SEC_SETTING, IDS_KEY_ABCMAP, (BYTE*)mabc, ABCARRAYSIZE) != ABCARRAYSIZE)
		memcpy(mabc, def_abc_key, ABCARRAYSIZE);
	if (GetRegBinary(IDS_SEC_SETTING, IDS_KEY_FNCMAP, (BYTE*)mfnc, FNCARRAYSIZE) != FNCARRAYSIZE)
		memcpy(mfnc, def_fnc_key, FNCARRAYSIZE);

	HACCEL hAccel = LoadAccelerators(ATL::_AtlBaseModule.GetResourceInstance(), _MKIR(IDR_FRAME));
	int numAccl = (hAccel != NULL) ? CopyAcceleratorTable(hAccel, NULL, 0) : 0;
	LPACCEL pa = new ACCEL[52 + 48 + numAccl];
	if (numAccl > 0)
		CopyAcceleratorTable(hAccel, pa, numAccl);
	if (hAccel)
		DestroyAcceleratorTable(hAccel);

	// Ctrl + A,B,C...
	for (int i = 0; i < 26; i++) {
		if (mabc[i]) {
			pa[numAccl].fVirt = FVIRTKEY | FCONTROL;
			pa[numAccl].key = 'A' + i;
			pa[numAccl].cmd = mabc[i];
			numAccl++;
		}
		if (mabc[i + 26]) {
			pa[numAccl].fVirt = FVIRTKEY | FCONTROL | FSHIFT;
			pa[numAccl].key = 'A' + i;
			pa[numAccl].cmd = mabc[i + 26];
			numAccl++;
		}
	}

	// Fxx ...
	for (i = 0; i < 12; i++) {
		if (mfnc[i]) {
			pa[numAccl].fVirt = FVIRTKEY;
			pa[numAccl].key = VK_F1 + i;
			pa[numAccl].cmd = mfnc[i];
			numAccl++;
		}
		if (mfnc[i + 12]) {				// shift
			pa[numAccl].fVirt = FVIRTKEY | FSHIFT;
			pa[numAccl].key = VK_F1 + i;
			pa[numAccl].cmd = mfnc[i + 12];
			numAccl++;
		}
		if (mfnc[i + 24]) {				// control
			pa[numAccl].fVirt = FVIRTKEY | FCONTROL;
			pa[numAccl].key = VK_F1 + i;
			pa[numAccl].cmd = mfnc[i + 24];
			numAccl++;
		}
		if (mfnc[i + 36]) {				// shift + control
			pa[numAccl].fVirt = FVIRTKEY | FSHIFT | FCONTROL;
			pa[numAccl].key = VK_F1 + i;
			pa[numAccl].cmd = mfnc[i + 36];
			numAccl++;
		}
	}

	// 機能キーのアクセラレータテーブルを作成
	if (m_hAccel) {
		DestroyAcceleratorTable(m_hAccel);
		m_hAccel = NULL;
	}
	m_hAccel = CreateAcceleratorTable(pa, numAccl);

	// メニューへ反映
	if (numAccl > 0) {
		HMENU hMenu = m_menubar.GetMenu();
		int numItems = ::GetMenuItemCount(hMenu);
		for (int i = 0; i < numItems; i++) {
			HMENU hSubMenu = GetSubMenu(hMenu, i);
			if (hSubMenu)
				lccApplyAccelMenu(hSubMenu, pa, numAccl);
		}
	}
	delete [] pa;
}
@@ */

void CMainFrame::MakeLocalAccelTable()
{
	UINT mabc[52], mfnc[48];
	CAppReg reg;
	if (reg.GetBinary(IDS_SEC_SETTING, IDS_KEY_ABCMAP, (BYTE*)mabc, ABCARRAYSIZE) != ABCARRAYSIZE)
		memcpy(mabc, def_abc_key, ABCARRAYSIZE);
	if (reg.GetBinary(IDS_SEC_SETTING, IDS_KEY_FNCMAP, (BYTE*)mfnc, FNCARRAYSIZE) != FNCARRAYSIZE)
		memcpy(mfnc, def_fnc_key, FNCARRAYSIZE);

	LPACCEL pa = new ACCEL[52 + 48];

	// Ctrl + A,B,C...
	int i, numAccl = 0;
	for (i = 0; i < 26; i++) {
		if (mabc[i]) {
			pa[numAccl].fVirt = FVIRTKEY | FCONTROL;
			pa[numAccl].key = 'A' + i;
			pa[numAccl].cmd = mabc[i];
			numAccl++;
		}
		if (mabc[i + 26]) {
			pa[numAccl].fVirt = FVIRTKEY | FCONTROL | FSHIFT;
			pa[numAccl].key = 'A' + i;
			pa[numAccl].cmd = mabc[i + 26];
			numAccl++;
		}
	}

	// Fxx ...
	for (i = 0; i < 12; i++) {
		if (mfnc[i]) {
			pa[numAccl].fVirt = FVIRTKEY;
			pa[numAccl].key = VK_F1 + i;
			pa[numAccl].cmd = mfnc[i];
			numAccl++;
		}
		if (mfnc[i + 12]) {				// shift
			pa[numAccl].fVirt = FVIRTKEY | FSHIFT;
			pa[numAccl].key = VK_F1 + i;
			pa[numAccl].cmd = mfnc[i + 12];
			numAccl++;
		}
		if (mfnc[i + 24]) {				// control
			pa[numAccl].fVirt = FVIRTKEY | FCONTROL;
			pa[numAccl].key = VK_F1 + i;
			pa[numAccl].cmd = mfnc[i + 24];
			numAccl++;
		}
		if (mfnc[i + 36]) {				// shift + control
			pa[numAccl].fVirt = FVIRTKEY | FSHIFT | FCONTROL;
			pa[numAccl].key = VK_F1 + i;
			pa[numAccl].cmd = mfnc[i + 36];
			numAccl++;
		}
	}

	// 機能キーのアクセラレータテーブルを作成
	if (m_hLocalAccel) {
		DestroyAcceleratorTable(m_hLocalAccel);
		m_hLocalAccel = NULL;
	}

	if (numAccl > 0)
		m_hLocalAccel = CreateAcceleratorTable(pa, numAccl);
	delete [] pa;
}

void CMainFrame::UpdateMenuAccelerator()
{
	HMENU hmenu = m_menubar.GetMenu();
	if (!IsMenu(hmenu)) return;

	int cnt, numAccl = 0;
	ACCEL* pa = NULL;
	if (m_hAccel) {
		cnt = CopyAcceleratorTable(m_hAccel, NULL, 0);
		pa = (ACCEL*)malloc(cnt * sizeof(ACCEL));
		numAccl = CopyAcceleratorTable(m_hAccel, pa, cnt);
	}
	if (m_hLocalAccel) {
		cnt = CopyAcceleratorTable(m_hLocalAccel, NULL, 0);

		ACCEL* paw;
		if (!pa) {
			pa = (ACCEL*)malloc(cnt * sizeof(ACCEL));
			paw = pa;
		}
		else {
			pa = (ACCEL*)realloc(pa, (cnt + numAccl) * sizeof(ACCEL));
			paw = pa + numAccl;
		}
		numAccl += CopyAcceleratorTable(m_hLocalAccel, paw, cnt);
	}

	if (pa) {
		// サブメニューを変換
		int numItems = ::GetMenuItemCount(hmenu);
		for (int i = 0; i < numItems; i++) {
			HMENU hSubMenu = GetSubMenu(hmenu, i);
			if (hSubMenu)
				lccApplyAccelMenu(hSubMenu, pa, numAccl);
		}

		// メインメニューを変換
		lccApplyAccelMenu(hmenu, pa, numAccl);
		delete [] pa;
	}
}

void CMainFrame::OnSetupKeymap() 
{
	CAppReg reg;

	HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
	CKeyPage page_abc;
	page_abc.m_himKey = lccLoadImageList(hInst, RT_RCDATA, _MKIR(IDB_ABC), X_ABC, RGB(255, 0, 255));
	page_abc.m_sizeImage = CSize(X_ABC, X_ABC);
	page_abc.m_numkeys = (ABCARRAYSIZE / sizeof(UINT)) / 2;
	page_abc.m_bUseCtrl = TRUE;
	if (reg.GetBinary(IDS_SEC_SETTING, IDS_KEY_ABCMAP, (BYTE*)page_abc.m_cmds, ABCARRAYSIZE) < ABCARRAYSIZE)
		CopyMemory(page_abc.m_cmds, def_abc_key, sizeof(def_abc_key));

	CKeyPage page_fnc;
	page_fnc.m_himKey = lccLoadImageList(hInst, RT_RCDATA, _MKIR(IDB_FNC), X_FNC, RGB(255, 0, 255));
	page_fnc.m_sizeImage = CSize(X_FNC, X_ABC);
	page_fnc.m_numkeys = (FNCARRAYSIZE / sizeof(UINT)) / 4;
	page_fnc.m_bUseCtrl = FALSE;
	if (reg.GetBinary(IDS_SEC_SETTING, IDS_KEY_FNCMAP, (BYTE*)page_fnc.m_cmds, FNCARRAYSIZE) != FNCARRAYSIZE)
		CopyMemory(page_fnc.m_cmds, def_fnc_key, FNCARRAYSIZE);

	TCHAR buf[MAX_PATH];
	GetCmdName(ID_OPTION_KEYMAP, buf);
	CKeySheet sheet(buf);
	sheet.AddPage(page_abc);
	sheet.AddPage(page_fnc);

	if (sheet.DoModal() == IDOK) {
		if (page_abc.m_bUpdate)
			reg.SetBinary(IDS_SEC_SETTING, IDS_KEY_ABCMAP, (BYTE*)page_abc.m_cmds, ABCARRAYSIZE);
		if (page_fnc.m_bUpdate)
			reg.SetBinary(IDS_SEC_SETTING, IDS_KEY_FNCMAP, (BYTE*)page_fnc.m_cmds, FNCARRAYSIZE);
		if (page_abc.m_bUpdate || page_fnc.m_bUpdate) {
			MakeLocalAccelTable();
			UpdateMenuAccelerator();
		}
	}
}

////////////////////////////////////////////////////////////////////////
// keymap.cpp : ショートカットキー
//

#include "stdafx.h"
#include "fv7.h"
#include "keymap.h"


#define DUMMY_VAL		20

typedef struct tagKEYMAPSTRUCT {
	int nPosition;
	int nMenuCommand;
} KEYMAPSTRUCT;

static const TCHAR t_ctl[] = _T("[c]+");
static const TCHAR t_sft[] = _T("[s]+");

////////////////////////////////////////////////////////////////////////
//

UINT gnKeyMapList[] = {
	0,
	IDM_FILE_MKDIR,
	IDM_FILE_OPEN,
	IDM_FILE_COPY,
	IDM_FILE_MOVE,
	IDM_FILE_ATTRIB,
	IDM_FILE_RENAME,
	IDM_FILE_RENEXT,
	IDM_FILE_SERIESNAME,
	IDM_FILE_REPNAMES,
	IDM_FILE_CHGCAPS,
	IDM_FILE_DELETE,
	IDM_FILE_FILTERDEL,
	IDM_FILE_SHRED,
	IDM_FILE_MERGE,
	IDM_FILE_DIVIDE,
	IDM_FILE_PACK,
	IDM_FILE_UNPACK,
	IDM_FILE_CHECKARC,
//@	IDM_FILE_ENCRYPT,
//@	IDM_FILE_DECRYPT,
	IDM_FILE_SENDTO,
	IDM_FILE_CONTEXTMENU,
	IDM_FILE_PROPERTY,
	IDM_FILE_ASSOCIATE,
	IDM_FILE_EXECUTE,
	IDM_FILE_SHELLNEW,
	IDM_FILE_SHORTCUT,
	IDM_FILE_NEWFILE,
	ID_APP_EXIT,
	ID_APP_EXITALL,
	IDM_EDIT_CUT,
	IDM_EDIT_COPY,
	IDM_EDIT_PASTE,
	IDM_EDIT_PASTESHORTCUT,
	IDM_COPY_NAMEONLY,
	IDM_COPY_DETAIL,
	IDM_COPY_FULLNAME,
	IDM_COPY_SHORTNAME,
	IDM_COPY_UNCNAME,
	IDM_EDIT_LISTOUTPUT,
//@	IDM_DISK_COPY,
//@	IDM_DISK_LABEL,
//@	IDM_DISK_FORMAT,
//@	IDM_DISK_MAKEIMAGE,
//@	IDM_DISK_RESTOREIMAGE,
	IDM_NET_CONNECT,
	IDM_NET_DISCONNECT,
	IDM_NET_ADDPLACE,
	IDM_DIR_UPDIR,
	IDM_DIR_HISTORY,
	IDM_DIR_PREV,
	IDM_DIR_NEXT,
	IDM_DIR_HOME,
	IDM_DIR_QUICKACCESS,
	IDM_DIR_ADDFAVORITE,
	IDM_DIR_SELECT,
	IDM_DIR_EMPTYRECYCLEBIN,
	IDM_DIR_SEARCH,
	IDM_DIR_SYNC,
	IDM_DIR_SCRIPTSYNC,
	IDM_VIEW_DESKTOPTREE,
	IDM_VIEW_ICON,
	IDM_HOT_ICON_NORMAL,
	IDM_HOT_ICON_REPORT,
	IDM_HOT_ICON_OUTLINE,
	IDM_HOT_ICON_IMAGE,
	IDM_HOT_ICON_ICON,
	IDM_HOT_ICON_SWITCH,
	IDM_ICON_RELAX,
	IDM_ICON_ENLARGE,
	IDM_VIEW_NAMEELLIPSIS,
	IDM_VIEW_SORT,
	IDM_SORT_NAME,
	IDM_SORT_TYPE,
	IDM_SORT_SIZE,
	IDM_SORT_TIME,
	IDM_FILE_SELALL,
	IDM_FILE_SELINVERT,
	IDM_VIEW_WILDCARD,
	IDM_VIEW_RELEASEFILTER,
	IDM_VIEW_WILDCARD_LIST,
	IDM_FILE_MASKSELECT,
	IDM_FILE_MASKSELECT_LIST,
	IDM_VIEW_FILEINFO,
	IDM_VIEW_FILESIZE,
	IDM_VIEW_DRIVEINFO,
	IDM_VIEW_KEYMAP,
	IDM_VIEW_HOTKEYMENU,
	IDM_BAR_SETTINGS,
	IDM_BAR_MENU,
	IDM_BAR_APPBAR,
	IDM_BAR_DRIVE,
	IDM_BAR_STAT,
	IDM_BAR_FUNCTION,
	IDM_BAR_FAVO,
	IDM_BAR_REPOSITION,
	IDM_TYPE_FILESUPERHIDDEN,
	IDM_TYPE_FILEHIDDEN,
	IDM_TOOL_VIEWER,
	IDM_TOOL_BDUMP,
	IDM_TOOL_EDITOR,
	IDM_TOOL_GREP,
  IDM_TOOL_MSDOS,
  IDM_TOOL_SHELL,
	IDM_TOOL_CALCHASH,
//@	IDM_TOOL_SLIDESHOW,
	IDM_OPTION_MISC,
	IDM_EXEC_OWN,
	IDM_TAB_NEW,
	IDM_TAB_CLOSE,
	IDM_TAB_PREV,
	IDM_TAB_NEXT,
	IDM_TAB_REOPEN_RECENT,
	IDM_TAB_SAVEALL,
	IDM_TAB_RESTOREALL,
	IDM_TAB_CLOSE_ANYOTHER,
	IDM_TAB_LOCK,
	IDM_TAB_EXPORT,
	IDM_WINDOW_MAXIMIZE,
	IDM_WINDOW_MINIMIZE,
	IDM_WINDOW_REFLESH,
	IDM_WINDOW_COMBINESAVE,
	IDM_WINDOW_COMBINELOAD,
	IDM_WINDOW_PREV,
	IDM_WINDOW_NEXT,
	ID_WINDOW_CASCADE,
	ID_WINDOW_TILE_HORZ,
	ID_WINDOW_TILE_VERT,
	ID_HELP_FINDER,
	ID_APP_ABOUT,
	IDM_DRVACCESS_A,
	IDM_DRVACCESS_B,
	IDM_DRVACCESS_C,
	IDM_DRVACCESS_D,
	IDM_DRVACCESS_E,
	IDM_DRVACCESS_F,
	IDM_DRVACCESS_G,
	IDM_DRVACCESS_H,
	IDM_DRVACCESS_I,
	IDM_DRVACCESS_J,
	IDM_DRVACCESS_K,
	IDM_DRVACCESS_L,
	IDM_DRVACCESS_M,
	IDM_DRVACCESS_N,
	IDM_DRVACCESS_O,
	IDM_DRVACCESS_P,
	IDM_DRVACCESS_Q,
	IDM_DRVACCESS_R,
	IDM_DRVACCESS_S,
	IDM_DRVACCESS_T,
	IDM_DRVACCESS_U,
	IDM_DRVACCESS_V,
	IDM_DRVACCESS_W,
	IDM_DRVACCESS_X,
	IDM_DRVACCESS_Y,
	IDM_DRVACCESS_Z,
};
int gnKeyMapListSize = ARRAYSIZE(gnKeyMapList);

/////////////////////////////////////////////////////////////////////////////
//

int GetCommandString(UINT nKey, CString& name)
{
	name.Empty();
	if (nKey) {
		CString str;
		CMenuHandle menu = fvApp.wndMain.GetMenu();
		if (menu.IsMenu() && menu.GetMenuString(nKey, str, MF_BYCOMMAND) > 0)
			name = str;
		else if (str.LoadString(nKey) > 0) {
			LPCTSTR p = StrRChr(str, NULL, _T('\n'));
			if (p)
				p++;
			else
				p = str;
			name = p;
		}
	}
	return lstrlen(name);
}

int GetCommandLabel(UINT nKey, CString& name)
{
	name.Empty();
	if (nKey) {
		GetCommandString(nKey, name);

		// アクセスキー'('あるいはTABを検索
		LPTSTR p = StrChr(name, _T('('));
		if (p)
			*p = 0;
		else {
			p = StrChr(name, _T('\t'));
			if (p)
				*p = 0;
		}
	}

	if (name.IsEmpty())
		name.LoadString(IDS_UNDEFINED);
	return lstrlen(name);
}

/////////////////////////////////////////////////////////////////////////////
// CKeyPage プロパティ ページ

CKeyPage::CKeyPage()
{
	m_bUpdate = false;
	m_bUseShift = false;
	m_bUseShiftCtrl = false;
	m_bShift = false;
	m_bCtrl = false;
	m_himKey = NULL;
	m_himCtrl = NULL;
	m_iStartImage = 0;
	m_numCmds = 0;
}

CKeyPage::~CKeyPage()
{
	m_himCtrl && ImageList_Destroy(m_himCtrl);
}

void CKeyPage::SetImage(HIMAGELIST himKey, int iStartImage, int numCmds)
{
	m_himKey = himKey;
	int cx = 0, cy = 0;
	m_himKey && ImageList_GetIconSize(m_himKey, &cx, &cy);
	m_sizeKey = CSize(cx, cy);
	m_iStartImage = iStartImage;
	m_numCmds = numCmds;
}

void CKeyPage::AddItem(UINT cmd)
{
	m_cmds.Add(cmd);
}
void CKeyPage::SetCommandList(const UINT* pcuarr, int arrsize)
{
	while (arrsize-- > 0) {
		m_cmds.Add(*pcuarr++);
	}
}
void CKeyPage::GetCommandList(UINT* puarr)
{
	CopyMemory(puarr, m_cmds.GetData(), m_cmds.GetSize() * sizeof(UINT));
}

UINT CKeyPage::GetItemValue(int index)
{
	return (index >= 0 && index < m_cmds.GetSize()) ? m_cmds[index] : 0;
}

/////////////////////////////////////////////////////////////////////////////
// CKeyPage メッセージ ハンドラ

BOOL CKeyPage::OnInitDialog(LPARAM lParam)
{
	m_keylist = GetDlgItem(IDC_KEYLIST);
	m_cmdlist = GetDlgItem(IDC_CMDLIST);

	// 制御キーメージリスト
	m_himCtrl = lccLoadImageList(RESINST, RT_RCDATA, _MKIR(IDB_KEYCTRL), 24, RGB(255, 0, 255));
	int cx, cy;
	ImageList_GetIconSize(m_himCtrl, &cx, &cy);
	m_sizeCtrl = CSize(cx, cy);

	// ツールバーイメージリスト
	m_mapTool.AddImageList(IDB_TOOLBAR);
	m_mapTool.AddImageList(IDB_SORTBAR);
	m_mapTool.AddImageList(IDB_PROPBAR);
	// Shiftキーの利用
	GetDlgItem(IDC_USESHIFT).EnableWindow(m_bUseShift || m_bUseShiftCtrl);

	// コマンドリスト列挙
	CString str;
	int i;
	for (i = 0; i < ARRAYSIZE(gnKeyMapList); i++) {
		GetCommandLabel(gnKeyMapList[i], str);
		m_cmdlist.AddString(str);
	}

	m_keylist.ResetContent();
	m_keylist.SetRedraw(FALSE);
	for (i = 0; i < m_numCmds; i++) {
		int n = m_keylist.AddString((LPCTSTR)DUMMY_VAL);
		m_keylist.SetItemData(n, (DWORD_PTR)i);
	}
	m_keylist.SetRedraw(TRUE);
	m_keylist.Invalidate();
	m_keylist.SetCurSel(0);
	SelectCommand();

	OnCtrlCheck();

	return TRUE;
}

void CKeyPage::OnMeasureItem(int idCtl, LPMEASUREITEMSTRUCT pmi)
{
	pmi->itemHeight = max((UINT)m_sizeKey.cy, pmi->itemHeight);
	pmi->itemHeight += pmi->itemHeight >> 2;
}

void CKeyPage::DrawKeyList(LPDRAWITEMSTRUCT pdi)
{
	CDCHandle dc = pdi->hDC;

	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		UINT cmd = GetCommand((int)(UINT)pdi->itemData);

		COLORREF clrFore, clrBack;
		if (pdi->itemState & ODS_SELECTED) {
			clrFore = GetSysColor(COLOR_HIGHLIGHTTEXT);
			clrBack = GetSysColor(COLOR_HIGHLIGHT);
		} else {
			clrFore = GetSysColor(COLOR_WINDOWTEXT);
			clrBack = GetSysColor(COLOR_WINDOW);
			if (cmd > 0)
				clrBack = mix_color(GetSysColor(COLOR_HIGHLIGHT), clrBack, 5);
		}
		dc.SetTextColor(clrFore);
		dc.SetBkColor(clrBack);
		CRect rc = pdi->rcItem;
		dc.ExtTextOut(rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc, NULL, 0, NULL);
		dc.SetBkMode(TRANSPARENT);

		rc.left += 2;
		if ((m_bUseShift || m_bUseShiftCtrl) && m_bShift) {
			ImageList_Draw(m_himCtrl, 1, dc, rc.left, (rc.top + rc.bottom - m_sizeCtrl.cy) / 2, ILD_NORMAL);
			rc.left += m_sizeCtrl.cx + 2;
		}
		if (m_bCtrl) {
			ImageList_Draw(m_himCtrl, 0, dc, rc.left, (rc.top + rc.bottom - m_sizeCtrl.cy) / 2, ILD_NORMAL);
			rc.left += m_sizeCtrl.cx + 2;
		}
		ImageList_Draw(m_himKey, (int)pdi->itemData + m_iStartImage, dc, rc.left, (rc.top + rc.bottom - m_sizeKey.cy) / 2, ILD_NORMAL);
		rc.left += m_sizeKey.cx + 6;

		if (cmd > 0) {
			int index = m_mapTool.FindCommand(cmd);
			CSize size = m_mapTool.GetImageSize();
			m_mapTool.Draw(dc, index, rc.left, (rc.top + rc.bottom - size.cy) / 2, ILD_NORMAL);

			rc.left += size.cx + 3;
			CString str;
			GetCommandLabel(cmd, str);
			dc.DrawText(str, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
		}
	}
	if (pdi->itemAction & ODA_FOCUS)
		dc.DrawFocusRect(&pdi->rcItem);
}

void CKeyPage::DrawCmdList(LPDRAWITEMSTRUCT pdi)
{
	CDCHandle dc = pdi->hDC;

	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		UINT cmd = gnKeyMapList[pdi->itemID];
		COLORREF clrFore, clrBack;
		if (pdi->itemState & ODS_SELECTED) {
			clrFore = GetSysColor(COLOR_HIGHLIGHTTEXT);
			clrBack = GetSysColor(COLOR_HIGHLIGHT);
		} else {
			clrFore = GetSysColor(COLOR_WINDOWTEXT);
			clrBack = GetSysColor(COLOR_WINDOW);
		}
		dc.SetTextColor(clrFore);
		dc.SetBkColor(clrBack);
		CRect rc = pdi->rcItem;
		dc.ExtTextOut(rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc, NULL, 0, NULL);
		dc.SetBkMode(TRANSPARENT);

		rc.left += 2;
		CSize size(20, 0);
		if (cmd > 0) {
			int index = m_mapTool.FindCommand(cmd);
			size = m_mapTool.GetImageSize();
			(index >= 0) && m_mapTool.Draw(dc, index, rc.left, (rc.top + rc.bottom - size.cy) / 2, ILD_NORMAL);
		}
		rc.left += size.cx + 8;

		CListBox list = GetDlgItem(IDC_CMDLIST);
		CString str;
		list.GetText(pdi->itemID, str);
		dc.DrawText(str, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
	}
	if (pdi->itemAction & ODA_FOCUS)
		dc.DrawFocusRect(&pdi->rcItem);
}

void CKeyPage::OnDrawItem(int idCtl, LPDRAWITEMSTRUCT pdi)
{
	if (idCtl == IDC_KEYLIST)
		DrawKeyList(pdi);
	else
		DrawCmdList(pdi);
}

void CKeyPage::OnSelChangeKeyList()
{
	SelectCommand();
}

void CKeyPage::OnApplyCommand()
{
	int index = m_keylist.GetCurSel();
	if (index != LB_ERR) {
		int sel = m_cmdlist.GetCurSel();
		if (sel != LB_ERR) {
			int key = index;
			if (!m_bUseShift) {
				if (m_bCtrl) {
					key += m_numCmds;
					if (m_bUseShiftCtrl && m_bShift)
						key += m_numCmds;
				}
			} else {
				if (m_bShift)	key += m_numCmds;
				if (m_bCtrl)	key += m_numCmds * 2;
			}
			m_cmds[key] = gnKeyMapList[sel];

			CRect rc;
			m_keylist.GetItemRect(index, &rc);
			m_keylist.InvalidateRect(&rc, TRUE);
			m_bUpdate = true;
		}
	}
}

void CKeyPage::OnReleaseCommand()
{
	int index = m_keylist.GetCurSel();
	if (index != LB_ERR) {
		int key = index;
		if (!m_bUseShift) {
			if (m_bCtrl) {
				key += m_numCmds;
				if (m_bUseShiftCtrl && m_bShift)
					key += m_numCmds;
			}
		} else {
			if (m_bShift)	key += m_numCmds;
			if (m_bCtrl)	key += m_numCmds * 2;
		}
		m_cmds[key] = 0;			// 未使用にする

		CRect rc;
		m_keylist.GetItemRect(index, &rc);
		m_keylist.InvalidateRect(&rc, TRUE);
		m_bUpdate = true;
	}
}

void CKeyPage::OnShiftCheck()
{
	m_bShift = (m_bUseShift || m_bUseShiftCtrl) ? (IsDlgButtonChecked(IDC_USESHIFT) != FALSE) : false;
	m_keylist.RedrawWindow();
	SelectCommand();
}

void CKeyPage::OnCtrlCheck()
{
	m_bCtrl = (IsDlgButtonChecked(IDC_USECTRL) != FALSE);
	if (m_bUseShiftCtrl) {
		if (!m_bCtrl) {
			m_bShift = false;
			CheckDlgButton(IDC_USESHIFT, FALSE);
		}
		GetDlgItem(IDC_USESHIFT).EnableWindow(m_bCtrl);
	}
	m_keylist.RedrawWindow();
	SelectCommand();
}

UINT CKeyPage::GetCommand(int index)
{
	if (!m_bUseShift) {
		if (m_bCtrl) {
			index += m_numCmds;
			if (m_bUseShiftCtrl && m_bShift)
				index += m_numCmds;
		}
	} else {
		if (m_bShift)	index += m_numCmds;
		if (m_bCtrl)	index += m_numCmds * 2;
	}
	return m_cmds[index];
}

void CKeyPage::SelectCommand()
{
	int sel = m_keylist.GetCurSel();
	if (sel == LB_ERR) return;

	UINT cmd = GetCommand(sel);
	for (int i = 0; i < sizeof(gnKeyMapList) / sizeof(UINT); i++) {
		if (gnKeyMapList[i] == cmd) {
			m_cmdlist.SetCurSel(i);
			return;
		}
	}
	m_cmdlist.SetCurSel(0);
}

/////////////////////////////////////////////////////////////////////////////
// CKeySheet

void CKeySheet::Init()
{
	if (m_bInitCompleted)
		return;

	CenterWindow();

	CTabCtrl tab = GetTabControl();
	m_himTab = lccLoadImageList(RESINST, RT_RCDATA, _MKIR(IDB_KEYBOARD), 20, RGB(255, 0, 255));
	tab.SetImageList(m_himTab);

	int i, images[] = { 10, 0, 36 };
	CSimpleArray<CString> arr;
	int cnt = LoadTokenizedString(IDS_FOLDER_KEYMAP, _T(";"), arr);
	TC_ITEM tci;
	tci.mask = TCIF_TEXT | TCIF_IMAGE;
	for (i = 0; i < cnt; i++) {
		tci.pszText = (TCHAR*)arr[i].GetString();
		tci.iImage = images[i];
		tab.SetItem(i, &tci);
	}
	m_bInitCompleted = TRUE;
}


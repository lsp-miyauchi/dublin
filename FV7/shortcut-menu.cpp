////////////////////////////////////////////////////////////////////////
// shortcut-menu.cpp : ショートカットメニュー
//

#include "stdafx.h"
#include "fv7.h"
#include "shortcut-menu.h"

extern UINT gnKeyMapList[];		// keymap.cppで定義
extern int gnKeyMapListSize;	// 〃

/////////////////////////////////////////////////////////////////////////////
// CShortcutPage

CShortcutMenuPage::CShortcutMenuPage()
{
	m_update = FALSE;
	m_himh = NULL;
}

CShortcutMenuPage::~CShortcutMenuPage()
{
	if (m_himh) ImageList_Destroy(m_himh);
}

void CShortcutMenuPage::SetCommands(CShortcutList& scut)
{
	scut.Load();
	m_menuid = scut.m_arrCommand;
}

void CShortcutMenuPage::GetCommands(CShortcutList& scut)
{
	scut.Replace(m_menuid);
}

BOOL CShortcutMenuPage::OnInitDialog(LPARAM lParam)
{
	m_cmd = GetDlgItem(IDC_CMDLIST);
	m_menu = GetDlgItem(IDC_MENULIST);

	CPropertySheetWindow sheet = GetPropertySheet();
	sheet.GetDlgItem(ID_APPLY_NOW).ShowWindow(SW_HIDE);
	CTabCtrl tab = sheet.GetTabControl();

	int i;

	CString str;
	HIMAGELIST him = tab.GetImageList();
	if (!him) {
		m_himh = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_CUSTMENU), 20, RGB(255, 0, 255));
		tab.SetImageList(m_himh);

		str.LoadString(IDS_TAB_SHORTCUTMENU);
		LPTSTR p = (LPTSTR)(LPCTSTR)str;
		TCITEM tci;
		for (i = 0; i < 2; i++) {
			LPTSTR q = StrChr(p, '|');
			if (q)
				*q++ = 0;
			tci.mask = TCIF_IMAGE | TCIF_TEXT;
			tci.iImage = i;
			tci.pszText = p;
			tab.SetItem(i, &tci);
			p = q;
		}
	}

	// ツールバーイメージリスト
	m_mapTool.AddImageList(IDB_TOOLBAR);
	m_menu.SetImageList(m_mapTool, LVSIL_SMALL);
	m_cmd.SetImageList(m_mapTool, LVSIL_SMALL);

	CRect rc;
	m_menu.GetClientRect(&rc);
	m_menu.InsertColumn(0, NULL, LVCFMT_LEFT, rc.Width() - GetSystemMetrics(SM_CXVSCROLL), 0);
	m_cmd.GetClientRect(&rc);
	m_cmd.InsertColumn(0, NULL, LVCFMT_LEFT, rc.Width() - GetSystemMetrics(SM_CXVSCROLL), 0);

	for (i = 0; i < gnKeyMapListSize; i++) {
		if (gnKeyMapList[i])
			InsertItem(&m_cmd, m_cmd.GetItemCount(), gnKeyMapList[i]);
	}
	m_cmd.SelectItem(0);

	int cnt = m_menuid.GetSize();
	for (i = 0; i < cnt; i++)
		InsertItem(&m_menu, m_menu.GetItemCount(), m_menuid[i]);
	m_menu.SelectItem(0);

	// Marlette
	m_marlett.CreatePointFont(90, _T("Marlett"));
	GetDlgItem(IDC_MOVEUP).SetFont(m_marlett);		GetDlgItem(IDC_MOVEUP).SetWindowText(_T("t"));
	GetDlgItem(IDC_MOVEDOWN).SetFont(m_marlett);	GetDlgItem(IDC_MOVEDOWN).SetWindowText(_T("u"));


	return TRUE;
}

BOOL CShortcutMenuPage::OnApply()
{
	if (m_update) {
		m_menuid.RemoveAll();
		int cnt = m_menu.GetItemCount();
		if (cnt > 0) {
			for (int i = 0; i < cnt; i++)
				m_menuid.Add((UINT)m_menu.GetItemData(i));
		}
	}
	return TRUE;
}

void CShortcutMenuPage::OnAdd()
{
	if (m_menu.GetItemCount() >= MAXSHORTCUT) {
		AppMessage(IDS_MAXSHORTCUT);
		return;
	}

	int index = m_cmd.GetNextItem(-1, LVNI_SELECTED);
	if (index < 0) {
		MessageBeep(0);
		return;
	}

	UINT id = (UINT)m_cmd.GetItemData(index);
	index = m_menu.GetNextItem(-1, LVNI_SELECTED);
	if (index < 0)
		index = 0;
	index = InsertItem(&m_menu, index, id);
	m_menu.SelectItem(index);
	m_update = TRUE;
}

void CShortcutMenuPage::OnDown()
{
	int sel = m_menu.GetNextItem(-1, LVNI_SELECTED);
	int cnt = m_menu.GetItemCount();
	if (sel < 0 || sel == cnt - 1) {
		MessageBeep(0);
		return;
	}

	UINT id = (UINT)m_menu.GetItemData(sel);
	m_menu.DeleteItem(sel);
	sel++;
	sel = InsertItem(&m_menu, sel, id);
	m_menu.SelectItem(sel);
	m_update = TRUE;
}

void CShortcutMenuPage::OnUp()
{
	int sel = m_menu.GetNextItem(-1, LVNI_SELECTED);
	if (sel < 1) {
		MessageBeep(0);
		return;
	}

	UINT id = (UINT)m_menu.GetItemData(sel);
	m_menu.DeleteItem(sel);
	sel--;
	sel = InsertItem(&m_menu, sel, id);
	m_menu.SelectItem(sel);
	m_update = TRUE;
}

void CShortcutMenuPage::OnRemove()
{
	int sel = m_menu.GetNextItem(-1, LVNI_SELECTED);
	if (sel < 0) {
		MessageBeep(0);
		return;
	}

	m_menu.DeleteItem(sel);
	int cnt = m_menu.GetItemCount();
	sel = min(max(cnt - 1, 0), sel);
	m_menu.SelectItem(sel);
	m_update = TRUE;
}

void CShortcutMenuPage::OnNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult)
{
	LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pNmhdr;
	*pResult = TRUE;

	BOOL vkeyControl = _ISKEYDOWN(VK_CONTROL);
	switch (plvkd->wVKey) {
	case VK_DELETE:
		OnRemove();
		return;

	case VK_UP:
		if (vkeyControl) {
			OnUp();
			return;
		}
		break;

	case VK_DOWN:
		if (vkeyControl) {
			OnDown();
			return;
		}
		break;

	default:
		break;
	}
	*pResult = FALSE;
}

void CShortcutMenuPage::OnSeparator()
{
	int cnt = m_menu.GetItemCount();
	if (cnt >= MAXSHORTCUT) {
		AppMessage(IDS_MAXSHORTCUT);
		return;
	}

	int sel = m_menu.GetNextItem(-1, LVNI_SELECTED);
	if (sel < 0)
		sel = 0;
	sel = InsertItem(&m_menu, sel, ID_SEPARATOR);
	m_menu.SelectItem(sel);
	m_update = TRUE;
}

int CShortcutMenuPage::InsertItem(CListViewCtrl* pList, int index, UINT menuid)
{
	int iImage;
	CString str;
	if (menuid != ID_SEPARATOR) {
		GetCommandLabel(menuid, str);
		iImage = m_mapTool.FindCommand(menuid);
	}
	else {
		str = _T("------------------------");
		iImage = -1;
	}

	index = pList->InsertItem(index, str, iImage);
	pList->SetItemData(index, menuid);
	return index;
}




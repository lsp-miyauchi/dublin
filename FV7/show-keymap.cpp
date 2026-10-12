////////////////////////////////////////////////////////////////////////
// show-keymap.cpp : ショートカットキーの表示
//

#include "stdafx.h"
#include "fv7.h"
#include "show-keymap.h"

/////////////////////////////////////////////////////////////////////////////
//

CShowKeymapWnd::CShowKeymapWnd()
{
	SetThemeClassList(L"Window");
}

CShowKeymapWnd::~CShowKeymapWnd()
{
}

int CShowKeymapWnd::_InsertMenuItems(HMENU hmenu, int idCmdStart, UINT* cmdTable, int iStart, int maxEntry)
{
	int idCmd = idCmdStart;

	CKeymapMenuItem* item;
	UINT flags;
	int i;
	for (i = 0; i < maxEntry; i++) {
		flags = 0;
		if ((::GetMenuItemCount(hmenu) > 0 && i == 0) || (i > 0 && (i % 13) == 0))
			flags |= MF_MENUBARBREAK;
		if (*cmdTable == 0)
			flags |= MF_GRAYED;

		item = new CKeymapMenuItem();
		GetCommandLabel(*cmdTable, item->m_str);
		item->m_cmd = *cmdTable++;
		m_array.Add(idCmd, item);

		::AppendMenu(hmenu, flags, idCmd, item->m_str);
		SetBitmapToMenuItem(hmenu, idCmd, m_hbmArray[i + iStart]);
		idCmd++;
	}
	return (idCmd - idCmdStart);
}

int CShowKeymapWnd::TrackKeymapMenu(HWND hParent)
{
	if (!Create(hParent, NULL, NULL, WS_POPUP)) return 0;

	CRect rc;
	::GetWindowRect(hParent, &rc);
	CPoint pt((rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2);

	// メニュー項目用ビットマップ作成
	CImageList il = lccLoadImageList(RESINST, RT_RCDATA, _MKIR(IDB_KEYBOARD), 20, RGB(255, 0, 255));
	int numImages = il.GetImageCount();
	for (int i = 0; i < numImages; i++) {
		m_hbmArray.Add(CreateBitmapFromImageList(il, i));
	}

	CMenu pop(::CreatePopupMenu());
	m_array.Add(0, NULL);				// 0 for no-command
	int idCmd = IDM_SHORTCUTKEY_FIRST;
	idCmd += _InsertMenuItems(pop, idCmd, fvApp.keymapAbc, 10, 26);
	idCmd += _InsertMenuItems(pop, idCmd, fvApp.keymapFnc, 36, 12);
	idCmd += _InsertMenuItems(pop, idCmd, fvApp.keymapNum, 0, 10);
	idCmd = pop.TrackPopupMenu(TPM_CENTERALIGN | TPM_VCENTERALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	pop.DestroyMenu();

	while (m_hbmArray.GetSize() > 0) {
		::DeleteObject(m_hbmArray[0]);
		m_hbmArray.RemoveAt(0);
	}

	int cmd = 0;
	while (m_array.GetSize() > 0) {
		CKeymapMenuItem* item = m_array.GetValueAt(0);
		if (item) {
			int key = m_array.GetKeyAt(0);
			if (key == idCmd) {
				cmd = item->m_cmd;
			}
			delete item;
		}
		m_array.RemoveAt(0);
	}

	DestroyWindow();
	return cmd;
}

LRESULT CShowKeymapWnd::OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu)
{
	int id = 0;
	int idCmd = 0;
	if (ISALPHA(nChar)) {
		id = ToUpper((TCHAR)nChar) - 'A';
		idCmd = fvApp.keymapAbc[id];
	}
	else if (ISDIGIT(nChar)) {
		id = (nChar - '0');
		idCmd = fvApp.keymapNum[id];
		id += 26 + 12;
	}
	if (idCmd > 0)
		return MAKELPARAM(id, MNC_EXECUTE);

	return DefWindowProc(WM_MENUCHAR, MAKELONG(nChar, nFlags), (LPARAM)hMenu);
}



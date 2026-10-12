#pragma once

#ifndef __SHORTCUT_MENU_H__
#define __SHORTCUT_MENU_H__

#include "multiple_imagelist.h"

/////////////////////////////////////////////////////////////////////////////
// CShortcutMenuPage

class CShortcutMenuPage : public CPropertyPageImpl<CShortcutMenuPage>
{
public:
	enum { IDD = IDD_SHORTCUTMENU };
	CSimpleArray<UINT> m_menuid;
	CListViewCtrl m_cmd;
	CListViewCtrl m_menu;
	BOOL m_update;
	HIMAGELIST m_himh;
	MultipleImageList m_mapTool;
	CFont m_marlett;

	CShortcutMenuPage();
	~CShortcutMenuPage();

	BEGIN_MSG_MAP(CShortcutMenuPage)
		ON_WM_INITDIALOG()
		ON_WM_HELPINFO()
		ON_NOTIFY(LVN_KEYDOWN, IDC_MENULIST, OnNotifyKeyDown)
		ON_BN_CLICKED(IDC_ADD, OnAdd)
		ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
		ON_BN_CLICKED(IDC_MOVEDOWN, OnDown)
		ON_BN_CLICKED(IDC_MOVEUP, OnUp)
		ON_BN_CLICKED(IDC_REMOVE, OnRemove)
		ON_BN_CLICKED(IDC_SPACE, OnSeparator)
		CHAIN_MSG_MAP(CPropertyPageImpl<CShortcutMenuPage>)
	END_MSG_MAP()

	BOOL OnApply();

public:
	void SetCommands(CShortcutList& scut);
	void GetCommands(CShortcutList& scut);

protected:
	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pHelpInfo) { fvApp.HelpTopic(NULL, HB_DLG, IDD); return TRUE; }
	void OnNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult);
	void OnAdd();
	void OnHelpCall() { };
	void OnDown();
	void OnUp();
	void OnRemove();
	void OnSeparator();

	int InsertItem(CListViewCtrl* pList, int index, UINT menuid);
};

#endif		// __SHORTCUT_MENU_H__

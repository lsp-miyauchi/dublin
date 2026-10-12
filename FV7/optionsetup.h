////////////////////////////////////////////////////////////////////////
// optionsetup.h

#pragma once
#ifndef __OPTIONSETUP_H__
#define __OPTIONSETUP_H__

#include "pagedialog.h"
#include "mapicon.h"
#include "addin.h"
#include "hotkeymenu.h"

/////////////////////////////////////////////////////////////////////////////
// CStartPageDlg ダイアログ

class CStartPageDlg : public CDialogImpl<CStartPageDlg>, public CPageDialogHandler
{
private:
	CComboBox m_startmode;

public:
	enum { IDD = IDD_OP_START };

BEGIN_MSG_MAP(CStartPageDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDC_OPCURFOLDER, OnSetCurrentFolder)
	ON_CBN_SELCHANGE(IDC_OPSTARTMODE, OnSelChangeStartMode)
	ON_NOTIFY(SEN_GETTEXT, IDC_OPTABGROUP, OnNotifyBrowseTabGroup)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnSetCurrentFolder();
	void OnSelChangeStartMode();
	void OnNotifyBrowseTabGroup(NMHDR* phdr, LRESULT* pResult);
};

/////////////////////////////////////////////////////////////////////////////
// CResidentPageDlg ダイアログ

class CResidentPageDlg : public CDialogImpl<CResidentPageDlg>, public CPageDialogHandler
{
private:
	DWORD m_tsractivekey;
	DWORD m_tsrpopupkey;
	CHotKeyCtrl m_active;
	CHotKeyCtrl m_popup;

public:
	enum { IDD = IDD_OP_RESIDENT };

	CResidentPageDlg() : m_tsractivekey(0), m_tsrpopupkey(0)
	{
	}

BEGIN_MSG_MAP(CResidentPageDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDC_OPTSR, OnTsr)
	ON_EN_CHANGE(IDC_ACTIVEKEY, OnChangeActiveHotKey)
	ON_EN_CHANGE(IDC_POPUPKEY, OnChangePopupHotKey)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnTsr();
	void OnChangeActiveHotKey();
	void OnChangePopupHotKey();
	BOOL CheckRegisterKey(DWORD key);
};

/////////////////////////////////////////////////////////////////////////////
// CDispPageDlg ダイアログ

class CDispPageDlg : public CDialogImpl<CDispPageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_DISPLAY };
	DWORD m_dwShellFlags;
	CCheckTreeViewCtrl m_tree;

	CDispPageDlg() : m_dwShellFlags(0)
	{
	}

BEGIN_MSG_MAP(CDispPageDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(CTN_CHECKCHANGED, IDC_OPCHECKTREE, OnNotifyCheckChanged)
	ON_BN_CLICKED(IDC_OPRESET_FOLDER, OnResetFolder)
END_MSG_MAP()

	BOOL OnApply();
	void OnResetFolder();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnNotifyCheckChanged(NMHDR* phdr, LRESULT* pResult);
};


/////////////////////////////////////////////////////////////////////////////
// CFopPageDlg ダイアログ

class CFopPageDlg : public CDialogImpl<CFopPageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_FILE };
	CComboBox	m_senter;
	CComboBox	m_matchtype;

BEGIN_MSG_MAP(CFopPageDlg)
	ON_WM_INITDIALOG()
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////
// CMousePageDlg ダイアログ

class CMousePageDlg : public CDialogImpl<CMousePageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_MOUSE };
	CComboBox	m_lbtn;
	CComboBox	m_rbtn;
	CComboBox	m_cbtn;
	CComboBox	m_fbtn;

BEGIN_MSG_MAP(CFopPageDlg)
	ON_WM_INITDIALOG()
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////
// CColorPage ダイアログ

class CColorPageDlg : public CDialogImpl<CColorPageDlg>, public CPageDialogHandler
{
private:
	CTreeViewCtrl m_fontlist;
	CTreeViewCtrl m_colorlist;
	CComboBox m_fontcombo;
	CComboBox m_sizecombo;

public:
	enum { IDD = IDD_OP_FONTS };

BEGIN_MSG_MAP(CColorPageDlg)
	ON_WM_INITDIALOG()
	ON_WM_DELETEITEM()
	ON_BN_CLICKED(IDC_TREE_LINE, OnUseTreeLine)
	ON_NOTIFY(TVN_SELCHANGED, IDC_FONT_LIST, OnNotifyFontListSelChanged)
	ON_NOTIFY(TVN_DELETEITEM, IDC_FONT_LIST, OnNotifyFontListDeleteItem)
	ON_NOTIFY(TVN_SELCHANGED, IDC_COLOR_LIST, OnNotifyColorListSelChanged)
	ON_NOTIFY(TVN_DELETEITEM, IDC_COLOR_LIST, OnNotifyColorListDeleteItem)
	ON_CBN_SELCHANGE(IDC_FONT_NAME, OnSelChangeFontName)
	ON_CBN_SELCHANGE(IDC_FONT_SIZE, OnSelChangeFontSize)
	ON_CBN_EDITCHANGE(IDC_FONT_SIZE, OnEditChangeFontSize)
	ON_CBN_SELCHANGE(IDC_COLOR_FORE, OnSelChangeForeColor)
	ON_CBN_SELCHANGE(IDC_COLOR_BACK, OnSelChangeBackColor)
	ON_BN_CLICKED(IDC_FONT_BOLD, OnFontBold)
	ON_BN_CLICKED(IDC_OPFONT_SAMEALL, OnFontSameAll)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

	static int CALLBACK FontCallback(const ENUMLOGFONT* pelf, const NEWTEXTMETRIC* pntm, DWORD fontType, LPARAM lParam)
	{
		CColorPageDlg* pThis = (CColorPageDlg*)lParam;
		return pThis->EnumComboFonts(pelf, pntm, fontType);
	}

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnUseTreeLine();
	void OnSelChangeFontName();
	void OnSelChangeFontSize();
	void OnFontBold();
	void OnFontSameAll();
	void OnEditChangeFontSize();
	void OnSelChangeForeColor();
	void OnSelChangeBackColor();
	void OnNotifyFontListSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyFontListDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyColorListSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyColorListDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	void OnDeleteItem(int idCtrl, DELETEITEMSTRUCT* pds);
 	int EnumComboFonts(const ENUMLOGFONT* pelf, const NEWTEXTMETRIC* pntm, DWORD fontType);
};

/////////////////////////////////////////////////////////////////////////////
// CCustomBarDlg ダイアログ

class CToolbarPageDlg : public CDialogImpl<CToolbarPageDlg>, public CPageDialogHandler
{
private:
	CListViewCtrl m_list;
	HIMAGELIST m_himChk;
	HIMAGELIST m_himBar;

public:
	CToolbarPageDlg();
	~CToolbarPageDlg();

	enum { IDD = IDD_OP_CUSTOMBAR };

BEGIN_MSG_MAP(CToolbarPageDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChangedList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST, OnEndLabelEditList)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST, OnBeginLabelEditList)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, OnKeydownList)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_CUSTOMIZE, OnCustomize)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNew();
	void OnCustomize();
	void OnRename();
	void OnEndLabelEditList(NMHDR* pNMHDR, LRESULT* pResult);
	void OnBeginLabelEditList(NMHDR* pNMHDR, LRESULT* pResult);
	void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	void OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult);
	void OnDelete();
};

/////////////////////////////////////////////////////////////////////////////
// CDrivePageDlg ダイアログ

class CDrivePageDlg : public CDialogImpl<CDrivePageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_DRIVE };
	CListViewCtrl m_list;
	CListViewCtrl m_net;
	CString m_csSecretDrives;

BEGIN_MSG_MAP(CDrivePageDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNotifyClick)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, OnNotifyKeyDown)
	ON_NOTIFY(LVN_KEYDOWN, IDC_NETCOM, OnNotifyCustomDriveKeyDown)
	ON_NOTIFY(LVN_DELETEITEM, IDC_NETCOM, OnNotifyCustomDriveDeleteItem)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_NETCOM, OnBeginLabelEditList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_NETCOM, OnEndLabelEditList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_CUSTICON, OnChangeIcon)
	ON_BN_CLICKED(IDC_OPMOBILEDEVICE, OnParamChanged)
	ON_BN_CLICKED(IDC_OPNETDEVICE, OnParamChanged)
	ON_CBN_SELCHANGE(IDC_DRIVE_LBTNDBLCLK, OnParamChanged)
	ON_CBN_SELCHANGE(IDC_DRIVE_MBTNDBLCLK, OnParamChanged)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnNotifyClick(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyKeyDown(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyCustomDriveKeyDown(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyCustomDriveDeleteItem(NMHDR* pNmhdr, LRESULT* pResult);
	void OnBeginLabelEditList(NMHDR* pNMHDR, LRESULT* pResult);
	void OnEndLabelEditList(NMHDR* pNMHDR, LRESULT* pResult);
	void OnAdd();
	void OnDelete();
	void OnChangeIcon();
	void OnParamChanged() { m_bUpdate = TRUE; }

	void ToggleMark(int nIndex);
	void ExchangeNetCom(int item1, int item2);

};

/////////////////////////////////////////////////////////////////////////////
// CProgramPageDlg ダイアログ

class CProgramPageDlg : public CDialogImpl<CProgramPageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_PROGRAM };

BEGIN_MSG_MAP(CProgramPage)
	ON_WM_INITDIALOG()
	ON_NOTIFY_CODE(SEN_GETTEXT, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnNotifyBrowse(int idCtrl, NMHDR* pNmhdr, LRESULT* pResult);

	BOOL Browse(LPTSTR psz, int cchMax);

};

/////////////////////////////////////////////////////////////////////////////
// CAddinToolsDlg ダイアログ

class CAddinPageDlg : public CDialogImpl<CAddinPageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_ADDINTOOLS };

	CTabCtrl m_tab;
	CListViewCtrl m_list;

	CSimpleMap<CString, CAddinTools*> m_mapViewer;
	CSimpleMap<CString, CAddinTools*> m_mapEditor;

BEGIN_MSG_MAP(CAddinPageDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnTabChanged)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, OnGetDispInfo)
	ON_BN_CLICKED(IDC_ADD, OnAddTools)
	ON_BN_CLICKED(IDC_DELETE, OnDeleteTools)
	ON_BN_CLICKED(IDC_EDIT, OnEditTools)
	ON_BN_CLICKED(IDC_DUPLICATE, OnDuplicateTools)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnTabChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult);

	void OnAddTools();
	void OnDeleteTools();
	void OnEditTools();
	void OnDuplicateTools();

	void SetActivePage(int page);
	int InsertItem(CAddinTools* pat);
};

/////////////////////////////////////////////////////////////////////////////
// CHotkeyPageDlg dialog

class CHotkeyPageDlg : public CDialogImpl<CHotkeyPageDlg>, public CPageDialogHandler
{
private:
	CHotKeyItem m_clip;

public:
	enum { IDD = IDD_OP_HOTKEY };

	CHotkeyPageDlg()
		: m_hImage(NULL)
		, m_hHeaderImage(NULL)
	{
	}

	~CHotkeyPageDlg()
	{
		if (m_hImage) ImageList_Destroy(m_hImage);
		if (m_hHeaderImage) ImageList_Destroy(m_hHeaderImage);
	}

	CListViewCtrl m_list;
	HIMAGELIST m_hImage;
	HIMAGELIST m_hHeaderImage;

BEGIN_MSG_MAP(CHotkeyPageDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDC_CONFIRM, OnConfirm)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_PASTE, OnPaste)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNotifyDblclklist)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, OnNotifyCustomDraw)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnNotifyItemChanged)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST, OnNotifyDeleteItem)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnOK();
	void OnCancel();
	void OnConfirm();
	void OnDelete();
	void OnCopy();
	void OnPaste();
	void OnNotifyDblclklist(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);

	void SetItemColumn(int index, CHotKeyItem* item);
};

/////////////////////////////////////////////////////////////////////////////
// CMapIconPageDlg dialog

class CMapIconPageDlg : public CDialogImpl<CMapIconPageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_MAPICON };

	CListViewCtrl m_list;

BEGIN_MSG_MAP(CMapIconPageDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST, OnNotifyDeleteItem)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	void OnAdd();
	void OnEdit();
	void OnDelete();

	void Listup();
};

/////////////////////////////////////////////////////////////////////////////
// CTabPageDlg ダイアログ

class CTabPageDlg : public CDialogImpl<CTabPageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_TAB };

BEGIN_MSG_MAP(CTabPageDlg)
	ON_WM_INITDIALOG()
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
};


/////////////////////////////////////////////////////////////////////////////
// CPrivacyPageDlg ダイアログ

class CPrivacyPageDlg : public CDialogImpl<CPrivacyPageDlg>, public CPageDialogHandler
{
public:
	enum { IDD = IDD_OP_PRIVACY };

BEGIN_MSG_MAP(CPrivacyPageDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
END_MSG_MAP()

	BOOL OnApply();
	void OnHelpInfo() { fvApp.HelpTopic(m_hWnd, HB_DLG, IDD); }

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnClear();
};


/////////////////////////////////////////////////////////////////////////////
// COptionSetup dialog

class COptionSetupDlg : public CDialogImpl<COptionSetupDlg>
{
private:
	CStartPageDlg m_pageStart;
	CResidentPageDlg m_pageResident;
	CDispPageDlg m_pageDisp;
	CFopPageDlg m_pageFop;
	CMousePageDlg m_pageMouse;
	CColorPageDlg m_pageColor;
	CToolbarPageDlg m_pageTBar;
	CDrivePageDlg m_pageDrive;
	CTabPageDlg m_pageTab;
	CProgramPageDlg m_pageProgram;
	CAddinPageDlg m_pageAddin;
	CHotkeyPageDlg m_pageHotkey;
	CMapIconPageDlg m_pageMapIcon;
	CPrivacyPageDlg m_pagePrivacy;

	CRect m_rcPane;

	CSimpleArray<CPageDialog*> m_pageArray;
	CSimpleMap<UINT, HTREEITEM> m_mapIDD;

public:
	bool m_bDisplaySetting;
	bool m_bDriveSetting;
	bool m_bAccelSetting;
	bool m_bTabSetting;

	COptionSetupDlg()
		: m_bDisplaySetting(false)
		, m_bDriveSetting(false)
		, m_bAccelSetting(false)
		, m_bTabSetting(false)
	{
	}

	enum { IDD = IDD_OPTION };

	CTreeViewCtrl m_menulist;

BEGIN_MSG_MAP(COptionSetupDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnNotifyMenuListSelChanged)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE, OnNotifyMenuListDeleteItem)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpInfo)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();

		GetDlgItem(IDC_PANE).GetWindowRect(&m_rcPane);
		ScreenToClient(&m_rcPane);
		GetDlgItem(IDC_PANE).DestroyWindow();

		m_menulist = GetDlgItem(IDC_TREE);
		m_menulist.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
		m_menulist.SetItemHeight(GetSystemMetrics(SM_CYSMICON) + 4);

		HTREEITEM hItem;
		AddPage(m_pageStart.IDD, NULL, &m_pageStart);
		AddPage(m_pageResident.IDD, NULL, &m_pageResident);
		hItem = AddPage(m_pageDisp.IDD, NULL, &m_pageDisp);
			AddPage(m_pageColor.IDD, NULL, &m_pageColor, hItem);
			AddPage(m_pageMapIcon.IDD, NULL, &m_pageMapIcon, hItem);
			m_menulist.Expand(hItem, TVE_EXPAND);
		AddPage(m_pageFop.IDD, NULL, &m_pageFop);
		AddPage(m_pageMouse.IDD, NULL, &m_pageMouse);
		AddPage(m_pageTBar.IDD, NULL, &m_pageTBar);
		AddPage(m_pageDrive.IDD, NULL, &m_pageDrive);
		AddPage(m_pageTab.IDD, NULL, &m_pageTab);
		hItem = AddPage(m_pageProgram.IDD, NULL, &m_pageProgram);
			AddPage(m_pageAddin.IDD, NULL, &m_pageAddin, hItem);
			AddPage(m_pageHotkey.IDD, NULL, &m_pageHotkey, hItem);
			m_menulist.Expand(hItem, TVE_EXPAND);
		AddPage(m_pagePrivacy.IDD, NULL, &m_pagePrivacy);

		hItem = NULL;
		int idd = 0;
		if (lParam != NULL) {
			// 指定のページ
			idd = (int)lParam;
		}
		else {
			// 前回のページ
			idd = GetRegValue(IDS_SEC_SETTINGS, IDS_KEY_LASTOPTIONPAGE, 0);
		}
		if (idd)
			hItem = m_mapIDD.Lookup(idd);
		if (!hItem)
			hItem = m_menulist.GetRootItem();
		m_menulist.SelectItem(hItem);

		return TRUE;
	}

	HTREEITEM AddPage(UINT idd, LPCTSTR pszTitle, CPageDialogHandler* handler, HTREEITEM hParentItem = TVI_ROOT)
	{
		CPageDialog* pp = new CPageDialog(idd, handler);
		CString str;
		if (pszTitle && *pszTitle)
			str = pszTitle;
		else
			LoadPageDialogCaption(idd, str);
		HTREEITEM hItem = m_menulist.InsertItem(str, hParentItem, TVI_LAST);
		m_menulist.SetItemData(hItem, (LPARAM)pp);
		m_mapIDD.Add(idd, hItem);
		m_pageArray.Add(pp);
		return hItem;
	}

	void OnNotifyMenuListSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
	{
		*pResult = 0;
		NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;

		CPageDialog* ppOld = (CPageDialog*)m_menulist.GetItemData(pnmtv->itemOld.hItem);
		CPageDialog* ppNew = (CPageDialog*)m_menulist.GetItemData(pnmtv->itemNew.hItem);
		if (ppOld && ppOld->IsWindow() && ppOld->IsWindowVisible())
			ppOld->ShowWindow(SW_HIDE);

		if (ppNew) {
			if (!ppNew->IsWindow()) {
				HWND hwnd = NULL;
				if      (ppNew->m_idd == m_pageStart.IDD)		hwnd = m_pageStart.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageResident.IDD)	hwnd = m_pageResident.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageDisp.IDD)		hwnd = m_pageDisp.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageFop.IDD)			hwnd = m_pageFop.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageMouse.IDD)		hwnd = m_pageMouse.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageColor.IDD)		hwnd = m_pageColor.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageTBar.IDD)		hwnd = m_pageTBar.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageDrive.IDD)		hwnd = m_pageDrive.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageTab.IDD)			hwnd = m_pageTab.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageProgram.IDD)		hwnd = m_pageProgram.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageAddin.IDD)		hwnd = m_pageAddin.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageHotkey.IDD)		hwnd = m_pageHotkey.Create(m_hWnd);
				else if (ppNew->m_idd == m_pageMapIcon.IDD)		hwnd = m_pageMapIcon.Create(m_hWnd);
				else if (ppNew->m_idd == m_pagePrivacy.IDD)		hwnd = m_pagePrivacy.Create(m_hWnd);

				if (hwnd) {
					ppNew->Attach(hwnd);
					ppNew->SetWindowPos(::GetNextWindow(GetDlgItem(IDOK), GW_HWNDPREV), m_rcPane.left, m_rcPane.top, 0, 0, SWP_NOSIZE);
				}
			}
			if (!ppNew->IsWindowVisible())
				ppNew->ShowWindow(SW_SHOW);
		}
	}

	void OnNotifyMenuListDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
	{
		*pResult = 0;
		NMTREEVIEW* pnmtv = (NMTREEVIEW*)pNMHDR;
		CPageDialog* pp = (CPageDialog*)m_menulist.GetItemData(pnmtv->itemOld.hItem);
		if (pp)
			delete pp;
	}

	void OnOK()
	{
		DWORD nListStatOld = gnListStat;

		int cnt = m_pageArray.GetSize();
		for (int i = 0; i < cnt; i++) {
			CPageDialog* pdlg = m_pageArray[i];
			if (pdlg->GetHandler()) {
				if (!pdlg->GetHandler()->OnApply()) {
					HTREEITEM hItem = m_mapIDD.Lookup(pdlg->m_idd);
					if (hItem)
						m_menulist.SelectItem(hItem);
					return;
				}
			}
		}

		m_bDisplaySetting = (m_pageDisp.isChanged() || m_pageMouse.isChanged() || m_pageColor.isChanged() || m_pageFop.isChanged() || m_pageMapIcon.isChanged());
		m_bDriveSetting = (m_pageDrive.isChanged() != FALSE);
		m_bAccelSetting = (m_pageHotkey.isChanged() || ((gnListStat ^ nListStatOld) & LTS_MENUACCEL));
		m_bTabSetting = (m_pageTab.isChanged() != FALSE);

		// 現在選択中のページ
		CPageDialog* ppd = (CPageDialog*)m_menulist.GetItemData(m_menulist.GetSelectedItem());
		if (ppd)
			SetRegValue(IDS_SEC_SETTINGS, IDS_KEY_LASTOPTIONPAGE, ppd->m_idd);

		EndDialog(IDOK);
	}

	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}

	void OnHelpInfo()
	{
		HTREEITEM hItem = m_menulist.GetSelectedItem();
		if (hItem) {
			CPageDialog* pp = (CPageDialog*)m_menulist.GetItemData(hItem);
			if (pp)
				pp->GetHandler()->OnHelpInfo();
		}
	}

	int LoadPageDialogCaption(UINT idd, CString& str)
	{
		// ダイアログリソースからダイアログボックスのキャプションテキストを取得する
		str.Empty();
		HINSTANCE hInst = _AtlBaseModule.GetResourceInstance();
		LPCTSTR lpTemplateName = MAKEINTRESOURCE(idd);
		HRSRC hDlgTempl = FindResource(hInst, lpTemplateName, RT_DIALOG);
		if (hDlgTempl == NULL) return 0;

		HGLOBAL hResource = LoadResource(hInst, hDlgTempl);
		ATL::_DialogSplitHelper::DLGTEMPLATEEX* pDlgTempl = (ATL::_DialogSplitHelper::DLGTEMPLATEEX*)LockResource(hResource);
		if (pDlgTempl == NULL) return 0;

// typedef struct {
//     WORD dlgVer;
//     WORD signature;
//     DWORD helpID;
//     DWORD exStyle;
//     DWORD style;
//     WORD cDlgItems;
//     short x;
//     short y;
//     short cx;
//     short cy;
//     sz_Or_Ord menu;
//     sz_Or_Ord windowClass;
//     WCHAR title[titleLen];
//     WORD pointsize;
//     WORD weight;
//     BYTE italic;
//     BYTE charset;
//     WCHAR typeface[stringLen];
// } DLGTEMPLATEEX;

		// point to sz_Or_Ord "menu"
		WORD* pw = (WORD*)((BYTE*)pDlgTempl + 26);
		switch (*pw) {
		case 0x0000: pw++; break;
		case 0xFFFF: pw++; pw++; break;
		default: while (*pw) pw++; break;
		}
		// point to sz_Or_Ord "windowClass"
		switch (*pw) {
		case 0x0000: pw++; break;
		case 0xFFFF: pw++; pw++; break;
		default: while (*pw) pw++; break;
		}
		// point to LPCWSTR "title"
		str = (LPCWSTR)pw;
		return str.GetLength();
	}
};

#endif		// __OPTIONSETUP_H__

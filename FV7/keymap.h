// keymap.h : header file
//

#pragma once
#ifndef __KEYMAP_H__
#define __KEYMAP_H__

#include "multiple_imagelist.h"

/////////////////////////////////////////////////////////////////////////////
// CKeyPage ダイアログ

class CKeyPage : public CPropertyPageImpl<CKeyPage>
{
private:
	CListBox m_keylist;
	CListBox m_cmdlist;
	MultipleImageList m_mapTool;
	int m_iStartImage;
	int m_numCmds;
	CSize m_sizeKey;
	CSize m_sizeCtrl;

public:
	enum { IDD = IDD_KEYMAP };
	HIMAGELIST m_himKey;
	HIMAGELIST m_himCtrl;
	CSimpleArray<UINT> m_cmds;
	bool m_bUpdate:1;
	bool m_bUseShift:1;
	bool m_bUseShiftCtrl:1;			// Shift+Ctrlのコンビネーション、Shiftの単独は無し
	bool m_bShift:1;
	bool m_bCtrl:1;

	CKeyPage();
	~CKeyPage();

	void SetImage(HIMAGELIST himKey, int iStart, int numCmds);
	void SetCommandList(const UINT* pcuarr, int arrsize);
	void GetCommandList(UINT* puarr);
	void AddItem(UINT cmd);
	UINT GetItemValue(int index);

BEGIN_MSG_MAP(CKeyPage)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_LBN_SELCHANGE(IDC_KEYLIST, OnSelChangeKeyList)
	ON_BN_CLICKED(IDC_USECTRL, OnCtrlCheck)
	ON_BN_CLICKED(IDC_USESHIFT, OnShiftCheck)
	ON_BN_CLICKED(IDC_APPLY, OnApplyCommand)
	ON_BN_CLICKED(IDC_RELEASE, OnReleaseCommand)
END_MSG_MAP()

protected:
	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pHelpInfo) { fvApp.HelpTopic(NULL, HB_DLG, IDD); return TRUE; }
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	void OnSelChangeKeyList();
	void OnCtrlCheck();
	void OnShiftCheck();
	void OnApplyCommand();
	void OnReleaseCommand();

	UINT GetCommand(int nIndex);
	void SelectCommand();
	void DrawKeyList(LPDRAWITEMSTRUCT pdi);
	void DrawCmdList(LPDRAWITEMSTRUCT pdi);
};


/////////////////////////////////////////////////////////////////////////////
// CKeySheet

class CKeySheet : public CPropertySheetImpl<CKeySheet>
{
private:
	bool m_bInitCompleted;
public:	
	HIMAGELIST m_himTab;

	CKeySheet(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheetImpl<CKeySheet>(title, uStartPage, hWndParent)
	{
		m_bInitCompleted = FALSE;
		m_himTab = NULL;
	}

	~CKeySheet(void)
	{
		if (m_himTab) ImageList_Destroy(m_himTab);
	}

	BEGIN_MSG_MAP(CKeySheet)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		CHAIN_MSG_MAP(CPropertySheetImpl<CKeySheet>)
	END_MSG_MAP()

	void Init(void);

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Centre?
		if (wParam == TRUE) Init();
		// Done
		bHandled = FALSE;
		return 0;
	}
};


#endif				// __KEYMAP_H__

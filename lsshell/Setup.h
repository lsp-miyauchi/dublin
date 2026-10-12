// Setup.h : CSetup の宣言

#pragma once

#include "resource.h"       // メイン シンボル
#include <atlhost.h>
#include "propsheet.h"

////////////////////////////////////////////////////////////////////////
//

class CSetupSheet : public CPropertySheet
{
private:
	bool m_bCentered;
public:	
	CSetupSheet(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheet(title, uStartPage, hWndParent), m_bCentered(false)
	{
	}

	~CSetupSheet(void)
	{
	}

	BEGIN_MSG_MAP(CSetupSheet)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		CHAIN_MSG_MAP(CPropertySheet)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Centre?
		if (wParam == TRUE) Center();
		// Done
		bHandled = FALSE;
		return 0;
	}

	void Center(void)
	{
		// Only do this once
		if (!m_bCentered) {
			CenterWindow();
			m_bCentered = true;
		}
	}
};

////////////////////////////////////////////////////////////////////////
//

class CPageGeneric : public CPropertyPageImpl<CPageGeneric>
{
public:
	enum { IDD = IDD_GENERIC };
	DWORD m_flag;
	BOOL m_update;

	CPageGeneric() { m_flag = 0; m_update = FALSE; }

	BEGIN_MSG_MAP(CPageGeneric)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HELP, OnPopupHelp)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPageGeneric>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPopupHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnApply();
};

////////////////////////////////////////////////////////////////////////
//

class CPagePack : public CPropertyPageImpl<CPagePack>
{
public:
	enum { IDD = IDD_PACK };
	DWORD m_flag;
	BOOL m_update;

	CPagePack() { m_flag = 0; m_update = FALSE; }

	BEGIN_MSG_MAP(CPagePack)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HELP, OnPopupHelp)
		ON_BN_CLICKED(IDC_SHELL_PACKASSOC, OnPackAssociation)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPagePack>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPopupHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnPackAssociation();
	BOOL OnApply();
};

////////////////////////////////////////////////////////////////////////
//

class CPageImage : public CPropertyPageImpl<CPageImage>
{
public:
	enum { IDD = IDD_IMAGETOOL };
	DWORD m_flag;
	BOOL m_update;

	CPageImage() { m_flag = 0; m_update = FALSE; }

	BEGIN_MSG_MAP(CPageImage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HELP, OnPopupHelp)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPageImage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPopupHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnApply();
};

////////////////////////////////////////////////////////////////////////
//

class CPageIcon : public CPropertyPageImpl<CPageIcon>
{
public:
	enum { IDD = IDD_ICON };
	DWORD m_flag;
	BOOL m_update;

	CPageIcon() { m_flag = 0; m_update = FALSE; }

	BEGIN_MSG_MAP(CPageIcon)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HELP, OnPopupHelp)
		CHAIN_MSG_MAP(CPropertyPageImpl<CPageIcon>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPopupHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnApply();
};

////////////////////////////////////////////////////////////////////////
//

void DoSetup(HWND hwnd);

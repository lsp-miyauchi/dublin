////////////////////////////////////////////////////////////////////////
// setupsheet.h

#pragma once
#ifndef __SETUPSHEET_H__
#define __SETUPSHEET_H__

// for non-customized sheets
class CSetupSheet : public CPropertySheetImpl<CSetupSheet>
{
private:
	bool m_bInitCompleted;

public:
	CSetupSheet(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheetImpl<CSetupSheet>(title, uStartPage, hWndParent)
	{ m_bInitCompleted = false; }

	BEGIN_MSG_MAP(CSetupSheet)
//@		MESSAGE_HANDLER(WM_COMMAND, CPropertySheetImpl<CSetupSheet>::OnCommand)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		CHAIN_MSG_MAP(CPropertySheetImpl<CSetupSheet>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Centre?
		if (wParam == TRUE) Init();
		// Done
		bHandled = FALSE;
		return 0;
	}

	void Init(void)
	{
		// Only do this once
		if (!m_bInitCompleted) {
			CenterWindow();
			m_bInitCompleted = true;
		}
	}
};


#endif		// __SETUPSHEET_H__

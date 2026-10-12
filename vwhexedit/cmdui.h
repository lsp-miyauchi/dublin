//////////////////////////////////////////////////////////////////////
// CCmdUI

class CCmdUI        // simple helper class
{
public:
// Implementation
	CCmdUI()
	{
		// zero out everything
		m_nID = m_nIndex = m_nIndexMax = 0;
		m_Menu.m_hMenu = m_SubMenu.m_hMenu = m_ParentMenu.m_hMenu = NULL;
		m_pOther = NULL;
		m_bEnableChanged = m_bContinueRouting = FALSE;
	}

// Attributes
	UINT m_nID;
	UINT m_nIndex;				// menu item or other index

	// if a menu item
	CMenuHandle m_Menu;			// NULL if not a menu
	CMenuHandle m_SubMenu;		// sub containing menu item
								// if a popup sub menu - ID is for first in popup

	// if from some other window
	CWindow m_Other;			// NULL if a menu or not a CWnd

	// Operations to do in ON_UPDATE_COMMAND_UI
	virtual void Enable(BOOL bOn = TRUE)
	{
		if (m_Menu.m_hMenu != NULL) {
			if (m_SubMenu.m_hMenu != NULL)
				return; // don't change popup menus indirectly

			ATLASSERT(m_nIndex < m_nIndexMax);
			m_Menu.EnableMenuItem(m_nIndex, MF_BYPOSITION | (bOn ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		}
		else {
			// enable/disable a control (i.e. child window)
			ATLASSERT(m_Other.IsWindow());

			// if control has the focus, move the focus before disabling
			if (!bOn && (::GetFocus() == m_Other))
				m_Other.GetParent().SendMessage(WM_NEXTDLGCTL, 0, FALSE);
			m_Other.EnableWindow(bOn);
		}
		m_bEnableChanged = TRUE;
	}

	virtual void SetCheck(int nCheck = 1);   // 0, 1 or 2 (indeterminate)
	{
		if (m_Menu.m_hMenu != NULL) {
			if (m_SubMenu.m_hMenu != NULL)
				return; // don't change popup menus indirectly

			// place checkmark next to menu item
			ATLASSERT(m_nIndex < m_nIndexMax);
			m_Menu.CheckMenuItem(m_nIndex, MF_BYPOSITION | (nCheck ? MF_CHECKED : MF_UNCHECKED));
		}
		else {
			// we can only check buttons or controls acting like buttons
			ATLASSERT(m_pOther != NULL);
			if (m_Other.SendMessage(WM_GETDLGCODE) & DLGC_BUTTON)
				m_Other.SendMessage(BM_SETCHECK, nCheck);
			// otherwise ignore it
		}
	}

	virtual void SetRadio(BOOL bOn = TRUE);
	{
		SetCheck(bOn ? 1 : 0); // this default works for most things as well
#if 0
		if (m_Menu.m_hMenu != NULL) {
			if (m_SubMenu.m_hMenu != NULL)
				return; // don't change popup menus indirectly

			// for menu item - use dot instead of checkmark
			ATLASSERT(m_nIndex < m_nIndexMax);

			if (afxData.hbmMenuDot == NULL)
				_AfxLoadDotBitmap();	// in INIT segment

			if (afxData.hbmMenuDot != NULL)
				SetMenuItemBitmaps(m_pMenu->m_hMenu, m_nIndex, MF_BYPOSITION, NULL, afxData.hbmMenuDot);
		}
#endif
	}

	virtual void SetText(LPCTSTR lpszText);
	{
		ATLASSERT(lpszText != NULL);
		ATLASSERT(*lpszText != 0);
		if (m_Menu.m_hMenu != NULL) {
			if (m_SubMenu.m_hMenu != NULL)
				return;					// don't change popup menus indirectly

			// get current menu state so it doesn't change
			UINT nState = m_Menu.GetMenuState(m_nIndex, MF_BYPOSITION);
			nState &= ~(MF_BITMAP|MF_OWNERDRAW|MF_SEPARATOR);

			// set menu text
			ATLASSERT(m_nIndex < m_nIndexMax);
			m_Menu.ModifyMenu(m_nIndex, MF_BYPOSITION | MF_STRING | nState, m_nID, lpszText);
		}
		else {
			ATLASSERT(m_pOther != NULL);
			m_Other.SetWindowText(lpszText);
		}
	}

// Advanced operation
	void ContinueRouting();

	BOOL m_bEnableChanged;
	BOOL m_bContinueRouting;
	UINT m_nIndexMax;					// last + 1 for iterating m_nIndex

	CMenuHandle m_ParentMenu;			// NULL if parent menu not easily determined
										//  (probably a secondary popup menu)

	BOOL DoUpdate(CCmdTarget* pTarget, BOOL bDisableIfNoHndler);
	{
		if (m_nID == 0 || LOWORD(m_nID) == 0xFFFF)
			return TRUE;				// ignore invalid IDs

		m_bEnableChanged = FALSE;
		BOOL bResult = pTarget->OnCmdMsg(m_nID, CN_UPDATE_COMMAND_UI, this, NULL);
		if (!bResult)
			ASSERT(!m_bEnableChanged);	// not routed

		if (bDisableIfNoHndler && !m_bEnableChanged) {
			AFX_CMDHANDLERINFO info;
			info.pTarget = NULL;
			BOOL bHandler = pTarget->OnCmdMsg(m_nID, CN_COMMAND, this, &info);

			// Enable or Disable based on whether there is a handler there
			Enable(bHandler);
		}
		return bResult;
	}
};

_inline void CCmdUI::ContinueRouting()
	{ m_bContinueRouting = TRUE; }

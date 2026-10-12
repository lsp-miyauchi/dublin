/////////////////////////////////////////////////////////////////////////////
// CCheckTreeViewCtrl

#pragma once
#ifndef __lcc_include_checktreeviewctrl_h__
#define __lcc_include_checktreeviewctrl_h__



template <class TBase>
class CCheckTreeViewCtrlT : public TBase
{
public:
// Constructors
	CCheckTreeViewCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CCheckTreeViewCtrlT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return CWindow::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_LSC_CHECKTREEVIEW;
	}

	int AddItem(CTITEM* pc)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CTM_ADDITEM, 0, (LPARAM)pc);
	}

	BOOL AddItem(LPTSTR pszText, int id, int idGroup, UINT state)
	{
		CTITEM c;
		memset(&c, 0, sizeof(c));
		c.pszText = pszText;
		c.id = id;
		c.state = state;
		return AddItem(&c);
	}

	UINT GetItemState(int id, UINT mask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, CTM_GETITEMSTATE, id, mask);
	}

	BOOL SetItemState(CTITEM* pc)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CTM_SETITEMSTATE, 0, (LPARAM)pc);
	}

	int GetItemText(int id, LPTSTR psz)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CTM_GETITEMTEXT, id, (LPARAM)psz);
	}

	BOOL SetItemText(int id, LPCTSTR psz)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CTM_SETITEMTEXT, id, (LPARAM)psz);
	}

	BOOL SetItemState(int id, UINT state, UINT stateMask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CTITEM c;
		memset(&c, 0, sizeof(c));
		c.id = id;
		c.state = state;
		c.stateMask = stateMask;
		return SetItemState(&c);
	}

	BOOL CheckItem(int id, BOOL bCheck)
	{
		return SetItemState(id, bCheck ? CTIS_CHECKED : 0, CTIS_CHECKED);
	}

	BOOL IsItemChecked(int id)
	{
		return (GetItemState(id, CTIS_CHECKED) != 0);
	}

	BOOL EnableItem(int id, BOOL bEnable)
	{
		return SetItemState(id, bEnable ? 0 : CTIS_DISABLED, CTIS_DISABLED);
	}

	BOOL IsItemEnabled(int id)
	{
		return (GetItemState(id, CTIS_DISABLED) == 0);
	}

	BOOL SelectItem(int id)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CTM_SELECTITEM, id, 0);
	}

	int GetSpacing()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CTM_GETSPACING, 0, 0);
	}

	int SetSpacing(int iSpacing)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CTM_SETSPACING, 0, iSpacing);
	}

};

typedef CCheckTreeViewCtrlT<CWindow>	CCheckTreeViewCtrl;


#endif			// __lcc_include_checktreeviewctrl_h__

/////////////////////////////////////////////////////////////////////////////
// CFolderTreeCtrl

#pragma once
#ifndef __lcc_include_foldertreectrl_h__
#define __lcc_include_foldertreectrl_h__


template <class TBase>
class CFolderTreeCtrlT : public TBase
{
public:
// Constructors
	CFolderTreeCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CFolderTreeCtrlT< TBase >& operator=(HWND hWnd)
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
		return WC_LSC_FOLDERTREE;
	}

	BOOL Select(UINT nType, void* pTarget)
	{
		// nType = FT_BYPATH, then pTarget = path name
		// nType = FT_BYIDLIST, then pTarget = pidl
		// nType = FT_BYCSIDL, then pTarget = CSIDL_xxxx
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, FTWM_SETCURSEL, (WPARAM)nType, (LPARAM)pTarget);
	}
	BOOL SelectByPath(LPCTSTR pszPath)
	{
		return Select(FT_BYPATH, (void*)pszPath);
	}
	BOOL SelectByIDList(LPCITEMIDLIST pidl)
	{
		return Select(FT_BYIDLIST, (void*)pidl);
	}
	BOOL SelectByCSIDL(UINT csidl)
	{
		return Select(FT_BYCSIDL, (void*)csidl);
	}
	LPITEMIDLIST GetSelItem(DWORD* pPidlAttr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LPITEMIDLIST)::SendMessage(m_hWnd, FTWM_GETCURSEL, 0, (LPARAM)pPidlAttr);
	}
	BOOL SelectParent()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, FTWM_SELECTPARENT, 0, 0);
	}
	BOOL SelectChild(LPITEMIDLIST pidlChild)
	{
		// pidlChild = simple idlist
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, FTWM_SELECTCHILD, 0, (LPARAM)pidlChild);
	}
	void Refresh()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FTWM_REFRESH, 0, 0);
	}
	BOOL SetRoot(UINT nType, void* pRoot)
	{
		// nType = FT_BYPATH, then pRoot = path name
		// nType = FT_BYIDLIST, then pRoot = pidl
		// nType = FT_BYCSIDL, then pRoot = CSIDL_xxxx
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, FTWM_SETROOT, (WPARAM)nType, (LPARAM)pRoot);
	}
	inline BOOL SetRoot(LPCTSTR pszRootPath)
	{
		return SetRoot(FT_BYPATH, (void*)pszRootPath);
	}
	inline BOOL SetRoot(LPCITEMIDLIST pidlRoot)
	{
		return SetRoot(FT_BYIDLIST, (void*)pidlRoot);
	}
	inline BOOL SetRoot(int csidl)
	{
		return SetRoot(FT_BYCSIDL, (void*)csidl);
	}
	void SetCustomColors(int numColors, COLORREF* pColors)
	{
		// pColors[0] = TEXTCOLOR
		// pColors[1] = BACKCOLOR
		// pColors[2] = SELECT_TEXTCOLOR
		// pColors[3] = SELECT_BACKCOLOR
		// 色指定(RGB)がCLR_DEFAULTであればデフォルト色が使われる
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, FTWM_SETCOLORS, (WPARAM)numColors, (LPARAM)pColors);
	}
	HWND SetOwner(HWND hwndOwner)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, FTWM_SETOWNER, 0, (LPARAM)hwndOwner);
	}
	BOOL QueryDescendant(UINT nType, void* pFolder)
	{
		// nType = FT_BYPATH, then pRoot = path name
		// nType = FT_BYIDLIST, then pRoot = pidl
		// nType = FT_BYCSIDL, then pRoot = CSIDL_xxxx
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, FTWM_QUERYDESCENDANT, (WPARAM)nType, (LPARAM)pFolder);
	}
	inline BOOL QueryDescendant(LPCTSTR pszFolder)
	{
		return QueryDescendant(FT_BYPATH, (void*)pszFolder);
	}
	inline BOOL QueryDescendant(LPCITEMIDLIST pidlFolder)
	{
		return QueryDescendant(FT_BYIDLIST, (void*)pidlFolder);
	}
	inline BOOL QueryDescendant(int csidl)
	{
		return QueryDescendant(FT_BYCSIDL, (void*)csidl);
	}
	DWORD GetTreeCtrlStyle()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, FTWM_GETTREECTRLSTYLE, 0, 0);
	}
	DWORD SetTreeCtrlStyle(DWORD mask, DWORD style)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, FTWM_SETTREECTRLSTYLE, (WPARAM)mask, (LPARAM)style);
	}
};

typedef CFolderTreeCtrlT<CWindow>	CFolderTreeCtrl;


#endif			// __lcc_include_foldertreectrl_h__

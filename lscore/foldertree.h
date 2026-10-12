// foldertree.h : CFolderTreeCtrl クラスのインターフェイス
//

#pragma once
#ifndef __FOLDERTREE_H__
#define __FOLDERTREE_H__


///////////////////////////////////////////////////////////////////////
//

typedef ATL::CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES , 0> CFolderTreeTraits;

template <class T, class TBase = CTreeViewCtrl, class TWinTraits = CFolderTreeTraits>
class ATL_NO_VTABLE CFolderTreeImpl : public ATL::CWindowImpl<T, TBase, TWinTraits>
{
private:
	HTREEITEM m_hHotItem;
	BOOL m_bMouseIsTracking;

public:
	CFolderTreeImpl()
		: m_hHotItem(NULL)
		, m_bMouseIsTracking(FALSE)
	{
	}

	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

BEGIN_MSG_MAP(CFolderTreeImpl)
	ON_WM_ERASEBKGND()
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnHandlerMouseMoveHook)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnHandlerMouseLeaveHook)
END_MSG_MAP()

	BOOL OnEraseBkgnd(HDC hdc)
	{
		T* pT = static_cast<T*>(this);
		CRect rc;
		::GetClientRect(pT->m_hWnd, &rc);
		COLORREF clr = (COLORREF)::SendMessage(pT->m_hWnd, TVM_GETBKCOLOR, 0, 0);
		if (clr == (COLORREF)-1)
			clr = GetSysColor(COLOR_WINDOW);

		CDCHandle dc = hdc;
		dc.FillSolidRect(&rc, clr);
		return TRUE;
	}

	LRESULT OnHandlerMouseMoveHook(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		if (GetCapture() == NULL) {
			if (!m_bMouseIsTracking) {
				TRACKMOUSEEVENT te;
				te.cbSize = sizeof(te);
				te.dwFlags = TME_LEAVE;
				te.hwndTrack = pT->m_hWnd;
				te.dwHoverTime = HOVER_DEFAULT;
				m_bMouseIsTracking = _TrackMouseEvent(&te);
			}

			if (m_bMouseIsTracking) {
				CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				pT->MouseMove(pt);
			}
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnHandlerMouseLeaveHook(UINT msg, WPARAM, LPARAM, BOOL& bHandled)
	{
		m_bMouseIsTracking = FALSE;
		T* pT = static_cast<T*>(this);
		pT->MouseLeave();
		bHandled = FALSE;
		return 0;
	}

	void MouseMove(POINT pt)
	{
		TVHITTESTINFO ht;
		ht.pt = pt;

		T* pT = static_cast<T*>(this);
		pT->HitTest(&ht);
		HTREEITEM hItem = (ht.flags & TVHT_ONITEM) ? ht.hItem : NULL;
		if (hItem == m_hHotItem)
			return;

		HTREEITEM hTemp = m_hHotItem;
		m_hHotItem = hItem;
		if (hTemp != NULL)
			RedrawTreeItem(hTemp);
		if (m_hHotItem != NULL)
			RedrawTreeItem(m_hHotItem);
	}

	void MouseLeave()
	{
		HTREEITEM hTemp = m_hHotItem;
		m_hHotItem = NULL;
		if (hTemp != NULL)
			RedrawTreeItem(hTemp);
	}

	void RedrawTreeItem(HTREEITEM hItem)
	{
		if (hItem) {
			CRect rc;
			T* pT = static_cast<T*>(this);
			if (pT->GetItemRect(hItem, &rc, TRUE))
				pT->InvalidateRect(&rc, FALSE);
		}
	}

	HTREEITEM GetHotItem() { return m_hHotItem; }
};

class CFolderTreeCtrl : public CFolderTreeImpl<CFolderTreeCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("LSC_InternalFolderTreeCtrl"), GetWndClassName())
};


///////////////////////////////////////////////////////////////////////
//

#define IDC_FOLDERTREE				64

enum {
	colNorm = 0,
	colBack,
	colSelect,
	colSelBack,
	colSystem,
	colHidden,
	colRdonly,
	colCmprs,
	colEncr,
	colRepp,
	colLine,
	colMark,
	colMarkBack,
	colEndValue,
};


class CShellTreeCtrl : public CWindowImpl<CShellTreeCtrl>, public CThemeImpl<CShellTreeCtrl>
{
private:
	CWindow m_owner;
	HIMAGELIST m_himSystem;
	HIMAGELIST m_himTree;
	CSize m_sizeIcon;
	CFolderTreeCtrl m_tree;
	UINT_PTR m_timerid;
	CShellNotify* m_pShellNotify;
	bool m_bInitializeCompleted:1;
	COLORREF* m_colors;
	int m_treatHidden;

public:
	CShellTreeCtrl()
		: m_himSystem(NULL)
		, m_himTree(NULL)
		, m_sizeIcon(CSize(0, 0))
		, m_timerid(0)
		, m_pShellNotify(NULL)
		, m_bInitializeCompleted(false)
		, m_treatHidden(0)
		, m_colors(NULL)
	{
		m_colors = new COLORREF[colEndValue];
		m_colors[colNorm] = GetSysColor(COLOR_WINDOWTEXT);
		m_colors[colBack] = GetSysColor(COLOR_WINDOW);
		m_colors[colSelect] = GetSysColor(COLOR_HIGHLIGHTTEXT);
		m_colors[colSelBack] = GetSysColor(COLOR_HIGHLIGHT);
		m_colors[colSystem] = RGB(128, 0, 128);
		m_colors[colHidden] = RGB(0, 0, 255);
		m_colors[colRdonly] = RGB(0, 128, 0);
		m_colors[colCmprs] = RGB(204, 153, 0);
		m_colors[colLine] = CLR_DEFAULT;
		m_colors[colEncr] = RGB(128, 0, 0);
		m_colors[colRepp] = RGB(204, 204, 0);

		// アプリ共通設定
		COLORREF* colors = new COLORREF[colEndValue];
		int size = lccRegGetBinary(_T("CommonSetting"), _T("Colors"), _T("ColorMap"), (BYTE*)colors, sizeof(COLORREF) * colEndValue);
		if (size > 0 && (size % sizeof(COLORREF)) == 0) {
			size = min(size, sizeof(COLORREF) * colEndValue);
			memcpy(m_colors, colors, size);
		}
		delete [] colors;

		// Theme
		SetThemeClassList(L"TreeView");
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
	}

	~CShellTreeCtrl()
	{
		if (m_colors) delete [] m_colors;
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CShellTreeCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CShellTreeCtrl>)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_SETFONT()
	ON_NOTIFY(TVN_GETDISPINFO, IDC_FOLDERTREE, OnNotifyGetDispInfo)
	ON_NOTIFY(TVN_SELCHANGED, IDC_FOLDERTREE, OnNotifySelChanged)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_FOLDERTREE, OnNotifyItemExpanding)
	ON_NOTIFY(TVN_DELETEITEM, IDC_FOLDERTREE, OnNotifyDeleteItem)
	ON_NOTIFY(TVN_KEYDOWN, IDC_FOLDERTREE, OnNotifyKeyDown)
	ON_NOTIFY(NM_RCLICK, IDC_FOLDERTREE, OnNotifyRClick)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_FOLDERTREE, OnNotifyCustomDraw)
	ON_MESSAGE(FTWM_SETCURSEL, OnSetCurSel)
	ON_MESSAGE(FTWM_GETCURSEL, OnGetCurSel)
	ON_MESSAGE(FTWM_SELECTCHILD, OnSelectChild)
	ON_MESSAGE(FTWM_SELECTPARENT, OnSelectParent)
	ON_MESSAGE(FTWM_SETROOT, OnSetRoot)
	ON_MESSAGE(FTWM_GETTREECTRLSTYLE, OnGetTreeCtrlStyle)
	ON_MESSAGE(FTWM_SETTREECTRLSTYLE, OnSetTreeCtrlStyle)
	ON_MESSAGE_VOID(FTWM_SETCOLORS, OnSetCustomColors)
	ON_MESSAGE_VOID(FTWM_REFRESH, OnRefreshTree)
	ON_MESSAGE_VOID(FTWM_SETOWNER, OnSetOwner)
	ON_REGISTERD_MESSAGE(nRM_SHNOTIFY, OnShellNotify)
END_MSG_MAP()

	// message handler
	int OnCreate(LPCREATESTRUCT pcs);
	void OnDestroy();
	inline void OnSize(UINT flags, int cx, int cy) { m_tree.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE); }
	inline BOOL OnEraseBkgnd(HDC hdc) { return (m_tree.IsWindow() && m_tree.IsWindowVisible()); }
	inline void OnSetFocus(HWND hOldWnd) { m_tree.SetFocus(); }
	inline void OnSetFont(HFONT hFont, BOOL bRedraw) { m_tree.SetFont(hFont, bRedraw); }
	void OnNotifyGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifySelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyRClick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyCustomDraw(NMHDR* pHdr, LRESULT* pResult);
	void OnTimer(UINT_PTR id);
	inline LRESULT OnSetRoot(WPARAM wParam, LPARAM lParam) { return (LRESULT)SetRoot((UINT)wParam, (void*)lParam); }
	inline LRESULT OnQueryDescendant(WPARAM wParam, LPARAM lParam) { return (LRESULT)QueryDescendant((UINT)wParam, (void*)lParam); }
	inline LRESULT OnSelectParent(WPARAM wParam, LPARAM lParam) { return (LRESULT)SelectParent(); }
	inline LRESULT OnSelectChild(WPARAM wParam, LPARAM lParam) { return (LRESULT)SelectChild((LPCITEMIDLIST)lParam); }
	LRESULT OnSetCurSel(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetCurSel(WPARAM wParam, LPARAM lParam);
	inline LRESULT OnGetTreeCtrlStyle(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_tree.GetStyle(); }
	LRESULT OnSetTreeCtrlStyle(WPARAM wParam, LPARAM lParam);
	void OnSetCustomColors(WPARAM wParam, LPARAM lParam);
	inline void OnRefreshTree(WPARAM wParam, LPARAM lParam) { RefreshTree(); }
	inline void OnSetOwner(WPARAM wParam, LPARAM lParam) { SetOwner((HWND)lParam); }
	LRESULT OnShellNotify(WPARAM wParam, LPARAM lParam);

	// functions
	void SetupTreeColors();
	bool InitializeTree(PCIDLIST_ABSOLUTE pidlRoot = NULL);
	COLORREF GetAttributeColor(CFolderItem* pf, bool bHilited);
	static int CALLBACK CompareFolder(LPARAM lp1, LPARAM lp2, LPARAM lps);
	void SortChildren(HTREEITEM hParentItem);
	HTREEITEM InsertTreeItem(HTREEITEM hParent, HTREEITEM hInsertAfter, CFolderItem* pf);
	bool EnumFolderObjects(HTREEITEM hParentItem, IShellFolder* psf, PCIDLIST_ABSOLUTE pidlParent);
	BOOL ContextMenu(HTREEITEM hItem, LPPOINT ppt);
	BOOL SetRoot(UINT mode, void* pv);
	BOOL QueryDescendant(UINT mode, void* pv);
	BOOL IsRootDescendant(PCIDLIST_ABSOLUTE pidl);
	HTREEITEM FindIDList(HTREEITEM hParent, PCUIDLIST_RELATIVE pidl);
	BOOL SelectChild(PCUIDLIST_RELATIVE pidlSingle);
	BOOL SelectParent();
	void RefreshTree();
	HWND SetOwner(HWND hwndOwner);
	BOOL FindFolder(PCIDLIST_ABSOLUTE pidlSelection, HTREEITEM* pItem, bool bExplore);
	BOOL UpdateFolder(PCIDLIST_ABSOLUTE pidl);
	BOOL AddFolder(PCIDLIST_ABSOLUTE pidl);
	BOOL ItemIsParent(HTREEITEM hParent, HTREEITEM hItem);
	void DeleteTreeItem(HTREEITEM hItem);
	BOOL DeleteFolder(PCIDLIST_ABSOLUTE pidl);
	int ListupAllItems(HTREEITEM hItem, CSimpleArray<HTREEITEM>& array);
	void UpdateImage(int iChangeImageIndex);
	void ProcessShellNotify(DWORD id, LPITEMIDLIST* pidls);
	LRESULT SendSimpleNotify(UINT code);
};



#endif			// __FOLDERTREE_H__

/////////////////////////////////////////////////////////////////////////////
// bookmark_view.h

#pragma once

#include "mypanecontainer_ex.h"
#include "dropper.h"
#include "mousewheeler.h"


/////////////////////////////////////////////////////////////////////////////
// CFaveView ビュー

#define FAM_UPDATEVIEW			(WM_USER + 430)
#define FAM_INSERTITEM			(WM_USER + 431)

#define IDC_FAVORITELIST		1001


class CBookmarkView : public CMyPaneContainerImpl<CBookmarkView>, public TDropperImpl<CBookmarkView>
{
private:
	CMWListViewCtrl m_list;
	CFont m_font;
	int m_hotIndex;
	int m_insIndex;
	bool m_bLabelEditing;

public:
	enum { m_nCloseBtnID = IDM_BAR_FAVO };
	DECLARE_WND_CLASS_EX(_T("FavoriteView"), CS_DBLCLKS, -1)

	CBookmarkView()
		: m_hotIndex(-1)
		, m_insIndex(-1)
		, m_bLabelEditing(false)
	{
	}

	HWND Create(HWND hWndParent, ATL::_U_STRINGorID title, UINT nID)
	{
		return CMyPaneContainerImpl<CBookmarkView>::Create(hWndParent, title, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, nID);
	}

public:
	inline BOOL IsMyMessage(const MSG* pMsg) const { return (pMsg->hwnd == m_list); }
	BOOL ProcessMessage(const MSG* pMsg)
	{
		switch (pMsg->message) {
		case WM_CHAR:
			switch (pMsg->wParam) {
			case VK_RETURN:
				return TRUE;
			}
			break;
		}
		return FALSE;
	}
	void UpdateView(WPARAM wHint = 0, LPARAM lHint = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::PostMessage(m_hWnd, FAM_UPDATEVIEW, wHint, lHint);
	}

	BOOL InsertFavoriteItem(LPCITEMIDLIST pidlFq, int iInsertAfter = -1)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, FAM_INSERTITEM, (WPARAM)iInsertAfter, (LPARAM)pidlFq);
	}

BEGIN_MSG_MAP(CBookmarkView)
	CHAIN_MSG_MAP(TDropperImpl<CBookmarkView>)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	ON_WM_MOUSEACTIVATE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_KEYDOWN, IDC_FAVORITELIST, OnNotifyKeyDown)
	ON_NOTIFY(LVN_DELETEITEM, IDC_FAVORITELIST, OnNotifyDeleteItem)
	ON_NOTIFY(LVN_GETINFOTIP, IDC_FAVORITELIST, OnNotifyGetInfoTip)
	ON_NOTIFY(NM_DBLCLK, IDC_FAVORITELIST, OnNotifyDblClk)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_FAVORITELIST, OnNotifyBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_FAVORITELIST, OnNotifyEndLabelEdit)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_FAVORITELIST, OnNotifyItemActivate)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_FAVORITELIST, OnNotifyCustomDraw)
	ON_COMMAND(IDM_FAVO_OPEN, OnItemOpen)
	ON_COMMAND(IDM_FAVO_TABNEW, OnItemTabNew)
	ON_COMMAND(IDM_FAVO_OPENNEW, OnItemOpenNew)
	ON_COMMAND(IDM_FAVO_RENAME, OnItemRename)
	ON_COMMAND(IDM_FAVO_DELETE, OnItemDelete)
	ON_COMMAND(IDM_FAVO_PROPERTY, OnItemProperty)
	ON_COMMAND(IDM_FAVO_ITEMUP, OnItemMoveUp)
	ON_COMMAND(IDM_FAVO_ITEMDOWN, OnItemMoveDown)
	ON_MESSAGE_VOID(FAM_UPDATEVIEW, OnUpdateView)
	ON_MESSAGE(FAM_INSERTITEM, OnInsertItem)
	CHAIN_MSG_MAP(CMyPaneContainerImpl<CBookmarkView>)
END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	int OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message);
	void OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyBeginLabelEdit(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyEndLabelEdit(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyItemActivate(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyCustomDraw(NMHDR* pNmhdr, LRESULT* pResult);
	void OnContextMenu(HWND hwnd, CPoint pt);
	void OnItemOpen();
	void OnItemTabNew();
	void OnItemOpenNew();
	void OnItemDelete();
	void OnItemRename();
	void OnItemProperty();
	void OnItemMoveUp();
	void OnItemMoveDown();
	inline void OnUpdateView(WPARAM wParam, LPARAM lParam) { UpdateProperty(wParam, lParam); }
	LRESULT OnInsertItem(WPARAM wParam, LPARAM lParam);

	void LoadList();
	void SaveList();
	void UpdateProperty(WPARAM wHint = 0, LPARAM lHint = 0);
	bool ItemDelete(int index);
	void OpenTabGroup(int index, CCmItem* pci);

	int CheckDragScroll(int index);
	void SetInsertionMark(int index);
	bool InsertItem(LPCITEMIDLIST pidl, int iInsertAfter);

	// TDropperImpl メンバ
	DWORD OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point);
	void OnDragLeave();
};


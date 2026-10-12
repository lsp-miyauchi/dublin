////////////////////////////////////////////////////////////////////////
// tabbarctrl.h

#pragma once
#ifndef __TABBARCTRL_H__
#define __TABBARCTRL_H__

#include "trackmenuwnd.h"
#include "resource.h"
#include "bar_container.h"

////////////////////////////////////////////////////////////////////////
//

template <class T, class TBase = CBarContainer>
class ATL_NO_VTABLE CTabBarContainerImpl
	: public ATL::CWindowImpl< T, TBase >
	, public TDropperImpl< T >
{
private:
	CLccTabExCtrl m_tab;

	int m_height;
	CFontHandle m_font;
	int m_dropIndex;
	int m_selfDragIndex;
	CSize m_sizeIcon;
	int m_indexHighlight;

public:
// Constructor/destructor
	CTabBarContainerImpl()
			: m_height(0)
			, m_dropIndex(-1)
			, m_selfDragIndex(-1)
			, m_indexHighlight(-1)
	{
		m_font = _GCDATA.GetIconTitleFont();
	}

	~CTabBarContainerImpl()
	{
	}

// Message map and handlers
	BEGIN_MSG_MAP(CTabBarContainerImpl< T >)
		CHAIN_MSG_MAP(TDropperImpl< T >)
		MESSAGE_HANDLER(WM_CREATE, OnHandlerCreate)
		MESSAGE_HANDLER(WM_SIZE, OnHandlerSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnHandlerSetFocus)
		ON_WM_GETDLGCODE()
	END_MSG_MAP()

private:
	LRESULT OnHandlerCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		DWORD style = WS_CHILD | WS_VISIBLE | TCS_HOTTRACK | TCS_TABS | TCS_SINGLELINE | TCS_RIGHTJUSTIFY | TCS_TOOLTIPS | TCS_FOCUSNEVER;
		CRect rc(0, 0, 0, 0);
		if (m_tab.Create(m_hWnd, rc, NULL, style, 0, GetDlgCtrlID())) {
			m_tab.SetFont(m_font);

			DWORD mask = TCS_EX_CLOSEBUTTON | TCS_EX_BOTTOMEDGE | TCS_EX_DRAGDROP | TCS_EX_HOVERSELECT;
			DWORD exStyle = TCS_EX_CLOSEBUTTON | TCS_EX_DRAGDROP | TCS_EX_BOTTOMEDGE;
			if (gnTabStat & TAB_HOVERSELECT)
				exStyle |= TCS_EX_HOVERSELECT;
			m_tab.SetExtendedStyle(mask, exStyle);
			m_tab.SetNotifyWnd(GetParent());
			m_tab.SetPaneColor(GetSysColor(COLOR_3DFACE));

			RecalcBarHeight();
		}
		return 0;
	}

	LRESULT OnHandlerSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if (wParam != SIZE_MINIMIZED) {
			CSize size((int)LOWORD(lParam), (int)HIWORD(lParam));
			SetWindowPos(NULL, 0, 0, size.cx, m_height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			m_tab.MoveWindow(0, 0, size.cx, m_height, TRUE);
		}
		return 0;
	}

	UINT OnGetDlgCode()
	{
		return (m_tab.GetStyle() & TCS_FOCUSNEVER) ? DLGC_STATIC : (DLGC_WANTARROWS | DLGC_WANTTAB);
	}

	LRESULT OnHandlerSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_tab.GetStyle() & TCS_FOCUSNEVER) {
			GetParent().SetFocus();
			return 0;
		}

		bHandled = FALSE;
		return 1;
	}

	void RecalcBarHeight()
	{
		CClientDC dc(NULL);
		HFONT hfont = dc.SelectFont(m_font);
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		dc.SelectFont(hfont);

		if (m_tab.IsWindow()) {
			HIMAGELIST him = m_tab.GetImageList();
			if (him) {
				int cx, cy;
				ImageList_GetIconSize(him, &cx, &cy);
				m_sizeIcon.cx = cx;
				m_sizeIcon.cy = cy;
			}
		}
		m_height = max(38, max(tm.tmHeight, m_sizeIcon.cy) + 11);
	}

public:
	inline int GetHeight() { return m_height; }
	inline int InsertItem(int nItem, LPCTSTR lpszItem, int iImage, LPARAM lParam)
	{
		return m_tab.InsertItem(nItem, TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, lpszItem, iImage, lParam);
	}
	inline BOOL GetItem(int nItem, TCITEM* pItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_GETITEM, nItem, (LPARAM)pItem);
	}
	inline BOOL GetItemText(int nItem, CString& str) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM tci;
		tci.mask = TCIF_TEXT;
		tci.pszText = str.GetBuffer(MAX_PATH);
		*tci.pszText = 0;
		tci.cchTextMax = MAX_PATH;
		BOOL bOK = (BOOL)::SendMessage(m_hWnd, TCM_GETITEM, nItem, (LPARAM)&tci);
		str.ReleaseBuffer();
		return bOK;
	}

	int AddItem(LPCTSTR lpszItem, int iImage, LPARAM lParam)
	{
		int nItem = m_tab.GetItemCount();
		return InsertItem(nItem, lpszItem, iImage, lParam);
	}

	BOOL SetItem(int nItem, LPCTSTR lpszItem, int iImage)
	{
		TCITEM item = { 0 };
		item.mask = TCIF_TEXT | TCIF_IMAGE;
		item.pszText = (LPTSTR)lpszItem;
		item.iImage = iImage;
		return m_tab.SetItem(nItem, &item);
	}

	inline BOOL DeleteItem(int nItem) { return m_tab.DeleteItem(nItem); }
	inline int GetCount() { return m_tab.GetItemCount(); }
	inline int GetCurSel() { return m_tab.GetCurSel(); }
	inline int SetCurSel(int index) { return m_tab.SetCurSel(index); }
	inline DWORD_PTR GetItemData(int index) { return m_tab.GetItemData(index); }
	inline DWORD_PTR SetItemData(int index, DWORD_PTR param) { return m_tab.SetItemData(index, param); }
	inline int DeleteAllItems() { return m_tab.DeleteAllItems(); }
	inline BOOL isItemLocked(int index) { return ((m_tab.GetItemState(index) & TCIS_LOCKED) != 0); }
	inline void LockItem(int index, BOOL bLock) { m_tab.LockItem(index, bLock); }
	inline BOOL ExchangeItem(int index1, int index2) { return m_tab.TabExchange(index1, index2); }

	HIMAGELIST SetImageList(HIMAGELIST him)
	{
		HIMAGELIST himPrev = m_tab.SetImageList(him);
		RecalcBarHeight();
		return himPrev;
	}

	inline void SetExStyle(DWORD mask, DWORD style) { m_tab.SetExtendedStyle(mask, style); }

	DWORD OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		if (IsDropFormatsSupported(SDO_HDROP|SDO_SHELLIDLIST))
			return OnDragOver(pdo, keyState, dropEffect, point);
		return DROPEFFECT_NONE;
	}

	DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		DROPEFFECT eff = GetDropEffect(dropEffect, keyState);

		// ドロップ可能タブを特定する
		TCHITTESTINFO hit;
		hit.pt = point;
		m_tab.ScreenToClient(&hit.pt);
		int index = m_tab.HitTest(&hit);

		// ドロップ可能パスに変化がある場合
		if (index != m_dropIndex) {
			ShowDropHelper(FALSE);
			if (m_dropIndex >= 0)
				m_tab.HighlightItem(m_dropIndex, FALSE);
			if (index >= 0)
				m_tab.HighlightItem(index, TRUE);
			m_dropIndex = index;
			ShowDropHelper(TRUE);
		}

		// VISTAのエアロD&Dテキスト
		CString strTargetDisplayName;
		if (m_dropIndex < 0) {
			eff = DROPEFFECT_LINK;
			strTargetDisplayName = _T("タブ");
		}
		else {
			TCITEM item = { 0 };
			item.mask = TCIF_TEXT;
			item.cchTextMax = MAX_PATH;
			item.pszText = strTargetDisplayName.GetBuffer(item.cchTextMax);
			m_tab.GetItem(m_dropIndex, &item);
			strTargetDisplayName.ReleaseBuffer();
		}

		SetDropTargetDescription(pdo, strTargetDisplayName, eff);
		return eff;
	}

	DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		if (m_dropIndex >= 0) {
			CDocument* pDoc = (CDocument*)m_tab.GetItemData(m_dropIndex);
			DWORD eff = pDoc->m_pWndBrowser->LocalDropFile(GetSupportedDropFormats(), pdo, keyState, dropEffect, point);
			if (gnTabStat & TAB_DROPOPEN) {
				int curIndex = m_tab.GetCurSel();
				if (m_dropIndex != curIndex && m_tab.SetCurSel(m_dropIndex) >= 0) {
					// タブ変更を擬似生成
					NMTCSELCHANGE sc;
					sc.hdr.hwndFrom = m_tab.m_hWnd;
					sc.hdr.idFrom = m_tab.GetDlgCtrlID();
					sc.hdr.code = TCN_SELCHANGE;
					sc.iItemOld = curIndex;
					sc.iItemNew = m_dropIndex;
					GetParent().SendMessage(WM_NOTIFY, (WPARAM)sc.hdr.idFrom, (LPARAM)&sc);
				}
			}
			return eff;
		}

		// タブを追加
		DROPEFFECT eff = DROPEFFECT_NONE;
		CSimpleArray<PCIDLIST_ABSOLUTE> array;
		int cnt = GetIDListsFromDataObject(pdo, array);
		if (cnt > 0) {
			PCIDLIST_ABSOLUTE pidl;
			for (int i = 0; i < cnt; i++) {
				pidl = array[i];
				// 最初のエントリーのみを処理
				if (i == 0) {
					if (!(idl_GetAttributes(pidl, SFGAO_FOLDER) & SFGAO_FOLDER)) {
						// フォルダでなければ親フォルダを取得
						ILRemoveLastID((LPITEMIDLIST)pidl);
					}
					fvApp.OpenTabFolder(pidl, TRUE);
					eff = DROPEFFECT_LINK;
				}
				CoTaskMemFree((void*)pidl);
			}
		}
		return eff;
	}

	void OnDragLeave()
	{
		// ドロップ待機状態になっているパスを元に戻す
		if (m_dropIndex >= 0) {
			m_tab.HighlightItem(m_dropIndex, FALSE);
			m_dropIndex = -1;
		}
	}

	void SetHilightItem(int index)
	{
		m_indexHighlight = index;
		m_tab.HighlightItem(index, TRUE);
	}

	void ResetHilightItem()
	{
		if (m_indexHighlight >= 0)
			m_tab.HighlightItem(m_indexHighlight, FALSE);
		m_indexHighlight = -1;
	}

	inline int GetHilightItem() { return m_indexHighlight; }
	inline CLccTabExCtrl GetTabCtrl() { return m_tab; }
};

////////////////////////////////////////////////////////////////////////
//

class CTabBarCtrl : public CTabBarContainerImpl<CTabBarCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("TabbarCtrl"), GetWndClassName())

	HWND Create(HWND hParent, UINT idCtrl)
	{
		CRect rc(0, 0, 0, 0);
		return CTabBarContainerImpl::Create(hParent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, idCtrl);
	}
};


#endif		// __TABBARCTRL_H__

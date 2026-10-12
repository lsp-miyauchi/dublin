// CCheckTreeViewCtrl.h : interface of the CCheckTreeViewCtrl class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
//

class tcbstr
{
private:
	LPTSTR m_psz;
public:
	inline tcbstr() { m_psz = NULL; }
	tcbstr(LPCTSTR psz) { m_psz = NULL; set(psz); }
	inline ~tcbstr() { clear(); }
	void clear()
	{
		if (m_psz && m_psz != LPSTR_TEXTCALLBACK)
			delete [] m_psz;
		m_psz = NULL;
	}
	void set(LPCTSTR psz)
	{
		clear();
		if (psz == NULL || psz == LPSTR_TEXTCALLBACK)
			m_psz = (LPTSTR)psz;
		else if (*psz) {
			int len = (lstrlen(psz) + 1) * sizeof(TCHAR);
			m_psz = new TCHAR[len + 64];
			memcpy(m_psz, psz, len);
		}
	}
	inline operator LPCTSTR() const { return (this == NULL) ? NULL : m_psz; }
	inline const tcbstr& operator=(LPCTSTR psz) { set(psz); return *this; }
	inline bool istextcallback() { return (m_psz == LPSTR_TEXTCALLBACK); }
	inline bool isempty() { return ((m_psz == NULL) || (m_psz == LPSTR_TEXTCALLBACK)); }
};

class CCheckItem
{
public:
	int m_id;
	UINT m_state;
	HTREEITEM m_hItem;
	int m_index;
	tcbstr m_tsTitle;
	tcbstr m_tsFollow;
	tcbstr m_tsEdit;

	CCheckItem(HTREEITEM hItem = NULL, int id = 0, UINT state = 0, LPTSTR psz = NULL)
		:m_hItem(hItem)
		,m_id(id)
		,m_state(state)
		,m_index(0)
	{
		if (psz)
			SetTitle(psz);
	}
	inline ~CCheckItem() { }
	void SetTitle(LPTSTR psz)
	{
		if (psz && *psz) {
			TCHAR* buffer = new TCHAR[lstrlen(psz) + 16];
			lstrcpy(buffer, psz);
			TCHAR* p = StrChr(buffer, _T('\n'));
			if (p) {
				*p++ = 0;
				m_tsFollow = p;
			}
			m_tsTitle = buffer;
			delete [] buffer;
		}
		else
			m_tsTitle = psz;
	}
	void SetText(LPTSTR psz)
	{
		m_tsEdit = psz;
	}
	inline bool isGroup() { return ((m_state & CTIS_GROUP) != 0); }
	inline bool isExpanded() { return ((m_state & CTIS_EXPANDED) != 0); }
	inline bool isButton() { return ((m_state & (CTIS_GROUP | CTIS_EDIT)) == 0); }
	inline bool isRadio() { return ((m_state & CTIS_RADIO) != 0); }
	inline bool isEdit() { return ((m_state & CTIS_EDIT) != 0); }
	inline bool isChecked() { return ((m_state & CTIS_CHECKED) != 0); }
	inline bool isDisabled() { return ((m_state & CTIS_DISABLED) != 0); }
	inline void SetCheck(bool bCheck) { m_state = (m_state & ~CTIS_CHECKED) | (bCheck ? CTIS_CHECKED : 0); }
};

class CItemArray : public CSimpleArray<CCheckItem*>
{
public:
	~CItemArray() { DeleteAll(); }

	void DeleteAll()
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			CCheckItem* pi = (*this)[i];
			if (pi) {
				SetAtIndex(i, (CCheckItem*)NULL);
				delete pi;
			}
		}
		RemoveAll();
	}

	BOOL Add(CCheckItem* pi)
	{
		if (pi) pi->m_index = GetSize();
		return CSimpleArray<CCheckItem*>::Add(pi);
	}

	CCheckItem* Lookup(int id)
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			CCheckItem* pi = (*this)[i];
			if (pi && pi->m_id == id)
				return pi;
		}
		return NULL;
	}

	CCheckItem* Lookup(HTREEITEM hItem)
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			CCheckItem* pi = (*this)[i];
			if (pi && pi->m_hItem == hItem)
				return pi;
		}
		return NULL;
	}

	bool IsItemChecked(int id)
	{
		CCheckItem* pi = Lookup(id);
		return (pi ? pi->isChecked() : false);
	}

	CCheckItem* GetRadioGroupTopItem(int id)
	{
		CCheckItem* pi = Lookup(id);
		if (!pi || (pi->m_state & CTIS_RADIO) == 0) return NULL;

		for (int i = pi->m_index; i >= 0; i--) {
			CCheckItem* pip = (*this)[i];
			if (!pip || !pip->isRadio())
				break;
			pi = pip;
		}
		return pi;
	}

	CCheckItem* GetRadioGroupNextItem(int id)
	{
		CCheckItem* pi = Lookup(id);
		if (!pi || (pi->m_state & CTIS_RADIO) == 0) return NULL;

		int cnt = GetSize();
		if (pi->m_index < cnt - 1) {
			pi = (*this)[pi->m_index + 1];
			if (pi->isRadio()) return pi;
		}
		return NULL;
	}
};

///////////////////////////////////////////////////////////////////////
//

typedef ATL::CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | TVS_NOTOOLTIPS , WS_EX_CLIENTEDGE> CMyTreeCtrlTraits;

template <class T, class TBase = CTreeViewCtrl, class TWinTraits = CMyTreeCtrlTraits>
class ATL_NO_VTABLE CMyTreeCtrlImpl: public ATL::CWindowImpl<T, TBase, TWinTraits>
{
private:
	HTREEITEM m_hHotItem;
	BOOL m_bMouseIsTracking;

public:
	CMyTreeCtrlImpl()
		: m_hHotItem(NULL)
		, m_bMouseIsTracking(FALSE)
	{
	}

	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

BEGIN_MSG_MAP(CMyTreeCtrlImpl)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnHandlerMouseMoveHook)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnHandlerMouseLeaveHook)
END_MSG_MAP()

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

	HTREEITEM GetHotItem()
	{
		return m_hHotItem;
	}
};

class CMyTreeCtrl : public CMyTreeCtrlImpl<CMyTreeCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("LSC_CheckTreeCtrl"), GetWndClassName())
};


/////////////////////////////////////////////////////////////////////////////
//

#define IDC_TREE					8765

class CCheckTreeViewCtrl : public CWindowImpl<CCheckTreeViewCtrl>, public CThemeImpl<CCheckTreeViewCtrl>
{
public:
	CMyTreeCtrl m_tree;
	CImageList m_image;
	CItemArray m_array;
	HTREEITEM m_hCurrentRootItem;
	int m_iSpacing;

	CCheckTreeViewCtrl();
	~CCheckTreeViewCtrl();

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}


BEGIN_MSG_MAP(CCheckTreeViewCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CCheckTreeViewCtrl>)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_PRINTCLIENT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_SETFONT()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TREE, OnNotifyCustomDraw)
	ON_NOTIFY(NM_CLICK, IDC_TREE, OnNotifyClick)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE, OnNotifyKeyDown)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_TREE, OnNotifyItemExpanded)
	ON_NOTIFY(TVN_GETDISPINFO, IDC_TREE, OnNotifyGetDispInfo)
	ON_MESSAGE(CTM_ADDITEM, OnAddItem)
	ON_MESSAGE(CTM_GETITEMSTATE, OnGetItemState)
	ON_MESSAGE(CTM_SETITEMSTATE, OnSetItemState)
	ON_MESSAGE(CTM_GETITEMTEXT, OnGetItemText)
	ON_MESSAGE(CTM_SETITEMTEXT, OnSetItemText)
	ON_MESSAGE(CTM_SELECTITEM, OnSelectItem)
	ON_MESSAGE(CTM_GETSPACING, OnGetSpacing)
	ON_MESSAGE(CTM_SETSPACING, OnSetSpacing)
END_MSG_MAP()

	int OnCreate(CREATESTRUCT* pcs);
	void OnPaint();
	inline BOOL OnEraseBkgnd(HDC hdc) { return TRUE; }
	void OnSize(UINT flags, int cx, int cy);
	inline UINT OnGetDlgCode() { return DLGC_WANTARROWS | DLGC_WANTCHARS; }
	inline void OnSetFocus(HWND hOldWnd) { m_tree.SetFocus(); }
	void OnSetFont(HFONT hFont, BOOL bRedraw);
	void OnNotifyCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnAddItem(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetItemState(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetItemState(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetItemText(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetItemText(WPARAM wParam, LPARAM lParam);
	LRESULT OnSelectItem(WPARAM wParam, LPARAM lParam);
	void OnPrintClient(HDC hdc, LPARAM flags);

	LRESULT OnPrePaint(LPNMTVCUSTOMDRAW ptvcd);
	LRESULT OnItemPrePaint(LPNMTVCUSTOMDRAW ptvcd);
	LRESULT OnItemPostPaint(LPNMTVCUSTOMDRAW ptvcd);

	LRESULT OnGetSpacing(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetSpacing(WPARAM wParam, LPARAM lParam);
private:
	void OnDraw(HDC hdc);
	int GetEditItemTitle(CCheckItem* pi, CString& str);
	void EditItemText(CCheckItem* pi);
	bool ToggleCheck(CCheckItem* pi);
	bool RadioCheck(CCheckItem* pi);
	void RedrawItem(HTREEITEM hItem);
	void ResetItemHeight(HFONT hFont = NULL);
};


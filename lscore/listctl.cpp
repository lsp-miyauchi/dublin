///////////////////////////////////////////////////////
// LIST control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"

#ifdef USE_LISTCONTROL

class CListData
{
public:
	CListData();
	~CListData() { }

	CAtlArray<void*> m_array;
	int m_sel;
	int m_top;
	int m_col;				// 列幅
	int m_row;				// 行幅
	int m_numcol;			// 列数
	int m_numrow;			// 行数
	int m_page;
};

CListData::CListData()
{
	m_sel = -1;
	m_top = 0;
	m_col = 32;
	m_row = 32;
	m_numcol = 0;
	m_numrow = 0;
}

//////////////////////////////////////////////////////////////////////
//

class CListCtrl : public CWindowImpl<CListCtrl>
{
private:
	CAtlArray<void*> m_array;
	int m_sel;
	int m_top;
	int m_col;				// 列幅
	int m_row;				// 行幅
	int m_numcol;			// 列数
	int m_numrow;			// 行数
	int m_page;

public:
	CListCtrl();
	~CListCtrl();

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CListCtrl)
	ON_MESSAGE(WM_GETDLGCODE, OnGetDlgCode)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_DESTROY()
	ON_MESSAGE(OLM_RESETCONTENT, OnResetContent)
	ON_MESSAGE(OLM_SETCURSEL, OnSetCurSel)
	ON_MESSAGE(OLM_GETCURSEL, OnGetCurSel)
	ON_MESSAGE(OLM_GETTOPINDEX, OnGetTopIndex)
//@	ON_MESSAGE(OLM_SETTOPINDEX, OnSetTopIndex)
	ON_MESSAGE(OLM_GETCOUNT, OnGetCount)
	ON_MESSAGE(OLM_ADDDATA, OnAddData)
	ON_MESSAGE(OLM_DELETEDATA, OnDeleteData)
	ON_MESSAGE(OLM_INSERTDATA, OnInsertData)
	ON_MESSAGE(OLM_GETITEMDATA, OnGetItemData)
	ON_MESSAGE(OLM_SETITEMDATA, OnSetItemData)
	ON_MESSAGE(OLM_GETITEMRECT, OnGetItemRect)
END_MSG_MAP()

	LRESULT OnGetDlgCode(WPARAM wParam, LPARAM lParam);
	int OnCreate(LPCREATESTRUCT pcs);
	void OnSize(UINT flag, int cx, int cy);
	void OnSetFocus(HWND hOldWnd);
	void OnKillFocus(HWND hNewWnd);
	void OnPaint();
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnDestroy();
	BOOL OnEraseBkgnd(HDC hdc);
	LRESULT OnResetContent(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCurSel(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetCurSel(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetTopIndex(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetTopIndex(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetCount(WPARAM wParam, LPARAM lParam);
	LRESULT OnAddData(WPARAM wParam, LPARAM lParam);
	LRESULT OnDeleteData(WPARAM wParam, LPARAM lParam);
	LRESULT OnInsertData(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetItemData(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetItemData(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetItemRect(WPARAM wParam, LPARAM lParam);

	void Recalc();
	void ChangeFocus(BOOL bFocus);
	int SetCurSel(int index);
	int SetTopIndex(int index);
	void Scroll(BOOL bUp);
	int GetItemRect(int index, LPRECT prc);
};

CListCtrl::CListCtrl()
{
	m_sel = -1;
	m_top = 0;
	m_col = 32;
	m_row = 32;
	m_numcol = 0;
	m_numrow = 0;
}

CListCtrl::~CListCtrl()
{
	m_array.RemoveAll();
}

_inline LRESULT CListCtrl::OnGetDlgCode(WPARAM wParam, LPARAM lParam)
	{ return DLGC_WANTALLKEYS; }

int CListCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
	ModifyStyle(0, WS_VSCROLL);
	return 0;
}

void CListCtrl::OnSize(UINT flg, int cx, int cy)
{
	if (flg != SIZE_MINIMIZED)
		Recalc();
}

void CListCtrl::Recalc()
{
	CRect rc;
	GetClientRect(&rc);

	int cnt = (int)m_array.GetCount();
	m_numcol = rc.Width() / m_col;				// 列数
	m_numrow = cnt / m_numcol;					// 行数
	if (cnt % m_numcol)
		m_numrow++;
	m_page = m_numcol * (rc.Height() / m_row);	// ページサイズ

	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE;
	si.nPage = m_row * (rc.Height() / m_row);
	si.nMin = 0;
	si.nMax = m_row * m_numrow;
	SetScrollInfo(SB_VERT, &si, TRUE);
}

_inline void CListCtrl::OnSetFocus(HWND hwndPrev)
	{ ChangeFocus(TRUE); }

_inline void CListCtrl::OnKillFocus(HWND hwndNext)
	{ ChangeFocus(FALSE); }

void CListCtrl::ChangeFocus(BOOL bFocus)
{
	if (m_array.GetCount() > 0 && m_sel >= 0) {
		CRect rc;
		GetItemRect(m_sel, &rc);

		CClientDC dc(m_hWnd);
		DRAWITEMSTRUCT di;
		di.hDC = dc;
		di.itemID = m_sel;
		di.CtlType = ODT_LISTBOX;
		di.CtlID = GetDlgCtrlID();
		di.itemAction = ODA_FOCUS;
		di.itemState = ODS_SELECTED | (bFocus ? ODS_FOCUS : 0);
		di.hwndItem = m_hWnd;
		di.itemData = (DWORD_PTR)m_array[m_sel];
		GetParent().SendMessage(WM_DRAWITEM, di.CtlID, (LPARAM)&di);
	}
}

void CListCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);

	DRAWITEMSTRUCT di;
	di.hDC = dc;
	di.CtlType = ODT_LISTBOX;
	di.CtlID = GetDlgCtrlID();
	di.itemAction = ODA_DRAWENTIRE;
	di.hwndItem = m_hWnd;

	int cnt = (int)m_array.GetCount();
	for (int i = m_top; i < cnt; i++) {
		GetItemRect(i, &di.rcItem);
		if (RectVisible(dc, &di.rcItem)) {
			di.itemID = i;
			di.itemState = (m_sel == i) ? ODS_SELECTED : 0;
			if (di.itemState)
				di.itemState |= (GetFocus() == m_hWnd) ? ODS_FOCUS : 0;
			di.itemData = (DWORD_PTR)m_array[i];
			GetParent().SendMessage(WM_DRAWITEM, di.CtlID, (LPARAM)&di);
		}
	}
}

void CListCtrl::OnDestroy()
{
	OnResetContent(0, 0);
}

void CListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int cnt = (int)m_array.GetCount();
	if (cnt > 0) {
		int sel = m_sel;
		if (sel < 0) sel = 0;

		if (nChar == VK_DOWN) {
			if (sel + m_numcol < cnt) {
				sel += m_numcol;
				if (sel > m_top + m_page)
					Scroll(TRUE);
				SetCurSel(sel);
				return;
			}
	    }
		else if (nChar == VK_UP) {
			if (sel - m_numcol >= 0) {
				sel -= m_numcol;
				if (sel < m_top)
					Scroll(FALSE);
				SetCurSel(sel);
				return;
			}
		}
		else if (nChar == VK_RIGHT) {
			if (sel + 1 < cnt)
				sel++;
		}
		else if (nChar == VK_LEFT) {
			if (sel - 1 >= 0)
				sel--;
		}
		else if (nChar == VK_PRIOR) {
			sel = max(0, sel - m_page);
		}
		else if (nChar == VK_NEXT) {
			sel = min(cnt - 1, sel + m_page);
		}
		else if (nChar == VK_HOME) {
			sel = 0;
		}
		else if (nChar == VK_END) {
			sel = cnt - 1;
		}
		else {
			DefWindowProc();
			return;
		}

		if (m_sel != sel)
			SetCurSel(sel);
		return;
	}
	DefWindowProc();
}

LRESULT CListCtrl::OnResetContent(WPARAM wParam, LPARAM lParam)
{
	DELETEITEMSTRUCT ds;
	memset(&ds, 0, sizeof(ds));
	ds.CtlType = ODT_LISTBOX;
	ds.CtlID = GetDlgCtrlID();
	ds.hwndItem = m_hWnd;
	ds.itemID = 0;
	while (m_array.GetCount() > 0) {
		ds.itemData = (DWORD_PTR)m_array[0];
		GetParent().SendMessage(WM_DELETEITEM, ds.CtlID, (LPARAM)&ds);
		m_array.RemoveAt(0);
	}
	Recalc();
	Invalidate(TRUE);
	return 0;
}

_inline LRESULT CListCtrl::OnSetCurSel(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)SetCurSel((int)wParam); }

int CListCtrl::SetCurSel(int index)
{
	if (m_sel != index) {
		int oldsel = m_sel;
		m_sel = index;

		CRect rc;
		GetItemRect(oldsel, &rc);
		InvalidateRect(&rc, TRUE);

		if (m_top > index) {
			// 下スクロール
		}
		else if (m_top + m_page < index)
		{
			// 上スクロール
		}
		GetItemRect(index, &rc);
		InvalidateRect(&rc, TRUE);
	}
	return m_sel;
}

_inline LRESULT CListCtrl::OnGetCurSel(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)m_sel; }

_inline LRESULT CListCtrl::OnGetTopIndex(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)m_top; }

_inline LRESULT CListCtrl::OnSetTopIndex(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)SetTopIndex((int)wParam); }

int CListCtrl::SetTopIndex(int index)
{
	int cnt = (int)m_array.GetCount();
	if (cnt > 0 && index >= 0 && index < cnt) {
		if (m_top != index) {
			m_top = index;
			Recalc();
			Invalidate(TRUE);
		}
		return index;
	}
	return -1;
}

_inline LRESULT CListCtrl::OnGetCount(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)m_array.GetCount(); }

void CListCtrl::Scroll(BOOL bUp)
{
	CRect rc;
	GetClientRect(&rc);
	CRect rcScroll = rc;
	int nScroll = 0;
	if (bUp) {
		if (m_top + m_page + m_numcol < (int)m_array.GetCount()) {
			rcScroll.top += m_row;
			rc.top = rc.bottom - m_row;
			nScroll = -m_row;
			m_top += m_numcol;
		}
	}
	else {
		if (m_top > 0) {
			rcScroll.bottom -= m_row;
			rc.bottom = rc.top + m_row;
			nScroll = m_row;
			m_top -= m_numcol;
		}
	}

	if (nScroll) {
		ScrollWindow(0, nScroll, &rcScroll, &rcScroll);
		InvalidateRect(&rc, TRUE);
	}
}

LRESULT CListCtrl::OnAddData(WPARAM wParam, LPARAM lParam)
{
	m_array.Add((void*)lParam);
	Recalc();

	int index = (int)m_array.GetCount() - 1;
	CRect rc;
	GetItemRect(index, &rc);
	InvalidateRect(&rc, TRUE);
	return (LRESULT)index;
}

LRESULT CListCtrl::OnDeleteData(WPARAM wParam, LPARAM lParam)
{
	int cnt = (int)m_array.GetCount();
	int index = (int)wParam;
	if (cnt > 0 && index >= 0 && cnt < index) {
		DELETEITEMSTRUCT ds;
		memset(&ds, 0, sizeof(ds));
		ds.CtlType = ODT_LISTBOX;
		ds.CtlID = GetDlgCtrlID();
		ds.itemID = index;
		ds.hwndItem = m_hWnd;
		ds.itemData = (DWORD_PTR)m_array[index];
		GetParent().SendMessage(WM_DELETEITEM, ds.CtlID, (LPARAM)&ds);
		m_array.RemoveAt(index);

		Recalc();
		Invalidate(TRUE);

		return (LRESULT)(cnt - 1);
	}
	return (LRESULT)-1;
}

LRESULT CListCtrl::OnInsertData(WPARAM wParam, LPARAM lParam)
{
	int index = (int)wParam;
	m_array.InsertAt(index, (void*)lParam);
	Recalc();

	Invalidate(TRUE);
	return (LRESULT)index;
}

LRESULT CListCtrl::OnGetItemData(WPARAM wParam, LPARAM lParam)
{
	int cnt = (int)m_array.GetCount();
	int index = (int)wParam;
	if (cnt > 0 && index >= 0 && cnt < index)
		return (LRESULT)m_array[index];
	return 0;
}

LRESULT CListCtrl::OnSetItemData(WPARAM wParam, LPARAM lParam)
{
	int cnt = (int)m_array.GetCount();
	int index = (int)wParam;
	if (cnt > 0 && index >= 0 && cnt < index) {
		m_array[index] = (void*)lParam;
		return (LRESULT)index;
	}
	return (LRESULT)-1;
}

_inline LRESULT CListCtrl::OnGetItemRect(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)GetItemRect((int)wParam, (LPRECT)lParam); }

int CListCtrl::GetItemRect(int index, LPRECT prc)
{
	int cnt = (int)m_array.GetCount();
	if (prc && cnt > 0 && index >= 0 && index < cnt) {
		int x = m_col * (index % m_numcol);
		int y = m_row * (index / m_numcol);
		CRect rc(0, 0, m_col, m_row);
		rc.OffsetRect(x, y);
		rc.OffsetRect(0, -(m_row * (m_top / m_col)));
		CopyRect(prc, &rc);
		return index;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK ListWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CListCtrl* pWnd = new CListCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}


#endif			// USE_LISTCONTROL

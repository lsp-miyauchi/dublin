//////////////////////////////////////////////////////////////////////
// mousewheeler.h

#pragma once
#ifndef __MOUSEWHEELER_H__
#define __MOUSEWHEELER_H__

///////////////////////////////////////////////////////////////////////
//

template <class T>
class CMouseTrackerImpl
{
private:
	BOOL m_bMouseIsTracking;

public:
	CMouseTrackerImpl() : m_bMouseIsTracking(FALSE)
	{
	}

BEGIN_MSG_MAP(CMouseTrackerImpl)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnHandlerMTMouseMoveHook)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnHandlerMTMouseLeaveHook)
END_MSG_MAP()

	LRESULT OnHandlerMTMouseMoveHook(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

	LRESULT OnHandlerMTMouseLeaveHook(UINT msg, WPARAM, LPARAM, BOOL& bHandled)
	{
		m_bMouseIsTracking = FALSE;
		T* pT = static_cast<T*>(this);
		pT->MouseLeave();
		bHandled = FALSE;
		return 0;
	}

	virtual void MouseMove(POINT pt) { }
	virtual void MouseLeave() { }
};

///////////////////////////////////////////////////////////////////////
//

template <class T>
class CMouseWheelerImpl
{
private:

BEGIN_MSG_MAP(CMouseWheelerImpl)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnHandlerMWMouseWheelHook)
END_MSG_MAP()

	LRESULT OnHandlerMWMouseWheelHook(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		T* pT = static_cast<T*>(this);
		CRect rc;
		pT->GetWindowRect(&rc);
		if (!rc.PtInRect(pt)) {
			CWindow wnd = pT->GetParent();
			if (wnd.IsWindow())
				return wnd.SendMessage(msg, wParam, lParam);
		}
		bHandled = FALSE;
		return 0;
	}
};

///////////////////////////////////////////////////////////////////////
//

class CMWListViewCtrl :	public CWindowImpl<CMWListViewCtrl, CListViewCtrl>,
						public CMouseWheelerImpl<CMWListViewCtrl>,
						public CMouseTrackerImpl<CMWListViewCtrl>
{
private:

BEGIN_MSG_MAP(CMWListViewCtrl)
	CHAIN_MSG_MAP(CMouseWheelerImpl<CMWListViewCtrl>)
	CHAIN_MSG_MAP(CMouseTrackerImpl<CMWListViewCtrl>)
END_MSG_MAP()

	void MouseMove(POINT pt)
	{
		LVHITTESTINFO ht;
		ht.pt = pt;
		ListView_HitTest(m_hWnd, &ht);
		int index  = (ht.flags & LVHT_ONITEM) ? ht.iItem : -1;
		int iHot = ListView_GetHotItem(m_hWnd);
		if (index == iHot)
			return;

		ListView_SetHotItem(m_hWnd, index);
		CRect rc;
		if (iHot >= 0) {
			ListView_GetItemRect(m_hWnd, iHot, &rc, LVIR_BOUNDS);
			InvalidateRect(&rc);
		}
		if (index >= 0) {
			ListView_GetItemRect(m_hWnd, index, &rc, LVIR_BOUNDS);
			InvalidateRect(&rc);
		}
	}

	void MouseLeave()
	{
		int iHot = ListView_GetHotItem(m_hWnd);
		if (iHot >= 0) {
			ListView_SetHotItem(m_hWnd, -1);
			CRect rc;
			ListView_GetItemRect(m_hWnd, iHot, &rc, LVIR_BOUNDS);
			InvalidateRect(&rc);
		}
	}
};

///////////////////////////////////////////////////////////////////////
//

class CMWTreeViewCtrl :	public CWindowImpl<CMWTreeViewCtrl, CTreeViewCtrl>,
						public CMouseWheelerImpl<CMWTreeViewCtrl>,
						public CMouseTrackerImpl<CMWTreeViewCtrl>
{
private:
	bool m_bHotTrackEnabled;
	HTREEITEM m_hHotItem;

BEGIN_MSG_MAP(CMWTreeViewCtrl)
	CHAIN_MSG_MAP(CMouseWheelerImpl<CMWTreeViewCtrl>)
	CHAIN_MSG_MAP(CMouseTrackerImpl<CMWTreeViewCtrl>)
	MESSAGE_HANDLER(WM_CREATE, OnHandlerMwtvCreate)
	MESSAGE_HANDLER(WM_KEYDOWN, OnHandlerMwtvKeyDown)
END_MSG_MAP()

	LRESULT OnHandlerMwtvCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bHotTrackEnabled = true;
		m_hHotItem = NULL;
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnHandlerMwtvKeyDown(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		switch (wParam) {
		case VK_MULTIPLY:
		case VK_ADD:
		case VK_SUBTRACT:
			// *, +, - でフォルダの開閉をさせない
			return 0;
		}
		bHandled = FALSE;
		return 1;
	}

	void EnableHotTrack(bool bEnable)
	{
		bool bPrevSetting = m_bHotTrackEnabled;
		m_bHotTrackEnabled = bEnable;
		if (bPrevSetting)
			MouseLeave();
		else
			m_hHotItem = NULL;
	}

	void RedrawTreeItem(HTREEITEM hItem)
	{
		if (hItem) {
			CRect rc;
			if (TreeView_GetItemRect(m_hWnd, hItem, &rc, TRUE))
				InvalidateRect(&rc, FALSE);
		}
	}

	void MouseMove(POINT pt)
	{
		TVHITTESTINFO ht;
		ht.pt = pt;
		TreeView_HitTest(m_hWnd, &ht);
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

	HTREEITEM GetHotItem()
	{
		return m_bHotTrackEnabled ? m_hHotItem : NULL;
	}
};

///////////////////////////////////////////////////////////////////////
//

template <class T>
class CMouseWheelServerImpl
{
private:
	bool m_bMouseWheelProcessing:1;
public:

	CMouseWheelServerImpl() : m_bMouseWheelProcessing(false)
	{
	}

BEGIN_MSG_MAP(CMouseWheelServerImpl)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnHandlerMouseWheel)
END_MSG_MAP()

	LRESULT OnHandlerMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lr = 0;
		if (!m_bMouseWheelProcessing) {
			CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			CWindow wnd = WindowFromPoint(pt);
			T* pT = static_cast<T*>(this);
			if (pT->IsChild(wnd)) {
				m_bMouseWheelProcessing = true;
				lr = wnd.SendMessage(msg, wParam, lParam);
				m_bMouseWheelProcessing = false;
			}
		}
		return lr;
	}
};


#endif			// __MOUSEWHEELER_H__

//////////////////////////////////////////////////////////////////////
// mousewheeler.h

#pragma once
#ifndef __MOUSEWHEELER_H__
#define __MOUSEWHEELER_H__

///////////////////////////////////////////////////////////////////////
//

template <class T>
class CMouseWheelerImpl
{
private:

BEGIN_MSG_MAP(CMouseWheelerImpl)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnHandlerMouseWheelHook)
END_MSG_MAP()

	LRESULT OnHandlerMouseWheelHook(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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
						public CMouseWheelerImpl<CMWListViewCtrl>
{
public:
BEGIN_MSG_MAP(CMWListViewCtrl)
	CHAIN_MSG_MAP(CMouseWheelerImpl<CMWListViewCtrl>)
END_MSG_MAP()
};

///////////////////////////////////////////////////////////////////////
//

class CMWTreeViewCtrl :	public CWindowImpl<CMWTreeViewCtrl, CTreeViewCtrl>,
						public CMouseWheelerImpl<CMWTreeViewCtrl>
{
BEGIN_MSG_MAP(CMWTreeViewCtrl)
	CHAIN_MSG_MAP(CMouseWheelerImpl<CMWTreeViewCtrl>)
END_MSG_MAP()
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

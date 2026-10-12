////////////////////////////////////////////////////////////////////////
// waiting.h

#pragma once
#ifndef __WAITING_H__
#define __WAITING_H__

HIMAGELIST ImageList_LoadFromResourceData(UINT idResource, int itemWidth, COLORREF clrMask);

class CWaitingMarkerWindow : public CWindowImpl<CWaitingMarkerWindow>
{
private:
	HIMAGELIST m_him;
	int m_bmx;
	int m_bmy;
	CAnimationImageCtrl m_anim;

public:
	CWaitingMarkerWindow() : m_him(NULL), m_bmx(0), m_bmy(0)
	{
	}

BEGIN_MSG_MAP(CWaitingMarkerWindow)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		m_him = ImageList_LoadFromResourceData(IDB_WAITMARKER, 45, RGB(255, 0, 255));
//@		m_him = lccLIMHorzMarker();
		if (!m_him) return -1;

		ImageList_GetIconSize(m_him, &m_bmx, &m_bmy);

		CRect rc(0, 0, m_bmx, m_bmy);
		m_anim.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE, 0);
		m_anim.SetBkColor(GetSysColor(COLOR_3DFACE));
		m_anim.SetAnimationImage(m_him, 50);

		SetWindowPos(NULL, 0, 0, m_bmx, m_bmy, SWP_NOZORDER | SWP_NOMOVE);
		return 0;
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		return TRUE;
	}

	void OnDestroy()
	{
		if (m_him) {
			m_anim.Stop();
			ImageList_Destroy(m_him);
		}
	}

	void Start()
	{
		if (::IsWindow(m_anim))
			m_anim.Start();
	}
};


#endif		// __WAITING_H__

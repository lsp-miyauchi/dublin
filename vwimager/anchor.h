// anchor.h : ヘッダー ファイル
//

#pragma once
#ifndef __ANCHOR_H__
#define __ANCHOR_H__

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CAncorWnd ウィンドウ

#define ANCHOR_ARROW_SIZE				23
#define ANCHOR_BLOCK_SIZE				24

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CAnchorWndImpl : public CWindowImpl< T, TBase, TWinTraits>
{
private:
	HIMAGELIST m_him;
	int m_anchor;

public:
	CAnchorWndImpl()
	{
		m_him = NULL;
		m_anchor = 0;
	}

	~CAnchorWndImpl()
	{
		if (m_him) ImageList_Destroy(m_him);
	}

	DECLARE_WND_CLASS(NULL)

	int GetAnchor()
	{
		return m_anchor;
	}

	void SetAnchor(int anchor)
	{
		if (anchor >= 0 && anchor < 9 && anchor != m_anchor) {
			m_anchor = anchor;
			Invalidate();
		}
	}

BEGIN_MSG_MAP(this)
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_MESSAGE(WM_ENABLE, OnEnableHandler)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		m_him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_ANCHOR), 23, CLR_NONE);
		m_anchor = 4;
		return 0;
	}

	BOOL OnSetCursor(HWND hWnd, UINT nHitTest, UINT message)
	{
		if (nHitTest == HTCLIENT) {
			SetCursor(LoadCursor(NULL, IDC_HAND));
			return TRUE;
		}
		return (BOOL)DefWindowProc();
	}

	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		DefWindowProc();
		int yan = point.y / ANCHOR_BLOCK_SIZE;
		int xan = point.x / ANCHOR_BLOCK_SIZE;
		if (yan < 0 || yan > 2 || xan < 0 || xan > 2)
			return;

		int index = yan * 3 + xan;
		if (index != m_anchor) {
			m_anchor = index;
			Invalidate();
		}
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		return TRUE;
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);

		BOOL bWindowEnabled = ::IsWindowEnabled(m_hWnd);

		CRect rc, rcb;
		GetClientRect(&rc);
		int x, y, index = 0;
		for (y = 0; y < 3; y++) {
			for (x = 0; x < 3; x++) {
				rcb.SetRect(0, 0, ANCHOR_BLOCK_SIZE, ANCHOR_BLOCK_SIZE);
				rcb.OffsetRect(rc.left + x * ANCHOR_BLOCK_SIZE, rc.top + y * ANCHOR_BLOCK_SIZE);

				if (m_anchor == index) {
					dc.FillSolidRect(&rcb, GetSysColor(COLOR_WINDOW));
				}
				else {
					dc.FillSolidRect(&rcb, GetSysColor(COLOR_3DFACE));
					if (bWindowEnabled)
						dc.DrawEdge(&rcb, BDR_RAISEDINNER, BF_TOPLEFT | BF_BOTTOMRIGHT);
				}
				index++;
			}
		}

		int xan = m_anchor % 3;
		int yan = m_anchor / 3;
		rcb.SetRect(0, 0, ANCHOR_BLOCK_SIZE, ANCHOR_BLOCK_SIZE);
		rcb.OffsetRect(xan * ANCHOR_BLOCK_SIZE, yan * ANCHOR_BLOCK_SIZE);
		CPoint pt = rcb.CenterPoint();
		for (y = -1, index = 0; y <= 1; y++) {
			for (x = -1; x <= 1; x++) {
				if (index != 4 && (0 <= yan + y) && (yan + y <= 2) && (0 <= xan + x) && (xan + x <= 2)) {
					CPoint pta = pt;
					if      (x < 0)	pta.x -= (ANCHOR_ARROW_SIZE + ANCHOR_ARROW_SIZE / 4);
					else if (x > 0)	pta.x += ANCHOR_ARROW_SIZE / 4;
					else			pta.x -= ANCHOR_ARROW_SIZE / 2;

					if      (y < 0)	pta.y -= (ANCHOR_ARROW_SIZE + ANCHOR_ARROW_SIZE / 4);
					else if (y > 0)	pta.y += ANCHOR_ARROW_SIZE / 4;
					else			pta.y -= ANCHOR_ARROW_SIZE / 2;

					UINT flag = ILD_TRANSPARENT;
					COLORREF clr = CLR_NONE;
					if (!bWindowEnabled) {
						clr = RGB(255, 255, 255);
						flag |= ILD_BLEND50;
					}
					ImageList_DrawEx(m_him, index, dc, pta.x, pta.y, 0, 0, CLR_NONE, clr, flag);
				}
				index++;
			}
		}
	}

	LRESULT OnEnableHandler(WPARAM wParam, LPARAM lParam)
	{
		LRESULT lr = DefWindowProc();
		Invalidate();
		return lr;
	}
};

class CAnchorWnd : public CAnchorWndImpl<CAnchorWnd>
{
public:
};



#endif				// __ANCHOR_H__

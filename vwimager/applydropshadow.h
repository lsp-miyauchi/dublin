/////////////////////////////////////////////////////////////////////////////
// applydropshadow.h
//

#pragma once
#ifndef __APPLYDROPSHADOW_H__
#define __APPLYDROPSHADOW_H__

#include "resource.h"

class CApplyDropShadow
{
private:
	HIMAGELIST m_himParts;

	enum {
		m_deltaShadow = 5
	};

public:
	CApplyDropShadow() : m_himParts(NULL)
	{
		m_himParts = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_DROPSHADOW_PARTS), m_deltaShadow, RGB(255, 0, 255));
	}

	~CApplyDropShadow()
	{
		if (m_himParts) ImageList_Destroy(m_himParts);
	}

	int delta() const
	{
		return m_deltaShadow;
	}

	void Apply(HDC hdc, const RECT* prc)
	{
		CRect rc = *prc;
		if (m_himParts) {
			CDCHandle dc = hdc;

			int save = dc.SaveDC();

			// 左下
			ImageList_Draw(m_himParts, 0, dc, rc.left, rc.bottom - m_deltaShadow, ILD_TRANSPARENT);
			dc.ExcludeClipRect(rc.left, rc.bottom - m_deltaShadow, rc.left + m_deltaShadow, rc.bottom);
			// 右下
			ImageList_Draw(m_himParts, 1, dc, rc.right - m_deltaShadow, rc.bottom - m_deltaShadow, ILD_TRANSPARENT);
			dc.ExcludeClipRect(rc.right - m_deltaShadow, rc.bottom - m_deltaShadow, rc.right, rc.bottom);
			// 右上
			ImageList_Draw(m_himParts, 2, dc, rc.right - m_deltaShadow, rc.top, ILD_TRANSPARENT);
			dc.ExcludeClipRect(rc.right - m_deltaShadow, rc.top, rc.right, rc.top + m_deltaShadow);
			// 底辺
			int x = rc.left + m_deltaShadow;
			while (x < rc.right - m_deltaShadow) {
				ImageList_Draw(m_himParts, 3, dc, x, rc.bottom - m_deltaShadow, ILD_TRANSPARENT);
				x += m_deltaShadow;
			}
			// 右辺
			int y = rc.top + m_deltaShadow;
			while (y < rc.bottom - m_deltaShadow) {
				ImageList_Draw(m_himParts, 4, dc, rc.right - m_deltaShadow, y, ILD_TRANSPARENT);
				y += m_deltaShadow;
			}
			dc.RestoreDC(save);
		}
		else {
			Graphics g(hdc);
			COLORREF clr = GetSysColor(COLOR_3DSHADOW);
			SolidBrush sbr(Color(128, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
			// 描画不可部分を除外
			Rect r(rc.left, rc.top, rc.Width() - m_deltaShadow, rc.Height() - m_deltaShadow);
			g.ExcludeClip(r);
			// 影を描画
			r.Offset(3, 3);
			g.FillRectangle(&sbr, r);
		}
	}
};



#endif		// __APPLYDROPSHADOW_H__

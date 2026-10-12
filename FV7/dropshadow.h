/////////////////////////////////////////////////////////////////////////////
// dropshadow.h ドロップシャドウ
//

#pragma once
#ifndef __DROPSHADOW_H__
#define __DROPSHADOW_H__

Bitmap* LoadImageFromResource(HINSTANCE hInst, LPCTSTR pszResourceType, LPCTSTR pszResourceName);

class CApplyDropShadow
{
private:
	HIMAGELIST m_himParts;
	int m_deltaShadow;

public:
	CApplyDropShadow()
		: m_himParts(NULL)
		, m_deltaShadow(0)
	{
		m_deltaShadow = 4;
		m_himParts = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_DROPSHADOW_PARTS), m_deltaShadow, RGB(255, 0, 255));
	}

	~CApplyDropShadow()
	{
		if (m_himParts)
			ImageList_Destroy(m_himParts);
	}

	int delta() const
	{
		return m_deltaShadow;
	}

	void Apply(Graphics& g, const RECT* prc)
	{
		CRect rc = *prc;
		if (m_himParts) {
			CDCHandle dc = g.GetHDC();
			int save = dc.SaveDC();

			rc.DeflateRect(m_deltaShadow, m_deltaShadow);
			// 左下
			ImageList_Draw(m_himParts, 0, dc, rc.left - m_deltaShadow, rc.bottom, ILD_TRANSPARENT);
			dc.ExcludeClipRect(rc.left - m_deltaShadow, rc.bottom, rc.left, rc.bottom + m_deltaShadow);
			// 右下
			ImageList_Draw(m_himParts, 1, dc, rc.right, rc.bottom, ILD_TRANSPARENT);
			dc.ExcludeClipRect(rc.right, rc.bottom, rc.right + m_deltaShadow, rc.bottom + m_deltaShadow);
			// 右上
			ImageList_Draw(m_himParts, 2, dc, rc.right, rc.top - m_deltaShadow, ILD_TRANSPARENT);
			dc.ExcludeClipRect(rc.right, rc.top - m_deltaShadow, rc.right + m_deltaShadow, rc.top);
			// 底辺
			for (int x = rc.left; x < rc.right; x += m_deltaShadow)
				ImageList_Draw(m_himParts, 3, dc, x, rc.bottom, ILD_TRANSPARENT);
			// 右辺
			for (int y = rc.top; y < rc.bottom; y += m_deltaShadow)
				ImageList_Draw(m_himParts, 4, dc, rc.right, y, ILD_TRANSPARENT);
			dc.RestoreDC(save);
		}
		else {
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

	void Apply(HDC hdc, const RECT* prc)
	{
		Graphics g(hdc);
		Apply(g, prc);
	}
};


#endif		// __DROPSHADOW_H__


/////////////////////////////////////////////////////////////////////////
// buttonframe.h

#pragma once
#ifndef __BUTTONFRAME_H__
#define __BUTTONFRAME_H__

#ifndef __GDIPLUS_SUP_H__
#include "gdiplus_sup.h"
#endif


#define CDIS_PRESSED			0x80000
#define CDIS_DROPDOWN			0x40000

#define ROUND_TOPLEFT			0x0001
#define ROUND_TOPRIGHT			0x0002
#define ROUND_BOTTOMLEFT		0x0004
#define ROUND_BOTTOMRIGHT		0x0008
#define ROUND_ALL				(ROUND_TOPLEFT | ROUND_TOPRIGHT | ROUND_BOTTOMLEFT | ROUND_BOTTOMRIGHT)
#define ROUND_TOP				(ROUND_TOPLEFT | ROUND_TOPRIGHT)
#define ROUND_BOTTOM			(ROUND_BOTTOMLEFT | ROUND_BOTTOMRIGHT)
#define ROUND_LEFT				(ROUND_TOPLEFT | ROUND_BOTTOMLEFT)
#define ROUND_RIGHT				(ROUND_TOPRIGHT | ROUND_BOTTOMRIGHT)


inline void iLspGdipRoundRect(Graphics& g, Pen* pen, Brush* brush, RectF& r, CSize size, UINT flag)
{
	// Create a GraphicsPath, and add an ellipse.
	REAL left = r.X;
	REAL right = r.GetRight();
	REAL top = r.Y;
	REAL bottom = r.GetBottom();
	SizeF sf((Gdiplus::REAL)size.cx, (Gdiplus::REAL)size.cy);

	GraphicsPath p;
	p.AddLine(left + sf.Width / 2, top, right - sf.Width / 2, top);
	if (flag & ROUND_TOPRIGHT) {
		// 右上角を丸く
		p.AddArc(right - sf.Width, top, sf.Width, sf.Height, 270.0, 90.0);
	} else {
		p.AddLine(right - sf.Width / 2, top, right, top);
		p.AddLine(right, top, right, top + sf.Height / 2);
	}
	p.AddLine(right, top + sf.Height / 2, right, bottom - sf.Height / 2);
	if (flag & ROUND_BOTTOMRIGHT) {
		// 右下角を丸く
		p.AddArc(right - sf.Width, bottom - sf.Height, sf.Width, sf.Height, 0, 90);
	} else {
		p.AddLine(right, bottom - sf.Height / 2, right, bottom);
		p.AddLine(right, bottom, right - sf.Width / 2, bottom);
	}
	p.AddLine(right - sf.Width / 2, bottom, left + sf.Width / 2, bottom);
	if (flag & ROUND_BOTTOMLEFT) {
		// 左下角を丸く
		p.AddArc(left, bottom - sf.Height, sf.Width, sf.Height, 90, 90);
	} else {
		p.AddLine(left + sf.Width / 2, bottom, left, bottom);
		p.AddLine(left, bottom, left, bottom - sf.Height / 2);
	}
	p.AddLine(left, bottom - sf.Height / 2, left, top + sf.Height / 2);
	if (flag & ROUND_TOPLEFT) {
		// 左上角を丸く
		p.AddArc(left, top, sf.Width, sf.Height, 180, 90);
	} else {
		p.AddLine(left, top + sf.Height / 2, left, top);
		p.AddLine(left, top, left + sf.Width / 2, top);
	}

	if (brush) g.FillPath(brush, &p);			// Fill Path.
	if (pen) g.DrawPath(pen, &p);				// Draw Path.
}

inline void iLspDrawButtonBackground(HDC hdc, const CRect& rect, UINT state, UINT roundFlags = ROUND_ALL)
{
	if (state & (CDIS_HOT | CDIS_PRESSED | CDIS_CHECKED | CDIS_SELECTED | CDIS_DROPDOWN)) {
		Graphics g(hdc);

		g.SetSmoothingMode(SmoothingModeHighQuality);
		RectF r((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.Width() - 1, (Gdiplus::REAL)rect.Height() - 1);

		COLORREF clr;
		Color penColor, brushColor;
		if (state & CDIS_DROPDOWN) {
			clr = COLOR_VISTA_MENUBKGND;
			penColor.SetValue(Color::MakeARGB(96, 0, 0, 0));
			brushColor.SetValue(Color::MakeARGB(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));

			Pen pen(penColor, 1);
			SolidBrush brush(brushColor);
			iLspGdipRoundRect(g, &pen, &brush, r, CSize(5, 5), roundFlags);
		} else if (state & CDIS_PRESSED) {
			LinearGradientBrush brush(r, Color(0, 0, 0, 0), Color(24, 0, 0, 0), LinearGradientModeVertical);
			penColor.SetValue(Color::MakeARGB(96, 0, 0, 0));
			Color colors[2];
			brush.GetLinearColors(colors);
			brush.SetLinearColors(colors[1], colors[0]);
			Pen pen(penColor, 1);
			iLspGdipRoundRect(g, &pen, &brush, r, CSize(5, 5), roundFlags);
		} else {
			// state & (CDIS_CHECKED | CDIS_SELECTED | CDIS_HOT)
			if (state & (CDIS_CHECKED | CDIS_SELECTED)) {
				clr = GetSysColor(COLOR_HIGHLIGHT);
				penColor.SetValue(Color::MakeARGB(192, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
				if (state & CDIS_HOT) {
					brushColor.SetValue(Color::MakeARGB(32, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
				} else {
					clr = COLOR_VISTA_MENUBKGND;
					brushColor.SetValue(Color::MakeARGB(224, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
				}

				Pen pen(penColor, 1);
				SolidBrush brush(brushColor);
				iLspGdipRoundRect(g, &pen, &brush, r, CSize(5, 5), roundFlags);
			} else if (state & CDIS_HOT) {
/*
				penColor.SetValue(Color::MakeARGB(64, 0, 0, 0));
				LinearGradientBrush brush(r, Color(0, 0, 0, 0), Color(24, 0, 0, 0), LinearGradientModeVertical);
				REAL factors[4] = {0.0f, 0.0f, 1.0f, 0.2f};
				REAL positions[4] = {0.0f, 0.6f, 0.7f, 1.0f};
				brush.SetBlend(factors, positions, 4);
				Pen pen(penColor, 1);
				iLspGdipRoundRect(g, &pen, &brush, r, CSize(5, 5), roundFlags);
*/
				penColor.SetValue(Color::MakeARGB(64, 0, 0, 0));
				clr = COLOR_VISTA_MENUBKGND;
				brushColor.SetValue(Color::MakeARGB(160, GetRValue(clr), GetGValue(clr), GetBValue(clr)));

				Pen pen(penColor, 1);
				SolidBrush brush(brushColor);
				iLspGdipRoundRect(g, &pen, &brush, r, CSize(5, 5), roundFlags);
			}
		}
	}
}

#define _MKARGB(a,r,g,b)	(ARGB)(((ARGB)(b)|((ARGB)(g)<< 8)|((ARGB)(r)<<16)|((ARGB)(a)<<24)))

typedef struct tagARGBPAIR {
	ARGB clrLine;
	ARGB clrFace;
} ARGBPAIR;

class CLspDrawButton {
public:
	ARGBPAIR m_dropdown;
	ARGBPAIR m_pressed;
	ARGBPAIR m_hot;
	ARGBPAIR m_selected;
	ARGBPAIR m_hotSelected;

	CLspDrawButton() 
		: m_dropdown({ _MKARGB(96, 0, 0, 0), _MKARGB(255, 254, 254, 248) })
		, m_pressed({ _MKARGB(0, 0, 0, 0), _MKARGB(32, 0, 0, 0) })
		, m_hot({ _MKARGB(0, 0, 0, 0), _MKARGB(18, 0, 0, 0) })
		, m_selected({ _MKARGB(255, 0, 172, 237), _MKARGB(224, 254, 254, 248) })
		, m_hotSelected({ _MKARGB(255, 0, 172, 237), _MKARGB(32, 0, 172, 237) })
	{}
	~CLspDrawButton()
	{}

	void SetColorMetrics(const ARGBPAIR* pressed, const ARGBPAIR* hot, const ARGBPAIR* selected, const ARGBPAIR* hotSelected, const ARGBPAIR* dropdown)
	{
		if (pressed) SetPressedColor(pressed);
		if (hot) SetPressedColor(hot);
		if (selected) SetPressedColor(selected);
		if (hotSelected) SetPressedColor(hotSelected);
		if (dropdown) SetPressedColor(dropdown);
	}
	void SetPressedColor(const ARGBPAIR* clr) { m_pressed = *clr; }
	void SetHotColor(const ARGBPAIR* clr) { m_hot = *clr; }
	void SetSelectedColor(const ARGBPAIR* clr) { m_selected = *clr; }
	void SetHotSelectedColor(const ARGBPAIR* clr) { m_hotSelected = *clr; }
	void SetDropdownColor(const ARGBPAIR* clr) { m_dropdown = *clr; }

	void Draw(HDC hdc, const CRect& rect, UINT state, UINT roundFlags = ROUND_ALL)
	{
		if (state & (CDIS_HOT | CDIS_PRESSED | CDIS_CHECKED | CDIS_SELECTED | CDIS_DROPDOWN)) {
			Graphics g(hdc);
			g.SetSmoothingMode(SmoothingModeHighQuality);
			RectF r((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.top, (Gdiplus::REAL)rect.Width() - 1, (Gdiplus::REAL)rect.Height() - 1);

			if (state & CDIS_DROPDOWN) {
				DrawButton(g, m_dropdown, r, roundFlags);
			} else if (state & CDIS_PRESSED) {
				DrawButton(g, m_pressed, r, roundFlags);
			} else {
				// state & (CDIS_CHECKED | CDIS_SELECTED | CDIS_HOT)
				if (state & (CDIS_CHECKED | CDIS_SELECTED)) {
					ARGBPAIR clr = m_selected;
					if (state & CDIS_HOT) {
						clr.clrFace = m_hotSelected.clrFace;
					}
					DrawButton(g, clr, r, roundFlags);
				} else if (state & CDIS_HOT) {
					DrawButton(g, m_hot, r, roundFlags);
				}
			}
		}
	}

	void DrawButton(Graphics& g, ARGBPAIR& clr, RectF& rect, UINT roundFlags)
	{
		Pen pen(clr.clrLine, 1);
		SolidBrush brush(clr.clrFace);
		DrawRoundRect(g, &pen, &brush, rect, CSize(5, 5), roundFlags);
	}

	void DrawRoundRect(Graphics& g, Pen* pen, Brush* brush, RectF& r, CSize size, UINT flag)
	{
		// Create a GraphicsPath, and add an ellipse.
		REAL left = r.X;
		REAL right = r.GetRight();
		REAL top = r.Y;
		REAL bottom = r.GetBottom();
		SizeF sf((Gdiplus::REAL)size.cx, (Gdiplus::REAL)size.cy);

		GraphicsPath p;
		p.AddLine(left + sf.Width / 2, top, right - sf.Width / 2, top);
		if (flag & ROUND_TOPRIGHT) {
			// 右上角を丸く
			p.AddArc(right - sf.Width, top, sf.Width, sf.Height, 270.0, 90.0);
		} else {
			p.AddLine(right - sf.Width / 2, top, right, top);
			p.AddLine(right, top, right, top + sf.Height / 2);
		}
		p.AddLine(right, top + sf.Height / 2, right, bottom - sf.Height / 2);
		if (flag & ROUND_BOTTOMRIGHT) {
			// 右下角を丸く
			p.AddArc(right - sf.Width, bottom - sf.Height, sf.Width, sf.Height, 0, 90);
		} else {
			p.AddLine(right, bottom - sf.Height / 2, right, bottom);
			p.AddLine(right, bottom, right - sf.Width / 2, bottom);
		}
		p.AddLine(right - sf.Width / 2, bottom, left + sf.Width / 2, bottom);
		if (flag & ROUND_BOTTOMLEFT) {
			// 左下角を丸く
			p.AddArc(left, bottom - sf.Height, sf.Width, sf.Height, 90, 90);
		} else {
			p.AddLine(left + sf.Width / 2, bottom, left, bottom);
			p.AddLine(left, bottom, left, bottom - sf.Height / 2);
		}
		p.AddLine(left, bottom - sf.Height / 2, left, top + sf.Height / 2);
		if (flag & ROUND_TOPLEFT) {
			// 左上角を丸く
			p.AddArc(left, top, sf.Width, sf.Height, 180, 90);
		} else {
			p.AddLine(left, top + sf.Height / 2, left, top);
			p.AddLine(left, top, left + sf.Width / 2, top);
		}

		if (brush) g.FillPath(brush, &p);			// Fill Path.
		if (pen) g.DrawPath(pen, &p);				// Draw Path.
	}
};




#endif		// __BUTTONFRAME_H__

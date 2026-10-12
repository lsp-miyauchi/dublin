////////////////////////////////////////////////////////////////////////////////
// color_theme.h


#pragma once
#ifndef __COLOR_THEME_H__
#define __COLOR_THEME_H__

enum {
	colNorm = 0,
	colBack,
	colSelect,
	colSelBack,
	colSystem,
	colHidden,
	colRdonly,
	colCmprs,
	colEncr,
	colRepp,
	colLine,
	colMark,
	colMarkBack,
	colEndValue,
};

class CColorTheme
{
private:
	COLORREF m_colors[colEndValue];
public:
	CColorTheme()
	{
		Reset();
	}
	void Reset()
	{
		m_colors[colNorm]  = GetSysColor(COLOR_WINDOWTEXT);
		m_colors[colBack]  = GetSysColor(COLOR_WINDOW);
		m_colors[colSelect] = GetSysColor(COLOR_HIGHLIGHTTEXT);
		m_colors[colSelBack] = GetSysColor(COLOR_HIGHLIGHT);
		m_colors[colSystem] = RGB(128, 0, 128);
		m_colors[colHidden] = RGB(0, 0, 192);
		m_colors[colRdonly] = RGB(0, 128, 0);
		m_colors[colCmprs] = RGB(208, 152, 0);
		m_colors[colEncr] = RGB(153, 0, 102);
		m_colors[colRepp] = RGB(153, 51, 0);
		m_colors[colLine] = CLR_DEFAULT;
		m_colors[colMark] = RGB(255, 255, 255);
		m_colors[colMarkBack] = RGB(0x2e, 0x8b, 0x57);			// seagreen
	}
	void operator=(const CColorTheme& src) { memcpy(m_colors, src.m_colors, sizeof(m_colors)); }
	void Set(COLORREF* clrTable, int numEntries)
	{
		int cnt = min(numEntries, colEndValue);
		memcpy(m_colors, clrTable, cnt * sizeof(COLORREF));
	}
	COLORREF SetAt(int index, COLORREF clr)
	{
		COLORREF oldclr = CLR_NONE;
		if (index >= 0 && index < colEndValue) {
			oldclr = m_colors[index];
			m_colors[index] = clr;
		}
		return oldclr;
	}
	COLORREF GetAt(int index) const
	{
		return ((index >= 0 && index < colEndValue) ? m_colors[index] : CLR_NONE);
	}
	COLORREF operator [](int index) const
	{
		return ((index >= 0 && index < colEndValue) ? m_colors[index] : CLR_NONE);
	}
	COLORREF& operator [](int index)
	{
		return m_colors[index];
	}
	operator COLORREF*()
	{
		return m_colors;
	}
	int GetSize()
	{
		return ARRAYSIZE(m_colors);
	}
};


#endif		// __COLOR_THEME_H__

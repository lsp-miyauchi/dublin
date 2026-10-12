////////////////////////////////////////////////////////////////////////
// custom_bkgnd_draw.h

#pragma once
#ifndef __CUSTOM_BKGND_H__
#define __CUSTOM_BKGND_H__


template <class T>
class CCustomBkgndDrawImpl
{
private:
	COLORREF m_clrCbdBkgnd;
	CBrush m_brCbdBkgnd;

public:
	CCustomBkgndDrawImpl()
	{
		m_clrCbdBkgnd = RGB(238, 238, 238);
		m_brCbdBkgnd.CreateSolidBrush(m_clrCbdBkgnd);
	}

BEGIN_MSG_MAP(CCustomBkgndDrawImpl)
	ON_MESSAGE(WM_CTLCOLORDLG, OnCustomBkgndDraw_CtlColorDialog)
	ON_MESSAGE(WM_CTLCOLORSTATIC, OnCustomBkgndDraw_CtlColorDialog)
	ON_MESSAGE(WM_CTLCOLORBTN, OnCustomBkgndDraw_CtlColorDialog)
END_MSG_MAP()

	LRESULT OnCustomBkgndDraw_CtlColorDialog(WPARAM wParam, LPARAM lParam)
	{
		::SetBkColor((HDC)wParam, m_clrCbdBkgnd);
		return (LRESULT)(HBRUSH)m_brCbdBkgnd;
	}


};


#endif			// __CUSTOM_BKGND_H__

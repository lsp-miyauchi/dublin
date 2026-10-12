// paledit.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "paledit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPalEditDlg::CPalEditDlg()
{
	m_hpal = NULL;
	m_crCur = CLR_NONE;
}

CPalEditDlg::~CPalEditDlg()
{
	DeleteObject(m_hpal);
}

BOOL CPalEditDlg::OnInitDialog(LPARAM lParam)
{
	if (!m_hpal) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	CenterWindow();
	m_spinRed = GetDlgItem(IDC_RED_SPIN);
	m_spinGreen = GetDlgItem(IDC_GREEN_SPIN);
	m_spinBlue = GetDlgItem(IDC_BLUE_SPIN);
	m_spinHue = GetDlgItem(IDC_HUE_SPIN);
	m_spinSat = GetDlgItem(IDC_SAT_SPIN);
	m_spinLum = GetDlgItem(IDC_LUM_SPIN);
	m_palette = GetDlgItem(IDC_PALETTE);

	m_spinRed.SetRange(0, 255);
	m_spinGreen.SetRange(0, 255);
	m_spinBlue.SetRange(0, 255);
	m_spinHue.SetRange(0, 239);
	m_spinSat.SetRange(0, 240);
	m_spinLum.SetRange(0, 240);

	m_palette.SendMessage(PLTM_SETPALETTE, FALSE, (LPARAM)m_hpal);
	m_palette.SendMessage(PLTM_SETCURSEL, 0, 0);
	COLORREF cr;
	if (m_palette.SendMessage(PLTM_GETCOLOR, 0, (LPARAM)&cr))
		ShowPalInfo(cr);
	return TRUE;
}

void CPalEditDlg::OnOK()
{
	EndDialog(m_bUpdate ? IDOK : IDCANCEL);
}

void CPalEditDlg::ShowPalInfo(COLORREF cr)
{
	if (m_crCur != cr) {
		m_spinRed.SetPos(GetRValue(cr));
		m_spinGreen.SetPos(GetGValue(cr));
		m_spinBlue.SetPos(GetBValue(cr));

		WORD h, s, l;
		ColorRGBToHLS(cr, &h, &l, &s);
		m_spinHue.SetPos(h);
		m_spinSat.SetPos(s);
		m_spinLum.SetPos(l);

		RectCtrl_SetBkColor(GetDlgItem(IDC_SAMPLE), cr);
	}
}

void CPalEditDlg::OnNotifyPalClick(NMHDR* phdr, LRESULT* pResult)
{
	*pResult = 0;
	PALNOTIFY* ppn = (PALNOTIFY*)phdr;
	ShowPalInfo(ppn->rgb);
}

void CPalEditDlg::OnNotifyPalChanged(NMHDR* phdr, LRESULT* pResult)
{
	*pResult = 0;

	PALNOTIFY* ppn = (PALNOTIFY*)phdr;
	PALETTEENTRY pe;
	pe.peRed = GetRValue(ppn->rgb);
	pe.peGreen = GetGValue(ppn->rgb);
	pe.peBlue = GetBValue(ppn->rgb);
	pe.peFlags = 0;
	if (SetPaletteEntries(m_hpal, ppn->nIndex, 1, &pe))
		m_bUpdate = TRUE;
}

void CPalEditDlg::OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar)
{
	int id = ::GetDlgCtrlID(hScrollBar);
	int r, g, b;
	WORD h, l, s;
	COLORREF cr;
	if (id == IDC_RED_SPIN || id == IDC_GREEN_SPIN || id == IDC_BLUE_SPIN) {
		r = m_spinRed.GetPos();
		g = m_spinGreen.GetPos();
		b = m_spinBlue.GetPos();

		cr = RGB(r, g, b);
		ColorRGBToHLS(cr, &h, &l, &s);

		m_spinHue.SetPos(h);
		m_spinSat.SetPos(s);
		m_spinLum.SetPos(l);
	}
	else if (id == IDC_HUE_SPIN || id == IDC_SAT_SPIN || id == IDC_LUM_SPIN) {
		h = m_spinHue.GetPos();
		s = m_spinSat.GetPos();
		l = m_spinLum.GetPos();

		cr = ColorHLSToRGB(h, l, s);

		m_spinRed.SetPos(GetRValue(cr));
		m_spinGreen.SetPos(GetGValue(cr));
		m_spinBlue.SetPos(GetBValue(cr));
	}
	else {
		DefWindowProc();
		return;
	}

	if (m_crCur != cr) {
		m_crCur = cr;
		RectCtrl_SetBkColor(GetDlgItem(IDC_SAMPLE), cr);

		int sel = (int)m_palette.SendMessage(PLTM_GETCURSEL, 0, 0);
		m_palette.SendMessage(PLTM_SETCOLOR, sel, cr);
		RectCtrl_SetBkColor(GetDlgItem(IDC_SAMPLE), cr);

		PALETTEENTRY pe;
		pe.peRed = GetRValue(cr);
		pe.peGreen = GetGValue(cr);
		pe.peBlue = GetBValue(cr);
		pe.peFlags = 0;
		if (SetPaletteEntries(m_hpal, sel, 1, &pe))
			m_bUpdate = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////
//

HDIB WINAPI edit_palette(HWND hwnd, HDIB hdib)
{
	HDIB hdibNew = NULL;
	CPalEditDlg dlg;
	dlg.m_hpal = imgCreatePalette(hdib);
	dlg.m_crCur = CLR_NONE;
	if (dlg.DoModal(hwnd) == IDOK) {
		mif image(hdib);
		image.replace_palette(dlg.m_hpal);
		hdibNew = image.createdib();
	}
	return hdibNew;
}

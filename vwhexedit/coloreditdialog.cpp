// ColorEditDialog.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "hexmanager.h"
#include "ColorEditDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct tagSAMPLEINFO {
	TCHAR* offs;
	TCHAR* data;
} SAMPLEINFO;

static const SAMPLEINFO samples[7] = {
	_T("0000"), _T("11 AF 2B CE 54 67 09 78 87 12 56 33 AF 2B CE 54"),
	_T("0010"), _T("5A 11 31 7C DA 3F 34 2A 09 AA FA B3 5B 98 EE 31"),
	_T("0020"), _T("AA 1F 5B 98 EE 31 01 5D 43 38 B8 CA 5B 98 EE 31"),
	_T("0030"), _T("DA 3F 34 09 AA FA B3 9F 5A 11 31 7C 3F 34 09 AA"),
	_T("0040"), _T("11 AF 2B CE 54 67 09 78 87 12 56 33 AF 2B CE 54"),
	_T("0050"), _T("5A 11 31 7C DA 3F 34 2A 09 AA FA B3 5B 98 EE 31"),
	_T("0060"), _T("AA 1F 5B 98 EE 31 01 5D 43 38 B8 CA 5B 98 EE 31"),
};


/////////////////////////////////////////////////////////////////////////////
// CColorEditDialog メッセージ ハンドラ


BOOL CColorEditDialog::OnInitDialog(LPARAM lParam) 
{
	CenterWindow();
	SendDlgItemMessage(IDC_FONT, FPM_SETLOGFONT, 0, (LPARAM)&m_lf);
	return TRUE;
}

COLORREF CColorEditDialog::DoSelectColor(UINT nIDCtrl, COLORREF clrCurrent)
{
	CRect rc;
	GetDlgItem(nIDCtrl).GetWindowRect(&rc);
	CPoint pt(rc.left, rc.bottom - 1);
	COLORREF clr = clrCurrent;

	CClientDC dc(NULL);
	BOOL mode = (dc.GetDeviceCaps(BITSPIXEL) > 8);
	if (lccColorPicker(m_hWnd, pt, &clr, mode)) {
		CClientDC dc(m_hWnd);
		return dc.GetNearestColor(clr);
	}
	return (COLORREF)-1;
}

void CColorEditDialog::OnPointColor() 
{
	COLORREF rgb = DoSelectColor(IDC_POINT, m_vc.rgbAddr);
	if (rgb != (COLORREF)-1) {
		m_vc.rgbAddr = rgb;
		GetDlgItem(IDC_SAMPLE1).Invalidate();
	}
}

void CColorEditDialog::OnTextColor() 
{
	COLORREF rgb = DoSelectColor(IDC_TEXT1, m_vc.rgbText);
	if (rgb != (COLORREF)-1) {
		m_vc.rgbText = rgb;
		GetDlgItem(IDC_SAMPLE1).Invalidate();
	}
}

void CColorEditDialog::OnBackColor() 
{
	COLORREF rgb = DoSelectColor(IDC_BACK1, m_vc.rgbBack);
	if (rgb != (COLORREF)-1) {
		m_vc.rgbBack = rgb;
		GetDlgItem(IDC_SAMPLE1).Invalidate();
	}
}

void CColorEditDialog::OnMarkTextColor() 
{
	COLORREF rgb = DoSelectColor(IDC_MARKTEXT, m_vc.rgbMarkText);
	if (rgb != (COLORREF)-1) {
		m_vc.rgbMarkText = rgb;
		GetDlgItem(IDC_SAMPLE1).Invalidate();
	}
}

void CColorEditDialog::OnMarkBackColor() 
{
	COLORREF rgb = DoSelectColor(IDC_MARKBACK, m_vc.rgbMarkBack);
	if (rgb != (COLORREF)-1) {
		m_vc.rgbMarkBack = rgb;
		GetDlgItem(IDC_SAMPLE1).Invalidate();
	}
}

void CColorEditDialog::OnFont() 
{
	SendDlgItemMessage(IDC_FONT, FPM_GETLOGFONT, 0, (LPARAM)&m_lf);
	GetDlgItem(IDC_SAMPLE1).Invalidate();
}

void CColorEditDialog::OnLine() 
{
	COLORREF rgb = DoSelectColor(IDC_LINE, m_vc.rgbLine);
	if (rgb != (COLORREF)-1) {
		m_vc.rgbLine = rgb;
		GetDlgItem(IDC_SAMPLE1).Invalidate();
	}
}

void CColorEditDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT pdi)
{
	if (nIDCtl != IDC_SAMPLE1)
		return;

	CDCHandle dc = pdi->hDC;
	CRect rc(&pdi->rcItem);

// 背景
	dc.FillSolidRect(&rc, m_vc.rgbBack);

	CFont font;
	font.CreateFontIndirect(&m_lf);
	CFontHandle fontOld = dc.SelectFont(font);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	dc.SetBkMode(TRANSPARENT);

// 文字を描画

	CRect rcw = rc;
	rcw.bottom = rcw.top + tm.tmHeight;
	for (int i = 0; i < 7; i++) {
		dc.FillSolidRect(rcw.left, rcw.top, rcw.Width(), rcw.Height(), m_vc.rgbBack);
		if (i == 0) {
			dc.FillSolidRect(rcw.left, rcw.top, tm.tmAveCharWidth * 4, rcw.Height(), m_vc.rgbLine);
			dc.SetTextColor(m_vc.rgbBack);
		}
		else
			dc.SetTextColor(m_vc.rgbAddr);

		dc.DrawText(samples[i].offs, -1, &rcw, DT_LEFT | DT_TOP | DT_SINGLELINE);

		dc.SetTextColor(m_vc.rgbText);
		rcw.left += 6 * tm.tmAveCharWidth;

		if (i == 3) {
			dc.SetBkMode(OPAQUE);
			dc.SetTextColor(m_vc.rgbMarkText);
			dc.SetBkColor(m_vc.rgbMarkBack);
		}
		dc.DrawText(samples[i].data, -1, &rcw, DT_LEFT | DT_TOP | DT_SINGLELINE);
		if (i == 3) {
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(m_vc.rgbText);
			dc.SetBkColor(m_vc.rgbBack);
		}
		rcw.left -= 6 * tm.tmAveCharWidth;

		rcw.OffsetRect(0, tm.tmHeight);
		rcw &= rc;
		if (rcw.IsRectEmpty())
			break;
	}
	dc.SelectFont(fontOld);

// 線を描画
	CPen pen = CreatePen(PS_SOLID, 1, m_vc.rgbLine);
	CPenHandle penOld = dc.SelectPen(pen);
	dc.MoveTo(rc.left, rc.top + tm.tmHeight - 1);
	dc.LineTo(rc.right, rc.top + tm.tmHeight - 1);
	dc.SelectPen(penOld);
}


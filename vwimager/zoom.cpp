// Zoom.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "common.h"
#include "resource.h"
#include "Zoom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int zooms[] = {
	200, 180, 160, 140, 120, 100, 90, 80, 70, 60, 50, 40, 30, 20
};

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg メッセージ ハンドラ

BOOL CZoomDlg::OnInitDialog(LPARAM lParam) 
{
	CenterWindow();

	m_spin = GetDlgItem(IDC_SPIN);
	m_zoom = GetDlgItem(IDC_ZVAL);
	m_sample = GetDlgItem(IDC_SAMPLE);
	InitDlgCtrl(m_zoom, IDS_ZOOM_ZVAL);
	InitDlgCtrl(m_sample, IDS_ZOOM_SAMPLE);

	TCHAR buf[MAX_PATH];
	wsprintf(buf, _T("%d x %d"), m_sizeSrc.cx, m_sizeSrc.cy);
	SetDlgItemText(IDC_TITLE, buf);
	m_sizeDst = m_sizeSrc;
	SetDlgItemInt(IDC_XSIZE, m_sizeDst.cx, FALSE);
	SetDlgItemInt(IDC_YSIZE, m_sizeDst.cy, FALSE);
	ShowSize();

	CheckDlgButton(IDC_CHECK, TRUE);

	m_zoom.SetCurSel(5);
	m_spin.SetRange(10, 800);
	m_spin.SetPos(100);
	OnZoomClicked();

	m_sample.SetCurSel(2);
	return TRUE;
}

void CZoomDlg::OnChangeXsize() 
{
	if (GetFocus() == GetDlgItem(IDC_XSIZE) && IsDlgButtonChecked(IDC_CHECK)) {
		int y = MulDiv(GetDlgItemInt(IDC_XSIZE, FALSE), m_sizeSrc.cy, m_sizeSrc.cx);
		SetDlgItemInt(IDC_YSIZE, y);

		ShowSize();
	}
}

void CZoomDlg::OnChangeYsize() 
{
	if (GetFocus() == GetDlgItem(IDC_YSIZE) && IsDlgButtonChecked(IDC_CHECK)) {
		int x = MulDiv(GetDlgItemInt(IDC_YSIZE, FALSE), m_sizeSrc.cx, m_sizeSrc.cy);
		SetDlgItemInt(IDC_XSIZE, x, FALSE);

		ShowSize();
	}
}

void CZoomDlg::OnChangeZoomValue() 
{
	if (IsDlgButtonChecked(IDC_ZOOM)) {
		int x = m_sizeSrc.cx;
		int y = m_sizeSrc.cy;
		int zoom = GetDlgItemInt(IDC_ZVAL, FALSE);
		if (zoom != 100) {
			x = MulDiv(zoom, m_sizeSrc.cx, 100);
			y = MulDiv(zoom, m_sizeSrc.cy, 100);
		}
		SetDlgItemInt(IDC_XSIZE, x, FALSE);
		SetDlgItemInt(IDC_YSIZE, y, FALSE);

		ShowSize();
	}
}

void CZoomDlg::OnSelChangeZoomValue() 
{
	if (IsDlgButtonChecked(IDC_ZOOM)) {
		int x = m_sizeSrc.cx;
		int y = m_sizeSrc.cy;
		int zoom = zooms[m_zoom.GetCurSel()];
		if (zoom != 100) {
			x = MulDiv(zoom, m_sizeSrc.cx, 100);
			y = MulDiv(zoom, m_sizeSrc.cy, 100);
		}
		SetDlgItemInt(IDC_XSIZE, x, FALSE);
		SetDlgItemInt(IDC_YSIZE, y, FALSE);

		ShowSize();
	}
}

void CZoomDlg::OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pud = (NM_UPDOWN*)pNMHDR;
	int x = m_sizeSrc.cx;
	int y = m_sizeSrc.cy;
	int zoom = pud->iPos;
	if (zoom != 100) {
		x = MulDiv(zoom, m_sizeSrc.cx, 100);
		y = MulDiv(zoom, m_sizeSrc.cy, 100);
	}
	SetDlgItemInt(IDC_XSIZE, x, FALSE);
	SetDlgItemInt(IDC_YSIZE, y, FALSE);

	ShowSize();
	*pResult = 0;
}

void CZoomDlg::OnOK() 
{
	m_sizeDst.cx = GetDlgItemInt(IDC_XSIZE, FALSE);
	m_sizeDst.cy = GetDlgItemInt(IDC_YSIZE, FALSE);
	m_nType = (m_sizeSrc.cx > m_sizeDst.cx || m_sizeSrc.cy > m_sizeDst.cy) ? zoomMitchell : zoomCatrom;
	m_resample = m_sample.GetCurSel();
	EndDialog(IDOK);
}

void CZoomDlg::OnZoomClicked() 
{
	BOOL bEnable = (IsDlgButtonChecked(IDC_ZOOM) == 0);
	GetDlgItem(IDC_XSIZE).EnableWindow(bEnable);
	GetDlgItem(IDC_YSIZE).EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK).EnableWindow(bEnable);
	m_spin.EnableWindow(!bEnable);
	GetDlgItem(IDC_ZVAL).EnableWindow(!bEnable);
}

void CZoomDlg::ShowSize()
{
	int x = GetDlgItemInt(IDC_XSIZE, FALSE);
	int y = GetDlgItemInt(IDC_YSIZE, FALSE);

	int total = x * y * 3;
	double mega = (double)total / 1048576.0;
	TCHAR w[MAX_PATH];
	_stprintf_s(w, ARRAYSIZE(w), _T("%.2f"), mega);
	SetDlgItemText(IDC_VALUE, w);
}

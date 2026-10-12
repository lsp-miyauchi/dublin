// resize.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "math.h"
#include "resize.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CResizeDlg ダイアログ


CResizeDlg::CResizeDlg()
{
	m_cr = RGB(255, 255, 255);
}

BOOL CResizeDlg::OnInitDialog(LPARAM lParam) 
{
	CenterWindow();

	m_scale = (double)m_size.cx / (double)m_size.cy;

	TCHAR buf[MAX_PATH];
	wsprintf(buf, _T("%d x %d"), m_size.cx, m_size.cy);
	SetDlgItemText(IDC_TITLE, buf);
	SetDlgItemInt(IDC_XSIZE, m_size.cx, FALSE);
	SetDlgItemInt(IDC_YSIZE, m_size.cy, FALSE);

	ShowSize();

	CRect rc;
	GetDlgItem(IDC_ANCHOR).GetWindowRect(&rc);
	ScreenToClient(&rc);
	GetDlgItem(IDC_ANCHOR).DestroyWindow();

	rc.right = rc.left + ANCHOR_BLOCK_SIZE * 3;
	rc.bottom = rc.top + ANCHOR_BLOCK_SIZE * 3;
	AdjustWindowRectEx(&rc, WS_CHILD | WS_VISIBLE, FALSE, WS_EX_CLIENTEDGE);
	m_anchorwnd.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE, IDC_ANCHOR);

	ColorPicker_SetBkColor(GetDlgItem(IDC_COLOR), m_cr);
	if (m_bits < 24)
		GetDlgItem(IDC_COLOR).EnableWindow(FALSE);
	return TRUE;
}

void CResizeDlg::OnChangeXsize() 
{
	if (GetFocus() == GetDlgItem(IDC_XSIZE) && IsDlgButtonChecked(IDC_CHECK)) {
		int x = GetDlgItemInt(IDC_XSIZE, FALSE);
		int y = (int)floor((x / m_scale) + 0.5);
		SetDlgItemInt(IDC_YSIZE, y);
		ShowSize();
	}
}

void CResizeDlg::OnChangeYsize() 
{
	if (GetFocus() == GetDlgItem(IDC_YSIZE) && IsDlgButtonChecked(IDC_CHECK)) {
		int y = GetDlgItemInt(IDC_YSIZE, FALSE);
		int x = (int)floor((y * m_scale) + 0.5);
		SetDlgItemInt(IDC_XSIZE, x, FALSE);
		ShowSize();
	}
}

void CResizeDlg::ShowSize()
{
	int x = GetDlgItemInt(IDC_XSIZE, FALSE);
	int y = GetDlgItemInt(IDC_YSIZE, FALSE);

	int total = x * y * 3;
	double mega = (double)total / 1048576.0;
	TCHAR w[MAX_PATH];
	_stprintf_s(w, ARRAYSIZE(w), _T("%.2f"), mega);
	SetDlgItemText(IDC_VALUE, w);
}

void CResizeDlg::OnOK() 
{
	m_size.cx = GetDlgItemInt(IDC_XSIZE, FALSE);
	m_size.cy = GetDlgItemInt(IDC_YSIZE, FALSE);
	m_anchor = m_anchorwnd.GetAnchor();
	m_cr = ColorPicker_GetBkColor(GetDlgItem(IDC_COLOR));

	EndDialog(IDOK);
}

////////////////////////////////////////////////////////////////////////
//

HDIB resize_canvas(HWND hParent, HDIB hdib)
{
	int sx, sy, bits;
	if (!hdib || !dib_getinfo(hdib, &sx, &sy, NULL, &bits)) return NULL;

	CSize sizeOrg(sx, sy);

	CResizeDlg dlg;
	dlg.m_size = sizeOrg;
	dlg.m_bits = bits;
	if (dlg.DoModal(hParent) != IDOK || sizeOrg == dlg.m_size)
		return NULL;

	CSize size = dlg.m_size;
	if (size.cx < sizeOrg.cx || size.cy < sizeOrg.cy) {
		if (AppMessage(IDR_FRAME, IDS_SIZE_WARNING, NULL, MB_QYESNO) != IDYES)
			return NULL;
	}

	mif src(hdib);
	mif dst(size.cx, size.cy, src.m_bits);
	if (dst.m_bits < 24) {
		dst.alloccmap(src.m_colors);
		memcpy(dst.m_map, src.m_map, sizeof(RGBA) * src.m_colors);
	}
	else
		dst.solidfill(NULL, dlg.m_cr, 255);

	switch (dlg.m_anchor % 3) {
		case 0: sx = 0; break;
		case 1: sx = (size.cx - sizeOrg.cx) / 2; break;
		case 2: sx = size.cx - sizeOrg.cx; break;
	}

	switch (dlg.m_anchor / 3) {
		case 0: sy = 0; break;
		case 1: sy = (size.cy - sizeOrg.cy) / 2; break;
		case 2: sy = size.cy - sizeOrg.cy; break;
	}

	dst.blt(sx, sy, sizeOrg.cx, sizeOrg.cy, &src, 0, 0);
	return dst.createdib();
}

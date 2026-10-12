// SetupDlg.cpp : CSetupDlg の実装

#include "stdafx.h"
#include "common.h"
#include "setup.h"


// CSetupDlg


BOOL CSetupDlg::OnInitDialog(LPARAM /*lParam*/)
{
	CenterWindow();

	// リスト表示
	CheckDlgButton(IDC_OPLOWER, (m_ai.stat & ATS_LOWER) != 0);
	CheckDlgButton(IDC_OPGRID, (m_ai.stat & ATS_GRID) != 0);
	CheckDlgButton(IDC_OPSIZEKB, (m_ai.stat & ATS_SIZEKB) != 0);
	CheckDlgButton(IDC_OPREPCHECKER, (m_ai.stat & ATS_REPCHECKER) != 0);

#ifdef MOUSE_HOTTRACKING
	if (m_ai.stat & ATS_HOTTRACK)	CheckDlgButton(IDC_OPHOTTRACK, TRUE);
#else
	::ShowWindow(GetDlgItem(IDC_OPHOTTRACK), SW_HIDE);
#endif

	SetDlgItemText(IDC_EDIT, m_ai.path);

	// 配色
	SendDlgItemMessage(IDC_COL_NORMAL, RM_SETTEXTCOLOR, 0, m_ai.colors[0]);
	SendDlgItemMessage(IDC_COL_NORMAL, RM_SETBKCOLOR, 0, m_ai.colors[1]);
	SendDlgItemMessage(IDC_COL_SELECT, RM_SETTEXTCOLOR, 0, m_ai.colors[2]);
	SendDlgItemMessage(IDC_COL_SELECT, RM_SETBKCOLOR, 0, m_ai.colors[3]);

	return TRUE;
}

void CSetupDlg::OnOK()
{
	m_ai.stat = 0;
	if (IsDlgButtonChecked(IDC_OPLOWER))		m_ai.stat |= ATS_LOWER;
	if (IsDlgButtonChecked(IDC_OPGRID))			m_ai.stat |= ATS_GRID;
	if (IsDlgButtonChecked(IDC_OPSIZEKB))		m_ai.stat |= ATS_SIZEKB;
	if (IsDlgButtonChecked(IDC_OPREPCHECKER))	m_ai.stat |= ATS_REPCHECKER;
#ifdef MOUSE_HOTTRACKING
	if (IsDlgButtonChecked(IDC_OPHOTTRACK))		m_ai.stat |= ATS_HOTTRACK;
#endif
	GetDlgItemText(IDC_EDIT, m_ai.path, _COUNTOF(m_ai.path));

	// 配色
	m_ai.colors[0] = (COLORREF)SendDlgItemMessage(IDC_COL_NORMAL, RM_GETTEXTCOLOR, 0, 0);
	m_ai.colors[1] = (COLORREF)SendDlgItemMessage(IDC_COL_NORMAL, RM_GETBKCOLOR, 0, 0);
	m_ai.colors[2] = (COLORREF)SendDlgItemMessage(IDC_COL_SELECT, RM_GETTEXTCOLOR, 0, 0);
	m_ai.colors[3] = (COLORREF)SendDlgItemMessage(IDC_COL_SELECT, RM_GETBKCOLOR, 0, 0);

	EndDialog(IDOK);
}

void CSetupDlg::OnCancel()
{
	EndDialog(IDCANCEL);
}

void CSetupDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (fopBrowseForFolder(m_hWnd, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

COLORREF CSetupDlg::SelectColor(UINT id, COLORREF cr)
{
	CRect rc;
	::GetWindowRect(GetDlgItem(id), &rc);
	CPoint pt(rc.left, rc.bottom - 1);
	COLORREF r = cr;
	HDC hdc = GetDC();
	BOOL mode = (GetDeviceCaps(hdc, BITSPIXEL) > 8);
	if (!lccColorPicker(m_hWnd, pt, &r, mode))
		r = (COLORREF)-1;
	else {
		if (!mode)
			r = GetNearestColor(hdc, r);
	}
	ReleaseDC(hdc);
	return r;
}

void CSetupDlg::OnTextColor()
{
	COLORREF cr = (COLORREF)SendDlgItemMessage(IDC_COL_NORMAL, RM_GETTEXTCOLOR, 0, 0);
	cr = SelectColor(IDC_BTN_TEXT, cr);
	if (cr != (COLORREF)-1)
		SendDlgItemMessage(IDC_COL_NORMAL, RM_SETTEXTCOLOR, 0, cr);
}

void CSetupDlg::OnBackColor()
{
	COLORREF cr = (COLORREF)SendDlgItemMessage(IDC_COL_NORMAL, RM_GETBKCOLOR, 0, 0);
	cr = SelectColor(IDC_BTN_BACK, cr);
	if (cr != (COLORREF)-1)
		SendDlgItemMessage(IDC_COL_NORMAL, RM_SETBKCOLOR, 0, cr);
}

void CSetupDlg::OnRTextColor()
{
	COLORREF cr = (COLORREF)SendDlgItemMessage(IDC_COL_SELECT, RM_GETTEXTCOLOR, 0, 0);
	cr = SelectColor(IDC_BTN_RTEXT, cr);
	if (cr != (COLORREF)-1)
		SendDlgItemMessage(IDC_COL_SELECT, RM_SETTEXTCOLOR, 0, cr);
}

void CSetupDlg::OnRBackColor()
{
	COLORREF cr = (COLORREF)SendDlgItemMessage(IDC_COL_SELECT, RM_GETBKCOLOR, 0, 0);
	cr = SelectColor(IDC_BTN_RBACK, cr);
	if (cr != (COLORREF)-1)
		SendDlgItemMessage(IDC_COL_SELECT, RM_SETBKCOLOR, 0, cr);
}

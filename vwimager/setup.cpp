// setup.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "setup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupDlg メッセージ ハンドラ

BOOL CSetupDlg::OnInitDialog(LPARAM lParam) 
{
	CenterWindow();

	CheckRadioButton(IDC_REALSCALE, IDC_PREVSCALE, IDC_REALSCALE + m_startup_scale);

	SendDlgItemMessage(IDC_GRIDCOLOR, CPM_SETBKCOLOR, 0, m_crGrid);
	SendDlgItemMessage(IDC_BKCOLOR, CPM_SETBKCOLOR, 0, m_crBack);
	CheckDlgButton(IDC_SHOWGRID, m_showGrid);
	SetDlgItemInt(IDC_XGRID, m_sizeGrid.cx, FALSE);
	SetDlgItemInt(IDC_YGRID, m_sizeGrid.cy, FALSE);
	OnShowGrid();

	return TRUE;
}

void CSetupDlg::OnShowGrid() 
{
	m_showGrid = IsDlgButtonChecked(IDC_SHOWGRID);
	GetDlgItem(IDC_XGRID).EnableWindow(m_showGrid);
	GetDlgItem(IDC_YGRID).EnableWindow(m_showGrid);
	GetDlgItem(IDC_GRIDCOLOR).EnableWindow(m_showGrid);
}

void CSetupDlg::OnOK() 
{
	m_startup_scale = GetCheckedRadioButton(m_hWnd, IDC_REALSCALE, IDC_PREVSCALE) - IDC_REALSCALE;

	CSize size;
	size.cx = GetDlgItemInt(IDC_XGRID);
	if (size.cx < 1 || size.cx > 128) {
		MessageBeep(0);
		GetDlgItem(IDC_XGRID).SetFocus();
	}

	size.cy = GetDlgItemInt(IDC_YGRID);
	if (size.cy < 1 || size.cy > 128) {
		MessageBeep(0);
		GetDlgItem(IDC_YGRID).SetFocus();
	}
	m_sizeGrid = size;

	m_crGrid = (COLORREF)SendDlgItemMessage(IDC_GRIDCOLOR, CPM_GETBKCOLOR);
	m_crBack = (COLORREF)SendDlgItemMessage(IDC_BKCOLOR, CPM_GETBKCOLOR);

	EndDialog(IDOK);
}

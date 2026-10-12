/////////////////////////////////////////////////////////////////////////////
// CSetupDlg

#include "stdafx.h"
#include "resource.h"
#include "setup.h"

/////////////////////////////////////////////////////////////////////////////
// CSetupDlg メッセージ ハンドラ

BOOL CSetupDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	if (m_status & LTS_LOWERCASE)		CheckDlgButton(IDC_LOWERCASE, TRUE);
	if (m_status & LTS_ALIGNEXT)		CheckDlgButton(IDC_ALIGNEXT, TRUE);
	if (m_status & LTS_FMTDATE)			CheckDlgButton(IDC_FMTDATE, TRUE);
	if (m_status & LTS_FMTSIZE)			CheckDlgButton(IDC_FMTSIZE, TRUE);
	if (m_status & LTS_RELATIVEDATE)	CheckDlgButton(IDC_RELATIVEDATE, TRUE);
	if (m_status & LTS_INFOTIP)			CheckDlgButton(IDC_INFOTIP, TRUE);
	if (m_status & LTS_REPCHECKER)		CheckDlgButton(IDC_REPCHECKER, TRUE);

	return TRUE;
}

void CSetupDlg::OnOK()
{
	m_status = 0;
	if (IsDlgButtonChecked(IDC_LOWERCASE))		m_status |= LTS_LOWERCASE;
	if (IsDlgButtonChecked(IDC_ALIGNEXT))		m_status |= LTS_ALIGNEXT;
	if (IsDlgButtonChecked(IDC_FMTDATE))		m_status |= LTS_FMTDATE;
	if (IsDlgButtonChecked(IDC_FMTSIZE))		m_status |= LTS_FMTSIZE;
	if (IsDlgButtonChecked(IDC_RELATIVEDATE))	m_status |= LTS_RELATIVEDATE;
	if (IsDlgButtonChecked(IDC_INFOTIP))		m_status |= LTS_INFOTIP;
	if (IsDlgButtonChecked(IDC_REPCHECKER))		m_status |= LTS_REPCHECKER;

	EndDialog(IDOK);
}


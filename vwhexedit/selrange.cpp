// selrange.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "selrange.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelRangeDlg メッセージ ハンドラ

static int str2int(LPCTSTR psz)
{
	int num;
	if (StrToIntEx(psz, STIF_SUPPORT_HEX, &num))
		return num;
	return 0;
}

BOOL CSelRangeDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	if (m_start) {
		TCHAR buf[MAX_PATH];
		wsprintf(buf, _T("0x%X"), m_start);
		SetDlgItemText(IDC_START, buf);
	}
	return TRUE;
}

void CSelRangeDlg::OnOK()
{
	TCHAR buf[MAX_PATH * 2];
	if (GetDlgItemText(IDC_START, buf, ARRAYSIZE(buf)) < 1) {
		Warn(IDC_START);
		return;
	}
	m_start = str2int(buf);
	if (m_start < 0) {
		Warn(IDC_START);
		return;
	}

	if (GetDlgItemText(IDC_END, buf, ARRAYSIZE(buf)) < 1) {
		Warn(IDC_END);
		return;
	}
	m_length = str2int(buf);
	if (!IsDlgButtonChecked(IDC_BYSIZE))
		m_length -= m_start;
	EndDialog(IDOK);
}

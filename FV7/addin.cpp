// addin.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"
#include "addin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAddinToolsDlg dialog


CAddinToolsDlg::CAddinToolsDlg()
{
	m_bAdd = TRUE;
	GetCurrentDirectory(ARRAYSIZE(m_szOlddir), m_szOlddir);
}

CAddinToolsDlg::~CAddinToolsDlg()
{
	SetCurrentDirectory(m_szOlddir);
}

/////////////////////////////////////////////////////////////////////////////
// AddinToolsDlg message handlers

BOOL CAddinToolsDlg::OnInitDialog(LPARAM lParam)
{
	SetDlgItemText(IDC_EXT, m_at.m_strExt);
	SetDlgItemText(IDC_PATH, m_at.m_strPath);
	return TRUE;
}

void CAddinToolsDlg::OnConfirm()
{
	TCHAR buf[MAX_PATH * 2];
	if (GetDlgItemText(IDC_PATH, buf, ARRAYSIZE(buf)) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_PATH).SetFocus();
		return;
	}
	CString str = buf;

	// 実行プログラムのみ抽出
	TCHAR* p;
	if (buf[0] == '"') {
		p = StrChr(&buf[1], '"');
		if (p) *p = 0;
		PathUnquoteSpaces(buf);
	}
	else {
		if (!_pathisfile(buf)) {
			p = StrChr(&buf[1], ' ');
			if (p) *p = 0;
		}
		else {
			PathQuoteSpaces(buf);
			SetDlgItemText(IDC_PATH, buf);
			return;
		}
	}

	BOOL bFound = FALSE;
	if (!_pathisfile(buf)) {
		TCHAR app[MAX_PATH * 2];
		bFound = lccRegGetAppPath(buf, app, ARRAYSIZE(app));
		if (!bFound) {
			GetSystemDirectory(app, ARRAYSIZE(app));
			PathAppend(app, buf);
			bFound = _pathisfile(app);
			if (!bFound) {
				GetWindowsDirectory(app, ARRAYSIZE(app));
				PathAppend(app, buf);
				bFound = _pathisfile(app);
			}
		}

		if (bFound) {
			if (str[0] != '"')
				PathQuoteSpaces(app);
			str.Replace(buf, app);
			SetDlgItemText(IDC_PATH, str);
		}
	}

	if (!bFound) {
		MessageBeep(0);
		GetDlgItem(IDC_EXT).SetFocus();
	}
}

void CAddinToolsDlg::OnOK()
{
	CString str;
	if (GetDlgItemText(IDC_EXT, str) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_EXT).SetFocus();
		return;
	}
	if (str[0] != '.')
		str.Insert(0, '.');
	m_at.m_strExt = str;

	if (GetDlgItemText(IDC_PATH, str) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_EXT).SetFocus();
		return;
	}
	m_at.m_strPath = str;

	EndDialog(IDOK);
}


void CAddinToolsDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;
	CMemTask path;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, (PWSTR*)&path);
	CString str;
	if (FileBrowse(TRUE, m_hWnd, IDS_FILTER_EXECUTE, str, path)) {
		StrCpyN(psgt->pszText, str, psgt->cchTextMax);
		PathQuoteSpaces(psgt->pszText);
		*pResult = TRUE;
	}
}

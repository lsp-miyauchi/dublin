//////////////////////////////////////////////////////////////////////
// newarc.cpp

#include "stdafx.h"
#include "common.h"
#include "newarc.h"

// CNewArcDlg

BOOL CNewArcDlg::OnInitDialog(LPARAM /*lParam*/)
{
	CenterWindow();
	CheckDlgButton(IDC_LZH, TRUE);

	// オートコンプリート
	SHAutoComplete(GetDlgItem(IDC_EDIT), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYSTEM);

	return TRUE;
}

void CNewArcDlg::OnOK()
{
	CString str;
	if (GetDlgItemText(IDC_EDIT, str) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}

	if (PathFileExists(str)) {
		if (AppMessage(m_hWnd, IDS_FILE_ALREADYEXIST, str, MB_YESNO | MB_ICONQUESTION) == IDNO)
			return;

		if (!DeleteFile(str)) {
			LastError(str);
			return;
		}
	}
	lstrcpy(m_nai.name, str);
	m_nai.arctype = IsDlgButtonChecked(IDC_LZH) ? ARC_LZH : ARC_ZIP;

	EndDialog(IDOK);
}

void CNewArcDlg::OnCancel()
{
	EndDialog(IDCANCEL);
}

void CNewArcDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	buf[0] = 0;
	if (FileBrowse(FALSE, m_hWnd, IDS_CAPTION_CREATEARCHIVE, IDS_FILTER_ARCHIVE, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

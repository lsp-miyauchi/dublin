////////////////////////////////////////////////////////////
// findstring.cpp

#include "stdafx.h"
#include "common.h"
#include "mainfrm.h"
#include "findstring.h"


/////////////////////////////////////////////////////////////////////////////
// CFindStringDlg ダイアログ

BOOL CFindStringDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	SetDlgItemText(IDC_TEXT, m_strSearchFor);
	if (m_pFinder->m_stat & FIS_CASE)
		CheckDlgButton(IDC_CAPS, TRUE);

	if (m_pFinder->m_stat & FIS_JRE)
		CheckDlgButton(IDC_USEJRE, TRUE);
	CheckDlgButton((m_pFinder->m_stat & FIS_NEXT) ? IDC_FINDNEXT : IDC_FINDPREV, TRUE);

	// ヒストリの設定
	CComboBox combo = GetDlgItem(IDC_TEXT);
	CAppReg reg;
	reg.LoadComboHistory(combo, IDS_SEC_SETTING, IDS_KEY_FINDSTRING, 16);

	// 検索方向のデフォルトボタンを設定
	::SendMessage(m_hWnd, DM_SETDEFID, (WPARAM)(m_bSearchForward ? IDC_FINDNEXT : IDC_FINDPREV), 0L);

	OnUseJre();
	return TRUE;
}

LRESULT CFindStringDlg::OnHelpCall(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	HELPINFO* phi = (HELPINFO*)lParam;
	if (phi->iContextType == HELPINFO_WINDOW && phi->iCtrlId) {
		UINT id = 0;
		if (phi->iCtrlId == IDC_TEXT)			id = IDS_HELP_GREPSTRING;
		else if (phi->iCtrlId == IDC_CAPS)		id = IDS_HELP_GREPCASE;
		else if (phi->iCtrlId == IDC_USEJRE)	ShowHtmlHelp(m_hWnd, IDD_GREP);

		if (id)
			PopupHint(m_hWnd, id);
		bHandled = TRUE;
	}
	return 0;
}

void CFindStringDlg::OnUseJre()
{
	BOOL b = IsDlgButtonChecked(IDC_USEJRE);
	GetDlgItem(IDC_CAPS).EnableWindow(!b);
}

BOOL CFindStringDlg::Checkup()
{
	// データ取得
	CComboBox combo = GetDlgItem(IDC_TEXT);

	TCHAR buf[MAX_PATH];
	if (combo.GetWindowText(buf, MAX_PATH) < 1) {
		MessageBeep(0);
		combo.SetFocus();
		return FALSE;
	}

	m_pFinder->m_stat = 0;
	m_pFinder->SetText(buf);
	if (IsDlgButtonChecked(IDC_CAPS))
		m_pFinder->m_stat |= FIS_CASE;
	if (IsDlgButtonChecked(IDC_USEJRE)) {
		// 正規表現の確認
		CAtlRegExp<> jre;
		REParseError status = jre.Parse(m_pFinder->m_strText);
		if (REPARSE_ERROR_OK != status) {
			MessageBeep(0);
			combo.SetFocus();
			return FALSE;
		}
		m_pFinder->m_stat |= FIS_JRE;
	}

	CAppReg reg;
	reg.SaveComboHistory(combo, IDS_SEC_SETTING, IDS_KEY_FINDSTRING, 16, buf);
	return TRUE;
}

void CFindStringDlg::OnFindNext()
{
	if (Checkup()) {
		m_pFinder->m_stat |= FIS_NEXT;
		EndDialog(IDOK);
	}
}

void CFindStringDlg::OnFindPrev()
{
	if (Checkup()) {
		EndDialog(IDOK);
	}
}

void CFindStringDlg::OnHelpCall()
{
	ResHelp(m_hWnd, IDR_HTML_REGEXP);
}

//////////////////////////////////////////////////////////////////////
// CReplaceStringDlg


BOOL CReplaceStringDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	if (m_pFinder->m_stat & FIS_CASE)	CheckDlgButton(IDC_CAPS, TRUE);
	if (m_pFinder->m_stat & FIS_JRE)	CheckDlgButton(IDC_USEJRE, TRUE);
	m_pFinder->m_stat &= ~FIS_NEXT;

	// 検索する文字のヒストリの設定
	CAppReg reg;
	CComboBox combo = GetDlgItem(IDC_SRC);
	combo.SetWindowText(m_pFinder->m_strText);
	reg.LoadComboHistory(combo, IDS_SEC_SETTING, IDS_KEY_FINDSTRING, 16);

	// 置換後文字のヒストリの設定
	combo = GetDlgItem(IDC_DST);
	reg.LoadComboHistory(combo, IDS_SEC_SETTING, IDS_KEY_REPLACESTRING, 16);

	OnUseJre();
	return TRUE;
}

LRESULT CReplaceStringDlg::OnHelpCall(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	HELPINFO* phi = (HELPINFO*)lParam;
	if (phi->iContextType == HELPINFO_WINDOW && phi->iCtrlId) {
		UINT id = 0;
		if (phi->iCtrlId == IDC_SRC)			id = IDS_HELP_GREPSTRING;
		else if (phi->iCtrlId == IDC_CAPS)		id = IDS_HELP_GREPCASE;
		else if (phi->iCtrlId == IDC_USEJRE)	ShowHtmlHelp(m_hWnd, IDD_GREP);

		if (id)
			PopupHint(m_hWnd, id);
		bHandled = TRUE;
	}
	return 0;
}

void CReplaceStringDlg::OnUseJre()
{
	BOOL b = IsDlgButtonChecked(IDC_USEJRE);
	GetDlgItem(IDC_CAPS).EnableWindow(!b);
}

BOOL CReplaceStringDlg::Checkup()
{
	// データ取得
	CComboBox combo = GetDlgItem(IDC_SRC);
	TCHAR buf[MAX_PATH];
	if (combo.GetWindowText(buf, MAX_PATH) < 1) {
		MessageBeep(0);
		combo.SetFocus();
		return FALSE;
	}

	m_pFinder->m_stat = FIS_NEXT;
	m_pFinder->SetText(buf);
	if (IsDlgButtonChecked(IDC_CAPS))
		m_pFinder->m_stat |= FIS_CASE;
	if (IsDlgButtonChecked(IDC_USEJRE)) {
		// 正規表現の確認
		CAtlRegExp<> jre;
		REParseError status = jre.Parse(m_pFinder->m_strText);
		if (REPARSE_ERROR_OK != status) {
			MessageBeep(0);
			combo.SetFocus();
			return FALSE;
		}
		m_pFinder->m_stat |= FIS_JRE;
	}
	CAppReg reg;
	reg.SaveComboHistory(combo, IDS_SEC_SETTING, IDS_KEY_FINDSTRING, 16, buf);

	combo = GetDlgItem(IDC_DST);
	combo.GetWindowText(buf, MAX_PATH);
	m_pFinder->SetReplaceText(buf);
	reg.SaveComboHistory(combo, IDS_SEC_SETTING, IDS_KEY_REPLACESTRING, 16, buf);
	return TRUE;
}

void CReplaceStringDlg::OnOK()
{
	if (Checkup()) {
		EndDialog(IDOK);
	}
}

void CReplaceStringDlg::OnReplaceAll()
{
	if (Checkup()) {
		m_pFinder->m_stat |= FIS_FROMTOP;
		EndDialog(IDOK);
	}
}

void CReplaceStringDlg::OnHelpCall()
{
	ResHelp(m_hWnd, IDR_HTML_REGEXP);
}


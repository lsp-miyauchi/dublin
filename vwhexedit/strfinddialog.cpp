// StrFindDialog.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "StrFindDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStrFindDialog ダイアログ


CStrFindDialog::CStrFindDialog()
{
	m_bBinarySearch = TRUE;
	m_csSearch = _T("\0");
	m_ifindtype = FIND_NEXT;
	m_bCase = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CStrFindDialog メッセージ ハンドラ

BOOL CStrFindDialog::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	m_combo = GetDlgItem(IDC_COMBO);

	CAppReg reg;
	reg.LoadComboHistory(m_combo, IDS_SEC_FIND, IDS_KEY_HISTORY, 16);
	if (m_combo.GetCount() > 0)
		m_combo.SetCurSel(0);

	// 大文字小文字で検索？
	m_bCase = reg.GetValue(IDS_SEC_FIND, IDS_KEY_CAPS, FALSE);
	if (m_bCase)
		CheckDlgButton(IDC_CAPS, TRUE);

	// チェックボタンを設定
	m_bBinarySearch = reg.GetValue(IDS_SEC_FIND, IDS_KEY_BINSEARCH, TRUE);
	if (m_bBinarySearch)
		CheckDlgButton(IDC_BCHECK, TRUE);
	else
		CheckDlgButton(IDC_TCHECK, TRUE);

	OnSearchMode();

	return TRUE;
}

void CStrFindDialog::OnSearchMode()
{
	BOOL bEnable = IsDlgButtonChecked(IDC_TCHECK);
	GetDlgItem(IDC_CAPS).EnableWindow(bEnable);
}

void CStrFindDialog::OnFinddown() 
{
	// 下方向へ検索
	CString str;
	if (GetDlgItemText(IDC_COMBO, str) > 0) {
		m_csSearch = str;
		m_ifindtype = FIND_NEXT;
		OnOK();
	}
}


void CStrFindDialog::OnFindup() 
{
	// 上方向へ検索
	if (GetDlgItemText(IDC_COMBO, m_csSearch) > 0) {
		m_ifindtype = FIND_PREV;
		OnOK();
	}
}

void CStrFindDialog::OnHelpCall()
{
	CString cs;
	cs.LoadString(IDS_HELP_SEARCH);
	lccPopView(m_hWnd, NULL, cs, PVS_CAPTION);
}

void CStrFindDialog::OnCancel()
{
	EndDialog(IDCANCEL);
}

void CStrFindDialog::OnOK()
{
	// 文字列を書き込む
	CString cs;
	m_combo.GetWindowText(cs);
	if (cs.IsEmpty()) {
		MessageBeep(0);
		m_combo.SetFocus();
		return;
	}

	CAppReg reg;

	// チェックボックスの状態
	m_bBinarySearch = IsDlgButtonChecked(IDC_BCHECK);
	reg.SetValue(IDS_SEC_FIND, IDS_KEY_BINSEARCH, m_bBinarySearch);

	// 大文字小文字
	m_bCase = IsDlgButtonChecked(IDC_CAPS);
	reg.SetValue(IDS_SEC_FIND, IDS_KEY_CAPS, m_bCase);

	// ヒストリ
	reg.SaveComboHistory(m_combo, IDS_SEC_FIND, IDS_KEY_HISTORY, 16, cs);

	// ウィンドウの位置を書く
	CRect rc;
	GetWindowRect(&rc);
	reg.SetBinary(IDS_SEC_FIND, IDS_KEY_WNDPOS, &rc, sizeof(rc));

	EndDialog(IDOK);
}


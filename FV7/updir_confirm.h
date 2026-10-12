// updir_confirm.h : ヘッダー ファイル
//

#pragma once
#ifndef __UPDIR_COMFIRM_H__
#define __UPDIR_COMFIRM_H__


/////////////////////////////////////////////////////////////////////////////
// CUpdirConfirmDlg ダイアログ

class CUpdirConfirmDlg : public CDialogImpl<CUpdirConfirmDlg>
{
public:
	BOOL m_bConfirmAnyTime;
	enum { IDD = IDD_UPDIRCONFIRM };

	CUpdirConfirmDlg() : m_bConfirmAnyTime(TRUE) { }

BEGIN_MSG_MAP(CUpdirConfirmDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		CheckDlgButton(IDC_CONFIRM, m_bConfirmAnyTime);
		CStatic icon = GetDlgItem(IDC_APPICON);
		icon.SetIcon(::LoadIcon(NULL, IDI_QUESTION));
		return TRUE;
	}
	void OnOK()
	{
		m_bConfirmAnyTime = IsDlgButtonChecked(IDC_CONFIRM);
		EndDialog(IDOK);
	}
	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
	{
		// vista以降はTaskDialogを利用
		TASKDIALOGCONFIG tdc;
		memset(&tdc, 0, sizeof(tdc));
		tdc.cbSize =				sizeof(tdc);
		tdc.hwndParent =			hWndParent;
		tdc.hInstance =				RESINST;
		tdc.dwFlags =				TDF_ALLOW_DIALOG_CANCELLATION | TDF_VERIFICATION_FLAG_CHECKED;
		tdc.dwCommonButtons =		TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
		tdc.pszWindowTitle =		MAKEINTRESOURCE(IDR_MAINFRAME);
		tdc.pszMainIcon =			MAKEINTRESOURCE(IDI_MBX_QUESTION);
		tdc.pszMainInstruction =	MAKEINTRESOURCE(IDS_UPDIR_INSTRUCTION);
		tdc.pszContent =			MAKEINTRESOURCE(IDS_UPDIR_QUESTION);
		tdc.pszVerificationText =	MAKEINTRESOURCE(IDS_UPDIR_VERIFICATION);
		int nButton = 0;
		if ((TaskDialogIndirect(&tdc, &nButton, NULL, &m_bConfirmAnyTime) == S_OK) && (nButton == IDYES))
			return IDOK;
		return IDCANCEL;
	}

};

#endif		// __TABCLOSE_H__

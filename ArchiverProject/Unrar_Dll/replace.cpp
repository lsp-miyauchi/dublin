// replace.cpp : implementation file
//

#include "stdafx.h"
#include "common.h"
#include "unrarenv.h"

/////////////////////////////////////////////////////////////////////////////
// ReplaceDlg dialog


//////////////////////////////////////////////////////////////////////
//

class CReplaceDlg : public CDialogImpl<CReplaceDlg>
{
public:
	CReplaceDlg() : m_prd(NULL)
	{
	}
	~CReplaceDlg() { }

	enum { IDD = IDD_REPLACE };

BEGIN_MSG_MAP(CReplaceDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_YESTOALL, OnYesToAll)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		m_prd = (REPLDATA*)lParam;
		if (!m_prd) {
			EndDialog(IDCANCEL);
			return TRUE;
		}

		// 元ファイル情報
		SetDlgItemText(IDC_SRCNAME, m_prd->lpSrc);
		FormatDetails(IDC_SRCPROP, m_prd->pwfdSrc);

		// 先ファイル情報
		SetDlgItemText(IDC_DSTNAME, m_prd->lpDst);
		FormatDetails(IDC_DSTPROP, m_prd->pwfdDst);

		int idCtrl;
		if (m_prd->nAction == C_QUERY || m_prd->nAction == C_NEWONLY)
			idCtrl = IDC_NEWONLY;
		else if (m_prd->nAction == C_OVERWRITE)
			idCtrl = IDC_OVERWRITE;
		else
			idCtrl = IDC_IGNORE;
		CheckDlgButton(idCtrl, TRUE);

		GetDlgItem(IDC_YESTOALL).EnableWindow(m_prd->bMultiDest);
		MessageBeep(0);
		return TRUE;
	}

	void OnOK()
	{
		QueryFileOperation();
		EndDialog(IDOK);
	}

	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}

	void OnYesToAll()
	{
		m_prd->bYesToAll = TRUE;
		QueryFileOperation();
		EndDialog(IDOK);
	}

	void QueryFileOperation()
	{
		if      (IsDlgButtonChecked(IDC_NEWONLY))		m_prd->nAction = C_NEWONLY;
		else if (IsDlgButtonChecked(IDC_OVERWRITE))		m_prd->nAction = C_OVERWRITE;
		else											m_prd->nAction = C_IGNORE;
	}

	void FormatDetails(UINT id, WIN32_FIND_DATA* pwfd)
	{
		WCHAR szDate[MAX_PATH], szSize[MAX_PATH];

		// サイズ情報を設定
		LARGE_INTEGER lis;
		lis.HighPart = pwfd->nFileSizeHigh;
		lis.LowPart = pwfd->nFileSizeLow;
		StrFormatByteSize(lis.QuadPart, szSize, ARRAYSIZE(szSize));

		// 時刻情報を設定
		SYSTEMTIME st;
		FileTimeToSystemTime(&pwfd->ftLastWriteTime, &st);
		swprintf(szDate, L"%4u/%02u/%02u %02u:%02u:%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		CStringW str;
		str.Format(L"サイズ:  %s\n更新日時:  %s", szSize, szDate);
		SetDlgItemText(id, str);
	}

	REPLDATA* m_prd;
};

////////////////////////////////////////////////////////////////////////
//

BOOL QueryReplace(HWND hwnd, LPREPLDATA prd)
{
	CReplaceDlg dlg;
	return (dlg.DoModal(hwnd, (LPARAM)prd) == IDOK);
}

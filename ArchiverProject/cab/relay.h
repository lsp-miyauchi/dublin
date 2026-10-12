////////////////////////////////////////////////////////////
// relay.h

#pragma once
#ifndef __RELAY_H__
#define __RELAY_H__



int WStrToStr(LPCWSTR pszWide, CStringA& str);

////////////////////////////////////////////////////////////
//

class CRelayDlg : public CDialogImpl<CRelayDlg>
{
public:
	CRelayDlg()
	{
		m_bCanceled = 0;
		m_lTotal = 0;
		m_lDone = 0;
		m_par = 0;
	}
	~CRelayDlg() { }

	enum { IDD = IDD_CABPACK };

BEGIN_MSG_MAP(CRelayDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		CenterWindow();
		m_progress = GetDlgItem(IDC_METER);
		m_progress.SetRange(0, 100);
		m_progress.SetPos(0);
		return TRUE;
	}

	void OnCancel()
	{
		m_bCanceled = TRUE;
		ShowWindow(SW_HIDE);
	}

	void SetCtrlText(UINT idCtrl, LPCWSTR pszText, BOOL bCompact = FALSE)
	{
		if (::IsWindow(m_hWnd)) {
			if (bCompact)
				PathSetDlgItemPath(m_hWnd, idCtrl, pszText);
			else
				SetDlgItemText(idCtrl, pszText);
			ZapMessage();
		}
	}

	__inline void SetTitle(LPCWSTR pszTitle) { SetCtrlText(IDC_TITLE, pszTitle); }
	__inline void SetFile(LPCWSTR pszFile) { SetCtrlText(IDC_TEXT, pszFile, TRUE); }
	__inline void SetMessage(LPCWSTR pszMessage) { SetCtrlText(IDC_MESSAGE, pszMessage); }

	void SetMessage(UINT idResource)
	{
		CString str;
		str.LoadString(idResource);
		SetMessage(str);
	}

	void SetRange(ULONG lTotal)
	{
		if (::IsWindow(m_hWnd)) {
			m_lTotal = !lTotal ? 1 : lTotal;
			m_lDone = 0;
			m_par = -1;

			m_progress.SetPos(0);
			m_progress.RedrawWindow(NULL, NULL, RDW_UPDATENOW);
			ZapMessage();
		}
	}

	void SetComplete(ULONG lDone)
	{
		if (::IsWindow(m_hWnd)) {
			m_lDone = lDone;
			int par = MulDiv(m_lDone, 100, m_lTotal);
			if (par != m_par) {
				m_progress.SetPos(par);
				m_par = par;
			}
			ZapMessage();
		}
	}

	BOOL IsCanceled()
	{
		if (::IsWindow(m_hWnd)) {
			ZapMessage();
			return m_bCanceled;
		}
		return FALSE;
	}

	void ZapMessage()
	{
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!IsDialogMessage(&msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

private:
	CProgressBarCtrl m_progress;

	BOOL m_bCanceled;
	ULONG m_lTotal;
	ULONG m_lDone;
	int m_par;
};


#endif			// __RELAY_H__

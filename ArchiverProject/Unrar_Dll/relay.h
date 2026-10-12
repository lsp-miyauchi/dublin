////////////////////////////////////////////////////////////
// relay.h

#pragma once
#ifndef __RELAY_H__
#define __RELAY_H__


////////////////////////////////////////////////////////////
//

#define TICKS		100

class CRelayDlg : public CDialogImpl<CRelayDlg>
{
private:
	CProgressBarCtrl m_progress;
	bool m_bCanceled;
public:
	CRelayDlg() : m_bCanceled(false) { }
	~CRelayDlg() { }

	enum { IDD = IDD_UNPACK };

BEGIN_MSG_MAP(CRelayDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		CenterWindow();
		m_progress = GetDlgItem(IDC_METER);
		return TRUE;
	}

	void OnCancel()
	{
		m_bCanceled = true;
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
	__inline DWORDLONG GetDoneSize() { return m_progress.GetPos(); }
	__inline DWORDLONG GetTotalSize() { int low, high; m_progress.GetRange(low, high); return high; }

	void SetMessage(UINT idResource)
	{
		CString str;
		str.LoadString(idResource);
		SetMessage(str);
	}

	void SetRange(DWORDLONG dlTotal)
	{
		if (::IsWindow(m_hWnd)) {
			dlTotal = !dlTotal ? 1 : dlTotal;
			m_progress.SetRange32(0, (int)dlTotal);
			m_progress.SetPos(0);
			m_progress.RedrawWindow(NULL, NULL, RDW_UPDATENOW);
			ZapMessage();
		}
	}

	void SetComplete(DWORDLONG dlDone)
	{
		if (::IsWindow(m_hWnd)) {
			if (dlDone > 0) {
				m_progress.OffsetPos((int)dlDone);
				ZapMessage();
			}
		}
	}

	bool IsCanceled()
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
};


#endif			// __RELAY_H__

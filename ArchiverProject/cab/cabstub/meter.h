//////////////////////////////////////////////////////////////////////
// Meter.h

#pragma once
#ifndef __METER_H__
#define __METER_H__

////////////////////////////////////////////////////////////////////////
//

extern HINSTANCE ghInst;

BOOL CenterWindow(HWND hWnd, HWND hWndCenter);

class CProgressDlg
{
public:
	HWND m_hWnd;
	long m_lTotal;
	long m_lDone;

public:
	CProgressDlg()
		: m_hWnd(NULL)
		, m_lTotal(0)
		, m_lDone(0)
	{ }
	~CProgressDlg()
	{ }

	enum { IDD = IDD_RELAY };

	static INT_PTR CALLBACK DlgProcEntry(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(hwnd);
		UNREFERENCED_PARAMETER(wParam);

		if (msg == WM_INITDIALOG) {
			CProgressDlg* pThis = (CProgressDlg*)lParam;
#ifdef _WIN64
			SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pThis);
#else
			SetWindowLongPtr(hwnd, DWLP_USER, PtrToLong(pThis));
#endif
			return pThis->OnInitDialog(lParam);
		}
		return FALSE;
	}

	HWND Start(HWND hwndParent, long lTotal)
	{
		m_lTotal = lTotal;
		m_hWnd = ::CreateDialogParam(ghInst, MAKEINTRESOURCE(IDD), hwndParent, DlgProcEntry, (LPARAM)this);
		return m_hWnd;
	}

	void Stop()
	{
		if (::IsWindow(m_hWnd))
			::DestroyWindow(m_hWnd);
	}

	void SetTitle(LPCTSTR pszTitle)
	{
		if (::IsWindow(m_hWnd)) {
			::SetDlgItemText(m_hWnd, IDC_TITLE, pszTitle);
			ZapMessage();
		}
	}

	void SetComplete(long lDone)
	{
		if (::IsWindow(m_hWnd)) {
			m_lDone += lDone;
			int par = MulDiv(m_lDone, 100, m_lTotal);
			::SendDlgItemMessage(m_hWnd, IDC_METER, PBM_SETPOS, par, 0);
			ZapMessage();
		}
	}

	BOOL ZapMessage()
	{
		MSG	msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!::IsDialogMessage(m_hWnd, &msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		return TRUE;
	}

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		CenterWindow(m_hWnd, NULL);
		::SendDlgItemMessage(m_hWnd, IDC_METER, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		::SendDlgItemMessage(m_hWnd, IDC_METER, PBM_SETPOS, 0, 0);
		return TRUE;
	}
};



#endif		// __METER_H__

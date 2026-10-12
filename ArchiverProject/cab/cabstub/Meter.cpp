#include "stdafx.h"
#include "cabstub.h"

/////////////////////////////////////////////////////////////////////////////
// ProgressDlgFn dialog

class CProgressInfo
{
public:
	CProgressInfo() : m_lTotal(0), m_lDone(0)
	{
	}

	long m_lTotal;
	long m_lDone;
};

static BOOL CALLBACK ProgressDlgFn(HWND hdlg, UINT msg, WPARAM /*wParam*/, LPARAM lParam)
{
	CProgressInfo* pmi;
	switch (msg) {
		case WM_INITDIALOG:
			CenterWindow(hdlg, NULL);
			pmi = (CProgressInfo*)lParam;
			SetWindowLongPtr(hdlg, DWLP_USER, PtrToLong(pmi));

			SendDlgItemMessage(hdlg, IDC_METER, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendDlgItemMessage(hdlg, IDC_METER, PBM_SETPOS, 0, 0);
			return TRUE;

		case WM_DESTROY:
#ifdef _WIN64
			pmi = (CProgressInfo*)GetWindowLongPtr(hdlg, DWLP_USER);
#else
			pmi = (CProgressInfo*)LongToPtr(GetWindowLongPtr(hdlg, DWLP_USER));
#endif	// _WIN64
			if (pmi)
				free(pmi);
			break;

		default:
			break;
	}
	return FALSE;
}

BOOL Progress_ZapMessage(HWND hdlg)
{
	MSG	msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!::IsDialogMessage(hdlg, &msg)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	return TRUE;
}

HWND Progress_Create(HWND hwnd, long lTotal)
{
	CProgressInfo* pmi = new CProgressInfo;
	pmi->m_lTotal = lTotal;
	return CreateDialogParam(ghInst, MAKEINTRESOURCE(IDD_RELAY), hwnd, (DLGPROC)ProgressDlgFn, (LPARAM)pmi);
}

void Progress_Close(HWND hdlg)
{
	if (IsWindow(hdlg))
		DestroyWindow(hdlg);
}

void Progress_SetTitle(HWND hdlg, LPCTSTR pszTitle)
{
	if (IsWindow(hdlg)) {
		SetDlgItemText(hdlg, IDC_TITLE, pszTitle);
		Progress_ZapMessage(hdlg);
	}
}

void Progress_SetComplete(HWND hdlg, long lDone)
{
	if (IsWindow(hdlg)) {
		CProgressInfo* pmi;
#ifdef _WIN64
		pmi = (CProgressInfo*)GetWindowLongPtr(hdlg, DWLP_USER);
#else
		pmi = (CProgressInfo*)LongToPtr(GetWindowLongPtr(hdlg, DWLP_USER));
#endif	// _WIN64
		pmi->m_lDone += lDone;
		int par = MulDiv(pmi->m_lDone, 100, pmi->m_lTotal);
		SendDlgItemMessage(hdlg, IDC_METER, PBM_SETPOS, par, 0);
		Progress_ZapMessage(hdlg);
	}
}


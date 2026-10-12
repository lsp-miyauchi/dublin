//////////////////////////////////////////////////////////////////////
// grep.h

#ifndef _GREP_H_
#define _GREP_H_

#include "regexp.h"

/////////////////////////////////////////////////////////////////////////////
// CGrepDlg ダイアログ

class CGrepDlg : public CDialogImpl<CGrepDlg>
{
public:
	CGrepDlg() { m_pGrep = NULL; }
	~CGrepDlg() { }

	enum { IDD = IDD_GREP };

	CGrep* m_pGrep;

BEGIN_MSG_MAP(CGrepDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	MESSAGE_HANDLER(WM_HELP, OnHelpCall)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_CONTROL(FTWN_SELCHANGED, IDC_TREE, OnTreeChanged)
	ON_BN_CLICKED(IDC_USEJRE, OnUseJre)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	LRESULT OnHelpCall(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnTreeChanged();
	void OnUseJre();
};

/////////////////////////////////////////////////////////////////////////////
// CGrepPProg ダイアログ

class CGrepProg : public CDialogImpl<CGrepProg>
{
public:
	CGrepProg() { m_bCanceled = FALSE; m_found = 0; }
	~CGrepProg() { if (IsWindow()) DestroyWindow();  }

	enum { IDD = IDD_GREPPROG };

	BOOL m_bCanceled;
	int m_found;

BEGIN_MSG_MAP(CGrepProg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		return TRUE;
	}

	void OnCancel()
	{
		m_bCanceled = TRUE;
		ShowWindow(SW_HIDE);
	}

	BOOL IsCanceled() { return m_bCanceled; }

	void SetFolder(LPCTSTR pszPath)
	{
		if (IsWindow()) {
			PathSetDlgItemPath(m_hWnd, IDC_PATH, pszPath);
			ZapMessage();
		}
	}

	void SetFile(LPCTSTR pszFile)
	{
		if (IsWindow()) {
			SetDlgItemText(IDC_FILE, pszFile);
			ZapMessage();
		}
	}

	void Countup()
	{
		m_found++;
		if (IsWindow()) {
			SetDlgItemInt(IDC_FOUND, m_found, FALSE);
			ZapMessage();
		}
	}

	BOOL ZapMessage()
	{
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!IsDialogMessage(&msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			if (IsWindow()) {
				if (m_bCanceled)
					return FALSE;
			}
		}
		return TRUE;
	}
};

#endif		// _GREP_H_

/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "fopstd.h"

/////////////////////////////////////////////////////////////////////////////
// CDiscStreamDlg dialog

class CDiscStreamDlg : public CDialogImpl<CDiscStreamDlg>
{
public:
	DISCSTREAM* m_pds;

	CDiscStreamDlg() : m_pds(NULL), m_idTitle(0), m_idMessage(0) { }
	~CDiscStreamDlg() { }

	enum { IDD = IDD_DISCARDSTREAM };

	UINT m_idTitle;
	UINT m_idMessage;

BEGIN_MSG_MAP(CDiscStreamDlg)
	ON_WM_INITDIALOG()
	ON_WM_COMMAND()
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnCommand(UINT id, UINT code, HWND hwnd);
};

BOOL CDiscStreamDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pds = (DISCSTREAM*)lParam;

	CString str;
	if (m_idTitle) {
		str.LoadString(m_idTitle);
		SetWindowText(str);
	}
	if (m_idMessage) {
		str.LoadString(m_idMessage);
		SetDlgItemText(IDC_TEXT, str);
	}

	MessageBeep(0);

	TCHAR buf[MAX_PATH];
	SetDlgItemText(IDC_NAME, PathFindFileName(m_pds->szName));
	PathGetParent(m_pds->szName, buf);
	SetDlgItemText(IDC_PATH, buf);
	if (m_pds->numfiles < 2)
		GetDlgItem(IDC_YESTOALL).EnableWindow(FALSE);
	return TRUE;
}

BOOL CDiscStreamDlg::OnCommand(UINT id, UINT code, HWND hwnd)
{
	switch (id) {
	case IDOK:
		m_pds->operation = DSO_YES;
		EndDialog(IDOK);
		break;

	case IDC_YESTOALL:
		m_pds->operation = DSO_YESTOALL;
		EndDialog(IDOK);
		break;

	case IDC_SKIP:
		m_pds->operation = DSO_SKIP;
		EndDialog(IDOK);
		break;

	case IDCANCEL:
		m_pds->operation = DSO_CANCEL;
		EndDialog(IDCANCEL);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////
//

BOOL QueryDiscardStream(HWND hwnd, DISCSTREAM* pds)
{
	CDiscStreamDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)pds) != IDOK)
		pds->operation = DSO_CANCEL;
	return (pds->operation != DSO_CANCEL);
}

////////////////////////////////////////////////////////////////
//

BOOL QueryDecryptCopy(HWND hwnd, DISCSTREAM* pds)
{
	CDiscStreamDlg dlg;
	dlg.m_idTitle = IDS_CAPTION_DECRYPTCOPY;
	dlg.m_idMessage = IDS_QUERY_DECRYPTCOPY;
	if (dlg.DoModal(hwnd, (LPARAM)pds) != IDOK)
		pds->operation = DSO_CANCEL;
	return (pds->operation != DSO_CANCEL);
}

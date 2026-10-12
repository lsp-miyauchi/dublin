#include "stdafx.h"
#include "cab.h"

static const WCHAR sszKeyMode[] = L"CompressionMode";
static const WCHAR sszKeyLevel[] = L"LzxCompressLevel";


//////////////////////////////////////////////////////////////////////
//

class CConfigDlg : public CDialogImpl<CConfigDlg>
{
public:
	CConfigDlg() 
	{
	}
	~CConfigDlg() { }

	enum { IDD = IDD_CONFIG };

BEGIN_MSG_MAP(CConfigDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		CenterWindow();

		BOOL bLzx = GetRegInt(NULL, sszKeyMode, FALSE);
		CheckDlgButton(bLzx ? IDC_PACK_LZX : IDC_PACK_MSZIP, TRUE);

		m_spin = GetDlgItem(IDC_LZX_SPIN);
		m_spin.SetRange(15, 21);
		int level = GetRegInt(NULL, sszKeyLevel, 15);
		m_spin.SetPos(level);

		return TRUE;
	}

	void OnOK()
	{
		BOOL bLzx = IsDlgButtonChecked(IDC_PACK_LZX);
		SetRegInt(NULL, sszKeyMode, bLzx);

		int level = m_spin.GetPos();
		SetRegInt(NULL, sszKeyLevel, level);

		EndDialog(IDOK);
	}

	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}

	CUpDownCtrl m_spin;
};

////////////////////////////////////////////////////////////////////////
//

BOOL DoCabConfig(HWND hwndParent)
{
	CConfigDlg dlg;
	return (dlg.DoModal(hwndParent) == IDOK);
}

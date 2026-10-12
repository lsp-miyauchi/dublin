// NewImage_ui.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewImageDlg ダイアログ

typedef struct tagNEWIMAGEINFO {
	int width;
	int height;
	COLORREF clr;
} NEWIMAGEINFO;

class CNewImageDlg : public CDialogImpl<CNewImageDlg>
{
public:
	CNewImageDlg();

	enum { IDD = IDD_NEWIMAGE };
	NEWIMAGEINFO m_nii;

BEGIN_MSG_MAP(CNewImageDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
};

CNewImageDlg::CNewImageDlg()
{
	m_nii.width = 480;
	m_nii.height = 360;
	m_nii.clr = RGB(255, 255, 255);
}

/////////////////////////////////////////////////////////////////////////////
// CNewImageDlg メッセージ ハンドラ

BOOL CNewImageDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	SetDlgItemInt(IDC_XSIZE, m_nii.width, FALSE);
	SetDlgItemInt(IDC_YSIZE, m_nii.height, FALSE);
	ColorPicker_SetBkColor(GetDlgItem(IDC_COLOR), m_nii.clr);
	return TRUE;
}

void CNewImageDlg::OnOK() 
{
	UINT idErr = 0;
	m_nii.width = GetDlgItemInt(IDC_XSIZE, NULL, FALSE);
	if (m_nii.width < 1) idErr = IDC_XSIZE;

	m_nii.height = GetDlgItemInt(IDC_YSIZE, NULL, FALSE);
	if (m_nii.height < 1) idErr = IDC_YSIZE;

	m_nii.clr = ColorPicker_GetBkColor(GetDlgItem(IDC_COLOR));

	if (idErr) {
		MessageBeep(0);
		GetDlgItem(idErr).SetFocus();
		return;
	}
	EndDialog(IDOK);
}

/////////////////////////////////////////////////////////////////////////////
//

HDIB create_newimage(HWND hwnd)
{
	CNewImageDlg dlg;
	NEWIMAGEINFO nii;
	CAppReg reg;
	if (reg.GetBinary(IDS_SEC_EDIT, IDS_KEY_NEWIMAGE, &nii, sizeof(nii)) == sizeof(nii))
		memcpy(&dlg.m_nii, &nii, sizeof(nii));

	if (dlg.DoModal(hwnd) == IDOK) {
		reg.SetBinary(IDS_SEC_EDIT, IDS_KEY_NEWIMAGE, &dlg.m_nii, sizeof(dlg.m_nii));
		return imgCreateSolid(dlg.m_nii.width, dlg.m_nii.height, dlg.m_nii.clr);
	}
	return NULL;
}

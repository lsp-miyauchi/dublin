// video_noise.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

/////////////////////////////////////////////////////////////////////
//

static HDIB WINAPI videonoise(HDIB hsrc, LPRECT prc, MIF_VIDEO* pvp)
{
	mif image;
	if (image.attachdib(hsrc) && image.video(prc, pvp))
		return image.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////
//

class CVideoNoiseDlg : public CDialogImpl<CVideoNoiseDlg>, public CFilterDialogImpl<CVideoNoiseDlg>
{
public:
	enum { IDD = IDD_VIDEONOISE };
	MIF_VIDEO m_vp;
	HDIB m_hdib;
	CComboBox m_combo;

	CVideoNoiseDlg();

BEGIN_MSG_MAP(CVideoNoiseDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CVideoNoiseDlg>)
	ON_BN_CLICKED(IDC_ADDITIVE, OnCheck)
	ON_BN_CLICKED(IDC_ROTATE, OnCheck)
	ON_CBN_SELCHANGE(IDC_LIST, OnSelChange)
END_MSG_MAP()

	void OnCheck();
	void OnSelChange();

	BOOL InitDialog(LPARAM lParam);
	HDIB GetPreview(HDIB hdib);
};

/////////////////////////////////////////////////////////////////////
//

CVideoNoiseDlg::CVideoNoiseDlg()
{
	m_vp.pattern_number = 2;
	m_vp.additive = TRUE;
	m_vp.rotated = FALSE;
}

BOOL CVideoNoiseDlg::InitDialog(LPARAM lParam)
{
	m_combo = GetDlgItem(IDC_LIST);
	m_combo.SetCurSel(m_vp.pattern_number);
	CheckDlgButton(IDC_ADDITIVE, m_vp.additive);
	CheckDlgButton(IDC_ROTATE, m_vp.rotated);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CVideoNoiseDlg::OnCheck()
{
	m_vp.additive = IsDlgButtonChecked(IDC_ADDITIVE);
	m_vp.rotated = IsDlgButtonChecked(IDC_ROTATE);
	UpdatePreview();
}

void CVideoNoiseDlg::OnSelChange()
{
	m_vp.pattern_number = m_combo.GetCurSel();
	UpdatePreview();
}

HDIB CVideoNoiseDlg::GetPreview(HDIB hdib)
{
	return videonoise(hdib, NULL, &m_vp);
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("noise\\video_noise");
static TCHAR keyLast[] = _T("last");

HANDLE video_noise_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CVideoNoiseDlg dlg;

	regkey reg(pfe->hkey, secRoot);
	MIF_VIDEO v;
	if (reg.getbinary(keyLast, &v, sizeof(v)) == sizeof(v))
		memcpy(&dlg.m_vp, &v, sizeof(v));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_vp, sizeof(dlg.m_vp));

		CWaitCursor wc;
		h = videonoise(hdib, prc, &dlg.m_vp);
	}
	return h;
}

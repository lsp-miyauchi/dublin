// blurgauss.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct _GAUSSIIR {
	int horz;
	int vert;
} GAUSSIIR;


static HDIB blur_gauss_iir2(HDIB hsrc, LPRECT prc, double horz, double vert, PROGRESSCALLBACK progress)
{
	mif src;
	if (src.attachdib(hsrc)) {
		src.m_progress = progress;
		if (src.blur_gauss_iir(prc, horz, vert)) return src.createdib();
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurGaussDlg ダイアログ


class CBlurGaussDlg : public CDialogImpl<CBlurGaussDlg>, public CFilterDialogImpl<CBlurGaussDlg>
{
public:
	CBlurGaussDlg();

	enum { IDD = IDD_BLURGAUSS };
	CTrackCtrl m_hslider;
	CTrackCtrl m_vslider;
	double m_horz;
	double m_vert;
	HDIB m_hdib;

BEGIN_MSG_MAP(CBlurGaussDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CBlurGaussDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CBlurGaussDlg::CBlurGaussDlg()
{
	m_horz = 5.0;
	m_vert = 5.0;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurGaussDlg メッセージ ハンドラ

BOOL CBlurGaussDlg::InitDialog(LPARAM lParam)
{
	m_hslider = GetDlgItem(IDC_HSLIDER);
	m_vslider = GetDlgItem(IDC_VSLIDER);

	m_hslider.SetRange(5, 200);
	m_hslider.SetPageSize(40);
	m_hslider.SetTicFreq(40);
	m_hslider.SetPos((int)(m_horz * 10.0));
	m_hslider.SetBuddyBase(1);

	m_vslider.SetRange(5, 200);
	m_vslider.SetPageSize(40);
	m_vslider.SetTicFreq(40);
	m_vslider.SetPos((int)(m_vert * 10.0));
	m_vslider.SetBuddyBase(1);

	SetPreviewImage(m_hdib);
	UpdatePreview();

	return TRUE;
}

void CBlurGaussDlg::HScroll(int val, HWND hBar)
{
	if (hBar == m_hslider)		m_horz = (double)val / 10.0;
	else if (hBar == m_vslider)	m_vert = (double)val / 10.0;
	else						return;
	UpdatePreview();
}

HDIB CBlurGaussDlg::GetPreview(HDIB hdib)
{
	return blur_gauss_iir2(hdib, NULL, m_horz, m_vert, NULL);
}


///////////////////////////////////////////////////////////////////////
//

static TCHAR sszKey[] = _T("GaussIIR");

HANDLE blur_gauss_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CBlurGaussDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, _T("blur\\blur_gauss"));
	GAUSSIIR gi;
	if (reg.getbinary(sszKey, &gi, sizeof(gi)) == sizeof(gi)) {
		dlg.m_horz = (double)gi.horz / 10.0;
		dlg.m_vert = (double)gi.vert / 10.0;
	}

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		gi.horz = (int)(dlg.m_horz * 10.0);
		gi.vert = (int)(dlg.m_vert * 10.0);
		reg.setbinary(sszKey, &gi, sizeof(gi));

		return blur_gauss_iir2(hdib, prc, dlg.m_horz, dlg.m_vert, pfe->progress);
	}
	return NULL;
}

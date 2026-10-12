// emboss.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static MIF_EMBOSS defep = {
	35.0, 45.0, 20, TRUE
};

static HDIB WINAPI emboss(HDIB hsrc, LPRECT prc, MIF_EMBOSS* pep)
{
	mif src;
	if (src.attachdib(hsrc) && src.emboss(prc, pep))
		return src.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CEmbossDlg ダイアログ

class CEmbossDlg : public CDialogImpl<CEmbossDlg>, public CFilterDialogImpl<CEmbossDlg>
{
public:
	enum { IDD = IDD_EMBOSS };
	MIF_EMBOSS m_ep;
	HDIB m_hdib;
	CTrackCtrl m_dslider;
	CTrackCtrl m_eslider;
	CTrackCtrl m_aslider;

	CEmbossDlg();

BEGIN_MSG_MAP(CEmbossDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CEmbossDlg>)
	ON_BN_CLICKED(IDC_EMBOSS, OnCheckEmboss)
	ON_BN_CLICKED(IDC_BUMPMAP, OnCheckEmboss)
END_MSG_MAP()

	void OnCheckEmboss();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CEmbossDlg::CEmbossDlg()
{
	memcpy(&m_ep, &defep, sizeof(m_ep));
}

/////////////////////////////////////////////////////////////////////////////
// CEmbossDlg メッセージ ハンドラ

BOOL CEmbossDlg::InitDialog(LPARAM lParam) 
{
	m_dslider = GetDlgItem(IDC_DEPTH);
	m_eslider = GetDlgItem(IDC_ELEVATION);
	m_aslider = GetDlgItem(IDC_AZIMUTH);

	CheckDlgButton(m_ep.embossp ? IDC_EMBOSS : IDC_BUMPMAP, TRUE);

	m_aslider.SetRange(0, 3600);
	m_aslider.SetPageSize(300);
	m_aslider.SetTicFreq(400);
	m_aslider.SetPos((int)(m_ep.azimuth * 10));
	m_aslider.SetBuddyBase(1);

	m_eslider.SetRange(5, 900);
	m_eslider.SetPageSize(100);
	m_eslider.SetTicFreq(100);
	m_eslider.SetPos((int)(m_ep.elevation * 10));
	m_eslider.SetBuddyBase(1);

	m_dslider.SetRange(1, 60);
	m_dslider.SetPageSize(6);
	m_dslider.SetTicFreq(10);
	m_dslider.SetPos(m_ep.depth);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CEmbossDlg::HScroll(int val, HWND hBar)
{
	if (hBar == m_aslider)		m_ep.azimuth = (double)val / 10.0;
	else if (hBar == m_eslider)	m_ep.elevation = (double)val / 10.0;
	else if (hBar == m_dslider)	m_ep.depth = val;
	else						return;

	UpdatePreview();
}

HDIB CEmbossDlg::GetPreview(HDIB hdib)
{
	return emboss(hdib, NULL, &m_ep);
}

void CEmbossDlg::OnCheckEmboss() 
{
	m_ep.embossp = IsDlgButtonChecked(IDC_EMBOSS);
	UpdatePreview();
}

/////////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("edge\\emboss");
static TCHAR keyLast[] = _T("last");

HANDLE emboss_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CEmbossDlg dlg;

	regkey reg(pfe->hkey, secRoot);
	MIF_EMBOSS e;
	if (reg.getbinary(keyLast, &e, sizeof(e)) == sizeof(e))
		memcpy(&dlg.m_ep, &e, sizeof(e));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_ep, sizeof(dlg.m_ep));

		CWaitCursor wc;
		h = emboss(hdib, prc, &dlg.m_ep);
	}
	return h;
}

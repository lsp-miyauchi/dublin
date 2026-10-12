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


static HDIB unsharp_mask(HDIB hsrc, LPRECT prc, MIF_UNSHARPMASK* pup, PROGRESSCALLBACK progress)
{
	mif src;
	if (src.attachdib(hsrc)) {
		src.m_progress = progress;
		if (src.unsharp_mask(prc, pup)) return src.createdib();
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CUnsharpDlg ダイアログ


class CUnsharpDlg : public CDialogImpl<CUnsharpDlg>, public CFilterDialogImpl<CUnsharpDlg>
{
public:
	CUnsharpDlg();

	enum { IDD = IDD_UNSHARP };
	CTrackCtrl m_rslider;
	CTrackCtrl m_aslider;
	CTrackCtrl m_tslider;
	MIF_UNSHARPMASK m_up;
	HDIB m_hdib;

BEGIN_MSG_MAP(CUnsharpDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CUnsharpDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CUnsharpDlg::CUnsharpDlg()
{
	m_up.radius = 5.0;
	m_up.amount = 0.5;
	m_up.threshold = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CUnsharpDlg メッセージ ハンドラ

BOOL CUnsharpDlg::InitDialog(LPARAM lParam)
{
	m_rslider = GetDlgItem(IDC_RSLIDER);
	m_aslider = GetDlgItem(IDC_ASLIDER);
	m_tslider = GetDlgItem(IDC_TSLIDER);

	m_rslider.SetRange(10, 250);
	m_rslider.SetPageSize(40);
	m_rslider.SetTicFreq(40);
	m_rslider.SetPos((int)(m_up.radius * 10.0));
	m_rslider.SetBuddyBase(1);

	m_aslider.SetRange(1, 500);
	m_aslider.SetPageSize(50);
	m_aslider.SetTicFreq(50);
	m_aslider.SetPos((int)(m_up.amount * 100.0));
	m_aslider.SetBuddyBase(2);

	m_tslider.SetRange(0, 255);
	m_tslider.SetPageSize(16);
	m_tslider.SetTicFreq(32);
	m_tslider.SetPos(m_up.threshold);

	SetPreviewImage(m_hdib);
	UpdatePreview();

	return TRUE;
}

void CUnsharpDlg::HScroll(int val, HWND hBar)
{
	if (hBar == m_rslider)		m_up.radius = (double)val / 10.0;
	else if (hBar == m_aslider)	m_up.amount = (double)val / 100.0;
	else if (hBar == m_tslider)	m_up.threshold = val;
	else						return;
	UpdatePreview();
}

inline HDIB CUnsharpDlg::GetPreview(HDIB hdib)
{
	return unsharp_mask(hdib, NULL, &m_up, NULL);
}


///////////////////////////////////////////////////////////////////////
//

static TCHAR sszKey[] = _T("UnsharpMask");

HANDLE unsharp_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CUnsharpDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, _T("sharpen\\unsharpmask"));
	if (reg.getbinary(sszKey, &dlg.m_up, sizeof(MIF_UNSHARPMASK)) != sizeof(MIF_UNSHARPMASK)) {
		dlg.m_up.radius = 4.0;
		dlg.m_up.amount = 0.3;
		dlg.m_up.threshold = 0;
	}

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(sszKey, &dlg.m_up, sizeof(MIF_UNSHARPMASK));
		return unsharp_mask(hdib, prc, &dlg.m_up, pfe->progress);
	}
	return NULL;
}

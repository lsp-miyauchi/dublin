// bumpmap.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static MIF_BUMPMAP defbp = {
	135.0, 45.0, 3, 0, 0, 0, 0, FALSE, FALSE, bumpmapLinear, FALSE
};

static HDIB bumpmap(HDIB hsrc, LPRECT prc, MIF_BUMPMAP* pbp)
{
	mif src;
	if (src.attachdib(hsrc) && src.bumpmap(prc, pbp))
		return src.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBumpmapDlg ダイアログ

class CBumpmapDlg : public CDialogImpl<CBumpmapDlg>, public CFilterDialogImpl<CBumpmapDlg>
{
public:
	enum { IDD = IDD_BUMPMAP };
	MIF_BUMPMAP m_bp;
	HDIB m_hdib;
	CTrackCtrl m_dslider;
	CTrackCtrl m_lslider;
	CTrackCtrl m_eslider;
	CTrackCtrl m_aslider;
	CComboBox m_combo;

	CBumpmapDlg();

BEGIN_MSG_MAP(CBumpmapDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CBumpmapDlg>)
	ON_BN_CLICKED(IDC_COMPENSATE, OnCheckButton)
	ON_BN_CLICKED(IDC_INVERT, OnCheckButton)
	ON_BN_CLICKED(IDC_LINEAR, OnCheckButton)
	ON_BN_CLICKED(IDC_SINUOSIDAL, OnCheckButton)
	ON_BN_CLICKED(IDC_SPHERICAL, OnCheckButton)
END_MSG_MAP()

	void OnCheckButton();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CBumpmapDlg::CBumpmapDlg()
{
	memcpy(&m_bp, &defbp, sizeof(m_bp));
	m_bp.xofs = 0;
	m_bp.yofs = 0;
	m_bp.waterlevel = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBumpmapDlg メッセージ ハンドラ

BOOL CBumpmapDlg::InitDialog(LPARAM lParam)
{
	m_dslider = GetDlgItem(IDC_DEPTH);
	m_lslider = GetDlgItem(IDC_LIGHTNING);
	m_eslider = GetDlgItem(IDC_ELEVATION);
	m_aslider = GetDlgItem(IDC_AZIMUTH);

	// アジマス - 方向
	m_aslider.SetRange(0, 3600);
	m_aslider.SetPageSize(300);
	m_aslider.SetTicFreq(400);
	m_aslider.SetPos((int)(m_bp.azimuth * 10));
	m_aslider.SetBuddyBase(1);

	// エレベーション - 持ち上げ
	m_eslider.SetRange(5, 900);
	m_eslider.SetPageSize(100);
	m_eslider.SetTicFreq(100);
	m_eslider.SetPos((int)(m_bp.elevation * 10));
	m_eslider.SetBuddyBase(1);

	// 深度
	m_dslider.SetRange(1, 65);
	m_dslider.SetPageSize(10);
	m_dslider.SetTicFreq(10);
	m_dslider.SetPos(m_bp.depth);

	// 飽和度
	m_lslider.SetRange(0, 255);
	m_lslider.SetPageSize(32);
	m_lslider.SetTicFreq(64);
	m_lslider.SetPos(m_bp.ambient);

	if (m_bp.compensate) CheckDlgButton(IDC_COMPENSATE, TRUE);
	if (m_bp.invert) CheckDlgButton(IDC_INVERT, TRUE);

	if (m_bp.type == bumpmapLinear) CheckDlgButton(IDC_LINEAR, TRUE);
	else if (m_bp.type == bumpmapSpherical) CheckDlgButton(IDC_SPHERICAL, TRUE);
	else CheckDlgButton(IDC_SINUOSIDAL, TRUE);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CBumpmapDlg::HScroll(int val, HWND hBar) 
{
	if (hBar == m_aslider)		m_bp.azimuth = (double)val / 10.0;
	else if (hBar == m_eslider)	m_bp.elevation = (double)val / 10.0;
	else if (hBar == m_dslider)	m_bp.depth = val;
	else if (hBar == m_lslider)	m_bp.ambient = val;
	else return;

	UpdatePreview();
}

void CBumpmapDlg::OnCheckButton() 
{
	m_bp.compensate = IsDlgButtonChecked(IDC_COMPENSATE);
	m_bp.invert = IsDlgButtonChecked(IDC_INVERT);

	if (IsDlgButtonChecked(IDC_LINEAR))				m_bp.type = bumpmapLinear;
	else if (IsDlgButtonChecked(IDC_SPHERICAL))		m_bp.type = bumpmapSpherical;
	else											m_bp.type = bumpmapSinuosidal;

	UpdatePreview();
}

HDIB CBumpmapDlg::GetPreview(HDIB hdib)
{
	return bumpmap(hdib, NULL, &m_bp);
}

/////////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("edge\\bumpmap");
static TCHAR keyLast[] = _T("last");

HANDLE bumpmap_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CBumpmapDlg dlg;

	regkey reg(pfe->hkey, secRoot);
	MIF_BUMPMAP b;
	if (reg.getbinary(keyLast, &b, sizeof(b)) == sizeof(b))
		memcpy(&dlg.m_bp, &b, sizeof(b));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_bp, sizeof(dlg.m_bp));

		CWaitCursor wc;
		h = bumpmap(hdib, prc, &dlg.m_bp);
	}
	return h;
}

// colorcut.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "ffactory_sup.h"
#include "..\filterdialog.h"

static const TCHAR* sszSpace[] = {
	_T("OHTA"), _T("sRGB"), _T("XYZ"), _T("YCbCr"), _T("YCC"), _T("YIQ"), _T("YPbPr"), _T("YUV"), NULL
};

HDIB WINAPI color_cut(HDIB hsrc, LPRECT prc, MIF_COLORCUT* pcp)
{
	mif image;
	if (image.attachdib(hsrc) && image.colorcut(prc, pcp))
		return image.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////
//

class CColorCutDlg : public CDialogImpl<CColorCutDlg>, public CFilterDialogImpl<CColorCutDlg>
{
public:
	enum { IDD = IDD_COLORCUT };
	MIF_COLORCUT m_cc;
	HDIB m_hdib;
	CTrackCtrl m_xslider;
	CTrackCtrl m_yslider;
	CTrackCtrl m_zslider;

	CColorCutDlg();

BEGIN_MSG_MAP(CColorCutDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CColorCutDlg>)
	ON_CBN_SELCHANGE(IDC_LIST, OnSelChange)
END_MSG_MAP()

	void OnSelChange();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CColorCutDlg::CColorCutDlg()
{
	m_cc.xstep = 32;
	m_cc.ystep = 16;
	m_cc.zstep = 16;
	m_cc.color_space = clrspcYUV;
}

BOOL CColorCutDlg::InitDialog(LPARAM lParam)
{
	m_xslider = GetDlgItem(IDC_XSLIDER);
	m_yslider = GetDlgItem(IDC_YSLIDER);
	m_zslider = GetDlgItem(IDC_ZSLIDER);

	m_xslider.SetRange(1, 256, TRUE);
	m_xslider.SetPageSize(16);
	m_xslider.SetTicFreq(32);
	m_xslider.SetPos(m_cc.xstep, TRUE);
	m_yslider.SetRange(1, 256, TRUE);
	m_yslider.SetPageSize(16);
	m_yslider.SetTicFreq(32);
	m_yslider.SetPos(m_cc.ystep, TRUE);
	m_zslider.SetRange(1, 256, TRUE);
	m_zslider.SetPageSize(16);
	m_zslider.SetTicFreq(32);
	m_zslider.SetPos(m_cc.zstep, TRUE);

	CComboBox combo = GetDlgItem(IDC_LIST);
	int i = 0;
	while (sszSpace[i]) {
		combo.AddString(sszSpace[i]);
		i++;
	}
	combo.SetCurSel(m_cc.color_space);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CColorCutDlg::OnSelChange()
{
	CComboBox combo = GetDlgItem(IDC_LIST);
	m_cc.color_space = combo.GetCurSel();
	UpdatePreview();
}

void CColorCutDlg::HScroll(int val, HWND hbar)
{
	switch (::GetDlgCtrlID(hbar)) {
	case IDC_XSLIDER: m_cc.xstep = val; break;
	case IDC_YSLIDER: m_cc.ystep = val; break;
	case IDC_ZSLIDER: m_cc.zstep = val; break;
	default: return;
	}
	UpdatePreview();
}

HDIB CColorCutDlg::GetPreview(HDIB hdib)
{
	return color_cut(hdib, NULL, &m_cc);
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("color\\color_cut");
static TCHAR keyLast[] = _T("last");

HANDLE color_cut_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CColorCutDlg dlg;

	MIF_COLORCUT c;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &c, sizeof(c)) == sizeof(c))
		memcpy(&dlg.m_cc, &c, sizeof(c));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent, (LPARAM)&c) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &c, sizeof(c));

		CWaitCursor wc;
		return color_cut(hdib, prc, &dlg.m_cc);
	}
	return NULL;
}

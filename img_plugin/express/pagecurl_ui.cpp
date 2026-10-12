//////////////////////////////////////////////////////////////////
// pagecurl_ui.cpp

#include "stdafx.h"
#include "pagecurl.h"
#include "..\filterdialog.h"

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();


//////////////////////////////////////////////////////////////////////
//

class CPageCurlDlg : public CDialogImpl<CPageCurlDlg>, public CFilterDialogImpl<CPageCurlDlg>
{
public:
	CPageCurlDlg();

	enum { IDD = IDD_PAGECURL };
	HDIB m_hdib;
	PAGECURLINFO m_ci;

BEGIN_MSG_MAP(CPageCurlDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CPageCurlDlg>)
	ON_BN_CLICKED(IDC_VERT, OnCheckParam)
	ON_BN_CLICKED(IDC_UPPERLEFT, OnCheckParam)
	ON_BN_CLICKED(IDC_UPPERRIGHT, OnCheckParam)
	ON_BN_CLICKED(IDC_LOWERLEFT, OnCheckParam)
	ON_BN_CLICKED(IDC_LOWERRIGHT, OnCheckParam)
	ON_CBN_SELCHANGE(IDC_LIGHTENCOLOR, OnCheckParam)
	ON_CBN_SELCHANGE(IDC_DARKENCOLOR, OnCheckParam)
	ON_CBN_SELCHANGE(IDC_BACKCOLOR, OnCheckParam)
END_MSG_MAP()

	void OnCheckParam();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);

};

CPageCurlDlg::CPageCurlDlg()
{
	m_hdib = NULL;
	m_ci.corner = curlLowerRight;
	m_ci.vert = TRUE;
	m_ci.opaque = 100;
	m_ci.xper = 100;
	m_ci.yper = 100;
	m_ci.darken_color  = RGB(0, 0, 0);
	m_ci.lighten_color = RGB(255, 255, 255);
	m_ci.back_color    = RGB(255, 255, 255);
}

BOOL CPageCurlDlg::InitDialog(LPARAM lParam)
{
	// めくる場所
	switch (m_ci.corner) {
		case curlUpperLeft:		CheckDlgButton(IDC_UPPERLEFT, TRUE); break;
		case curlLowerLeft:		CheckDlgButton(IDC_LOWERLEFT, TRUE); break;
		case curlUpperRight:	CheckDlgButton(IDC_UPPERRIGHT, TRUE); break;
		default:				CheckDlgButton(IDC_LOWERRIGHT, TRUE); break;
	}

	// めくる方向
	CheckDlgButton(IDC_VERT, m_ci.vert);

	// 色
	ColorPicker_SetBkColor(GetDlgItem(IDC_LIGHTENCOLOR), m_ci.lighten_color);
	ColorPicker_SetBkColor(GetDlgItem(IDC_DARKENCOLOR), m_ci.darken_color);
	ColorPicker_SetBkColor(GetDlgItem(IDC_BACKCOLOR), m_ci.back_color);

	// 不透明度
	CTrackCtrl track = GetDlgItem(IDC_SLIDER);
	track.SetRange(0, 100);
	track.SetTicFreq(10);
	track.SetPos(m_ci.opaque);

	track = GetDlgItem(IDC_XSLIDER);
	track.SetRange(0, 100);
	track.SetTicFreq(10);
	track.SetPos(m_ci.xper);

	track = GetDlgItem(IDC_YSLIDER);
	track.SetRange(0, 100);
	track.SetTicFreq(10);
	track.SetPos(m_ci.yper);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CPageCurlDlg::HScroll(int val, HWND hbar)
{
	switch (::GetDlgCtrlID(hbar)) {
		case IDC_SLIDER: m_ci.opaque = val; break;
		case IDC_XSLIDER: m_ci.xper = val; break;
		case IDC_YSLIDER: m_ci.yper = val; break;
		default: return;
	}
	UpdatePreview();
}

void CPageCurlDlg::OnCheckParam()
{
	// めくる場所
	if (IsDlgButtonChecked(IDC_UPPERLEFT))			m_ci.corner = curlUpperLeft;
	else if (IsDlgButtonChecked(IDC_LOWERLEFT))		m_ci.corner = curlLowerLeft;
	else if (IsDlgButtonChecked(IDC_UPPERRIGHT))	m_ci.corner = curlUpperRight;
	else											m_ci.corner = curlLowerRight;

	// めくる方向
	m_ci.vert = IsDlgButtonChecked(IDC_VERT);

	// 色
	m_ci.lighten_color = ColorPicker_GetBkColor(GetDlgItem(IDC_LIGHTENCOLOR));
	m_ci.darken_color = ColorPicker_GetBkColor(GetDlgItem(IDC_DARKENCOLOR));
	m_ci.back_color = ColorPicker_GetBkColor(GetDlgItem(IDC_BACKCOLOR));

	UpdatePreview();
}

HDIB CPageCurlDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	page_curl(image, NULL, &m_ci);
	return image.createdib();
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("expression\\pagecurl");
static TCHAR keyLast[] = _T("last");

HANDLE pagecurl_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CPageCurlDlg dlg;

	PAGECURLINFO ci;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &ci, sizeof(ci)) == sizeof(ci))
		memcpy(&dlg.m_ci, &ci, sizeof(ci));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_ci, sizeof(dlg.m_ci));

		image.attachdib(hdib);
pfnProgress = pfe->progress;
		page_curl(image, prc, &dlg.m_ci);
pfnProgress = NULL;
		return image.createdib();
	}
	return NULL;
}

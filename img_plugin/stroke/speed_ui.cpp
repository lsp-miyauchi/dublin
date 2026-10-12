// speed_ui.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct tagSTROKESPEED {
	BOOL blast;
	int threshold;
	int strength;
	int direction;
	int edge;
} STROKESPEED;

static HDIB stroke_speed(HDIB hsrc, LPRECT prc, int alg, int threshold, int strength, int direction, int edge)
{
	mif src;
	if (!src.attachdib(hsrc)) return NULL;

	BOOL bOK;
	if (alg)	bOK = src.sblur_wind(prc, threshold, strength, direction, edge);
	else		bOK = src.sblur_blast(prc, threshold, strength, direction, edge);

	return bOK ? src.createdib() : NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CStrokeSpeedDlg ダイアログ

class CStrokeSpeedDlg : public CDialogImpl<CStrokeSpeedDlg>, public CFilterDialogImpl<CStrokeSpeedDlg>
{
public:
	enum { IDD = IDD_STROKE_SPEED };
	STROKESPEED m_sp;
	HDIB m_hdib;
	CTrackCtrl m_tslider;
	CTrackCtrl m_sslider;
	CComboBox m_dircombo;

	CStrokeSpeedDlg();

BEGIN_MSG_MAP(CStrokeSpeedDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CStrokeSpeedDlg>)
	ON_BN_CLICKED(IDC_BLAST, OnCheckBlast)
	ON_BN_CLICKED(IDC_LEADING, OnCheckEdge)
	ON_BN_CLICKED(IDC_TRAILING, OnCheckEdge)
	ON_BN_CLICKED(IDC_BOTH, OnCheckEdge)
	ON_CBN_SELCHANGE(IDC_COMBO, OnSelchangeDirection)
END_MSG_MAP()

	void OnCheckBlast();
	void OnCheckEdge();
	void OnSelchangeDirection();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CStrokeSpeedDlg::CStrokeSpeedDlg()
{
	m_sp.threshold = 10;
	m_sp.strength = 10;
	m_sp.direction = sblurLeft;
	m_sp.edge = sblurLeading;
	m_sp.blast = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBlurSpeed メッセージ ハンドラ

BOOL CStrokeSpeedDlg::InitDialog(LPARAM lParam)
{
	m_dircombo = GetDlgItem(IDC_COMBO);
	m_tslider = GetDlgItem(IDC_THRESHOLD);
	m_sslider = GetDlgItem(IDC_STRENGTH);

	m_dircombo.SetCurSel(m_sp.direction);

	switch (m_sp.edge) {
		case sblurLeading:	CheckDlgButton(IDC_LEADING, TRUE); break;
		case sblurTrailing:	CheckDlgButton(IDC_TRAILING, TRUE); break;
		case sblurBoth:
		default:			CheckDlgButton(IDC_BOTH, TRUE); break;
	}
	CheckDlgButton(IDC_BLAST, m_sp.blast);

	m_tslider.SetRange(0, 50);
	m_tslider.SetTicFreq(10);
	m_tslider.SetPos(m_sp.threshold);

	m_sslider.SetRange(1, 50);
	m_sslider.SetTicFreq(10);
	m_sslider.SetPos(m_sp.strength);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CStrokeSpeedDlg::HScroll(int val, HWND hBar) 
{
	switch (::GetDlgCtrlID(hBar)) {
		case IDC_THRESHOLD:	m_sp.threshold = val; break;
		case IDC_STRENGTH:	m_sp.strength = val; break;
		default:			return;
	}
	UpdatePreview();
}

HDIB CStrokeSpeedDlg::GetPreview(HDIB hdib)
{
	return stroke_speed(hdib, NULL, m_sp.blast, m_sp.threshold, m_sp.strength, m_sp.direction, m_sp.edge);
}

void CStrokeSpeedDlg::OnCheckBlast() 
{
	m_sp.blast = IsDlgButtonChecked(IDC_BLAST);
	UpdatePreview();
}

void CStrokeSpeedDlg::OnCheckEdge()
{
	if (IsDlgButtonChecked(IDC_LEADING)) m_sp.edge = sblurLeading;
	else if (IsDlgButtonChecked(IDC_TRAILING)) m_sp.edge = sblurTrailing;
	else m_sp.edge = sblurBoth;
	UpdatePreview();
}

void CStrokeSpeedDlg::OnSelchangeDirection() 
{
	m_sp.direction = m_dircombo.GetCurSel();
	UpdatePreview();
}

/////////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("stroke\\stroke_speed");
static TCHAR keyLast[] = _T("last");

HANDLE stroke_speed_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CStrokeSpeedDlg dlg;

	regkey reg(pfe->hkey, secRoot);
	STROKESPEED s;
	if (reg.getbinary(keyLast, &s, sizeof(s)) == sizeof(s))
		memcpy(&dlg.m_sp, &s, sizeof(s));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_sp, sizeof(dlg.m_sp));

		CWaitCursor wc;
		h = stroke_speed(hdib, prc, dlg.m_sp.blast, dlg.m_sp.threshold, dlg.m_sp.strength, dlg.m_sp.direction, dlg.m_sp.edge);
	}
	return h;
}

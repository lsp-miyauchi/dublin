//////////////////////////////////////////////////////////////////////
// grad_blinds_ui.cpp

#include "stdafx.h"
#include "..\filterdialog.h"

extern HINSTANCE ghInst;

typedef struct tagGRADBLINDS {
	int division;
	int red;
	int green;
	int blue;
	int decline;
	BOOL vert;
} GRADBLINDS;

static int map_val(int n, int vmin, int vmax)
{
	double delta = (double)(vmax - vmin) / 255.0;
	return vmin + (int)((double)n * delta);
}

BOOL grad_blinds(mif& image, LPRECT prc, GRADBLINDS* pgb)
{
	CRect rc;
	if (!image.validaterect(prc, &rc)) return FALSE;

	int r = map_val(pgb->red,   0, pgb->decline);
	int g = map_val(pgb->green, 0, pgb->decline);
	int b = map_val(pgb->blue,  0, pgb->decline);

	int x, y, c;
	RGBA* ps;
	if (pgb->vert) {
		for (y = 0; y < rc.Height(); y++) {
			ps = image.pos(rc.left, y + rc.top);
			for (x = 0; x < rc.Width(); x++) {
				c = (int)ps->red - (x % pgb->division) * r;
				ps->red = min(255, max(0, c));
				c = (int)ps->green - (x % pgb->division) * g;
				ps->green = min(255, max(0, c));
				c = (int)ps->blue - (x % pgb->division) * b;
				ps->blue = min(255, max(0, c));
				ps++;
			}
		}
	}
	else {
		for (x = 0; x < rc.Width(); x++) {
			ps = image.pos(x + rc.left, rc.top);
			for (y = 0; y < rc.Height(); y++) {
				c = (int)ps->red - (y % pgb->division) * r;
				ps->red = min(255, max(0, c));
				c = (int)ps->green - (y % pgb->division) * g;
				ps->green = min(255, max(0, c));
				c = (int)ps->blue - (y % pgb->division) * b;
				ps->blue = min(255, max(0, c));
				ps += image.m_width;
			}
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//

class CGradBlindsDlg : public CDialogImpl<CGradBlindsDlg>, public CFilterDialogImpl<CGradBlindsDlg>
{
public:
	CGradBlindsDlg();

	enum { IDD = IDD_GRADBLINDS };
	GRADBLINDS m_gb;
	HDIB m_hdib;

BEGIN_MSG_MAP(CGradBlindsDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CGradBlindsDlg>)
	ON_BN_CLICKED(IDC_VERT, OnCheckVert)
END_MSG_MAP()

	void OnCheckVert();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CGradBlindsDlg::CGradBlindsDlg()
{
	m_hdib = NULL;
	m_gb.division = 20;
	m_gb.red = m_gb.green = m_gb.blue = 255;
	m_gb.decline = 10;
	m_gb.vert = TRUE;
}

BOOL CGradBlindsDlg::InitDialog(LPARAM lParam)
{
	CTrackCtrl track = GetDlgItem(IDC_DSLIDER);
	track.SetRange(1, 100);
	track.SetTicFreq(10);
	track.SetPos(m_gb.division);

	track = GetDlgItem(IDC_RSLIDER);
	track.SetRange(0, 255);
	track.SetTicFreq(64);
	track.SetPos(m_gb.red);

	track = GetDlgItem(IDC_GSLIDER);
	track.SetRange(0, 255);
	track.SetTicFreq(64);
	track.SetPos(m_gb.green);

	track = GetDlgItem(IDC_BSLIDER);
	track.SetRange(0, 255);
	track.SetTicFreq(64);
	track.SetPos(m_gb.blue);

	track = GetDlgItem(IDC_LSLIDER);
	track.SetRange(2, 30);
	track.SetTicFreq(7);
	track.SetPos(m_gb.decline);

	CheckDlgButton(IDC_VERT, m_gb.vert);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CGradBlindsDlg::OnCheckVert()
{
	m_gb.vert = IsDlgButtonChecked(IDC_VERT);
	UpdatePreview();
}

void CGradBlindsDlg::HScroll(int pos, HWND hbar)
{
	switch (::GetDlgCtrlID(hbar)) {
		case IDC_DSLIDER: m_gb.division = pos; break;
		case IDC_RSLIDER: m_gb.red = pos; break;
		case IDC_GSLIDER: m_gb.green = pos; break;
		case IDC_BSLIDER: m_gb.blue = pos; break;
		case IDC_LSLIDER: m_gb.decline = pos; break;
		default: return;
	}
	UpdatePreview();
}

HDIB CGradBlindsDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	grad_blinds(image, NULL, &m_gb);
	return image.createdib();
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("expression\\grad_blinds");
static TCHAR keyLast[] = _T("last");

HANDLE grad_blinds_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CGradBlindsDlg dlg;

	GRADBLINDS g;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &g, sizeof(g)) == sizeof(g))
		memcpy(&dlg.m_gb, &g, sizeof(g));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_gb, sizeof(dlg.m_gb));

		CWaitCursor wc;
		image.attachdib(hdib);
		grad_blinds(image, prc, &dlg.m_gb);
		return image.createdib();
	}
	return NULL;
}

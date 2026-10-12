// gradmap.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct tagGRADMAP {
	BOOL whole;
	int numcolors;
	GRADCOLORS grad[MAX_GRADCOLORS];
} GRADMAP;


#define INTENS(r,g,b)		((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29))

static BOOL gradation_map(mif& image, LPRECT prc, GRADCOLORS* pcol, int numcolors)
{
	CRect rc;
	if (image.m_bits == 24 && !image.validaterect(prc, &rc)) return FALSE;

	LOGPALETTE* plgp = image.make_gradtable(pcol, numcolors, 1000);
	if (!plgp) return FALSE;

	RGBA* p;
	int lum;
	if (image.m_bits < 24) {
		p = image.m_map;
		int cnt = image.m_colors;
		while (cnt-- > 0) {
			lum = INTENS(p->red, p->green, p->blue);
			lum = MulDiv(lum, 999, 0xff00);
			p->red   = plgp->palPalEntry[lum].peRed;
			p->green = plgp->palPalEntry[lum].peGreen;
			p->blue  = plgp->palPalEntry[lum].peBlue;
			p++;
		}
	}
	else {
		for (int y = rc.top; y < rc.bottom; y++) {
			p = image.pos(rc.left, y);
			for (int x = rc.left; x < rc.right; x++) {
				lum = INTENS(p->red, p->green, p->blue);
				lum = MulDiv(lum, 999, 0xff00);
				p->red   = plgp->palPalEntry[lum].peRed;
				p->green = plgp->palPalEntry[lum].peGreen;
				p->blue  = plgp->palPalEntry[lum].peBlue;
				p++;
			}
		}
	}
	image.free_gradtable(plgp);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGradMapDlg ダイアログ

class CGradMapDlg : public CDialogImpl<CGradMapDlg>, public CFilterDialogImpl<CGradMapDlg>
{
public:
	CGradMapDlg();

	enum { IDD = IDD_GRADMAP };
	GRADMAP m_gm;
	HDIB m_hdib;
	FILTER_EXEC* m_pfe;

BEGIN_MSG_MAP(CGradMapDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CGradMapDlg>)
	ON_BN_CLICKED(IDC_GRADEDIT, OnLoadPreset)
	ON_BN_CLICKED(IDC_RESET, OnGradReset)
	ON_BN_CLICKED(IDC_WHOLEVIEW, OnCheckWhole)
	ON_CONTROL(GEN_COLORCHANGE, IDC_COLOR, OnColorChanged)
END_MSG_MAP()

	void OnCheckWhole();
	void OnColorChanged();
	void OnLoadPreset();
	void OnGradReset();

	BOOL InitDialog(LPARAM lParam);
	HDIB GetPreview(HDIB hdib);
};

CGradMapDlg::CGradMapDlg()
{
	m_pfe = NULL;
	m_gm.numcolors = 2;
	m_gm.grad[0].cr = RGB(0, 0, 0);
	m_gm.grad[0].pos = 0;
	m_gm.grad[1].cr = RGB(255, 255, 255);
	m_gm.grad[1].pos = 1000;
	m_gm.whole = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CGradMapDlg メッセージ ハンドラ

BOOL CGradMapDlg::InitDialog(LPARAM lParam)
{
	// グラデーション色
	GradEdit_SetNumLimit(GetDlgItem(IDC_COLOR), MAX_GRADCOLORS);
	GradEdit_SetColors(GetDlgItem(IDC_COLOR), m_gm.numcolors, m_gm.grad);

	CheckDlgButton(IDC_WHOLEVIEW, m_gm.whole);

	SetWholePreview(m_gm.whole);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CGradMapDlg::OnColorChanged()
{
	m_gm.numcolors = GradEdit_GetColors(GetDlgItem(IDC_COLOR), m_gm.grad);
	UpdatePreview();
}

HDIB CGradMapDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	gradation_map(image, NULL, m_gm.grad, m_gm.numcolors);
	return image.createdib();
}

void CGradMapDlg::OnLoadPreset()
{
	GRADEDITINFO info;
	info.numcolors = GradEdit_GetColors(GetDlgItem(IDC_COLOR), info.grad);
	if (m_pfe->gradeditor(m_hWnd, &info)) {
		m_gm.numcolors = info.numcolors;
		memcpy(m_gm.grad, info.grad, sizeof(GRADCOLORS) * info.numcolors);
		GradEdit_SetColors(GetDlgItem(IDC_COLOR), m_gm.numcolors, m_gm.grad);
		UpdatePreview();
	}
}

void CGradMapDlg::OnGradReset()
{
	m_gm.numcolors = 2;
	m_gm.grad[0].pos = 0;
	m_gm.grad[0].cr = RGB(0, 0, 0);
	m_gm.grad[1].pos = 1000;
	m_gm.grad[1].cr = RGB(255, 255, 255);
	GradEdit_SetColors(GetDlgItem(IDC_COLOR), m_gm.numcolors, m_gm.grad);
	UpdatePreview();
}

void CGradMapDlg::OnCheckWhole()
{
	m_gm.whole = IsDlgButtonChecked(IDC_WHOLEVIEW);
	SetWholePreview(m_gm.whole);
	UpdatePreview();
}

////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("color\\gradmap");
static TCHAR keyLast[] = _T("last");

HANDLE gradationmap_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	CGradMapDlg dlg;
	dlg.m_pfe = pfe;
	regkey reg(pfe->hkey, secRoot);

	GRADMAP gm;
	if (reg.getbinary(keyLast, &gm, sizeof(gm)) == sizeof(gm))
		memcpy(&dlg.m_gm, &gm, sizeof(gm));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_gm, sizeof(dlg.m_gm));

		CWaitCursor wc;
		image.attachdib(hdib);
		gradation_map(image, prc, dlg.m_gm.grad, dlg.m_gm.numcolors);
		return image.createdib();
	}
	return NULL;
}

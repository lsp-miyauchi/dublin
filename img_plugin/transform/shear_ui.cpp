// shear.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();


#define WITHIN(a,b,c)			((((a)<=(b))&&((b)<=(c)))?1:0)

static RGBA __fastcall bilinear(double x, double y, RGBA* v)
{
	x = fmod(x, 1.0);
	y = fmod(y, 1.0);
	if (x < 0) x += 1.0;
	if (y < 0) y += 1.0;

	RGBA pix;
	pix.red = (BYTE)((1.0-y)*((1.0-x)*v[0].red+x*v[1].red) + y*((1.0-x)*v[2].red+x*v[3].red));
	pix.green = (BYTE)((1.0-y)*((1.0-x)*v[0].green+x*v[1].green) + y*((1.0-x)*v[2].green+x*v[3].green));
	pix.blue = (BYTE)((1.0-y)*((1.0-x)*v[0].blue+x*v[1].blue) + y*((1.0-x)*v[2].blue+x*v[3].blue));
	return pix;
}

static BOOL shear_horz(mif& src, int xshear)
{
	int length = src.m_width * xshear / 100;
	int delta_width = src.m_width - abs(length);
	double xstep = (double)delta_width / (double)src.m_width;
	double ystep = (double)length / (double)src.m_height;

	mif dst(src.m_width, src.m_height, 24);

	double left_space;
	if (length < 0)
		left_space = src.m_width - delta_width;
	else
		left_space = 0;

	int x, y;
	RGBA* pd = dst.m_pixels;
	for (y = 0; y < src.m_height; y++) {
		for (x = 0; x < src.m_width; x++) {
			if (x < (int)left_space || x >= (int)(left_space + delta_width)) {
				pd++;
				continue;
			}

			double dx = (x - left_space) / xstep;
			double dy = y;
			BOOL x1_in = WITHIN(0, dx, src.m_width - 1);
			BOOL y1_in = WITHIN(0, dy, src.m_height - 1);
			BOOL x2_in = WITHIN(0, dx + 1, src.m_width - 1);
			BOOL y2_in = WITHIN(0, dy + 1, src.m_height - 1);

			RGBA values[4];
			memset(values, 0, sizeof(values));
			if (x1_in && y1_in) values[0] = *src.pos((int)dx, (int)dy);
			if (x2_in && y1_in) values[1] = *src.pos((int)dx + 1, (int)dy);
			if (x1_in && y2_in) values[2] = *src.pos((int)dx, (int)dy + 1);
			if (x2_in && y2_in) values[3] = *src.pos((int)dx + 1, (int)dy + 1);

			*pd = bilinear(dx, 0, values);
			pd++;
		}
		left_space += ystep;
progress_continue();
	}
	src.movefrom(&dst);
	return TRUE;
}

static BOOL shear_vert(mif& src, int yshear)
{
	int length = src.m_height * yshear / 100;
	int delta_height = src.m_height - abs(length);
	double xstep = (double)length / (double)src.m_width;
	double ystep = (double)delta_height / (double)src.m_height;

	mif dst(src.m_width, src.m_height, 24);

	double top_space;
	if (length < 0)
		top_space = src.m_height - delta_height;
	else
		top_space = 0;

	int x, y;
	for (x = 0; x < src.m_width; x++) {
		for (y = 0; y < src.m_height; y++) {
			if (y < (int)top_space || y >= (int)(top_space + delta_height))
				continue;

			double dx = x;
			double dy = (y - top_space) / ystep;
			BOOL x1_in = WITHIN(0, dx, src.m_width - 1);
			BOOL y1_in = WITHIN(0, dy, src.m_height - 1);
			BOOL x2_in = WITHIN(0, dx + 1, src.m_width - 1);
			BOOL y2_in = WITHIN(0, dy + 1, src.m_height - 1);

			RGBA values[4];
			memset(values, 0, sizeof(values));
			if (x1_in && y1_in) values[0] = *src.pos((int)dx, (int)dy);
			if (x2_in && y1_in) values[1] = *src.pos((int)dx + 1, (int)dy);
			if (x1_in && y2_in) values[2] = *src.pos((int)dx, (int)dy + 1);
			if (x2_in && y2_in) values[3] = *src.pos((int)dx + 1, (int)dy + 1);

			*(dst.pos(x, y)) = bilinear(0, dy, values);
		}
		top_space += xstep;
progress_continue();
	}
	src.movefrom(&dst);
	return TRUE;
}

HDIB shear(HDIB hdib, LPRECT prc, int xshear, int yshear)
{
	CRect rc;
	mif src;
	if (!src.attachdib(hdib) || !src.validaterect(prc, &rc))
		return NULL;

int maxprogress = (xshear ? rc.Height() : 0) + (yshear ? rc.Width() : 0);
progress_init(maxprogress);

	mif dst(rc.Width(), rc.Height(), 24);
	dst.blt(0, 0, rc.Width(), rc.Height(), &src, rc.left, rc.top);
	if (xshear) shear_horz(dst, xshear);
	if (yshear) shear_vert(dst, yshear);
	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
progress_term();
	return src.createdib();
}


/////////////////////////////////////////////////////////////////////////////
// CShearDlg ダイアログ

class CShearDlg : public CDialogImpl<CShearDlg>, public CFilterDialogImpl<CShearDlg>
{
public:
	CShearDlg();

	enum { IDD = IDD_SHEAR };

	CTrackCtrl m_xslider;
	CTrackCtrl m_yslider;
	HDIB m_hdib;
	int m_xshear;
	int m_yshear;

BEGIN_MSG_MAP(CShearDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CShearDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CShearDlg::CShearDlg()
{
	m_xshear = m_yshear = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CShearDlg メッセージ ハンドラ

BOOL CShearDlg::InitDialog(LPARAM lParam) 
{
	m_xslider = GetDlgItem(IDC_XSLIDER);
	m_yslider = GetDlgItem(IDC_YSLIDER);

	m_xslider.SetRange(-90, 90);
	m_xslider.SetPageSize(100);
	m_xslider.SetTicFreq(45);
	m_xslider.SetPos(m_xshear);

	m_yslider.SetRange(-90, 90);
	m_yslider.SetPageSize(100);
	m_yslider.SetTicFreq(45);
	m_yslider.SetPos(m_yshear);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CShearDlg::HScroll(int val, HWND hBar) 
{
	switch (::GetDlgCtrlID(hBar)) {
		case IDC_XSLIDER: m_xshear = val; break;
		case IDC_YSLIDER: m_yshear = val; break;
		default: return;
	}
	UpdatePreview();
}

HDIB CShearDlg::GetPreview(HDIB hdib)
{
	return shear(hdib, NULL, m_xshear, m_yshear);
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("transform\\shear");
static TCHAR keyLast[] = _T("last");

HANDLE shear_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CShearDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, secRoot);
	int a[2];
	int len = reg.getbinary(keyLast, a, sizeof(a));
	if (len == sizeof(a)) {
		dlg.m_xshear = a[0];
		dlg.m_yshear = a[1];
	}

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		a[0] = dlg.m_xshear;
		a[1] = dlg.m_yshear;
		reg.setbinary(keyLast, a, sizeof(a));

pfnProgress = pfe->progress;
		HDIB h = shear(hdib, prc, dlg.m_xshear, dlg.m_yshear);
pfnProgress = NULL;
		return h;
	}
	return NULL;
}

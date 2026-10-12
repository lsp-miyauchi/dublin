//////////////////////////////////////////////////////////////////////
// perspect.cpp

#include "stdafx.h"
#include "ffactory_sup.h"
#include "..\filterdialog.h"

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();

static RGBA pixdef;

static RGBA* __fastcall getpix(mif& src, int x, int y, COLORREF clrDefault)
{
	if (x < 0 || y < 0 || x >= src.m_width || y >= src.m_height) {
		pixdef.red   = GetRValue(clrDefault);
		pixdef.green = GetGValue(clrDefault);
		pixdef.blue  = GetBValue(clrDefault);
		pixdef.alpha = 0;
		return &pixdef;
	}
	return src.pos(x, y);
}

static void matrix(double l[4][4], double m[4][4], double n[4][4])
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			double p = 0;
			for (int k = 0; k < 4; k++)
				p = p + l[i][k] * m[k][j];
			n[i][j] = p;
		}
	}
}

static void param_pers(double* k, double a, double b, double x0, double y0, double z0, double z, double x, double y, double t, double s)
{
	double l[4][4], m[4][4], n[4][4], k1, k2, k3, k4, k5, k6, k7, k8, k9;
	int xs = X_center;
	int ys = Y_center;

	double u = DEG2RAD(x);
	double v = DEG2RAD(y);
	double w = DEG2RAD(z);

	l[0][0] = 1.0/xs;	l[0][1] = 0; 		l[0][2] = 0;		l[0][3] = 0;
	l[1][0] = 0;		l[1][1] = -1.0/xs; 	l[1][2] = 0;		l[1][3] = 0;
	l[2][0] = 0;		l[2][1] = 0; 		l[2][2] = 1;		l[2][3] = 0;
	l[3][0] = 0;		l[3][1] = 0; 		l[3][2] = 0;		l[3][3] = 1;
	m[0][0] = a;		m[0][1] = 0; 		m[0][2] = 0;		m[0][3] = 0;
	m[1][0] = 0;		m[1][1] = b;	 	m[1][2] = 0;		m[1][3] = 0;
	m[2][0] = 0;		m[2][1] = 0; 		m[2][2] = 1;		m[2][3] = 0;
	m[3][0] = 0;		m[3][1] = 0; 		m[3][2] = 0;		m[3][3] = 1;
	matrix(l, m, n);	// 正規化マトリクス x 拡大縮小マトリクス

	l[0][0] = 1;		l[0][1] = 0; 		l[0][2] = 0;		l[0][3] = 0;
	l[1][0] = 0;		l[1][1] = 1;	 	l[1][2] = 0;		l[1][3] = 0;
	l[2][0] = 0;		l[2][1] = 0; 		l[2][2] = 1;		l[2][3] = 0;
	l[3][0] = x0;		l[3][1] = y0; 		l[3][2] = z0;		l[3][3] = 1;
	matrix(n, l, m);	// x 移動マトリクス

	n[0][0] = cos(w);	n[0][1] = sin(w);	n[0][2] = 0;		n[0][3] = 0;
	n[1][0] = -sin(w);	n[1][1] = cos(w); 	n[1][2] = 0;		n[1][3] = 0;
	n[2][0] = 0;		n[2][1] = 0; 		n[2][2] = 1;		n[2][3] = 0;
	n[3][0] = 0;		n[3][1] = 0; 		n[3][2] = 0;		n[3][3] = 1;
	matrix(m, n, l);	// x Z軸の回転マトリクス

	m[0][0] = 1;		m[0][1] = 0; 		m[0][2] = 0;		m[0][3] = 0;
	m[1][0] = 0;		m[1][1] = cos(u); 	m[1][2] = sin(u);	m[1][3] = 0;
	m[2][0] = 0;		m[2][1] = -sin(u); 	m[2][2] = cos(u);	m[2][3] = 0;
	m[3][0] = 0;		m[3][1] = 0; 		m[3][2] = 0;		m[3][3] = 1;
	matrix(l, m, n);	// x x軸の回転マトリクス

	l[0][0] = cos(v);	l[0][1] = 0; 		l[0][2] = sin(v);	l[0][3] = 0;
	l[1][0] = 0;		l[1][1] = 1;	 	l[1][2] = 0;		l[1][3] = 0;
	l[2][0] = -sin(v);	l[2][1] = 0; 		l[2][2] = cos(v);	l[2][3] = 0;
	l[3][0] = 0;		l[3][1] = 0; 		l[3][2] = 0;		l[3][3] = 1;
	matrix(n, l, m);	// x y軸の回転マトリクス

	n[0][0] = 1;		n[0][1] = 0;		n[0][2] = 0;		n[0][3] = 0;
	n[1][0] = 0;		n[1][1] = 1;	 	n[1][2] = 0;		n[1][3] = 0;
	n[2][0] = 0;		n[2][1] = 0; 		n[2][2] = -1;		n[2][3] = 0;
	n[3][0] = 0;		n[3][1] = 0; 		n[3][2] = t;		n[3][3] = 1;
	matrix(m, n, l);	// x 視点座標変換マトリクス

	m[0][0] = 1;		m[0][1] = 0; 		m[0][2] = 0;		m[0][3] = 0;
	m[1][0] = 0;		m[1][1] = 1;	 	m[1][2] = 0;		m[1][3] = 0;
	m[2][0] = 0;		m[2][1] = 0; 		m[2][2] = 1/s;		m[2][3] = 1/s;
	m[3][0] = 0;		m[3][1] = 0; 		m[3][2] = -1;		m[3][3] = 0;
	matrix(l, m, n);	// x 透視変換マトリクス

	l[0][0] = xs;		l[0][1] = 0; 		l[0][2] = 0;		l[0][3] = 0;
	l[1][0] = 0;		l[1][1] = -xs; 		l[1][2] = 0;		l[1][3] = 0;
	l[2][0] = 0;		l[2][1] = 0; 		l[2][2] = 1;		l[2][3] = 0;
	l[3][0] = 0;		l[3][1] = 0; 		l[3][2] = 0;		l[3][3] = 1;
	matrix(n, l, m);	// x 正規化マトリクス

	k1 = m[0][3];		k2 = m[1][3];		k3 = m[3][3];
	k4 = m[0][0];		k5 = m[1][0];		k6 = m[3][0];
	k7 = m[0][1];		k8 = m[1][1];		k9 = m[3][1];

	k[0] = k7*k2-k8*k1;	k[1] = k5*k1-k4*k2;	k[2] = k4*k8-k7*k5;
	k[3] = k8*k3-k9*k2;	k[6] = k9*k1-k7*k3;	k[4] = k6*k2-k5*k3;
	k[7] = k4*k3-k6*k1;	k[5] = k5*k9-k8*k6;	k[8] = k7*k6-k4*k9;
}

BOOL perspect(mif& src, LPRECT prc, double ax, double ay, double px, double py, double pz, double rz, double rx, double ry, double v, double s, COLORREF clrBkgnd)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	X_center = rc.Width() / 2;
	Y_center = rc.Height() / 2;

	double k[9];
	param_pers(k, ax, ay, px, py, pz, rz, rx, ry, v, s);

	mif dst(rc.Width(), rc.Height(), 24);
	dst.blt(0, 0, rc.Width(), rc.Height(), &src, rc.left, rc.top);

progress_init(rc.Height() >> 1);
	for (int i = 0; i < rc.Height(); i++) {
		RGBA* ps = src.pos(rc.left, i + rc.top);
		for (int j = 0; j < rc.Width(); j++) {
			double w = k[0]*(j-X_center) + k[1]*(i-Y_center) + k[2];
			double x = k[3]*(j-X_center) + k[4]*(i-Y_center) + k[5];
			double y = k[6]*(j-X_center) + k[7]*(i-Y_center) + k[8];
			x = x/w;
			y = y/w;
			double m = (y > 0) ? y : y - 1;
			double n = (x > 0) ? x : x - 1;
			double q = y - m;
			double p = x - n;

			RGBA* p1 = getpix(dst, X_center+(int)n,     Y_center+(int)m,     clrBkgnd);
			RGBA* p2 = getpix(dst, X_center+(int)(n+1), Y_center+(int)m,     clrBkgnd);
			RGBA* p3 = getpix(dst, X_center+(int)n,     Y_center+(int)(m+1), clrBkgnd);
			RGBA* p4 = getpix(dst, X_center+(int)(n+1), Y_center+(int)(m+1), clrBkgnd);

			double d;
			d = (1.0-q)*((1.0-p)*(double)p1->red + p*(double)p2->red) +
						q*((1.0-p)*(double)p2->red + p * (double)p4->red);
			ps->red = CLAMP((int)d);

			d = (1.0-q)*((1.0-p)*(double)p1->green + p*(double)p2->green) +
						q*((1.0-p)*(double)p2->green + p * (double)p4->green);
			ps->green = CLAMP((int)d);

			d = (1.0-q)*((1.0-p)*(double)p1->blue + p*(double)p2->blue) +
						q*((1.0-p)*(double)p2->blue + p * (double)p4->blue);
			ps->blue = CLAMP((int)d);

			ps++;
		}
if (i & 1) progress_continue();
	}
progress_term();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//

typedef struct tagPERSPECT {
	int zoom_x;					// 横拡大率
	int zoom_y;					// 縦拡大率
	int move_z;					// Z移動量
	int move_x;					// X移動量
	int move_y;					// Y移動量
	int rot_x;					// X回転角
	int rot_y;					// Y回転角
	int rot_z;					// Z回転角
	int z_view;					// Z視点
	int z_screen;				// Zスクリーン
	COLORREF clr;				// 背景色
} PERSPECT;

static void default_param(PERSPECT* pp)
{
	memset(pp, 0, sizeof(PERSPECT));
	pp->zoom_x = 10;
	pp->zoom_y = 10;
	pp->z_view = 100;
	pp->z_screen = 50;
}

class CPerspectDlg : public CDialogImpl<CPerspectDlg>, public CFilterDialogImpl<CPerspectDlg>
{
public:
	CPerspectDlg() { default_param(&m_ps); }

	enum { IDD = IDD_PERSPECT };
	PERSPECT m_ps;
	HDIB m_hdib;

BEGIN_MSG_MAP(CPerspectDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CPerspectDlg>)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_CBN_SELCHANGE(IDC_COLOR, OnColorChange)
END_MSG_MAP()

	void OnReset();
	void OnColorChange();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

BOOL CPerspectDlg::InitDialog(LPARAM lParam)
{
	// 拡大縮小
	CTrackCtrl track = GetDlgItem(IDC_ASLIDER);
	track.SetRange(1, 100);
	track.SetTicFreq(10);
	track.SetPos(m_ps.zoom_x);
	track.SetBuddyBase(1);

	track = GetDlgItem(IDC_BSLIDER);
	track.SetRange(1, 100);
	track.SetTicFreq(10);
	track.SetPos(m_ps.zoom_y);
	track.SetBuddyBase(1);

	// 移動
	track = GetDlgItem(IDC_CSLIDER);
	track.SetRange(-100, 100);
	track.SetTicFreq(20);
	track.SetPos(m_ps.move_x);
	track.SetBuddyBase(2);

	track = GetDlgItem(IDC_DSLIDER);
	track.SetRange(-100, 100);
	track.SetTicFreq(20);
	track.SetPos(m_ps.move_y);
	track.SetBuddyBase(2);

	track = GetDlgItem(IDC_ESLIDER);
	track.SetRange(-100, 100);
	track.SetTicFreq(20);
	track.SetPos(m_ps.move_z);
	track.SetBuddyBase(2);

	// 回転角
	track = GetDlgItem(IDC_FSLIDER);
	track.SetRange(-180, 180);
	track.SetTicFreq(30);
	track.SetPos(m_ps.rot_x);

	track = GetDlgItem(IDC_GSLIDER);
	track.SetRange(-180, 180);
	track.SetTicFreq(30);
	track.SetPos(m_ps.rot_y);

	track = GetDlgItem(IDC_HSLIDER);
	track.SetRange(-180, 180);
	track.SetTicFreq(30);
	track.SetPos(m_ps.rot_z);

	// 位置
	track = GetDlgItem(IDC_ISLIDER);
	track.SetRange(0, 200);
	track.SetTicFreq(20);
	track.SetPos(m_ps.z_view);
	track.SetBuddyBase(1);

	track = GetDlgItem(IDC_JSLIDER);
	track.SetRange(0, 200);
	track.SetTicFreq(20);
	track.SetPos(m_ps.z_screen);
	track.SetBuddyBase(1);

	// 背景色
	ColorPicker_SetBkColor(GetDlgItem(IDC_COLOR), m_ps.clr);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CPerspectDlg::OnReset()
{
	default_param(&m_ps);

	// 拡大縮小
	CTrackCtrl track;
	track = GetDlgItem(IDC_ASLIDER);
	track.SetPos(m_ps.zoom_x, TRUE);

	track = GetDlgItem(IDC_BSLIDER);
	track.SetPos(m_ps.zoom_y, TRUE);

	// 移動
	track = GetDlgItem(IDC_CSLIDER);
	track.SetPos(m_ps.move_x, TRUE);

	track = GetDlgItem(IDC_DSLIDER);
	track.SetPos(m_ps.move_y, TRUE);

	track = GetDlgItem(IDC_ESLIDER);
	track.SetPos(m_ps.move_z, TRUE);

	// 回転角
	track = GetDlgItem(IDC_FSLIDER);
	track.SetPos(m_ps.rot_x, TRUE);

	track = GetDlgItem(IDC_GSLIDER);
	track.SetPos(m_ps.rot_y, TRUE);

	track = GetDlgItem(IDC_HSLIDER);
	track.SetPos(m_ps.rot_z, TRUE);

	// 位置
	track = GetDlgItem(IDC_ISLIDER);
	track.SetPos(m_ps.z_view, TRUE);

	track = GetDlgItem(IDC_JSLIDER);
	track.SetPos(m_ps.z_screen, TRUE);

	// 背景色
	ColorPicker_SetBkColor(GetDlgItem(IDC_COLOR), m_ps.clr);

	UpdatePreview();
}

void CPerspectDlg::OnColorChange()
{
	m_ps.clr = ColorPicker_GetBkColor(GetDlgItem(IDC_COLOR));
	UpdatePreview();
}

void CPerspectDlg::HScroll(int pos, HWND hbar)
{
	switch (::GetDlgCtrlID(hbar)) {
		case IDC_ASLIDER:	m_ps.zoom_x = pos;			break;
		case IDC_BSLIDER:	m_ps.zoom_y = pos;			break;
		case IDC_CSLIDER:	m_ps.move_x = pos;			break;
		case IDC_DSLIDER:	m_ps.move_y = pos;			break;
		case IDC_ESLIDER:	m_ps.move_z = pos;			break;
		case IDC_FSLIDER:	m_ps.rot_x = pos;			break;
		case IDC_GSLIDER:	m_ps.rot_y = pos;			break;
		case IDC_HSLIDER:	m_ps.rot_z = pos;			break;
		case IDC_ISLIDER:	m_ps.z_view = pos;			break;
		case IDC_JSLIDER:	m_ps.z_screen = pos;		break;
		default:	return;
	}
	UpdatePreview();
}

HDIB CPerspectDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	perspect(image, NULL,
			(double)m_ps.zoom_x/10.0, (double)m_ps.zoom_y/10.0,
			(double)m_ps.move_x/100.0, (double)m_ps.move_y/100.0, (double)m_ps.move_z/100.0,
			(double)m_ps.rot_z, (double)m_ps.rot_x, (double)m_ps.rot_y,
			(double)m_ps.z_view/10.0, (double)m_ps.z_screen/10.0, m_ps.clr);
	int xcen = image.m_width / 2;
	int ycen = image.m_height / 2;
	RGBA* ps = image.m_pixels;
	for (int y = 0; y < image.m_height; y++) {
		for (int x = 0; x < image.m_width; x++) {
			if ((x+y)&1 && (x==xcen || y==ycen)) {
				ps->red ^= 255;
				ps->green ^= 255;
				ps->blue ^= 255;
			}
			ps++;
		}
	}
	return image.createdib();
}

/////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("transform\\perspect");
static TCHAR keyLast[] = _T("last");

HANDLE perspect_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CPerspectDlg dlg;
	PERSPECT s;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &s, sizeof(s)) == sizeof(s))
		memcpy(&dlg.m_ps, &s, sizeof(s));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_ps, sizeof(dlg.m_ps));

		CWaitCursor wc;
pfnProgress = pfe->progress;
		image.attachdib(hdib);
		perspect(image, prc,
				(double)dlg.m_ps.zoom_x/10.0, (double)dlg.m_ps.zoom_y/10.0,
				(double)dlg.m_ps.move_x/100.0, (double)dlg.m_ps.move_y/100.0, (double)dlg.m_ps.move_z/100.0,
				(double)dlg.m_ps.rot_z, (double)dlg.m_ps.rot_x, (double)dlg.m_ps.rot_y,
				(double)dlg.m_ps.z_view/10.0, (double)dlg.m_ps.z_screen/10.0, dlg.m_ps.clr);
pfnProgress = NULL;
		return image.createdib();
	}
	return NULL;
}

// mosaic_polygon.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"
#include "ffactory_sup.h"

extern PROGRESSCALLBACK pfnProgress;

void progress_init(int maxprogress);
void progress_continue();
void progress_term();


typedef struct tagMOSAICPOLYGON {
	int numpoly;
	int tilesize;
} MOSAICPOLYGON;

/**************************************************************** 
* 
* numpoly 1 - 30  number of polygons per smaller part
*                 (higher value..slower)!! 
* repeat  1 - 100 repetion factor 
* 
****************************************************************/

BOOL mosaic_polygon(mif& src, LPRECT prc, int numpoly, int tilesize)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;

	int width = rc.Width();
	int height = rc.Height();

	if (tilesize * 2 > width)
		tilesize = width / 2;
	if (tilesize * 2 > height)
		tilesize = height / 2;

	srand(0);

	int* base_mem = (int*)malloc(numpoly * 2 * sizeof(int));
	int* xarray = base_mem;
	int* yarray = base_mem + numpoly;

	int xp  = tilesize * 2;
	int xp2 = tilesize;
	int yp  = tilesize * 2;
	int yp2 = tilesize;
	int i;
	for (i = 0; i < numpoly; i++) {
		xarray[i] = rand() % xp;
		yarray[i] = rand() % yp;
	}

progress_init(height);
	mif dst(width, height, 24);
	for (int y = 0; y < height; y++) {
		RGBA* pd = dst.pos(0, y);
		for (int x = 0; x < width; x++) {
			// main pass - always get the nearest point and use its color 
			int xx, yy;
			int bestidx = -1;
			int bestnum = xp * xp + yp * yp;
			for (i = 0; i < numpoly; i++) {
				int xc, xx, yc, yy;
				xc = yc = 0;
				xx = (x + xp - xarray[i]) % xp;
				if (xx > xp2) xx -= xp;
				yy = (y + yp - yarray[i]) % yp;
				if (yy > yp2) yy -= yp;
				int dist = xx * xx + yy * yy;

				if (dist < bestnum) {
					bestidx = i;
					bestnum = dist;
				}
			}

			if (bestidx < 0)
				pd->red = pd->green = pd->blue = 0;
			else {
				xx = xarray[bestidx] + x - (x % xp);
				if (x - xx > xp2)		xx += xp;
				else if(xx - x > xp2)	xx -= xp;
				yy = yarray[bestidx] + y - y % yp;
				if (y - yy > yp2)		yy += yp;
				else if (yy - y > yp2)	yy -= yp;

				xx += rc.left;
				yy += rc.top;
				xx = max(0, min(xx, src.m_width - 1));
				yy = max(0, min(yy, src.m_height - 1));

				RGBA* ps = src.pos(xx, yy);
				*pd = *ps;
			}
			pd++;
		}
progress_continue();
	}
	free(base_mem);
	dst.blur_gauss_rle(NULL, 0.5);
	src.blt(rc.left, rc.top, width, height, &dst, 0, 0);
progress_term();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMosaicPolygonDlg ダイアログ

class CMosaicPolygonDlg : public CDialogImpl<CMosaicPolygonDlg>, public CFilterDialogImpl<CMosaicPolygonDlg>
{
public:
	CMosaicPolygonDlg();

	enum { IDD = IDD_MOSAIC_POLYGON };
	MOSAICPOLYGON m_mp;
	HDIB m_hdib;

BEGIN_MSG_MAP(CMosaicPolygonDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CMosaicPolygonDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CMosaicPolygonDlg::CMosaicPolygonDlg()
{
	m_mp.numpoly = 16;
	m_mp.tilesize = 16;
}

BOOL CMosaicPolygonDlg::InitDialog(LPARAM lParam)
{
	CTrackCtrl track = GetDlgItem(IDC_PSLIDER);
	track.SetRange(2, 40);
	track.SetTicFreq(10);
	track.SetPos(m_mp.numpoly);

	track = GetDlgItem(IDC_TSLIDER);
	track.SetRange(4, 80);
	track.SetTicFreq(20);
	track.SetPos(m_mp.tilesize);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CMosaicPolygonDlg::HScroll(int val, HWND hScrollBar)
{
	switch (::GetDlgCtrlID(hScrollBar)) {
		case IDC_PSLIDER: m_mp.numpoly = val; break;
		case IDC_TSLIDER: m_mp.tilesize = val; break;
		default: return;
	}
	UpdatePreview();
}

HDIB CMosaicPolygonDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	mosaic_polygon(image, NULL, m_mp.numpoly, m_mp.tilesize);
	return image.createdib();
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("mosaic\\mosaic_polygon");
static TCHAR keyLast[] = _T("last");

HANDLE mosaic_polygon_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CMosaicPolygonDlg dlg;

	MOSAICPOLYGON m;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &m, sizeof(m)) == sizeof(m))
		memcpy(&dlg.m_mp, &m, sizeof(m));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_mp, sizeof(dlg.m_mp));

		CWaitCursor wc;
pfnProgress = pfe->progress;
		BOOL bOK = (image.attachdib(hdib) && mosaic_polygon(image, prc, dlg.m_mp.numpoly, dlg.m_mp.tilesize));
pfnProgress = NULL;
		if (bOK)
			return image.createdib();
	}
	return NULL;
}

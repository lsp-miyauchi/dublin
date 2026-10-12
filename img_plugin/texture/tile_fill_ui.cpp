// tiling.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////
//

static void __fastcall do_tile(mif* src, int xblock, int yblock)
{
	mif dst;
	dst.create(src->m_width * xblock, src->m_height * yblock, src->m_bits);
	if (src->m_bits < 24)
		memcpy(dst.m_map, src->m_map, sizeof(RGBA) * src->m_colors);
	for (int y = 0; y < yblock; y++)
		for (int x = 0; x < xblock; x++)
			dst.blt(x * src->m_width, y * src->m_height,
					src->m_width, src->m_height, src, 0, 0);
	src->movefrom(&dst);
}

static HDIB WINAPI tilefill(HDIB hsrc, LPRECT prc, int divide)
{
	CRect rc;
	mif src;
	if (divide < 2 || !src.attachdib(hsrc) || !src.crop(prc)) return NULL;
	if (src.m_bits < 24) src.m_bits = 24;

	if (src.zoom(src.m_width / divide, src.m_height / divide, zoomMitchell, FALSE)) {
		do_tile(&src, divide, divide);
		return src.createdib();
	}
	return NULL;
}

static HDIB tiling(HDIB hsrc, LPRECT prc, int xblock, int yblock)
{
	CRect rc;
	mif src;
	if (!xblock || !yblock || !src.attachdib(hsrc) || !src.crop(prc)) return NULL;

	do_tile(&src, xblock, yblock);
	return src.createdib();
}

///////////////////////////////////////////////////////////////////////////
//

class CTileFillDlg : public CDialogImpl<CTileFillDlg>, public CFilterDialogImpl<CTileFillDlg>
{
public:
	CTileFillDlg();

	enum { IDD = IDD_TILEFILL };
	CTrackCtrl m_slider;
	HDIB m_hdib;
	int m_block;

BEGIN_MSG_MAP(CTileFillDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CTileFillDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CTileFillDlg::CTileFillDlg()
{
	m_block = 3;
	m_hdib = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CTileFillDlg メッセージ ハンドラ

BOOL CTileFillDlg::InitDialog(LPARAM lParam)
{
	m_slider = GetDlgItem(IDC_SLIDER);
	m_slider.SetRange(2, 30);
	m_slider.SetTicFreq(10);
	m_slider.SetPageSize(4);
	m_slider.SetPos(m_block);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CTileFillDlg::HScroll(int val, HWND hScrollBar)
{
	if (hScrollBar == m_slider) {
		m_block = val;
		UpdatePreview();
m_view.Invalidate();
	}
}

HDIB CTileFillDlg::GetPreview(HDIB hdib)
{
	return tilefill(hdib, NULL, m_block);
}

////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("texture\\tile_fill");
static TCHAR keyLast[] = _T("last");

HANDLE tile_fill_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	regkey reg(pfe->hkey, secRoot);

	CTileFillDlg dlg;
	dlg.m_hdib = hdib;
	dlg.m_block = reg.getvalue(keyLast, dlg.m_block);

	HDIB h = NULL;
	if (dlg.DoModal(pfe->hparent) == IDOK) {
		reg.setvalue(keyLast, dlg.m_block);
		CWaitCursor wc;
		h = tilefill(hdib, NULL, dlg.m_block);
	}
	return h;
}

////////////////////////////////////////////////////////////////////////
//

class CTilingDlg : public CDialogImpl<CTilingDlg>, public CFilterDialogImpl<CTilingDlg>
{
public:
	CTilingDlg();

	enum { IDD = IDD_TITLING };
	CTrackCtrl m_xslider;
	CTrackCtrl m_yslider;
	int m_xblock;
	int m_yblock;

BEGIN_MSG_MAP(CTilingDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CTilingDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
};

CTilingDlg::CTilingDlg()
{
	m_xblock = 3;
	m_yblock = 3;
}

BOOL CTilingDlg::InitDialog(LPARAM lParam)
{
	m_xslider = GetDlgItem(IDC_XSLIDER);
	m_yslider = GetDlgItem(IDC_YSLIDER);

	m_xslider.SetRange(1, 20);
	m_xslider.SetPageSize(4);
	m_xslider.SetTicFreq(5);
	m_xslider.SetPos(m_xblock);

	m_yslider.SetRange(1, 20);
	m_yslider.SetPageSize(4);
	m_yslider.SetTicFreq(5);
	m_yslider.SetPos(m_yblock);

	return TRUE;
}

void CTilingDlg::HScroll(int val, HWND hBar)
{
	if (hBar == m_xslider)		m_xblock = val;
	else if (hBar == m_yslider)	m_yblock = val;
}

////////////////////////////////////////////////////////////////////////
//

static TCHAR secTiling[] = _T("texture\\tiling");

HANDLE tiling_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	regkey reg(pfe->hkey, secTiling);

	CTilingDlg dlg;
	DWORD dw = reg.getvalue(keyLast, MAKELONG(dlg.m_xblock, dlg.m_yblock));
	dlg.m_xblock = LOWORD(dw);
	dlg.m_yblock = HIWORD(dw);

	HDIB h = NULL;
	if (dlg.DoModal(pfe->hparent) == IDOK) {
		reg.setvalue(keyLast, MAKELONG(dlg.m_xblock, dlg.m_yblock));
		CWaitCursor wc;
		h = tiling(hdib, prc, dlg.m_xblock, dlg.m_yblock);
	}
	return h;
}


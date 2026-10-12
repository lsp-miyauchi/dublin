//////////////////////////////////////////////////////////////////////
// font.cpp

#include "stdafx.h"
#include <math.h>
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CMergeDlg ダイアログ

class CMergeDlg : public CDialogImpl<CMergeDlg>
{
public:
	CStatic m_icon;
	int m_align;
	int m_span;
	CString m_path;

public:

	enum { IDD = IDD_MERGE };

BEGIN_MSG_MAP(CMergeDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDC_CLIPBOARD, OnClipboard)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_LEFT, OnDirection)
	ON_BN_CLICKED(IDC_RIGHT, OnDirection)
	ON_BN_CLICKED(IDC_TOP, OnDirection)
	ON_BN_CLICKED(IDC_BOTTOM, OnDirection)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();

		m_icon = GetDlgItem(IDC_TYPE);

		UINT ui[2] = { CF_DIB, CF_BITMAP };
		BOOL bEnable = (GetPriorityClipboardFormat(ui, 2) >= 0);
		GetDlgItem(IDC_CLIPBOARD).EnableWindow(bEnable);

		SetDlgItemInt(IDC_SPAN, m_span, TRUE);

		int id;
		switch (m_align) {
		case 0:  id = IDC_LEFT; break;
		case 1:  id = IDC_RIGHT; break;
		case 2:  id = IDC_TOP; break;
		case 3:
		default: id = IDC_BOTTOM; break;
		}
		CheckDlgButton(id, TRUE);
		OnDirection();

		return TRUE;
	}

	void OnOK()
	{
		if (IsDlgButtonChecked(IDC_CLIPBOARD))
			m_path = "/clipboard";
		else {
			if (GetDlgItemText(IDC_PATH, m_path) < 1 || !PathFileExists(m_path)) {
				MessageBeep(0);
				GetDlgItem(IDC_PATH).SetFocus();
				return;
			}
		}

		m_span = GetDlgItemInt(IDC_SPAN, NULL, TRUE);
		EndDialog(IDOK);
	}

	void OnCancel() { EndDialog(IDCANCEL); }

	void OnClipboard()
	{
		BOOL clip = IsDlgButtonChecked(IDC_CLIPBOARD);
		GetDlgItem(IDC_PATH).EnableWindow(!clip);
		GetDlgItem(IDC_BROWSE).EnableWindow(!clip);
	}

	void OnBrowse()
	{
		static TCHAR sszFilter[] = _T("すべてのファイル(*.*)\0*.*\0\0");
		CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, sszFilter, m_hWnd);
		if (dlg.DoModal() == IDOK) {
			m_path = dlg.m_szFileName;
			SetDlgItemText(IDC_PATH, m_path);
		}
	}

	void OnDirection()
	{
		HICON hic = NULL;
		if (IsDlgButtonChecked(IDC_LEFT)) {
			m_align = 0;
			hic = LoadIcon(RESINST, MAKEINTRESOURCE(IDI_ADDLEFT));
		}
		else if (IsDlgButtonChecked(IDC_RIGHT)) {
			m_align = 1;
			hic = LoadIcon(RESINST, MAKEINTRESOURCE(IDI_ADDRIGHT));
		}
		else if (IsDlgButtonChecked(IDC_TOP)) {
			m_align = 2;
			hic = LoadIcon(RESINST, MAKEINTRESOURCE(IDI_ADDTOP));
		}
		else if (IsDlgButtonChecked(IDC_BOTTOM)) {
			m_align = 3;
			hic = LoadIcon(RESINST, MAKEINTRESOURCE(IDI_ADDBOTTOM));
		}
		hic = m_icon.SetIcon(hic);

		if (hic)
			DestroyIcon(hic);
	}
};


//////////////////////////////////////////////////////////////////////
//

enum {
	BLENDMERGE_SOLID = 0,
	BLENDMERGE_SLANTUP,
	BLENDMERGE_SLANTDOWN
};

enum {
	BLENDMERGE_HORZ = 0,
	BLENDMERGE_VERT
};

RGBA blend_pixel(RGBA* p, RGBA* q, int blend)
{
	RGBA pix;
	pix.red = ((int)p->red * (255 - blend) + (int)q->red * blend) / 255;
	pix.green = ((int)p->green * (255 - blend) + (int)q->green * blend) / 255;
	pix.blue = ((int)p->blue * (255 - blend) + (int)q->blue * blend) / 255;
	return pix;
}

void calc_alpha(mif& image, int pos, int len, int bounds)
{
	// アルファチャネルの設定
	register int x, y;
	register RGBA* p = image.m_pixels;
	int alpha;
	if (bounds == BLENDMERGE_VERT) {
		for (y = 0; y < image.m_height; y++) {
			if (y < pos)
				alpha = 0xff;
			else if (y > pos + len)
				alpha = 0;
			else
				alpha = 255 - ((y - pos) * 255 / len);
			for (x = 0; x < image.m_width; x++) {
				p->alpha = (BYTE)alpha;
				p++;
			}
		}
	}
	else {
		for (y = 0; y < image.m_height; y++) {
			for (x = 0; x < image.m_width; x++) {
				if (x < pos)
					alpha = 0xff;
				else if (x > pos + len)
					alpha = 0;
				else
					alpha = 255 - ((x - pos) * 255 / len);
				p->alpha = (BYTE)alpha;
				p++;
			}
		}
	}
}

void blendmerge_vert(mif& src, mif& dst, int area)
{
	int src_height = src.m_height;
	mif mrg;
	mrg.create(src.m_width, src.m_height + dst.m_height - area, 24);
	mrg.blt(0, 0, src.m_width, src.m_height, &src, 0, 0);
	src.movefrom(&mrg);

	register int x, y;
	register RGBA *ps, *pd;

	// アルファチャネルの設定
	calc_alpha(src, src_height - area, area, BLENDMERGE_VERT);

	// dstのセット
	for (y = 0; y < dst.m_height; y++) {
		ps = src.pos(0, y + src_height - area);
		pd = dst.pos(0, y);
		for (x = 0; x < dst.m_width; x++) {
			*ps = blend_pixel(pd, ps, ps->alpha);
			ps++;
			pd++;
		}
	}
}

void blendmerge_horz(mif& src, mif& dst, int area)
{
	int src_width = src.m_width;
	mif mrg;
	mrg.create(src.m_width + dst.m_width - area, src.m_height, 24);
	mrg.blt(0, 0, src.m_width, src.m_height, &src, 0, 0);
	src.movefrom(&mrg);

	register int x, y;
	register RGBA *ps, *pd;

	// アルファチャネルの設定
	calc_alpha(src, src_width - area, area, BLENDMERGE_HORZ);

	// dstのセット
	for (y = 0; y < dst.height(); y++) {
		ps = src.pos(src_width - area, y);
		pd = dst.pos(0, y);
		for (x = 0; x < dst.m_width; x++) {
			*ps = blend_pixel(pd, ps, ps->alpha);
			ps++;
			pd++;
		}
	}
}

HDIB BlendMerge(HDIB hsrc, HDIB hdst, int bounds, int area)
{
	if (!hsrc || !hdst || area < 1)
		return NULL;

	mif src, dst;
	if (!src.attachdib(hsrc) || !dst.attachdib(hdst))
		return NULL;
	if (src.m_bits < 24)
		src.m_bits = 24;
	if (dst.m_bits < 24)
		dst.m_bits = 24;

	int x, y;
	if (bounds == BLENDMERGE_VERT) {
		// 小さな幅に合わせる
		if (src.m_width < dst.m_width) {
			y = (src.m_width * dst.m_height) / dst.m_width;
			dst.zoom(src.m_width, y, zoomMitchell, FALSE);
		}
		else if (src.m_width > dst.m_width) {
			y = (dst.m_width * src.m_height) / src.m_width;
			src.zoom(dst.m_width, y, zoomMitchell, FALSE);
		}
		area = min(area, min(src.m_height, dst.m_height));
		blendmerge_vert(src, dst, area);
	}
	else {
		// 小さな高さに合わせる
		if (src.m_height < dst.m_height) {
			x = (src.m_height * dst.m_width) / dst.m_height;
			dst.zoom(x, src.m_height, zoomMitchell, FALSE);
		}
		else if (src.m_height > dst.m_height) {
			x = (dst.m_height * src.m_width) / src.m_height;
			src.zoom(x, dst.m_height, zoomMitchell, FALSE);
		}
		area = min(area, min(src.m_width, dst.m_width));
		blendmerge_horz(src, dst, area);
	}
	dst.destroy();
	return src.createdib();
}

HDIB Merge(HDIB hSrc, HDIB hDst, BOOL bHorz, int span, COLORREF rgb)
{
	mif src, dst;
	if (src.attachdib(hSrc) &&
		dst.attachdib(hDst) &&
		src.merge(&dst, bHorz, span, rgb))
		return src.createdib();
	return NULL;
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("montage\\merge");
static TCHAR keyParam[] = _T("param");

static HDIB copy_handle(HDIB hsrc)
{

	size_t size = GlobalSize(hsrc);
	HDIB hdst = GlobalAlloc(GHND, size);
	if (hdst) {
		void* ps = GlobalLock(hsrc);
		void* pd = GlobalLock(hdst);
		memcpy(pd, ps, size);
		GlobalUnlock(hsrc);
		GlobalUnlock(hdst);
	}
	return hdst;
}

HANDLE merge_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	regkey reg(pfe->hkey, secRoot);

	CMergeDlg dlg;
	DWORD dw = (DWORD)reg.getvalue(keyParam, 0);
	dlg.m_align = LOWORD(dw);
	dlg.m_span = (int)(short)(HIWORD(dw));
	if (dlg.DoModal(pfe->hparent) != IDOK)
		return NULL;

	dw = MAKELONG(dlg.m_align, dlg.m_span);
	reg.setvalue(keyParam, dw);

	HDIB hsrc = NULL;
	if (!dlg.m_path.Compare(_T("/clipboard"))) {
		if (OpenClipboard(pfe->hparent)) {
			HANDLE hf;
			if (hf = GetClipboardData(CF_DIB))
				hsrc = copy_handle(hf);
			else if (hf = GetClipboardData(CF_BITMAP)) {
				HPALETTE hpal = (HPALETTE)GetClipboardData(CF_PALETTE);
				hsrc = imgCreateDIBFromBitmap((HBITMAP)hf, hpal);
			}
			CloseClipboard();
		}
	}
	else
		hsrc = imgLoadImage(dlg.m_path);

	if (!hsrc) {
		MessageBeep(0);
		return NULL;
	}

	HDIB hdst, hwrk, hdibNew = NULL;
	if (dlg.m_span >= 0) {
		// 連結
		BOOL bHorz;
		hwrk = hsrc;
		if (dlg.m_align == 0) {
			hdst = hdib;
			bHorz = TRUE;
		}
		else if (dlg.m_align == 1) {
			hdst = hsrc;
			hsrc = hdib;
			bHorz = TRUE;
		}
		else if (dlg.m_align == 2) {
			hdst = hdib;
			bHorz = FALSE;
		}
		else {
			hdst = hsrc;
			hsrc = hdib;
			bHorz = FALSE;
		}

		CWaitCursor wc;
		hdibNew = Merge(hsrc, hdst, bHorz, dlg.m_span, RGB(255, 255, 255));
		GlobalFree(hwrk);
	}
	else {
		// ブレンド
		hdst = copy_handle(hdib);

		int bounds;
		if (dlg.m_align == 0)
			bounds = BLENDMERGE_HORZ;
		else if (dlg.m_align == 1) {
			hwrk = hsrc;
			hsrc = hdst;
			hdst = hwrk;
			bounds = BLENDMERGE_HORZ;
		}
		else if (dlg.m_align == 2)
			bounds = BLENDMERGE_VERT;
		else {
			hwrk = hsrc;
			hsrc = hdst;
			hdst = hwrk;
			bounds = BLENDMERGE_VERT;
		}

		hdibNew = BlendMerge(hsrc, hdst, bounds, -dlg.m_span);
		GlobalFree(hsrc);
		GlobalFree(hdst);
	}

	return hdibNew;
}

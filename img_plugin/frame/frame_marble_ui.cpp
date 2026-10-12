//////////////////////////////////////////////////////////////////////
// frame_marble.cpp

#include "stdafx.h"
#include "..\filterdialog.h"

#define CLAMP(a)			(min(255,max(0,(a))))
#define scl(a,b,c,d,e)		(((c)!=(b))?((((a)-(b))*((e)-(d)))/((c)-(b)))+(d):0)

static HDIB hdibMarble = NULL;

HDIB __fastcall load_respng(int idResource)
{
	HINSTANCE hInst = RESINST;
	HRSRC hrsrc = FindResource(hInst, _MKIR(idResource), RT_RCDATA);
	if (hrsrc) {
		DWORD sizeResource = SizeofResource(hInst, hrsrc);
		if (sizeResource) {
			HGLOBAL hglb = LoadResource(hInst, hrsrc);
			if (hglb) {
				void* pr = LockResource(hglb);
				if (pr) {
					HGLOBAL hmem = GlobalAlloc(GHND, sizeResource);
					if (hmem) {
						void* pd = GlobalLock(hmem);
						memcpy(pd, pr, sizeResource);
						GlobalUnlock(hmem);

						HDIB hdib = imgPngLoadMem(hmem);
						GlobalFree(hmem);
						return hdib;
					}
				}
			}
		}
	}
	return NULL;
}

static void make_marble_bar(HDIB htxr)
{
	// フレームバー作成
	mif bar(550, 48, 24);
	mif txr(htxr);

	RGBA* pb = bar.m_pixels;
	for (int y = 0; y < bar.m_height; y++) {
		for (int x = 0; x < bar.m_width; x++) {
			RGBA* pt = txr.pos(x % txr.m_width, y % txr.m_height);
			*pb = *pt;

			if (y < 8 || y >= 40) {
				int val = (y < 8) ? -60 : 60;
				pb->red = CLAMP((int)pb->red + val);
				pb->green = CLAMP((int)pb->green + val);
				pb->blue = CLAMP((int)pb->blue + val);
			}
			pb++;
		}
	}

	if (hdibMarble) {
		GlobalFree(hdibMarble);
		hdibMarble = NULL;
	}
	hdibMarble = bar.createdib();
}

static RGBA* __fastcall getsrc(mif& src, int x, int y)
{
	x = min(src.m_width - 1, max(0, x));
	y = min(src.m_height - 1, max(0, y));
	return src.pos(x, y);
}

static BOOL __fastcall marble_frame(mif& src, LPRECT prc, int size, int r_adj, int g_adj, int b_adj, int shadow)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	int X = rc.Width();
	int Y = rc.Height();

	int half = min(X, Y) / 2;
	size = (size * half) / 255;
	if (size < 2)
		return TRUE;

	mif barY(hdibMarble);
	barY.zoom(Y, size, zoomMitchell, FALSE);
	mif barX(hdibMarble);
	barX.zoom(X, size, zoomMitchell, FALSE);

	for (int y = 0; y < Y; y++) {
		RGBA* p = src.pos(rc.left, y + rc.top);
		for (int x = 0; x < X; x++) {
			int area = ((x < half) && (y >= x) && (y < Y-x)) ? 1 :
							((y < half) && (x > y) && (x < X - y)) ? 2 :
								((X - x <= half) && (Y - y >= X - x) && (y > x - X)) ? 3 :
									((Y - y <= half) && (X - x > Y - y) && (x > y - Y)) ? 4 : 0;

			int b11, b12;
			if (area == 1) {
				b11 = y;
				b12 = size - x;
			}
			else {
				if (area == 2) {
					b11 = X - x - 1;
					b12 = size - y;
				}
				else {
					if (area == 3) {
						b11 = Y - y - 1;
						b12 = X - x - 1;
					}
					else {
						if (area == 4) {
							b11 = x;
							b12 = Y - y - 1;
						}
					}
				}
			}

			int sbd = min(min(x,X-x-1),min(y,Y-y-1));
			if (area && sbd < size) {
				RGBA* q;
				if (area == 1 || area == 3)
					q = getsrc(barY, b11, b12);
				else
					q = getsrc(barX, b11, b12);

				p->red = CLAMP((int)q->red + r_adj);
				p->green = CLAMP((int)q->green  + g_adj);
				p->blue = CLAMP((int)q->blue  + b_adj);
			}
			else {
				int shadow_delta = size / 2;
				if (min(x,y) <= size + shadow_delta) {
					int shade = scl(min(x,y) - size, 0, shadow_delta, shadow, 255);
					p->red   = p->red   * shade / 255;
					p->green = p->green * shade / 255;
					p->blue  = p->blue  * shade / 255;
				}
			}
			p++;
		}
	}
	return TRUE;
}

HDIB __fastcall marbleframe(HDIB hdib, LPRECT prc, int size, int r_adj, int g_adj, int b_adj, int shadow)
{
	mif src;
	if (src.attachdib(hdib) && marble_frame(src, prc, size, r_adj, g_adj, b_adj, shadow))
		return src.createdib();
	return NULL;
}


/////////////////////////////////////////////////////////////////////
//

static TCHAR sszPreset[] = _T("#プリセット%02d");

typedef struct tagMARBLEFRAME {
	HDIB hdib;
	TCHAR txr[MAX_PATH];
	int size;
	int r;
	int g;
	int b;
	int shadow;
} MARBLEFRAME;

class CMarbleFrameDlg : public CDialogImpl<CMarbleFrameDlg>, public CFilterDialogImpl<CMarbleFrameDlg>
{
public:
	CMarbleFrameDlg();

	enum { IDD = IDD_MARBLEFRAME };
	MARBLEFRAME m_mf;
	CComboBox m_preset;
	bool m_initdone;
	HDIB m_hdib;

BEGIN_MSG_MAP(CMarbleFrameDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CMarbleFrameDlg>)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_CBN_SELCHANGE(IDC_PATH, OnPreset)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void OnBrowse();
	void OnPreset();
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CMarbleFrameDlg::CMarbleFrameDlg()
{
	m_initdone = false;
	m_mf.size = 16;
	m_mf.r = 0;
	m_mf.g = 0;
	m_mf.b = 0;
	m_mf.shadow = 128;
}

BOOL CMarbleFrameDlg::InitDialog(LPARAM lParam)
{
	m_preset = GetDlgItem(IDC_PATH);

	CTrackCtrl track = GetDlgItem(IDC_ASLIDER);
	track.SetRange(1, 255);
	track.SetPageSize(10);
	track.SetTicFreq(64);
	track.SetPos(m_mf.size);

	track = GetDlgItem(IDC_BSLIDER);
	track.SetRange(-128, 127);
	track.SetPageSize(20);
	track.SetTicFreq(64);
	track.SetPos(m_mf.r);

	track = GetDlgItem(IDC_CSLIDER);
	track.SetRange(-128, 127);
	track.SetPageSize(20);
	track.SetTicFreq(64);
	track.SetPos(m_mf.g);

	track = GetDlgItem(IDC_DSLIDER);
	track.SetRange(-128, 127);
	track.SetPageSize(20);
	track.SetTicFreq(64);
	track.SetPos(m_mf.b);

	track = GetDlgItem(IDC_ESLIDER);
	track.SetRange(0, 255);
	track.SetPageSize(20);
	track.SetTicFreq(64);
	track.SetPos(m_mf.shadow);

	for (int i = 0; i < 12; i++) {
		TCHAR buf[MAX_PATH];
		wsprintf(buf, sszPreset, i + 1);
		m_preset.AddString(buf);
	}

	HDIB htxr = NULL;
	if (PathFileExists(m_mf.txr)) {
		htxr = imgLoadImage(m_mf.txr);
		if (htxr)
			m_preset.SetWindowText(m_mf.txr);
	}

	if (!htxr) {
		int index = 0;
		if (m_mf.txr[0] == '#') {
			index = m_preset.FindStringExact(-1, m_mf.txr);
			if (index == CB_ERR)
				index = 0;
		}
		m_preset.SetCurSel(index);
		m_preset.GetLBText(index, m_mf.txr);
		htxr = load_respng(index + ID_TXR_01);
		m_preset.SetCurSel(index);
	}
	make_marble_bar(htxr);
	GlobalFree(htxr);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();

	m_initdone = true;
	return TRUE;
}

void CMarbleFrameDlg::OnPreset()
{
	int index = m_preset.GetCurSel();
	m_preset.GetLBText(index, m_mf.txr);

	HDIB htxr = load_respng(index + ID_TXR_01);
	make_marble_bar(htxr);
	GlobalFree(htxr);
	UpdatePreview();
}

void CMarbleFrameDlg::OnBrowse()
{
	TCHAR szFileName[MAX_PATH * 2], szFileTitle[MAX_PATH];
	szFileTitle[0] = '\0';
	if (m_preset.GetWindowText(szFileName, _COUNTOF(szFileName)) < 1)
		szFileName[0] = '\0';

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrDefExt = NULL;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _COUNTOF(szFileName);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = _COUNTOF(szFileTitle);
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;
	ofn.lpstrFilter = _T("画像ファイル\0*.bmp;*.jpg;*.jpeg;*.jpe;*.gif;*.png;*.tif;*.psd;*.pnm;*.pgm;*.txr\0\0");
	if (GetOpenFileName(&ofn)) {
		HDIB hdib = imgLoadImage(ofn.lpstrFile);
		if (hdib) {
			make_marble_bar(hdib);
			GlobalFree(hdib);

			lstrcpy(m_mf.txr, ofn.lpstrFile);
			m_preset.SetWindowText(m_mf.txr);
			UpdatePreview();
		}
	}
}

void CMarbleFrameDlg::HScroll(int val, HWND hbar)
{
	switch (::GetDlgCtrlID(hbar)) {
		case IDC_ASLIDER: m_mf.size = val; break;
		case IDC_BSLIDER: m_mf.r = val; break;
		case IDC_CSLIDER: m_mf.g = val; break;
		case IDC_DSLIDER: m_mf.b = val; break;
		case IDC_ESLIDER: m_mf.shadow = val; break;
		default: return;
	}
	UpdatePreview();
}

HDIB CMarbleFrameDlg::GetPreview(HDIB hdib)
{
	return marbleframe(hdib, NULL, m_mf.size, m_mf.r, m_mf.g, m_mf.b, m_mf.shadow);
}


//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("frame\\frame_marble");
static TCHAR keyLast[] = _T("last");

HANDLE framemarble_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CMarbleFrameDlg dlg;
	regkey reg(pfe->hkey, secRoot);
	MARBLEFRAME m;
	if (reg.getbinary(keyLast, &m, sizeof(m)) == sizeof(m))
		memcpy(&dlg.m_mf, &m, sizeof(m));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	bool bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);

	HDIB h = NULL;
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_mf, sizeof(dlg.m_mf));
		CWaitCursor wc;
		h = marbleframe(hdib, prc, dlg.m_mf.size, dlg.m_mf.r, dlg.m_mf.g, dlg.m_mf.b, dlg.m_mf.shadow);
	}
	GlobalFree(hdibMarble);
	hdibMarble = NULL;

	return h;
}


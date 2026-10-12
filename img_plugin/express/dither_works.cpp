//////////////////////////////////////////////////////////////////////
// dither_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"
#include "..\filterdialog.h"

typedef struct __tagPIXELPROC {
	void (__fastcall *pfn)(RGBA*, int, int);
	TCHAR* filter_name;
} __PIXELPROC;

static void __fastcall Dither2x2(RGBA* ps, int x, int y)
{
	int i = yuvY(ps);
	put(i,0);					// subrange dither
	put(153,100);
	put(102,101);
	put(51,102);
	put(204,103);				// 2x2 dither matrix 
	put(100+2*(y%2)+x%2,1);		// matrix index 
	put(get(0)>get(get(1))?255:0,99);
	ps->red = ps->green = ps->blue = get(99);
}

static void __fastcall Dither4x4(RGBA* ps, int x, int y)
{
	int i = yuvY(ps);
	put(i,0);					// subrange dither
	put(30,100);
	put(240,101);
	put(45,102);
	put(195,103);				// dither matrix 4x4 row 1 
	put(150,104);
	put(90,105);
	put(165,106);
	put(105,107);				// dither matrix 4x4 row 2
	put(60,108);
	put(210,109);
	put(15,110);
	put(225,111);				// dither matrix 4x4 row 3
	put(180,112);
	put(120,113);
	put(135,114);
	put(75,115);				// dither matrix 4x4 row 4
	put(100+4*(y%4)+x%4,1);		// matrix index 
	put(get(0)>get(get(1))?255:0,99);
	ps->red = ps->green = ps->blue = get(99);
}

static void __fastcall Diagonal2x2(RGBA* ps, int x, int y)
{
	int i = yuvY(ps);
	put(i,0);					// subrange dither
	put(85,100);
	put(170,101);
	put(170,102);
	put(85,103);				// 2x2 diagonal matrix 
	put(100+2*(y%2)+x%2,1);		// matrix index 
	put(get(0)>get(get(1))?255:0,99);
	ps->red = ps->green = ps->blue = get(99);
}

static void __fastcall Diagonal4x4(RGBA* ps, int x, int y)
{
	int i = yuvY(ps);
	put(i,0);					// subrange dither
	put(28,100);
	put(227,101);
	put(57,102);
	put(198,103);				// diagonal matrix 4x4 row 1 
	put(142,104);
	put(85,105);
	put(170,106);
	put(113,107);				// diagonal matrix 4x4 row 2
	put(60,108);
	put(198,109);
	put(28,110);
	put(227,111);				// diagonal matrix 4x4 row 3
	put(170,112);
	put(113,113);
	put(142,114);
	put(85,115);				// diagonal matrix 4x4 row 4
	put(100+4*(y%4)+x%4,1);		// matrix index 
	put(get(0)>get(get(1))?255:0,99);
	ps->red = ps->green = ps->blue = get(99);
}

static void __fastcall HexOrder1(RGBA* ps, int x, int y)
{
	int i = yuvY(ps);
	put(i,0);					// subrange dither
	put(63,100);
	put(127,101);
	put(191,102);
	put(63,103);				// 2x2 dither matrix 
	put(100+2*(y%2)+x%2,1);		// matrix index 
	put(get(0)>get(get(1))?255:0,99);
	ps->red = ps->green = ps->blue = get(99);
}

static void __fastcall HexOrder2(RGBA* ps, int x, int y)
{
	int i = yuvY(ps);
	put(i,0);					// subrange dither
	put(2,100);
	put(5,101);
	put(8,102);					// 3x3 hex matrix row 1 
	put(7,103);
	put(1,102);
	put(4,105);					// 3x3 hex matrix row 2 
	put(3,106);
	put(6,104);
	put(9,108);					// 3x3 hex matrix row 3 
	put(100+3*(y%3)+(x+(y%3/3))%3,1);	// matrix index 
	put(get(0)>(get(get(1))*255/10)?255:0,99);
	ps->red = ps->green = ps->blue = get(99);
}

static void __fastcall SpiralDot4x4(RGBA* ps, int x, int y)
{
	int i = yuvY(ps);
	put(i,0);					// subrange dither
	put(150,100);
	put(135,101);
	put(120,102);
	put(105,103);				// spiral matrix 4x4 row 1 
	put(165,104);
	put(240,105);
	put(225,106);
	put(90,107);				// spiral matrix 4x4 row 2
	put(180,108);
	put(195,109);
	put(210,110);
	put(75,111);				// spiral matrix 4x4 row 3  
	put(15,112);
	put(30,113);
	put(45,114);
	put(60,115);				// spiral matrix 4x4 row 4
	put(100+4*(y%4)+x%4,1);		// matrix index 
	put(get(0)>get(get(1))?255:0,99);
	ps->red = ps->green = ps->blue = get(99);
}

static void __fastcall RandomDot(RGBA* ps, int x, int y)
{
	int i = yuvY(ps);
	put(i,0);					// subrange dither
	put(get(0)>rnd(0,255)?255:0,99);
	ps->red = ps->green = ps->blue = get(99);
}


static __PIXELPROC filter_array[] = {
	{ Dither2x2,		_T("モノクロディザ 2x2") },
	{ Dither4x4,		_T("モノクロディザ 4x4") },
	{ Diagonal2x2,		_T("モノクロディザ 斜角 2x2") },
	{ Diagonal4x4,		_T("モノクロディザ 斜角 4x4") },
	{ HexOrder1,		_T("モノクロディザ ヘキサオーダ 2x2") },
	{ HexOrder2,		_T("モノクロディザ ヘキサオーダ 3x3") },
	{ SpiralDot4x4,		_T("モノクロディザ スパイラルドット 4x4") },
	{ RandomDot,		_T("モノクロディザ ランダムドット") },
};

#define MAX_FILTER			(sizeof(filter_array)/sizeof(__PIXELPROC))

BOOL dither_works(mif& src, LPRECT prc, int filter)
{
	CRect rc;
	if (!src.validaterect(prc, &rc) || filter < 0 || filter >= MAX_FILTER)
		return FALSE;
	src.m_bits = 24;

	int width = rc.Width();
	int height = rc.Height();

	init_stack();

	void (__fastcall *pixelproc)(RGBA*, int, int);
	pixelproc = filter_array[filter].pfn;

	for (int y = 0; y < height; y++) {
		RGBA* ps = src.pos(rc.left, y + rc.top);
		for (int x = 0; x < width; x++) {
			(*pixelproc)(ps, x, y);
			ps++;
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//

class CDitherDlg : public CDialogImpl<CDitherDlg>, public CFilterDialogImpl<CDitherDlg>
{
public:
	CDitherDlg() : m_filter_number(0) { }

	enum { IDD = IDD_DITHERWORKS };
	int m_filter_number;
	CListBox m_list;
	HDIB m_hdib;

BEGIN_MSG_MAP(CDitherDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CDitherDlg>)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChange)
END_MSG_MAP()

	void OnSelChange();

	BOOL InitDialog(LPARAM lParam);
	HDIB GetPreview(HDIB hdib);
};

static void OnSelChange(HWND hdlg);
static void OnNotifyPreview(HWND hdlg, NMHDR* phdr);

BOOL CDitherDlg::InitDialog(LPARAM lParam)
{
	m_list = GetDlgItem(IDC_LIST);
	for (int i = 0; i < MAX_FILTER; i++)
		m_list.AddString(filter_array[i].filter_name);
	m_list.SetCurSel(m_filter_number);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CDitherDlg::OnSelChange()
{
	m_filter_number = m_list.GetCurSel();
	UpdatePreview();
}

HDIB CDitherDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	dither_works(image, NULL, m_filter_number);
	return image.createdib();
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("expression\\dither");
static TCHAR keyLast[] = _T("last");

HANDLE dither_works_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	srand(0);

	CDitherDlg dlg;
	regkey reg(pfe->hkey, secRoot);
	dlg.m_filter_number = reg.getvalue(keyLast, dlg.m_filter_number);

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setvalue(keyLast, dlg.m_filter_number);

		CWaitCursor wc;
		if (image.attachdib(hdib) && dither_works(image, prc, dlg.m_filter_number))
			return image.createdib();
	}
	return NULL;
}


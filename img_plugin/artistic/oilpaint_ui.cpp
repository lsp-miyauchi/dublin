// oilpaint.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define INTENSITY(r,g,b)	((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();

HDIB oilpaint(HDIB hSrc, LPRECT prc, int rad)
{
	mif src;
	if (src.attachdib(hSrc) && src.oilpaint(prc, rad))
		return src.createdib();
	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// COilPaintDlg ダイアログ

class COilPaintDlg : public CDialogImpl<COilPaintDlg>, public CFilterDialogImpl<COilPaintDlg>
{
public:
	COilPaintDlg() : m_amount(3) { }

	enum { IDD = IDD_OILPAINT };
	HDIB m_hdib;
	CTrackCtrl m_slider;
	int m_amount;

BEGIN_MSG_MAP(COilPaintDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<COilPaintDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

/////////////////////////////////////////////////////////////////////////////
// COilPaintDlg メッセージ ハンドラ

BOOL COilPaintDlg::InitDialog(LPARAM lParam) 
{
	m_slider = GetDlgItem(IDC_SLIDER);
	m_slider.SetRange(1, 8);
	m_slider.SetPos(m_amount);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void COilPaintDlg::HScroll(int val, HWND hScrollBar)
{
	if (hScrollBar == m_slider) {
		m_amount = val;
		UpdatePreview();
	}
}

HDIB COilPaintDlg::GetPreview(HDIB hdib)
{
	return oilpaint(hdib, NULL, m_amount);
}

////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("artistic\\oilpaint");
static TCHAR keyLast[] = _T("last");

HANDLE oilpaint_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	regkey reg(pfe->hkey, secRoot);

	COilPaintDlg dlg;
	dlg.m_amount = reg.getvalue(keyLast, 3);

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	if (dlg.DoModal(pfe->hparent) == IDOK) {
		reg.setvalue(keyLast, dlg.m_amount);
		CWaitCursor wc;
pfnProgress = pfe->progress;
		h = oilpaint(hdib, prc, dlg.m_amount);
pfnProgress = NULL;
	}
	GlobalFree(dlg.m_hdib);
	return h;
}

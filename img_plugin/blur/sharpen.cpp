// blur_simple.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HDIB __fastcall sharpen(HDIB hdib, LPRECT prc, int level)
{
	mif image;
	if (image.attachdib(hdib) && image.sharpen2(prc, level))
		return image.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CSharpenDlg ダイアログ

class CSharpenDlg : public CDialogImpl<CSharpenDlg>, public CFilterDialogImpl<CSharpenDlg>
{
public:
	CSharpenDlg();

	enum { IDD = IDD_SHARPEN };
	CTrackCtrl m_slider;
	int m_level;
	HDIB m_hdib;

BEGIN_MSG_MAP(CSharpenDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CSharpenDlg>)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};


CSharpenDlg::CSharpenDlg()
{
	m_level = 5;
}

/////////////////////////////////////////////////////////////////////////////
// CSharpenDlg メッセージ ハンドラ

BOOL CSharpenDlg::InitDialog(LPARAM lParam) 
{
	m_slider = GetDlgItem(IDC_SLIDER);
	m_slider.SetRange(1, 80);
	m_slider.SetPageSize(5);
	m_slider.SetTicFreq(10);
	m_slider.SetPos(m_level);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CSharpenDlg::HScroll(int val, HWND hScrollBar)
{
	if (::GetDlgCtrlID(hScrollBar) == IDC_SLIDER) {
		m_level = val;
		UpdatePreview();
	}
}

HDIB CSharpenDlg::GetPreview(HDIB hdib)
{
	return sharpen(hdib, NULL, m_level);
}

///////////////////////////////////////////////////////////////////////
//
static TCHAR sszKey[] = _T("Sharpen");

HANDLE sharpen_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CSharpenDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, _T("sharpen\\sharpen"));
	dlg.m_level = reg.getvalue(sszKey, 3);

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setvalue(sszKey, dlg.m_level);

		CWaitCursor wc;
		return sharpen(hdib, prc, dlg.m_level);
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////
//

HANDLE sharpen_edge(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image;
	if (image.attachdib(hdib) && image.filter_3x3(prc, filterSharpenEdge))
		return image.createdib();
	return NULL;
}

// mosaic.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MOSAIC_RENGA			0x0001
#define MOSAIC_CROSS			0x0002

static HDIB mosaic(HDIB hsrc, LPRECT prc, int cxBlock, int cyBlock, UINT flags)
{
	mif src;
	if (src.attachdib(hsrc) && src.pixelize(prc, cxBlock, cyBlock, flags & MOSAIC_RENGA, flags & MOSAIC_CROSS))
		return src.createdib();
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CMosaicDlg ダイアログ

class CMosaicDlg : public CDialogImpl<CMosaicDlg>, public CFilterDialogImpl<CMosaicDlg>
{
public:
	CMosaicDlg();

	enum { IDD = IDD_MOSAIC };
	CTrackCtrl m_xslider;
	CTrackCtrl m_yslider;
	int m_xBlock;
	int m_yBlock;
	UINT m_nFlags;
	HDIB m_hdib;

BEGIN_MSG_MAP(CMosaicDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CMosaicDlg>)
	ON_BN_CLICKED(IDC_RENGA, OnCheckChanged)
	ON_BN_CLICKED(IDC_ROWCROSS, OnCheckChanged)
END_MSG_MAP()

	void OnCheckChanged();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CMosaicDlg::CMosaicDlg()
{
	m_xBlock = 16;
	m_yBlock = 10;
	m_nFlags = MOSAIC_CROSS | MOSAIC_RENGA;
}

/////////////////////////////////////////////////////////////////////////////
// CMosaicDlg メッセージ ハンドラ

BOOL CMosaicDlg::InitDialog(LPARAM lParam)
{
	m_xslider = GetDlgItem(IDC_XSLIDER);
	m_yslider = GetDlgItem(IDC_YSLIDER);

	m_xslider.SetRange(2, 255);
	m_xslider.SetPageSize(8);
	m_xslider.SetTicFreq(32);
	m_xslider.SetPos(m_xBlock);

	m_yslider.SetRange(2, 255);
	m_yslider.SetPageSize(8);
	m_yslider.SetTicFreq(32);
	m_yslider.SetPos(m_yBlock);

	if (m_nFlags & MOSAIC_RENGA) CheckDlgButton(IDC_RENGA, TRUE);
	if (m_nFlags & MOSAIC_CROSS) CheckDlgButton(IDC_ROWCROSS, TRUE);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CMosaicDlg::HScroll(int val, HWND hBar) 
{
	if (hBar == m_xslider)		m_xBlock = val;
	else if (hBar == m_yslider)	m_yBlock = val;
	else						return;
	UpdatePreview();
}

void CMosaicDlg::OnCheckChanged() 
{
	m_nFlags = 0;
	if (IsDlgButtonChecked(IDC_RENGA))		m_nFlags |= MOSAIC_RENGA;
	if (IsDlgButtonChecked(IDC_ROWCROSS))	m_nFlags |= MOSAIC_CROSS;
	UpdatePreview();
}

HDIB CMosaicDlg::GetPreview(HDIB hdib)
{
	return mosaic(hdib, NULL, m_xBlock, m_yBlock, m_nFlags);
}


//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("mosaic\\mosaic");
static TCHAR keyLast[] = _T("last");

HANDLE mosaic_normal_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CMosaicDlg dlg;

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, secRoot);
	int a[3];
	int len = reg.getbinary(keyLast, a, sizeof(a));
	if (len == sizeof(a)) {
		dlg.m_xBlock = a[0];
		dlg.m_yBlock = a[1];
		dlg.m_nFlags = a[2];
	}

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		a[0] = dlg.m_xBlock;
		a[1] = dlg.m_yBlock;
		a[2] = dlg.m_nFlags;
		reg.setbinary(keyLast, a, sizeof(int) * 3);

		return mosaic(hdib, prc, dlg.m_xBlock, dlg.m_yBlock, dlg.m_nFlags);
	}
	return NULL;
}

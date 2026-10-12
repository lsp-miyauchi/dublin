// cubism_ui.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "cubism.h"
#include "..\filterdialog.h"

extern PROGRESSCALLBACK pfnProgress;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCubismDlg ダイアログ


class CCubismDlg : public CDialogImpl<CCubismDlg>, public CFilterDialogImpl<CCubismDlg>
{
public:
	CCubismDlg();

	enum { IDD = IDD_CUBISM };
	CComboBox m_combo;
	CTrackCtrl m_tslider;
	CTrackCtrl m_sslider;
	CUBISMINFO m_cp;
	HDIB m_hdib;

BEGIN_MSG_MAP(CCubismDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CCubismDlg>)
	ON_CBN_SELCHANGE(IDC_COMBO, OnFormChange)
	ON_CBN_SELCHANGE(IDC_COLOR, OnColorChange)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
	void OnFormChange();
	void OnColorChange();
};

CCubismDlg::CCubismDlg()
{
	m_cp.tile_size = 15;
	m_cp.tile_saturation = 200;
	m_cp.bg_color = RGB(0, 0, 0);
	m_cp.numpolygon = 4;
}

/////////////////////////////////////////////////////////////////////////////
// CCubismDlg メッセージ ハンドラ

BOOL CCubismDlg::InitDialog(LPARAM lParam) 
{
	m_combo = GetDlgItem(IDC_COMBO);
	m_tslider = GetDlgItem(IDC_TILESIZE);
	m_sslider = GetDlgItem(IDC_TILESAT);

	m_tslider.SetRange(5, 120);
	m_tslider.SetTicFreq(25);
	m_tslider.SetPos(m_cp.tile_size);

	m_sslider.SetRange(25, 300);
	m_sslider.SetPageSize(25);
	m_sslider.SetTicFreq(25);
	m_sslider.SetPos(m_cp.tile_saturation);
	m_sslider.SetBuddyBase(1);

	int sel = m_cp.numpolygon - 3;
	if (sel < 0 || sel > 6)
		sel = 1;
	m_combo.SetCurSel(sel);
	ColorPicker_SetBkColor(GetDlgItem(IDC_COLOR), m_cp.bg_color);

	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CCubismDlg::OnFormChange()
{
	m_cp.numpolygon = m_combo.GetCurSel() + 3;
	UpdatePreview();
}

void CCubismDlg::OnColorChange()
{
	m_cp.bg_color = ColorPicker_GetBkColor(GetDlgItem(IDC_COLOR));
	UpdatePreview();
}

void CCubismDlg::HScroll(int val, HWND hScrollBar)
{
	switch (::GetDlgCtrlID(hScrollBar)) {
		case IDC_TILESIZE: m_cp.tile_size = val; break;
		case IDC_TILESAT: m_cp.tile_saturation = val; break;
		default: return;
	}
	UpdatePreview();
}

HDIB CCubismDlg::GetPreview(HDIB hdib)
{
	return cubism(hdib, NULL, &m_cp);
}


////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("artistic\\cubism");
static TCHAR keyLast[] = _T("last");

HANDLE cubism_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	regkey reg(pfe->hkey, secRoot);

	CCubismDlg dlg;
	CUBISMINFO cp;
	if (reg.getbinary(keyLast, &cp, sizeof(cp)) == sizeof(cp))
		memcpy(&dlg.m_cp, &cp, sizeof(cp));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	if (dlg.DoModal(pfe->hparent) == IDOK) {
		reg.setbinary(keyLast, &dlg.m_cp, sizeof(dlg.m_cp));
		CWaitCursor wc;
pfnProgress = pfe->progress;
		h = cubism(hdib, prc, &dlg.m_cp);
pfnProgress = NULL;
	}
	GlobalFree(dlg.m_hdib);
	return h;
}

// edge.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct tagBLENDTYPE {
	int mode;
	TCHAR* text;
} BLENDTYPE;

static BLENDTYPE blendtype[] = {
	blendNormal,				_T("通常"),
	blendDark,					_T("比較（暗）"),
	blendLite,					_T("比較（明）"),
	blendHue,					_T("色相"),
	blendSat,					_T("彩度"),
	blendLum,					_T("明度"),
	blendMul,					_T("乗算"),
	blendScreen,				_T("スクリーン"),
	blendOverlay,				_T("オーバーレイ"),
	blendHardLight,				_T("ハードライト"),
	blendSoftLight,				_T("ソフトライト"),
	blendDiff,					_T("差の絶対値"),
	blendExclusion,				_T("除外"),
	blendDodge,					_T("覆い焼きカラー"),
	blendBurn,					_T("焼き込みカラー"),
	blendAdd,					_T("加算"),
	blendDel,					_T("減算"),
	-1,							_T("")
};

typedef struct tagEDGEDETECTINFO {
	double amount;
	BOOL negative;
	BOOL gray;
	BOOL merge_source;
	int opaque;
	int blendtype;
} EDGEDETECTINFO;

static HDIB edge_detect(HDIB hsrc, LPRECT prc, EDGEDETECTINFO* pei)
{
	mif image;
	CRect rc;
	if (!image.attachdib(hsrc) || !image.validaterect(prc, &rc))
		return NULL;
	image.m_bits = 24;

	mif work(rc.Width(), rc.Height(), 24);
	work.blt(0, 0, rc.Width(), rc.Height(), &image, rc.left, rc.top);
	work.edge(NULL, pei->amount);

	if (pei->negative) work.negative(NULL);
	if (pei->gray) work.desaturate(NULL);

	if (pei->merge_source && pei->opaque > 0) {
		register RGBA* p = work.m_pixels;
		for (int i = 0; i < work.m_packets; i++) {
			p->alpha = 255;
			p++;
		}
		image.addlayer(&work, rc.left, rc.top, pei->blendtype, pei->opaque * 255 / 100);
	}
	else
		image.blt(rc.left, rc.top, rc.Width(), rc.Height(), &work, 0, 0);
	return image.createdib();
}


/////////////////////////////////////////////////////////////////////////////
// CEdgeDlg ダイアログ

class CEdgeDlg : public CDialogImpl<CEdgeDlg>, public CFilterDialogImpl<CEdgeDlg>
{
public:
	enum { IDD = IDD_EDGE };
	EDGEDETECTINFO m_ei;
	HDIB m_hdib;
	CTrackCtrl m_aslider;
	CTrackCtrl m_oslider;
	CComboBox m_combo;

	CEdgeDlg();

BEGIN_MSG_MAP(CEdgeDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CEdgeDlg>)
	ON_BN_CLICKED(IDC_MERGE, OnCheckMerge)
	ON_BN_CLICKED(IDC_NEGATIVE, OnNegative)
	ON_BN_CLICKED(IDC_GRAY, OnNegative)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_CBN_SELCHANGE(IDC_COMBO, OnSelChange)
END_MSG_MAP()

	void OnCheckMerge();
	void OnSelChange();
	void OnNegative();
	void OnReset();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};

CEdgeDlg::CEdgeDlg()
{
	m_ei.amount = 2.0;
	m_ei.negative = FALSE;
	m_ei.merge_source = FALSE;
	m_ei.blendtype = blendScreen;
	m_ei.opaque = 100;
	m_ei.gray = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CEdgeDlg メッセージ ハンドラ

BOOL CEdgeDlg::InitDialog(LPARAM lParam) 
{
	m_aslider = GetDlgItem(IDC_AMOUNT);
	m_oslider = GetDlgItem(IDC_OPAQUE);
	m_combo = GetDlgItem(IDC_COMBO);

	m_aslider.SetRange(10, 300);
	m_aslider.SetTicFreq(30);
	m_aslider.SetPos((int)(m_ei.amount * 10));
	m_aslider.SetBuddyBase(1);
	CheckDlgButton(IDC_NEGATIVE, m_ei.negative);
	CheckDlgButton(IDC_GRAY, m_ei.gray);

	CheckDlgButton(IDC_MERGE, m_ei.merge_source);
	int sel = 0;
	int i = 0;
	while (blendtype[i].mode != -1) {
		m_combo.AddString(blendtype[i].text);
		if (blendtype[i].mode == m_ei.blendtype)
			sel = i;
		i++;
	}
	m_combo.SetCurSel(sel);

	m_oslider.SetRange(0, 100);
	m_oslider.SetTicFreq(10);
	m_oslider.SetPos(m_ei.opaque);

	SetPreviewImage(m_hdib);
	OnCheckMerge();
	return TRUE;
}

void CEdgeDlg::HScroll(int val, HWND hBar)
{
	if (hBar == m_aslider)		m_ei.amount = (double)val / 10.0;
	else if (hBar == m_oslider)	m_ei.opaque = val;
	else						return;

	UpdatePreview();
}

HDIB CEdgeDlg::GetPreview(HDIB hdib)
{
	return edge_detect(hdib, NULL, &m_ei);
}

void CEdgeDlg::OnReset()
{
	m_ei.amount = 2.0;
	m_ei.negative = FALSE;
	m_ei.merge_source = FALSE;
	m_ei.blendtype = blendScreen;
	m_ei.opaque = 100;
	m_ei.gray = FALSE;

	m_aslider.SetPos((int)(m_ei.amount * 10));
	m_oslider.SetPos(m_ei.opaque);

	int i = 0;
	while (blendtype[i].mode != -1) {
		if (blendtype[i].mode == m_ei.blendtype) {
			m_combo.SetCurSel(i);
			break;
		}
		i++;
	}

	CheckDlgButton(IDC_NEGATIVE, m_ei.negative);
	CheckDlgButton(IDC_GRAY, m_ei.gray);
	CheckDlgButton(IDC_MERGE, m_ei.merge_source);

	OnCheckMerge();
}

void CEdgeDlg::OnCheckMerge() 
{
	m_ei.merge_source = IsDlgButtonChecked(IDC_MERGE);
	m_combo.EnableWindow(m_ei.merge_source);
	m_oslider.EnableWindow(m_ei.merge_source);
	UpdatePreview();
}

void CEdgeDlg::OnSelChange() 
{
	m_ei.blendtype = blendtype[m_combo.GetCurSel()].mode;
	UpdatePreview();
}

void CEdgeDlg::OnNegative() 
{
	m_ei.negative = IsDlgButtonChecked(IDC_NEGATIVE);
	m_ei.gray = IsDlgButtonChecked(IDC_GRAY);
	UpdatePreview();
}


/////////////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("edge\\edge_detect");
static TCHAR keyLast[] = _T("last");

HANDLE edge_detect_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CEdgeDlg dlg;

	regkey reg(pfe->hkey, secRoot);
	EDGEDETECTINFO ei;
	if (reg.getbinary(keyLast, &ei, sizeof(ei)) == sizeof(ei))
		memcpy(&dlg.m_ei, &ei, sizeof(ei));

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	HDIB h = NULL;
	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_ei, sizeof(dlg.m_ei));

		CWaitCursor wc;
		h = edge_detect(hdib, prc, &dlg.m_ei);
	}
	return h;
}

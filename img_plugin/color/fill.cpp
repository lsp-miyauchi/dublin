// fill.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

enum {
	LEFTtoRIGHT = 0,
	RIGHTtoLEFT,
	TOPtoBOTTOM,
	BOTTOMPtoTOP,
};

typedef struct __BLENDTYPE {
	int mode;
	LPCTSTR text;
} BLENDTYPE;

////////////////////////////////////////////////////////////////////////
//

static HDIB fill_gradient(HDIB hdib, LPRECT prc, GRADCOLORS* map, int numcolors, int direction, int opaq, int blendtype)
{
	mif image;
	if (image.attachdib(hdib)) {
		int opaq_val = opaq * 255 / 100;
		image.gradientfill(prc, map, numcolors, direction, opaq_val, blendtype);
		return image.createdib();
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// CFillDlg

static TCHAR* sszBounds[] = {
	_T("左上に向かって"),
	_T("上に向かって"),
	_T("右上に向かって"),
	_T("左に向かって"),
	_T("右に向かって"),
	_T("左下に向かって"),
	_T("下に向かって"),
	_T("右下に向かって")
};

static const BLENDTYPE blendtype[] = {
	blendNormal,				_T("通常"),
	blendDark,					_T("比較（暗）"),
	blendLite,					_T("比較（明）"),
	blendHue,					_T("色相"),
	blendSat,					_T("彩度"),
	blendColor,					_T("カラー"),
	blendLum,					_T("輝度"),
	blendMul,					_T("乗算"),
	blendScreen,				_T("スクリーン"),
	blendDither,				_T("ディザ合成"),
	blendOverlay,				_T("オーバーレイ"),
	blendHardLight,				_T("ハードライト"),
	blendSoftLight,				_T("ソフトライト"),
	blendDiff,					_T("差の絶対値"),
	blendExclusion,				_T("除外"),
	blendDodge,					_T("覆い焼きカラー"),
	blendBurn,					_T("焼き込みカラー"),
	blendAdd,					_T("加算"),
	blendDel,					_T("減算"),
	-1,							NULL,
};

class CFillDlg : public CDialogImpl<CFillDlg>, public CFilterDialogImpl<CFillDlg>{
public:
	CFillDlg();

	enum { IDD = IDD_FILL };

	FILTER_EXEC* m_pfe;
	CComboBox m_blendcombo;
	CComboBox m_boundcombo;
	CTrackCtrl m_slider;
	CWindow m_view;
	GRADCOLORS m_grad[MAX_GRADCOLORS];
	int m_colors;
	int m_direction;
	int m_opaq;
	int m_blendtype;

	HIMAGELIST m_him;
	HDIB m_hdib;

BEGIN_MSG_MAP(CFillDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CFillDlg>)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_GRADLIST, OnLoadPreset)
	ON_BN_CLICKED(IDC_RESET, OnGradReset)
	ON_CBN_SELCHANGE(IDC_COMBO, OnDirChange)
	ON_CBN_SELCHANGE(IDC_BLENDCOMBO, OnBlendChange)
	ON_CONTROL(GEN_COLORCHANGE, IDC_COLOR, OnGradChanged)
END_MSG_MAP()

	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpdi);
	void OnBlendChange();
	void OnDirChange();
	void OnLoadPreset();
	void OnGradReset();
	void OnGradChanged();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
};


CFillDlg::CFillDlg()
{
	m_blendtype = blendNormal;
	m_direction = LEFTtoRIGHT;
	m_grad[0].cr = RGB(0, 0, 0);
	m_grad[0].pos = 0;
	m_grad[1].cr = RGB(255, 255, 255);
	m_grad[1].pos = 1000;
	m_colors = 2;
	m_opaq = 100;

	m_him = ImageList_LoadImage(RESINST, MAKEINTRESOURCE(IDB_ARROW), 11, 0, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
}

BOOL CFillDlg::InitDialog(LPARAM lParam)
{
	m_blendcombo = GetDlgItem(IDC_BLENDCOMBO);
	m_boundcombo = GetDlgItem(IDC_COMBO);
	m_slider = GetDlgItem(IDC_SLIDER);

	// グラデーションの向き
	CClientDC dc(NULL);
	HFONT hfont = dc.SelectFont(m_boundcombo.GetFont());
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int height = max(tm.tmHeight, 11) + 2;
	dc.SelectFont(hfont);
	m_boundcombo.SetItemHeight(0, height);
	for (int i = 0; i < 8; i++)
		m_boundcombo.AddString(sszBounds[i]);
	m_boundcombo.SetCurSel(m_direction);

	SendDlgItemMessage(IDC_COLOR, GEM_SETNUMLIMIT, sizeof(m_grad) / sizeof(GRADCOLORS));
	SendDlgItemMessage(IDC_COLOR, GEM_SETCOLORS, m_colors, (LPARAM)m_grad);

	// 描画モード
	int sel = 0, i = 0;
	while (blendtype[i].mode != -1) {
		m_blendcombo.AddString(blendtype[i].text);
		if (blendtype[i].mode == m_blendtype)
			sel = i;
		i++;
	}
	m_blendcombo.SetCurSel(sel);

	// 不透明度
	m_slider.SetRange(0, 100);
	m_slider.SetPos(m_opaq);

	SetWholePreview(TRUE);
	SetPreviewImage(m_hdib);
	UpdatePreview();
	return TRUE;
}

void CFillDlg::OnDrawItem(int id, DRAWITEMSTRUCT* pdi)
{
	CDCHandle dc = pdi->hDC;
	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		COLORREF clrText, clrBack;
		if (pdi->itemState & ODS_SELECTED) {
			clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
			clrBack = GetSysColor(COLOR_HIGHLIGHT);
		}
		else {
			clrText = GetSysColor(COLOR_WINDOWTEXT);
			clrBack = GetSysColor(COLOR_WINDOW);
		}
		dc.SetTextColor(clrText);
		dc.SetBkColor(clrBack);

		CRect rc(&pdi->rcItem);
		dc.ExtTextOut(rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc, NULL, 0, NULL);
		ImageList_Draw(m_him, pdi->itemID, dc, rc.left + 2, (rc.top + rc.bottom - 11) / 2, ILD_TRANSPARENT);
		rc.left += 11 + 4;

		CString str;
		m_boundcombo.GetLBText(pdi->itemID, str);
		dc.DrawText(str, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}
}

void CFillDlg::HScroll(int val, HWND hScrollBar)
{
	if (::GetDlgCtrlID(hScrollBar) == IDC_SLIDER) {
		m_opaq = val;
		UpdatePreview();
	}
}

HDIB CFillDlg::GetPreview(HDIB hdib)
{
	return fill_gradient(hdib, NULL, m_grad, m_colors, m_direction, m_opaq, m_blendtype);
}

void CFillDlg::OnBlendChange() 
{
	m_blendtype = blendtype[m_blendcombo.GetCurSel()].mode;
	UpdatePreview();
}

void CFillDlg::OnDirChange()
{
	m_direction = m_boundcombo.GetCurSel();
	UpdatePreview();
}

void CFillDlg::OnLoadPreset()
{
	GRADEDITINFO info;
	info.numcolors = GradEdit_GetColors(GetDlgItem(IDC_COLOR), info.grad);
	if (m_pfe->gradeditor(m_hWnd, &info)) {
		m_colors = info.numcolors;
		memcpy(m_grad, info.grad, sizeof(GRADCOLORS) * info.numcolors);
		GradEdit_SetColors(GetDlgItem(IDC_COLOR), m_colors, m_grad);
		UpdatePreview();
	}
}

void CFillDlg::OnGradReset()
{
	m_colors = 2;
	m_grad[0].pos = 0;
	m_grad[0].cr = RGB(0, 0, 0);
	m_grad[1].pos = 1000;
	m_grad[1].cr = RGB(255, 255, 255);
	SendDlgItemMessage(IDC_COLOR, GEM_SETCOLORS, m_colors, (LPARAM)m_grad);
	UpdatePreview();
}

void CFillDlg::OnGradChanged()
{
	m_colors = GradEdit_GetColors(GetDlgItem(IDC_COLOR), m_grad);
	UpdatePreview();
}

/////////////////////////////////////////////////////////////////////////////
//

typedef struct __FILLGRADIENT {
	int blendtype;
	int direction;
	int opaq;
	int numcolors;
	GRADCOLORS grad[MAX_GRADCOLORS];
} FILLGRADIENT;

static TCHAR secRoot[] = _T("color\\fill");
static TCHAR keyLast[] = _T("last");

HANDLE fill_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	CFillDlg dlg;
	dlg.m_pfe = pfe;

	FILLGRADIENT fg;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &fg, sizeof(fg)) == sizeof(fg)) {
		dlg.m_blendtype = fg.blendtype;
		dlg.m_direction = fg.direction;
		dlg.m_opaq = fg.opaq;
		dlg.m_colors = fg.numcolors;
		memcpy(dlg.m_grad, fg.grad, sizeof(dlg.m_grad));
	}

	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		fg.blendtype = dlg.m_blendtype;
		fg.direction = dlg.m_direction;
		fg.opaq = dlg.m_opaq;
		fg.numcolors = dlg.m_colors;
		memcpy(fg.grad, dlg.m_grad, sizeof(dlg.m_grad));
		reg.setbinary(keyLast, &fg, sizeof(fg));
		return fill_gradient(hdib, prc, dlg.m_grad, dlg.m_colors, dlg.m_direction, dlg.m_opaq, dlg.m_blendtype);
	}
	return NULL;
}

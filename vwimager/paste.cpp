// paste.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include <math.h>
#include "paste.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct tagBLENDTYPE {
	int mode;
	TCHAR* text;
} BLENDTYPE;

BLENDTYPE blendtype[] = {
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

/////////////////////////////////////////////////////////////////////////////
// CPasteDlg ダイアログ

CPasteDlg::CPasteDlg()
{
	CAppReg reg;
	if (reg.GetBinary(IDS_SEC_EDIT, IDS_KEY_PASTE, &m_pp, sizeof(m_pp)) != sizeof(m_pp)) {
		m_pp.ai.method = blendNormal;
		m_pp.ai.opaque = 100;
		m_pp.ai.shadow = FALSE;
		m_pp.ai.shadow_gauss = 16;
		m_pp.ai.shadow_distance = 3;
		m_pp.ai.shadow_color = RGB(0, 0, 0);
		m_pp.noalign = FALSE;
		m_pp.ptOffset = CPoint(0, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPasteDlg メッセージ ハンドラ

BOOL CPasteDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_combo = GetDlgItem(IDC_COMBO);
	m_oslider = GetDlgItem(IDC_SLIDER);
	m_gslider = GetDlgItem(IDC_GSLIDER);
	m_dslider = GetDlgItem(IDC_DSLIDER);

	int sel = 0;
	int i = 0;
	while (blendtype[i].mode != -1) {
		m_combo.AddString(blendtype[i].text);
		if (blendtype[i].mode == m_pp.ai.method)
			sel = i;
		i++;
	}
	m_combo.SetCurSel(sel);

	m_oslider.SetRange(0, 100);
	m_oslider.SetPos(m_pp.ai.opaque);

	CheckDlgButton(IDC_DROPSHADOW, m_pp.ai.shadow);

	m_gslider.SetRange(0, 50);
	m_gslider.SetPageSize(10);
	m_gslider.SetPos(m_pp.ai.shadow_gauss);
	m_gslider.SetBuddyBase(1);

	m_dslider.SetRange(-16, 16);
	m_dslider.SetPageSize(4);
	m_dslider.SetPos(m_pp.ai.shadow_distance);

	ColorPicker_SetBkColor(GetDlgItem(IDC_SHADOWCOLOR), m_pp.ai.shadow_color);
	OnDropShadow();

	CRect rc;
	GetDlgItem(IDC_ANCHOR).GetWindowRect(&rc);
	ScreenToClient(&rc);
	GetDlgItem(IDC_ANCHOR).DestroyWindow();

	rc.right = rc.left + ANCHOR_BLOCK_SIZE * 3;
	rc.bottom = rc.top + ANCHOR_BLOCK_SIZE * 3;
	AdjustWindowRectEx(&rc, WS_CHILD | WS_VISIBLE, FALSE, WS_EX_CLIENTEDGE);
	m_anchorwnd.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE, IDC_ANCHOR);
	m_anchorwnd.SetAnchor(m_pp.anchor);

	CheckDlgButton(m_pp.noalign ? IDC_NOALIGN : IDC_ALIGN, TRUE);
	SetDlgItemInt(IDC_XOFFSET, m_pp.ptOffset.x);
	SetDlgItemInt(IDC_YOFFSET, m_pp.ptOffset.y);
	OnCheckAlign();

	return TRUE;
}

void CPasteDlg::OnDropShadow()
{
	m_pp.ai.shadow = IsDlgButtonChecked(IDC_DROPSHADOW);
	m_gslider.EnableWindow(m_pp.ai.shadow);
	m_dslider.EnableWindow(m_pp.ai.shadow);
	GetDlgItem(IDC_SHADOWCOLOR).EnableWindow(m_pp.ai.shadow);
}

void CPasteDlg::OnCheckAlign()
{
	m_pp.noalign = IsDlgButtonChecked(IDC_NOALIGN);
	GetDlgItem(IDC_XOFFSET).EnableWindow(!m_pp.noalign);
	GetDlgItem(IDC_YOFFSET).EnableWindow(!m_pp.noalign);
	m_anchorwnd.EnableWindow(!m_pp.noalign);
}

void CPasteDlg::OnOK()
{
	m_pp.ai.opaque = m_oslider.GetPos();
	m_pp.ai.method = blendtype[m_combo.GetCurSel()].mode;
	m_pp.ai.shadow = IsDlgButtonChecked(IDC_DROPSHADOW);
	m_pp.ai.shadow_gauss = m_gslider.GetPos();
	m_pp.ai.shadow_distance = m_dslider.GetPos();
	m_pp.ai.shadow_color = ColorPicker_GetBkColor(GetDlgItem(IDC_SHADOWCOLOR));

	m_pp.ptOffset.x = (int)GetDlgItemInt(IDC_XOFFSET);
	m_pp.ptOffset.y = (int)GetDlgItemInt(IDC_YOFFSET);
	m_pp.anchor = m_anchorwnd.GetAnchor();

	CAppReg reg;
	reg.SetBinary(IDS_SEC_EDIT, IDS_KEY_PASTE, &m_pp, sizeof(m_pp));
	EndDialog(IDOK);
}

//////////////////////////////////////////////////////////////////////
//

#define CBLACK		RGB(0,0,0)
#define CWHITE		RGB(255,255,255)

BOOL __fastcall paste_image(mif& src, mif& dst, int x, int y, ARITHINFO* pai)
{
	if (!pai || pai->opaque == 0)
		return FALSE;

	int opaque = pai->opaque * 255 / 100;

	src.m_bits = 24;
	dst.m_bits = 24;

	register RGBA* p = dst.m_pixels;
	for (int i = 0; i < dst.m_packets; i++) {
		p->alpha = 255;
		p++;
	}

	if (pai->shadow) {
		// ぼかしの適用範囲を保証する意味で、直径ピクセル分を確保する
		int delta;
		if (pai->shadow_gauss > 0)
			delta = (int)floor((double)(pai->shadow_gauss + 10) / 5.0);
		else
			delta = 0;

		mif shadow(dst.m_width + delta * 2, dst.m_height + delta * 2, 24);
		shadow.solidfill(0, 0, shadow.m_width, shadow.m_height, CBLACK, 255);
		shadow.solidfill(delta, delta, dst.m_width, dst.m_height, CWHITE, 255);
		if (pai->shadow_gauss > 0)
			shadow.blur_gauss_rle(NULL, (double)pai->shadow_gauss / 10.0);

		CRect rc(0, 0, dst.m_width, dst.m_height);
		rc.OffsetRect(delta - pai->shadow_distance, delta - pai->shadow_distance);
		shadow.validaterect(&rc, &rc);
		shadow.solidfill(&rc, CBLACK, 255);
		shadow.settoalpha();
		shadow.solidfill(NULL, pai->shadow_color, 255);

		src.addlayer(&shadow,
					x - delta + pai->shadow_distance,
					y - delta + pai->shadow_distance,
					blendNormal, opaque);
	}

	src.addlayer(&dst, x, y, pai->method, opaque);
	return TRUE;
}

HDIB __fastcall PasteImage(HDIB hsrc, HDIB hdst, int x, int y, ARITHINFO* pai)
{
	if (!hsrc || !hdst || !pai || pai->opaque == 0)
		return NULL;

	mif src(hsrc);
	mif dst(hdst);
	if (paste_image(src, dst, x, y, pai))
		return src.createdib();
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// font.cpp

#include "stdafx.h"
#include <math.h>
#include "resource.h"
#include "common.h"
#include "font_util.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CTextInputDlg ダイアログ

class CTextInputDlg : public CDialogImpl<CTextInputDlg>
{
public:
	CTextInputDlg();
	~CTextInputDlg();

	enum { IDD = IDD_TEXTINPUT };

BEGIN_MSG_MAP(CTextInputDlg)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_DROPSHADOW, OnDropShadow)
	ON_BN_CLICKED(IDC_OUTLINE, OnOutline)
	ON_BN_CLICKED(IDC_DRAWINNER, OnDrawInner)
	ON_CBN_SELCHANGE(IDC_FONT, OnFontChange)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_RESET, OnGradReset)
	ON_BN_CLICKED(IDC_GRADEDIT, OnGradEdit)
	ON_BN_CLICKED(IDC_EMBOSS_EDGE, OnEmbossEdge)
	ON_BN_CLICKED(IDC_FLIP_HORZ, OnOtherParamChanged)
	ON_BN_CLICKED(IDC_FLIP_VERT, OnOtherParamChanged)
	ON_CBN_SELCHANGE(IDC_ANTIALIAS, OnOtherParamChanged)
	ON_CBN_SELCHANGE(IDC_FORMAT, OnOtherParamChanged)
	ON_CBN_SELCHANGE(IDC_DCOMBO, OnOtherParamChanged)
	ON_CBN_SELCHANGE(IDC_INNERCOLOR, OnOtherParamChanged)
	ON_CBN_SELCHANGE(IDC_SHADOWCOLOR, OnOtherParamChanged)
	ON_EN_CHANGE(IDC_EDIT, OnOtherParamChanged)
	ON_CONTROL(GEN_COLORCHANGE, IDC_COLOR, OnOtherParamChanged)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnFontChange();
	void OnDropShadow();
	void OnEmbossEdge();
	void OnOutline();
	void OnDrawInner();
	void OnLoad();
	void OnSave();
	void OnGradReset();
	void OnGradEdit();
	void OnOtherParamChanged();

	void SetParams();
	void ShowFontInfo();

	CEdit m_edit;
	CTrackCtrl m_shadow_gauss;
	CTrackCtrl m_shadow_distance;
	CTrackCtrl m_shadow_opaque;
	CTrackCtrl m_shear;
	CTrackCtrl m_scale;
	CTrackCtrl m_opaque;
	CTrackCtrl m_azimuth;
	CTrackCtrl m_elevation;
	CFontPickerCtrl m_fontpick;

	CString m_csText;
	DRAWSTRINGINFO m_dsi;
	HDIB m_hdib;
	FILTER_EXEC* m_pfe;
};


CTextInputDlg::CTextInputDlg()
{
	m_pfe = NULL;
	m_hdib = NULL;
	memset(&m_dsi, 0, sizeof(m_dsi));

	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &m_dsi.lf);
	m_dsi.lf.lfHeight <<= 2;
	m_dsi.lf.lfWidth = 0;
	m_dsi.numcolors = 2;
	m_dsi.grad[0].cr = RGB(0, 0, 0);
	m_dsi.grad[0].pos = 0;
	m_dsi.grad[1].cr = RGB(255, 255, 255);
	m_dsi.grad[1].pos = 1000;
	m_dsi.state |= DSS_DROPSHADOW;
	m_dsi.shadow_gauss = 10;
	m_dsi.shadow_distance = 3;
	m_dsi.shadow_cr = RGB(0, 0, 0);
	m_dsi.shadow_opaque = 80;
	m_dsi.direction = strokeRight;
	m_dsi.outline_cr = RGB(255, 255, 255);
	m_dsi.shear = 0;
	m_dsi.opaque = 100;
	m_dsi.char_scale = 100;
	m_dsi.ee_azimuth = 135.0;
	m_dsi.ee_elevation = 60.0;
}

CTextInputDlg::~CTextInputDlg()
{
	if (m_hdib) GlobalFree(m_hdib);
}

/////////////////////////////////////////////////////////////////////////////
// CTextInputDlg メッセージ ハンドラ

static const TCHAR sfPixel[] = _T("px");
static const TCHAR sfPer[] = _T("%");
static const TCHAR sfRad[] = _T("度");


BOOL CTextInputDlg::OnInitDialog(LPARAM lParam) 
{
	ExecuteDlgInit(IDD);
	CenterWindow();

	m_edit = GetDlgItem(IDC_EDIT);
	m_shadow_gauss = GetDlgItem(IDC_GSLIDER);
	m_shadow_distance = GetDlgItem(IDC_DSLIDER);
	m_shadow_opaque = GetDlgItem(IDC_SHADOWOPAQUE);
	m_shear = GetDlgItem(IDC_SSLIDER);
	m_scale = GetDlgItem(IDC_WSLIDER);
	m_opaque = GetDlgItem(IDC_OSLIDER);
	m_azimuth = GetDlgItem(IDC_AZIMUTH);
	m_elevation = GetDlgItem(IDC_ELEVATION);
	m_fontpick = GetDlgItem(IDC_FONT);

	// 塗りの色
	GradEdit_SetNumLimit(GetDlgItem(IDC_COLOR), MAX_GRADCOLORS);

	// ドロップシャドウ
	m_shadow_gauss.SetSuffix(sfPixel);
	m_shadow_gauss.SetRange(0, 50);
	m_shadow_gauss.SetPageSize(10);
	m_shadow_gauss.SetTicFreq(10);
	m_shadow_gauss.SetBuddyBase(1);

	m_shadow_distance.SetSuffix(sfPixel);
	m_shadow_distance.SetRange(0, 16);
	m_shadow_distance.SetPageSize(5);
	m_shadow_distance.SetTicFreq(4);

	m_shadow_opaque.SetSuffix(sfPer);
	m_shadow_opaque.SetRange(10, 100);
	m_shadow_opaque.SetPageSize(10);
	m_shadow_opaque.SetTicFreq(20);

	// 傾斜
	m_shear.SetSuffix(sfRad);
	m_shear.SetRange(-45, 45);
	m_shear.SetPageSize(10);
	m_shear.SetTicFreq(15);

	// 文字幅スケール
	m_scale.SetSuffix(sfPer);
	m_scale.SetRange(20, 200);
	m_scale.SetPageSize(20);
	m_scale.SetTicFreq(40);

	// 不透明度
	m_opaque.SetSuffix(sfPer);
	m_opaque.SetRange(10, 100);
	m_opaque.SetPageSize(10);
	m_opaque.SetTicFreq(20);

	// アジマス - 方向
	m_azimuth.SetSuffix(sfRad);
	m_azimuth.SetRange(0, 360);
	m_azimuth.SetPageSize(30);
	m_azimuth.SetTicFreq(36);

	// エレベーション - 持ち上げ
	m_elevation.SetRange(20, 90);
	m_elevation.SetPageSize(10);
	m_elevation.SetTicFreq(10);

	// 文字列
	m_edit.SetLimitText(511);

	SetParams();
	return TRUE;
}

void CTextInputDlg::SetParams()
{
	// フォント
	m_fontpick.SetLogFont(&m_dsi.lf);

	// アンチエリアス
	CComboBox combo = GetDlgItem(IDC_ANTIALIAS);
	int sel;
	if (m_dsi.state & DSS_AA_CLEAR) sel = 1;
	else if (m_dsi.state & DSS_AA_STRONG) sel = 2;
	else sel = 0;
	combo.SetCurSel(sel);

	// 配置
	combo = GetDlgItem(IDC_FORMAT);
	if (m_dsi.state & DSS_FMT_CENTER) sel = 1;
	else if (m_dsi.state & DSS_FMT_RIGHT) sel = 2;
	else sel = 0;
	combo.SetCurSel(sel);

	// 塗りの色
	GradEdit_SetColors(GetDlgItem(IDC_COLOR), m_dsi.numcolors, m_dsi.grad);

	// 塗りの向き
	combo = GetDlgItem(IDC_DCOMBO);
	if (m_dsi.direction == strokeRight)		sel = 0;
	else if (m_dsi.direction == strokeDown)	sel = 1;
	else									sel = 2;
	combo.SetCurSel(sel);

	// 袋文字
	CheckDlgButton(IDC_OUTLINE, (m_dsi.state & DSS_OUTLINE) != 0);
	CheckDlgButton(IDC_DRAWINNER, (m_dsi.state & DSS_OUTLINE_INNER) != 0);
	// 内部色
	ColorPicker_SetBkColor(GetDlgItem(IDC_INNERCOLOR), m_dsi.outline_cr);
	OnOutline();

	// ドロップシャドウ
	CheckDlgButton(IDC_DROPSHADOW, (m_dsi.state & DSS_DROPSHADOW) != 0);
	m_shadow_gauss.SetPos(m_dsi.shadow_gauss);
	m_shadow_distance.SetPos(m_dsi.shadow_distance);
	m_shadow_opaque.SetPos(m_dsi.shadow_opaque);

	// 傾斜
	m_shear.SetPos(m_dsi.shear);

	// 文字幅スケール
	m_scale.SetPos(m_dsi.char_scale);

	// 不透明度
	m_opaque.SetPos(m_dsi.opaque);

	ColorPicker_SetBkColor(GetDlgItem(IDC_SHADOWCOLOR), m_dsi.shadow_cr);
	OnDropShadow();

	// 上下反転
	CheckDlgButton(IDC_FLIP_VERT, (m_dsi.state & DSS_FLIP_VERT) != 0);

	// 左右反転
	CheckDlgButton(IDC_FLIP_HORZ, (m_dsi.state & DSS_FLIP_HORZ) != 0);

	// 立体化
	CheckDlgButton(IDC_EMBOSS_EDGE, (m_dsi.state & DSS_EMBOSS_EDGE) != 0);
	m_azimuth.SetPos((int)m_dsi.ee_azimuth);
	m_elevation.SetPos((int)m_dsi.ee_elevation);
	OnEmbossEdge();

	m_edit.SetWindowText(m_csText);

	ShowFontInfo();
}

void CTextInputDlg::OnOK()
{
	if (m_csText.IsEmpty()) {
		MessageBeep(0);
		m_edit.SetFocus();
		return;
	}
	EndDialog(IDOK);
}

void CTextInputDlg::OnFontChange()
{
	m_fontpick.GetLogFont(&m_dsi.lf);
	ShowFontInfo();
}

void CTextInputDlg::ShowFontInfo()
{
	CZoomViewCtrl view = GetDlgItem(IDC_PREVIEW);
	if (!m_hdib) {
		CRect rc;
		view.GetClientRect(&rc);
		mif image(rc.Width(), rc.Height(), 24);
		image.solidfill(NULL, RGB(176, 184, 234), 255);
		m_hdib = image.createdib();
	}

	HDIB hdib;
	if (!m_csText.IsEmpty()) {
		m_dsi.text = m_csText;
		m_dsi.pt.x = m_dsi.pt.y = 0;
		hdib = draw_string(m_hdib, &m_dsi);
	}
	else
		hdib = copyhandle(m_hdib);

	hdib = view.UpdateImage(hdib, TRUE);
	if (hdib) GlobalFree(hdib);
}

void CTextInputDlg::OnDropShadow()
{
	m_dsi.state &= ~DSS_DROPSHADOW;
	bool enable = (IsDlgButtonChecked(IDC_DROPSHADOW) != 0);
	if (enable)
		m_dsi.state |= DSS_DROPSHADOW;
	m_shadow_gauss.EnableWindow(enable);
	m_shadow_distance.EnableWindow(enable);
	m_shadow_opaque.EnableWindow(enable);
	GetDlgItem(IDC_SHADOWCOLOR).EnableWindow(enable);
	ShowFontInfo();
}

void CTextInputDlg::OnEmbossEdge()
{
	m_dsi.state &= ~DSS_EMBOSS_EDGE;
	bool enable = (IsDlgButtonChecked(IDC_EMBOSS_EDGE) != 0);
	if (enable)
		m_dsi.state |= DSS_EMBOSS_EDGE;

	m_azimuth.EnableWindow(enable);
	m_elevation.EnableWindow(enable);
	ShowFontInfo();
}

void CTextInputDlg::OnOutline()
{
	m_dsi.state &= ~DSS_OUTLINE;
	bool enable = (IsDlgButtonChecked(IDC_OUTLINE) != 0);
	if (enable)
		m_dsi.state |= DSS_OUTLINE;
	GetDlgItem(IDC_DRAWINNER).EnableWindow(enable);
	OnDrawInner();
	ShowFontInfo();
}

void CTextInputDlg::OnDrawInner()
{
	m_dsi.state &= ~DSS_OUTLINE_INNER;
	bool enable = (IsDlgButtonChecked(IDC_DRAWINNER) != 0);
	if (enable)
		m_dsi.state |= DSS_OUTLINE_INNER;
	GetDlgItem(IDC_INNERCOLOR).EnableWindow((m_dsi.state & DSS_OUTLINE) && enable);
	ShowFontInfo();
}

void CTextInputDlg::OnHScroll(UINT nSBCode, UINT nPos, HWND hBar)
{
	CTrackCtrl bar = hBar;
	int n;
	switch (nSBCode) {
		case TB_TOP:
		case TB_BOTTOM:
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_PAGEUP:
		case TB_PAGEDOWN: n = bar.GetPos(); break;
		case TB_THUMBTRACK: n = (int)nPos; break;
		default: return;
	}

	switch (::GetDlgCtrlID(hBar)) {
		case IDC_GSLIDER:		m_dsi.shadow_gauss = (WORD)n; break;
		case IDC_DSLIDER:		m_dsi.shadow_distance = (short)n; break;
		case IDC_SHADOWOPAQUE:	m_dsi.shadow_opaque = (short)n; break;
		case IDC_SSLIDER:		m_dsi.shear = n; break;
		case IDC_OSLIDER:		m_dsi.opaque = n; break;
		case IDC_WSLIDER:		m_dsi.char_scale = n; ShowFontInfo(); break;
		case IDC_AZIMUTH:		m_dsi.ee_azimuth = (double)n; break;
		case IDC_ELEVATION:		m_dsi.ee_elevation = (double)n; break;
		default: return;
	}
	ShowFontInfo();
}

void CTextInputDlg::OnOtherParamChanged()
{
	// 文字列
	m_edit.GetWindowText(m_csText);
	m_dsi.text = m_csText;

	// 上下反転
	m_dsi.state &= ~DSS_FLIP_VERT;
	if (IsDlgButtonChecked(IDC_FLIP_VERT)) m_dsi.state |= DSS_FLIP_VERT;

	// 左右反転
	m_dsi.state &= ~DSS_FLIP_HORZ;
	if (IsDlgButtonChecked(IDC_FLIP_HORZ)) m_dsi.state |= DSS_FLIP_HORZ;

	// アンチエリアス
	CComboBox combo = GetDlgItem(IDC_ANTIALIAS);
	m_dsi.state &= ~DSS_AA_MASK;
	switch (combo.GetCurSel()) {
		case 1: m_dsi.state |= DSS_AA_CLEAR; break;
		case 2: m_dsi.state |= DSS_AA_STRONG; break;
	}

	// 配置
	combo = GetDlgItem(IDC_FORMAT);
	m_dsi.state &= ~DSS_FMT_MASK;
	switch (combo.GetCurSel()) {
		case 1: m_dsi.state |= DSS_FMT_CENTER; break;
		case 2: m_dsi.state |= DSS_FMT_RIGHT; break;
	}

	// 塗りの向き
	combo = GetDlgItem(IDC_DCOMBO);
	switch (combo.GetCurSel()) {
		case 0: m_dsi.direction = strokeRight; break;
		case 1: m_dsi.direction = strokeDown; break;
		default: m_dsi.direction = strokeDownRight; break;
	}

	// 塗りの色
	m_dsi.numcolors = GradEdit_GetColors(GetDlgItem(IDC_COLOR), m_dsi.grad);

	// 袋文字
	m_dsi.outline_cr = ColorPicker_GetBkColor(GetDlgItem(IDC_INNERCOLOR));

	// ドロップシャドウ
	m_dsi.shadow_cr = ColorPicker_GetBkColor(GetDlgItem(IDC_SHADOWCOLOR));

	ShowFontInfo();
}

static TCHAR sszExt[] = _T(".dsf");
static TCHAR sszFilter[] = _T("DSFファイル(*.dsf)\0*.dsf\0すべてのファイル(*.*)\0*.*\0\0");

#define MAGIC_DRAWSTRING				0x00465344

typedef struct tagDRAWSTRINGFILE {
	DWORD magic;
	DRAWSTRINGINFO dsi;
	TCHAR szText[MAX_PATH * 4];
} DRAWSTRINGFILE;

void CTextInputDlg::OnLoad()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, sszFilter, m_hWnd);
	TCHAR dir[MAX_PATH * 2];
	lstrcpy(dir, m_pfe->userdir);
	dlg.m_ofn.lpstrInitialDir = dir;
	dlg.m_ofn.lpstrDefExt = sszExt;
	if (dlg.DoModal(m_hWnd) != IDOK) return;

	HANDLE hf = CreateFile(dlg.m_szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		DRAWSTRINGFILE dsf;
		DWORD dw;
		if (ReadFile(hf, &dsf, sizeof(dsf), &dw, NULL) && dw == sizeof(dsf) && dsf.magic == MAGIC_DRAWSTRING) {
			m_dsi = dsf.dsi;
			m_csText = dsf.szText;

			SetParams();
		}
		CloseHandle(hf);
	}
}

void CTextInputDlg::OnSave()
{
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY, sszFilter, m_hWnd);
	TCHAR dir[MAX_PATH * 2];
	lstrcpy(dir, m_pfe->userdir);
	dlg.m_ofn.lpstrInitialDir = dir;
	dlg.m_ofn.lpstrDefExt = sszExt;
	if (dlg.DoModal(m_hWnd) != IDOK) return;

	HANDLE hf = CreateFile(dlg.m_szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		DRAWSTRINGFILE dsf;
		memset(&dsf, 0, sizeof(dsf));
		dsf.magic = MAGIC_DRAWSTRING;
		dsf.dsi = m_dsi;
		lstrcpy(dsf.szText, m_csText);

		DWORD dw;
		WriteFile(hf, &dsf, sizeof(dsf), &dw, NULL);
		CloseHandle(hf);
	}
}

void CTextInputDlg::OnGradReset()
{
	m_dsi.numcolors = 2;
	m_dsi.grad[0].pos = 0;
	m_dsi.grad[0].cr = RGB(0, 0, 0);
	m_dsi.grad[1].pos = 1000;
	m_dsi.grad[1].cr = RGB(255, 255, 255);
	GradEdit_SetColors(GetDlgItem(IDC_COLOR), m_dsi.numcolors, m_dsi.grad);
	ShowFontInfo();
}

void CTextInputDlg::OnGradEdit()
{
	GRADEDITINFO info;
	info.numcolors = GradEdit_GetColors(GetDlgItem(IDC_COLOR), info.grad);
	if (m_pfe->gradeditor(m_hWnd, &info)) {
		m_dsi.numcolors = info.numcolors;
		memcpy(m_dsi.grad, info.grad, sizeof(GRADCOLORS) * info.numcolors);
		GradEdit_SetColors(GetDlgItem(IDC_COLOR), m_dsi.numcolors, m_dsi.grad);
		ShowFontInfo();
	}
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("textinput\\string");
static TCHAR keyParam[] = _T("param");
static TCHAR keyText[] = _T("text");

HANDLE textinput_string_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CTextInputDlg dlg;
	dlg.m_pfe = pfe;
	regkey reg(pfe->hkey, secRoot);

	DRAWSTRINGINFO dsi;
	memset(&dsi, 0, sizeof(dsi));
	if (reg.getbinary(keyParam, &dsi, sizeof(dsi)) == sizeof(dsi))
		memcpy(&dlg.m_dsi, &dsi, sizeof(dsi));

	TCHAR buf[MAX_PATH * 2];
	if (reg.gettext(keyText, buf, _COUNTOF(buf)) > 0)
		dlg.m_csText = buf;

	if (dlg.DoModal(pfe->hparent) != IDOK)
		return NULL;

	memcpy(&dsi, &dlg.m_dsi, sizeof(dsi));
	reg.setbinary(keyParam, &dsi, sizeof(dsi));
	reg.settext(keyText, dlg.m_csText);

	if (dsi.lf.lfFaceName[0] == '@') {
		dsi.state |= DSS_TATEGAKI;
		dsi.shear *= -1;
	}

	HDIB h = create_dragstring(dlg.m_csText, &dsi.lf, dsi.char_scale, dsi.state, dsi.shear);
	CPoint pt;
	if (pfe->tracker(h, &pt, RGB(255, 255, 255))) {
		GlobalFree(h);
		dsi.pt.x = pt.x;
		dsi.pt.y = pt.y;
		CWaitCursor wc;
		return draw_string(hdib, &dsi);
	}
	return NULL;
}

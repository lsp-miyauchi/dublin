// bumpmap_string.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "font_util.h"
#include "resource.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define MAGIC_BUMPMAPSTRING					0x46534d42		// "BMSF"

typedef struct tagBUMPMAPSTRINGFILE {
	DWORD magic;
	BUMPMAPSTRINGINFO bsi;
	TCHAR szText[MAX_PATH * 2];
} BUMPMAPSTRINGFILE;


/////////////////////////////////////////////////////////////////////////////
// CBumpmapStringDlg ダイアログ

class CBumpmapStringDlg : public CDialogImpl<CBumpmapStringDlg>
{
public:
	CBumpmapStringDlg();

	enum { IDD = IDD_BUMPMAPSTRING };
	CEdit m_edit;
	CComboBox m_aliascombo;
	CTrackCtrl m_widthslider;
	CTrackCtrl m_shearslider;
	CTrackCtrl m_azimuthslider;
	CTrackCtrl m_elevationslider;
	CTrackCtrl m_depthslider;
	CTrackCtrl m_ambientslider;
	CFontPickerCtrl m_fontpick;
	CZoomViewCtrl m_view;

	CString m_csText;
	BUMPMAPSTRINGINFO m_bsi;
	HDIB m_hdib;
	FILTER_EXEC* m_pfe;

BEGIN_MSG_MAP(CBumpmapStringDlg)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT, OnParamChange)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_INVERT, OnParamChange)
	ON_BN_CLICKED(IDC_LINEAR, OnParamChange)
	ON_BN_CLICKED(IDC_SINUOSIDAL, OnParamChange)
	ON_BN_CLICKED(IDC_SPHERICAL, OnParamChange)
	ON_CBN_SELCHANGE(IDC_ANTIALIAS, OnParamChange)
	ON_CBN_SELCHANGE(IDC_FONT, OnParamChange)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hBar);
	void OnDestroy();
	void OnParamChange();
	void OnLoad();
	void OnSave();

	void SetParams();
	void ShowFontInfo();
};

CBumpmapStringDlg::CBumpmapStringDlg()
{
	m_pfe = NULL;
	m_hdib = NULL;
	memset(&m_bsi, 0, sizeof(m_bsi));

	NONCLIENTMETRICS nm;
	// VISTA!!
	nm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(nm), &nm, FALSE);

	m_bsi.lf = nm.lfMenuFont;
	m_bsi.char_scale = 100;
	m_bsi.bp.azimuth    = 135.0;
	m_bsi.bp.elevation  = 45.0;
	m_bsi.bp.depth      = 1;
	m_bsi.bp.xofs       = 0;
	m_bsi.bp.yofs       = 0;
	m_bsi.bp.waterlevel = 0;
	m_bsi.bp.ambient    = 0;
	m_bsi.bp.compensate = TRUE;
	m_bsi.bp.invert     = FALSE;
	m_bsi.bp.type       = bumpmapLinear;
	m_bsi.bp.tiled      = FALSE;
	m_bsi.bp.compensate = TRUE;
	m_csText = _T("");
}


/////////////////////////////////////////////////////////////////////////////
// CBumpmapStringDlg メッセージ ハンドラ

BOOL CBumpmapStringDlg::OnInitDialog(LPARAM lParam)
{
	ExecuteDlgInit(IDD);
	CenterWindow();
	m_edit = GetDlgItem(IDC_EDIT);
	m_aliascombo = GetDlgItem(IDC_ANTIALIAS);
	m_widthslider = GetDlgItem(IDC_WSLIDER);
	m_shearslider = GetDlgItem(IDC_SSLIDER);
	m_depthslider = GetDlgItem(IDC_DEPTH);
	m_ambientslider = GetDlgItem(IDC_LIGHTNING);
	m_elevationslider = GetDlgItem(IDC_ELEVATION);
	m_azimuthslider = GetDlgItem(IDC_AZIMUTH);
	m_fontpick = GetDlgItem(IDC_FONT);
	m_view = GetDlgItem(IDC_PREVIEW);

	// 文字幅
	m_widthslider.SetRange(20, 200);
	m_widthslider.SetPageSize(20);
	m_widthslider.SetTicFreq(20);

	// 傾斜角
	m_shearslider.SetRange(-45, 45);
	m_shearslider.SetPageSize(10);
	m_shearslider.SetTicFreq(10);

	// アジマス - 方向
	m_azimuthslider.SetRange(0, 3600);
	m_azimuthslider.SetPageSize(300);
	m_azimuthslider.SetBuddyBase(1);
	m_azimuthslider.SetTicFreq(360);

	// エレベーション - 持ち上げ
	m_elevationslider.SetRange(5, 900);
	m_elevationslider.SetPageSize(100);
	m_elevationslider.SetBuddyBase(1);
	m_elevationslider.SetTicFreq(100);

	// 深度
	m_depthslider.SetRange(1, 65);
	m_depthslider.SetPageSize(10);
	m_depthslider.SetTicFreq(10);

	// 飽和度
	m_ambientslider.SetRange(0, 255);
	m_ambientslider.SetPageSize(32);
	m_ambientslider.SetTicFreq(64);

	CRect rc;
	m_view.GetClientRect(&rc);
	mif image(rc.Width(), rc.Height(), 24);
	image.solidfill(NULL, RGB(176, 184, 234), 255);
	m_hdib = image.createdib();

	// 文字列
	m_edit.SetLimitText(511);

	SetParams();
	return TRUE;
}

void CBumpmapStringDlg::SetParams()
{
	m_fontpick.SetLogFont(&m_bsi.lf);

	m_widthslider.SetPos(m_bsi.char_scale);
	m_shearslider.SetPos(m_bsi.shear);
	m_azimuthslider.SetPos((int)(m_bsi.bp.azimuth * 10));
	m_elevationslider.SetPos((int)(m_bsi.bp.elevation * 10));
	m_depthslider.SetPos(m_bsi.bp.depth);
	m_ambientslider.SetPos(m_bsi.bp.ambient);

	CheckDlgButton(IDC_INVERT, (m_bsi.bp.invert != FALSE));
	CheckDlgButton(IDC_LINEAR, (m_bsi.bp.type == bumpmapLinear));
	CheckDlgButton(IDC_SPHERICAL, (m_bsi.bp.type == bumpmapSpherical));
	CheckDlgButton(IDC_SINUOSIDAL, (m_bsi.bp.type == bumpmapSinuosidal));

	int sel = 0;
	if (m_bsi.state & DSS_AA_CLEAR) sel = 1;
	else if (m_bsi.state & DSS_AA_STRONG) sel = 2;
	m_aliascombo.SetCurSel(sel);
	m_edit.SetWindowText(m_csText);

	ShowFontInfo();
}

void CBumpmapStringDlg::OnOK()
{
	m_edit.GetWindowText(m_csText);
	if (m_csText.IsEmpty()) {
		MessageBeep(0);
		m_edit.SetFocus();
		return;
	}
	EndDialog(IDOK);
}

void CBumpmapStringDlg::ShowFontInfo()
{
	HDIB hdib;
	if (!m_csText.IsEmpty()) {
		m_bsi.text = m_csText;
		hdib = draw_bumpmapstring(m_hdib, &m_bsi);
	}
	else
		hdib = copyhandle(m_hdib);
	hdib = m_view.UpdateImage(hdib, TRUE);
	if (hdib) GlobalFree(hdib);
}

void CBumpmapStringDlg::OnDestroy() 
{
	if (m_hdib) GlobalFree(m_hdib);
}

void CBumpmapStringDlg::OnHScroll(UINT nSBCode, UINT nPos, HWND hBar)
{
	CTrackCtrl track = hBar;
	int n;
	switch (nSBCode) {
	case TB_TOP:
	case TB_BOTTOM:
	case TB_LINEUP:
	case TB_LINEDOWN:
	case TB_PAGEUP:
	case TB_PAGEDOWN: n = track.GetPos(); break;
	case TB_THUMBTRACK: n = nPos; break;
	default: DefWindowProc(); return;
	}

	switch (::GetDlgCtrlID(hBar)) {
	case IDC_WSLIDER: m_bsi.char_scale = n; break;
	case IDC_SSLIDER: m_bsi.shear = n; break;
	case IDC_AZIMUTH: m_bsi.bp.azimuth = (double)n / 10.0; break;
	case IDC_DEPTH: m_bsi.bp.depth = n; break;
	case IDC_ELEVATION: m_bsi.bp.elevation = (double)n / 10.0; break;
	case IDC_LIGHTNING: m_bsi.bp.ambient = n; break;
	default: return;
	}
	ShowFontInfo();
}

void CBumpmapStringDlg::OnParamChange()
{
	m_edit.GetWindowText(m_csText);

	m_bsi.state &= ~DSS_AA_MASK;
	switch (m_aliascombo.GetCurSel()) {
	case 1: m_bsi.state |= DSS_AA_CLEAR; break;
	case 2: m_bsi.state |= DSS_AA_STRONG; break;
	}

	m_bsi.bp.invert = IsDlgButtonChecked(IDC_INVERT);
	if (IsDlgButtonChecked(IDC_LINEAR))			m_bsi.bp.type = bumpmapLinear;
	else if (IsDlgButtonChecked(IDC_SPHERICAL))	m_bsi.bp.type = bumpmapSpherical;
	else										m_bsi.bp.type = bumpmapSinuosidal;
	m_fontpick.GetLogFont(&m_bsi.lf);

	ShowFontInfo();
}

static TCHAR sszExt[] = _T(".bsf");
static TCHAR sszFilter[] = _T("BSFファイル(*.bsf)\0*.bsf\0すべてのファイル(*.*)\0*.*\0\0");

void CBumpmapStringDlg::OnLoad()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, sszFilter, m_hWnd);
	TCHAR dir[MAX_PATH * 2];
	lstrcpy(dir, m_pfe->userdir);
	dlg.m_ofn.lpstrInitialDir = dir;
	dlg.m_ofn.lpstrDefExt = sszExt;
	if (dlg.DoModal(m_hWnd) != IDOK) return;

	HANDLE hf = CreateFile(dlg.m_szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		BUMPMAPSTRINGFILE bsf;
		DWORD dw;
		if (ReadFile(hf, &bsf, sizeof(bsf), &dw, NULL) && dw == sizeof(bsf) && bsf.magic == MAGIC_BUMPMAPSTRING) {
			memcpy(&m_bsi, &bsf.bsi, sizeof(m_bsi));
			m_csText = bsf.szText;
			SetParams();
		}
		CloseHandle(hf);
	}
}

void CBumpmapStringDlg::OnSave()
{
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY, sszFilter, m_hWnd);
	TCHAR dir[MAX_PATH * 2];
	lstrcpy(dir, m_pfe->userdir);
	dlg.m_ofn.lpstrInitialDir = dir;
	dlg.m_ofn.lpstrDefExt = sszExt;
	if (dlg.DoModal(m_hWnd) != IDOK) return;

	HANDLE hf = CreateFile(dlg.m_szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		BUMPMAPSTRINGFILE bsf;
		memset(&bsf, 0, sizeof(bsf));
		bsf.magic = MAGIC_BUMPMAPSTRING;
		memcpy(&bsf.bsi, &m_bsi, sizeof(m_bsi));
		lstrcpy(bsf.szText, m_csText);

		DWORD dw;
		WriteFile(hf, &bsf, sizeof(bsf), &dw, NULL);
		CloseHandle(hf);
	}
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("textinput\\bumpmap_string");
static TCHAR keyParam[] = _T("param");
static TCHAR keyText[] = _T("text");

HANDLE textinput_bumpmap_string_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CBumpmapStringDlg dlg;
	dlg.m_pfe = pfe;
	regkey reg(pfe->hkey, secRoot);

	BUMPMAPSTRINGINFO bsi;
	memset(&bsi, 0, sizeof(bsi));
	if (reg.getbinary(keyParam, &bsi, sizeof(bsi)) == sizeof(bsi))
		memcpy(&dlg.m_bsi, &bsi, sizeof(bsi));

	TCHAR buf[MAX_PATH * 2];
	if (reg.gettext(keyText, buf, _COUNTOF(buf)) > 0)
		dlg.m_csText = buf;

	if (dlg.DoModal(pfe->hparent) != IDOK)
		return NULL;

	memcpy(&bsi, &dlg.m_bsi, sizeof(bsi));
	reg.setbinary(keyParam, &bsi, sizeof(bsi));
	reg.settext(keyText, dlg.m_csText);

	if (bsi.lf.lfFaceName[0] == '@') {
		bsi.state |= DSS_TATEGAKI;
		bsi.shear *= -1;
	}

	HDIB h = create_dragstring(bsi.text, &bsi.lf, bsi.char_scale, bsi.state, bsi.shear);
	CPoint pt;
	if (pfe->tracker(h, &pt, RGB(255, 255, 255))) {
		GlobalFree(h);
		bsi.pt.x = pt.x;
		bsi.pt.y = pt.y;
		CWaitCursor wc;
		return draw_bumpmapstring(hdib, &bsi);
	}
	return NULL;
}

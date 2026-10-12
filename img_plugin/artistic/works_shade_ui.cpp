//////////////////////////////////////////////////////////////////////
// works_shade_ui.cpp

#include "stdafx.h"
#include "ffactory_sup.h"
#include "..\filterdialog.h"

void progress_init(int maxprogress);
void progress_continue();
void progress_term();


extern PROGRESSCALLBACK pfnProgress;

class CShadeWorks : public tagSINGLEWORKS
{
public:
	CShadeWorks() {};
	int shade_radius;
	int fade_level;
	BOOL brightness;
};

static BOOL shade_works(mif& src, LPRECT prc, const CShadeWorks& cw)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	init_stack();

	// 広域変数設定
	X = rc.Width();
	Y = rc.Height();
	X_center = X / 2;
	Y_center = Y / 2;
	M = dist(0, 0, X, Y) / 2;
	item_ptr = cw.magic->item;

	mif org(X, Y, 24);
	org.blt(0, 0, X, Y, &src, rc.left, rc.top);

	if (cw.shade_radius > 0)
		src.blur_simple(rc, cw.shade_radius);

progress_init(Y);
	mif dst(X, Y, 24);
	dst.blt(0, 0, X, Y, &src, rc.left, rc.top);
	RGBA* po = org.m_pixels;
	for (int y = 0; y < Y; y++) {
		RGBA* ps = src.pos(rc.left, y + rc.top);
		for (int x = 0; x < X; x++) {
			cw.magic->pfn(ps, dst, x, y);
			if (cw.fade_level > 0) {
				if (cw.brightness) {
					po->red = max(ps->red, po->red);
					po->green = max(ps->green, po->green);
					po->blue = max(ps->blue, po->blue);
				}
				ps->red = mix(po->red, ps->red, cw.fade_level, 255);
				ps->green = mix(po->green, ps->green, cw.fade_level, 255);
				ps->blue = mix(po->blue, ps->blue, cw.fade_level, 255);
			}
			po++;
			ps++;
		}
progress_continue();
	}
progress_term();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//

class CShadeWorksDlg : public CDialogImpl<CShadeWorksDlg>, public CFilterDialogImpl<CShadeWorksDlg>
{
public:
	enum { IDD = IDD_SHADEWORKS };
	CShadeWorks m_cw;

BEGIN_MSG_MAP(CShadeWorksDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CShadeWorksDlg>)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_WHOLEVIEW, OnWholeView)
	ON_BN_CLICKED(IDC_BRIGHTNESS, OnBrightness)
	COMMAND_HANDLER(IDC_BRIGHTNESS, BN_CLICKED, OnBnClickedBrightness)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
	void OnReset();
	void OnWholeView();
	void OnBrightness();

	void SetParams(MAGIC* magic);
	LRESULT OnBnClickedBrightness(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

BOOL CShadeWorksDlg::InitDialog(LPARAM lParam)
{
	SetWindowText(m_cw.magic->fnc_name);
	CTrackCtrl track = GetDlgItem(IDC_ZSLIDER);
	track.SetRange(0, 40);
	track.SetTicFreq(10);

	track = GetDlgItem(IDC_YSLIDER);
	track.SetRange(0, 255);
	track.SetTicFreq(32);

	for (int i = 0; i < MAXPARAMS; i++) {
		track = GetDlgItem(IDC_ASLIDER + i);
		track.SetRange(0, 255);
		track.SetTicFreq(32);
	}

	SetParams(m_cw.magic);

	if ((m_cw.preview_mode & PREVIEW_BOTH) != PREVIEW_BOTH) {
		CheckDlgButton(IDC_WHOLEVIEW, (m_cw.preview_mode & PREVIEW_WHOLE) ? TRUE : FALSE);
		GetDlgItem(IDC_WHOLEVIEW).ShowWindow(SW_HIDE);
	}
	else
		CheckDlgButton(IDC_WHOLEVIEW, m_cw.wholeview);

	SetWholePreview(m_cw.wholeview);
	SetPreviewImage(m_cw.hdib);
	UpdatePreview();
	return TRUE;
}

void CShadeWorksDlg::OnWholeView()
{
	m_cw.wholeview = IsDlgButtonChecked(IDC_WHOLEVIEW);
	SetWholePreview(m_cw.wholeview);
	UpdatePreview();
}

void CShadeWorksDlg::SetParams(MAGIC* filter)
{
	int last_valid_control = 0;
	CTrackCtrl track = GetDlgItem(IDC_ZSLIDER);
	track.SetPos(m_cw.shade_radius);

	track = GetDlgItem(IDC_YSLIDER);
	track.SetPos(m_cw.fade_level);

	CheckDlgButton(IDC_BRIGHTNESS, m_cw.brightness);

	for (int i = 0; i < MAXPARAMS; i++) {
		track = GetDlgItem(IDC_ASLIDER + i);
		if (i < filter->numparams && filter->item[i].label) {
			TCHAR buf[256];
			wsprintf(buf, _T("%s(&%c)"), filter->item[i].label, 'A' + i);
			track.SetWindowText(buf);
			track.SetPos(filter->item[i].val);
			track.ShowWindow(SW_SHOW);
			last_valid_control = i;
		}
		else
			track.ShowWindow(SW_HIDE);
	}

	CRect rc;
	track = GetDlgItem(IDC_ASLIDER + last_valid_control);
	track.GetClientRect(&rc);
	track.ClientToScreen(&rc.BottomRight());
	int bottom = rc.bottom + 7 * HIWORD(GetDialogBaseUnits()) / 8;

	GetWindowRect(&rc);
	if (rc.bottom != bottom) {
		rc.bottom = bottom;
		SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}
}

void CShadeWorksDlg::OnReset()
{
	for (int n = 0; n < m_cw.magic->numparams; n++)
		m_cw.magic->item[n].val = m_cw.magic->item[n].def;
	m_cw.shade_radius = 4;
	m_cw.fade_level = 0;
	m_cw.brightness = TRUE;
	SetParams(m_cw.magic);

	UpdatePreview();
}

void CShadeWorksDlg::HScroll(int val, HWND hScrollBar)
{
	MAGIC* curfilter = m_cw.magic;

	int id = ::GetDlgCtrlID(hScrollBar);
	if (id == IDC_ZSLIDER)
		m_cw.shade_radius = val;
	else if (id == IDC_YSLIDER)
		m_cw.fade_level = val;
	else if (id >= IDC_ASLIDER && id < IDC_ASLIDER + curfilter->numparams) {
		id -= IDC_ASLIDER;
		curfilter->item[id].val = val;
	}
	else
		return;

	UpdatePreview();
}

HDIB CShadeWorksDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	shade_works(image, NULL, m_cw);
	return image.createdib();
}

void CShadeWorksDlg::OnBrightness()
{
	m_cw.brightness = IsDlgButtonChecked(IDC_BRIGHTNESS);
	UpdatePreview();
}

/////////////////////////////////////////////////////////////////////
//

static TCHAR keyLast[] = _T("last");

HANDLE shade_works_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe, MAGIC* magic, int preview_mode)
{
	int i;

	TCHAR w[128];
	TCHAR* p = w;
	if (HIWORD(pfe->param))
		p = (TCHAR*)pfe->param;
	else
		wsprintf(p, _T("shade_%02d"), pfe->param);

	TCHAR root[128];
	wsprintf(root, _T("artistic\\%s\\%s"), p, magic->fnc_name);

	CShadeWorksDlg dlg;
	CShadeWorks& cw = dlg.m_cw;

	regkey reg(pfe->hkey, root);
	if (reg.getbinary(keyLast, &cw, sizeof(cw)) == sizeof(cw)) {
		for (i = 0; i < magic->numparams; i++)
			magic->item[i].val = cw.val[i];
	}
	else {
		cw.shade_radius = 4;
		cw.fade_level = 0;
		cw.brightness = TRUE;
	}

	cw.magic = magic;
	cw.preview_mode = preview_mode;
	if ((cw.preview_mode & PREVIEW_BOTH) != PREVIEW_BOTH)
		cw.wholeview = (cw.preview_mode & PREVIEW_PART) ? FALSE : TRUE;

	mif image(hdib);
	image.crop(prc);
	cw.hdib = image.createdib();
	image.destroy();

	INT_PTR r = dlg.DoModal(pfe->hparent);
	GlobalFree(cw.hdib);
	if (r == IDOK) {
		for (i = 0; i < magic->numparams; i++)
			cw.val[i] = magic->item[i].val;

		reg.setbinary(keyLast, &cw, sizeof(cw));

		CWaitCursor wc;
pfnProgress = pfe->progress;
		BOOL bOK = (image.attachdib(hdib) && shade_works(image, prc, cw));
pfnProgress = NULL;
		if (bOK)
			return image.createdib();
	}
	return NULL;
}


LRESULT CShadeWorksDlg::OnBnClickedBrightness(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。

	return 0;
}

// Tonecurve.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"

void InitDlgCtrl(HWND hctl, UINT idResource);

/////////////////////////////////////////////////////////////////////////////
//

static COLORREF rgbShade[] = {
	RGB(255, 255, 255),
	RGB(255,   0,   0),
	RGB(  0, 255,   0),
	RGB(  0,   0, 255),
};

/////////////////////////////////////////////////////////////////////////////
// CToneCurveDlg

class CToneCurveDlg : public CDialogImpl<CToneCurveDlg>
{
public:
	CToneCurveDlg();

	enum { IDD = IDD_TONECURVE };
	CComboBox m_combo;
	CWindow m_view;

	TONEINFO m_ti;
	HDIB m_hdib;

BEGIN_MSG_MAP(CToneCurveDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(MGN_SELCHANGE, IDC_GRAPH, OnNotifyGraphChanged)
	ON_NOTIFY(MVN_GETEFFECT, IDC_PREVIEW, OnNotifyGetEffect)
	ON_CBN_SELCHANGE(IDC_COMBO, OnSelChange)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_WHOLE, OnCheckWhole)
	ON_BN_CLICKED(IDC_RESET, OnReset)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnNotifyGraphChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyGetEffect(NMHDR* pNmhdr, LRESULT* pResult);
	void OnSelChange();
	void OnCheckWhole();
	void OnReset();
	void OnOK() { EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }

	void SetValue(int ref, int val);
	void ShowParam(int chn);
};

CToneCurveDlg::CToneCurveDlg()
{
	memset(&m_ti, 0, sizeof(m_ti));
	m_hdib = NULL;
}

BOOL CToneCurveDlg::OnInitDialog(LPARAM lParam)
{
	if (!m_hdib) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	CenterWindow();
	m_combo = GetDlgItem(IDC_COMBO);
	InitDlgCtrl(m_combo, IDS_TYPE_RGB);
	m_view = GetDlgItem(IDC_PREVIEW);

	m_view.SendMessage(IVM_SETWHOLEVIEW, 0, 0);
	m_view.SendMessage(IVM_SETDIB, 0, (LPARAM)m_hdib);
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);

	m_combo.SetCurSel(0);

	// シェーダーを移動
	InfoBar_SetBkColor(GetDlgItem(IDC_SHADE), RGB(255,255,255), RGB(0,0,0));
	CRect rc, rcs;
	GetDlgItem(IDC_GRAPH).GetWindowRect(&rc);
	ScreenToClient(&rc);
	GetDlgItem(IDC_SHADE).GetWindowRect(&rcs);
	ScreenToClient(&rcs);
	GetDlgItem(IDC_SHADE).MoveWindow(rc.left, rc.bottom + 1, rc.Width(), rcs.Height(), TRUE);

	OnReset();
	return TRUE;
}

void CToneCurveDlg::OnCheckWhole()
{
	BOOL whole = IsDlgButtonChecked(IDC_WHOLE);
	m_view.SendMessage(IVM_SETWHOLEVIEW, whole, 0);
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);
}

void CToneCurveDlg::SetValue(int ref, int val)
{
	TONE* pt;
	int chn = m_combo.GetCurSel();
	if (chn == 0)		pt = &m_ti.rgb;
	else if (chn == 1)	pt = &m_ti.red;
	else if (chn == 2)	pt = &m_ti.green;
	else				pt = &m_ti.blue;
	pt->ref[0] = ref;
	pt->val[0] = val;

	ShowParam(chn);

	InfoBar_SetBkColor(GetDlgItem(IDC_SHADE), 0, rgbShade[chn]);
	SendDlgItemMessage(IDC_GRAPH, MGM_SETTONE, 0, MAKELPARAM(ref, val));
	m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);
}

void CToneCurveDlg::ShowParam(int chn)
{
	TONE* pt;
	UINT nID;
	if (chn == 0)		{ pt = &m_ti.rgb; nID = IDC_RGB; }
	else if (chn == 1)	{ pt = &m_ti.red; nID = IDC_RED; }
	else if (chn == 2)	{ pt = &m_ti.green; nID = IDC_GREEN; }
	else				{ pt = &m_ti.blue; nID = IDC_BLUE; }

	TCHAR buf[MAX_PATH];
	wsprintf(buf, _T("%03d/%03d"), pt->ref[0], pt->val[0]);
	SetDlgItemText(nID, buf);
}

void CToneCurveDlg::OnNotifyGraphChanged(NMHDR* phdr, LRESULT* pResult)
{
	MG_SELCHANGE* pmg = (MG_SELCHANGE*)phdr;
	SetValue(pmg->ref, pmg->val);
	*pResult = TRUE;
}

void CToneCurveDlg::OnNotifyGetEffect(NMHDR* phdr, LRESULT* pResult)
{
	*pResult = FALSE;
	MV_GETEFFECT* pmg = (MV_GETEFFECT*)phdr;
	if (pmg->hdibSrc) {
		pmg->hdibDst = imgToneCurveEx(pmg->hdibSrc, &m_ti);
		*pResult = TRUE;
	}
}

void CToneCurveDlg::OnSelChange()
{
	int chn = m_combo.GetCurSel();
	TONE* pt;
	if (chn == 0)		pt = &m_ti.rgb;
	else if (chn == 1)	pt = &m_ti.red;
	else if (chn == 2)	pt = &m_ti.green;
	else				pt = &m_ti.blue;

	SetValue(pt->ref[0], pt->val[0]);
}

void CToneCurveDlg::OnReset()
{
	m_ti.rgb.count = 1;
	m_ti.rgb.ref[0] = m_ti.rgb.val[0] = 128;
	m_ti.red.count = 1;
	m_ti.red.ref[0] = m_ti.red.val[0] = 128;
	m_ti.green.count = 1;
	m_ti.green.ref[0] = m_ti.green.val[0] = 128;
	m_ti.blue.count = 1;
	m_ti.blue.ref[0] = m_ti.blue.val[0] = 128;

	m_combo.SetCurSel(0);
	OnSelChange();

	SetValue(128, 128);
	ShowParam(1);
	ShowParam(2);
	ShowParam(3);
}

//////////////////////////////////////////////////////////////////////
//

HANDLE tonecurve_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	if (!hdib) return NULL;

	CToneCurveDlg dlg;
	dlg.m_hdib = hdib;
	if (dlg.DoModal(pfe->hparent) == IDOK)
		return imgToneCurveEx(hdib, &dlg.m_ti);
	return NULL;
}

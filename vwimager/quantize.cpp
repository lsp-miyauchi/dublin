// quantize.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "quantize.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CSpinCtrl	CUpDownCtrl


static int qcolor[] = {
	2, 4, 8, 16, 32, 64, 128, 256
};

static int methods[] = {
	ditherNo, ditherFS, ditherFSLowBleed, ditherFixed
};

/////////////////////////////////////////////////////////////////////////////
//

CQuantizeDlg::CQuantizeDlg()
{
	m_colors = 256;
	m_method = ditherFS;
}

BOOL CQuantizeDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	m_colcombo = GetDlgItem(IDC_COLCOMBO);
	m_metcombo = GetDlgItem(IDC_METHOD);
	InitDlgCtrl(m_colcombo, IDS_QUANTIZE_COLOR);
	InitDlgCtrl(m_metcombo, IDS_QUANTIZE_METHOD);

	m_view = GetDlgItem(IDC_PREVIEW);

	CSpinCtrl spin = GetDlgItem(IDC_SPIN);
	spin.SetRange(2, 256);
	spin.SetPos(256);

	m_colcombo.SetCurSel(m_colcombo.GetCount() - 1);

	int sel;
	if (m_method == ditherNo)
		sel = 0;
	else if (m_method == ditherFS)
		sel = 1;
	else if (m_method == ditherFSLowBleed)
		sel = 2;
	else
		sel = 3;
	m_metcombo.SetCurSel(sel);

	if (!m_hdib) {
		// プレビュー画像が無い場合
		CRect rc, rcw;
		GetWindowRect(&rc);

		m_view.GetWindowRect(&rcw);
		rc.right = rcw.left;
		SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
		m_view.EnableWindow(FALSE);
		GetDlgItem(IDC_ZOOMIN).EnableWindow(FALSE);
		GetDlgItem(IDC_ZOOMOUT).EnableWindow(FALSE);
	}

	UpdatePreview();
	return TRUE;
}

void CQuantizeDlg::OnOK()
{
	TCHAR buf[MAX_PATH];
	m_colcombo.GetWindowText(buf, _COUNTOF(buf));
	m_colors = StrToInt(buf);
	if (m_colors < 2 || m_colors > 256) {
		MessageBeep(0);
		m_colcombo.SetFocus();
		return;
	}
	EndDialog(IDOK);
}

void CQuantizeDlg::UpdatePreview()
{
	if (!m_hdib) return;

	Signal_SetBusy(GetDlgItem(IDC_SIGNAL), TRUE);
	HDIB h = quantize(m_hdib, m_method, m_colors);
	if (h) {
		h = m_view.UpdateImage(h, TRUE);
		if (h) GlobalFree(h);
	}
	Signal_SetBusy(GetDlgItem(IDC_SIGNAL), FALSE);
}

void CQuantizeDlg::OnZoomIn()
{
	m_view.ZoomIn();
}

void CQuantizeDlg::OnZoomOut() 
{
	m_view.ZoomOut();
}

void CQuantizeDlg::OnDither()
{
	int sel = m_metcombo.GetCurSel();
	m_method = methods[sel];
	UpdatePreview();
}

void CQuantizeDlg::OnNumColorEnChange()
{
	TCHAR buf[MAX_PATH];
	m_colcombo.GetWindowText(buf, _COUNTOF(buf));
	m_colors = StrToInt(buf);
	if (m_colors > 1 && m_colors <= 256)
		UpdatePreview();
}

void CQuantizeDlg::OnNumColorChange() 
{
	int sel = m_colcombo.GetCurSel();
	if (sel != CB_ERR) {
		m_colors = qcolor[sel];
		UpdatePreview();
	}
}

void CQuantizeDlg::OnNotifyDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pud = (NM_UPDOWN*)pNMHDR;
	m_colors = pud->iPos;
	UpdatePreview();
	*pResult = 0;
}

void CQuantizeDlg::OnNotifyPreview(NMHDR* phdr, LRESULT* pResult)
{
	ZOOM_SCALE* pzs = (ZOOM_SCALE*)phdr;

	TCHAR buf[MAX_PATH];
	int iScale = (int)(pzs->scale * 1000);
	int s = iScale / 10;
	int m = iScale % 10;
	if (!m)
		wsprintf(buf, _T("%d%%"), s);
	else
		wsprintf(buf, _T("%d.%d%%"), s, m);
	SetDlgItemText(IDC_POWER, buf);
}

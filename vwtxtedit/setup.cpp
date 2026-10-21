////////////////////////////////////////////////////////////
// setup.cpp

#include "stdafx.h"
#include "common.h"
#include "mainfrm.h"
#include "setup.h"

////////////////////////////////////////////////////////////////////////
//

static int snLineDenom[] = { 0, 10, 8, 6, 4, 3, 2 };

BOOL CSetupDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	// フォント情報
	SendDlgItemMessage(IDC_FONT, FPM_SETLOGFONT, 0, (LPARAM)m_vc.Font());
	// TAB変換数
	SetDlgItemInt(IDC_NUMTAB, m_vc.m_tabstep, FALSE);
	// 行番号・改行表示
	if (m_vc.m_sc[scEOL])	CheckDlgButton(IDC_SHOW_CR, TRUE);
	if (m_vc.m_sc[scTAB])	CheckDlgButton(IDC_SHOW_TAB, TRUE);
	if (m_vc.m_sc[scNUM])	CheckDlgButton(IDC_SHOW_LINENUM, TRUE);
	if (m_vc.m_sc[scZSP])	CheckDlgButton(IDC_SHOW_ZEN, TRUE);
	if (m_vc.m_sc[scHSP])	CheckDlgButton(IDC_SHOW_HAN, TRUE);

	// カスタムカラーリスト
	m_list = GetDlgItem(IDC_LIST);
	InitDlgCtrl(m_list, IDS_COLORLIST);
	OnSelChangeColorList();

	COLORREF clr = GetSysColor(COLOR_3DSHADOW);
	SendDlgItemMessage(IDC_SAMPLE, RM_SETEDGECOLOR, 0, clr);
	m_font.CreateFontIndirect(m_vc.Font());
	SendDlgItemMessage(IDC_SAMPLE, WM_SETFONT, (WPARAM)(HFONT)m_font, TRUE);

	// 行の折り返し
	m_wrapline = GetDlgItem(IDC_WRAPLINETYPE);
	InitDlgCtrl(m_wrapline, IDS_WRAPLINES);
	int sel;
	if (m_vc.m_wraptype == WRAP_NONE)
		sel = 0;
	else if (m_vc.m_wraptype == WRAP_WINDOWEDGE)
		sel = 1;
	else {
		sel = 2;
		SetDlgItemInt(IDC_MAXLINECHAR, m_vc.m_wraptype, FALSE);
	}
	m_wrapline.SetCurSel(sel);
	OnSelChangeWrapType();

	// 行間
	sel = 1;
	CString str;
	m_linedenom = GetDlgItem(IDC_LINEDENOM);
	for (int i = 0; i < ARRAYSIZE(snLineDenom); i++) {
		str.Format((snLineDenom[i] == 0) ? _T("%d") : _T("1/%d"), snLineDenom[i]);
		m_linedenom.AddString(str);
		if (snLineDenom[i] == m_vc.m_linedenom)
			sel = i;
	}
	m_linedenom.SetCurSel(sel);
	return TRUE;
}

void CSetupDlg::OnOK()
{
	int n = GetDlgItemInt(IDC_NUMTAB, NULL, FALSE);
	if (n < 1 || n > 8) {
		MessageBeep(0);
		GetDlgItem(IDC_NUMTAB).SetFocus();
		return;
	}
	m_vc.SetTabStep(n);

	// 行番号表示・改行表示
	m_vc.m_sc[scEOL] = IsDlgButtonChecked(IDC_SHOW_CR) ? true : false;
	m_vc.m_sc[scTAB] = IsDlgButtonChecked(IDC_SHOW_TAB) ? true : false;
	m_vc.m_sc[scNUM] = IsDlgButtonChecked(IDC_SHOW_LINENUM) ? true : false;
	m_vc.m_sc[scZSP] = IsDlgButtonChecked(IDC_SHOW_ZEN) ? true : false;;
	m_vc.m_sc[scHSP] = IsDlgButtonChecked(IDC_SHOW_HAN) ? true : false;;

	// フォント
	SendDlgItemMessage(IDC_FONT, FPM_GETLOGFONT, 0, (LPARAM)m_vc.Font());

	// 行折り返し
	n = m_wrapline.GetCurSel();
	if (n == 2) {
		n = GetDlgItemInt(IDC_MAXLINECHAR, NULL, FALSE);
		if (n < 10) {
			GetDlgItem(IDC_MAXLINECHAR).SetFocus();
			MessageBeep(0);
			return;
		}
	}
	else if (n == 0)
		n = WRAP_NONE;
	else
		n = WRAP_WINDOWEDGE;
	m_vc.m_wraptype = n;

	// 行間
	n = m_linedenom.GetCurSel();
	if (n >= 0)
		m_vc.m_linedenom = snLineDenom[n];

	EndDialog(IDOK);
}

void CSetupDlg::OnTextColor()
{
	int idx = -1;
	int sel = m_list.GetCurSel();
	switch (sel) {
	case  0: idx = CLR_TXT;  break;				// テキスト
	case  1: break;								// カーソル行
	case  2: idx = CLR_NUM;  break;				// 行番号
	case  3: idx = CLR_EOL;  break;				// 改行
	case  4: idx = CLR_CTL;  break;				// TAB/全角空白
	case  5: idx = CLR_CMT;  break;				// コメント
	case  6: idx = CLR_KWD1; break;				// キーワード1
	case  7: idx = CLR_KWD2; break;				// キーワード2
	case  8: idx = CLR_KWD3; break;				// キーワード3
	case  9: idx = CLR_KWD4; break;				// キーワード4
	default: break;
	}

	if (idx >= 0) {
		COLORREF clr = SelectColor(IDC_BTN_TEXT, m_vc.m_color[idx]);
		if (clr != (COLORREF)-1 && clr != m_vc.m_color[idx]) {
			m_vc.m_color[idx] = clr;
			UpdateSample();
		}
	}
}

void CSetupDlg::OnBkgndColor()
{
	int idx = -1;
	int sel = m_list.GetCurSel();
	switch (sel) {
	case 0: idx = CLR_BG;  break;				// 背景色
	case 1: idx = CLR_CUR; break;				// カーソル行
	default: break;
	}

	if (idx >= 0) {
		COLORREF clr = SelectColor(IDC_BTN_BKGND, m_vc.m_color[idx]);
		if (clr != (COLORREF)-1 && clr != m_vc.m_color[idx]) {
			m_vc.m_color[idx] = clr;
			UpdateSample();
		}
	}
}

COLORREF CSetupDlg::SelectColor(UINT idCtrl, COLORREF cr)
{
	CRect rc;
	GetDlgItem(idCtrl).GetWindowRect(&rc);
	CPoint pt(rc.left, rc.bottom - 1);
	COLORREF r = cr;
	CClientDC dc(m_hWnd);
	BOOL mode = (dc.GetDeviceCaps(BITSPIXEL) > 8);
	if (!lccColorPicker(m_hWnd, pt, &r, mode))
		r = (COLORREF)-1;
	else {
		if (!mode)
			r = dc.GetNearestColor(r);
	}
	return r;
}

void CSetupDlg::OnSelChangeWrapType()
{
	int sel = m_wrapline.GetCurSel();
	GetDlgItem(IDC_MAXLINECHAR).EnableWindow(sel > 1);
}

void CSetupDlg::OnSelChangeColorList()
{
	int sel = m_list.GetCurSel();
	GetDlgItem(IDC_BTN_TEXT).EnableWindow(sel != 1);
	GetDlgItem(IDC_BTN_BKGND).EnableWindow(sel < 2);
	UpdateSample();
}

void CSetupDlg::UpdateSample()
{
	int b = CLR_BG;
	int sel = m_list.GetCurSel();
	int f = CLR_TXT;
	switch (sel) {
	case  1: f = CLR_TXT;								// カーソル行
           b = CLR_CUR; break;				//
	case  2: f = CLR_NUM; break;				// 行番号
	case  3: f = CLR_EOL; break;				// 改行
	case  4: f = CLR_CTL; break;				// TAB/全角空白
	case  5: f = CLR_CMT; break;				// コメント
	case  6: f = CLR_KWD1; break;				// キーワード1
	case  7: f = CLR_KWD2; break;				// キーワード2
	case  8: f = CLR_KWD3; break;				// キーワード3
	case  9: f = CLR_KWD4; break;				// キーワード4
	case  0:
	default: f = CLR_TXT; break;				// テキスト
	}

	// 配色を適用
	SendDlgItemMessage(IDC_SAMPLE, RM_SETBKCOLOR, 0, m_vc.m_color[b]);
	SendDlgItemMessage(IDC_SAMPLE, RM_SETTEXTCOLOR, 0, m_vc.m_color[f]);
}

void CSetupDlg::OnSelChangeFont()
{
	SendDlgItemMessage(IDC_FONT, FPM_GETLOGFONT, 0, (LPARAM)m_vc.Font());
	m_font.DeleteObject();

	m_font.CreateFontIndirect(m_vc.Font());
	SendDlgItemMessage(IDC_SAMPLE, WM_SETFONT, (WPARAM)(HFONT)m_font, TRUE);
}


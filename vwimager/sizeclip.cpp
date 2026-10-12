// sizeclip.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "sizeclip.h"
#include "common.h"

static SIZE cpair[] = {
	{ 240, 320 },
	{ 240, 400 },
	{ 320, 480 },
	{ 426, 640 },
	{ 480, 640 },
	{ 480, 754 },
	{ 480, 800 },
	{ 480, 854 },
	{ 480, 864 },
	{ 768, 1024 },
	{ 800, 600 },
	{ 1024, 768 },
	{ 1152, 864 },
	{ 1280, 720 },
	{ 1280, 768 },
	{ 1280, 800 },
	{ 1280, 960 },
	{ 1280, 1024 },
	{ 1360, 768 },
	{ 1600, 900 },
	{ 1600, 1024 },
	{ 1680, 1050 },
};

/////////////////////////////////////////////////////////////////////////////
// CSizeClipDlg ダイアログ

BOOL CSizeClipDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_list = GetDlgItem(IDC_LIST);
	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, 100, 0);
	CString str;
	int index = -1;
	for (int i = 0; i < ARRAYSIZE(cpair); i++) {
		str.Format(_T("%d x %d"), cpair[i].cx, cpair[i].cy);
		index = m_list.InsertItem(m_list.GetItemCount(), str, index);
		m_list.SetItemData(index, (DWORD_PTR)MAKELONG(cpair[i].cx, cpair[i].cy));
	}
	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);

	SIZE s;
	if (m_sizeClip.cx == 0 && m_sizeClip.cy == 0) {
		CAppReg reg;
		if (reg.GetBinary(IDS_SEC_MAIN_VIEWER, IDS_KEY_SIZECLIP, &s, sizeof(s)) != sizeof(s)) {
			s.cx = 0;
			s.cy = 0;
		}
	}
	else
		s = m_sizeClip;
	SetDlgItemInt(IDC_XSIZE, s.cx);
	SetDlgItemInt(IDC_YSIZE, s.cy);
	return TRUE;
}

void CSizeClipDlg::OnNotifyItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pNMHDR;
	DWORD val = (DWORD)pnmlv->lParam;
	SetDlgItemInt(IDC_XSIZE, LOWORD(val));
	SetDlgItemInt(IDC_YSIZE, HIWORD(val));
	*pResult = 0;
}

#define pixelInRange(a)		(16<=(a)&&(a)<=4800)

void CSizeClipDlg::OnOK()
{
	m_sizeClip.cx = GetDlgItemInt(IDC_XSIZE, NULL, FALSE);
	m_sizeClip.cy = GetDlgItemInt(IDC_YSIZE, NULL, FALSE);
	if (!pixelInRange(m_sizeClip.cx)) {
		MessageBeep(0);
		GetDlgItem(IDC_XSIZE).SetFocus();
	}
	else if (!pixelInRange(m_sizeClip.cy)) {
		MessageBeep(0);
		GetDlgItem(IDC_YSIZE).SetFocus();
	}
	else {
		CAppReg reg;
		reg.SetBinary(IDS_SEC_MAIN_VIEWER, IDS_KEY_SIZECLIP, (SIZE*)&m_sizeClip, sizeof(m_sizeClip));
		EndDialog(IDOK);
	}
}

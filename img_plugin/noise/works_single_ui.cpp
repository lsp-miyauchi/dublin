//////////////////////////////////////////////////////////////////////
// works_single_ui.cpp

#include "stdafx.h"
#include "ffactory_sup.h"
#include "..\filterdialog.h"

extern PROGRESSCALLBACK pfnProgress;

/////////////////////////////////////////////////////////////////////
//

class CSingleWorksDlg : public CDialogImpl<CSingleWorksDlg>, public CFilterDialogImpl<CSingleWorksDlg>
{
public:
	enum { IDD = IDD_SINGLEWORKS };
	SINGLEWORKS* m_pcw;

BEGIN_MSG_MAP(CSingleWorksDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CSingleWorksDlg>)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_WHOLEVIEW, OnWholeView)
END_MSG_MAP()

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);
	void OnReset();
	void OnWholeView();

	void SetParams(MAGIC* magic);
};

BOOL CSingleWorksDlg::InitDialog(LPARAM lParam)
{
	m_pcw = (SINGLEWORKS*)lParam;
	SetWindowText(m_pcw->magic->fnc_name);

	CTrackCtrl track;
	for (int i = 0; i < MAXPARAMS; i++) {
		track = GetDlgItem(IDC_ASLIDER + i);
		track.SetRange(0, 255, TRUE);
		track.SetPageSize(16);
		track.SetTicFreq(32);
	}

	SetParams(m_pcw->magic);

	if ((m_pcw->preview_mode & PREVIEW_BOTH) != PREVIEW_BOTH) {
		CheckDlgButton(IDC_WHOLEVIEW, (m_pcw->preview_mode & PREVIEW_WHOLE) ? TRUE : FALSE);
		GetDlgItem(IDC_WHOLEVIEW).ShowWindow(SW_HIDE);
	}
	else
		CheckDlgButton(IDC_WHOLEVIEW, m_pcw->wholeview);

	SetWholePreview(m_pcw->wholeview);
	SetPreviewImage(m_pcw->hdib);
	UpdatePreview();
	return TRUE;
}

void CSingleWorksDlg::OnWholeView()
{
	m_pcw->wholeview = IsDlgButtonChecked(IDC_WHOLEVIEW);
	SetWholePreview(m_pcw->wholeview);
	UpdatePreview();
}

void CSingleWorksDlg::SetParams(MAGIC* filter)
{
	int last_valid_control = 0;
	CTrackCtrl track;
	for (int i = 0; i < MAXPARAMS; i++) {
		track = GetDlgItem(IDC_ASLIDER + i);
		if (i < filter->numparams && filter->item[i].label) {
			TCHAR buf[256];
			wsprintf(buf, _T("%s(&%c)"), filter->item[i].label, 'A' + i);
			track.SetWindowText(buf);
			track.SetPos(filter->item[i].val, TRUE);
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

void CSingleWorksDlg::OnReset()
{
	for (int n = 0; n < m_pcw->magic->numparams; n++)
		m_pcw->magic->item[n].val = m_pcw->magic->item[n].def;
	SetParams(m_pcw->magic);

	UpdatePreview();
}

void CSingleWorksDlg::HScroll(int val, HWND hScrollBar)
{
	MAGIC* curfilter = m_pcw->magic;
	int id = ::GetDlgCtrlID(hScrollBar);
	if (id >= IDC_ASLIDER && id < IDC_ASLIDER + curfilter->numparams) {
		id -= IDC_ASLIDER;
		curfilter->item[id].val = val;
		UpdatePreview();
	}
}

HDIB CSingleWorksDlg::GetPreview(HDIB hdib)
{
	mif image(hdib);
	magic_works(image, NULL, m_pcw->magic);
	return image.createdib();
}

/////////////////////////////////////////////////////////////////////
//

static TCHAR keyLast[] = _T("last");

HANDLE single_works_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe, MAGIC* magic, int preview_mode)
{
	int i;

	TCHAR w[128];
	LPTSTR p = w;
	if (HIWORD(pfe->param))
		p = (LPTSTR)pfe->param;
	else
		wsprintf(p, _T("%02d"), pfe->param);

	TCHAR root[128];
	wsprintf(root, _T("noise\\%s\\%s"), p, magic->fnc_name);

	SINGLEWORKS cw;
	regkey reg(pfe->hkey, root);
	if (reg.getbinary(keyLast, &cw, sizeof(cw)) == sizeof(cw)) {
		for (i = 0; i < magic->numparams; i++)
			magic->item[i].val = cw.val[i];
	}

	cw.magic = magic;
	cw.preview_mode = preview_mode;
	if ((cw.preview_mode & PREVIEW_BOTH) != PREVIEW_BOTH)
		cw.wholeview = (cw.preview_mode & PREVIEW_PART) ? FALSE : TRUE;

	mif image(hdib);
	image.crop(prc);
	cw.hdib = image.createdib();
	image.destroy();

	CSingleWorksDlg dlg;
	BOOL bOK = (dlg.DoModal(pfe->hparent, (LPARAM)&cw) == IDOK);
	GlobalFree(cw.hdib);
	if (bOK) {
		for (i = 0; i < magic->numparams; i++)
			cw.val[i] = magic->item[i].val;

		reg.setbinary(keyLast, &cw, sizeof(cw));

		CWaitCursor wc;
pfnProgress = pfe->progress;
		bOK = (image.attachdib(hdib) && magic_works(image, prc, cw.magic));
pfnProgress = NULL;
		if (bOK)
			return image.createdib();
	}
	return NULL;
}


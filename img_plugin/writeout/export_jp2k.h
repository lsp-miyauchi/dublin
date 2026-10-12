// Gamma.h : ヘッダー ファイル
//

#pragma once
#ifndef __EXPORT_JP2K_H__
#define __EXPORT_JP2K_H__

#include "resource.h"

BOOL export_jp2k(HWND hwnd, HDIB hsrc, LPCTSTR pszName);

/////////////////////////////////////////////////////////////////////////////
// CExportJp2kDlg ダイアログ

class CExportJp2kDlg : public CDialogImpl<CExportJp2kDlg>
{
public:
	CExportJp2kDlg();

	enum { IDD = IDD_EXPORT_JP2K };

	HDIB m_hdib;
	JP2_CPARAMS m_jc;
	DWORD m_savedsize;

	CZoomViewCtrl m_view;
	CComboBox m_bps;
	CTrackBarCtrl m_qtracker;

BEGIN_MSG_MAP(CExportJp2kDlg)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_NOTIFY(ZMVN_SCALECHANGED, IDC_PREVIEW, OnNotifyPreview)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ZOOMIN, OnZoomIn)
	ON_BN_CLICKED(IDC_ZOOMOUT, OnZoomOut)
	ON_CBN_SELCHANGE(IDC_BPS, OnBpsChanged)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnNotifyPreview(NMHDR* pNmhdr, LRESULT* pResult);
	void OnZoomIn();
	void OnZoomOut();
	void OnBpsChanged();
	void OnOK() { EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }

	void UpdatePreview();
};


#endif			// __EXPORT_JP2K_H__

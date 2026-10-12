// Gamma.h : ヘッダー ファイル
//

#pragma once
#ifndef __EXPORT_JPEG_H__
#define __EXPORT_JPEG_H__

#include "resource.h"

BOOL export_jpeg(HWND hwnd, HDIB hsrc, LPCTSTR pszName);

/////////////////////////////////////////////////////////////////////////////
// CExportJpegDlg ダイアログ

class CExportJpegDlg : public CDialogImpl<CExportJpegDlg>
{
public:
	CExportJpegDlg();

	enum { IDD = IDD_EXPORT_JPEG };

	HDIB m_hdib;
	JPEG_CPARAMS m_jc;
	DWORD m_savedsize;

	CZoomViewCtrl m_view;
	CComboBox m_bps;
	CTrackBarCtrl m_qtracker;
	CTrackBarCtrl m_stracker;

BEGIN_MSG_MAP(CExportJpegDlg)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_NOTIFY(ZMVN_SCALECHANGED, IDC_PREVIEW, OnNotifyPreview)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ZOOMIN, OnZoomIn)
	ON_BN_CLICKED(IDC_ZOOMOUT, OnZoomOut)
	ON_BN_CLICKED(IDC_JPGSTANDARD, OnFormat)
	ON_BN_CLICKED(IDC_JPGPROGRESSIVE, OnFormat)
	ON_BN_CLICKED(IDC_JPGBASELINE, OnFormat)
	ON_CBN_SELCHANGE(IDC_BPS, OnBpsChanged)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnNotifyPreview(NMHDR* pNmhdr, LRESULT* pResult);
	void OnZoomIn();
	void OnZoomOut();
	void OnBpsChanged();
	void OnFormat();
	void OnOK() { EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }

	void UpdatePreview();
};


#endif			// __EXPORT_JPEG_H__

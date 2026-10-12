// quantize.h : インプリメンテーション ファイル
//

#pragma once
#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

#include "resource.h"


class CQuantizeDlg : public CDialogImpl<CQuantizeDlg>
{
public:
	CQuantizeDlg();

	enum { IDD = IDD_QUANTIZE };

	CComboBox m_colcombo;
	CComboBox m_metcombo;
	CZoomViewCtrl m_view;

	int m_method;
	int m_colors;
	HDIB m_hdib;

BEGIN_MSG_MAP(CQuantizeDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, OnNotifyDeltaPosSpin)
	ON_NOTIFY(ZMVN_SCALECHANGED, IDC_PREVIEW, OnNotifyPreview)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ZOOMIN, OnZoomIn)
	ON_BN_CLICKED(IDC_ZOOMOUT, OnZoomOut)
	ON_CBN_SELCHANGE(IDC_METHOD, OnDither)
	ON_CBN_SELCHANGE(IDC_COLCOMBO, OnNumColorChange)
	ON_CBN_EDITCHANGE(IDC_COLCOMBO, OnNumColorEnChange)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnNotifyDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyPreview(NMHDR* phdr, LRESULT* pResult);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnZoomIn();
	void OnZoomOut();
	void OnDither();
	void OnNumColorEnChange();
	void OnNumColorChange();

	void UpdatePreview();
};


#endif			// __QUANTIZE_H__

// export_gif : ヘッダー ファイル
//

#pragma once
#ifndef __EXPORT_GIF_H__
#define __EXPORT_GIF_H__

#include "resource.h"

BOOL export_gif(HWND hwnd, HDIB hsrc, LPCTSTR pszName);
BOOL export_png(HWND hwnd, HDIB hsrc, LPCTSTR pszName);


/////////////////////////////////////////////////////////////////////////////
// CExportGifDlg ダイアログ

class CExportGifDlg : public CDialogImpl<CExportGifDlg>
{
public:
	CExportGifDlg();
	~CExportGifDlg();

	enum { IDD = IDD_EXPORT_GIF };
	CComboBox m_palcombo;
	CComboBox m_colcombo;
	CComboBox m_metcombo;
	CComboBox m_bps;
	CUpDownCtrl m_spin;
	BOOL m_interlace;

	CZoomViewCtrl m_view;
	CWindow m_palette;
	COLORREF m_xparent;
	int m_xsel[256];
	int m_xselcnt;

	HPALETTE m_hpal;
	int m_method;
	int m_colors;
	DWORD m_savedsize;

	HDIB m_hdib;
	HDIB m_hdibWorker;
	HDIB m_hdibResult;

	UINT m_idCaption;


BEGIN_MSG_MAP(CExportGifDlg)
	ON_WM_INITDIALOG()
	ON_NOTIFY(PLTN_SELCHANGED, IDC_PALETTE, OnNotifySelChanged)
	ON_NOTIFY(ZMVN_SCALECHANGED, IDC_PREVIEW, OnNotifyScaleChanged)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, OnDeltaposSpin)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ZOOMIN, OnZoomIn)
	ON_BN_CLICKED(IDC_ZOOMOUT, OnZoomOut)
	ON_BN_CLICKED(IDC_INTERLACE, OnInterlace)
	ON_CBN_SELCHANGE(IDC_COMBO, OnPalSelchange)
	ON_CBN_SELCHANGE(IDC_METHOD, OnDither)
	ON_CBN_SELCHANGE(IDC_BPS, OnBpsChanged)
	ON_CBN_EDITCHANGE(IDC_EDIT, OnNumColorEnChange)
	ON_CBN_SELCHANGE(IDC_EDIT, OnNumColorChange)
	ON_CBN_SELCHANGE(IDC_XPARENT, OnXparentColorChange)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnPalSelchange();
	void OnZoomIn();
	void OnZoomOut();
	void OnDestroy();
	void OnDither();
	void OnBpsChanged();
	void OnNumColorEnChange();
	void OnNumColorChange();
	void OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult);
	void OnXparentColorChange();
	void OnInterlace();
	void OnNotifySelChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyScaleChanged(NMHDR* phdr, LRESULT* pResult);

	void UpdatePreview();
	void SetFileSize();
	void SetViewImage();
	void ApplyPalette();

	virtual HANDLE MakePreviewImage();
};

class CExportPngDlg : public CExportGifDlg
{
public:
	CExportPngDlg();
	virtual HANDLE MakePreviewImage();
};


#endif				// __EXPORT_GIF_H__

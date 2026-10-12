#if !defined(AFX_ZOOM_H__E5711ECF_DD7E_11D1_8915_00C04FD8DD3F__INCLUDED_)
#define AFX_ZOOM_H__E5711ECF_DD7E_11D1_8915_00C04FD8DD3F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Zoom.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg ダイアログ

class CZoomDlg : public CDialogImpl<CZoomDlg>
{
public:
	CZoomDlg() { }
	~CZoomDlg() { }

	enum { IDD = IDD_ZOOM };

BEGIN_MSG_MAP(CZoomDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ZOOM, OnZoomClicked)
	ON_EN_KILLFOCUS(IDC_XSIZE, OnChangeXsize)
	ON_EN_KILLFOCUS(IDC_YSIZE, OnChangeYsize)
	ON_EN_UPDATE(IDC_XSIZE, OnChangeXsize)
	ON_EN_UPDATE(IDC_YSIZE, OnChangeYsize)
	ON_CBN_EDITUPDATE(IDC_ZVAL, OnChangeZoomValue)
	ON_CBN_SELCHANGE(IDC_ZVAL, OnSelChangeZoomValue)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, OnDeltaposSpin)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnChangeXsize();
	void OnChangeYsize();
	void OnChangeZoomValue();
	void OnSelChangeZoomValue();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnZoomClicked();
	void OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult);

	void ShowSize();

	CUpDownCtrl		m_spin;
	CComboBox		m_zoom;
	CComboBox		m_sample;

	CSize m_sizeSrc;
	CSize m_sizeDst;
	int m_nBitCount;
	int m_nType;
	int m_resample;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_RESIZE_H__E5711ECF_DD7E_11D1_8915_00C04FD8DD3F__INCLUDED_)

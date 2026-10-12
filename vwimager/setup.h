#if !defined(AFX_SETUP_H__0DDF7480_9746_4DB5_BADA_92686ECBDF76__INCLUDED_)
#define AFX_SETUP_H__0DDF7480_9746_4DB5_BADA_92686ECBDF76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// setup.h : ヘッダー ファイル
//

enum {
	STARTUP_SCALE_REAL,
	STARTUP_SCALE_AUTO,
	STARTUP_SCALE_PREV,
};

/////////////////////////////////////////////////////////////////////////////
// CSetupDlg ダイアログ

class CSetupDlg : public CDialogImpl<CSetupDlg>
{
public:
	CSetupDlg() { }
	~CSetupDlg() { }

	enum { IDD = IDD_SETUP };

BEGIN_MSG_MAP(CSetupDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_SHOWGRID, OnShowGrid)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnShowGrid();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	BOOL m_showGrid;
	CSize m_sizeGrid;
	COLORREF m_crGrid;
	COLORREF m_crBack;
	int m_startup_scale;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SETUP_H__0DDF7480_9746_4DB5_BADA_92686ECBDF76__INCLUDED_)

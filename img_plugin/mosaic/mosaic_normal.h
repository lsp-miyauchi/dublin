#if !defined(AFX_MOSAIC_H__855F8538_DC1B_11D1_A49B_0000F4AB9945__INCLUDED_)
#define AFX_MOSAIC_H__855F8538_DC1B_11D1_A49B_0000F4AB9945__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Mosaic.h : ヘッダー ファイル
//


/////////////////////////////////////////////////////////////////////////////
// CMosaicDlg ダイアログ

class CMosaicDlg : public CDialog
{
// コンストラクション
public:
	CMosaicDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CMosaicDlg)
	enum { IDD = IDD_MOSAIC };
	int m_xBlock;
	int m_yBlock;
	//}}AFX_DATA
	CDegreeCtrl m_xslider;
	CDegreeCtrl m_yslider;
	UINT m_nFlags;
	CWnd m_view;
	HDIB m_hdib;

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CMosaicDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual void OnOK();
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CMosaicDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheckChanged();
	//}}AFX_MSG
	void OnNotifyGetEffect(NMHDR* pNmhdr, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MOSAIC_H__855F8538_DC1B_11D1_A49B_0000F4AB9945__INCLUDED_)

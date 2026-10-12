#if !defined(AFX_BLUR_SIMPLE_H__B3F422D0_413A_43D0_ADAE_A085D6546A0B__INCLUDED_)
#define AFX_BLUR_SIMPLE_H__B3F422D0_413A_43D0_ADAE_A085D6546A0B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// blur_simple.h : ヘッダー ファイル
//

HDIB __fastcall blur(HDIB hdib, LPRECT prc, int radius);


/////////////////////////////////////////////////////////////////////////////
// CBlurSimpleDlg ダイアログ

class CBlurSimpleDlg : public CDialog
{
// コンストラクション
public:
	CBlurSimpleDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CBlurSimpleDlg)
	enum { IDD = IDD_BLUR };
	//}}AFX_DATA

	CDegreeCtrl m_slider;
	CWnd m_view;
	int m_radius;
	HDIB m_hdib;


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CBlurSimpleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CBlurSimpleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnOK();
	//}}AFX_MSG
	afx_msg void OnNotifyGetEffect(NMHDR* pNmhdr, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_BLUR_SIMPLE_H__B3F422D0_413A_43D0_ADAE_A085D6546A0B__INCLUDED_)

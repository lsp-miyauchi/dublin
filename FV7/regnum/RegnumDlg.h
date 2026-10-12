// RegnumDlg.h : ヘッダー ファイル
//

#if !defined(AFX_REGNUMDLG_H__726ACF15_16BD_11D1_8817_00C04FD8DD3F__INCLUDED_)
#define AFX_REGNUMDLG_H__726ACF15_16BD_11D1_8817_00C04FD8DD3F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CRegnumDlg dialog

class CRegnumDlg : public CDialog
{
// 構築
public:
	CRegnumDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// Dialog Data
	//{{AFX_DATA(CRegnumDlg)
	enum { IDD = IDD_REGNUM_DIALOG };
	int		m_prefix;
	int		m_seq;
	//}}AFX_DATA

	CDatePickCtrl m_dtp;

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CRegnumDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CRegnumDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnClip();
	afx_msg void OnIsvalid();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_REGNUMDLG_H__726ACF15_16BD_11D1_8817_00C04FD8DD3F__INCLUDED_)

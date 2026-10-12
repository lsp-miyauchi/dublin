// checkarc.h : CCheckArcDlg の宣言

#pragma once

#include "resource.h"       // メイン シンボル
#include <atlhost.h>

enum {
	CHKARC_NG,
	CHKARC_OK,
	CHKARC_UNK
};

typedef struct tagCHKARC {
	BOOL bCanceled;
	BOOL bChecking;
} CHKARC;


#define LVIS_SETSELECT			(LVIS_SELECTED | LVIS_FOCUSED)


// CCheckArcDlg

class CCheckArcDlg : public CDialogImpl<CCheckArcDlg>, public CDialogResize<CCheckArcDlg>
{
public:
	CCheckArcDlg()
	{
		m_hdrop = NULL;
		m_pArcManager = NULL;
	}

	~CCheckArcDlg()
	{
	}

	enum { IDD = IDD_CHECKARCDLG };

BEGIN_MSG_MAP(CCheckArcDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CCheckArcDlg>)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNotifyDblClick)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST, OnNotifyDeleteItem)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CCheckArcDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_START,		DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_COPY,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnStart();
	void OnCopy();
	void OnNotifyDblClick(NMHDR* pHdr, LRESULT* plr);
	void OnNotifyDeleteItem(NMHDR* pHdr, LRESULT* plr);

protected:
	int DoCheck(LPCTSTR pszName, CHAR* pResult, ULONG sizeResult);

public:
	HDROP m_hdrop;
	CArcManager* m_pArcManager;
	CHKARC m_ca;
	CListViewCtrl m_list;
	CSimpleArray<CString> m_array;
};



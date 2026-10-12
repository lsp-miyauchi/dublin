/////////////////////////////////////////////////////////////////////////
// gradlist.h

#pragma once
#ifndef __GRADLIST_H__
#define __GRADLIST_H__

#include "filter.h"
#include "resource.h"


BOOL GradationEdit(HWND hwnd, GRADEDITINFO* pi);


class CGradItem
{
public:
	CGradItem() { m_name = _T(""); m_colors = 0; m_fixed = FALSE; }

	CString m_name;
	int m_colors;
	BOOL m_fixed;
	GRADCOLORS m_grad[MAX_GRADCOLORS];
};

/////////////////////////////////////////////////////////////////////////////
// CGradListDlg ダイアログ

class CGradListDlg : public CDialogImpl<CGradListDlg>
{
public:
	enum { IDD = IDD_GRADLIST };

	BOOL m_bInitialize;
	CListViewCtrl m_list;
	int m_colors;
	GRADCOLORS m_grad[MAX_GRADCOLORS];
	CString m_csPreset;
	HIMAGELIST m_lock;

BEGIN_MSG_MAP(CGradListDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ADD, OnItemAdd)
	ON_BN_CLICKED(IDC_COPY, OnItemCopy)
	ON_BN_CLICKED(IDC_DELETE, OnItemDelete)
	ON_BN_CLICKED(IDC_RENAME, OnItemRename)
	ON_BN_CLICKED(IDC_UPDATE, OnItemUpdate)
	ON_BN_CLICKED(IDC_LOCK, OnItemLock)
	ON_BN_CLICKED(IDC_REVERSE, OnReverseMap)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNotifyClick)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNotifyDblClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnNotifyItemChanged)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST, OnNotifyBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST, OnNotifyEndLabelEdit)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST, OnNotifyDeleteItem)
	ON_CONTROL(GEN_COLORCHANGE, IDC_GRADBAR, OnGradChanged)
END_MSG_MAP()

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnOK() { EndDialog(IDOK); }
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnItemAdd();
	void OnItemCopy();
	void OnItemDelete();
	void OnItemRename();
	void OnItemUpdate();
	void OnItemLock();
	void OnReverseMap();
	void OnGradChanged();
	void OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);

	void BuildNewItemName(LPCTSTR pszPrefix, CString& csNewName);
	void UpdatePreset(LPCTSTR pszOldName, CGradItem* pgi);
};

#endif				// __GRADLIST_H__

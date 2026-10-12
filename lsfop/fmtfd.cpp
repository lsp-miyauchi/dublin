// fmtfd.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "fopstd.h"

//@ extern "C" WINAPI SHFormatDrive(HWND, UINT, UINT, UINT);


/////////////////////////////////////////////////////////////////////////////
// CFormatFDDlg dialog

class CFormatFDDlg : public CDialogImpl<CFormatFDDlg>
{
public:
	CFormatFDDlg() { }
	~CFormatFDDlg() { }

	enum { IDD = IDD_FORMATFD };

BEGIN_MSG_MAP(CFormatFDDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
};

BOOL CFormatFDDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	CListViewCtrl list(GetDlgItem(IDC_LIST));
	// イメージリスト
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	list.SetImageList(himS, LVSIL_SMALL);

	// カラム挿入
	CRect rc;
	list.GetClientRect(&rc);
	list.InsertColumn(0, NULL, LVCFMT_LEFT, rc.Width(), 0);

	// ドライブのリストアップ
	TCHAR buf[MAX_PATH];
	SHFILEINFO sfi;
	for (int i = 0; i < 26; i++) {
		PathBuildRoot(buf, i);
		if (GetDriveType(buf) == DRIVE_REMOVABLE) {
			SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME);
			int idx = list.InsertItem(i, sfi.szDisplayName, sfi.iIcon);
			list.SetItemData(idx, i);
		}
	}
	if (list.GetItemCount() < 1)
		EndDialog(IDCANCEL);
	else
		list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	return TRUE;
}

void CFormatFDDlg::OnOK()
{
	CListViewCtrl list(GetDlgItem(IDC_LIST));
	int sel = list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (sel >= 0) {
		int drive = (int)list.GetItemData(sel);
		if (drive >= 0)
			EndDialog((int)MAKELONG(IDOK, drive));
	}
}

////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI fopFormatDrive(HWND hwndOwner)
{
	CFormatFDDlg dlg;
	INT_PTR nRes = dlg.DoModal(hwndOwner);
	if (LOWORD(nRes) == IDOK)
		SHFormatDrive(hwndOwner, HIWORD(nRes), 0, 0);
}

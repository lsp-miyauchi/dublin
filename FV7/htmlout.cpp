// htmlout.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "fv7.h"
#include "htmlout.h"

/////////////////////////////////////////////////////////////////////////////
// CListOutputDlg メッセージ ハンドラ

BOOL CListOutputDlg::OnInitDialog(LPARAM lParam) 
{
	CenterWindow();

	// 前回の設定
	LISTOUTPARAMS lop;
	if (GetRegBinary(IDS_SEC_LISTOUTPUT, IDS_KEY_LASTSELECTION, &lop, sizeof(lop)) == sizeof(lop)) {
		m_nOutputType = lop.nOutputType;
		m_bLaunchApp = lop.bLaunchApp;
		m_bListAll = lop.bListAll;
	}

	// 出力形式
	CComboBoxEx combo = GetDlgItem(IDC_COMBO);

	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	combo.SetImageList(himS);

	// 出力リストタイプの登録
	CSimpleArray<CString> arr;
	int cnt = LoadTokenizedString(IDS_LISTOUTPUT_TYPES, _T(";"), arr);
	SHFILEINFO sfi;
	COMBOBOXEXITEM cbi;
	for (int i = 0; i < cnt; i++) {
		int pos = arr[i].ReverseFind(_T('.'));
		CString s = arr[i].Left(pos);
		cbi.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT;
		cbi.iItem = i;
		cbi.pszText = (LPTSTR)s.GetString();
		SHGetFileInfo(arr[i].GetString() + pos, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
		cbi.iImage = cbi.iSelectedImage = sfi.iIcon;
		combo.InsertItem(&cbi);
	}
	combo.SetCurSel(m_nOutputType);

	CheckDlgButton(IDC_ALLITEM, m_bListAll);
	CheckDlgButton(IDC_RUN, m_bLaunchApp);
	return TRUE;
}

void CListOutputDlg::OnOK()
{
	TCHAR buf[MAX_PATH * 2], w[MAX_PATH * 2];
	if (GetDlgItemText(IDC_PATH, w, ARRAYSIZE(w)) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_PATH).SetFocus();
		return;
	}

	lccPathBuild(w, m_strPath, buf);
	m_strPath = buf;

	CComboBox combo = GetDlgItem(IDC_COMBO);
	m_nOutputType = combo.GetCurSel();

	m_bListAll = IsDlgButtonChecked(IDC_ALLITEM);
	m_bLaunchApp = IsDlgButtonChecked(IDC_RUN);

	// 設定を保存
	LISTOUTPARAMS lop;
	lop.nOutputType = m_nOutputType;
	lop.bLaunchApp = m_bLaunchApp;
	lop.bListAll = m_bListAll;
	SetRegBinary(IDS_SEC_LISTOUTPUT, IDS_KEY_LASTSELECTION, &lop, sizeof(lop));

	EndDialog(IDOK);
}

void CListOutputDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	CString str;
	if (FileBrowse(TRUE, m_hWnd, IDS_FILTER_ALLFILES, str, NULL)) {
		lstrcpyn(psgt->pszText, str, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

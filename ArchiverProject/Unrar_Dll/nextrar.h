//////////////////////////////////////////////////////////////////////
// nextrar.h

#pragma once
#ifndef __NEXTRAR_H__
#define __NEXTRAR_H__

//////////////////////////////////////////////////////////////////////
//

class CNextRarDlg : public CDialogImpl<CNextRarDlg>
{
public:
	CStringW m_vol;
	CStringW m_name;

	CNextRarDlg() { }
	~CNextRarDlg() { }

	enum { IDD = IDD_NEXTRAR };

BEGIN_MSG_MAP(CNextRarDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		MessageBeep(0);
		SetDlgItemText(IDC_RARFILE, PathFindFileName(m_vol));
		return TRUE;
	}

	void OnOK()
	{
		if (GetDlgItemText(IDC_PATH, m_name) < 1 || !PathFileExists(m_name)) {
			MessageBeep(0);
			GetDlgItem(IDC_PATH).SetFocus();
		}
		EndDialog(IDOK);
	}
	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}
	void OnBrowse()
	{
		WCHAR dir[MAX_PATH * 2], rar[MAX_PATH * 2], flt[MAX_PATH * 2];
		lstrcpy(dir, m_vol);
		PathRemoveFileSpec(dir);
		lstrcpyW(flt, L"*.rar|*.rar|*.*|*.*||");
		if (BrowseFile(flt, dir, rar))
			SetDlgItemText(IDC_PATH, rar);
	}

	BOOL BrowseFile(LPWSTR pszFilter, LPWSTR pszDir, LPWSTR pszRar)
	{
		WCHAR name[MAX_PATH * 2], title[MAX_PATH * 2];
		name[0] = title[0] = 0;
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrInitialDir = pszDir;
		ofn.lpstrDefExt = NULL;
		ofn.lpstrFile = name;
		ofn.nMaxFile = ARRAYSIZE(name);
		ofn.lpstrFileTitle = title;
		ofn.nMaxFileTitle = ARRAYSIZE(title);
		ofn.Flags = OFN_HIDEREADONLY;
		if ((BYTE)GetVersion() >= 4)
			ofn.Flags |= OFN_EXPLORER;

		ofn.lpstrTitle = L"ボリューム ファイルの参照";
		WCHAR* p = pszFilter;
		while ((p = wcschr(p, '|')) != NULL)
			*p++ = 0;
		ofn.lpstrFilter = pszFilter;
		if (GetOpenFileName(&ofn)) {
			wcscpy(pszRar, ofn.lpstrFile);
			return TRUE;
		}
		return FALSE;
	}
};


#endif		// __NEXTRAR_H__

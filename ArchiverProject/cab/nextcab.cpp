#include "stdafx.h"
#include "cab.h"


//////////////////////////////////////////////////////////////////////
//

typedef struct tagNEXTCAB {
	WCHAR szPath[MAX_PATH * 2];
	WCHAR szCab[MAX_PATH * 2];
} NEXTCAB;
typedef NEXTCAB* LPNEXTCAB;


//////////////////////////////////////////////////////////////////////
//

class CNextCabDlg : public CDialogImpl<CNextCabDlg>
{
public:
	CNextCabDlg() : m_pnc(NULL)
	{
	}
	~CNextCabDlg() { }

	enum { IDD = IDD_SFXCFG };

BEGIN_MSG_MAP(CNextCabDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		MessageBeep(0);
		m_pnc = (LPNEXTCAB)lParam;
		SetDlgItemText(IDC_CABFILE, m_pnc->szCab);
		SetDlgItemText(IDC_PATH, m_pnc->szPath);
		return TRUE;
	}

	void OnOK()
	{
		WCHAR cab[MAX_PATH * 2], buf[MAX_PATH * 2];
		if (GetDlgItemText(IDC_PATH, buf, ARRAYSIZE(buf)) > 0) {
			PathCombine(cab, buf, m_pnc->szCab);
		}
		else
			wcscpy(cab, m_pnc->szCab);

		if (!PathFileExists(cab)) {
			MessageBeep(0);
			GetDlgItem(IDC_PATH).SetFocus();
		}
		else {
			wcscpy(m_pnc->szPath, buf);
			EndDialog(IDOK);
		}
	}

	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}

	void OnBrowse()
	{
		WCHAR dir[MAX_PATH * 2], cab[MAX_PATH * 2], flt[MAX_PATH * 2];
		GetDlgItemText(IDC_PATH, dir, ARRAYSIZE(dir));
		swprintf(flt, L"%s|%s||", m_pnc->szCab, m_pnc->szCab);
		if (BrowseFile(flt, dir, cab)) {
			WCHAR* p = PathFindFileName(cab);
			if (!_wcsicmp(p, m_pnc->szCab)) {
				wcscpy(dir, cab);
				PathRemoveFileSpec(dir);
				SetDlgItemText(IDC_PATH, dir);
				return;
			}
			MessageBeep(0);
		}
	}

	BOOL BrowseFile(LPWSTR pFilter, LPWSTR pDir, LPWSTR pCab)
	{
		WCHAR name[MAX_PATH * 2], title[MAX_PATH * 2];
		name[0] = title[0] = 0;

		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrInitialDir = pDir;
		ofn.lpstrDefExt = NULL;
		ofn.lpstrFile = name;
		ofn.nMaxFile = MAX_PATH * 2;
		ofn.lpstrFileTitle = title;
		ofn.nMaxFileTitle = ARRAYSIZE(title);
		ofn.Flags = OFN_HIDEREADONLY;
		ofn.Flags |= OFN_EXPLORER;

		ofn.lpstrTitle = L"キャビネット ファイルの参照";
		WCHAR* p = pFilter;
		while ((p = wcschr(p, '|')) != NULL)
			*p++ = 0;
		ofn.lpstrFilter = pFilter;

		if (GetOpenFileName(&ofn)) {
			wcscpy(pCab, ofn.lpstrFile);
			return TRUE;
		}
		return FALSE;
	}

	NEXTCAB* m_pnc;
};

//////////////////////////////////////////////////////////////////////
//

BOOL FindNextCab(HWND hwnd, LPWSTR pszCabFile)
{
	NEXTCAB nc;

	WCHAR* p = PathFindFileName(pszCabFile);
	if (p) {
		wcscpy(nc.szPath, pszCabFile);
		PathRemoveFileSpec(nc.szPath);
	}
	else {
		nc.szPath[0] = 0;
		p = pszCabFile;
	}
	wcscpy(nc.szCab, p);

	CNextCabDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&nc) != IDOK)
		return FALSE;

	PathCombine(pszCabFile, nc.szPath, nc.szCab);
	return TRUE;
}

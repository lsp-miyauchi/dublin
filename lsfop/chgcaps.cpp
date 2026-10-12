#include "stdafx.h"
#include "fopstd.h"

/////////////////////////////////////////////////////////////////////////////
//

enum {
	CCP_TOUPPER,
	CCP_TOLOWER,
	CCP_CAPSUPPER,
};

#define CCPS_EXTONLY		1
#define CCPS_SUBFOLDER		2
#define CCPS_EXCFOLDER		4

typedef struct tagCAPSDATA {
	int type;
	BOOL hasfolder;
	DWORD stat;
} CAPSDATA;


//////////////////////////////////////////////////////////////////////
//

class CChgCapsDlg : public CDialogImpl<CChgCapsDlg>
{
public:
	CChgCapsDlg() { m_pcd = NULL; }
	~CChgCapsDlg() { }

	enum { IDD = IDD_CHGCAPS };

BEGIN_MSG_MAP(CChgCapsDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	CAPSDATA* m_pcd;
};

//////////////////////////////////////////////////////////////////////
//

BOOL CChgCapsDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pcd = (CAPSDATA*)lParam;

	if (!m_pcd->hasfolder) {
		GetDlgItem(IDC_SUBFOLDER).EnableWindow(FALSE);
		GetDlgItem(IDC_EXCFOLDER).EnableWindow(FALSE);
	}
	else
		CheckDlgButton(IDC_SUBFOLDER, TRUE);

	CheckDlgButton(IDC_TOLOWER, TRUE);
	return TRUE;
}

BOOL CChgCapsDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CChgCapsDlg::OnOK()
{
	if (IsDlgButtonChecked(IDC_TOUPPER))		m_pcd->type = CCP_TOUPPER;
	else if (IsDlgButtonChecked(IDC_TOLOWER))	m_pcd->type = CCP_TOLOWER;
	else										m_pcd->type = CCP_CAPSUPPER;

	m_pcd->stat = 0;
	if (IsDlgButtonChecked(IDC_EXTONLY))		m_pcd->stat |= CCPS_EXTONLY;
	if (IsDlgButtonChecked(IDC_SUBFOLDER))		m_pcd->stat |= CCPS_SUBFOLDER;
	if (IsDlgButtonChecked(IDC_EXCFOLDER))		m_pcd->stat |= CCPS_EXCFOLDER;

	EndDialog(IDOK);
}

//////////////////////////////////////////////////////////////////////
//

static BOOL change_caps_file(LPCTSTR src, int type, DWORD stat)
{
	if (!_pathfileexists(src)) return FALSE;
	BOOL bDirectory = _pathisdirectory(src);
	if (bDirectory && (stat & CCPS_EXCFOLDER)) return TRUE;

	TCHAR dst[MAX_PATH * 2];
	lstrcpy(dst, src);
	LPTSTR spec = PathFindFileName(dst);
	if (stat & CCPS_EXTONLY) {
		spec = StrRChr(spec, NULL, '.');
		if (!spec) return TRUE;
	}

	if (type == CCP_TOUPPER)
		CharUpper(spec);
	else if (type == CCP_TOLOWER)
		CharLower(spec);
	else {
		CharLower(spec);
		CharUpperBuff(spec, 1);
	}

	if (!_movefile(src, dst)) {
		LastError(IDS_TITLE_RENAME, PathFindFileName(src));
		return FALSE;
	}
	SHChangeNotify(bDirectory ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM,
				SHCNF_PATH | SHCNF_FLUSHNOWAIT, src, dst);
	return TRUE;
}

static BOOL change_caps_tree(LPCTSTR pszPath, int type, DWORD stat)
{
	BOOL bOK = FALSE;
	if (stat & CCPS_SUBFOLDER) {
		// ファイルを検索
		CSimpleArray<CString> arrFile;
		CSimpleArray<CString> arrFolder;
		EnumFileDir(pszPath, gszAst, arrFile, arrFolder);

		TCHAR buf[MAX_PATH * 2];

		// まずファイルを先に削除
		int i, cnt = (int)arrFile.GetSize();
		for (i = 0; i < cnt; i++) {
			PathCombine(buf, pszPath, arrFile[i]);
			if (!change_caps_file(buf, type, stat)) return FALSE;
		}
		arrFile.RemoveAll();

		// 次にフォルダを削除
		cnt = (int)arrFolder.GetSize();
		for (i = 0; i < cnt; i++) {
			PathCombine(buf, pszPath, arrFolder[i]);
			if (!change_caps_tree(buf, type, stat)) return FALSE;
		}
		arrFolder.RemoveAll();
	}
	return change_caps_file(pszPath, type, stat);
}

extern "C"
BOOL WINAPI fopChangeFileCaps(HWND hwnd, HDROP hDrop)
{
	CFileArray array;
	TCHAR buf[MAX_PATH * 2];
	int cnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	int i;
	for (i = 0; i < cnt; i++) {
		DragQueryFile(hDrop, i, buf, ARRAYSIZE(buf));
		array.AddFile(buf);
	}

	if (array.GetCount() == 0) return FALSE;

	CAPSDATA cd;
	cd.hasfolder = array.IsFolderInclude();

	BOOL bOK = FALSE;
	CChgCapsDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&cd) == IDOK) {
		CWaitCursor wc;

		CFileInfo* pf;
		cnt = (int)array.GetCount();
		for (i = 0; i < cnt; i++) {
			pf = array[i];
			if (pf->IsFolder())
				bOK = change_caps_tree(pf->m_strFullPathName, cd.type, cd.stat);
			else
				bOK = change_caps_file(pf->m_strFullPathName, cd.type, cd.stat);
			if (!bOK)
				break;
		}
	}
	array.DeleteAll();
	return bOK;
}

//////////////////////////////////////////////////////////////////////
// chkdsk.cpp

#include "stdafx.h"
#include "lsmedia.h"
#include "nfresize.h"
#include "fmifs.h"

class CCheckDiskDlg : public CDialogImpl<CCheckDiskDlg>, public CNoFlickerDialogResize<CCheckDiskDlg>
{
public:
	CCheckDiskDlg();
	~CCheckDiskDlg();

	enum { IDD = IDD_CHKDSK };

	TCHAR m_szDrive[MAX_PATH];
	TCHAR m_szFileSystem[MAX_PATH];
	TCHAR m_szOlddir[MAX_PATH * 2];
	BOOL m_bFixErrors;
	BOOL m_bVerbose;
	BOOL m_bSkipClean;
	BOOL m_bScanSectors;
	HMODULE m_hdll;
	PCHKDSK m_pfn;
	BOOL m_bInProgress;
	BOOL m_bCanceled;
	BOOL m_bError;
	HIMAGELIST m_hImage;

BEGIN_MSG_MAP(CCheckDiskDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CNoFlickerDialogResize<CCheckDiskDlg>)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CCheckDiskDlg)
	DLGRESIZE_CONTROL(IDC_GROUP_STATIC,	DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_METER,		DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_TEXT,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT pdi);
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT pmi);
	void OnOK();
	void OnCancel();

	static BOOL __stdcall Callback(CALLBACKCOMMAND cmd, DWORD mod, PVOID pva);
};

CCheckDiskDlg::CCheckDiskDlg()
{
	m_szDrive[0] = 0;
	m_szFileSystem[0] = 0;
	m_szOlddir[0] = 0;
	m_bFixErrors = FALSE;
	m_bVerbose = FALSE;
	m_bSkipClean = FALSE;
	m_bScanSectors = FALSE;
	m_bInProgress = FALSE;
	m_bCanceled = FALSE;
	m_bError = FALSE;
	m_pfn = NULL;

	GetCurrentDirectory(MAX_PATH * 2, m_szOlddir);

	m_hdll = LoadLibrary(_T("fmifs.dll"));
	if (m_hdll) {
		HIMAGELIST dummy;
		Shell_GetImageLists(&dummy, &m_hImage);
		(FARPROC&)m_pfn = GetProcAddress(m_hdll, "Chkdsk");
	}
}

CCheckDiskDlg::~CCheckDiskDlg()
{
	if (m_hdll) FreeLibrary(m_hdll);
	SetCurrentDirectory(m_szOlddir);
}

BOOL CCheckDiskDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();
	CenterWindow();

	CWindow meter = GetDlgItem(IDC_METER);
	meter.SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	meter.SendMessage(PBM_SETPOS, 0, 0);

	CComboBox combo = GetDlgItem(IDC_COMBO);
	TCHAR buf[MAX_PATH];
	lstrcpy(buf, _T("x:\\"));

	SHFILEINFO sfi;
	TCHAR ww[MAX_PATH];
	int index, type, startdrive = 0;
	for (int i = 0; i < 26; i++) {
		buf[0] = i + 'A';
		type = GetDriveType(buf);
		switch (type) {
			case DRIVE_REMOVABLE:
			case DRIVE_FIXED:
			case DRIVE_RAMDISK:
				SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME);
				lstrcpy(ww, sfi.szDisplayName);
				index = combo.AddString(ww);
				combo.SetItemData(index, MAKELONG(i, sfi.iIcon));
				if (i == tolower(m_szOlddir[0]) - 'a')
					startdrive = index;
				break;

			default:
				break;
		}
	}
	combo.SetCurSel(startdrive);
	return TRUE;
}

BOOL __stdcall CCheckDiskDlg::Callback(CALLBACKCOMMAND cmd, DWORD mod, PVOID pva)
{
	static CCheckDiskDlg* pThis = NULL;
	BOOL* status;

	// We get other types of commands, but we don't have to pay attention to them
	switch (cmd) {
		case PROGRESS:
			if (pThis->IsWindow()) {
				DWORD* pper = (DWORD*)pva;
				pThis->SendDlgItemMessage(IDC_METER, PBM_SETPOS, *pper, 0);
				ZapMessage(pThis->m_hWnd);
			}
			break;

		case OUTPUT:
			if (pThis->IsWindow()) {
				TEXTOUTPUT* pto = (TEXTOUTPUT*)pva;

				int len = lstrlenA(pto->Output);
				TCHAR* psz = new TCHAR[len * 2];
				MultiByteToWideChar(CP_ACP, 0, pto->Output, -1, psz, len * 2);
				pThis->SendDlgItemMessage(IDC_TEXT, LGM_ADDLOG, 0, (LPARAM)psz);
				delete [] psz;
				ZapMessage(pThis->m_hWnd);
			}
			break;

		case DONE:
			status = (BOOL*)pva;
			if (*status == TRUE) pThis->m_bError = TRUE;

			if (pThis->IsWindow()) {
				DWORD* pper = (DWORD*)pva;
				pThis->SendDlgItemMessage(IDC_METER, PBM_SETPOS, 0, 0);
			}
			AppMessage(IDS_TITLE_CHKDSK,
						pThis->m_bError ? IDS_ERR_CHKDSK : IDS_OK_CHKDSK,
						pThis->m_szDrive,
						pThis->m_bError ? MB_STOP : MB_ATTENTION);
			break;

		case 1128:
			if (mod == 1128) {
				pThis = (CCheckDiskDlg*)pva;
				pThis->SendDlgItemMessage(IDC_METER, PBM_SETPOS, 0, 0);
			}
			break;
	}
	return TRUE;
}

void CCheckDiskDlg::OnOK()
{
	CComboBox combo = GetDlgItem(IDC_COMBO);
	int drive = combo.GetCurSel();
	if (drive < 0) return;

	drive = LOWORD(combo.GetItemData(drive));
	wsprintf(m_szDrive, _T("%C:\\"), drive + 'A');

	m_bFixErrors = IsDlgButtonChecked(IDC_FIX);
	m_bVerbose = FALSE;
	m_bScanSectors = IsDlgButtonChecked(IDC_RECOVER);
	m_bSkipClean = FALSE;

	DWORD mc, fsf;
	TCHAR sys[64];
	if (!GetVolumeInformation(m_szDrive, NULL, 0, NULL, &mc, &fsf, sys, ARRAYSIZE(sys))) {
		AppMessage(IDS_TITLE_CHKDSK, IDS_ERR_VOLUMEINFO, m_szDrive);
		return;
	}

	if (m_bFixErrors) {
		TCHAR buf[MAX_PATH];
		wsprintf(buf, _T("\\\\.\\%C:"), drive + 'A');
		HANDLE hf = CreateFile(buf, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
		if (hf == INVALID_HANDLE_VALUE) {
			AppMessage(IDS_TITLE_CHKDSK, IDS_ERR_DRIVESHARE, m_szDrive);
			return;
		}
		CloseHandle(hf);
	}
	SendDlgItemMessage(IDC_TEXT, LGM_CLEARLOG, 0, 0);

	combo.EnableWindow(FALSE);
	combo.UpdateWindow();
	GetDlgItem(IDC_FIX).EnableWindow(FALSE);
	GetDlgItem(IDC_RECOVER).EnableWindow(FALSE);
	GetDlgItem(IDOK).EnableWindow(FALSE);
	GetDlgItem(IDCANCEL).EnableWindow(FALSE);

	TCHAR buf[MAX_PATH], mes[MAX_PATH];
	LoadString(ghInst, IDS_FILESYSTEM, buf, ARRAYSIZE(buf));
	wsprintf(mes, buf, drive + 'A', sys);
	SendDlgItemMessage(IDC_TEXT, LGM_ADDLOG, 0, (LPARAM)mes);

	m_bError = FALSE;
	m_bInProgress = TRUE;
	m_bCanceled = FALSE;

	Callback((CALLBACKCOMMAND)1128, 1128, (void*)this);
	(*m_pfn)(m_szDrive, sys, m_bFixErrors, m_bVerbose, m_bSkipClean,
					m_bScanSectors, NULL, NULL, CCheckDiskDlg::Callback);

	m_bInProgress = FALSE;

	combo.EnableWindow(TRUE);
	combo.UpdateWindow();
	GetDlgItem(IDC_FIX).EnableWindow(TRUE);
	GetDlgItem(IDC_RECOVER).EnableWindow(TRUE);
	GetDlgItem(IDOK).EnableWindow(TRUE);
	GetDlgItem(IDCANCEL).EnableWindow(TRUE);

	GetDlgItem(IDOK).SetFocus();
}

void CCheckDiskDlg::OnCancel()
{
	if (!m_bInProgress) {
		EndDialog(IDCANCEL);
		return;
	}
	m_bCanceled = TRUE;
}

void CCheckDiskDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT pdi)
{
	CComboBox combo = pdi->hwndItem;
	CDCHandle dc = pdi->hDC;
	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		if (!combo.IsWindowEnabled()) {
			dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
			dc.SetBkColor(GetSysColor(COLOR_WINDOW));
		}
		else {
			if (pdi->itemState & ODS_SELECTED) {
				dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
				dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
			}
			else {
				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
				dc.SetBkColor(GetSysColor(COLOR_WINDOW));
			}
		}

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);

		TCHAR buf[MAX_PATH];
		combo.GetLBText(pdi->itemID, buf);

		dc.ExtTextOut(pdi->rcItem.left + 16 + 4,
					(pdi->rcItem.top + pdi->rcItem.bottom - tm.tmHeight) / 2,
					ETO_OPAQUE | ETO_CLIPPED, &pdi->rcItem, buf, lstrlen(buf), NULL);
		ImageList_Draw(m_hImage, HIWORD(pdi->itemData),
				dc, pdi->rcItem.left + 2,
				(pdi->rcItem.top + pdi->rcItem.bottom - 16) / 2, ILD_TRANSPARENT);
	}
	if (pdi->itemAction & ODA_FOCUS)
		dc.DrawFocusRect(&pdi->rcItem);
}

void CCheckDiskDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT pmi)
{
	CClientDC dc(m_hWnd);

	HFONT hFont = GetFont();
	HFONT hfOld = NULL;
	if (hFont) hfOld = dc.SelectFont(hFont);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	pmi->itemHeight = max(16, tm.tmHeight) + 2;

	if (hfOld) dc.SelectFont(hfOld);
}

extern "C"
void WINAPI fmCheckDisk(HWND hwnd)
{
/* --------- Windows 95/98/Me
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(osv);
	GetVersionEx(&osv);
	if (osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		TCHAR cmd[MAX_PATH * 2], dir[MAX_PATH * 2];
		GetCurrentDirectory(ARRAYSIZE(dir), dir);
		if (dir[0] == '\\')
			GetWindowsDirectory(dir, ARRAYSIZE(dir));
		lstrcpy(cmd, dir);
		if (lstrlen(cmd) > 2 && cmd[1] == ':' && cmd[2] == '\\')
			cmd[2] = '\0';
		else
			cmd[0] = '\0';

		ShellExecute(hwnd, NULL, _T("scandskw.exe"), cmd, dir, SW_SHOWNORMAL);
		return;
	}
------ */

	CCheckDiskDlg dlg;
	if (dlg.m_pfn) dlg.DoModal(hwnd);
}

extern "C"
void WINAPI RunCheckDiskW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	if (lpCmdLine && PathIsDirectory(lpCmdLine) && !PathIsUNC(lpCmdLine))
		SetCurrentDirectory(lpCmdLine);
	fmCheckDisk(hwnd);
}

extern "C"
void WINAPI RunCheckDisk(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR wz[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, wz, ARRAYSIZE(wz));
	RunCheckDiskW(hwnd, hInst, wz, nCmdShow);
}

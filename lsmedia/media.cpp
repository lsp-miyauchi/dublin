// media.cpp : implementation file
//

#include "stdafx.h"
#include "lsmedapi.h"
#include "lsmedia.h"
#include "relay.h"

typedef struct tagMKIMAGE {
	int nDrive;
	TCHAR szImage[MAX_PATH];

	// 復元用パラメタ
	BOOL bRestore;
	int nNumOfIssue;
	BOOL bFormat;
	BOOL bTypeMatched;
	DISK_GEOMETRY dg;
} MKIMAGE;
typedef MKIMAGE* LPMKIMAGE;

static BOOL CALLBACK llf_Relay(UINT iPart, LPARAM lParam);
static BOOL lfmRestoreImage(HWND hwnd, LPMKIMAGE pmi);
static TCHAR sszBrowseFilters[] = _T("Disk images(fdi,img,dmp)\0*.fdi;*.img;*.dmp\0All files(*.*)\0*.*\0\0");


enum {
	MCR_OK,
	MCR_ERROR,
	MCR_MEDIA_NOMATCH,
	MCR_DEVICE_ERROR
};

#define DUMMY_VAL	20

/////////////////////////////////////////////////////////////////////////////
// MediaInfoDlg dialog

class CMediaInfoDlg : public CDialogImpl<CMediaInfoDlg>
{
public:
	enum { IDD = IDD_MEDIAINFO };

BEGIN_MSG_MAP(CMediaInfoDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnCancel() { EndDialog(IDCANCEL); }
};

BOOL CMediaInfoDlg::OnInitDialog(LPARAM lParam)
{
	int nDrive = (int)lParam;
	HANDLE hVolume = fmOpenVolume((int)lParam, 0);
	if (hVolume == INVALID_HANDLE_VALUE) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	TCHAR buf[MAX_PATH];
	GetDlgItem(IDC_DRIVEGROUP).GetWindowText(buf, ARRAYSIZE(buf));
	CString cs = buf;
	wsprintf(buf, cs, nDrive + 'A');
	GetDlgItem(IDC_DRIVEGROUP).SetWindowText(buf);

	DISK_GEOMETRY dg[8];
	if (fmGetDiskGeometry(hVolume, &dg[0])) {
		cs.LoadString(IDS_MEDIA_BASE + dg[0].MediaType);
		SetDlgItemText(IDC_MEDIATYPE, cs);
		if (dg[0].MediaType != Unknown) {
			SetDlgItemInt(IDC_CYLINDER, dg[0].Cylinders.LowPart, FALSE);
			SetDlgItemInt(IDC_TRACK, dg[0].TracksPerCylinder, FALSE);
			SetDlgItemInt(IDC_SECTOR, dg[0].SectorsPerTrack, FALSE);
			SetDlgItemInt(IDC_BYTE, dg[0].BytesPerSector, FALSE);
		}
	}

	int nEntry = fmGetMediaTypes(hVolume, dg, sizeof(dg));
	if (nEntry > 0) {
		CListBox list = GetDlgItem(IDC_GEOMETRY);
		CString cs;
		for (int i = 0; i < nEntry; i++) {
			cs.LoadString(IDS_MEDIA_BASE + dg[i].MediaType);
			list.AddString(cs);
		}
	}
	fmCloseVolume(hVolume);
	return TRUE;
}

extern "C"
void WINAPI fmMediaInfo(HWND hwnd, int nDrive)
{
	CMediaInfoDlg dlg;
	dlg.DoModal(hwnd, nDrive);
}

/////////////////////////////////////////////////////////////////////////////
// combo drive list drawing

static BOOL AddDriveItem(HWND hCombo)
{
	CComboBox combo = hCombo;
	TCHAR buf[MAX_PATH];
	for (int i = 0; i < 26; i++) {
		PathBuildRoot(buf, i);
		if (GetDriveType(buf) == DRIVE_REMOVABLE) {
			int index = combo.AddString((LPCTSTR)DUMMY_VAL);
			combo.SetItemData(index, (DWORD)i);
		}
	}
	if (combo.GetCount() < 1) return FALSE;

	combo.SetCurSel(0);
	return TRUE;
}

static void MeasureDriveItem(HWND hwnd, int nIDCtl, LPMEASUREITEMSTRUCT pmi)
{
	CClientDC dc(hwnd);
	HFONT hFont = (HFONT)::SendMessage(hwnd, WM_GETFONT, 0, 0);
	HFONT hfOld = NULL;
	if (hFont) hfOld = dc.SelectFont(hFont);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	if (hfOld) dc.SelectFont(hfOld);

	pmi->itemHeight = max(16, tm.tmHeight) + 2;
}

static void DrawDriveItem(HWND hwnd, int nIDCtl, LPDRAWITEMSTRUCT pdi)
{
	CDCHandle dc = pdi->hDC;
	CRect rc = pdi->rcItem;
	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		if (pdi->itemState & ODS_SELECTED) {
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
		}
		else {
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			dc.SetBkColor(GetSysColor(COLOR_WINDOW));
		}

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);

		TCHAR buf[MAX_PATH];
		PathBuildRoot(buf, (int)pdi->itemData);
		SHFILEINFO sfi;
		HIMAGELIST him = (HIMAGELIST)SHGetFileInfo(buf, 0, &sfi, sizeof(sfi),
								SHGFI_SYSICONINDEX | SHGFI_SMALLICON |
								SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
		dc.ExtTextOut(rc.left + 16 + 4, (rc.top + rc.bottom - tm.tmHeight) / 2, ETO_OPAQUE | ETO_CLIPPED,
					&rc, sfi.szDisplayName, lstrlen(sfi.szDisplayName), NULL);
		ImageList_Draw(him, sfi.iIcon, dc, rc.left + 2, (rc.top + rc.bottom - 16) / 2, ILD_TRANSPARENT);
	}
	if (pdi->itemAction & ODA_FOCUS)
		dc.DrawFocusRect(&rc);
}

/////////////////////////////////////////////////////////////////////////////
// MakeImageDlg dialog

class CMakeImageDlg : public CDialogImpl<CMakeImageDlg>
{
public:
	enum { IDD = IDD_MAKEIMAGE };
	MKIMAGE* m_pmi;

	CMakeImageDlg() : m_pmi(NULL) { }

BEGIN_MSG_MAP(CMakeImageDlg)
	ON_WM_INITDIALOG()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDrawItem(int nIDCtl, DRAWITEMSTRUCT* pdi) { DrawDriveItem(m_hWnd, nIDCtl, pdi); }
	void OnMeasureItem(int nIDCtl, MEASUREITEMSTRUCT* pmi) { MeasureDriveItem(m_hWnd, nIDCtl, pmi); }
	void OnHelpCall();
	void OnBrowse();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	int CheckImage();
};

BOOL CMakeImageDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pmi = (MKIMAGE*)lParam;

	if (!AddDriveItem(GetDlgItem(IDC_LIST))) {
		AppMessage(IDS_TITLE_MAKEIMAGE, IDS_NOREMOVABLE_DISK, NULL, MB_STOP);
		EndDialog(IDCANCEL);
	}

	// オートコンプリート
	SHAutoComplete(GetDlgItem(IDC_EDIT), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYSTEM);
	return TRUE;
}

void CMakeImageDlg::OnHelpCall()
{
	UINT nRM = RegisterWindowMessage(_T("wm_fv6helpreq"));
	GetParent().SendMessage(nRM, IDM_DISK_MAKEIMAGE, 0);
}

void CMakeImageDlg::OnBrowse()
{
	TCHAR buf[MAX_PATH];
	buf[0] = '\0';
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = sszBrowseFilters;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = ARRAYSIZE(buf);
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_EXPLORER | OFN_OVERWRITEPROMPT;
	if (GetSaveFileName(&ofn)) {
		if (!PathFindExtension(buf)) {
			lstrcat(buf, _T(".fdi"));
		}
		SetDlgItemText(IDC_EDIT, buf);
	}
}

int CMakeImageDlg::CheckImage()
{
	m_pmi->bTypeMatched = FALSE;

	if (GetDlgItemText(IDC_EDIT, m_pmi->szImage, ARRAYSIZE(m_pmi->szImage)) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_EDIT).SetFocus();
		return MCR_ERROR;
	}

	CComboBox combo = GetDlgItem(IDC_LIST);
	int sel = combo.GetCurSel();
	if (sel == CB_ERR) {
		MessageBeep(0);
		combo.SetFocus();
		return MCR_ERROR;
	}
	m_pmi->nDrive = (int)combo.GetItemData(sel);
	return MCR_OK;
}

void CMakeImageDlg::OnOK()
{
	if (CheckImage() != MCR_OK) return;
	EndDialog(IDOK);
}

/////////////////////////////////////////////////////////////////////////////
// RestoreImageDlg dialog

class CRestoreImageDlg : public CDialogImpl<CRestoreImageDlg>
{
public:
	enum { IDD = IDD_RESTOREIMAGE };
	MKIMAGE* m_pmi;

	CRestoreImageDlg() : m_pmi(NULL) { }

BEGIN_MSG_MAP(CRestoreImageDlg)
	ON_WM_INITDIALOG()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDrawItem(int nIDCtl, DRAWITEMSTRUCT* pdi) { DrawDriveItem(m_hWnd, nIDCtl, pdi); }
	void OnMeasureItem(int nIDCtl, MEASUREITEMSTRUCT* pmi) { MeasureDriveItem(m_hWnd, nIDCtl, pmi); }
	void OnHelpCall();
	void OnBrowse();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	int CheckImage();
};

BOOL CRestoreImageDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pmi = (MKIMAGE*)lParam;

	if (!AddDriveItem(GetDlgItem(IDC_LIST))) {
		AppMessage(IDS_TITLE_MAKEIMAGE, IDS_NOREMOVABLE_DISK, NULL, MB_STOP);
		EndDialog(IDCANCEL);
		return TRUE;
	}

	CheckDlgButton(IDC_DOFORMAT, TRUE);
	CUpDownCtrl spin = GetDlgItem(IDC_SPIN);
	spin.SetRange(1, 100);
	spin.SetPos(1);

	// オートコンプリート
	SHAutoComplete(GetDlgItem(IDC_EDIT), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYSTEM);

	return TRUE;
}

void CRestoreImageDlg::OnHelpCall()
{
	UINT nRM = RegisterWindowMessage(_T("wm_fv6helpreq"));
	GetParent().SendMessage(nRM, IDM_DISK_RESTOREIMAGE, 0);
}

void CRestoreImageDlg::OnBrowse()
{
	TCHAR buf[MAX_PATH];
	buf[0] = '\0';
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = sszBrowseFilters;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = ARRAYSIZE(buf);
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_EXPLORER | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn)) {
		SetDlgItemText(IDC_EDIT, buf);
		CheckImage();
	}
}

int CRestoreImageDlg::CheckImage()
{
	m_pmi->bTypeMatched = FALSE;

	WIN32_FIND_DATA wfd;
	if (GetDlgItemText(IDC_EDIT, m_pmi->szImage, ARRAYSIZE(m_pmi->szImage)) < 1 ||
		!GetFileData(m_pmi->szImage, &wfd))
	{
		MessageBeep(0);
		GetDlgItem(IDC_EDIT).SetFocus();
		return MCR_ERROR;
	}

	CComboBox combo = GetDlgItem(IDC_LIST);
	int sel = combo.GetCurSel();
	if (sel == CB_ERR) {
		MessageBeep(0);
		combo.SetFocus();
		return MCR_ERROR;
	}
	m_pmi->nDrive = (int)combo.GetItemData(sel);

	CWaitCursor wc;

	TCHAR szDrive[MAX_PATH];
	PathBuildRoot(szDrive, m_pmi->nDrive);

	int nRes = MCR_ERROR;
	HANDLE hVolume = fmOpenVolume(m_pmi->nDrive, GENERIC_READ | GENERIC_WRITE);
	if (hVolume != INVALID_HANDLE_VALUE) {
		if (fmLockVolume(hVolume)) {
			DISK_GEOMETRY dgCur, dgEnum[4];
			if (fmGetDiskGeometry(hVolume, &dgCur)) {
				int nEntry = fmGetMediaTypes(hVolume, dgEnum, sizeof(DISK_GEOMETRY) * 4);
				if (nEntry > 0) {
					DWORD dwVirtSize, NumBufs;
					BOOL bOK = FALSE;
					int i;
					for (i = 0; i < nEntry; i++) {
						NumBufs = dgEnum[i].Cylinders.LowPart;
						dwVirtSize = dgEnum[i].TracksPerCylinder *
												dgEnum[i].SectorsPerTrack *
												dgEnum[i].BytesPerSector;
						if (dwVirtSize * NumBufs >= wfd.nFileSizeLow)
							break;
					}
					if (i != nEntry) {
						CString cs;
						cs.LoadString(IDS_MEDIA_BASE + dgEnum[i].MediaType);
						SetDlgItemText(IDC_SRCPROP, cs);
						cs.LoadString(IDS_MEDIA_BASE + dgCur.MediaType);
						SetDlgItemText(IDC_DSTPROP, cs);

						m_pmi->bTypeMatched = (dgEnum[i].MediaType == dgCur.MediaType);
						CopyMemory(&m_pmi->dg, &dgEnum[i], sizeof(DISK_GEOMETRY));
						nRes = MCR_OK;
					}
					else
						nRes = MCR_MEDIA_NOMATCH;
				}
				else
					nRes = MCR_MEDIA_NOMATCH;
			}
			else {
				LastError(IDS_TITLE_RESTOREIMAGE, szDrive);
				nRes = MCR_DEVICE_ERROR;
			}
			fmUnlockVolume(hVolume);
		}
		else {
			LastError(IDS_TITLE_RESTOREIMAGE, szDrive);
			nRes = MCR_DEVICE_ERROR;
		}
		fmCloseVolume(hVolume);
	}
	else {
		LastError(IDS_TITLE_RESTOREIMAGE, szDrive);
		nRes = MCR_DEVICE_ERROR;
	}
	return nRes;
}

void CRestoreImageDlg::OnOK()
{
	if (CheckImage() != MCR_OK) return;

	m_pmi->nNumOfIssue = GetDlgItemInt(IDC_NUMBER, NULL, FALSE);
	m_pmi->bFormat = IsDlgButtonChecked(IDC_DOFORMAT);
	if (!m_pmi->bFormat && !m_pmi->bTypeMatched) {
		AppMessage(IDS_TITLE_RESTOREIMAGE, IDS_MEDIA_NOT_MATCHED);
		return;
	}
	EndDialog(IDOK);
}

////////////////////////////////////////////////////////////////
// MEDIA Functions

extern "C"
BOOL WINAPI fmRestoreImage(HWND hwnd, int nDrive, LPCTSTR lpImage, BOOL bFormat)
{
	return FALSE;

/* @@@@
	TCHAR szDrive[MAX_PATH];
	PathBuildRoot(szDrive, nDrive);

	// Open and Lock the drive
	HANDLE hVolume = fmOpenVolume(nDrive, GENERIC_READ | GENERIC_WRITE);
	if (hVolume == INVALID_HANDLE_VALUE)
	{
		LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
		return FALSE;
	}

	if (!fmLockVolume(hVolume))
	{
		LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	DISK_GEOMETRY dgCur, dgEnum[4];
	if (!fmGetDiskGeometry(hVolume, &dgCur))
	{
		LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	int nEntry = fmGetMediaTypes(hVolume, dgEnum, sizeof(DISK_GEOMETRY) * 4);
	if (nEntry == 0)
	{
		LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	// Open the disk image file
	HANDLE hDiskImage = CreateFile(lpImage, GENERIC_READ,
									0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDiskImage == INVALID_HANDLE_VALUE)
	{
		LastError(IDS_TITLE_RESTOREIMAGE, lpImage, MB_STOP);
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	// If we are copying from floppy to file, just do the copy
	// Otherwise, we might have to format the floppy first

	// Check to see if the image will fit on the floppy. If it
	// will, then LowLevelFormat the floppy and press on

	DWORD dwRead, dwWrite;
	DWORD dwVirtSize;
	DWORD NumBufs;

	DWORD dwSize = GetFileSize(hDiskImage, NULL);
	BOOL bOK = FALSE;
	for (int i = 0; i < nEntry; i++) {
		NumBufs = dgEnum[i].Cylinders.LowPart;
		dwVirtSize = dgEnum[i].TracksPerCylinder *
							  dgEnum[i].SectorsPerTrack *
							  dgEnum[i].BytesPerSector;
		if (dwVirtSize * NumBufs >= dwSize)
			break;
	}
	if (i == nEntry) {
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		CloseHandle(hDiskImage);

		TCHAR buf[MAX_PATH];
		wsprintf(buf, _T("サイズ %d はドライブ %s 上でサポートされていません."), dwSize, szDrive);
		::MessageBox(hwnd, buf, _T("ディスクイメージの復元"), MB_STOP);
		return FALSE;
	}

	HWND hRelay = NULL;
	if (bFormat) {
		hRelay = ry_Create(hwnd);
		ry_InitBar(hRelay, dgEnum[i].Cylinders.LowPart, IDB_FORMAT, 36);
		CStr cs;
		cs.LoadString(ghInst, IDS_TITLE_FORMATFD);
		ry_SetCaption(hRelay, cs);
		cs.LoadString(ghInst, IDS_TEXT_FORMATFD);
		ry_SetFile(hRelay, cs);

		bOK = fmLowLevelFormat(hVolume, &dgEnum[i], llf_Relay, (LPARAM)hRelay);
		if (!bOK) {
			ry_Close(hRelay);
			fmUnlockVolume(hVolume);
			fmCloseVolume(hVolume);
			CloseHandle(hDiskImage);
			return FALSE;
		}
	}

	LPVOID lpvIO = VirtualAlloc(NULL, dwVirtSize, MEM_COMMIT, PAGE_READWRITE);

	if (!hRelay)
		hRelay = ry_Create(hwnd);
	ry_InitBar(hRelay, NumBufs, (gdwComCtlVersion < VERSION_XP) ? IDB_RESTOREIMAGE : IDB_XP_RESTOREIMAGE, 36);
	ry_SetFile(hRelay, lpImage);

	CStr cs;
	cs.LoadString(ghInst, IDS_TITLE_RESTOREIMAGE);
	ry_SetCaption(hRelay, cs);
	FmtStr2(ghInst, cs, IDS_TEXT_RESTOREIMAGE, lpImage, szDrive);
	ry_SetFile(hRelay, cs);

	for (;NumBufs > 0; NumBufs--) {
		if (ry_IsCanceled(hRelay)) {
			bOK = FALSE;
			break;
		}

		bOK = ReadFile(hDiskImage, lpvIO, dwVirtSize, &dwRead, NULL);
		if (bOK && dwRead) {
			ry_SetComplete(hRelay, 1);

			bOK = WriteFile(hVolume, lpvIO, dwRead, &dwWrite, NULL);
			if (!bOK || (dwRead != dwWrite)) {
				LastError(IDS_TITLE_RESTOREIMAGE, lpImage, MB_STOP);
				bOK = FALSE;
				break;
			}
		}
		else {
			LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
			bOK = FALSE;
			break;
		}
	}
	ry_Close(hRelay);

	VirtualFree(lpvIO, 0, MEM_DECOMMIT | MEM_RELEASE);

	// Dismounting forces the filesystem to re-evaluate the media id
	// and geometry. This is the same as popping the floppy in and out
	// of the disk drive

	fmDismountVolume(hVolume);
	fmUnlockVolume(hVolume);
	fmCloseVolume(hVolume);
	CloseHandle(hDiskImage);
	return bOK;
@@@@ */
}

static BOOL lfmRestoreImage(HWND hwnd, LPMKIMAGE pmi)
{
	TCHAR szDrive[MAX_PATH];
	PathBuildRoot(szDrive, pmi->nDrive);

	// Open and Lock the drive
	HANDLE hVolume = fmOpenVolume(pmi->nDrive, GENERIC_READ | GENERIC_WRITE);
	if (hVolume == INVALID_HANDLE_VALUE)
	{
		LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
		return FALSE;
	}

	if (!fmLockVolume(hVolume))
	{
		LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	// Open the disk image file
	HANDLE hDiskImage = CreateFile(pmi->szImage, GENERIC_READ,
									0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDiskImage == INVALID_HANDLE_VALUE)
	{
		LastError(IDS_TITLE_RESTOREIMAGE, pmi->szImage, MB_STOP);
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	// If we are copying from floppy to file, just do the copy
	// Otherwise, we might have to format the floppy first

	// Check to see if the image will fit on the floppy. If it
	// will, then LowLevelFormat the floppy and press on

	BOOL bOK = FALSE;
	DWORD dwCylinders = pmi->dg.Cylinders.LowPart;
	DWORD dwVirtSize = pmi->dg.TracksPerCylinder *
							pmi->dg.SectorsPerTrack *
							pmi->dg.BytesPerSector;

	HWND hRelay = NULL;
/* @@@@
	if (pmi->bFormat)
	{
		hRelay = ry_Create(hwnd);
		ry_InitBar(hRelay, pmi->dg.Cylinders.LowPart, IDB_FORMAT, 36);
		CStr cs;
		cs.LoadString(ghInst, IDS_TITLE_FORMATFD);
		ry_SetCaption(hRelay, cs);
		cs.LoadString(ghInst, IDS_TEXT_FORMATFD);
		ry_SetFile(hRelay, cs);

		bOK = fmLowLevelFormat(hVolume, &pmi->dg, llf_Relay, (LPARAM)hRelay);
		if (!bOK)
		{
			ry_Close(hRelay);
			fmUnlockVolume(hVolume);
			fmCloseVolume(hVolume);
			CloseHandle(hDiskImage);
			return FALSE;
		}
	}
@@@@ */

	LPVOID lpvIO = VirtualAlloc(NULL, dwVirtSize, MEM_COMMIT, PAGE_READWRITE);

	if (!hRelay)
		hRelay = ry_Create(hwnd);
	ry_InitBar(hRelay, dwCylinders, (gdwComCtlVersion < VERSION_XP) ? IDB_RESTOREIMAGE : IDB_XP_RESTOREIMAGE, 36);
	ry_SetFile(hRelay, pmi->szImage);

	CString cs;
	cs.LoadString(IDS_TITLE_RESTOREIMAGE);
	ry_SetCaption(hRelay, cs);
	cs.LoadString(IDS_TEXT_RESTOREIMAGE);
	cs.Replace(_T("%1"), pmi->szImage);
	cs.Replace(_T("%2"), szDrive);
	ry_SetFile(hRelay, cs);

	DWORD dwRead, dwWrite;
	for (DWORD dw = 0; dw < dwCylinders; dw++) {
		if (ry_IsCanceled(hRelay)) {
			bOK = FALSE;
			break;
		}

		if (pmi->bFormat) {
			bOK = FormatCylinder(hVolume, &pmi->dg, (int)dw);
			if (!bOK) {
				LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
				break;
			}
		}

		bOK = ReadFile(hDiskImage, lpvIO, dwVirtSize, &dwRead, NULL);
		if (bOK && dwRead) {
			ry_SetComplete(hRelay, 1);

			bOK = WriteFile(hVolume, lpvIO, dwRead, &dwWrite, NULL);
			if (!bOK || (dwRead != dwWrite)) {
				LastError(IDS_TITLE_RESTOREIMAGE, pmi->szImage, MB_STOP);
				bOK = FALSE;
				break;
			}
		}
		else {
			LastError(IDS_TITLE_RESTOREIMAGE, szDrive, MB_STOP);
			bOK = FALSE;
			break;
		}
	}
	ry_Close(hRelay);

	VirtualFree(lpvIO, 0, MEM_DECOMMIT | MEM_RELEASE);

	// Dismounting forces the filesystem to re-evaluate the media id
	// and geometry. This is the same as popping the floppy in and out
	// of the disk drive

	fmDismountVolume(hVolume);
	fmUnlockVolume(hVolume);
	fmCloseVolume(hVolume);
	CloseHandle(hDiskImage);
	return bOK;
}

extern "C"
BOOL WINAPI fmRestoreImageUI(HWND hwnd)
{
	MKIMAGE mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.nDrive = -1;
	mi.szImage[0] = '\0';
	mi.bRestore = TRUE;

	CRestoreImageDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&mi) != IDOK)
		return FALSE;

	TCHAR szDrive[MAX_PATH];
	PathBuildRoot(szDrive, mi.nDrive);

	BOOL bOK = FALSE;
	int i = 0;
	while (i++ < mi.nNumOfIssue) {
		if (!lfmRestoreImage(hwnd, &mi)) return FALSE;

		if (i < mi.nNumOfIssue) {
			if (AppMessage(IDS_TITLE_RESTOREIMAGE, IDS_NEXT_MEDIA, szDrive, MB_QOKCANCEL) == IDCANCEL)
				break;
		}
	}
	return TRUE;
}

extern "C"
BOOL WINAPI fmMakeImage(HWND hwnd, int nDrive, LPCTSTR lpImage)
{
	TCHAR szDrive[MAX_PATH];
	PathBuildRoot(szDrive, nDrive);

	// Open and Lock the drive
	HANDLE hVolume = fmOpenVolume(nDrive, GENERIC_READ | GENERIC_WRITE);
	if (hVolume == INVALID_HANDLE_VALUE) {
		LastError(IDS_TITLE_MAKEIMAGE, szDrive, MB_STOP);
		return FALSE;
	}

	if (!fmLockVolume(hVolume)) {
		LastError(IDS_TITLE_MAKEIMAGE, szDrive, MB_STOP);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	DISK_GEOMETRY dgCur, dgEnum[4];
	if (!fmGetDiskGeometry(hVolume, &dgCur)) {
		LastError(IDS_TITLE_MAKEIMAGE, szDrive, MB_STOP);
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	int nEntry = fmGetMediaTypes(hVolume, dgEnum, sizeof(DISK_GEOMETRY)*4);
	if (nEntry == 0) {
		LastError(IDS_TITLE_MAKEIMAGE, szDrive, MB_STOP);
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	// Open the disk image file
	HANDLE hDiskImage = CreateFile(lpImage, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hDiskImage == INVALID_HANDLE_VALUE) {
		LastError(IDS_TITLE_MAKEIMAGE, lpImage, MB_STOP);
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		return FALSE;
	}

	// If we are copying from floppy to file, just do the copy
	// Otherwise, we might have to format the floppy first

	// Device reads must be sector aligned. VirtualAlloc will
	// garuntee alignment

	DWORD dwCylinders = dgCur.Cylinders.LowPart;
	DWORD dwVirtSize = dgCur.TracksPerCylinder * dgCur.SectorsPerTrack * dgCur.BytesPerSector;

	LPVOID lpvIO = VirtualAlloc(NULL, dwVirtSize, MEM_COMMIT, PAGE_READWRITE);
	if (!lpvIO) {
		LastError(IDS_TITLE_MAKEIMAGE, NULL, MB_STOP);
		fmUnlockVolume(hVolume);
		fmCloseVolume(hVolume);
		CloseHandle(hDiskImage);
		return FALSE;
	}

	HWND hRelay = ry_Create(hwnd);
	ry_InitBar(hRelay, dwCylinders, (gdwComCtlVersion < VERSION_XP) ? IDB_MAKEIMAGE : IDB_XP_MAKEIMAGE, 36);
	ry_SetFile(hRelay, lpImage);

	CString cs;
	cs.LoadString(IDS_TITLE_MAKEIMAGE);
	ry_SetCaption(hRelay, cs);
	cs.Replace(_T("%1"), szDrive);
	cs.Replace(_T("%2"), lpImage);
	ry_SetFile(hRelay, cs);

	DWORD dwRead, dwWrite;
	BOOL bOK = TRUE;
	for ( ;dwCylinders > 0; dwCylinders--) {
		BOOL bOK = ReadFile(hVolume, lpvIO, dwVirtSize, &dwRead, NULL);
		if (bOK) {
			ry_SetComplete(hRelay, 1);
			if (!dwRead)
				break;

			bOK = WriteFile(hDiskImage,lpvIO, dwRead, &dwWrite, NULL);
			if (!bOK || (dwRead != dwWrite)) {
				LastError(IDS_TITLE_MAKEIMAGE, NULL, MB_STOP);
				bOK = FALSE;
				break;
			}
		}
		else {
			LastError(IDS_TITLE_MAKEIMAGE, szDrive, MB_STOP);
			bOK = FALSE;
			break;
		}
		if (ry_IsCanceled(hRelay))
			break;
	}
	ry_Close(hRelay);

	VirtualFree(lpvIO, 0, MEM_DECOMMIT | MEM_RELEASE);

	// Dismounting forces the filesystem to re-evaluate the media id
	// and geometry. This is the same as popping the floppy in and out
	// of the disk drive

	fmDismountVolume(hVolume);
	fmUnlockVolume(hVolume);
	fmCloseVolume(hVolume);
	CloseHandle(hDiskImage);
	return TRUE;
}

extern "C"
BOOL WINAPI fmMakeImageUI(HWND hwnd)
{
	MKIMAGE mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.nDrive = -1;
	mi.szImage[0] = '\0';

	CMakeImageDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&mi) != IDOK)
		return FALSE;

	return fmMakeImage(hwnd, mi.nDrive, mi.szImage);
}

static BOOL CALLBACK llf_Relay(UINT iPart, LPARAM lParam)
{
	HWND hRelay = (HWND)lParam;
	ry_SetComplete(hRelay, 1);
	if (ry_IsCanceled(hRelay))
		return FALSE;
	return TRUE;
}

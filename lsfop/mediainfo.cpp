/////////////////////////////////////////////////////////////////////////////
// MediaInfo dialog

#include "stdafx.h"
#include "fopstd.h"

///////////////////////////////////////////////////////////////////////
//

class CMediaInfoDlg : public CDialogImpl<CMediaInfoDlg>
{
public:
	enum { IDD = IDD_MEDIAINFO };

BEGIN_MSG_MAP(CMediaInfoDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		int nDrive = (int)lParam;
		CString str;
		str.Format(_T("\\\\.\\%c:"), nDrive + 'A');
		HANDLE hvol = CreateFile(str, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hvol == INVALID_HANDLE_VALUE) {
			EndDialog(IDCANCEL);
			return TRUE;
		}

		GetDlgItem(IDC_DRIVEGROUP).GetWindowText(str);
		TCHAR buf[MAX_PATH];
		wsprintf(buf, str, nDrive + 'A');
		GetDlgItem(IDC_DRIVEGROUP).SetWindowText(buf);

		DISK_GEOMETRY dg[8];
		DWORD dw;
		if (DeviceIoControl(hvol, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg[0], sizeof(DISK_GEOMETRY), &dw, NULL)) {
			str.LoadString(IDS_MEDIA_BASE + dg[0].MediaType);
			SetDlgItemText(IDC_MEDIATYPE, str);
			if (dg[0].MediaType != Unknown) {
				SetDlgItemInt(IDC_CYLINDER, dg[0].Cylinders.LowPart, FALSE);
				SetDlgItemInt(IDC_TRACK, dg[0].TracksPerCylinder, FALSE);
				SetDlgItemInt(IDC_SECTOR, dg[0].SectorsPerTrack, FALSE);
				SetDlgItemInt(IDC_BYTE, dg[0].BytesPerSector, FALSE);
			}
		}

		if (DeviceIoControl(hvol, IOCTL_DISK_GET_MEDIA_TYPES, NULL, 0, dg, sizeof(dg), &dw, NULL) && dw > 0) {
			int cnt = (int)(dw / sizeof(DISK_GEOMETRY));
			if (cnt > 0) {
				CListBox list = GetDlgItem(IDC_GEOMETRY);
				for (int i = 0; i < cnt; i++) {
					str.LoadString(IDS_MEDIA_BASE + dg[i].MediaType);
					list.AddString(str);
				}
			}
		}

		CloseHandle(hvol);
		return TRUE;
	}

	void OnCancel() { EndDialog(IDCANCEL); }
};

////////////////////////////////////////////////////////////////////////
//

void ShowMediaInfo(HWND hwnd, int nDrive)
{
	CMediaInfoDlg dlg;
	dlg.DoModal(hwnd, nDrive);
}



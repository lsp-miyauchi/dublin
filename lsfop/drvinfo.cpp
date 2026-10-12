// drvinfo.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"
#include "drivemedia.h"

typedef struct tagDRVINFO {
	int drive;
	BOOL removableonly;
	DWORDLONG free_size;
	DWORDLONG total_size;
} DRVINFO;

typedef struct tagDRVHEAD {
	LPTSTR pszText;
	int fmt;
	int cx;
} DRVHEAD;

static DRVHEAD cold[17] = {
	{ _T("Drive"),			LVCFMT_LEFT,	160 },		// 0
	{ _T("Type"),			LVCFMT_LEFT,	46 },		// 1
	{ _T("Free"),			LVCFMT_RIGHT,	58 },		// 2
	{ _T("Total"),			LVCFMT_RIGHT,	58 },		// 3
	{ _T("Used"),			LVCFMT_RIGHT,	38 },		// 4
	{ _T("Sector"),			LVCFMT_RIGHT,	68 },		// 5
	{ _T("Cluster"),		LVCFMT_RIGHT,	68 },		// 6
	{ _T("Label"),			LVCFMT_LEFT,	48 },		// 7
	{ _T("Network"),		LVCFMT_LEFT,	80 },		// 8
	{ _T("Serial"),			LVCFMT_LEFT,	70 },		// 9
	{ _T("Cylinders"),		LVCFMT_RIGHT,	68 },		// 10
	{ _T("Tracks/Cylinder"),LVCFMT_RIGHT,	68 },		// 11
	{ _T("Sectors/Track"),	LVCFMT_RIGHT,	68 },		// 12
	{ _T("VenderID"),		LVCFMT_LEFT,	70 },		// 13
	{ _T("ProductID"),		LVCFMT_LEFT,	70 },		// 14
	{ _T("Revision#"),		LVCFMT_LEFT,	70 },		// 15
	{ _T("Interface"),		LVCFMT_LEFT,	70 },		// 16
};

typedef struct tagDRIVETYPES {
	int id;
	LPWSTR tit;
} DRIVETYPES;

static DRIVETYPES stDriveTypes[] = {
	0, L"ハードディスク ドライブ",
	1, L"取り外し可能 ドライブ",
	2, L"ネットワーク ドライブ",
	3, L"その他"
};


/////////////////////////////////////////////////////////////////////////////
// CDriveInfoDlg dialog

class CDriveInfoDlg : public CDialogImpl<CDriveInfoDlg>, public CDialogResize<CDriveInfoDlg>
{
private:
	bool m_bInitCompleted;
public:
	DRVINFO m_di;

	CDriveInfoDlg() : m_bInitCompleted(false) { }
	~CDriveInfoDlg() { }

	enum { IDD = IDD_DRIVEINFO };

BEGIN_MSG_MAP(CDriveInfoDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CDriveInfoDlg>)
	ON_WM_HELPINFO()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChanged)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_WM_DESTROY()
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CDriveInfoDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnItemChanged(LPNMHDR pnmh, LRESULT* pResult);
	void OnCancel() { EndDialog(IDCANCEL); }
	void SetDriveInfo(int nIndex, LPCTSTR pszRoot);
	void OnDestroy();
};

BOOL CDriveInfoDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();
	CenterWindow();
	SendDlgItemMessage(IDC_DRVMETER, MM_SETTEXTCOLOR, 0, RGB(0,0,128));
	SendDlgItemMessage(IDC_DRVMETER, MM_SETBKCOLOR,   0, RGB(128,128,128));

	CListViewCtrl list(GetDlgItem(IDC_LIST));
	list.SetExtendedListViewStyle(list.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	list.SetImageList(himS, LVSIL_SMALL);
	list.EnableGroupView(TRUE);

	int maxColumn = 17;
	int i;
	for (i = 0; i < maxColumn; i++)
		list.InsertColumn(i, cold[i].pszText, cold[i].fmt, cold[i].cx, i);

	LVGROUP group;
	memset(&group, 0, sizeof(group));
	group.cbSize = sizeof(group);
	group.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
	group.stateMask = LVGS_NORMAL | LVGS_COLLAPSIBLE;
	group.state = LVGS_NORMAL | LVGS_COLLAPSIBLE;
	group.uAlign = LVGA_HEADER_LEFT | LVGA_FOOTER_LEFT;

	for (i = 0; i < 4; i++) {
		group.iGroupId = stDriveTypes[i].id;
		group.pszHeader = stDriveTypes[i].tit;
		group.cchHeader = lstrlenW(group.pszHeader);
		list.InsertGroup(i, &group);
	}
	TCHAR root[MAX_PATH];
	SHFILEINFO sfi;
	int cnt = 0, idx, sel = -1;
	DWORD dwDrives = GetLogicalDrives();
	LVITEM item;
	item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_GROUPID;
	item.iSubItem = 0;
	item.state = 0;
	item.stateMask = 0;
	for (i = 0; i < 26; i++) {
		if (dwDrives & 1) {
			PathBuildRoot(root, i);
			SHGetFileInfo(root, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME);

			UINT type = GetDriveType(root);
			if (!m_di.removableonly || type == DRIVE_REMOVABLE) {
				item.iItem = cnt++;
				item.pszText = sfi.szDisplayName;
				item.iImage = sfi.iIcon;
				switch (type) {
					case DRIVE_FIXED:		item.iGroupId = stDriveTypes[0].id; break;
					case DRIVE_REMOTE:		item.iGroupId = stDriveTypes[2].id; break;
					case DRIVE_REMOVABLE:
					case DRIVE_CDROM:		item.iGroupId = stDriveTypes[1].id; break;
					default:				item.iGroupId = stDriveTypes[3].id; break;
				}
				idx = list.InsertItem(&item);
				list.SetItemData(idx, i);
				if (type != DRIVE_REMOVABLE)
					SetDriveInfo(idx, root);
				if (i == m_di.drive)
					sel = idx;
			}
		}
		dwDrives = dwDrives >> 1;
	}

	for (i = 0; i < maxColumn; i++) {
		if (i != 8) {
			// ネットワークパス以外
			list.SetColumnWidth(i, LVSCW_AUTOSIZE);
		}
	}

	if (sel >= 0)
		list.SetItemState(sel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	// 位置復帰
	WINDOWPLACEMENT wpm;
	if (GetRegBinary(_MKIR(IDS_SEC_WNDPLACE), _MKIR(IDS_KEY_WP_DRIVEINFO), (LPBYTE)&wpm, sizeof(wpm)) == sizeof(wpm)) {
		wpm.showCmd = SW_SHOWNORMAL;
		SetWindowPlacement(&wpm);
	}

	m_bInitCompleted = true;
	return TRUE;
}

BOOL CDriveInfoDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CDriveInfoDlg::OnItemChanged(LPNMHDR pnmh, LRESULT* pResult)
{
	if (m_bInitCompleted) {
		NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)pnmh;
		TCHAR buf[MAX_PATH * 2];
		PathBuildRoot(buf, (int)pnmlv->lParam);
		SetDriveInfo(pnmlv->iItem, buf);
	}
}

void CDriveInfoDlg::SetDriveInfo(int index, LPCTSTR pszRoot)
{
	CListViewCtrl list(GetDlgItem(IDC_LIST));

	SetCurrentDirectory(pszRoot);

	// ネットワーク名
	int drive = PathGetDriveNumber(pszRoot);
	TCHAR buf[MAX_PATH];
	if (lccGetNetworkPath(drive, buf))
		list.SetItemText(index, 8, buf);

	CString str;
	CDiskSpace di(pszRoot);
	if (di.GetDriveInfo()) {
		// FileSystem name
		list.SetItemText(index, 1, di.m_strFileSysName);

		// ラベル名
		if (di.m_strVolume.IsEmpty()) {
			str.LoadString(IDS_UNDEFINED);
			list.SetItemText(index, 7, str);
		}
		else
			list.SetItemText(index, 7, di.m_strVolume);

		// 全容量
		di.TotalSizeToStr(buf);
		list.SetItemText(index, 3, buf);

		// 空き容量
		di.FreeSizeToStr(buf);
		list.SetItemText(index, 2, buf);

		// 利用率
		int nPer = di.UsagePercent();
		str.Format(_T("%d%%"), nPer);
		list.SetItemText(index, 4, str);
		list.SetItemText(index, 5, StrFormatByteSize(di.m_dwBytesPerSector, buf, ARRAYSIZE(buf)));
		list.SetItemText(index, 6, StrFormatByteSize(di.m_dwBytesPerCluster, buf, ARRAYSIZE(buf)));

		// ボリュームシリアルナンバ
		str.Format(_T("%08X"), di.m_dwVolser);
		list.SetItemText(index, 9, str);
	}

	str.Format(_T("\\\\.\\%c:"), *pszRoot);
	HANDLE hvol = CreateFile(str, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hvol != INVALID_HANDLE_VALUE) {
		DISK_GEOMETRY dg;
		DWORD dw;
		if (DeviceIoControl(hvol, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(DISK_GEOMETRY), &dw, NULL)) {
			if (dg.MediaType != Unknown) {
				str.Format(_T("%d"), dg.Cylinders.LowPart);	list.SetItemText(index, 10, str);
				str.Format(_T("%d"), dg.TracksPerCylinder);	list.SetItemText(index, 11, str);
				str.Format(_T("%d"), dg.SectorsPerTrack);	list.SetItemText(index, 12, str);
			}
		}
		CloseHandle(hvol);
	}

	CDriveMediaInfo dmi;
	if (dmi.GetStorageProperty(drive)) {
		list.SetItemText(index, 13, dmi.m_strVendorID);
		list.SetItemText(index, 14, dmi.m_strProductID);
		list.SetItemText(index, 15, dmi.m_strRevision);
		str.Empty();
		switch (dmi.m_enumBusType) {
		case BusTypeScsi:		str = _T("SCSI"); break;
		case BusTypeAtapi:		str = _T("ATAPI"); break;
		case BusTypeAta:		str = _T("ATA"); break;
		case BusType1394:		str = _T("IEEE1394"); break;
		case BusTypeSsa:		str = _T("BusTypeSsa"); break;
		case BusTypeFibre:		str = _T("BusTypeFibre"); break;
		case BusTypeUsb:		str = _T("USB"); break;
		case BusTypeRAID:		str = _T("RAID"); break;
		case BusTypeiScsi:		str = _T("iSCSI"); break;
		case BusTypeSas:		str = _T("SAS"); break;
		case BusTypeSata:		str = _T("SerialATA"); break;
		case BusTypeSd:			str = _T("SD"); break;
		case BusTypeMmc:		str = _T("MMC"); break;
		case BusTypeUnknown:
		default:				str = _T("???"); break;
		}
		list.SetItemText(index, 16, str);
	}
}

void CDriveInfoDlg::OnDestroy()
{
	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wpm);
	SetRegBinary(_MKIR(IDS_SEC_WNDPLACE), _MKIR(IDS_KEY_WP_DRIVEINFO), &wpm, sizeof(wpm));
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI fopDriveInfo(HWND hwnd, int nDrive)
{
	TCHAR olddir[MAX_PATH * 2];
	GetCurrentDirectory(ARRAYSIZE(olddir), olddir);

	CDriveInfoDlg dlg;
	dlg.m_di.drive = nDrive;
	dlg.m_di.removableonly = FALSE;
	dlg.DoModal(hwnd);

	SetCurrentDirectory(olddir);
}

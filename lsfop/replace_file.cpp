/////////////////////////////////////////////////////////////////////////////
// replace.cpp

#include "stdafx.h"
#include "fopstd.h"
#include "copy.h"


////////////////////////////////////////////////////////////////////////
//

static long CompareFileSize(const WIN32_FIND_DATA& src, const WIN32_FIND_DATA& dst)
{
	DWORDLONG s, t;
	SETDWORDLONG(s, src.nFileSizeLow, src.nFileSizeHigh);
	SETDWORDLONG(t, dst.nFileSizeLow, dst.nFileSizeHigh);
	return ((s < t) ? -1 : ((s > t) ? 1 : 0));
}

static void FormatFileDetails(CString& str, const WIN32_FIND_DATA& wfd)
{
	// サイズ情報を設定
	TCHAR szs[64];
	DWORDLONG dl;
	SETDWORDLONG(dl, wfd.nFileSizeLow, wfd.nFileSizeHigh);
	FormatByteSize(dl, szs, ARRAYSIZE(szs));

	// 時刻情報を設定
	TCHAR szt[64];
	SYSTEMTIME st;
	FileTimeToSystemTime(&wfd.ftLastWriteTime, &st);
	wsprintf(szt, _T("%4u/%02u/%02u %02u:%02u:%02u"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	str.Format(IDS_FILEPROP, szs, szt);
}

////////////////////////////////////////////////////////////////////////
// CReplaceDlg

class CReplaceDlg : public CDialogImpl<CReplaceDlg>
{
public:
	CReplaceFileInfo* m_info;

public:
	CReplaceDlg() { m_info = NULL; }
	~CReplaceDlg() { }

	enum { IDD = IDD_REPLACE };

BEGIN_MSG_MAP(CReplaceDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_YESTOALL, OnYesToAll)
	ON_BN_CLICKED(IDC_NEWONLY, OnAutoRename)
	ON_BN_CLICKED(IDC_OVERWRITE, OnAutoRename)
	ON_BN_CLICKED(IDC_RENAME, OnAutoRename)
	ON_BN_CLICKED(IDC_MERGE, OnAutoRename)
	ON_BN_CLICKED(IDC_RENAMESRC, OnAutoRename)
	ON_BN_CLICKED(IDC_IGNORE, OnAutoRename)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }
	void OnYesToAll();
	void OnAutoRename();

	void QueryFileOperation();
};

BOOL CReplaceDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	m_info = (CReplaceFileInfo*)lParam;

	HICON hIcon;
	CString str;

	// 元ファイル情報
	CDiskSpace ds(m_info->m_src, true);
	PathSetDlgItemPath(m_hWnd, IDC_SRCNAME, m_info->m_src);
	hIcon = GetFileIcon(m_info->m_src, SHIL_EXTRALARGE);
	CStatic sticon = GetDlgItem(IDC_FROMICON);
	sticon.SetIcon(hIcon);
	FormatFileDetails(str, m_info->m_srcWfd);
	str.AppendFormat(_T(" (%s)"), ds.m_strFileSysName.GetString());
	SetDlgItemText(IDC_SRCPROP, str);

	// 先ファイル情報
	CDiskSpace dd(m_info->m_dst, true);
	PathSetDlgItemPath(m_hWnd, IDC_DSTNAME, m_info->m_dst);
	hIcon = GetFileIcon(m_info->m_dst, SHIL_EXTRALARGE);
	sticon = GetDlgItem(IDC_TOICON);
	sticon.SetIcon(hIcon);
	FormatFileDetails(str, m_info->m_dstWfd);
	str.AppendFormat(_T(" (%s)"), dd.m_strFileSysName.GetString());
	SetDlgItemText(IDC_DSTPROP, str);

	// ２つのファイルの新旧を比較
	FILETIME fts = m_info->m_srcWfd.ftLastWriteTime;
	FILETIME ftd = m_info->m_dstWfd.ftLastWriteTime;
	if (ds.m_nFileSystem != dd.m_nFileSystem) {
		if (!ds.isDosDateTime() && dd.isDosDateTime()) {
			_time_NTFS2FAT(fts);
		}
		else if (ds.isDosDateTime() && !dd.isDosDateTime()) {
			_time_NTFS2FAT(ftd);
		}
	}
	long diffTime = CompareFileTime(&fts, &ftd);
	// ２つのファイルのサイズを比較
	long diffSize = CompareFileSize(m_info->m_srcWfd, m_info->m_dstWfd);

	hIcon = (HICON)LoadImage(RESINST, MAKEINTRESOURCE(IDI_ITSNEW), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	CString parts;
	if (diffTime < 0) {
		parts.LoadString((diffSize < 0) ? IDS_QUERYCOPY_NEWLARGE : ((diffSize > 0) ? IDS_QUERYCOPY_NEWSMALL : IDS_QUERYCOPY_NEW));
		sticon = GetDlgItem(IDC_TONEW);
		sticon.SetIcon(hIcon);
	}
	else if (diffTime > 0) {
		parts.LoadString((diffSize < 0) ? IDS_QUERYCOPY_OLDLARGE : ((diffSize > 0) ? IDS_QUERYCOPY_OLDSMALL : IDS_QUERYCOPY_OLD));
		sticon = GetDlgItem(IDC_FROMNEW);
		sticon.SetIcon(hIcon);
	}
	else if (diffSize != 0)
		parts.LoadString((diffSize < 0) ? IDS_QUERYCOPY_LARGE : IDS_QUERYCOPY_SMALL);

	if (!parts.IsEmpty()) {
		str.Format(IDS_QUERYCOPY_SAMENAME, parts);
		SetDlgItemText(IDC_TITLE, str);
	}

	UINT idCtrl;
	switch (m_info->m_fopAction) {
	case FAC_QUERY:
	case FAC_NEWONLY:	idCtrl = IDC_NEWONLY; break;
	case FAC_OVERWRITE:	idCtrl = IDC_OVERWRITE; break;
	case FAC_MERGE: 	idCtrl = IDC_MERGE; break;
	case FAC_RENAMEDST:	idCtrl = IDC_RENAME; break;
	case FAC_RENAMESRC:	idCtrl = IDC_RENAMESRC; break;
	default:			idCtrl = IDC_IGNORE; break;
	}
	CheckDlgButton(idCtrl, TRUE);

	CWindow ctrl = GetDlgItem(IDC_AUTORENAME);
	if (ctrl.IsWindow()) {
		if (m_info->m_bAutoRename)
			CheckDlgButton(IDC_AUTORENAME, TRUE);
		OnAutoRename();
	}

	BOOL bSupportRecycle = FALSE;
	if (!PathIsUNC(m_info->m_dst)) {
		if (DriveType(PathGetDriveNumber(m_info->m_dst)) == DRIVE_FIXED) {
			CheckDlgButton(IDC_RECYCLE, m_info->m_bUseRecycle);
			bSupportRecycle = TRUE;
		}
	}
	if (!bSupportRecycle)
		GetDlgItem(IDC_RECYCLE).EnableWindow(FALSE);

	GetDlgItem(IDC_YESTOALL).EnableWindow(m_info->m_bMultiDest);
	MessageBeep(0);
	return TRUE;
}

BOOL CReplaceDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CReplaceDlg::OnOK()
{
	QueryFileOperation();
	EndDialog(IDOK);
}

void CReplaceDlg::OnYesToAll()
{
	m_info->m_bYesToAll = true;
	OnOK();
}

void CReplaceDlg::OnAutoRename()
{
	CWindow ctrl = GetDlgItem(IDC_AUTORENAME);
	if (ctrl.IsWindow()) {
		BOOL bEnable = IsDlgButtonChecked(IDC_RENAME) || IsDlgButtonChecked(IDC_RENAMESRC);
		ctrl.EnableWindow(bEnable);
	}
}

void CReplaceDlg::QueryFileOperation()
{
	if      (IsDlgButtonChecked(IDC_NEWONLY))	m_info->m_fopAction = FAC_NEWONLY;
	else if (IsDlgButtonChecked(IDC_RENAME))	m_info->m_fopAction = FAC_RENAMEDST;
	else if (IsDlgButtonChecked(IDC_OVERWRITE))	m_info->m_fopAction = FAC_OVERWRITE;
	else if (IsDlgButtonChecked(IDC_MERGE))		m_info->m_fopAction = FAC_MERGE;
	else if (IsDlgButtonChecked(IDC_RENAMESRC))	m_info->m_fopAction = FAC_RENAMESRC;
	else										m_info->m_fopAction = FAC_IGNORE;

	m_info->m_bUseRecycle = false;
	if (GetDlgItem(IDC_RECYCLE).IsWindowEnabled())
		m_info->m_bUseRecycle = (IsDlgButtonChecked(IDC_RECYCLE) != FALSE);

	m_info->m_bAutoRename = false;
	CWindow ctrl = GetDlgItem(IDC_AUTORENAME);
	if (ctrl.IsWindow()) {
		if (ctrl.IsWindowEnabled())
			m_info->m_bAutoRename = (IsDlgButtonChecked(IDC_AUTORENAME) != FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
//

BOOL QueryReplace(HWND hwnd, CReplaceFileInfo* prd)
{
	if (IsWindow(hwnd)) SetForegroundWindow(hwnd);

	CReplaceDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)prd) == IDOK)
		return TRUE;
	return FALSE;
}

// Divide.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"
#include "copy.h"

#include "..\libarcspec\libarcspec.h"
#include "lsarcsvr_i.h"

#define MAX_DEFENTRY	4
static int nDefSize[MAX_DEFENTRY] = { 634, 713, 1221, 1423 };

typedef struct tagDIVIDETARGET {
	long size;
	TCHAR title[64];
} DIVIDETARGET;

static DIVIDETARGET arrDiv[16] = {
	634,	_T("640 KB"),
	713,	_T("720 KB"),
	1221,	_T("1.25 MB"),
	1423,	_T("1.44 MB"),
};


/////////////////////////////////////////////////////////////////////////////
// DivideFileDlg message handlers

class CDivideDlg : public CDialogImpl<CDivideDlg>
{
public:
	CDivideDlg() { m_pdd = NULL; }
	~CDivideDlg() { }

	enum { IDD = IDD_DIVIDE };

BEGIN_MSG_MAP(CDivideDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_CBN_SELCHANGE(IDC_ARCPACK, OnDivideTypeSelChange)
	ON_BN_CLICKED(IDC_MKBAT, OnMakeBatchClicked)
	ON_EN_CHANGE(IDC_DIVSIZE, OnEditChange)
	ON_CBN_SELCHANGE(IDC_DRIVE, OnChangeDivideSize)
	ON_NOTIFY(SEN_GETTEXT, IDC_EDIT, OnNotifyBrowse)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnDivideTypeSelChange();
	void OnMakeBatchClicked();
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }
	void OnEditChange();
	void OnChangeDivideSize();

	void UpdateDivision();

	DIVIDEDATA* m_pdd;
};

BOOL CDivideDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pdd = (DIVIDEDATA*)lParam;

	lccDisableCharInput(GetDlgItem(IDC_SRC));
	SetDlgItemText(IDC_SRC, m_pdd->szTarget);
	SetDlgItemText(IDC_EDIT, m_pdd->szDivideTo);

	TCHAR buf[MAX_PATH];
	lstrcpy(buf, PathFindFileName(m_pdd->szTarget));
	LPTSTR p = PathFindExtension(buf);
	if (*p == '.')
		*p = 0;
	lstrcpy(m_pdd->szPrefix, buf);

	CComboBox combo = GetDlgItem(IDC_DRIVE);
	int id;
	for (id = 0; id < MAX_DEFENTRY; id++) {
		combo.AddString(arrDiv[id].title);
		if (m_pdd->nDivideSize == arrDiv[id].size) {
			combo.SetCurSel(id);
			SetDlgItemInt(IDC_DIVSIZE, arrDiv[id].size, FALSE);
		}
	}

	TCHAR fmt[MAX_PATH];
	::LoadString(RESINST, IDS_DIVIDE_DRIVETYPE, fmt, ARRAYSIZE(fmt));
	int i;
	for (i = 0; i < 26; i++) {
		PathBuildRoot(buf, i);
		if (GetDriveType(buf) == DRIVE_REMOVABLE) {
			arrDiv[id].size = i;
			wsprintf(arrDiv[id].title, fmt, buf);
			combo.AddString(arrDiv[id].title);
			id++;
		}
	}

	// 分割種別
	combo = GetDlgItem(IDC_ARCPACK);
	InitDlgCtrl(combo, MAKEINTRESOURCE(IDS_DIVIDE_TYPES));
	combo.SetCurSel(0);

	if (m_pdd->bMakeBatch)
		CheckDlgButton(IDC_MKBAT, TRUE);

	OnDivideTypeSelChange();
	return TRUE;
}

BOOL CDivideDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CDivideDlg::OnChangeDivideSize()
{
	CComboBox combo(GetDlgItem(IDC_DRIVE));
	int sel = combo.GetCurSel();
	if (sel < MAX_DEFENTRY)
		m_pdd->nDivideSize = arrDiv[sel].size;
	else {
		TCHAR dir[MAX_PATH * 2], path[MAX_PATH];
		PathBuildRoot(path, arrDiv[sel].size);
		CDiskSpace di(path);
		GetCurrentDirectory(ARRAYSIZE(dir), dir);
		while (!SetCurrentDirectory(path)) {
			if (LastError(IDS_TITLE_FOPSERVER, path, MB_QRETRY) == IDCANCEL)
				return;
		}
		SetCurrentDirectory(dir);
		if (!di.GetDriveInfo())
			return;
		m_pdd->nDivideSize = (int)Int64ShrlMod32(di.m_dlTotal, 10);
	}
	SetDlgItemInt(IDC_DIVSIZE, m_pdd->nDivideSize, FALSE);
	UpdateDivision();
}

LONGLONG __fastcall str2int64(LPCTSTR psz)
{
	LONGLONG num = 0;
	while (psz && '0' <= *psz && *psz <= '9') {
		num = num * 10 + *psz - '0';
		psz++;
	}
	return num;
}

void CDivideDlg::OnEditChange()
{
	TCHAR buf[MAX_PATH];
	GetDlgItemText(IDC_DIVSIZE, buf, ARRAYSIZE(buf));

//@	LONGLONG divsize = 0;
//@	if (!StrToInt64Ex(buf, STIF_DEFAULT, &divsize) || (divsize >= 0x200000)) {
	LONGLONG divsize = str2int64(buf);
	if (divsize >= 0x200000) {
		// 2GB以上の分割サイズは不可
		MessageBeep(0);
		if (divsize) divsize = 0x1fffff;
		else divsize = 1;
		wsprintf(buf, _T("%d"), (long)divsize);
		SetDlgItemText(IDC_DIVSIZE, buf);
		return;
	}
	m_pdd->nDivideSize = (long)divsize;
	UpdateDivision();
}

void CDivideDlg::OnDivideTypeSelChange()
{
	CComboBox combo = GetDlgItem(IDC_ARCPACK);
	m_pdd->nDivideType = combo.GetCurSel();
	if (m_pdd->nDivideType != DIVIDETYPE_NORMAL) {
		CheckDlgButton(IDC_MKBAT, FALSE);
		m_pdd->bMakeBatch = FALSE;
	}
	UpdateDivision();
}

void CDivideDlg::OnMakeBatchClicked()
{
	m_pdd->bMakeBatch = IsDlgButtonChecked(IDC_MKBAT);
	CComboBox combo = GetDlgItem(IDC_ARCPACK);
	combo.EnableWindow(!m_pdd->bMakeBatch);

	if (m_pdd->bMakeBatch)
		m_pdd->nDivideType = DIVIDETYPE_NORMAL;
	else
		m_pdd->nDivideType = combo.GetCurSel();

	UpdateDivision();
}

void CDivideDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH];
	lstrcpy(buf, m_pdd->szDivideTo);
	if (fopBrowseForFolder(m_hWnd, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		lstrcpy(m_pdd->szDivideTo, buf);
		*pResult = TRUE;
	}
}

void CDivideDlg::OnOK()
{
	TCHAR buf[MAX_PATH];
	if (GetDlgItemText(IDC_EDIT, buf, ARRAYSIZE(buf)) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}
	if (!_pathisdirectory(buf) && !fopValidateDir(m_hWnd, buf))
		return;
	lstrcpy(m_pdd->szDivideTo, buf);
	EndDialog(IDOK);
}

void CDivideDlg::UpdateDivision()
{
	if (m_pdd->nDivideType != DIVIDETYPE_NORMAL) {
		SetDlgItemText(IDC_TEXT, _T("*****"));
		return;
	}

	DWORDLONG nFiles = 0;
	if (m_pdd->nDivideSize) {
		DWORDLONG dl = (DWORDLONG)m_pdd->nDivideSize * 1024;
		nFiles = m_pdd->nFileSize / dl;
		if ((nFiles * (DWORDLONG)m_pdd->nDivideSize) < m_pdd->nFileSize)
			nFiles++;
	}

	TCHAR buf[MAX_PATH];
	if (nFiles > 1)
		wsprintf(buf, _T("*.001 - *.%03d"), (DWORD)nFiles);
	else
		lstrcpy(buf, _T("*.001"));
	SetDlgItemText(IDC_TEXT, buf);
}


//////////////////////////////////////////////////////////////////////

extern "C"
BOOL WINAPI fopQueryDivide(HWND hwnd, DIVIDEDATA* pdd)
{
	CFileInfo fi;
	if (!GetFileInfo(pdd->szTarget, fi) || fi.IsFolder())
		return FALSE;

	pdd->nFileSize = fi.m_dlSize;
	LPTSTR p = StrRChr(pdd->szTarget, NULL, '\\');
	if (p) {
		PathGetParent(pdd->szTarget, pdd->szDivideTo);
		p++;
	}
	else {
		GetCurrentDirectory(ARRAYSIZE(pdd->szDivideTo), pdd->szDivideTo);
		p = (LPTSTR)pdd->szTarget;
	}

	int nDivSize = GetRegValue(_MKIR(IDS_SEC_DIVIDE), _MKIR(IDS_KEY_DIVPIECESIZE), nDefSize[3]);
	pdd->nDivideSize = max(1, nDivSize);
	pdd->nDivideType = GetRegValue(_MKIR(IDS_SEC_DIVIDE), _MKIR(IDS_KEY_DIVUSECABINET), DIVIDETYPE_NORMAL);
	pdd->bMakeBatch = GetRegValue(_MKIR(IDS_SEC_DIVIDE), _MKIR(IDS_KEY_DIVMAKEBATCH), FALSE);

	CDivideDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)pdd) == IDOK) {
		SetRegValue(_MKIR(IDS_SEC_DIVIDE), _MKIR(IDS_KEY_DIVPIECESIZE), pdd->nDivideSize);
		SetRegValue(_MKIR(IDS_SEC_DIVIDE), _MKIR(IDS_KEY_DIVUSECABINET), pdd->nDivideType);
		SetRegValue(_MKIR(IDS_SEC_DIVIDE), _MKIR(IDS_KEY_DIVMAKEBATCH), pdd->bMakeBatch);
		return TRUE;
	}
	return FALSE;
}

BOOL BlockCopy(HANDLE hfSrc, HANDLE hfDst, DWORDLONG dlCopySize, CCopyProgressDlg* progress)
{
	// コピーバッファを確保
	DWORD dwBlockSize = 0x10000;
	LPVOID lp = VirtualAlloc(NULL, dwBlockSize, MEM_COMMIT, PAGE_READWRITE);
	if (!lp)
		return FALSE;

	DWORD lasterror = 0;
	DWORD lr, lw; 
	while (dlCopySize > 0) {
		if (!progress->ZapMessage()) {
			lasterror = ERROR_CANCELLED;
			break;
		}

		DWORD size = (DWORD)(min((DWORDLONG)dwBlockSize, dlCopySize));
		if (!ReadFile(hfSrc, lp, size, &lr, NULL)) {
			lasterror = GetLastError();
			break;
		}

		if (!lr)
			break;

		progress->SetPartCompleted(lr / 2);

		if (!WriteFile(hfDst, lp, lr, &lw, NULL)) {
			lasterror = GetLastError();
			break;
		}

		progress->SetPartCompleted(lw / 2);
		dlCopySize -= (DWORDLONG)size;
	}
	VirtualFree(lp, 0, MEM_RELEASE);

	if (lasterror)
		SetLastError(lasterror);
	return (lasterror == 0);
}

extern "C"
BOOL WINAPI fopDivideFile(HWND hwnd, DIVIDEDATA* pdd)
{
	// 分割元ファイルの最新情報を取得
	CWin32FindData wfd;
	if (!GetFileData(pdd->szTarget, wfd))
		return FALSE;

	CString str;

	CPath src;
	if (pdd->nDivideType != DIVIDETYPE_NORMAL) {
		// 書庫サーバ
		CComPtr<IArcManager> pam;
		HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
		if (SUCCEEDED(hr)) {
			src.Combine(pdd->szDivideTo, pdd->szPrefix);
			int arctype = ARC_ERROR;
			switch (pdd->nDivideType) {
			case DIVIDETYPE_CAB:
				arctype = ARC_CAB;
				str.Format(_T("-a -ml:21 -v:%d \"%s\" \"%s\""), pdd->nDivideSize, src, pdd->szTarget);
				break;
			case DIVIDETYPE_7ZIP:
				arctype = ARC_7ZIP;
				str.Format(_T("a -t7z %s -m0=LZMA -mhc=on -ms=off -v%dk -mx9 %s"), src, pdd->nDivideSize, pdd->szTarget);
				break;
			}
			if (arctype != ARC_ERROR) {
				LONG err;
				hr = pam->ArchiveW(hwnd, arctype, str, NULL, 0, &err);
			}
			else
				hr = E_FAIL;
		}
		return (SUCCEEDED(hr) ? TRUE : FALSE);
	}

	src.m_strPath = pdd->szTarget;

	CFileHandle fs, fd;
	fs.CreateFile(src, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0);
	if (fs.IsInvalid())
		return FALSE;

	int nExt = 1;
	BOOL bOK = TRUE;
	CCopyProgressDlg gage;

	DWORDLONG dlTotal;
	SETDWORDLONG(dlTotal, wfd.nFileSizeLow, wfd.nFileSizeHigh);
	gage.Start(hwnd, IDS_DO_DIVIDE, dlTotal);

	CPath body;
	CPath dst;
	DWORDLONG dlSize, dlDoneSize = 0;
	while (dlDoneSize < dlTotal) {
		CDiskSpace di(pdd->szDivideTo, true);

		// 分割ファイル名の作成
		dst.Combine(pdd->szDivideTo, pdd->szPrefix);
		str.Format(_T(".%03d"), nExt);
		dst.m_strPath += str;

		if (!_pathcompare(src, dst)) {
			AppMessage(IDS_TITLE_DIVIDE, IDS_SAMEFILENAME);
			bOK = FALSE;
			break;
		}

		// 残容量のチェック
		dlSize = UInt32x32To64(pdd->nDivideSize, 1024);
		dlSize = min(dlSize, dlTotal - dlDoneSize);
		if (!di.isSpaceAvailable(dlSize)) {
			body.m_strPath = pdd->szDivideTo;
			body.StripToRoot();
			if (body.GetDriveNumber() >= 0 && GetDriveType(body) == DRIVE_REMOVABLE) {
				if (AppMessage(IDS_TITLE_DIVIDE, IDS_DIV_INSERTFD, NULL, MB_QYESNO) == IDNO) {
					bOK = FALSE;
					break;
				}
				else
					continue;
			}
			else {
				AppMessage(IDS_TITLE_DIVIDE, IDS_DISKFULL);
				bOK = FALSE;
				break;
			}
		}

		// 分割先ファイル作成
		fd.CreateFile(dst, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
		if (fs.IsInvalid()) {
			AppMessage(IDS_TITLE_DIVIDE, IDS_FILE_CREATEERROR, dst, MB_STOP);
			bOK = FALSE;
			break;
		}

		body = dst;
		body.StripPath();
		FormatFromToString(wfd.cFileName, body, str);
		gage.SetTitle(str);

		BOOL bDivide = BlockCopy(fs, fd, dlSize, &gage);
		DWORD err = GetLastError();
		fd.CloseHandle();

		if (!bDivide) {
			ErrorBox(hwnd, err, MAKEINTRESOURCE(IDS_TITLE_DIVIDE), NULL, MB_STOP);
			DeleteFile(dst);
			bOK = FALSE;
			break;
		}
		dlDoneSize += dlSize;
		nExt++;
	}
	fs.CloseHandle();
	gage.Close();

	if (!pdd->bMakeBatch || !bOK)
		return bOK;

	// バッチファイル作成

static TCHAR* batsnr[] = {
	_T("@echo off\r\necho 連結しています...\r\n"),
	_T("if not exist %s goto error\r\n"),
	_T("copy %s %s\r\n"),
	_T("copy /b %s+%s %s\r\n"),
	_T("if errorlevel 0 goto exit\r\n@echo 連結に失敗しました\r\npause\r\ngoto exit\r\n:error\r\n@echo 連結に必要なファイルが足りません\r\npause\r\n:exit\r\n")
};

	dst.Combine(pdd->szDivideTo, pdd->szPrefix);
	dst.m_strPath += _T(".bat");

	fd.CreateFile(dst, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
	if (fd.IsInvalid()) {
		AppMessage(IDS_TITLE_DIVIDE, IDS_FILE_CREATEERROR, dst, MB_STOP);
		return FALSE;
	}

	// 連結するファイルのリストを作成
	CSimpleArray<CString> arr;
	int i;
	for (i = 1; i < nExt; i++) {
//		str.Format(_T("%s.%03d"), pdd->szPrefix, i);
//		body = str;
		body.m_strPath.Format(_T("%s.%03d"), pdd->szPrefix, i);
		body.StripPath();
		arr.Add(body);
	}

	WriteAnsiString(fd, batsnr[0], -1);
	for (i = 0; i < arr.GetSize(); i++) {
		src.m_strPath = arr[i];
		src.QuoteSpaces();
		str.Format(batsnr[1], src);
		WriteAnsiString(fd, str, -1);
	}

	// 連結先ファイル名
	body = pdd->szTarget;
	body.StripPath();
	dst = body;
	dst.QuoteSpaces();

	src.m_strPath = arr[0];
	src.QuoteSpaces();

	str.Format(batsnr[2], src, dst);
	WriteAnsiString(fd, str, -1);

	for (i = 1; i < arr.GetSize(); i++) {
		src.m_strPath = arr[i];
		src.QuoteSpaces();
		str.Format(batsnr[3], dst, src, dst);
		WriteAnsiString(fd, str, -1);
	}

	WriteAnsiString(fd, batsnr[4], -1);

	return TRUE;
}

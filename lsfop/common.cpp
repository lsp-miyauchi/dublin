#include "stdafx.h"
#include <limits.h>
#include "fopstd.h"
#include "tiphelp.h"

static TCHAR sszWinExtensions[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Extensions");

///////////////////////////////////////////////////////////////////////
//

static DWORD dwCookie = 0;
static TCHAR sszHtmlHelp[MAX_PATH];

void InitHtmlHelp()
{
	HWND hwndCapture = GetCapture();
	if (hwndCapture != NULL)
		SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

	if (!dwCookie) {
		GetModuleFileName(MODINST, sszHtmlHelp, ARRAYSIZE(sszHtmlHelp));
		PathRenameExtension(sszHtmlHelp, _T(".chm"));
		HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&dwCookie);
	}
}

void __fastcall ShowHtmlHelp(HWND hwnd, UINT idDlg)
{
	InitHtmlHelp();
	HtmlHelp(hwnd, sszHtmlHelp, HH_HELP_CONTEXT, idDlg + HID_BASE_RESOURCE);
}

void __fastcall ShowCtrlHelp(HWND hwndCtrl, UINT idCtrl)
{
	CTipHelp tiphelp;
	tiphelp.ShowTips(hwndCtrl, idCtrl);
}

///////////////////////////////////////////////////////////////////////
//

int __fastcall FormatFileDate(FILETIME* pft, LPTSTR psz, int cch)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(pft, &st);
	CString str; str.Format(_T("%04u/%02u/%02u"), st.wYear, st.wMonth, st.wDay);
	lstrcpyn(psz, str, cch);
	return lstrlen(psz);
}

int __fastcall FormatFileTime(FILETIME* pft, LPTSTR psz, int cch)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(pft, &st);
	CString str; str.Format(_T("%2u:%02u:%02u"), st.wHour, st.wMinute, st.wSecond);
	lstrcpyn(psz, str, cch);
	return lstrlen(psz);
}

int __fastcall FormatFileDateTime(FILETIME* pft, LPTSTR psz, int cch)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(pft, &st);
	CString str; str.Format(_T("%04u/%02u/%02u %02u:%02u:%02u"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	lstrcpyn(psz, str, cch);
	return lstrlen(psz);
}

int __fastcall FormatFileDateTimeEx(FILETIME* pft, LPTSTR psz, int cch)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(pft, &st);
	CString str; str.Format(_T("%04u/%02u/%02u %02u:%02u:%02u.%03u"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	lstrcpyn(psz, str, cch);
	return lstrlen(psz);
}

int __fastcall FormatFileDateTimeWin(FILETIME* pft, LPTSTR psz, int cch)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(pft, &st);
	TCHAR buf[128];
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, buf, ARRAYSIZE(buf));
	CString str = buf;
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, buf, ARRAYSIZE(buf));
	str.AppendFormat(_T(" %s"), buf);
	lstrcpyn(psz, str, cch);
	return lstrlen(psz);
}

DWORD __fastcall _getfileinfo(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	HANDLE hf = FindFirstFile(pszName, pwfd);
	DWORD code = GetLastError();
	if (hf != INVALID_HANDLE_VALUE)
		FindClose(hf);
	if (hf != INVALID_HANDLE_VALUE || code == ERROR_MORE_DATA)
		code = NOERROR;
	SetLastError(code);
	return code;
}

BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	if (_getfileinfo(pszName, pwfd) == NOERROR) {
		// ローカル時間に変換
		FILETIME ft;
		FileTimeToLocalFileTime(&pwfd->ftCreationTime, &ft);
		pwfd->ftCreationTime = ft;
		FileTimeToLocalFileTime(&pwfd->ftLastWriteTime, &ft);
		pwfd->ftLastWriteTime = ft;
		FileTimeToLocalFileTime(&pwfd->ftLastAccessTime, &ft);
		pwfd->ftLastAccessTime = ft;
		return TRUE;
	}
	return FALSE;
}

BOOL __fastcall GetFileInfo(LPCTSTR pszName, CFileInfo& fi)
{
	CWin32FindData wfd;
	if (_getfileinfo(pszName, &wfd) == NOERROR) {
		TCHAR path[MAX_PATH * 2];
		PathGetParent(pszName, path);
		fi.SetFile(wfd, path);
		return TRUE;
	}
	return FALSE;
}

LPTSTR __fastcall SkipTabSpace(LPTSTR p)
{
	if (p) {
		while (*p == ' ' || *p == '\t')
			p++;
	}
	return p;
}

int SeparateString(LPCTSTR pszText, int delim, CSimpleArray<CString>& array)
{
	array.RemoveAll();
	CString base(pszText);
	if (!base.IsEmpty()) {
		LPTSTR p = SkipTabSpace((LPTSTR)base.GetString());
		LPTSTR q;
		while (p && *p) {
			if (*p == '"') {
				q = StrChr(++p, '"');
				if (q)
					*q++ = 0;
				else
					break;
			}
			else {
				q = StrChr(p + 1, delim);
				if (q)
					*q++ = 0;
			}
			array.Add(p);
			p = SkipTabSpace(q);
		}
	}
	return array.GetSize();
}

int AppMessage(UINT idTitle, UINT idMsg, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString msg;
	if (pszInsert && *pszInsert)
		msg.Format(idMsg, pszInsert);
	else
		msg.LoadString(idMsg);
	return AtlMessageBox(::GetActiveWindow(), msg.GetString(), idTitle, nType);
}

int AppMessage(LPCTSTR pszTitle, LPCTSTR pszMsg, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString tit;
	if (IS_INTRESOURCE(pszTitle))
		tit.LoadString(PtrToUint(pszTitle));
	else
		tit = pszTitle;

	CString msg;
	if (IS_INTRESOURCE(pszMsg)) {
		if (pszInsert)
			msg.Format(PtrToUint(pszMsg), pszInsert);
		else
			msg.LoadString(PtrToUint(pszMsg));
	}
	else {
		if (pszInsert)
			msg.Format(pszMsg, pszInsert);
		else
			msg = pszMsg;
	}
	return AtlMessageBox(::GetActiveWindow(), msg.GetString(), tit.GetString(), nType);
}

void GetErrorString(DWORD code, CString& str)
{
	str.Empty();
	LPTSTR p;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&p, 0, NULL);
	str = p;
	LocalFree(p);
}

int ErrorBox(HWND hwnd, DWORD code, LPCTSTR pszCaption, LPCTSTR pszAddtional, int typeAction)
{
	CString str;
	GetErrorString(code, str);
	if (pszAddtional && *pszAddtional) {
		str.Insert(0, _T("\n"));
		str.Insert(0, pszAddtional);
	}
	CString cap(pszCaption);
	return ::MessageBox(hwnd, str, cap, typeAction);
}

int LastError(UINT idCaption, LPCTSTR pszAddtional, UINT typeAction)
{
	DWORD code = GetLastError();
	if (code != NO_ERROR)
		return ErrorBox(::GetActiveWindow(), code, MAKEINTRESOURCE(idCaption), pszAddtional, typeAction);
	return 0;
}

BOOL FileBrowse(BOOL bRead, HWND hwnd, _RES_STRING caption, _RES_STRING filter, LPTSTR pszInitialDir, LPTSTR pszName)
{
	TCHAR szFileName[MAX_PATH * 2], szFileTitle[MAX_PATH];
	szFileName[0] = szFileTitle[0] = 0;

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	if (*pszName)
		lstrcpy(szFileName, pszName);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrDefExt = NULL;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = ARRAYSIZE(szFileName);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = ARRAYSIZE(szFileTitle);
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;

	ofn.lpstrTitle = caption;
	ofn.lpstrFilter = filter;
	LPTSTR p = (LPTSTR)(LPCTSTR)filter;
	while (p = StrChr(p, '|'))
		*p++ = 0;
	if (pszInitialDir)
		ofn.lpstrInitialDir = pszInitialDir;

	int nResult;
	if (bRead)
		nResult = GetOpenFileName(&ofn);
	else
		nResult = GetSaveFileName(&ofn);

	if (nResult)
		lstrcpy(pszName, ofn.lpstrFile);
	return nResult;
}

BOOL FileBrowseMultiSel(HWND hwnd, _RES_STRING caption, _RES_STRING filter, LPTSTR pszInitialDir, LPTSTR pszName, int cchMax)
{
	TCHAR szFileTitle[MAX_PATH];
	szFileTitle[0] = '\0';
	*pszName = '\0';

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrDefExt = NULL;
	ofn.lpstrFile = pszName;
	ofn.nMaxFile = cchMax;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = ARRAYSIZE(szFileTitle);
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

	CString csFilter, csTitle;

	ofn.lpstrTitle = caption;
	ofn.lpstrFilter = filter;
	LPTSTR p = (LPTSTR)(LPCTSTR)filter;
	while (p = StrChr(p, '|'))
		*p++ = '\0';
	if (pszInitialDir)
		ofn.lpstrInitialDir = pszInitialDir;

	return GetOpenFileName(&ofn);
}

int DoMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return 0;
}

BOOL HasFiles(LPCTSTR lpPath)
{
	BOOL bExist = FALSE;
	CEnterFolder ef(lpPath);
	if (ef.IsFolderChanged()) {
		CFileFinder ff;
		if (ff.BeginFind(gszAst)) {
			do {
				if (!ff.IsDot()) {
					bExist = TRUE;
					break;
				}
			} while (ff.NextFind());
			ff.EndFind();
		}
	}
	return bExist;
}

HICON __fastcall GetFileIcon(LPCTSTR pName, int typeIcon)
{
	SHFILEINFO sfi;
	UINT flags = SHGFI_SYSICONINDEX;
	if (typeIcon == SHIL_LARGE)
		flags |= SHGFI_LARGEICON;
	else if (typeIcon == SHIL_SMALL)
		flags |= SHGFI_SMALLICON;
	HIMAGELIST him = (HIMAGELIST)SHGetFileInfo(pName, 0, &sfi, sizeof(sfi), flags);

	HICON hic = NULL;
	if (typeIcon == SHIL_EXTRALARGE || typeIcon == SHIL_JUMBO) {
		IImageList* pii;
		if (SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&pii) == S_OK) {
			if (pii->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hic) != S_OK)
				hic = NULL;
			pii->Release();
		}
	}
	if (!hic)
		hic = ImageList_ExtractIcon(NULL, him, sfi.iIcon);
	return hic;
}

void ReplaceString(LPTSTR lpText, LPCTSTR lpSrc, LPCTSTR lpDst)
{
	LPTSTR p = lpText;
	TCHAR* pRep = new TCHAR[MAX_PATH * 4];
	memset(pRep, 0, sizeof(TCHAR) * (MAX_PATH * 4));
	LPTSTR q = pRep;
	while (*p) {
		if (*p != *lpSrc || StrCmpN(p, lpSrc, lstrlen(lpSrc)))
			*q++ = *p++;
		else {
			lstrcpy(q, lpDst);
			q += lstrlen(lpDst);
			p += lstrlen(lpSrc);
		}
	}
	*q = '\0';
	lstrcpy(lpText, pRep);
	delete [] pRep;
}

void __cdecl FormatStringV(CString& str, UINT nIDS, ...)
{
	CString fmt;
	if (fmt.LoadString(nIDS)) {
		va_list args;
		va_start(args, nIDS);
		str.FormatV(fmt, args);
		va_end(args);
	}
}

void StringSerialize(CString& str, CFile& file, BOOL bRead)
{
	BYTE len;
	if (bRead) {
		// ANSI で読み込んで UNICODE 変換
		str.Empty();
		file.Read(&len, sizeof(BYTE));
		if (len > 0) {
			if (len != 0xff) {			// for ANSI
				char cb[256];
				file.Read(cb, len);
				cb[len] = 0;

				str = cb;
			}
			else {						// for UNICODE
				WORD wlen;
				file.Read(&wlen, sizeof(WORD));
				if (wlen > 0) {
					LPTSTR p = str.GetBuffer(wlen + 1);
					file.Read(p, wlen * sizeof(TCHAR));
					p[wlen] = 0;
					str.ReleaseBuffer();
				}
			}
		}
	}
	else {
		// UNICODEで書き出す
		len = 0xff;
		file.Write(&len, sizeof(BYTE));			// 0xff は UNICODEマーカー

		WORD wlen = str.GetLength();
		file.Write(&wlen, sizeof(WORD));
		if (wlen > 0) file.Write((void*)(LPCTSTR)str, wlen * sizeof(TCHAR));
	}
}

static const TCHAR sszShellClassINfo[] = _T(".ShellClassInfo");

extern "C"
BOOL WINAPI fopGetSyncDataFolder(LPTSTR pszFolder)
{
	TCHAR buf[MAX_PATH * 2];
	SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, buf);
	PathAppend(buf, _T("My FolderSync Projects"));

	if (!_pathisdirectory(buf)) {
		SHCreateDirectory(NULL, buf);
		// フォルダをReadOnlyに
		SetFileAttributes(buf, FILE_ATTRIBUTE_READONLY);

		TCHAR name[MAX_PATH], cmd[MAX_PATH];
		GetModuleFileName(MODINST, name, ARRAYSIZE(name));
		GetShortPathName(name, name, ARRAYSIZE(name));
		// InfoTip
		wsprintf(cmd, _T("@%s,-%d"), name, IDS_SYNCFOLDER_INFOTIP);

		// desktop.iniファイルを作成
		PathCombine(name, buf, _T("desktop.ini"));
		WritePrivateProfileString(sszShellClassINfo, _T("InfoTip"), cmd, name);
		// Icon
		WritePrivateProfileString(sszShellClassINfo, _T("IconFile"), _T("%SystemRoot%\\system32\\SHELL32.dll"), name);
		WritePrivateProfileString(sszShellClassINfo, _T("IconIndex"), _T("110"), name);
		// iniファイルの属性を変更
		SetFileAttributes(name, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
	}

	lstrcpy(pszFolder, buf);
	return TRUE;
}

HRESULT GetSyncDataFolderItem(IShellItem** ppsi)
{
	if (!ppsi) return E_INVALIDARG;

	HRESULT hr = E_FAIL;
	*ppsi = NULL;
	TCHAR path[MAX_PATH * 2];
	if (fopGetSyncDataFolder(path))
		hr = SHCreateItemFromParsingName(path, NULL, IID_IShellItem, (void**)ppsi);
	return hr;
}

BOOL GetFopLocalDataFolder(LPTSTR pszFolder)
{
	TCHAR buf[MAX_PATH * 2];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf);
	PathAppend(buf, _T("LightShip\\FopSettings"));
	if (!_pathisdirectory(buf)) {
		SHCreateDirectory(NULL, buf);
	}
	lstrcpy(pszFolder, buf);
	return TRUE;
}

#define CRCPOLY16		0x1021

void __fastcall initcrc16tbl(WORD* pcrctbl)
{
	register WORD i, j , r;
	for (i = 0; i <= UCHAR_MAX; i++) {
		r = i << (16 - CHAR_BIT);
		for (j = 0; j < CHAR_BIT; j++) {
			if (r & 0x8000)
				r = (r << 1) ^ CRCPOLY16;
			else
				r <<= 1;
		}
		pcrctbl[i] = r & 0xFFFF;
	}
}

WORD __fastcall crc16(WORD* pcrctbl, int n, BYTE* pcb, WORD crc)
{
	register WORD r = crc;
	while (--n >= 0)
		r = (r << CHAR_BIT) ^ pcrctbl[(BYTE)(r >> (16 - CHAR_BIT)) ^ *pcb++];
	return ~r & 0xFFFF;
}

int RegLoadHistory(CSimpleArray<CString>& array, LPCTSTR pszSec, LPCTSTR pszKey, int maxEntry)
{
	CString sec, key;
	if (IS_INTRESOURCE(pszSec)) { sec.LoadString(PtrToUint(pszSec)); pszSec = sec; }
	if (IS_INTRESOURCE(pszKey)) { key.LoadString(PtrToUint(pszKey)); pszKey = key; }

	TCHAR subkey[MAX_PATH * 2];
	key_combine(subkey, gszRegRoot, gszApp, pszSec, pszKey, NULL);

	int numItems = 0;
	CMRUProc mru;
	if (mru.Create(HKEY_CURRENT_USER, subkey, maxEntry)) {
		int n = mru.GetCount();
		TCHAR buf[MAX_PATH * 2];
		for (int i = 0; i < n; i++) {
			if (mru.Enum(i, buf, ARRAYSIZE(buf)) > 0) {
				array.Add(buf);
				numItems++;
			}
		}
		mru.Close();
	}
	return numItems;
}

int RegLoadHistoryEx(HWND hCombo, LPCTSTR pszSec, LPCTSTR pszKey, int maxEntry)
{
	CComboBox combo = hCombo;
	if (!combo.IsWindow()) return -1;

	CSimpleArray<CString> array;
	int cnt = RegLoadHistory(array, pszSec, pszKey, maxEntry);
	for (int i = 0; i < cnt; i++)
		combo.AddString(array[i]);
	return combo.GetCount();
}

BOOL RegSaveHistory(LPCTSTR pszSec, LPCTSTR pszKey, int maxEntry, LPCTSTR pszString)
{
	CString sec, key;
	if (IS_INTRESOURCE(pszSec)) { sec.LoadString(PtrToUint(pszSec)); pszSec = sec; }
	if (IS_INTRESOURCE(pszKey)) { key.LoadString(PtrToUint(pszKey)); pszKey = key; }

	TCHAR subkey[MAX_PATH * 2];
	key_combine(subkey, gszRegRoot, gszApp, pszSec, pszKey, NULL);

	CMRUProc mru;
	if (mru.Create(HKEY_CURRENT_USER, subkey, maxEntry)) {
		mru.Add(pszString);
		mru.Close();
	}
	return TRUE;
}

void RegClearHistory(LPCTSTR pszSec, LPCTSTR pszKey)
{
	CString sec, key;
	if (IS_INTRESOURCE(pszSec)) { sec.LoadString(PtrToUint(pszSec)); pszSec = sec; }
	if (IS_INTRESOURCE(pszKey)) { key.LoadString(PtrToUint(pszKey)); pszKey = key; }

	TCHAR subkey[MAX_PATH * 2];
	key_combine(subkey, gszRegRoot, gszApp, pszSec, pszKey, NULL);
	SHDeleteKey(HKEY_CURRENT_USER, subkey);
}

void RegClearHistoryEx(HWND hCombo, LPCTSTR pszSec, LPCTSTR pszKey)
{
	CComboBox combo = hCombo;
	if (combo.IsWindow())
		combo.ResetContent();

	RegClearHistory(pszSec, pszKey);
}

extern "C"
BOOL WINAPI fopInvokeIDList(HWND hwndOwner, LPITEMIDLIST pidl, LPCTSTR pVerb, LPCTSTR pParams, LPCTSTR pDefdir)
{
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_IDLIST | SEE_MASK_INVOKEIDLIST | SEE_MASK_UNICODE;
	sei.hwnd = hwndOwner;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpIDList = pidl;
	sei.lpVerb = pVerb;
	sei.lpParameters = pParams;
	sei.lpDirectory = pDefdir;
	return ShellExecuteEx(&sei);
}

BOOL InvokeFile(HWND hwndOwner, LPCTSTR pVerb, LPCTSTR pFile, LPCTSTR pParam, LPCTSTR pDir, bool bWaitExitProgram)
{
	TCHAR path[MAX_PATH];
	if (!pDir || *pDir == '\0') {
		lstrcpy(path, pFile);
		PathStripPath(path);
	}
	else
		lstrcpy(path, pDir);

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_UNICODE;
	sei.hwnd = hwndOwner;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpVerb = pVerb;
	sei.lpFile = pFile;
	sei.lpParameters = pParam;
	sei.lpDirectory = path;

	if (bWaitExitProgram)
		sei.fMask |= SEE_MASK_NOCLOSEPROCESS;
	BOOL bOK = ShellExecuteEx(&sei);
	if (bOK && bWaitExitProgram && sei.hProcess) {
		WaitForSingleObject(sei.hProcess, INFINITE);
		CloseHandle(sei.hProcess);
	}
	return bOK;
}

extern "C"
BOOL WINAPI fopInvokeFile(HWND hwndOwner, LPCTSTR pVerb, LPCTSTR pFile, LPCTSTR pParam, LPCTSTR pDir)
{
	return InvokeFile(hwndOwner, pVerb, pFile, pParam, pDir, false);
}

void __fastcall unrap_quote_split(LPTSTR pszName)
{
	TCHAR buf[MAX_PATH];
	lstrcpy(buf, pszName);
	LPTSTR ps = buf;
	if (*ps == '"') {
		ps++;
		LPTSTR pe = StrChr(ps, '"');
		if (pe)
			*pe = '\0';
		lstrcpy(pszName, ps);
	}
	else {
		ps = StrChr(buf, '"');
		if (ps)
			*ps = '\0';

		ps = StrChr(buf, '%');
		if (ps)
			*ps = '\0';

		ps = StrChr(buf, ',');
		if (ps)
			*ps = '\0';

		BOOL bFound = FALSE;
		CWin32FindData wfd;
		ps = buf;
		while (ps && *ps) {
			ps = StrChr(ps, ' ');
			if (ps) {
				*ps = '\0';
				if (_getfileinfo(buf, &wfd) == NOERROR && !wfd.IsFolder()) {
					bFound = TRUE;
					break;
				}
				*ps++ = ' ';
			}
		}
		if (!bFound) {
			ps = StrChr(buf, ' ');
			if (ps)
				*ps = '\0';
		}
		lstrcpy(pszName, buf);
	}
}

BOOL WINAPI fopFindExecutable(LPCTSTR pszFile, LPTSTR pszExec)
{
	//
	// pszFile と pszExec は同じ配列でもOK
	//
	if (!pszFile || !pszExec) return FALSE;

	LPTSTR pExt = PathFindExtension(pszFile);
	if (*pExt != '.') return FALSE;

	// ショートカットチェック
	if (!StrCmpI(pExt, gszLNK)) {
		FSLINKINFO fli;
		if (fopParseLinkInfo(pszFile, &fli)) {
			lstrcpy(pszExec, fli.szTargetName);
			return TRUE;
		}
	}

	BOOL bOK = FALSE;
	TCHAR exec[MAX_PATH * 2];
	DWORD cch = ARRAYSIZE(exec);
	if (AssocQueryString(ASSOCF_REMAPRUNDLL, ASSOCSTR_EXECUTABLE, pExt, NULL, exec, &cch) == S_OK) {
		if (exec[0] == _T('%') && exec[1] == _T('1'))	// 自分自身?
			lstrcpy(exec, pszFile);
		bOK = TRUE;
	}
	else {
		// Software\Microsoft\Windows\CurrentVersion\Extensions のチェック
		CRegKey reg;
		if (reg.Open(HKEY_CURRENT_USER, sszWinExtensions) == ERROR_SUCCESS) {
			ULONG len = ARRAYSIZE(exec);
			if (reg.QueryStringValue(pExt + 1, exec, &len) == ERROR_SUCCESS && len > 1) {
				unrap_quote_split(exec);
				bOK = TRUE;
			}
			reg.Close();
		}
	}

	if (bOK) {
		GetLongPathName(exec, exec, ARRAYSIZE(exec));
		lstrcpy(pszExec, exec);
	}
	return bOK;
}

#if 0	// -- 未使用 ------
void WINAPI fopAdjustStaticCtrlString(HWND hctl, LPTSTR pszText)
{
	HFONT hf = GetWindowFont(hctl);
	CRect rc;
	GetClientRect(hctl, &rc);

	HDC hdc = GetDC(hctl);
	HDC hdcw = CreateCompatibleDC(hdc);
	ReleaseDC(hctl, hdc);

	HFONT hfo = SelectFont(hdcw, hf);

	DRAWTEXTPARAMS dtp;
	memset(&dtp, 0, sizeof(dtp));
	dtp.cbSize = sizeof(dtp);
	DrawTextEx(hdcw, pszText, -1, &rc,
					DT_SINGLELINE | DT_LEFT | DT_NOPREFIX |
					DT_PATH_ELLIPSIS | DT_MODIFYSTRING, &dtp);
	SelectFont(hdcw, hfo);
	DeleteDC(hdcw);
}
#endif	// -- 未使用 ------

#if 0	// -- 未使用 ------
extern "C"
BOOL WINAPI fopGetDiskFreeSpace(LPCTSTR pRoot, DISKSPACEINFO* pdsi)
{
	if (!pdsi)
		return FALSE;
	memset(pdsi, 0, sizeof(DISKSPACEINFO));

	// ディスク空き情報
	ULARGE_INTEGER t, f, tf;
	if (GetDiskFreeSpaceEx(pRoot, &f, &t, &tf)) {
		pdsi->dlTotal = t.QuadPart;
		pdsi->dlFreeCaller = f.QuadPart;
		pdsi->dlFreeTotal = tf.QuadPart;
	}

	// ディスク情報の確定
	if (ISALPHA(*pRoot)) {
		DWORD spc, bps, fc, tc;
		if (GetDiskFreeSpace(pRoot, &spc, &bps, &fc, &tc)) {
			pdsi->dwBytesPerSector = bps;
			pdsi->dwBytesPerCluster = spc * bps;
			pdsi->dwFreeClusters = fc;
			pdsi->dwTotalClusters = tc;
		}

		if (pdsi->dlTotal == 0) {
			pdsi->dlTotal = UInt32x32To64(pdsi->dwTotalClusters, pdsi->dwBytesPerCluster);
			pdsi->dlFreeCaller = UInt32x32To64(pdsi->dwFreeClusters, pdsi->dwBytesPerCluster);
			pdsi->dlFreeTotal = pdsi->dlFreeCaller;
		}
	}

	// ボリューム情報
	DWORD fsf;
	if (GetVolumeInformation(pRoot, pdsi->szVolume, MAX_PATH, &pdsi->dwVolser,
							&fsf, &pdsi->dwFileSystemFlags, pdsi->szFileSystem, MAX_PATH)) {
		CharUpper(pdsi->szFileSystem);
		if      (!StrCmp(pdsi->szFileSystem,  _T("FAT")))		pdsi->nFileSystemType = FSTYPE_FAT;
		else if (!StrCmp(pdsi->szFileSystem,  _T("FAT32")))		pdsi->nFileSystemType = FSTYPE_FAT32;
		else if (!StrCmp(pdsi->szFileSystem,  _T("NTFS")))		pdsi->nFileSystemType = FSTYPE_NTFS;
		else if (!StrCmp(pdsi->szFileSystem,  _T("EXFAT")))		pdsi->nFileSystemType = FSTYPE_EXFAT;
		else if (!StrCmp(pdsi->szFileSystem,  _T("CDFS")))		pdsi->nFileSystemType = FSTYPE_CDFS;
		else if (!StrCmpN(pdsi->szFileSystem, _T("UDF"), 3))	pdsi->nFileSystemType = FSTYPE_UDF;
		else if (!StrCmp(pdsi->szFileSystem,  _T("HPFS")))		pdsi->nFileSystemType = FSTYPE_HPFS;
		else													pdsi->nFileSystemType = FSTYPE_UNKNOWN;

		return TRUE;
	}
	return FALSE;
}
#endif	// -- 未使用 ------

void __fastcall ReplaceInvalidToken(LPTSTR pszFile, TCHAR chReplaceWith)
{
	CString with;
	CString str = pszFile;
	int pos = 0;
	CString token = str.Tokenize(gszInvalidToken, pos);
	while (!token.IsEmpty()) {
		with += token;
		if (str.GetAt(pos - 1) && chReplaceWith) {
			with += chReplaceWith;
		}
		token = str.Tokenize(gszInvalidToken, pos);
	}
	lstrcpy(pszFile, with);
}

void __fastcall ConvertInvalidToken(CString& str)
{
	if (str.FindOneOf(gszInvalidToken) >= 0) {
		LPCTSTR p = gszInvalidToken, q = gszSafetyToken;
		while (*p) {
			str.Replace(*p++, *q++);
		}
	}
}

static TCHAR sszIniSec[] = _T("files");
static TCHAR sszIniEnt[] = _T("entries");

extern "C"
HDROP WINAPI fopGetFileNamesFromTempIniFile(LPTSTR pszName)
{
	if (!pszName || !_pathfileexists(pszName))
		return NULL;

	TCHAR name[MAX_PATH * 2], buf[MAX_PATH * 2];
	if (GetPrivateProfileString(sszIniSec, sszIniEnt, _T(""), buf, ARRAYSIZE(buf), pszName) < 1)
		return NULL;

	int cnt = StrToInt(buf);
	if (cnt > 0) {
		HDROP hdrop = fopDragInitBlock(CPoint(0, 0), FALSE);
		if (hdrop) {
			for (int i = 0; i < cnt; i++) {
				wsprintf(buf, _T("%04d"), i);
				if (GetPrivateProfileString(sszIniSec, buf, _T(""), name, ARRAYSIZE(name), pszName) > 0)
					hdrop = fopDragAppendFile(hdrop, name);
			}
			return hdrop;
		}
	}
	return NULL;
}

extern "C"
BOOL WINAPI fopSaveFileNamesToTempIniFile(HDROP hdrop, LPTSTR pszName)
{
	if (!hdrop || !pszName) return FALSE;

	*pszName = '\0';
	TCHAR path[MAX_PATH * 2];
	GetTempPath(MAX_PATH, path);
	GetTempFileName(path, _T("~ft"), 0, pszName);

	TCHAR name[MAX_PATH * 2], buf[MAX_PATH * 2];
	int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < cnt; i++) {
		wsprintf(buf, _T("%04d"), i);
		WritePrivateProfileString(sszIniSec, buf, name, pszName);
	}
	wsprintf(buf, _T("%d"), cnt);
	WritePrivateProfileString(sszIniSec, sszIniEnt, buf, pszName);
	return TRUE;
}

BOOL __fastcall _pathfileexists(LPCTSTR pszName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	return (pszName && GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad));
}

BOOL __fastcall _pathisfile(LPCTSTR pszName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	return (pszName && GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad) &&
			!(wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

}

BOOL __fastcall _pathisdirectory(LPCTSTR pszName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	return (pszName && GetFileAttributesEx(pszName, GetFileExInfoStandard, &wfad) &&
			(wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

static inline void pathflatten(LPCTSTR path, CString& flatten)
{
	LPTSTR p = flatten.GetBuffer(lstrlen(path) + 2);
	lstrcpy(p, path);
	PathAddBackslash(p);
	CharLower(p);
	flatten.ReleaseBuffer();
	flatten.Replace(_T('\\'), _T('\t'));
}

int __fastcall _pathcompare(LPCTSTR src, LPCTSTR dst)
{
	CString s; pathflatten(src, s);
	CString t; pathflatten(dst, t);
	return StrCmpLogicalW(s, t);
}

int __fastcall _pathisparent(LPCTSTR parent, LPCTSTR child)
{
	CString s; pathflatten(parent, s);
	CString t; pathflatten(child, t);
	return StrCmpN(s, t, lstrlen(s));
}

BOOL __fastcall _pathdecorate(LPCTSTR pszOriginal, LPTSTR pszDecolate, int cchMax)
{
	if (!pszOriginal || *pszOriginal == 0)
		return FALSE;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, pszOriginal);
	if (!_pathfileexists(buf)) {
		lstrcpyn(pszDecolate, buf, cchMax);
		return TRUE;
	}
	else {
		LPCTSTR ext = PathFindExtension(buf);
		TCHAR tmp[MAX_PATH * 2];
		for (int i = 1; i < 1000; i++) {
			lstrcpy(tmp, buf);
			PathRemoveExtension(tmp);
			wsprintf(&tmp[lstrlen(tmp)], _T("(%d)"), i);
			if (*ext == '.')
				lstrcat(tmp, ext);
			if (!_pathfileexists(tmp)) {
				lstrcpyn(pszDecolate, tmp, cchMax);
				return TRUE;
			}
		}
	}
	return FALSE;
}

void __fastcall _pathaddsuffix(LPCTSTR pszOriginal, LPCTSTR pszSuffix, LPTSTR pszDecolate, int cchMax)
{
	CString ext = PathFindExtension(pszOriginal);
	CString str = pszOriginal;
	PathRemoveExtension((LPTSTR)str.GetString());
	str.ReleaseBuffer();
	str.AppendFormat(_T(" - %s"), pszSuffix);
	str += ext;
	lstrcpyn(pszDecolate, str, cchMax);
}

LPTSTR __fastcall FormatByteSize(DWORDLONG dlNum, LPTSTR pszFormat, int cch)
{
	if (pszFormat && cch > 1) {
		CString str;
		if (dlNum == 0) {
			pszFormat[0] = _T('0');
			pszFormat[1] = 0;
		}
		else {
			while (dlNum) {
				DWORD t = (dlNum % 1000);
				CString parts; parts.Format(_T(",%03d"), t);
				str.Insert(0, parts);
				dlNum /= 1000;
			}
			LPCTSTR p = str;
			while (*p == _T('0') || *p == _T(',')) p++;
			lstrcpyn(pszFormat, p, cch);
		}
	}
	return pszFormat;
}

const static TCHAR sszByteForm[] = _T("%s (%s バイト)");

void FileSizeInformation(DWORDLONG dlNum, CString& str)
{
	TCHAR kb[128], bb[128];
	StrFormatByteSize64(dlNum, kb, ARRAYSIZE(kb));
	if (dlNum >= 1024) {
		FormatByteSize(dlNum, bb, ARRAYSIZE(bb));
		str.Format(sszByteForm, kb, bb);
	}
	else
		str = kb;
}


BOOL __fastcall PathGetUNCServerShare(LPCTSTR pPath, LPTSTR pShare)
{
	if (pPath && *pPath == '\\' && *(pPath + 1) == '\\') {
		lstrcpy(pShare, pPath);
		register LPTSTR p = pShare;

		p = StrChr(p + 2, '\\');
		if (p && *(p + 1) != '\0') {
			p = StrChr(p + 1, '\\');
			if (p)
				*p = '\0';
			return TRUE;
		}
	}
	return FALSE;
}

LPTSTR __fastcall PathGetRootFromPath(LPCTSTR pszPath, LPTSTR pszRoot)
{
	if (PathIsUNC(pszPath))
		return (PathGetUNCServerShare(pszPath, pszRoot) ? pszRoot : NULL);
	else if (ISALPHA(*pszPath) && *(pszPath + 1) == ':')
		return PathBuildRoot(pszRoot, PathGetDriveNumber(pszPath));
	return NULL;
}

bool __fastcall PathIsCommonPrefixFolder(LPCTSTR pSrc, LPCTSTR pDst)
{
	TCHAR buf[MAX_PATH];
	if (PathCommonPrefix(pSrc, pDst, buf) > 2) {
		if (ISALPHA(buf[0]) && buf[1] == ':')
			return true;
		if (buf[0] == '\\' && buf[1] == '\\' && StrChr(&buf[2], '\\'))
			return true;
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////
//

BOOL __fastcall PathFormatRename(LPTSTR pszName, LPCTSTR pszFormat)
{
	if (!pszName || *pszName == 0 || !pszFormat || *pszFormat == 0) return FALSE;

	TCHAR path[MAX_PATH * 2];
	PathGetParent(pszName, path);

	CString fname = PathFindFileName(pszName);
	int pos = fname.ReverseFind(_T('.'));
	CString body;
	CString ext;
	if (pos >= 0) {
		ext = fname.Mid(pos);
		body = fname.Left(pos);
	}
	else
		body = fname;

	CString fmt = pszFormat;
	fmt.Replace(_T("$N"), fname);
	fmt.Replace(_T("$F"), body);
	fmt.Replace(_T("$E"), ext);
	fmt.Replace(_T("$I"), _T("%d"));

	CString str;
	for (int i = 1; i < 10000; i++) {
		str.Format(fmt, i);
		if (!_pathfileexists(str)) {
			PathCombine(pszName, path, str);
			return TRUE;
		}
	}
	return FALSE;
}

static const TCHAR* sszRenameFormat[] = {
	_T("$F($I)$E"),
	_T("$F[$I]$E"),
	_T("$F~$I$E"),
	_T("$F-$I$E"),
	_T("$F_$I$E"),
	_T("$F.$I$E")
};

BOOL __fastcall PathSetUnique(LPTSTR pszName, int renametype)
{
	int type = min(ARRAYSIZE(sszRenameFormat), max(0, renametype));
	return PathFormatRename(pszName, sszRenameFormat[type]);
}

void __fastcall SetDialogIcon(HWND hdlg, UINT idIcon)
{
	HINSTANCE hInst = RESINST;
	HICON hic;
	hic = (HICON)LoadImage(hInst, _MKIR(idIcon), IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM)hic);
	hic = (HICON)LoadImage(hInst, _MKIR(idIcon), IMAGE_ICON,
							GetSystemMetrics(SM_CXICON),
							GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SendMessage(hdlg, WM_SETICON, ICON_BIG, (LPARAM)hic);
}

void __fastcall RemoveDialogIcon(HWND hdlg)
{
	HICON hic = (HICON)SendMessage(hdlg, WM_GETICON, ICON_SMALL, 0);
	DestroyIcon(hic);
	hic = (HICON)SendMessage(hdlg, WM_GETICON, ICON_BIG, 0);
	DestroyIcon(hic);
}

static void __fastcall build_unicode_path(LPCTSTR pSrc, LPWSTR pDst, int cchMax)
{
	TCHAR name[MAX_PATH * 2];
	lstrcpy(name, _T("\\\\?\\"));
	LPCTSTR p = pSrc;
	if (PathIsUNC(pSrc)) {
		p = PathFindNextComponent(pSrc);
		lstrcat(name, _T("UNC\\"));
	}
	lstrcat(name, p);
	lstrcpy(pDst, name);
}

static const TCHAR* invalid_names[] = {
	_T("CON"), _T("PRN"), _T("AUX"), _T("CLOCK$"), _T("NUL"),
	_T("COM1"), _T("COM2"), _T("COM3"), _T("COM4"), _T("COM5"), _T("COM6"), _T("COM7"), _T("COM8"), _T("COM9"),
	_T("LPT1"), _T("LPT2"), _T("LPT3"), _T("LPT4"), _T("LPT5"), _T("LPT6"), _T("LPT7"), _T("LPT8"), _T("LPT9"),
	NULL
};

BOOL __fastcall validate_makefile(LPTSTR pszName)
{
	// 文字列終端が ' ' または '.' で終わるフォルダ名の修正
	CString str = pszName;
	str.TrimRight(_T(" ."));
	if (str.IsEmpty())
		return FALSE;

	LPTSTR p = PathFindExtension(str.GetString());
	if (*p == '.')
		*p = 0;
	PathStripPath((LPTSTR)str.GetString());

	for (int i = 0; invalid_names[i] != NULL; i++) {
		if (!str.CompareNoCase(invalid_names[i]))
			return FALSE;
	}
	return TRUE;
}

HANDLE __fastcall _createfile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile)
{
	WCHAR wsz[MAX_PATH * 2];
	build_unicode_path(pName, wsz, ARRAYSIZE(wsz));
	return CreateFile(wsz, fdwAccess, fdwShareMode, psa, fdwCreate, fdwAttrsAndFlags, hTemplateFile);
}

BOOL __fastcall _createdirectory(LPCTSTR pName, LPSECURITY_ATTRIBUTES psa)
{
	TCHAR name[MAX_PATH];
	lstrcpy(name, pName);
	PathRemoveBlanks(name);

	WCHAR wsz[MAX_PATH * 2];
	build_unicode_path(name, wsz, ARRAYSIZE(wsz));
	return CreateDirectory(wsz, psa);
}

BOOL __fastcall _removedirectory(LPCTSTR pszPath)
{
	DWORD err;
	if (!RemoveDirectory(pszPath)) {
		err = GetLastError();
		WIN32_FIND_DATA wfd;
		if (_getfileinfo(pszPath, &wfd) == NOERROR) {
			TCHAR name[MAX_PATH * 2], dir[MAX_PATH * 2];
			PathCombine(name, pszPath, _T(".."));
			PathAppend(name, wfd.cAlternateFileName);
			PathCanonicalize(dir, name);
			return RemoveDirectory(dir);
		}
		else {
			SetLastError(err);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL __fastcall _movefile(LPCTSTR pszExist, LPCTSTR pszNew)
{
	DWORD flg = MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH;
	if (!PathIsSameRoot(pszExist, pszNew))
		flg |= MOVEFILE_COPY_ALLOWED;
	BOOL bOK = MoveFileWithProgress(pszExist, pszNew, NULL, NULL, flg);
	if (!bOK && (GetLastError() == ERROR_NOT_SAME_DEVICE)) {
		flg |= MOVEFILE_COPY_ALLOWED;
		bOK = MoveFileWithProgress(pszExist, pszNew, NULL, NULL, flg);
	}
	return bOK;
}

int __fastcall GetAnotherDataStream(LPCTSTR pszName)
{
	int cnt = 0;
	HANDLE hf = CreateFile(pszName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		void* pctx = NULL;
		WIN32_STREAM_ID sid;
		DWORD cb;
		WCHAR wsz[MAX_PATH * 2];

		// Enumerate the streams...
		BOOL bOK = TRUE;
		while (bOK) {
			// Calculate the size of the variable length
			// WIN32_STREAM_ID structure
			DWORD hdr_size = sizeof(sid) - sizeof(sid.cStreamName);
			bOK = BackupRead(hf, (BYTE*)&sid, hdr_size, &cb, FALSE, FALSE, &pctx);
			if (!cb) break;
			if (cb != hdr_size) break;

			BackupRead(hf, (BYTE*)wsz, sid.dwStreamNameSize, &cb, FALSE, FALSE, &pctx);
			if (cb != sid.dwStreamNameSize) break;

			if (sid.dwStreamId == BACKUP_ALTERNATE_DATA)
				cnt++;

			// Skip the stream body
			DWORD dw1, dw2;
			BackupSeek(hf, sid.Size.LowPart, sid.Size.HighPart, &dw1, &dw2, &pctx);
		}
		CloseHandle(hf);
	}
	return cnt;
}

void __fastcall _time_NTFS2FAT(FILETIME& ft)
{
	// 13:58:00.000 --> 13:58:00.000
	// 13:58:00.001 --> 13:58:02.000
	// 13:58:58.001 --> 13:59:00.000
	// 13:59:58.001 --> 14:00:00.000
	// 23:59:58.001 --> 00:00:00.000 (next day)
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);

	// 100ナノ秒 = 0.0001ミリ秒
	DWORDLONG ll = *((DWORDLONG*)&ft) / 10000000;
	if (st.wSecond & 1)
		ll += 1;
	else if (st.wMilliseconds)
		ll += 2;
	else
		return;

	ll *= 10000000;
	*((DWORDLONG*)&ft) = ll;
}

BOOL __fastcall WriteAnsiString(HANDLE hf, LPCTSTR psz, int cchMax)
{
	if (cchMax < 0)
		cchMax = lstrlen(psz);

	// UNICODE -> ANSI
	CStringA str(psz);
	DWORD dw;
	return WriteFile(hf, str.GetString(), str.GetLength(), &dw, NULL);
}

BOOL __fastcall RegGetAppPath(LPCTSTR pszAppName, LPTSTR pszPath)
{
	TCHAR buf[MAX_PATH * 2];
	wsprintf(buf, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s"), pszAppName);

	BOOL bOK = FALSE;
	CRegKey reg;
	if (reg.Open(HKEY_LOCAL_MACHINE, buf) == ERROR_SUCCESS) {
		ULONG len = ARRAYSIZE(buf);
		if (reg.QueryStringValue(NULL, buf, &len) == ERROR_SUCCESS && _pathfileexists(buf)) {
			lstrcpy(pszPath, buf);
			bOK = TRUE;
		}
		reg.Close();
	}
	return bOK;
}

void __fastcall InitDlgCtrl(HWND hctl, LPCTSTR pszResource)
{
	TCHAR buf[MAX_PATH * 2];
	if (GetClassName(hctl, buf, ARRAYSIZE(buf)) < 1) return;

	int type = -1;
	if (!StrCmpI(buf, WC_COMBOBOX))
		type = 1;
	else if (!StrCmpI(buf, WC_COMBOBOXEX))
		type = 2;

	CSimpleArray<CString> arr;
	int cnt = SeparateString(pszResource, _T(';'), arr);
	COMBOBOXEXITEM item;
	item.mask = CBEIF_TEXT;
	item.iItem = -1;
	for (int i = 0; i < cnt; i++) {
		switch (type) {
		case 1:
			::SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)arr[i].GetString());
			break;
		case 2:
			item.pszText = (LPTSTR)arr[i].GetString();
			SendMessage(hctl, CBEM_INSERTITEM, 0, (LPARAM)&item);
			break;
		}
	}
}

void EnumFileDir(LPCTSTR pszPath, LPCTSTR pszFilter, CSimpleArray<CString>& arrFile, CSimpleArray<CString>& arrFolder)
{
	// ファイルを検索
	CFileFinder ff;
	if (ff.BeginFind(pszPath, pszFilter)) {
		do {
			if (!ff.IsFolder())
				arrFile.Add(ff.cFileName);
			else if (!ff.IsDot())
				arrFolder.Add(ff.cFileName);
		} while (ff.NextFind());
		ff.EndFind();
	}
}

void FormatFromToString(LPCTSTR pszFrom, LPCTSTR pszTo, CString& str)
{
	CString fmt;
	fmt.LoadString(IDS_FROMTO);
	str.Format(fmt, pszFrom, pszTo);
}

COLORREF calc_mixed_color(COLORREF clrA, COLORREF clrB, int perA)
{
	// (9a + 1b)/10
	int r = (perA * GetRValue(clrA) + (100 - perA) * GetRValue(clrB)) / 100;
	int g = (perA * GetGValue(clrA) + (100 - perA) * GetGValue(clrB)) / 100;
	int b = (perA * GetBValue(clrA) + (100 - perA) * GetBValue(clrB)) / 100;
	return RGB(r, g, b);
}

UINT_PTR __ReadStringLength(IStream* pstrm, int& nCharSize)
{
	nCharSize = sizeof(char);

	// 1st, try to read a one-byte length
	ULONG len;
	BYTE bLength;
	pstrm->Read(&bLength, sizeof(bLength), &len);
	if (bLength < 0xff)
		return bLength;

	// 2nd, try a two-byte length
	WORD wLength;
	pstrm->Read(&wLength, sizeof(wLength), &len);
	if (wLength == 0xfffe) {
		// Unicode string.  Start over at 1-byte length
		nCharSize = sizeof(WCHAR);

		pstrm->Read(&bLength, sizeof(bLength), &len);
		if (bLength < 0xff)
			return bLength;

		// Two-byte length
		pstrm->Read(&wLength, sizeof(wLength), &len);
		// Fall through to continue on same branch as ANSI string
	}
	if (wLength < 0xffff)
		return wLength;

	// 3rd, 4-byte length
	DWORD dwLength;
	pstrm->Read(&dwLength, sizeof(dwLength), &len);
	if (dwLength < 0xffffffff)
		return dwLength;

	// 8-byte length
	ULONGLONG qwLength;
	pstrm->Read(&qwLength, sizeof(qwLength), &len);

	return (UINT_PTR)qwLength;
}

void ReadStringFromStream(IStream* pstrm, CString& str)
{
	str.Empty();

	ULONG len;

	int nCharSize;  // 1 = char, 2 = wchar_t
	UINT nLength = UINT(__ReadStringLength(pstrm, nCharSize) );
	if (nLength > 0) {
		if (nCharSize == sizeof(char)) {
			CStringA atr;
			char* pa = atr.GetBuffer(nLength + 1);
			pstrm->Read(pa, nLength * sizeof(char), &len);
			pa[len / sizeof(char)] = 0;
			str = pa;
		}
		else {
			LPTSTR p = str.GetBuffer(nLength + 1);
			pstrm->Read(p, nLength * sizeof(WCHAR), &len);
			p[len / sizeof(WCHAR)] = 0;
			str.ReleaseBuffer();
		}
	}
}

void __WriteStringLength(IStream* pstrm, UINT_PTR nLength, BOOL bUnicode)
{
	ULONG len;
	BYTE b;
	WORD w;
	DWORD d;
	ULONGLONG u;

	if (bUnicode) {
		// Tag Unicode strings
		b = 0xff;
		w = 0xfffe;
		pstrm->Write(&b, sizeof(b), &len);
		pstrm->Write(&w, sizeof(w), &len);
	}

	if (nLength < 255) {
		b = (BYTE)nLength;
		pstrm->Write(&b, sizeof(b), &len);
	}
	else if (nLength < 0xfffe) {
		b = (BYTE)0xff;
		w = (WORD)nLength;
		pstrm->Write(&b, sizeof(b), &len);
		pstrm->Write(&w, sizeof(w), &len);

	}
	else if (nLength < 0xffffffff) {
		b = (BYTE)0xff;
		w = (WORD)0xffff;
		d = (WORD)nLength;
		pstrm->Write(&b, sizeof(b), &len);
		pstrm->Write(&w, sizeof(w), &len);
		pstrm->Write(&d, sizeof(d), &len);
	}
	else {
		b = (BYTE)0xff;
		w = (WORD)0xffff;
		d = (WORD)nLength;
		u = (ULONGLONG)nLength;
		pstrm->Write(&b, sizeof(b), &len);
		pstrm->Write(&w, sizeof(w), &len);
		pstrm->Write(&d, sizeof(d), &len);
		pstrm->Write(&u, sizeof(u), &len);
	}
}

void WriteStringToStream(IStream* pstrm, CString& str)
{
	UINT_PTR nLength = (UINT_PTR)str.GetLength();
	__WriteStringLength(pstrm, nLength, sizeof(TCHAR) == sizeof(WCHAR));
	if (nLength > 0) {
		ULONG len;
		pstrm->Write((LPCTSTR)str, str.GetLength() * sizeof(TCHAR), &len);
	}
}

void FillShadeRect(HDC hdc, RECT* prc, COLORREF crFrom, COLORREF crTo, BOOL bVert)
{
	TRIVERTEX vert[2];
	GRADIENT_RECT grc;

	vert[0].x     = prc->left;
	vert[0].y     = prc->top;
	vert[0].Red   = GetRValue(crFrom) << 8;
	vert[0].Green = GetGValue(crFrom) << 8;
	vert[0].Blue  = GetBValue(crFrom) << 8;
	vert[0].Alpha = 0x0000;

	vert[1].x     = prc->right;
	vert[1].y     = prc->bottom;
	vert[1].Red   = GetRValue(crTo) << 8;
	vert[1].Green = GetGValue(crTo) << 8;
	vert[1].Blue  = GetBValue(crTo) << 8;
	vert[1].Alpha = 0x0000;

	grc.UpperLeft  = 0;
	grc.LowerRight = 1;
	GradientFill(hdc, vert, 2, &grc, 1, bVert ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

//
//	ディスクに書き込めるかを調べる
//
// nDriveNoは0～の数値で指定
// IOCTL_DISK_GET_CACHE_INFORMATIONによりディスク情報を取得する
// 対応OS：Windows 2000以降
//
bool GetDiskWriteProtectStatus(int nDriveNo, bool* pbWriteProtected)
{
	DWORD dwWritten;
	BOOL ret;
	HANDLE hDevice;

	if (pbWriteProtected == NULL)
		return false;

	CString strDevice;
	strDevice.Format(_T("\\\\.\\PHYSICALDRIVE%d"), nDriveNo);
	hDevice = ::CreateFile(strDevice, GENERIC_READ, FILE_SHARE_WRITE,NULL, OPEN_EXISTING, NULL, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
		return false;

	ret = ::DeviceIoControl(hDevice, IOCTL_DISK_IS_WRITABLE, NULL, 0, NULL, 0, &dwWritten, NULL);
	::CloseHandle(hDevice);

	if (ret == FALSE && GetLastError () != ERROR_WRITE_PROTECT)
		return false;

	*pbWriteProtected = (ret) ? false : true;
	return true;
}

//
//	ライト／リードキャッシュ設定状態の取得
//
// nDriveNoは0～の数値で指定
// IOCTL_DISK_GET_CACHE_INFORMATIONによりディスク情報を取得する
// 対応OS：Windows 2000以降
//
bool GetDiskCacheInformation(int nDriveNo, bool* pbWriteChachEnable, bool* pbReadChachEnable)
{
	DWORD dwWritten;
	BOOL ret;
	HANDLE hDevice;

	if (pbWriteChachEnable)
		*pbWriteChachEnable = false;
	if (pbReadChachEnable)
		*pbReadChachEnable = false;

	CString strDevice;
	strDevice.Format(_T("\\\\.\\PHYSICALDRIVE%d"), nDriveNo);
	hDevice = ::CreateFile(strDevice, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
		return	false;

	DISK_CACHE_INFORMATION sInfo;
	memset(&sInfo, 0, sizeof(DISK_CACHE_INFORMATION));
	ret = ::DeviceIoControl(hDevice, IOCTL_DISK_GET_CACHE_INFORMATION, NULL, 0, &sInfo, sizeof(DISK_CACHE_INFORMATION), &dwWritten, NULL);
	::CloseHandle(hDevice);

	if (ret == FALSE || dwWritten != sizeof(DISK_CACHE_INFORMATION))
		return false;

	if (pbWriteChachEnable)
		*pbWriteChachEnable = (sInfo.WriteCacheEnabled) ? true : false;
	if (pbReadChachEnable)
		*pbReadChachEnable = (sInfo.ReadCacheEnabled) ? true : false;

	return true;
}

BOOL ValidateFolderConnection(HWND hwnd, LPCTSTR pszFolder)
{
	// 注意：オーナーウィンドウを指定しないと、エラーメッセージや
	// アクセスダイアログは表示されない

	BOOL bOK = FALSE;
	LPITEMIDLIST pidl = ILCreateFromPath(pszFolder);
	if (pidl) {
		LPSHELLFOLDER psf = idl_GetFileFolder(pidl);
		if (psf) {
			LPENUMIDLIST pel;
			if (psf->EnumObjects(hwnd, SHCONTF_FOLDERS, &pel) == NOERROR) {
				bOK = TRUE;
				pel->Release();
			}
			psf->Release();
		}
		CoTaskMemFree(pidl);
	}
	return bOK;
}

HRESULT CoCreateInstanceAsAdmin(HWND hwnd, bool bAdmin, REFCLSID rclsid, REFIID riid, __out void ** ppv)
{
	if (bAdmin) {
		WCHAR  wszCLSID[80];
		StringFromGUID2(rclsid, wszCLSID, ARRAYSIZE(wszCLSID)); 
		CString moniker;
		moniker.Format(_T("Elevation:Administrator!new:%s"), wszCLSID);


		BIND_OPTS3 bo;
		memset(&bo, 0, sizeof(bo));
		bo.cbStruct = sizeof(bo);
		bo.hwnd = hwnd;
		bo.dwClassContext = CLSCTX_LOCAL_SERVER;
		HRESULT hr = CoGetObject(moniker, &bo, riid, ppv);
		return hr;
	}
	else
		return CoCreateInstance(rclsid, NULL, CLSCTX_INPROC, riid, ppv);
}

int wsz2utf(const WCHAR* wsz, char* utf, int cchMax)
{
	return WideCharToMultiByte(CP_UTF8, 0, wsz, -1, utf, cchMax, NULL, NULL);
}

int utf2wsz(const char* utf, WCHAR* wsz, int cchMax)
{
	return MultiByteToWideChar(CP_UTF8, 0, utf, -1, wsz, cchMax);
}

int utf2str(const char* utf, CString& str)
{
	str.Empty();
	int len = lstrlenA(utf) + 1;
	len = utf2wsz(utf, str.GetBuffer(len), len);
	str.ReleaseBuffer();
	return len;
}

////////////////////////////////////////////////////////////////////////
//

HBITMAP CreateVistaBitmapFromImageList(HIMAGELIST him, int index)
{
	int cx, cy;
	ImageList_GetIconSize(him, &cx, &cy);

	CClientDC dc(NULL);
	CDC dcw;
	dcw.CreateCompatibleDC(dc);

	// Create 32-bit bitmap
	BITMAPINFO bi = { 0 };
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = cx;
	bi.bmiHeader.biHeight = cy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	HBITMAP hbm = ::CreateDIBSection(dc, &bi, DIB_RGB_COLORS, NULL, NULL, 0);
	ATLASSERT(hbm != NULL);

	// Select bitmap into target DC and draw from image list to it
	if (hbm != NULL) {
		HBITMAP hbmw = dcw.SelectBitmap(hbm);

		IMAGELISTDRAWPARAMS ildp = { 0 };
		ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
		ildp.himl = him;
		ildp.i = index;
		ildp.hdcDst = dcw;
		ildp.x = 0;
		ildp.y = 0;
		ildp.cx = 0;
		ildp.cy = 0;
		ildp.xBitmap = 0;
		ildp.yBitmap = 0;
		ildp.fStyle = ILD_TRANSPARENT;
		ildp.fState = ILS_ALPHA;
		ildp.Frame = 255;
		::ImageList_DrawIndirect(&ildp);

		dcw.SelectBitmap(hbmw);
	}
	return hbm;
}

////////////////////////////////////////////////////////////////////////
//

typedef struct tagRCTOOLBARDATA {
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]
	WORD* items() { return (WORD*)(this+1); }
} RCTOOLBARDATA;

int CreateBitmapMapArray(UINT idResource, CSimpleMap<UINT, HBITMAP>& arr)
{
	// ツールバーデータから個々のビットマップを抽出してマップ配列を作成
	int cnt = 0;
	HINSTANCE hInst = RESINST;
	HRSRC hrsrc = ::FindResource(hInst, MAKEINTRESOURCE(idResource), RT_TOOLBAR);
	if (hrsrc) {
		HGLOBAL hmem = ::LoadResource(hInst, hrsrc);
		if (hmem) {
			RCTOOLBARDATA* pData = (RCTOOLBARDATA*)::LockResource(hmem);
			if (pData) {
				HIMAGELIST him = lccLoadImageList(hInst, RT_RCDATA, MAKEINTRESOURCE(idResource), 16, RGB(255, 0, 255));
				if (him) {
					WORD* pItems = pData->items();
					int nItems = pData->wItemCount;
					for (int i = 0, cnt = 0; i < pData->wItemCount; i++) {
						if (pItems[i] != 0) {
							HBITMAP hbm = CreateVistaBitmapFromImageList(him, cnt);
							arr.Add((UINT)pItems[i], hbm);
							cnt++;
						}
					}
				}
			}
		}
	}
	return cnt;
}


BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	BOOL bOK = FALSE;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		LUID luid;
		if (LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
			TOKEN_PRIVILEGES tp;
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			if (bEnablePrivilege)
				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			else
				tp.Privileges[0].Attributes = 0;

			// Enable the privilege or disable all privileges.
			bOK = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
			if (bOK) {
				if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
					// The token does not have the specified privilege.
					bOK = FALSE;
				}
			}
		}
		CloseHandle(hToken);
	}
	return bOK;
}

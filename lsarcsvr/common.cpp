////////////////////////////////////////////////////////////////////////
// common.cpp 共通関数

#include "stdafx.h"
#include <htmlhelp.h>
#include <htmlguid.h>
#include "common.h"
#include "tiphelp.h"


////////////////////////////////////////////////////////////////////
// Help ID bases

#pragma comment(lib, "htmlhelp.lib")

#define HID_BASE_COMMAND    0x00010000UL        // ID and IDM
#define HID_BASE_RESOURCE   0x00020000UL        // IDR and IDD
#define HID_BASE_PROMPT     0x00030000UL        // IDP
#define HID_BASE_NCAREAS    0x00040000UL
#define HID_BASE_CONTROL    0x00050000UL        // IDC
#define HID_BASE_DISPATCH   0x00060000UL        // IDispatch help codes

static DWORD sdwCookie = 0;
static TCHAR sszHtmlHelp[MAX_PATH];

void InitHtmlHelp()
{
	HWND hwndCapture = GetCapture();
	if (hwndCapture != NULL)
		SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

	if (!sdwCookie) {
		GetModuleFileName(ATL::_AtlBaseModule.GetModuleInstance(), sszHtmlHelp, ARRAYSIZE(sszHtmlHelp));
		PathRenameExtension(sszHtmlHelp, _T(".chm"));
		HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD_PTR)&sdwCookie);
	}
}

void __fastcall ShowHtmlHelp(HWND hwnd, UINT idDlg)
{
	InitHtmlHelp();
	HtmlHelp(hwnd, sszHtmlHelp, HH_HELP_CONTEXT, idDlg + HID_BASE_RESOURCE);
}

void __fastcall ShowCtrlHelp(HWND hwndCtrl, UINT idCtrl)
{
	HWND hwndCapture = GetCapture();
	if (hwndCapture != NULL)
		SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

	CPoint point;
	GetCursorPos(&point);
	CTipHelp tiphelp;
	if (tiphelp.Create(hwndCtrl) != NULL)
		tiphelp.Tracking(point, ATL::_AtlBaseModule.GetResourceInstance(), idCtrl);
}

///////////////////////////////////////////////////////////////////////
//

int WStrToStr(LPCWSTR pszWide, CStringA& str)
{
	int len = 0;
	str.Empty();
	if (pszWide && *pszWide) {
		len = lstrlenW(pszWide) * 2 + 32;
		LPSTR p = str.GetBuffer(len);
		WideCharToMultiByte(CP_ACP, 0, pszWide, -1, p, len, NULL, NULL);
		str.ReleaseBuffer();
		len = str.GetLength();
	}
	return len;
}

int StrToWStr(LPCSTR psz, CStringW& str)
{
	int len = 0;
	str.Empty();
	if (psz && *psz) {
		len = lstrlenA(psz) * 2 + 32;
		LPWSTR p = str.GetBuffer(len);
		MultiByteToWideChar(CP_ACP, 0, psz,  -1, p, len);
		str.ReleaseBuffer();
		len = str.GetLength();
	}
	return len;
}

int GetCheckedRadioButton(HWND hwnd, int nIDFirst, int nIDLast)
{
	for (int i = nIDFirst; i <= nIDLast; i++) {
		if (IsDlgButtonChecked(hwnd, i))
			return i;
	}
	return 0;
}

HGLOBAL GetDataFromDataObject(IDataObject* pdo, CLIPFORMAT type)
{
	HGLOBAL hmem = NULL;
	FORMATETC fe;
	SETFormatEtc(fe, type, TYMED_HGLOBAL, -1);
	if (pdo->QueryGetData(&fe) == S_OK) {
		STGMEDIUM sm;
		if (pdo->GetData(&fe, &sm) == S_OK) {
			SIZE_T size = GlobalSize(sm.hGlobal);
			hmem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, size);
			if (hmem) {
				LPVOID ps = GlobalLock(sm.hGlobal);
				LPVOID pd = GlobalLock(hmem);
				CopyMemory(pd, ps, size);
				GlobalUnlock(sm.hGlobal);
				GlobalUnlock(hmem);
			}
			ReleaseStgMedium(&sm);
		}
	}
	return hmem;
}

int AppMessage(UINT id, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString msg; msg.LoadString(id);
	if (pszInsert && *pszInsert)
		msg.Replace(_T("%1"), pszInsert);

	CString tit; tit.LoadString(IDS_PROJTITLE);
	return ::MessageBox(::GetActiveWindow(), (LPCTSTR)msg, (LPCTSTR)tit, nType);
}

int LastError(DWORD code, LPCTSTR pszInsert /*=NULL*/, UINT nTypeAction /*=MB_STOP*/)
{
	CString tit; tit.LoadString(IDS_PROJTITLE);
	return lccErrorBox(NULL, code, tit, pszInsert, nTypeAction);
}

#pragma pack(1)
typedef struct tagLOCALFILEHEADER {
    DWORD   lfhSignature;               // 0x04034B50
    WORD    lfhVerToExtract;
    WORD    lfhFlag;
    WORD    lfhMethod;
    WORD    lfhTime;
    WORD    lfhDate;
    DWORD   lfhCrc;
    DWORD   lfhSizeCompressed;
    DWORD   lfhSizeUncompressed;
    WORD    lfhFileNameLength;
    WORD    lfhExtraLength;
} LOCALFILEHEADER, *PLOCALFILEHEADER;
#pragma pack()

BOOL __iszipfile(LPCTSTR pszName)
{
	if (!pszName || *pszName == '\0')
		return FALSE;

	HANDLE hf = CreateFile(pszName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL bOK = FALSE;
	DWORD ss = GetFileSize(hf, NULL);
	HANDLE hm = CreateFileMapping(hf, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hm != NULL) {
		BYTE* pBase = (BYTE*)MapViewOfFile(hm, FILE_MAP_READ, 0, 0, 0);
		if (pBase) {
			register BYTE* pb = pBase;
			register DWORD s = ss;
			while (s) {
				if (s < sizeof(LOCALFILEHEADER))
					break;

				if (*((DWORD*)pb) == 0x04034B50) {
					bOK = TRUE;
					break;
				}
				pb++;
				s--;
			}
			UnmapViewOfFile(pBase);
		}
		CloseHandle(hm);
	}
	CloseHandle(hf);
	return bOK;
}

BOOL __fastcall FileBrowse(BOOL bRead, HWND hwnd, UINT nIDCaption, UINT nIDFilter, LPTSTR pszName)
{
	TCHAR szFileName[MAX_PATH], szFileTitle[MAX_PATH], szInitDir[MAX_PATH];
	szFileName[0] = szFileTitle[0] = szInitDir[0] = '\0';

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrDefExt = NULL;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;

	TCHAR szFilter[MAX_PATH], szTitle[MAX_PATH];

	HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
	LoadString(hInst, nIDCaption, szTitle, ARRAYSIZE(szTitle));
	ofn.lpstrTitle = szTitle;
	LoadString(hInst, nIDFilter, szFilter, ARRAYSIZE(szFilter));
	szFilter[lstrlen(szFilter) + 1] = '\0';
	LPTSTR p = szFilter;
	while (p = StrChr(p, '|'))
		*p++ = '\0';
	ofn.lpstrFilter = szFilter;
	if (pszName && *pszName) {
		lstrcpy(szInitDir, pszName);
		if (!PathIsDirectory(szInitDir)) {
			PathRemoveFileSpec(szInitDir);
			lstrcpy(szFileName, pszName);
		}
	}

	ofn.lpstrInitialDir = szInitDir;

	int nResult;
	if (bRead)	nResult = GetOpenFileName(&ofn);
	else		nResult = GetSaveFileName(&ofn);

	if (nResult)
		lstrcpy(pszName, ofn.lpstrFile);
	return nResult;
}


#define BFC_TREE		0x3741
#define BFC_TITLE		0x3742
#define BFC_PATH		0x3743
#define BFC_EDITBOX		0x3744
#define DONT_DISMISS		1

static int CALLBACK bcproc(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
	switch (msg) {
		case BFFM_INITIALIZED:
			if (lpData && PathFileExists((LPCTSTR)lpData))
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			break;

		case BFFM_SELCHANGED:
		{
			TCHAR buf[MAX_PATH * 2];
			BOOL bOK = SHGetPathFromIDList((LPITEMIDLIST)lParam, buf);
			SendMessage(hwnd, BFFM_ENABLEOK, 0, bOK);
			break;
		}

		case BFFM_VALIDATEFAILEDA:
		case BFFM_VALIDATEFAILEDW:
			return DONT_DISMISS;

		default:
			break;
	}
	return 0;
}

int __fastcall BrowseForFolder(HWND hwnd, LPTSTR pszPath)
{
	BOOL bOK = FALSE;
	TCHAR title[MAX_PATH];
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = hwnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = title;
	bi.lpszTitle = _T("フォルダを選択してください。");
	bi.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS | BIF_VALIDATE;
	bi.lpfn = bcproc;
	bi.lParam = (LPARAM)pszPath;

	TCHAR dir[MAX_PATH * 2];
	GetCurrentDirectory(ARRAYSIZE(dir), dir);
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	SetCurrentDirectory(dir);

	if (pidl) {
		if (SHGetPathFromIDList(pidl, pszPath)) {
			SHFILEINFO sfi;
			sfi.dwAttributes = SFGAO_LINK;
			SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
			if (sfi.dwAttributes & SFGAO_LINK) {
				LINKINFO li;
				if (ParseLinkInfo(pszPath, &li))
					StrCpy(pszPath, li.szTargetName);
			}
			bOK = TRUE;
		}
		IMalloc* pm;
		SHGetMalloc(&pm);
		pm->Free(pidl);
		pm->Release();
	}
	return bOK;
}

static BOOL _getlinkinfo(IShellLink* ps, IPersistFile* pp, LPCTSTR pszName, LPLINKINFO pli)
{
	if (FAILED(pp->Load(pszName, STGM_READ)))
		return FALSE;

	if (FAILED(ps->GetPath(pli->szTargetName, ARRAYSIZE(pli->szTargetName), &pli->wfd, 0)))
		pli->szTargetName[0] = '\0';

	if (FAILED(ps->GetWorkingDirectory(pli->szWorkDir, ARRAYSIZE(pli->szWorkDir))))
		pli->szWorkDir[0] = '\0';

	if (FAILED(ps->GetDescription(pli->szDescription, ARRAYSIZE(pli->szDescription))))
		pli->szDescription[0] = '\0';

	return TRUE;
}

BOOL __fastcall ParseLinkInfo(LPCTSTR pszName, LINKINFO* pli)
{
	BOOL bOK = FALSE;
	IShellLink* psl;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hr)) {
		IPersistFile* ppf;
		hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if (SUCCEEDED(hr)) {
			bOK = _getlinkinfo(psl, ppf, pszName, pli);
			ppf->Release();
		}
		psl->Release();
	}
	return bOK;
}

BOOL __fastcall GetPathCommonPrefix(HDROP hdrop, LPTSTR pszPrefix)
{
	if (!hdrop) return FALSE;

	int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	if (!cnt) return FALSE;

	// エントリの共通のパスを検証する
	CString str;
	TCHAR buf[MAX_PATH * 2], prx[MAX_PATH * 2];
	for (int i = 0; i < cnt; i++) {
		DragQueryFile(hdrop, i, buf, ARRAYSIZE(buf));
		PathRemoveFileSpec(buf);
		if (i == 0)
			str = buf;
		else {
			if (!str.IsEmpty()) {
				if (PathCommonPrefix(str, buf, prx) > 0)
					str = prx;
				else
					str.Empty();
			}
		}

		if (str.IsEmpty())
			return FALSE;
	}

	lstrcpy(pszPrefix, str);
	return TRUE;
}

bool MakeTempFile(HDROP hDrop, LPTSTR pszPath, int pathMode, bool bDos, int nArcType, bool bUnicode, bool bUtf8, LPCTSTR pszBasePath)
{
	if (!hDrop) return false;

	int cnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if (!cnt) return false;

	// エントリの共通のパスを検証する
	TCHAR buf[MAX_PATH * 2];
	CString prx;
	if (pszBasePath && *pszBasePath)
		prx = pszBasePath;
	else if (GetPathCommonPrefix(hDrop, buf))
		prx = buf;

	GetTempPath(ARRAYSIZE(buf), buf);
	PathCombine(pszPath, buf, _T("~lsarc.tmp"));

	HANDLE hf = CreateFile(pszPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf == INVALID_HANDLE_VALUE) {
		DeleteFile(pszPath);
		return false;
	}

	DWORD dw;
	if (bUnicode) {
		// UTF16LE BOM
		WORD bom = 0xfeff;
		WriteFile(hf, &bom, sizeof(bom), &dw, NULL);
	}

	TCHAR tmp[MAX_PATH * 2];
	BOOL bDir;
	int i;
	for (i = 0; i < cnt; i++) {
		DragQueryFile(hDrop, i, buf, ARRAYSIZE(buf));
		bDir = PathIsDirectory(buf);
		if (bDos)
			GetShortPathName(buf, buf, ARRAYSIZE(buf));

		if (pathMode != BASEDIR_ABSOLUTE) {
			if (!prx.IsEmpty()) {
				CString str = buf + prx.GetLength();
				LPCTSTR p = str;
				if (*p == '\\')
					p++;
				lstrcpy(buf, p);
			}
		}

		switch (nArcType) {
		case ARC_TAR:
		case ARC_ZIP:
		case ARC_LZH:
			break;
		case ARC_ISO:
		case ARC_RPM:
		case ARC_7ZIP:
			if (pathMode == BASEDIR_ABSOLUTE) {
				lstrcpy(tmp, PathSkipRoot(buf));
				lstrcpy(buf, tmp);
			}
			if (bDir)
				lstrcat(buf, _T("\\*"));
			break;
		default:
			if (bDir)
				lstrcat(buf, _T("\\*.*"));
			break;
		}
		PathQuoteSpaces(buf);
		lstrcat(buf, _T("\x0d\x0a"));

		if (bUnicode) {
			// UNICODE で GO
			WriteFile(hf, buf, lstrlen(buf) * sizeof(WCHAR), &dw, NULL);
		}
		else {
			char ssz[MAX_PATH * 2];
			if (bUtf8) {
				// UNICODE -> UTF8変換
				EncodeWStrToUTF(buf, -1, (BYTE*)ssz, sizeof(ssz));
			}
			else {
				// UNICODE -> ANSI変換
				WideCharToMultiByte(CP_ACP, 0, buf, -1, ssz, sizeof(ssz), NULL, NULL);
			}
			WriteFile(hf, ssz, lstrlenA(ssz), &dw, NULL);
		}
	}
	CloseHandle(hf);
	return true;
}

LPTSTR __fastcall _strcombine(LPTSTR pszDst, LPCTSTR pszSrc1, LPCTSTR pszSrc2)
{
	if (pszDst && pszSrc1 && pszSrc2) {
		lstrcpy(pszDst, pszSrc1);
		lstrcat(pszDst, pszSrc2);
		return pszDst;
	}
	return NULL;
}

LPTSTR __fastcall _addchar(LPTSTR psz, TCHAR ch)
{
	if (psz && ch) {
		int len = lstrlen(psz);
		psz[len] = ch;
		psz[len + 1] = '\0';
		return psz;
	}
	return NULL;
}

int ComparePath(LPCTSTR pszSrc, LPCTSTR pszDst)
{
	LPTSTR p;
	CString s, t;
	p = s.GetBuffer(MAX_PATH * 2); lstrcpy(p, pszSrc); PathRemoveBackslash(p); s.ReleaseBuffer(); s.Replace(_T('\\'), _T('\t'));
	p = t.GetBuffer(MAX_PATH * 2); lstrcpy(p, pszDst); PathRemoveBackslash(p); t.ReleaseBuffer(); t.Replace(_T('\\'), _T('\t'));
	return s.CompareNoCase(t);
}

BOOL PathBuild(LPCTSTR pszSrc, LPCTSTR pszBase, LPTSTR pszDst)
{
	if (!pszSrc || *pszSrc == 0 || !pszBase || *pszBase == 0 || !pszDst)
		return FALSE;

	// 作成ファイル名の解析
	TCHAR buf[MAX_PATH * 2], w[MAX_PATH * 2];
	lstrcpy(buf, pszSrc);
	if (!PathIsUNC(buf)) {								// ネットワークではない
		if (buf[0] == '.') {							// 相対パス
			lstrcpy(w, pszBase);
			PathAppend(w, buf);
			PathCanonicalize(buf, w);
		}
		else if (buf[0] == '\\') {						// ルートから
			lstrcpy(w, pszBase);
			PathStripToRoot(w);
			PathAppend(w, &buf[1]);
			lstrcat(buf, w);
		}
		else if (lstrlen(buf) < 2 || buf[1] != ':') {	// アペンド
			lstrcpy(w, pszBase);
			PathAppend(w, buf);
			lstrcpy(buf, w);
		}
	}
	lstrcpy(pszDst, buf);
	return TRUE;
}

FARPROC rGetProcAddress(HMODULE hmod, LPCTSTR pszProc)
{
#ifdef UNICODE
	char sz[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pszProc, -1, sz, ARRAYSIZE(sz), NULL, NULL);
	return GetProcAddress(hmod, sz);
#else
	return GetProcAddress(hmod, pszProc);
#endif
}


BOOL GetLinkTarget(LPCTSTR pszLinkFile, LPTSTR pszTarget, int cchMax)
{
	BOOL bOK = FALSE;
	IShellLink* psl;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hr)) {
		IPersistFile* ppf;
		hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if (SUCCEEDED(hr)) {
			hr = ppf->Load(pszLinkFile, STGM_READ);
			if (SUCCEEDED(hr)) {
				WIN32_FIND_DATA wfd;
				hr = psl->GetPath(pszTarget, cchMax, &wfd, 0);
				if (SUCCEEDED(hr))
					bOK = TRUE;
			}
			ppf->Release();
		}
		psl->Release();
	}
	return bOK;
}


BOOL exec_console_app(LPCTSTR pszCmdLine, LPCTSTR pszDir, LPCTSTR pszConsoleTitle)
{
	if (!pszCmdLine) return FALSE;

	TCHAR app[MAX_PATH * 3], cmd[MAX_PATH * 3], *p;
	cmd[0] = '\0';
	if (*pszCmdLine == '"') {
		lstrcpy(app, pszCmdLine + 1);
		p = StrChr(app, '"');
		if (p) {
			*p++ = '\0';
			while (*p && *p == ' ') p++;
			lstrcpy(cmd, p);
		}
	}
	else {
		lstrcpy(app, pszCmdLine);
		p = StrChr(app, ' ');
		if (p) {
			*p++ = '\0';
			while (*p && *p == ' ') p++;
			lstrcpy(cmd, p);
		}
	}

	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	BOOL bOK = CreateProcess(app, cmd, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, pszDir, &si, &pi);
	if (bOK) {
		// スレッド終了を待つ
		WaitForSingleObject(pi.hThread, INFINITE);
		CloseHandle(pi.hThread);
	}
	return bOK;
}

void __fastcall InitDlgCtrl(HWND hctl, UINT idResource)
{
	TCHAR buf[MAX_PATH * 2];
	if (GetClassName(hctl, buf, ARRAYSIZE(buf)) < 1) return;
	CString str;
	if (str.LoadString(idResource) < 1) return;

	int type = -1;
	if (!StrCmpI(buf, WC_COMBOBOX))
		type = 1;
	else if (!StrCmpI(buf, WC_COMBOBOXEX))
		type = 2;

	lstrcpy(buf, str);
	LPTSTR p = buf;
	COMBOBOXEXITEM item;
	while (p && *p) {
		LPTSTR q = StrChr(p, ';');
		if (q) *q++ = 0;

		switch (type) {
		case 1:
			::SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)p);
			break;
		case 2:
			item.mask = CBEIF_TEXT;
			item.iItem = -1;
			item.pszText = p;
			SendMessage(hctl, CBEM_INSERTITEM, 0, (LPARAM)&item);
			break;
		}
		p = q;
	}
}

void __fastcall SetDialogIcon(HWND hdlg, UINT idIcon)
{
	HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
	HICON hic;
	hic = (HICON)LoadImage(hInst, _MKIR(idIcon), IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SendMessage(hdlg, WM_SETICON, FALSE, (LPARAM)hic);
	hic = (HICON)LoadImage(hInst, _MKIR(idIcon), IMAGE_ICON,
							GetSystemMetrics(SM_CXICON),
							GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SendMessage(hdlg, WM_SETICON, TRUE, (LPARAM)hic);
}

void __fastcall RemoveDialogIcon(HWND hdlg)
{
	HICON hic = (HICON)SendMessage(hdlg, WM_GETICON, TRUE, 0);
	DestroyIcon(hic);
	hic = (HICON)SendMessage(hdlg, WM_GETICON, FALSE, 0);
	DestroyIcon(hic);
}

static const TCHAR envPath[] = _T("Path");

static void AddPathEnvironment(LPCTSTR pszAdd)
{
	BOOL (WINAPI* _SetDllDirectory)(LPCWSTR);
	(FARPROC&)_SetDllDirectory = ::GetProcAddress(::GetModuleHandle(_T("kernel32")), "SetDllDirectoryW");
	if (_SetDllDirectory) {
		_SetDllDirectory(pszAdd);
		return;
	}

	TCHAR path[MAX_PATH], spath[MAX_PATH];
	lstrcpy(path, pszAdd);
	PathRemoveBackslash(path);
	GetShortPathName(path, spath, ARRAYSIZE(spath));

	DWORD size = GetEnvironmentVariable(envPath, NULL, 0);
	if (size > 0) {
		CString env;
		LPTSTR p = env.GetBuffer(size + 16);
		GetEnvironmentVariable(envPath, p, size + 16);
		env.ReleaseBuffer();

		int pos = 0;
		CString token = env.Tokenize(_T(";"), pos);
		while (token != _T("")) {
			TCHAR t[MAX_PATH];
			lstrcpy(t, token);
			PathRemoveBackslash(t);
			if (!lstrcmpi(t, path) || !lstrcmpi(t, spath))
				return;

			token = env.Tokenize(_T(";"), pos);
		}

		CString newEnv = path;
		newEnv += _T(';');
		newEnv += env;
		SetEnvironmentVariable(envPath, newEnv);
	}
}

void AddMyPathEnvironment()
{
	TCHAR path[MAX_PATH * 2];
	GetModuleFileName(ATL::_AtlBaseModule.GetModuleInstance(), path, ARRAYSIZE(path));
	PathRemoveFileSpec(path);
	AddPathEnvironment(path);
}

BOOL CopyTextToClipboard(HWND hwnd, LPCTSTR pszText)
{
	BOOL bOK = FALSE;
	if (pszText && *pszText && OpenClipboard(hwnd)) {
		EmptyClipboard();

		DWORD dwSize = (lstrlen(pszText) + 1) * sizeof(TCHAR);
		HGLOBAL hmem = GlobalAlloc(GHND, dwSize);
		if (hmem) {
			LPTSTR p = (LPTSTR)GlobalLock(hmem);
			lstrcpy(p, pszText);
			GlobalUnlock(hmem);
			SetClipboardData(CF_UNICODETEXT, hmem);
			bOK = TRUE;
		}
		CloseClipboard();
	}
	return bOK;
}

LPTSTR FormatFileDateTime(FILETIME* pft, LPTSTR psz, int cchMax)
{
	FILETIME lft;
	FileTimeToLocalFileTime(pft, &lft);
	SYSTEMTIME st;
	FileTimeToSystemTime(&lft, &st);

	wsprintf(psz, _T("%02d/%02d/%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
/*@@@
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, psz, cchMax);
	int len = lstrlen(psz);
	psz[len++] = _T(' ');
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, psz + len, cchMax - len);
@@@ */
	return psz;
}

void iia2iiw(INDIVIDUALINFO* iia, INDIVIDUALINFOW* iiw, bool utf8)
{
	iiw->dwOriginalSize = iia->dwOriginalSize;
	iiw->dwCompressedSize = iia->dwCompressedSize;
	iiw->dwCRC = iia->dwCRC;
	iiw->uFlag = iia->uFlag;
	iiw->uOSType = iia->uOSType;
	iiw->wRatio = iia->wRatio;
	iiw->wDate = iia->wDate;
	iiw->wTime = iia->wTime;
	if (utf8) {
		DecodeUTFToWStr((BYTE*)iia->szFileName,  -1, iiw->szFileName,  ARRAYSIZE(iiw->szFileName));
		DecodeUTFToWStr((BYTE*)iia->szAttribute, -1, iiw->szAttribute, ARRAYSIZE(iiw->szAttribute));
		DecodeUTFToWStr((BYTE*)iia->szMode,      -1, iiw->szMode,      ARRAYSIZE(iiw->szMode));
	}
	else {
		MultiByteToWideChar(CP_ACP, 0, iia->szFileName,  -1, iiw->szFileName,  ARRAYSIZE(iiw->szFileName));
		MultiByteToWideChar(CP_ACP, 0, iia->szAttribute, -1, iiw->szAttribute, ARRAYSIZE(iiw->szAttribute));
		MultiByteToWideChar(CP_ACP, 0, iia->szMode,      -1, iiw->szMode,      ARRAYSIZE(iiw->szMode));
	}
}

void iiw2iia(INDIVIDUALINFOW* iiw, INDIVIDUALINFO* iia)
{
	iia->dwOriginalSize = iiw->dwOriginalSize;
	iia->dwCompressedSize = iiw->dwCompressedSize;
	iia->dwCRC = iiw->dwCRC;
	iia->uFlag = iiw->uFlag;
	iia->uOSType = iiw->uOSType;
	iia->wRatio = iiw->wRatio;
	iia->wDate = iiw->wDate;
	iia->wTime = iiw->wTime;
	WideCharToMultiByte(CP_ACP, 0, iiw->szFileName,  -1, iia->szFileName,  ARRAYSIZE(iia->szFileName),  NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, iiw->szAttribute, -1, iia->szAttribute, ARRAYSIZE(iia->szAttribute), NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, iiw->szMode,      -1, iia->szMode,      ARRAYSIZE(iia->szMode),      NULL, NULL);
}

// JavaScriptのsplitを再現した関数 (std::vector<CString> を返す)
CSimpleArray<CString> Split(LPCTSTR target, LPCTSTR delimiter) {
	CSimpleArray<CString> tokens;
	CString str(target);

	// 1. 区切り文字が空の場合は、1文字ずつに分解する (JSの split("") の挙動)
	if (delimiter == nullptr || *delimiter == _T('\0')) {
		int len = lstrlen(target);
		for (int i = 0; i < len; ++i) {
			tokens.Add(str.Mid(i, 1));
		}
		return tokens;
	}

	// 2. 通常の分割処理
	int pos = 0;
	CString tok = str.Tokenize(delimiter, pos);
	while (!tok.IsEmpty()) {
		tokens.Add(tok);
		tok = str.Tokenize(delimiter, pos);
	}
	return tokens;
}

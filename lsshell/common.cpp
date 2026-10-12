//////////////////////////////////////////////////////////////////////
// common.cpp

#include "stdafx.h"
#include <shellapi.h>
#include <shlobj.h>
#include "mainmodule.h"
#include "..\lscore\regapp.h"
#include "..\lscore\gdiplus_sup.h"
#include "lsarcsvr_i.h"
#include "common.h"
#include "resource.h"
#include "..\lsftc\lsftc_i.h"
#include "tiphelp.h"

static const TCHAR regKey[] = _T("UserName");

#ifndef _INC_SHLWAPI
#pragma pack(1)
	typedef struct _DllVersionInfo
	{
		DWORD cbSize;
		DWORD dwMajorVersion;                   // Major version
		DWORD dwMinorVersion;                   // Minor version
		DWORD dwBuildNumber;                    // Build number
		DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
	} DLLVERSIONINFO;

	typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

	// Platform IDs for DLLVERSIONINFO
	#define DLLVER_PLATFORM_WINDOWS         0x00000001      // Windows 95
	#define DLLVER_PLATFORM_NT              0x00000002      // Windows NT
#pragma pack()
#endif

int AppMessage(ATL::_U_STRINGorID title, ATL::_U_STRINGorID mes, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString tit;
	if (IS_INTRESOURCE(title.m_lpstr)) {
		tit.LoadString(PtrToUint(title.m_lpstr));
	} else {
		tit = title.m_lpstr;
	}
	CString msg;
	if (IS_INTRESOURCE(mes.m_lpstr)) {
		if (pszInsert) {
			msg.Format(PtrToUint(mes.m_lpstr), pszInsert);
		} else {
			msg.LoadString(PtrToUint(mes.m_lpstr));
		}
	} else {
		if (pszInsert) {
			msg.Format(mes.m_lpstr, pszInsert);
		} else {
			msg = mes.m_lpstr;
		}
	}
	return AtlMessageBox(::GetActiveWindow(), msg.GetString(), tit.GetString(), nType);
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

int PathComparePath(LPCTSTR pathParent, LPCTSTR pathChild)
{
	// 戻り値
	// -1=パスは無関係
	// 0=パスが一致
	// 1=pathParent は pathChild の直近の親
	// 2=pathParent は pathChild の直近ではない親

	if (!pathParent || (*pathParent == 0) || !pathChild || (*pathChild == 0))
		return -1;

	int type = -1;
	LPITEMIDLIST pidlParent = ILCreateFromPath(pathParent);
	if (pidlParent) {
		LPITEMIDLIST pidlChild = ILCreateFromPath(pathChild);
		if (pidlChild) {
			if (ILIsEqual(pidlParent, pidlChild))
				type = 0;
			else {
				// 親かチェック
				if (ILIsParent(pidlParent, pidlChild, FALSE)) {
					// 直近の親かチェック
					if (ILIsParent(pidlParent, pidlChild, TRUE))
						type = 1;
					else
						type = 2;
				}
			}
			ILFree(pidlChild);
		}
		ILFree(pidlParent);
	}
	return type;
}

BOOL CompressFiles(HWND hParent, LPCTSTR pszPath, HDROP hDrop)
{
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->CompressFilesExec(hParent, pszPath, (LONG_PTR)hDrop);
	}
else {
	CString str;
	str.Format(_T("cannot create instance. code=%08X"), hr);
	MessageBox(hParent, str, NULL, MB_OK | MB_ICONSTOP);
}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL ExtractFiles(HWND hParent, LPCTSTR pszPath, HDROP hDrop)
{
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->ExtractFiles(hParent, pszPath, (LONG_PTR)hDrop);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL PackAddFiles(HWND hParent, LPCTSTR pszArc, LPCTSTR pszBase, HDROP hdrop)
{
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->AddFiles(hParent, pszArc, pszBase, (LONG_PTR)hdrop);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL MakeSFX(HWND hParent, LPCTSTR pszArc, LPCTSTR pszPath)
{
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->MakeSFX(hParent, pszArc, pszPath);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL CheckArchive(HWND hParent, HDROP hdrop)
{
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->CheckFiles(hParent, (LONG_PTR)hdrop);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

#define LIMIT_OF_TRIAL_DATES			30
static int snUserLevel = USRL_NOCHECK;

int _check_user()
{
	CRegApp ra;
	ra.SetApplication(APP_COMPANY, APP_PRODUCT);

	TCHAR buf[MAX_PATH * 2];
#if 0
	if (snUserLevel == USRL_NOCHECK) {
		if (ra.GetString(RGI_USERINFO, regKey, buf, ARRAYSIZE(buf)) < 1) {
			snUserLevel = USRL_NOUSER;
			return snUserLevel;
		}
	}
#endif

	if (snUserLevel != USRL_VALID) {
		if (ra.GetStringJum(RGI_USERINFO, RGI_LICENSE, (char*)buf) > 0)
			snUserLevel = USRL_VALID;
		else {
			if (snUserLevel == USRL_NOCHECK) {
				long limits = LIMIT_OF_TRIAL_DATES;
				CComPtr<IFileXferComponent> pt;
				if (pt.CoCreateInstance(__uuidof(FileXferComponent)) == S_OK) {
					if (pt->GetInstance(&limits) != S_OK)
						limits = 0;
				}
				snUserLevel = (limits <= 0 || LIMIT_OF_TRIAL_DATES < limits) ? USRL_EXPIRE : USRL_TRIAL;
			}
		}
	}
	return snUserLevel;
}

void _fillrect(HDC hdc, LPRECT prc, COLORREF rgb)
{
	HGDIOBJ hobj = SelectObject(hdc, CreateSolidBrush(rgb));
	PatBlt(hdc, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top, PATCOPY);
	DeleteObject(SelectObject(hdc, hobj));
}

LPITEMIDLIST getlinktarget(LPCTSTR pszName)
{
	CComPtr<IShellLink> psl;
	if (psl.CoCreateInstance(CLSID_ShellLink) == S_OK) {
		CComQIPtr<IPersistFile, &IID_IPersistFile> ppf(psl);
		if (ppf.p != NULL) {
			if (ppf->Load(pszName, STGM_READ) == S_OK) {
				LPITEMIDLIST pidl;
				if (NOERROR == psl->GetIDList(&pidl)) {
					return pidl;
				}
			}
		}
	}
	return NULL;
}

int notify_message(UINT idTitle, UINT idText, LPCTSTR pszInsert, UINT nType)
{
	MessageBeep(0);

	CString tit;
	tit.LoadString(idTitle);

	CString mes;
	if (pszInsert && *pszInsert) {
		mes = pszInsert;
		mes += _T('\n');
	}
	else
		mes.Empty();
	CString str;
	str.LoadString(idText);
	mes += str;
	return MessageBox(GetActiveWindow(), mes, tit, nType);
}

BOOL dobj_getfilename(IDataObject* pdo, LPTSTR pszName)
{
	FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM sm;

	*pszName = '\0';
	if (pdo && SUCCEEDED(pdo->GetData(&fmte, &sm))) {
		if (sm.hGlobal) {
			if (DragQueryFile((HDROP)sm.hGlobal, (UINT)-1, 0, 0) == 1)
				DragQueryFile((HDROP)sm.hGlobal, 0, pszName, MAX_PATH * 2);
		}
		ReleaseStgMedium(&sm);
	}
	return (lstrlen(pszName) > 0) ? TRUE : FALSE;
}

void replace_str(LPTSTR pText, LPCTSTR pSrc, LPCTSTR pDst)
{
	TCHAR* p = pText;
	TCHAR* pRep = new TCHAR[MAX_PATH * 4];
	memset(pRep, 0, sizeof(TCHAR) * MAX_PATH * 4);
	TCHAR* q = pRep;
	while (*p) {
		if (*p != *pSrc || StrCmpN(p, pSrc, lstrlen(pSrc)))
			*q++ = *p++;
		else {
			lstrcpy(q, pDst);
			q += lstrlen(pDst);
			p += lstrlen(pSrc);
		}
	}
	*q = '\0';
	lstrcpy(pText, pRep);
	delete [] pRep;
}

BOOL checkmedia(LPCTSTR pszDriveName)
{
	UINT mode = SetErrorMode(SEM_FAILCRITICALERRORS);
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, pszDriveName);
	lstrcat(buf, _T("."));
	DWORD dw = GetFileAttributes(buf);
	SetErrorMode(mode);
	return (dw != 0xFFFFFFFF);
}

void ShowCtrlHelp(HWND hwnd, UINT idText)
{
/* @@@
	CString str;
	str.LoadString(idText);
	lccPopView(hwnd, NULL, str, style);
@@@ */
	CTipHelp tiphelp;
	tiphelp.ShowTips(hwnd, idText);
}

LPTSTR MakeFileNames(HDROP hdrop)
{
	if (!hdrop)
		return NULL;

	int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	if (cnt < 1)
		return NULL;

	TCHAR buf[MAX_PATH * 2];
	int len, size = 0;
	LPTSTR p = NULL, pw;
	for (int i = 0; i < cnt; i++) {
		if (DragQueryFile(hdrop, i, buf, ARRAYSIZE(buf)) > 0) {
			len = lstrlen(buf) + 2;
			if (p)
				pw = (LPTSTR)GlobalReAllocPtr(p, size + len * sizeof(TCHAR), GHND);
			else
				pw = (LPTSTR)GlobalAllocPtr(GHND, len * sizeof(TCHAR));
			if (!pw)
				break;
			p = pw;
			lstrcpy(p + size, buf);
			size += (len - 1);
		}
	}

	if (p)
		p[size] = '\0';
	return p;
}

HWND __fastcall GetTopParent(HWND hwnd)
{
	HWND hParent = GetParent(hwnd);
	while (hParent) {
		hwnd = hParent;
		hParent = GetParent(hwnd);
	}
	return hwnd;
}

BOOL IsValidExecuteModule()
{
#ifndef _WIN64
	HWND hwnd = GetActiveWindow();
	hwnd = GetTopParent(hwnd);
	if (IsWindow(hwnd)) {
		char name[MAX_PATH];
		if (GetModuleFileNameA((HMODULE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), name, ARRAYSIZE(name)) > 0) {
			char* p = PathFindFileNameA(name);
			if (p) {
				CharLowerA(p);
				switch (*(DWORD*)p) {
				case 0x656d616d:	// mame まめ
				case 0x33636174:	// tac3 卓
				case 0x65737063:	// cpse 卓DLL
				case 0x7a656573:	// seez SeeZ
				case 0x652e6678:	// X-Finder
				case 0x66736173:	// SuperASF
				case 0x2e786661:	// あふ
				case 0x7865646d:	// Multi Document Explorer
				case 0x70687366:	// FileShelf Plus
				case 0x69666f65:	// Environment of Operating File (EOFile)
//				case 0x666e6977:	// WinFD ってＸ
					return FALSE;
				}
			}
		}
	}
#endif
	return TRUE;
}

int WINAPIV key_combine(LPTSTR dst, LPCTSTR psz, ... )
{
	LPCTSTR p = psz;
	*dst = 0;
	va_list marker;
	va_start(marker, psz);
	while (p && *p) {
		if (*dst == 0)
			lstrcpy(dst, p);
		else
			PathAppend(dst, p);
		p = va_arg(marker, LPCTSTR);
	}
	va_end(marker);
	return lstrlen(dst);
}

void GetMyModulePath(LPTSTR psz, int cchMax)
{
	*psz = 0;
	GetModuleFileName(ATL::_AtlBaseModule.GetModuleInstance(), psz, cchMax);
	PathRemoveFileSpec(psz);
}

void GetMyModulePath(CString& path)
{
	GetMyModulePath(path.GetBuffer(MAX_PATH * 2), MAX_PATH * 2);
	path.ReleaseBuffer();
}

HMODULE LoadMyLibrary(LPCTSTR pszDll)
{
	CPath path;
	GetMyModulePath(path);
	path.Append(pszDll);
	return LoadLibrary(path);
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

		bool found = false;
		int pos = 0;
		CString token = env.Tokenize(_T(";"), pos);
		while (token != _T("")) {
			TCHAR t[MAX_PATH];
			lstrcpy(t, token);
			PathRemoveBackslash(t);
			if (!lstrcmpi(t, path) || !lstrcmpi(t, spath)) {
				found = true;
				break;
			}
			token = env.Tokenize(_T(";"), pos);
		}

		if (!found) {
			CString newEnv = path;
			newEnv += _T(';');
			newEnv += env;
			SetEnvironmentVariable(envPath, newEnv);
		}
	}
}

void AddMyPathEnvironment()
{
	TCHAR path[MAX_PATH];
	GetMyModulePath(path, ARRAYSIZE(path));
	AddPathEnvironment(path);
}

HGLOBAL DuplicateMemoryHandle(HGLOBAL hmem, UINT flag)
{
	HGLOBAL hmemNew = NULL;
	if (hmem) {
		SIZE_T size = GlobalSize(hmem);
		hmemNew = GlobalAlloc(flag, size);
		if (hmemNew) {
			LPVOID ps = GlobalLock(hmem);
			LPVOID pt = GlobalLock(hmemNew);
			memcpy(pt, ps, size);
			GlobalUnlock(hmem);
			GlobalUnlock(hmemNew);
		}
	}
	return hmemNew;
}

void ZapMessage()
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

BOOL RunDll(HWND hwnd, LPCTSTR pszDll, LPCTSTR pszFunc, LPCTSTR pszArg)
{
	CPath dll;
	GetMyModulePath(dll);
	dll.Append(pszDll);
	if (!dll.FileExists()) return FALSE;

	CPath dir = dll;
	dir.RemoveFileSpec();

	CPath exec = dir;
	exec.Append(_T("execdll.exe"));

	dll.QuoteSpaces();
	CString cmd;
	cmd.Format(_T("%s,%s %s"), (LPCTSTR)dll, pszFunc, pszArg);

	CEnterFolder ef;
	ef.Enter(dir);

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_UNICODE;
	sei.hwnd = hwnd;
	sei.lpFile = exec;
	sei.lpParameters = cmd;
	sei.lpDirectory = dir;
	sei.nShow = SW_SHOWNORMAL;
	BOOL bOK = ShellExecuteEx(&sei);

	return bOK;
}

static const TCHAR sszRegRoot[] = _T("Software\\LightShip");
static const TCHAR sszRegGuid[] = _T("GuidOperation");

static BOOL StoreGuidData(DWORD effect, LPCTSTR path, HDROP hdrop, LPTSTR guidstr)
{
	BOOL bOK = FALSE;
	CRegKey root;
	if (root.Create(HKEY_CURRENT_USER, sszRegRoot) == ERROR_SUCCESS) {
		CRegKey gtr;
		if (gtr.Create(root, sszRegGuid, REG_NONE, REG_OPTION_VOLATILE, KEY_WRITE) == ERROR_SUCCESS) {
			GUID g;
			if (CoCreateGuid(&g) == S_OK) {
				OLECHAR oz[128];
				StringFromGUID2(g, oz, ARRAYSIZE(oz));
				CRegKey sec;
				if (sec.Create(gtr, OLE2T(oz), REG_NONE, REG_OPTION_VOLATILE, KEY_WRITE) == ERROR_SUCCESS) {
					PathCombine(guidstr, sszRegRoot, sszRegGuid);
					PathAppend(guidstr, OLE2T(oz));
					// effect
					sec.SetDWORDValue(_T("effect"), effect);
					// path
					if (path && *path)
						sec.SetStringValue(_T("path"), path);
					// hdrop
					if (hdrop) {
						size_t size = GlobalSize((HGLOBAL)hdrop);
						void* p = GlobalLock((HGLOBAL)hdrop);
						sec.SetBinaryValue(_T("drop"), p, (ULONG)size);
						GlobalUnlock((HGLOBAL)hdrop);
					}
					sec.Close();
					bOK = TRUE;
				}
			}
			gtr.Close();
		}
		root.Close();
	}
	return bOK;
}

BOOL GuidRunDll(HWND hwnd, DWORD effect, LPCTSTR path, HDROP hdrop, LPCTSTR dll, LPCTSTR func)
{
	TCHAR cmd[MAX_PATH * 2];
	BOOL bOK = StoreGuidData(effect, path, hdrop, cmd);
	if (bOK) {
		bOK = RunDll(hwnd, dll, func, cmd);
		if (!bOK)
			SHDeleteKey(HKEY_CURRENT_USER, cmd);
	}
	return bOK;
}

BOOL CopyTextToClipboard(HWND hwnd, LPCTSTR pszText, bool bAppend)
{
	BOOL bOK = FALSE;
	if (pszText && lstrlen(pszText) > 0 && OpenClipboard(hwnd)) {
		CString str = pszText;
		if (bAppend) {
			HGLOBAL hmem = ::GetClipboardData(CF_UNICODETEXT);
			if (hmem) {
				LPCWSTR pw = (LPCWSTR)::GlobalLock(hmem);
				if (pw) {
					str += _T("\x0d\x0a");
					str += pw;
					::GlobalUnlock(hmem);
				}
			}
		}
		EmptyClipboard();

		DWORD size = (str.GetLength() + 1) * sizeof(TCHAR);
		HGLOBAL hmem = GlobalAlloc(GHND, size);
		if (hmem) {
			LPTSTR p = (LPTSTR)GlobalLock(hmem);
			lstrcpy(p, str);
			GlobalUnlock(hmem);
			SetClipboardData(CF_UNICODETEXT, hmem);
			bOK = TRUE;
		}
		CloseClipboard();
	}
	return bOK;
}

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

typedef struct tagRCTOOLBARDATA {
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]
	WORD* items() { return (WORD*)(this+1); }
} RCTOOLBARDATA;

HIMAGELIST CreateBitmapMapArrayFromImageList(UINT idResource, CSimpleMap<UINT, HBITMAP>& arr, int btnWidth)
{
	gdiplus_using gdip;

	// ツールバーデータから個々のビットマップを抽出してマップ配列を作成
	HIMAGELIST him = NULL;
	HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
	HRSRC hrsrc = ::FindResource(hInst, MAKEINTRESOURCE(idResource), RT_TOOLBAR);
	if (hrsrc) {
		HGLOBAL hmem = ::LoadResource(hInst, hrsrc);
		if (hmem) {
			RCTOOLBARDATA* pData = (RCTOOLBARDATA*)::LockResource(hmem);
			if (pData) {
				him = lccLoadImageList(hInst, RT_RCDATA, MAKEINTRESOURCE(idResource), btnWidth, RGB(255, 0, 255));
				if (him) {
					WORD* pItems = pData->items();
					int nItems = pData->wItemCount;
					for (int i = 0, j = 0; i < pData->wItemCount; i++) {
						if (pItems[i] != 0) {
							HBITMAP hbm = CreateVistaBitmapFromImageList(him, j);
							arr.Add((UINT)pItems[i], hbm);
							j++;
						}
					}
				}
			}
		}
	}
	return him;
}

#ifdef Shell_GetCachedImageIndex
#undef Shell_GetCachedImageIndex
#endif

static inline int rShell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
	return Shell_GetCachedImageIndex(pszIconPath, iIconIndex, uIconFlags);
}

HBITMAP CreateOemBitmap(int idResource)
{
	TCHAR path[MAX_PATH];
	::GetModuleFileName(::GetModuleHandle(_T("user32")), path, ARRAYSIZE(path));
	int iImage = rShell_GetCachedImageIndex(path, idResource, 0);
	HIMAGELIST hs, hl;
	Shell_GetImageLists(&hl, &hs);
	return CreateVistaBitmapFromImageList(hs, iImage);
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

HANDLE __fastcall _createfile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile)
{
	WCHAR wsz[MAX_PATH * 2];
	build_unicode_path(pName, wsz, ARRAYSIZE(wsz));
	return CreateFile(wsz, fdwAccess, fdwShareMode, psa, fdwCreate, fdwAttrsAndFlags, hTemplateFile);
}

HIMAGELIST __fastcall MakeDummyImageList(int cx, int cy /* = 0 */)
{
	const COLORREF clrMask = RGB(255, 0, 255);
	cy = !cy ? cx : cy;
	// ダミービットマップ作成
	HIMAGELIST him = ImageList_Create(cx, cy, ILC_COLORDDB | ILC_MASK, 1, 10);
	if (him) {
		CClientDC dc(NULL);
		HBITMAP hbm = ::CreateCompatibleBitmap(dc, cx, cy);
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		hbm = dcw.SelectBitmap(hbm);
		dcw.FillSolidRect(0, 0, cx, cy, clrMask);
		hbm = dcw.SelectBitmap(hbm);
		ImageList_AddMasked(him, hbm, clrMask);
		DeleteObject(hbm);
		ImageList_SetBkColor(him, CLR_NONE);
	}
	return him;
}

void SetBitmapToMenuItem(HMENU hmenu, UINT idMenu, UINT idBitmap)
{
	HBITMAP hbm = _AtlModule.GetCommandBitmap(idBitmap);
	if (hbm) {
		CMenuItemInfo mii;
		mii.fMask = MIIM_BITMAP;
		mii.hbmpItem = hbm;
		SetMenuItemInfo(hmenu, idMenu, FALSE, &mii);
	}
}
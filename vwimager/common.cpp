#include "stdafx.h"
#include "sys\stat.h"
#include "resource.h"
#include "common.h"
#include "..\lsftc\lsftc_i.h"


static TCHAR regKey[] = _T("UserName");

COSPlatform gosp;


IMAGETYPES img_types[] = {
	{ IMG_JPEG,		_T(".jpg"),	_T("Jpeg(*.jpg;*.jpeg;*.jpe)") },
	{ IMG_PNG,		_T(".png"),	_T("Png(*.png)") },
	{ IMG_WEBP,		_T(".webp"),_T("WebP(*.webp)") },
	{ IMG_AVIF,		_T(".avif"),_T("Avif(*.avif)") },
	{ IMG_GIF,		_T(".gif"),	_T("GIF(*.gif)") },
	{ IMG_TIFF,		_T(".tif"),	_T("Tiff(*.tif;*.tiff)") },
	{ IMG_BMP,		_T(".bmp"),	_T("Bitmap(*.bmp;*.dib)") },
	{ IMG_FPX,		_T(".fpx"),	_T("FlashPix(.fpx)") },
	{ IMG_TGA,		_T(".tga"),	_T("Targa(*.tga)") },
	{ IMG_RASTER,	_T(".ras"), _T("SunRaster(*.ras)") },
	{ IMG_PCX,		_T(".pcx"),	_T("Pcx(*.pcx)") },
	{ IMG_PNM,		_T(".pnm"),	_T("Pnm(*.ppm)") },
	{ IMG_JP2,		_T(".jp2"),	_T("Jpeg2000(*.jp2)") },
	{ IMG_SGI,		_T(".sgi"),	_T("Sgi(*.sgi)") },
	{ 0,			NULL,		NULL }
};

///////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS
//

int AppMessage(_U_STRINGorID tit, _U_STRINGorID id, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	CString cap;
	if (IS_INTRESOURCE(tit.m_lpstr))
		cap.LoadString(LOWORD(tit.m_lpstr));
	else
		cap = tit.m_lpstr;

	CString str;
	if (IS_INTRESOURCE(id.m_lpstr))
		str.LoadString(LOWORD(id.m_lpstr));
	else
		str = id.m_lpstr;

	if (pszInsert && lstrlen(pszInsert)) {
		str.Replace(_T("%1"), pszInsert);
	}

	return ::MessageBox(GetActiveWindow(), str, cap, nType);
}

int LastError(LPCTSTR pszTitle, UINT nType /*=MB_STOP*/)
{
	int nError = GetLastError();
	CString cs;
	cs.LoadString(RESINST, IDS_APP_TITLE);
	return lccErrorBox(::GetFocus(), nError, cs, pszTitle, nType);
}

BOOL FileBrowse(HWND hWnd, UINT nIDCaption, UINT nIDFilter, LPTSTR pszName, BOOL bRead, int* filterIndex)
{
	TCHAR szFileName[MAX_PATH * 2], szFileTitle[MAX_PATH];
	TCHAR szFilter[MAX_PATH], szInitialDir[MAX_PATH];
	if (PathFileExists(pszName)) {
		lstrcpy(szInitialDir, pszName);
		PathRemoveFileSpec(szInitialDir);
	}
	else
		SHGetFolderPath(hWnd, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szInitialDir);
	szFileName[0] = '\0';

	LoadString(RESINST, nIDCaption, szFileTitle, _COUNTOF(szFileTitle));
	LoadString(RESINST, nIDFilter, szFilter, _COUNTOF(szFilter));

	TCHAR* pch = szFilter;
	while (*pch) {
		if (*pch == '|') *pch = '\0';
		pch++;
	}

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _COUNTOF(szFileName);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = _COUNTOF(szFileTitle);
	ofn.lpstrInitialDir = szInitialDir;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
	ofn.lpstrFilter = szFilter;

	BOOL bOK;
	if (bRead)
		bOK = GetOpenFileName(&ofn);
	else
		bOK = GetSaveFileName(&ofn);

	if (!bOK)
		return FALSE;

	lstrcpy(pszName, ofn.lpstrFile);
	if (filterIndex)
		*filterIndex = ofn.nFilterIndex - 1;
	return TRUE;
}

int ListupSiblings(LPCTSTR lpClass, HWND *phWnd)
{
	int cnt = 0;
	HWND hDesktop = ::GetDesktopWindow();
	HWND hwnd = ::GetTopWindow(hDesktop);
	for (; hwnd; hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT))
	{
		TCHAR buf[_MAX_PATH];
		::GetClassName(hwnd, buf, _COUNTOF(buf));
		if (!lstrcmp(lpClass, buf) && ::IsWindowVisible(hwnd) && !::IsIconic(hwnd))
		{
			*phWnd++ = hwnd;
			cnt++;
		}
	}
    return cnt;
}

void NextPrevWindow(HWND hWnd, LPCTSTR lpClass, BOOL bNext)
{
	HWND hwnds[64];
	int cnt = ListupSiblings(lpClass, hwnds);

	if (cnt > 1)
	{
		if (bNext)
			::SetWindowPos(::GetLastActivePopup(hwnds[1]), HWND_TOP, 0, 0, 0, 0,
						SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
		else
			::SetWindowPos(::GetLastActivePopup(hwnds[cnt - 1]),
						HWND_TOP, 0, 0, 0, 0,
						SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}
}

int HowManySiblings(HWND hwndOwn, LPCTSTR lpClass)
{
	HWND hDesktop = ::GetDesktopWindow();
	HWND hwnd = ::GetTopWindow(hDesktop);
	int n = 0;
	TCHAR buf[128];
	for (; hwnd; hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT))
	{
		if (hwnd == hwndOwn)
			continue;

		if (::GetClassName(hwnd, buf, 128) && !lstrcmp(lpClass, buf))
			n++;
	}
	return n;
}

HWND Get1stSiblibgs(HWND hwndOwn, LPCTSTR lpClass)
{
	HWND hDesktop = ::GetDesktopWindow();
	HWND hwnd = ::GetTopWindow(hDesktop);
	TCHAR buf[128];
	for (; hwnd; hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT))
	{
		if (hwnd == hwndOwn)
			continue;

		if (::GetClassName(hwnd, buf, 128) && !lstrcmp(lpClass, buf))
			return hwnd;
	}
	return NULL;
}

void FormatText(LPCTSTR lpszSrc, LPTSTR lpszDst, int nTabConv, int nMax)
{
	int len = lstrlen(lpszSrc);
	register int i, j;
	for (i = j = 0; i < len; i++)
	{
		if (j >= nMax - 1)
			break;

		if (lpszSrc[i] == '\t')
		{
			for (int k = j % nTabConv; k < nTabConv; k++)
				lpszDst[j++] = ' ';
		}
		else
			lpszDst[j++] = lpszSrc[i];
	}
	lpszDst[j] = '\0';

	if (lpszDst[j - 1] == '\n')
		lpszDst[j - 1] = '\0';
}

void PaintBitmap(HDC hdc, CRect& rcd, CRect& rcs, HBITMAP hbm, HPALETTE hpal)
{
	if (!hbm)
		return;

	HDC hdcw = CreateCompatibleDC(hdc);
	if (!hdcw)
		return;

	HPALETTE hpal1 = NULL, hpal2 = NULL;
	if (hpal) {
		hpal1 = SelectPalette(hdcw, hpal, TRUE);
		hpal2 = SelectPalette(hdc, hpal, TRUE);
		RealizePalette(hdc);
	}

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcw, hbm);
	if (rcs.Width() == rcd.Width() && rcs.Height() == rcd.Height())
		BitBlt(hdc, rcd.left, rcd.top, rcd.Width(), rcd.Height(), hdcw, rcs.left, rcs.top, SRCCOPY);
	else {
		SetStretchBltMode(hdc, HALFTONE);
		StretchBlt(hdc, rcd.left, rcd.top, rcd.Width(), rcd.Height(), hdcw, rcs.left, rcs.top, rcs.Width(), rcs.Height(), SRCCOPY);
	}
	SelectObject(hdcw, hbmOld);
	if (hpal)
		SelectPalette(hdcw, hpal1, FALSE);
	if (hpal)
		SelectPalette(hdc, hpal2, FALSE);

	DeleteDC(hdcw);
}

void PaintDIB(HDC hdc, CRect& rcd, CRect& rcs, HDIB hdib)
{
	if (!hdib)
		return;

	LPBITMAPINFO pbi  = (LPBITMAPINFO)GlobalLock(hdib);
	int y = pbi->bmiHeader.biHeight;
	LPBYTE pbits = dib_findbits(pbi);

	SetStretchBltMode(hdc, COLORONCOLOR);
	if (rcs.Width() == rcd.Width() && rcs.Height() == rcd.Height())
		SetDIBitsToDevice(hdc, rcd.left, rcd.top, rcd.Width(), rcd.Height(),
							rcs.left, y - rcs.top - rcs.Height(), 0, y,
							pbits, pbi, DIB_RGB_COLORS);
	else
		StretchDIBits(hdc, rcd.left, rcd.top, rcd.Width(), rcd.Height(),
							rcs.left, rcs.top, rcs.Width(), rcs.Height(),
							pbits, pbi, DIB_RGB_COLORS, SRCCOPY);
	GlobalUnlock(hdib);
}

void calcinsiderect(CRect& rcMax, int x, int y, CRect& rcCalc)
{
	int dx = x;
	int dy = y;
	if (rcMax.Width() < x || rcMax.Height() < y)
	{
		for (float f = 1.25F; ; f += 0.25F)
		{
			dx = (int)((float)x / f);
			dy = (int)((float)y / f);
			if (dx <= rcMax.Width() && dy <= rcMax.Height())
				break;
		}
	}

	rcCalc = rcMax;
	rcCalc.left += (rcMax.Width() - dx) / 2;
	rcCalc.right = rcCalc.left + dx;
	rcCalc.top += (rcMax.Height() - dy) / 2;
	rcCalc.bottom = rcCalc.top + dy;
}

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

int dibandtodevice(HDC hdc, int sx, int sy, BITMAPINFO *pbi)
{
	if (!pbi)
		return 0;

	int width = pbi->bmiHeader.biWidth;
	int height = pbi->bmiHeader.biHeight;

	// Initialize pointer to the image data
	BYTE* pbits = dib_findbits(pbi);

	// Adjust image sizes for one scan line
	int widthbytes = WIDTHBYTES(width * pbi->bmiHeader.biBitCount);

	BITMAPINFO bi;
	memcpy(&bi, pbi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSizeImage = widthbytes;
	bi.bmiHeader.biHeight = 1;
	int i;
	for (i = 0; i < height; i++) {
		SetDIBitsToDevice(hdc, sx, sy + i,
							width, 1, 0, 0, 0, 1,
							pbits + widthbytes * (height - i - 1),
							&bi, DIB_RGB_COLORS);
	}
	return i;
}

BOOL __fastcall ImageInClipboard(HWND hwnd)
{
	BOOL bClipboard = FALSE;
	if (OpenClipboard(hwnd))
	{
		if (IsClipboardFormatAvailable(CF_DIB) ||
			IsClipboardFormatAvailable(CF_BITMAP))
			bClipboard = TRUE;
		CloseClipboard();
	}
	return bClipboard;
}

HDIB quantize(HDIB hsrc, int dither_type, int numcolors)
{
	mif image;
	if (numcolors > 256 || !image.attachdib(hsrc))
		return NULL;

	image.quantize(dither_type, numcolors, NULL);
	return image.createdib();
}

/* @@@
static BOOL _check_user()
{
	TCHAR buf[MAX_PATH];
	lstrcpy(buf, _T("Software"));
	PathAppend(buf, APP_COMPANY);
	PathAppend(buf, APP_PRODUCT);
	PathAppend(buf, RGI_USERINFO);

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, buf, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		BYTE data[256];
		DWORD len = sizeof(data);
		DWORD type = REG_BINARY;
		LONG res = RegQueryValueEx(hKey, RGI_LICENSE, 0, &type, data, &len);
		RegCloseKey(hKey);
		return (res == ERROR_SUCCESS);
	}
	return FALSE;
}
@@@ */

enum {
	USRL_NOCHECK = -1,
	USRL_TRIAL,
	USRL_EXPIRE,
	USRL_VALID,
};
static int snUserLevel = USRL_NOCHECK;
#define LIMIT_OF_TRIAL_DATES			30

BOOL user_ristriction()
{
	TCHAR buf[MAX_PATH * 2];
//@	if (lccRegGetString(APP_PRODUCT, RGI_USERINFO, regKey, buf, ARRAYSIZE(buf)) < 1) return FALSE;
	if (snUserLevel <= USRL_NOCHECK) {
		if (lccRegGetStrJum(APP_PRODUCT, RGI_USERINFO, RGI_LICENSE, (char*)buf, sizeof(buf)) < 1) {
			long limits = LIMIT_OF_TRIAL_DATES;
			CComPtr<IFileXferComponent> pt;
			HRESULT hr = pt.CoCreateInstance(__uuidof(FileXferComponent));
			if (SUCCEEDED(hr)) {
				if (pt->GetInstance(&limits) != S_OK)
					limits = 0;
			}
			snUserLevel = (limits <= 0 || LIMIT_OF_TRIAL_DATES < limits) ? USRL_EXPIRE : USRL_TRIAL;
		}
		else
			snUserLevel = USRL_VALID;
	}

	if (snUserLevel == USRL_EXPIRE) {
		AppMessage((UINT)0, IDS_USER_RISTRICTION);
		return FALSE;
	}
	return TRUE;
}

IActiveDesktop* GetActiveDesktop()
{
	IActiveDesktop* pad;
	HRESULT hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**)&pad);
	if (SUCCEEDED(hr))
		return pad;
	return NULL;
}

BOOL SetWallpaper(LPTSTR pszFile)
{
	if (!pszFile || !PathFileExists(pszFile))
		return FALSE;

	BOOL bOK = FALSE;
	IActiveDesktop* pad = GetActiveDesktop();
	if (pad)
	{
#ifndef UNICODE
		WCHAR wsz[MAX_PATH * 2];
		MultiByteToWideChar(CP_ACP, 0, pszFile, -1, wsz, MAX_PATH * 2);
		if (pad->SetWallpaper(wsz, 0) == S_OK)
#else
		if (pad->SetWallpaper(pszFile, 0) == S_OK)
#endif
		{
			pad->ApplyChanges(AD_APPLY_ALL);
			bOK = TRUE;
		}
		pad->Release();
	}
	else
	{
		SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, pszFile, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		bOK = TRUE;
	}
	return bOK;
}

HGLOBAL __fastcall copyhandle(HGLOBAL hsrc)
{
	HGLOBAL hdst = NULL;
	if (hsrc) {
		SIZE_T size = GlobalSize(hsrc);
		if (hdst = GlobalAlloc(GMEM_MOVEABLE, size)) {
			void* ps = GlobalLock(hsrc);
			void* pd = GlobalLock(hdst);
			memcpy(pd, ps, size);
			GlobalUnlock(hsrc);
			GlobalUnlock(hdst);
		}
	}
	return hdst;
}

int __fastcall dib_numcolors(void* pv)
{
	int colors = ((LPBITMAPINFOHEADER)pv)->biClrUsed;
	if (!colors) {
		int bits = ((LPBITMAPINFOHEADER)pv)->biBitCount;
		colors = (bits <= 8) ? (1 << bits) : 0;
	}
	return colors;
}

int __fastcall dib_palsize(void* pdib)
{
	int size = dib_numcolors(pdib) * sizeof(RGBQUAD);
	if (((BITMAPINFOHEADER*)pdib)->biCompression == BI_BITFIELDS)
		size += sizeof(DWORD) * 3;
	return size;
}

BOOL __fastcall dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits)
{
	BOOL bOK = FALSE;
	if (hdib) {
		LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
		if (pbh) {
			if (dx)		*dx = pbh->biWidth;
			if (dy)		*dy = pbh->biHeight;
			if (colors)	*colors = dib_numcolors(pbh);
			if (bits)	*bits = pbh->biBitCount;
			bOK= TRUE;
		}
		GlobalUnlock(hdib);
	}
	return bOK;
}

int __fastcall GetCheckedRadioButton(HWND hwnd, int nIDFirstButton, int nIDLastButton)
{
	for (int nID = nIDFirstButton; nID <= nIDLastButton; nID++) {
		if (IsDlgButtonChecked(hwnd, nID))
			return nID; // id that matched
	}
	return 0; // invalid ID
}

void __fastcall GetDataFolder(LPTSTR pszPath)
{
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, pszPath);
	PathAppend(pszPath, APP_COMPANY);
	// "<APPDATA>\LightShip\PhotoImager"
	CString str; str.LoadString(IDS_APP_TITLE);
	PathAppend(pszPath, str);
	if (!PathIsDirectory(pszPath))
		_pathmaketree(pszPath);
}

#include "..\lsimg\imgspecify.cpp"

BOOL __fastcall rename_imgextension(LPTSTR pszFile, int type)
{
	int deftype = imgSpecifyImageType(pszFile);
	switch (type) {
		case IMG_JPEG:
			if (deftype != type) PathRenameExtension(pszFile, _T(".jpg"));
			break;
		case IMG_BMP:
			if (deftype != type) PathRenameExtension(pszFile, _T(".bmp"));
			break;
		case IMG_GIF:
			if (deftype != type) PathRenameExtension(pszFile, _T(".gif"));
			break;
		case IMG_WEBP:
			if (deftype != type) PathRenameExtension(pszFile, _T(".webp"));
			break;
		case IMG_AVIF:
			if (deftype != type) PathRenameExtension(pszFile, _T(".avif"));
			break;
		case IMG_TIFF:
			if (deftype != type) PathRenameExtension(pszFile, _T(".tif"));
			break;
		case IMG_TGA:
			if (deftype != type) PathRenameExtension(pszFile, _T(".tga"));
			break;
		case IMG_PCX:
			if (deftype != type) PathRenameExtension(pszFile, _T(".pcx"));
			break;
		case IMG_FPX:
			if (deftype != type) PathRenameExtension(pszFile, _T(".fpx"));
			break;
		case IMG_PNG:
			if (deftype != type) PathRenameExtension(pszFile, _T(".png"));
			break;
		case IMG_PNM:
			if (deftype != type) PathRenameExtension(pszFile, _T(".ppm"));
			break;
		case IMG_JP2:
			if (deftype != type) PathRenameExtension(pszFile, _T(".jp2"));
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

void __fastcall InitDlgCtrl(HWND hctl, UINT idResource)
{
	TCHAR buf[MAX_PATH * 2];
	if (GetClassName(hctl, buf, _COUNTOF(buf)) < 1) return;
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

BOOL __fastcall _pathmaketree(LPCTSTR pszPath)
{
	if (PathIsRoot(pszPath) || lstrlen(pszPath) < 3) {
		SetLastError(ERROR_BAD_PATHNAME);
		return TRUE;
	}
	while (1) {
		int res = SHCreateDirectory(NULL, pszPath);
		switch (res) {
		case ERROR_SUCCESS:
			break;
		case ERROR_FILE_EXISTS:		// 同名のファイルが存在する
		case ERROR_ALREADY_EXISTS:
			if (PathIsDirectory(pszPath)) return TRUE;
			// fall
		default:
			SetLastError(res);
			if (LastError(pszPath, MB_QRETRY) == IDCANCEL)
				return FALSE;
		}
	}
	SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, pszPath, 0);
	return TRUE;
}

void __fastcall _pathdecorate(LPTSTR psz)
{
	if (!psz || *psz == 0) return;

	LPTSTR name = StrDup(psz);
	CString ext;
	LPTSTR p = PathFindExtension(name);
	if (*p == '.') {
		ext = p;
		*p = 0;
	}

	CString body;
	for (int i = 1; i < 1000; i++) {
		body.Format(_T("%s[%d]"), name, i);
		if (!ext.IsEmpty())
			body += ext;
		if (!PathFileExists(body)) {
			lstrcpy(psz, body);
			break;
		}
	}
	LocalFree(name);
}

int CountSiblings(HWND hwndOwn)
{
	TCHAR cname[128], buf[128];
	::GetClassName(hwndOwn, cname, ARRAYSIZE(cname));

	int cnt = 0;
	HWND hwnd = ::GetTopWindow(::GetDesktopWindow());
	for (; hwnd; hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT)) {
		if (hwnd == hwndOwn) continue;
		if (::GetClassName(hwnd, buf, ARRAYSIZE(buf)) && !lstrcmp(cname, buf))
			cnt++;
	}
	return cnt;
}

BOOL CheckDirectoryNameUI(HWND hwnd, LPCTSTR pszPath)
{
	// ディレクトリの検査…名前不正やネットワークリソースエラーのチェック
	// 対象のディレクトリが存在しなくても、エラーコードにより許容の是非を確認する
	if (!PathIsDirectory(pszPath)) {
		DWORD err = GetLastError();
		switch (err) {
		case ERROR_BAD_NETPATH:
		case ERROR_INVALID_NAME:
		case ERROR_NO_NETWORK:
			lccErrorBox(hwnd, err, APP_PRODUCT, pszPath, MB_STOP);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CheckServerUI(HWND hwnd, LPCTSTR pszFolder)
{
	if (PathIsUNC(pszFolder)) {
		// サーバの接続を確認
		TCHAR path[MAX_PATH * 2];
		lstrcpy(path, pszFolder);
		while (PathIsUNC(path) && !PathIsUNCServer(path) && !PathIsUNCServerShare(path))
			PathRemoveFileSpec(path);

		if (!SHValidateUNC(hwnd, path, 0)) {
			lccErrorBox(hwnd, GetLastError(), APP_PRODUCT, pszFolder, MB_STOP);
			return FALSE;
		}

		if (lstrcmp(pszFolder, path)) {
			// Sharedフォルダの接続を確認
			lstrcpy(path, pszFolder);
			return SHValidateUNC(hwnd, path, 0);
		}
	}
	return TRUE;
}

static IShellItem* CreateShellItemFromPath(LPCTSTR pszFileName)
{
	if (!pszFileName || *pszFileName == 0) return NULL;

	IShellItem* psi = NULL;
	LPITEMIDLIST pidlFullyQual = ILCreateFromPath(pszFileName);
	if (pidlFullyQual) {
		IShellFolder* psf;
		LPCITEMIDLIST pidlSelf;
		if (SUCCEEDED(SHBindToParent(pidlFullyQual, IID_IShellFolder, (void**)&psf, &pidlSelf))) {
			if (FAILED(SHCreateShellItem(NULL, psf, pidlSelf, &psi)))
				psi = NULL;
			psf->Release();
		}
		CoTaskMemFree(pidlFullyQual);
	}
	return psi;
}

static COMDLG_FILTERSPEC rgSpec[] = { { L"All(*.*)", L"*.*" }, };

bool ShowOpenSaveFileDialog(HWND hwnd, bool bOpen, LPCTSTR pszTitle, LPCTSTR pszDir, CString& strFile, const COMDLG_FILTERSPEC* pcfFilter, int numFilters)
{
	// デフォルトフィルタ
	if (pcfFilter == NULL) {
		pcfFilter = rgSpec;
		numFilters = ARRAYSIZE(rgSpec);
	}
	CString strTitle;
	if (pszTitle && IS_INTRESOURCE(pszTitle)) {
		strTitle.LoadString(PtrToUint(pszTitle));
		pszTitle = strTitle;
	}

	// 初期フォルダ
	TCHAR dir[MAX_PATH * 2];
	if (pszDir && *pszDir) {
		lstrcpy(dir, pszDir);
	} else if (!strFile.IsEmpty()) {
		lstrcpy(dir, strFile.GetString());
		PathRemoveFileSpec(dir);
	} else {
		dir[0] = 0;
	}
	bool ok = false;
	IFileDialog* pfd;
	HRESULT hr = CoCreateInstance(
						bOpen ? CLSID_FileOpenDialog : CLSID_FileSaveDialog,
						NULL, CLSCTX_INPROC_SERVER, IID_IFileDialog, (void**)&pfd);
	if (SUCCEEDED(hr)) {
		if (!strFile.IsEmpty())
			pfd->SetFileName(strFile.GetString());
		if (pszTitle)
			pfd->SetTitle(pszTitle);
		pfd->SetFileTypes(1, pcfFilter);
		pfd->SetFileTypeIndex(1);					// one base, not zero base

		IShellItem* psi = CreateShellItemFromPath(dir);
		if (psi)
			pfd->SetDefaultFolder(psi);

		hr = pfd->Show(hwnd);
		if (SUCCEEDED(hr)) {
			IShellItem* psiResult;
			hr = pfd->GetResult(&psiResult);
			if (SUCCEEDED(hr)) {
				LPTSTR psz;
				hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &psz);
				if (SUCCEEDED(hr)) {
					strFile = psz;
					CoTaskMemFree(psz);
				}
				psiResult->Release();
			}
		}

		if (psi)
			psi->Release();
		pfd->Release();
		ok = SUCCEEDED(hr) ? true : false;
	}
	else {
		CString str;
		const COMDLG_FILTERSPEC* pcf = pcfFilter;
		while (numFilters-- > 0) {
			str.AppendFormat(_T("%s|%s|"), pcf->pszName, pcf->pszSpec);
			pcf++;
		}
		TCHAR filter[MAX_PATH * 2];
		lstrcpy(filter, str.GetString());
		LPTSTR p = filter;
		while (*p) { if (*p == _T('|')) *p = 0; p++; }

		CFileDialog dlg(bOpen, 
						NULL,			// DefExt
						strFile.GetString(),
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						filter,
						hwnd);
		if (pszTitle)
			dlg.m_ofn.lpstrTitle = pszTitle;
		if (dir[0])
			dlg.m_ofn.lpstrInitialDir = dir;
		if (dlg.DoModal() == IDOK) {
			strFile = dlg.m_szFileName;
			ok = true;
		}
	}
	return ok;
}

double __fastcall CalcInnerImageSize(int cxMax, int cyMax, int* pcx, int* pcy)
{
	double ratio = 1.0;
	if (*pcx > cxMax || *pcy > cyMax) {
		double xRatio = (double)cxMax / (double)*pcx;
		double yRatio = (double)cyMax / (double)*pcy;
		ratio = (xRatio > yRatio) ? yRatio : xRatio;
		*pcx = (int)floor(*pcx * ratio);
		*pcy = (int)floor(*pcy * ratio);
	}
	return ratio;
}

inline void _SetvertTrivertex(TRIVERTEX& tvt, const POINT& pt, COLORREF clr, COLOR16 alpha = 0)
{
	tvt.x = pt.x;
	tvt.y = pt.y;
	tvt.Green = GetGValue(clr) << 8;
	tvt.Blue = GetBValue(clr) << 8;
	tvt.Red = GetRValue(clr) << 8;
	tvt.Alpha = alpha;
}

void FillShadeRect(HDC hdc, LPCRECT prc, COLORREF c1, COLORREF c2)
{
	CRect rc(*prc);
	CDCHandle dc(hdc);
	TRIVERTEX vert[2];
	_SetvertTrivertex(vert[0], rc.TopLeft(), c1);
	_SetvertTrivertex(vert[1], rc.BottomRight(), c2);
	GRADIENT_RECT grc;
	grc.UpperLeft = 0;
	grc.LowerRight = 1;
	dc.GradientFill(vert, 2, &grc, 1, GRADIENT_FILL_RECT_V);
}

void FillShadeRect(HDC hdc, int x, int y, int cx, int cy, COLORREF c1, COLORREF c2)
{
	RECT rc = { x, y, x + cx, y + cy };
	FillShadeRect(hdc, &rc, c1, c2);
}

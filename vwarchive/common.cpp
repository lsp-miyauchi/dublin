#include "stdafx.h"
#include "mmsystem.h"
#include "sys\stat.h"
#include "resource.h"
#include "common.h"
#include "..\lsftc\lsftc_i.h"

#include "..\lsimg\imgspecify.cpp"

//////////////////////////////////////////////////////////////////////
// グローバル変数

static TCHAR regKey[] = _T("UserName");

TCHAR gszAst[] = _T("*.*");
TCHAR gszLzhExt[] = _T(".lzh");
TCHAR gszZipExt[] = _T(".zip");

COSPlatform gosp;


extern "C" void WINAPI lavRunViewW(HWND, HINSTANCE, LPWSTR, int);


///////////////////////////////////////////////////////////////////////
//

#define _ISNETPATH(a)	(lstrlen((a)) > 3 && *(a) == '\\' && *((a) + 1) == '\\' && *((a) + 2) != '?')
//#define STRCOMP(a,b)	(StrCmpI((a),(b)))
#define STRCOMP(a,b)	(StrCmpLogicalW((a),(b)))


///////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS
//

BOOL get_fileinfo(LPCTSTR pszName, WIN32_FIND_DATA* pwfd)
{
	HANDLE hf = FindFirstFile(pszName, pwfd);
	DWORD dw = GetLastError();
	if (hf != INVALID_HANDLE_VALUE)
		FindClose(hf);
	return (hf != INVALID_HANDLE_VALUE || dw == ERROR_MORE_DATA);
}

static LPTSTR WINAPI fmt_str(LPCTSTR pFormat, LPCTSTR const* rgpIns, int nStr, LPTSTR pOutput, int cchMax)
{
	// determine length of destination string
	int nTotalLen = 1;						// '\0'ターミネータ分を付加
	LPCTSTR pchSrc = pFormat;
	while (*pchSrc != 0) {
		if (pchSrc[0] == '%' && (pchSrc[1] >= '1' && pchSrc[1] <= '9')) {
			int i = pchSrc[1] - '1';
			pchSrc += 2;
			if (i >= nStr)
				nTotalLen++;
			else if (rgpIns[i] != NULL)
				nTotalLen += lstrlen(rgpIns[i]);
		}
		else {
#ifndef UNICODE
			if (IsDBCSLeadByte(*pchSrc)) {
				pchSrc++;
				nTotalLen++;
			}
#endif
			pchSrc++;
			nTotalLen++;
		}
	}

	pchSrc = pFormat;
	LPTSTR pData = (LPTSTR)malloc(nTotalLen * sizeof(TCHAR));
	LPTSTR pchDest = pData;
	while (*pchSrc != 0) {
		if (pchSrc[0] == '%' && (pchSrc[1] >= '1' && pchSrc[1] <= '9')) {
			int i = pchSrc[1] - '1';
			pchSrc += 2;
			if (i >= nStr)
				*pchDest++ = '?';
			else if (rgpIns[i] != NULL) {
				lstrcpy(pchDest, rgpIns[i]);
				pchDest += lstrlen(pchDest);
			}
		}
		else {
#ifndef UNICODE
			if (IsDBCSLeadByte(*pchSrc))
				*pchDest++ = *pchSrc++;
#endif
			*pchDest++ = *pchSrc++;
		}
	}
	*pchDest = '\0';

	lstrcpyn(pOutput, pData, cchMax);
	free(pData);
	return pOutput;
}

static LPTSTR WINAPI fmt_str1(HINSTANCE hRes, UINT nIDS, LPCTSTR pIns, LPTSTR pOutput, int cchMax)
{
	TCHAR szFormat[512];
	if (!LoadString(hRes, nIDS, szFormat, _COUNTOF(szFormat)))
		return NULL;
	return fmt_str(szFormat, &pIns, 1, pOutput, cchMax);
}

int AppMessage(HWND hwnd, UINT id, LPCTSTR pszInsert /*=NULL*/, UINT nType /*=MB_STOP*/)
{
	TCHAR tit[MAX_PATH], sz[MAX_PATH * 4];
	HINSTANCE hInst = RESINST;
	LoadString(hInst, IDS_APP_TITLE, tit, _COUNTOF(tit));
	if (pszInsert && *pszInsert)
		fmt_str1(hInst, id, pszInsert, sz, _COUNTOF(sz));
	else
		LoadString(hInst, id, sz, _COUNTOF(sz));

	if (!hwnd || !IsWindow(hwnd)) hwnd = GetActiveWindow();
	return ::MessageBox(hwnd, sz, tit, nType);
}

int LastError(LPCTSTR pszTitle, UINT nType /*=MB_STOP*/)
{
	int nError = GetLastError();

	CString str;
	str.LoadString(IDS_APP_TITLE);
	return lccErrorBox(GetActiveWindow(), nError, str, pszTitle, nType);
}

BOOL FileBrowse(BOOL bRead, HWND hwnd, UINT nIDCaption, UINT nIDFilter, LPTSTR pszName)
{
	TCHAR szFilter[MAX_PATH], szFileName[MAX_PATH * 2];
	TCHAR szFileTitle[MAX_PATH], szDlgTitle[MAX_PATH];
	szFileName[0] = szFileTitle[0] = 0;

	if (!bRead)
		lstrcpy(szFileName, pszName);

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH * 2;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;

	HINSTANCE hInst = RESINST;
	LoadString(hInst, nIDCaption, szDlgTitle, _COUNTOF(szDlgTitle));
	ofn.lpstrTitle = szDlgTitle;

	LoadString(hInst, nIDFilter, szFilter, _COUNTOF(szFilter));
	TCHAR* p = szFilter;
	while (p = StrChr(p, '|'))
		*p++ = '\0';
	ofn.lpstrFilter = szFilter;

	TCHAR olddir[MAX_PATH * 2];
	GetCurrentDirectory(_COUNTOF(olddir), olddir);
	int nResult;
	if (bRead)
		nResult = GetOpenFileName(&ofn);
	else
		nResult = GetSaveFileName(&ofn);
	if (nResult)
		lstrcpy(pszName, ofn.lpstrFile);
	SetCurrentDirectory(olddir);
	return nResult;
}

void myShellExecute(HWND hWnd, LPCTSTR pName, LPCTSTR pDir)
{
	TCHAR buf[MAX_PATH * 2];
	PathCombine(buf, pDir, pName);

	int type = CheckFileType(buf);
	if (type == CE_PROGRAM || type == CE_ASSOCIATE)
		ShellExecute(hWnd, NULL, pName, NULL, pDir, SW_NORMAL);
	else
		ViewFile(hWnd, pName, pDir);
}

static int is_sfx(HWND hwnd, LPCTSTR pszFile)
{
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		if (pam->IsArchive(hwnd, ARC_CAB, pszFile) == S_OK) return ARC_CAB;
		if (pam->IsArchive(hwnd, ARC_LZH, pszFile) == S_OK) return ARC_LZH;
		if (pam->IsArchive(hwnd, ARC_ZIP, pszFile) == S_OK) return ARC_ZIP;
		if (pam->IsArchive(hwnd, ARC_7ZIP, pszFile) == S_OK) return ARC_7ZIP;
	}
	return ARC_ERROR;
}

static HDROP make_hdrop(LPCTSTR pszName)
{
	POINT pt;
	pt.x = pt.y = 0;
	HDROP hdrop = fopDragInitBlock(pt, FALSE);
	HDROP hw;
	hw = fopDragAppendFile(hdrop, pszName);
	if (!hw) {
		DragFinish(hdrop);
		return NULL;
	}
	return hw;
}

void ViewFile(HWND hWnd, LPCTSTR pName, LPCTSTR pDir)
{
	TCHAR buf[MAX_PATH * 2];
	PathCombine(buf, pDir, pName);
	TCHAR* p = PathFindExtension(buf);
	if (*p == '.') {
		int type = SpecifyArchiveType(buf);
		p++;
		if (!lstrcmpi(p, _T("WAV")))
			sndPlaySound(buf, SND_SYNC);
		else if (imgSpecifyImageType(buf))
			RunImager(hWnd, buf);
		else if ((type != ARC_ERROR && type != ARC_ISH) ||
				(!StrCmpI(p, _T("exe")) && is_sfx(hWnd, pName) != ARC_ERROR)) {
			RunArchiver(hWnd, buf);
		}
		else {
			if (!StrCmpI(p, _T("exe")) ||
				!StrCmpI(p, _T("dll")) ||
				!StrCmpI(p, _T("drv")) ||
				!StrCmpI(p, _T("w95")) ||
				!StrCmpI(p, _T("cpl")) ||
				!StrCmpI(p, _T("386")) ||
				!StrCmpI(p, _T("qtc")) ||
				!StrCmpI(p, _T("vbx")) ||
				!StrCmpI(p, _T("vxd")) ||
				!StrCmpI(p, _T("ocx"))) {
				HDROP hdrop = make_hdrop(buf);
				if (hdrop) {
					fopShowFileInfo(hWnd, hdrop);
					DragFinish(hdrop);
				}
			}
			else
				RunTextEditor(hWnd, buf);
		}
	}
	else
		RunTextEditor(hWnd, buf);
}

void RunHexEditor(HWND hWnd, LPCTSTR pszName)
{
	lccDllExecute(hWnd, DLL_BED, BED_EDIT, pszName, _T("/bdump"), FALSE);
}

void RunTextEditor(HWND hwnd, LPCTSTR pszFile)
{
	TCHAR cmd[MAX_PATH * 2];
	if (PathFileExists(pszFile))
		wsprintf(cmd, _T("/r \"%s\""), pszFile);
	else
		lstrcpy(cmd, pszFile);
	lccDllExecute(hwnd, DLL_MIL, MIL_EDIT, cmd, _T("/edit"), FALSE);
}

void RunArchiver(HWND hWnd, LPCTSTR pszName)
{
	lccDllExecute(hWnd, DLL_ARC, ARC_VIEW, pszName, _T("/archive"), FALSE);
}

void RunImager(HWND hWnd, LPCTSTR pszName)
{
	lccDllExecute(hWnd, DLL_GMIL, _T("imgRun"), pszName, _T("/image"), FALSE);
}

static void validate_path(LPTSTR pszPath)
{
	TCHAR* p = pszPath;
	while (p && *p) {
		if (*p == '\\' || *p == '/')
			*p = '\t';
#ifndef UNICODE
		TCHAR* q = CharNext(p);
#else
		TCHAR* q = p + 1;
#endif
		if (*q == 0 && *p != '\t') {
			*q++ = '\t';
			*q = 0;
		}
		p = q;
	}
}

static void validate_fullname(LPTSTR pszPath)
{
	TCHAR* p = pszPath;
	while (p && *p) {
		if (*p == '\\' || *p == '/')
			*p = '\t';
#ifndef UNICDE
		p = CharNext(p);
#else
		p++;
#endif
	}
}

int ispathchild(LPCTSTR pszSrc, LPCTSTR pszDst)
// pszDstがpszSrcの子孫フォルダかどうかをチェックする
{
	TCHAR s[MAX_PATH * 2];
	TCHAR t[MAX_PATH * 2];
	lstrcpy(s, pszSrc);
	lstrcpy(t, pszDst);
	validate_path(s);
	validate_path(t);

	return StrCmpNI(s, t, lstrlen(s));
}

int compare_path(LPCTSTR pszSrc, LPCTSTR pszDst)
{
	TCHAR s[MAX_PATH * 2], t[MAX_PATH * 2];
	lstrcpy(s, pszSrc);
	lstrcpy(t, pszDst);
	validate_path(s);
	validate_path(t);

	return STRCOMP(s, t);
}

int compare_fullname(LPCTSTR pszSrc, LPCTSTR pszDst)
{
	// フルネームの比較をおこなう
	// validate_fullnameはパス名の最後に'/'を付加することはしない

	TCHAR s[MAX_PATH * 2], t[MAX_PATH * 2];
	lstrcpy(s, pszSrc);
	lstrcpy(t, pszDst);
	validate_fullname(s);
	validate_fullname(t);

	return STRCOMP(s, t);
}

void add_slash(LPTSTR pszPath)
{
	LPTSTR p = StrRChr(pszPath, NULL, '/');
	if (!p || *(p + 1) != 0)
		lstrcat(pszPath, _T("/"));
}

void del_slash(LPTSTR pszPath)
{
	LPTSTR p = StrRChr(pszPath, NULL, '/');
	if (p && *(p + 1) == 0)
		*p = 0;
}

int split_pathcomponent(LPCTSTR pszPath, CSimpleArray<CString>& arr)
{
	arr.RemoveAll();
	TCHAR* pw = StrDup(pszPath);
	if (pw) {
		TCHAR* p = pw;
		TCHAR* q;
		while (p && *p) {
			q = p;
#ifndef UNICODE
			while (*q == '/')
				q = CharNext(q);
			q = StrChr(CharNext(q), '/');
#else
			while (*q == '/')
				q++;
			q = StrChr(++q, '/');
#endif
			if (q)
				*q = 0;

			arr.Add(pw);
			if (q)
				*q++ = '/';
			p = q;
		}
		LocalFree(pw);
	}
	return arr.GetSize();
}

int replace_char(LPTSTR psz, TCHAR src, TCHAR dst)
{
	int n = 0;
	if (psz) {
		while (*psz) {
			if (*psz == src) {
				*psz = dst;
				n++;
			}
			psz = CharNext(psz);
		}
	}
	return n;
}

void add_char(LPTSTR psz, TCHAR add)
{
	if (psz) {
		psz += lstrlen(psz);
		*psz++ = add;
		*psz = 0;
	}
}

static TCHAR sszShellOpenCommand[] = _T("shell\\open\\command");

BOOL RegGetShellCommand(LPCTSTR pszExt, LPTSTR pszCommand, int cchMax)
{
	if (!pszExt || *pszExt == 0) return 0;

	TCHAR buf[MAX_PATH * 2], name[MAX_PATH * 2];
	LONG size = sizeof(name);
	if (RegQueryValue(HKEY_CLASSES_ROOT, pszExt, name, &size) == ERROR_SUCCESS && size > 0) {
		PathCombine(buf, name, sszShellOpenCommand);
		size = sizeof(name);
		if (RegQueryValue(HKEY_CLASSES_ROOT, buf, name, &size) == ERROR_SUCCESS && size > 0) {
			if (pszCommand && cchMax > 0)
				lstrcpyn(pszCommand, name, cchMax);
			return TRUE;
		}
	}
	else {
		PathCombine(buf, pszExt, sszShellOpenCommand);
		size = sizeof(name);
		if (RegQueryValue(HKEY_CLASSES_ROOT, buf, name, &size) == ERROR_SUCCESS && size > 0) {
			if (pszCommand && cchMax > 0)
				lstrcpyn(pszCommand, name, cchMax);
			return TRUE;
		}
	}
	return FALSE;
}

int CheckFileType(LPCTSTR pszFileName)
{
	if (!pszFileName || *pszFileName == 0)
		return CE_NORMAL;

	TCHAR* p = PathFindExtension(pszFileName);
	if (*p != '.')
		return CE_NORMAL;

	TCHAR ext[MAX_PATH * 2];
	lstrcpy(ext, p);
	CharLower(ext);

	// 実行モジュールか否かを検証
	TCHAR szExec[MAX_PATH * 2];
	lstrcpy(szExec, _T("exe com pif bat cmd lnk"));
	if (lccPresent(szExec, ext + 1))
		return CE_PROGRAM;

	// 関連付けされているか否かを検証
	if (RegGetShellCommand(ext, NULL, 0))
		return CE_ASSOCIATE;

	// 圧縮ファイル
	if (SpecifyArchiveType(pszFileName) != ARC_ERROR)
		return CE_ARC;

	return CE_NORMAL;
}

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
//@	if (lccRegGetString(APP_PRODUCT, RGI_USERINFO, regKey, buf, _COUNTOF(buf)) < 1) return FALSE;
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
		AppMessage(NULL, IDS_USER_RISTRICTION);
		return FALSE;
	}
	return TRUE;
}

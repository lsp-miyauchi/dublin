#include "stdafx.h"
#include "sys\stat.h"
#include "math.h"
#include "lscore.h"
#include "common.h"
#include "fnc.h"
#include "findfile.h"
#include "resource.h"

extern TCHAR gszAst[];

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


#define DTE_SEC(d)		(((d) & 0x001f) << 1)
#define DTE_MIN(d)		(((d) & 0x07e0) >> 5)
#define DTE_HOUR(d)		(((d) & 0xf800) >> 11)
#define DTE_DAY(d)		(((d) & 0x001f))
#define DTE_MON(d)		(((d) & 0x01e0) >> 5)
#define DTE_YEAR(d)		(((d) & 0xfe00) >> 9)
#define DTC_SEC(d)		(((d) >> 1) & 0x001f)
#define DTC_MIN(d)		(((d) << 5) & 0x07e0)
#define DTC_HOUR(d)		(((d) << 11) & 0xf800)
#define DTC_DAY(d)		(((d) & 0x001f))
#define DTC_MON(d)		(((d) << 5) & 0x01e0)
#define DTC_YEAR(d)		(((d) << 9) & 0xfe00)

#define RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define CLR_TO_RGBQUAD(a)		(RGB(GetBValue(a), GetGValue(a), GetRValue(a)))
#define MAGICROP				0xb8074a


///////////////////////////////////////////////////////////////////////
//

static TCHAR szPathToken[] = _T(":|<>/,\\");


///////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI lccAddChar(LPTSTR pszString, TCHAR nChar)
{
	register int len = lstrlen(pszString);
	pszString[len] = nChar;
	pszString[len + 1] = '\0';
}

extern "C"
void WINAPI lccReplaceChar(LPTSTR pszText, TCHAR nSrcChar, TCHAR nDstChar)
{
	if (nSrcChar == nDstChar) return;

	register LPTSTR p = pszText;
	while (p) {
		p = StrChr(p, nSrcChar);
		if (p)
			*p++ = nDstChar;
	}
}

extern "C"
BOOL WINAPI lccGetValues(LPTSTR psz, LPINT v, int cnt)
{
	register int i;
	register LPTSTR p, q;

	p = psz;
	for (i = 0; i < cnt; i++) {
		if (!p)
			return FALSE;

		q = StrChr(p, ' ');
		if (q)
			*q++ = '\0';

		if (!lccIsDigitStr(p))
			return FALSE;
		v[i] = StrToInt(p);
		p = q;
	}
	return TRUE;
}

extern "C"
BOOL WINAPI lccIsDigitStr(LPCTSTR psz)
{
	if (!psz || *psz == '\0') return FALSE;

	while (*psz) {
		if (!IsDigit(*psz) && *psz != '-' && *psz != '+')
			return FALSE;
		psz++;
	}
	return TRUE;
}

extern "C"
LPTSTR WINAPI lccNumForm(DWORD lNum, LPTSTR pszFormat)
{
	DWORD dw = lNum % 1000000L;
	WORD t1 = (WORD)(lNum / 1000000L);
	WORD t2 = (WORD)(dw / 1000L);
	WORD t3 = (WORD)(dw % 1000L);

	TCHAR buf[32];
	wsprintf(buf, _T("%04u,%03u,%03u"), t1, t2, t3);
	register int i;
	for (i = 0; buf[i] != 0; i++) {
		if (buf[i] != '0' && buf[i] != ',')
			break;
	}

	if (!buf[i]) {
		buf[i]     = '0';
		buf[i + 1] = '\0';
	}
	lstrcpy(pszFormat, &buf[i]);
	return pszFormat;
}

#define LODWORD(a)				(*((DWORD*)&(a)))
#define HIDWORD(a)				(*(((DWORD*)&(a)) + 1))
#define SETDWORDLONG(a,b,c)		(*((DWORD*)&(a))=(b),*(((DWORD*)&(a))+1)=(c))


// Structure for formatting byte strings
typedef struct tagSHLWAPI_BYTEFORMATS {
	LONGLONG dLimit;
	double dDivisor;
	double dNormaliser;
	LPCTSTR pszFormat;
	TCHAR chPrefix;
} SHLWAPI_BYTEFORMATS;

extern "C"
LPTSTR WINAPI lccFormatByteSize64(LONGLONG llb, LPWSTR lpszDest, UINT cchMax)
{
	static const TCHAR wsz3_0[] = _T("%3.0f");
	static const TCHAR wsz3_1[] = _T("%3.1f");
	static const WCHAR wsz3_2[] = _T("%3.2f");

#define KB ((ULONGLONG)1024)
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define TB (KB*KB*KB*KB)
#define PB (KB*KB*KB*KB*KB)

	static const SHLWAPI_BYTEFORMATS bfFormats[] = {
		{ 10*KB, 10.24, 100.0, wsz3_2, 'K' }, /* 10 KB */
		{ 100*KB, 102.4, 10.0, wsz3_1, 'K' }, /* 100 KB */
		{ 1000*KB, 1024.0, 1.0, wsz3_0, 'K' }, /* 1000 KB */
		{ 10*MB, 10485.76, 100.0, wsz3_2, 'M' }, /* 10 MB */
		{ 100*MB, 104857.6, 10.0, wsz3_1, 'M' }, /* 100 MB */
		{ 1000*MB, 1048576.0, 1.0, wsz3_0, 'M' }, /* 1000 MB */
		{ 10*GB, 10737418.24, 100.0, wsz3_2, 'G' }, /* 10 GB */
		{ 100*GB, 107374182.4, 10.0, wsz3_1, 'G' }, /* 100 GB */
		{ 1000*GB, 1073741824.0, 1.0, wsz3_0, 'G' }, /* 1000 GB */
		{ 10*TB, 10485.76, 100.0, wsz3_2, 'T' }, /* 10 TB */
		{ 100*TB, 104857.6, 10.0, wsz3_1, 'T' }, /* 100 TB */
		{ 1000*TB, 1048576.0, 1.0, wsz3_0, 'T' }, /* 1000 TB */
		{ 10*PB, 10737418.24, 100.00, wsz3_2, 'P' }, /* 10 PB */
		{ 100*PB, 107374182.4, 10.00, wsz3_1, 'P' }, /* 100 PB */
		{ 1000*PB, 1073741824.0, 1.00, wsz3_0, 'P' }, /* 1000 PB */
		{ 0, 10995116277.76, 100.00, wsz3_2, 'E' } /* EB's, catch all */
	};
	TCHAR buf[64];
	TCHAR szAdd[] = _T(" ?B");
	UINT i = 0;
	if (!lpszDest || !cchMax)
		return lpszDest;

	if (llb < 1024) {		// 1K
		wsprintf(lpszDest, _T("%ld バイト"), (long)llb);
		return lpszDest;
	}

	// Note that if this loop completes without finding a match, i will be
	// pointing at the last entry, which is a catch all for > 1000 PB
	while (i < sizeof(bfFormats) / sizeof(SHLWAPI_BYTEFORMATS) - 1) {
		if (llb < bfFormats[i].dLimit)
			break;
		i++;
	}

	// Above 1 TB we encounter problems with FP accuracy. So for amounts above
	// this number we integer shift down by 1 MB first. The table above has
	// the divisors scaled down from the '< 10 TB' entry onwards, to account
	// for this. We also add a small fudge factor to get the correct result for
	// counts that lie exactly on a 1024 byte boundary.
	double dbb = (i > 8) ? (double)(llb >> 20) + 0.001 : (double)llb + 0.00001;
	dbb = floor(dbb / bfFormats[i].dDivisor) / bfFormats[i].dNormaliser;

	_stprintf_s(buf, ARRAYSIZE(buf), bfFormats[i].pszFormat, dbb);
	szAdd[1] = bfFormats[i].chPrefix;
	lstrcat(buf, szAdd);
	lstrcpyn(lpszDest, buf, cchMax);
	return lpszDest;
}


extern "C"
LPTSTR WINAPI lccFormatFileSize(DWORD lNum, LPTSTR pszFormat)
{
	if (lNum < 1024)
		lNum = (lNum == 0) ? 0 : 1;
	else
		lNum >>= 10;
	return lstrcat(lccNumForm(lNum, pszFormat), _T(" KB"));
}

extern "C"
BOOL WINAPI lccSetFileDate(LPCTSTR pszName, LPFILETIME lpftCreate, LPFILETIME lpftUpdate)
{
	DWORD dwOldAttrib = GetFileAttributes(pszName);
	if (dwOldAttrib == 0xFFFFFFFF)
		return FALSE;

	if (!SetFileAttributes(pszName, dwOldAttrib & ~FILE_ATTRIBUTE_READONLY))
		return FALSE;

	BOOL bOK = FALSE;
	DWORD fdwAttrsAndFlags = 0;
	if (dwOldAttrib & FILE_ATTRIBUTE_DIRECTORY)
		fdwAttrsAndFlags |= FILE_FLAG_BACKUP_SEMANTICS;

	HANDLE hf = CreateFile(pszName, GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL, OPEN_EXISTING, fdwAttrsAndFlags, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		FILETIME ftCreate, ftUpdate;
		LPFILETIME lpC = NULL, lpU = NULL;
		if (lpftCreate) {
			LocalFileTimeToFileTime(lpftCreate, &ftCreate);
			lpC = &ftCreate;
		}
		if (lpftUpdate) {
			LocalFileTimeToFileTime(lpftUpdate, &ftUpdate);
			lpU = &ftUpdate;
		}
		bOK = SetFileTime(hf, lpC, NULL, lpU);
		CloseHandle(hf);
	}
	SetFileAttributes(pszName, dwOldAttrib);
	return bOK;
}


extern "C"
HWND WINAPI lccFindWindowByClassName(LPCTSTR pszClassName)
{
	HWND hDesktop = GetDesktopWindow();
	HWND hwnd = GetTopWindow(hDesktop);
	TCHAR buf[128];
	for (; hwnd; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) {
		if (GetClassName(hwnd, buf, ARRAYSIZE(buf)) && !lstrcmp(pszClassName, buf))
			return hwnd;
	}
	return NULL;
}

extern "C"
BOOL WINAPI lccUniqueFilter(LPTSTR pszFilter)
{
	LPTSTR p, q, pFilter[64];
	TCHAR buf[MAX_PATH * 2];

	lstrcpy(buf, pszFilter);
	p = StrPBrk(buf, szPathToken);
	if (p)
		return FALSE;

	register int i, cnt = 0;
	p = buf;
	while (p && *p != NULL && cnt < 64) {
		q = StrChr(p, ';');
		if (q)
			*q++ = '\0';
		BOOL bMatch = FALSE;
		for (i = 0; i < cnt; i++) {
			if (!lstrcmp(p, gszAst)) {
				lstrcpy(pszFilter, gszAst);
				return TRUE;
			}

			if (!StrCmpI(p, pFilter[i])) {
				bMatch = TRUE;
				break;
			}
		}

		if (!bMatch && lstrlen(p)) {
			pFilter[cnt] = p;
			cnt++;
		}
		p = q;
	}

	pszFilter[0] = '\0';
	for (i = 0; i < cnt; i++) {
		lstrcat(pszFilter, pFilter[i]);
		lccAddChar(pszFilter, ';');
	}

	i = lstrlen(pszFilter);
	if (i > 0) {
		if (pszFilter[i - 1] == ';')
			pszFilter[i - 1] = '\0';

		if (lstrlen(pszFilter) > 0)
			return TRUE;
	}
	return FALSE;
}

extern "C"
void WINAPI lccFormatFileTime(LPTSTR pszFormat, FILETIME& ft)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	wsprintf(pszFormat, _T("%04u/%02u/%02u %02u:%02u:%02u"),
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond);
}

extern "C"
void WINAPI lccFileAttrToStr(DWORD dwAttr, LPTSTR psz)
{
	// 属性長
	if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)			*psz++ = 'J';
	if (dwAttr & FILE_ATTRIBUTE_SYSTEM)					*psz++ = 'S';
	if (dwAttr & FILE_ATTRIBUTE_HIDDEN)					*psz++ = 'H';
	if (dwAttr & FILE_ATTRIBUTE_READONLY)				*psz++ = 'R';
	if (dwAttr & FILE_ATTRIBUTE_ARCHIVE)				*psz++ = 'A';
	if (dwAttr & FILE_ATTRIBUTE_COMPRESSED)				*psz++ = 'C';
	if (dwAttr & FILE_ATTRIBUTE_ENCRYPTED)				*psz++ = 'E';
	if (dwAttr & FILE_ATTRIBUTE_OFFLINE)				*psz++ = 'O';
	if (dwAttr & FILE_ATTRIBUTE_SPARSE_FILE)			*psz++ = 'P';
	if (dwAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)	*psz++ = 'I';
	*psz = 0;
}

extern "C"
void WINAPI lccPatB(HDC hdc, int x, int y, int dx, int dy, COLORREF rgb)
{
	COLORREF rgbw = SetBkColor(hdc, rgb);
	CRect rc(x, y, x + dx, y + dy);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	SetBkColor(hdc, rgbw);
}

extern "C"
BOOL WINAPI lccPresent(LPCTSTR pList, LPCTSTR pToken)
{
	BOOL bFound;
	LPTSTR p, q, pBuf;
	pBuf = StrDup(pList);
	if (!pBuf)
		return FALSE;

	bFound = FALSE;
	p = pBuf;
	do {
		q = StrChr(p, ' ');
		if (q)
			*q++ = '\0';

		if (!StrCmpI(p, pToken))
			bFound = TRUE;
		p = q;
	} while (p);
	LocalFree(pBuf);
	return bFound;
}

extern "C"
BOOL WINAPI lccSetStringToClipboard(HWND hwnd, LPCTSTR pszText)
{
	BOOL bOK = FALSE;
	if (pszText && lstrlen(pszText) > 0 && OpenClipboard(hwnd)) {
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

extern "C"
BOOL WINAPI lccWriteTextToFile(HWND hwnd, LPCTSTR lpFile, LPCTSTR lpText)
{
	HANDLE hf = CreateFile(lpFile,
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_ARCHIVE,
							NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwWrite;
	BOOL bOK = WriteFile(hf, lpText, lstrlen(lpText), &dwWrite, NULL);
	CloseHandle(hf);
	if (!bOK) {
		DeleteFile(lpFile);
		return FALSE;
	}
	return TRUE;
}

static BOOL __fastcall win32_exec(LPCTSTR pszCmdLine, LPCTSTR pszDir, UINT nExecMode)
{
	TCHAR exe[MAX_PATH * 2];
	TCHAR cmd[MAX_PATH * 2];
	LPTSTR p;
	if (*pszCmdLine == '"') {
		lstrcpy(exe, pszCmdLine + 1);
		p = StrChr(exe, '"');
		if (p) {
			*p++ = '\0';
			while (*p == ' ' && *p)
				p++;
		}
		if (p && *p)
			lstrcpy(cmd, p);
		else
			cmd[0] = '\0';
	}
	else {
		lstrcpy(exe, pszCmdLine);
		p = StrChr(exe, ' ');
		if (p) {
			*p++ = '\0';
			lstrcpy(cmd, p);
		}
		else
			cmd[0] = '\0';
	}

	HINSTANCE hi = ShellExecute(GetFocus(), NULL, exe, cmd, pszDir, nExecMode);
	return (hi <= (HINSTANCE)32) ? FALSE : TRUE;
}

extern "C"
BOOL WINAPI lccExecApp(HWND hwnd, LPCTSTR pszCmdLine, LPCTSTR pszDir, UINT nExecMode)
{
	UINT mode = SetErrorMode(0);
	HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
	HINSTANCE hInst = (HINSTANCE)(HINSTANCE_ERROR + 1);
	if (!win32_exec(pszCmdLine, pszDir, nExecMode)) {
		LPTSTR p;
		TCHAR cmdline[MAX_PATH * 2];
		lstrcpy(cmdline, pszCmdLine);
		p = StrChr(cmdline, ' ');
		if (p)
			*p++ = '\0';

		hInst = ShellExecute(hwnd, NULL, cmdline, p, pszDir, nExecMode);
	}
	SetCursor(hcur);
	SetErrorMode(mode);

	return (hInst <= (HINSTANCE)HINSTANCE_ERROR) ? FALSE : TRUE;
}

extern "C"
void WINAPI lccGetErrorString(int nErrorCode, LPTSTR pszText)
{
	*pszText = 0;
	LPTSTR lp;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lp, 0, NULL);
	lstrcpy(pszText, lp);
	LocalFree(lp);
}

extern "C"
int WINAPI lccErrorBox(HWND hwnd, int nErrorCode, LPCTSTR pszTitle, LPCTSTR pszText, int nTypeAction)
{
	TCHAR buf[MAX_PATH * 2];
	buf[0] = '\0';
	if (pszText && *pszText)
	{
		lstrcat(buf, pszText);
		lstrcat(buf, _T("\n"));
	}

	lccGetErrorString(nErrorCode, &buf[lstrlen(buf)]);
	return MessageBox(hwnd, buf, pszTitle, nTypeAction);
}

extern "C"
HRESULT WINAPI lccGetShell32Version(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	return lccDllGetVersion(TEXT("shell32.dll"), pdwMajor, pdwMinor);
}

extern "C"
HRESULT WINAPI lccDllGetVersion(LPCTSTR pszDll, LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	*pdwMajor = 0;
	*pdwMinor = 0;

	DLLVERSIONINFO dvi;
	::ZeroMemory(&dvi, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	HRESULT hr = AtlGetDllVersion(pszDll, &dvi);
	if (SUCCEEDED(hr)) {
		*pdwMajor = dvi.dwMajorVersion;
		*pdwMinor = dvi.dwMinorVersion;
	}
	return hr;
}

extern "C"
BOOL WINAPI lccGetTempFileName(LPCTSTR pszPath, LPCTSTR pszPrefix, LPTSTR pszName)
{
	if (_pathfileexists(pszPath) && _pathisdirectory(pszPath)) {
		TCHAR buf[MAX_PATH * 2];
		for (int i = 0; i < 1000; i++) {
			wsprintf(buf, _T("%s%03d.tmp"), pszPrefix, i);
			PathCombine(pszName, pszPath, buf);
			if (!_pathfileexists(pszName))
				return TRUE;
		}
	}
	return FALSE;
}

extern "C"
BOOL WINAPI lccMemSaveToTempFile(HGLOBAL hmem, LPTSTR pszFile)
{
	BOOL bOK = FALSE;
	if (hmem) {
		TCHAR buf[MAX_PATH * 2];
		GetTempPath(ARRAYSIZE(buf), buf);
		if (lccGetTempFileName(buf, _T("~ls~"), pszFile)) {
			HANDLE hf = CreateFile(pszFile, GENERIC_WRITE, 0, NULL,
								CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hf != INVALID_HANDLE_VALUE) {
				SIZE_T size = GlobalSize(hmem);
				void* pv = GlobalLock(hmem);
				if (pv) {
					DWORD dw;
					if (WriteFile(hf, pv, (DWORD)size, &dw, NULL) && dw == size)
						bOK = TRUE;
					GlobalUnlock(hmem);
				}
				CloseHandle(hf);

				if (!bOK)
					DeleteFile(pszFile);
			}
		}
	}

	if (bOK)
		GetShortPathName(pszFile, pszFile, MAX_PATH * 2);
	return bOK;
}

extern "C"
HGLOBAL WINAPI lccMemLoadFromTempFile(LPTSTR pszFile)
{
	HGLOBAL hmem = NULL;
	if (pszFile && _pathfileexists(pszFile))
	{
		HANDLE hf = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ,
								NULL, OPEN_EXISTING, 0, NULL);
		if (hf != INVALID_HANDLE_VALUE)
		{
			DWORD size = GetFileSize(hf, NULL);
			if (size > 0)
			{
				hmem = GlobalAlloc(GMEM_MOVEABLE, size);
				if (hmem)
				{
					void* pv = GlobalLock(hmem);
					DWORD dw;
					BOOL bOK = (ReadFile(hf, pv, size, &dw, NULL) && dw == size);
					GlobalUnlock(hmem);
					if (!bOK)
					{
						GlobalFree(hmem);
						hmem = NULL;
					}
				}
			}
			CloseHandle(hf);
			DeleteFile(pszFile);
		}
	}
	return hmem;
}


extern "C"
HGLOBAL WINAPI lccDuplicateMemoryHandle(HGLOBAL hmem, UINT flag)
{
	HGLOBAL hmemNew = NULL;
	if (hmem) {
		SIZE_T size = GlobalSize(hmem);
		hmemNew = GlobalAlloc(flag, size);
		if (hmemNew) {
			LPVOID lps = GlobalLock(hmem);
			LPVOID lpt = GlobalLock(hmemNew);
			memcpy(lpt, lps, size);
			GlobalUnlock(hmem);
			GlobalUnlock(hmemNew);
		}
	}
	return hmemNew;
}

static BOOL GetTextualSid(
      PSID pSid,          // バイナリの Sid
      LPTSTR TextualSid,  // テキスト表現した Sid 用のバッファ
      LPDWORD dwBufferLen // TextualSid 用に必要な / 用意されたバッファの
                          // サイズ
      )
{
      PSID_IDENTIFIER_AUTHORITY psia;
      DWORD dwSubAuthorities;
      DWORD dwSidRev=SID_REVISION;
      DWORD dwCounter;
      DWORD dwSidSize;

      //
      // 渡された Sid が有効であるかチェックする
      //
      if(!IsValidSid(pSid)) return FALSE;

      // SidIdentifierAuthority を得る
      psia=GetSidIdentifierAuthority(pSid);

      // sidsubauthority カウントを得る
      dwSubAuthorities=*GetSidSubAuthorityCount(pSid);

      //
      // 以下のようにしてバッファ サイズを計算する
      // S-SID_REVISION- + identifierauthority- + subauthorities- + NULL
      //
      dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

      //
      // 用意されたバッファ サイズを確認する
      // 十分でない場合、必要なサイズを示しエラー情報を設定する
      //
      if (*dwBufferLen < dwSidSize)
      {
          *dwBufferLen = dwSidSize;
          SetLastError(ERROR_INSUFFICIENT_BUFFER);
          return FALSE;
      }

      //
      // S-SID_REVISION- を準備する
      //
      dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );

      //
      // SidIdentifierAuthority の準備
      //
      if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
      {
          dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                      TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                      (USHORT)psia->Value[0],
                      (USHORT)psia->Value[1],
                      (USHORT)psia->Value[2],
                      (USHORT)psia->Value[3],
                      (USHORT)psia->Value[4],
                      (USHORT)psia->Value[5]);
      }
      else
      {
          dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                      TEXT("%lu"),
                      (ULONG)(psia->Value[5]      )   +
                      (ULONG)(psia->Value[4] <<  8)   +
                      (ULONG)(psia->Value[3] << 16)   +
                      (ULONG)(psia->Value[2] << 24)   );
      }

      //
      // SidSubAuthorities が終了するまで繰り返す
      //
      for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
      {
          dwSidSize+=wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                      *GetSidSubAuthority(pSid, dwCounter) );
      }

      return TRUE;
}

#define SZ_REL_SD_BUF		1000
#define SZ_SID_OWN_BUF		256
#define SZ_SID_PG_BUF		256
#define SZ_ACCT_NAME_BUF	256
#define SZ_DMN_NAME_BUF		128

extern "C"
BOOL WINAPI lccGetFileCreateOwner(LPCTSTR pszFile, LPTSTR pszCreateOwner)
{
	UCHAR ucBuf[SZ_REL_SD_BUF];
	UCHAR ucBufCtrl[sizeof(PSECURITY_DESCRIPTOR_CONTROL)];
	UCHAR ucBufSidOwn[SZ_SID_OWN_BUF];
	DWORD dwSDLength = SZ_REL_SD_BUF;
	PSID psidSidOwn = (PSID)&ucBufSidOwn;
	DWORD dwSDLengthNeeded;
	PSECURITY_DESCRIPTOR psdSrelFileSD = (PSECURITY_DESCRIPTOR)&ucBuf;
	PSECURITY_DESCRIPTOR_CONTROL psdcCtrl = (PSECURITY_DESCRIPTOR_CONTROL)&ucBufCtrl;
	if (!GetFileSecurity(pszFile,
						(SECURITY_INFORMATION)(OWNER_SECURITY_INFORMATION),
						psdSrelFileSD,
						dwSDLength,
						(LPDWORD)&dwSDLengthNeeded))
		return FALSE;

	if (!IsValidSecurityDescriptor(psdSrelFileSD))
		return FALSE;

	DWORD dwRevision;
	if (!GetSecurityDescriptorControl(psdSrelFileSD, psdcCtrl, &dwRevision))
		return FALSE;

//	BOOL bSDSelfRelative = (SE_SELF_RELATIVE & *psdcCtrl);
	BOOL bOwnerDefaulted = (SE_OWNER_DEFAULTED & *psdcCtrl);
	if (GetSecurityDescriptorOwner(psdSrelFileSD, &psidSidOwn, &bOwnerDefaulted)) {
		TCHAR szAccount[SZ_ACCT_NAME_BUF], szDomain[SZ_DMN_NAME_BUF];
		DWORD lenAccount = sizeof(szAccount);
        DWORD lenDomain = sizeof(szDomain);
		SID_NAME_USE peAcctNameUse;

		if (LookupAccountSid(NULL, psidSidOwn, szAccount, &lenAccount, szDomain, &lenDomain, &peAcctNameUse)) {
			if (!StrCmpI(szDomain, _T("BUILTIN"))) {
				lenDomain = sizeof(szDomain);
				GetComputerName(szDomain, &lenDomain);
			}
			PathCombine(pszCreateOwner, szDomain, szAccount);
		}
		else {
			lenAccount = sizeof(szAccount);
			GetTextualSid(psidSidOwn, szAccount, &lenAccount);
			lstrcpy(pszCreateOwner, szAccount);
		}
		return TRUE;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI lccAddTextToListbox(HWND hlist, LPCTSTR pText)
{
	CListBox list = hlist;
	if (!list.IsWindow()) return FALSE;

	list.ResetContent();
	if (!pText && *pText == 0) return TRUE;

	CString str = pText;
	str.Replace(_T("\x0d\x0a"), _T("\n"));
	str.Replace(_T('\x0d'), _T('\n'));
	str.Replace(_T('\x0a'), _T('\n'));
	str.Replace(_T('\t'), _T(' '));

	CClientDC dc(list);
	HFONT hFont = dc.SelectFont(list.GetFont());

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	list.SetRedraw(FALSE);
	int pos = 0;
	int maxwidth = 0;
	CString token = str.Tokenize(_T("\n"), pos);
	while (token != _T("")) {
		if (list.AddString(token) < 0)
			break;

		CSize size;
		dc.GetTextExtent(token, token.GetLength(), &size);
		if (size.cx > maxwidth)
			maxwidth = size.cx;

		token = str.Tokenize(_T("\n"), pos);
	};

	dc.SelectFont(hFont);

	list.SetHorizontalExtent(maxwidth + tm.tmAveCharWidth);

	list.SetCurSel(0);
	list.SetRedraw(TRUE);
	list.UpdateWindow();
	return TRUE;
}

extern "C"
void WINAPI lccCompactPath(HWND hwnd, LPCTSTR pszText)
{
	if (pszText || *pszText != 0) {
		CPath path(pszText);
		CClientDC dc(hwnd);
		CRect rc;
		GetClientRect(hwnd, &rc);
		path.CompactPath(dc, rc.Width());
		SetWindowText(hwnd, path);
	}
	else
		SetWindowText(hwnd, NULL);
}

extern "C"
void WINAPI lccCommandPrompt(LPCTSTR pszInitialPath)
{
	TCHAR dos[MAX_PATH * 2], path[MAX_PATH * 2];
	if (GetEnvironmentVariable(_T("COMSPEC"), dos, ARRAYSIZE(dos)) == 0) { lstrcpy(dos, _T("cmd.exe")); }
	if (!pszInitialPath || !_pathisdirectory(pszInitialPath))
		GetCurrentDirectory(ARRAYSIZE(path), path);
	else
		lstrcpy(path, pszInitialPath);

	STARTUPINFO sui;
	memset(&sui, 0, sizeof(sui));
	sui.cb = sizeof(sui);

	PROCESS_INFORMATION pi;
	CreateProcess(NULL, dos, NULL, NULL, FALSE,
				NORMAL_PRIORITY_CLASS, NULL, path, &sui, &pi);
}

extern "C"
void WINAPI lccDrawBitmap(HDC hdc, HBITMAP hbm, int x, int y, int bx, int by)
{
	CDCHandle dc = hdc;
	CDC dcw = CreateCompatibleDC(dc);
	HBITMAP hbmOld = dcw.SelectBitmap(hbm);

	// マッピングモードはディスプレイのＤＣと同一にする
	dcw.SetMapMode(dc.GetMapMode());

	BITMAP bm;
	GetObject(hbm, sizeof(BITMAP), (LPTSTR)&bm);

	CPoint ptSiz(bm.bmWidth, bm.bmHeight);
	CPoint ptOrg(bx, by);

	// 物理座標→論理座標変換
	dc.DPtoLP(&ptSiz, 1);
	dcw.DPtoLP(&ptOrg, 1);
	dc.BitBlt(x, y, ptSiz.x, ptSiz.y, dcw, ptOrg.x, ptOrg.y, SRCCOPY);
	dcw.SelectBitmap(hbmOld);
}

extern "C"
void WINAPI lccScreenToClientRect(HWND hwnd, LPRECT lprc)
{

	ScreenToClient(hwnd, (LPPOINT)&lprc->left);
	ScreenToClient(hwnd, (LPPOINT)&lprc->right);
}

extern "C"
void WINAPI lccClientToScreenRect(HWND hwnd, LPRECT lprc)
{
	ClientToScreen(hwnd, (LPPOINT)&lprc->left);
	ClientToScreen(hwnd, (LPPOINT)&lprc->right);
}

extern "C"
void WINAPI lccCenterWindow(HWND hwnd, HWND hwndParent)
{
	int cxScreen = GetSystemMetrics(SM_CXSCREEN);
	int cyScreen = GetSystemMetrics(SM_CYSCREEN);

	CRect rc;
	GetWindowRect(hwnd, &rc);
	int cx = rc.Width();
	int cy = rc.Height();

	int x, y;
	if (IsWindow(hwndParent))
	{
		GetWindowRect(hwndParent, &rc);
		x = rc.left + (rc.Width() - cx) / 2;
		y = rc.top + (rc.Height() - cy) / 2;
		if (x + cx > cxScreen)
			x = cxScreen - cx;
		if (y + cy > cyScreen)
			y = cyScreen - cy;
	}
	else
	{
		x = (cxScreen - cx) / 2;
		y = (cyScreen - cy) / 2;
	}
	SetWindowPos(hwnd, NULL, max(x, 0), max(y, 0), 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

extern "C"
HICON WINAPI lccCreateUpdirIcon()
{
/* @@@ 未使用 @@@
	SHFILEINFO sfi;
	HIMAGELIST hImage = (HIMAGELIST)SHGetFileInfo(_T("dir::"),
								FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
								SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX |
								SHGFI_LARGEICON);
	HICON hIcon = ImageList_GetIcon(hImage, sfi.iIcon, ILD_NORMAL);

	ICONINFO icf;
	GetIconInfo(hIcon, &icf);

	BITMAP bm;
	GetObject(icf.hbmColor, sizeof(bm), &bm);

	CClientDC dc(nullptr);
	CDC dcw = CreateCompatibleDC(dc);
	HBITMAP hbm = dcw.SelectBitmap(icf.hbmColor);

	HICON hUpdir = LoadIcon(RESINST, _MKIR(IDI_UPDIR));
	dcw.DrawIcon((bm.bmWidth - 32) / 2, (bm.bmHeight - 32) / 2, hUpdir);
	DestroyIcon(hUpdir);
	dcw.SelectBitmap(hbm);

	HICON hIconNew = CreateIconIndirect(&icf);
	DestroyIcon(hIcon);
	return hIconNew;
@@@@@@@@@@@@@@@@@ */
	return NULL;
}

static TCHAR sszRDtit[] = _T("DLLExecute");

extern "C"
BOOL WINAPI lccDllExecute(HWND hwnd, LPCTSTR pszDll, LPCTSTR pszFunc, LPCTSTR pszFile, LPCTSTR pszIcon, BOOL bAsAdmin)
{
	TCHAR buf[MAX_PATH * 2], dir[MAX_PATH * 2];
	CString exec, cmd;
	BOOL bOK = FALSE;
	if (lccRegGetLibraryPath(buf)) {
		// execdll.exeのフルパス
		PathCombine(exec.GetBuffer(MAX_PATH * 2), buf, _T("execdll.exe"));
		exec.ReleaseBuffer();
		BOOL hasExec = _pathfileexists(exec);
		if (!hasExec) {
			// 見つからなければrundll32を代用
			exec = _T("rundll32.exe");
		}

		// 実行DLLのフルパス
		PathAppend(buf, pszDll);
		if (!_pathfileexists(buf)) {
			GetSystemDirectory(buf, ARRAYSIZE(buf));
			PathAppend(buf, pszDll);
			if (!_pathfileexists(buf)) {
				lccErrorBox(hwnd, ERROR_MOD_NOT_FOUND, sszRDtit, pszDll, MB_OK | MB_ICONSTOP);
				return FALSE;
			}
		}

		lstrcpy(dir, buf);
		PathRemoveFileSpec(dir);
		GetShortPathName(buf, buf, ARRAYSIZE(buf));

		if (hasExec && pszIcon && *pszIcon)
			cmd.Format(_T("%s,"), pszIcon);
		cmd.AppendFormat(_T("%s,%s %s"), buf, pszFunc, pszFile);

		TCHAR curdir[MAX_PATH * 2];
		GetCurrentDirectory(ARRAYSIZE(curdir), curdir);
		SetCurrentDirectory(dir);

		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(sei));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_UNICODE;
		sei.hwnd = hwnd;
		sei.lpVerb = bAsAdmin ? _T("runas") : NULL;
		sei.lpFile = exec.GetString();
		sei.lpParameters = cmd.GetString();
		sei.lpDirectory = dir;
		sei.nShow = SW_SHOWNORMAL;
		bOK = ShellExecuteEx(&sei);
		DWORD err = GetLastError();

		SetCurrentDirectory(curdir);

		if (!bOK)
			lccErrorBox(hwnd, err, sszRDtit, pszDll, MB_OK | MB_ICONSTOP);
	}
	return bOK;
}

extern "C"
BOOL WINAPI lccRunDllEx(HWND hwnd, LPCTSTR pszDll, LPCTSTR pszFunc, LPCTSTR pszFile, BOOL bAsAdmin)
{
	return lccDllExecute(hwnd, pszDll, pszFunc, pszFile, NULL, bAsAdmin);
}

extern "C"
BOOL WINAPI lccRunDll(HWND hwnd, LPCTSTR pszDll, LPCTSTR pszFunc, LPCTSTR pszFile)
{
	return lccDllExecute(hwnd, pszDll, pszFunc, pszFile, NULL, FALSE);
}

extern "C"
int WINAPI lccDrawShadowText(
	HDC hdc,
	LPCWSTR pszText,
	UINT cch,
	const RECT *pRect,
	DWORD dwFlags,
	COLORREF crText,
	COLORREF crShadow,
	int ixOffset,
	int iyOffset)
{
	return DrawShadowText(hdc, pszText, cch, (LPRECT)pRect, dwFlags, crText, crShadow, ixOffset, iyOffset);
}

#define DRAG_DELTA		8

extern "C"
BOOL WINAPI lccBeginTrack2(HWND hwnd, POINT pt, DWORD waittime)		// pt = スクリーン座標
{
	SetCapture(hwnd);
	BOOL bDragStarted = FALSE;

	// ドラッグチェック矩形を作成 ( 16 x 16 )
	CRect rc(pt.x - DRAG_DELTA, pt.y - DRAG_DELTA, pt.x + DRAG_DELTA, pt.y + DRAG_DELTA);
	CPoint ptCur;
	GetCursorPos(&ptCur);
	if (rc.PtInRect(ptCur)) {
		DWORD dwLastTick = GetTickCount();
		while (!bDragStarted) {
			// キャプチャチェック
			if (GetCapture() != hwnd)
				break;

			// メッセージチェック
			MSG msg;
			if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
				PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			{
				// マウスボタンチェック
				if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP ||
					msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN)
					break;

				// キーボードチェック
				if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
					break;

				// カーソル矩形チェック
				if (msg.message == WM_MOUSEMOVE)
					bDragStarted = !rc.PtInRect(msg.pt);
			}

			// 時間チェック
			if (GetTickCount() - dwLastTick > waittime)
				bDragStarted = TRUE;
		}
	}
	ReleaseCapture();
	return bDragStarted;
}


extern "C"
BOOL WINAPI lccBeginTrack(HWND hwnd, POINT pt)		// pt = スクリーン座標
{
	return lccBeginTrack2(hwnd, pt, 2000);
}

static TCHAR sszDCI[] = _T("lscDCIprop");

static LRESULT CALLBACK dciproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC pfn = (WNDPROC)GetProp(hwnd, sszDCI);
	switch (msg) {
		case WM_KEYDOWN:
			switch (wParam) {
				case VK_DELETE:
				case VK_BACK:
					return 0;
				default:
					break;
			}
			break;

		case WM_CHAR:
			return 0;

		case WM_DESTROY:
			RemoveProp(hwnd, sszDCI);
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pfn);
			break;

		default:
			break;
	}
	return CallWindowProc(pfn, hwnd, msg, wParam, lParam);
}

extern "C"
BOOL WINAPI lccDisableCharInput(HWND hwnd)
{
	if (IsWindow(hwnd) && !GetProp(hwnd, sszDCI)) {
		WNDPROC pfn = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
		SetProp(hwnd, sszDCI, (HANDLE)pfn);
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)dciproc);
		return TRUE;
	}
	return FALSE;
}

extern "C"
void WINAPI TileCascadeWindowsW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	// 形式は "<n> <classname>"

	if (!lpCmdLine || !IsDigit(lpCmdLine[0]) || lpCmdLine[1] != ' ') return;
	int type = *lpCmdLine - '0';
	TCHAR* pClassName = lpCmdLine + 2;

	CSimpleArray<HWND> array;
	HWND hDesktop = GetDesktopWindow();
	hwnd = GetTopWindow(hDesktop);
	for (; hwnd !=NULL; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) {
		TCHAR buf[MAX_PATH];
		GetClassName(hwnd, buf, ARRAYSIZE(buf));
		if (!StrCmp(pClassName, buf) && IsWindowVisible(hwnd) && !IsIconic(hwnd)) {
			array.Add(hwnd);
		}
	}

	int n = (int)array.GetSize();
	if (n > 1) {
		HWND hDesktop = ::GetDesktopWindow();
		switch (type) {
		case 0:					// Cascade
			CascadeWindows(hDesktop, MDITILE_SKIPDISABLED | MDITILE_ZORDER, NULL, n, array.GetData());
			break;
		case 1:					// TileVert
			TileWindows(hDesktop, MDITILE_SKIPDISABLED | MDITILE_ZORDER | MDITILE_VERTICAL, NULL, n, array.GetData());
			break;
		case 2:					// TileHorz
			TileWindows(hDesktop, MDITILE_SKIPDISABLED | MDITILE_ZORDER | MDITILE_HORIZONTAL, NULL, n, array.GetData());
			break;
		default:
			return;
		}
		UpdateWindow(hDesktop);
	}
}

extern "C"
void WINAPI TileCascadeWindows(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR cmd[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, ARRAYSIZE(cmd));
	TileCascadeWindowsW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI lccMapRectToProgmanPane(RECT* prc)
{
	if (prc) {
		CRect rc;
		SystemParametersInfo(SPI_GETWORKAREA, 0, rc, 0);
		prc->left += rc.left;
		prc->right += rc.left;
		prc->top += rc.top;
		prc->bottom += rc.top;

		// 指定矩形が含まれるモニタを取得する
		if (MonitorFromRect(prc, MONITOR_DEFAULTTONULL) == NULL) {
			// 矩形が含まれるモニタが無いのでプライマリモニタ内で矩形を調整する
			CopyRect(prc, &rc);
			InflateRect(prc, -(rc.Width() / 16), -(rc.Height() / 16));
		} else if (GetSystemMetrics(SM_CMONITORS) < 2) {
			int sx = GetSystemMetrics(SM_CXSCREEN);
			int sy = GetSystemMetrics(SM_CYSCREEN);
			prc->left   = max(0, min(prc->left, sx));
			prc->top    = max(0, min(prc->top, sy));
			prc->right  = min(prc->right - prc->left, sx) + prc->left;
			prc->bottom = min(prc->bottom - prc->top, sy) + prc->top;
		}
	}
}

extern "C"
void WINAPI lccUnmapRectFromProgmanPane(RECT* prc)
{
	if (prc) {
		CRect rc;
		SystemParametersInfo(SPI_GETWORKAREA, 0, rc, 0);
		prc->left -= rc.left;
		prc->right -= rc.left;
		prc->top -= rc.top;
		prc->bottom -= rc.top;
	}
}

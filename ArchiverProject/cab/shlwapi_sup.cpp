////////////////////////////////////////////////////////////////////////
// shlwapi コンパチ

#include "stdafx.h"
#include "math.h"


BOOL xPathIsRelative(LPCWSTR lpszPath)
{
	if (!lpszPath || !*lpszPath)
		return TRUE;
	if (*lpszPath == '\\' || (*lpszPath && lpszPath[1] == ':'))
		return FALSE;
	return TRUE;
}

BOOL xPathIsRoot(LPCWSTR lpszPath)
{
	if (lpszPath && *lpszPath) {
		if (*lpszPath == '\\') {
			if (!lpszPath[1])
				return TRUE;			/* \ */
			else if (lpszPath[1] == '\\') {
				BOOL bSeenSlash = FALSE;
				lpszPath += 2;

				// Check for UNC root path
				while (*lpszPath) {
					if (*lpszPath == '\\') {
						if (bSeenSlash)
							return FALSE;
						bSeenSlash = TRUE;
					}
					lpszPath++;
				}
				return TRUE;
			}
		}
		else if (lpszPath[1] == ':' && lpszPath[2] == '\\' && lpszPath[3] == 0)
			return TRUE;				/* X:\ */
	}
	return FALSE;
}

LPWSTR xPathFindFileName(LPCWSTR lpszPath)
{
	LPCWSTR lastSlash = lpszPath;
	while (lpszPath && *lpszPath) {
		if ((*lpszPath == '\\' || *lpszPath == '/' || *lpszPath == ':') &&
			lpszPath[1] && lpszPath[1] != '\\' && lpszPath[1] != '/')
			lastSlash = lpszPath + 1;
		lpszPath++;
	}
	return (LPWSTR)lastSlash;
}

LPWSTR xPathFindExtension(LPCWSTR lpszPath)
{
	LPCWSTR lastpoint = NULL;
	if (lpszPath) {
		while (*lpszPath) {
			if (*lpszPath == '\\' || *lpszPath==' ')
				lastpoint = NULL;
			else if (*lpszPath == '.')
				lastpoint = lpszPath;
			lpszPath++;
		}
	}
	return (LPWSTR)(lastpoint ? lastpoint : lpszPath);
}

void xPathRemoveExtension(LPWSTR lpszPath)
{
	if (lpszPath) {
		lpszPath = xPathFindExtension(lpszPath);
		*lpszPath = 0;
	}
}

LPWSTR xPathAddBackslash(LPWSTR lpszPath)
{
	size_t iLen;
	if (!lpszPath || (iLen = wcslen(lpszPath)) >= MAX_PATH)
		return NULL;

	if (iLen) {
		lpszPath += iLen;
		if (lpszPath[-1] != '\\') {
			*lpszPath++ = '\\';
			*lpszPath = 0;
		}
	}
	return lpszPath;
}

BOOL xPathRemoveFileSpec(LPWSTR lpszPath)
{
	LPWSTR lpszFileSpec = lpszPath;
	BOOL bModified = FALSE;

	if(lpszPath) {
		/* Skip directory or UNC path */
		if (*lpszPath == '\\')
			lpszFileSpec = ++lpszPath;
		if (*lpszPath == '\\')
			lpszFileSpec = ++lpszPath;

		while (*lpszPath) {
			if(*lpszPath == '\\')
				lpszFileSpec = lpszPath;		/* Skip dir */
			else if(*lpszPath == ':') {
				lpszFileSpec = ++lpszPath;		/* Skip drive */
				if (*lpszPath == '\\')
					lpszFileSpec++;
			}
			lpszPath++;
			if (*lpszPath == 0)
				break;
		}
		if (*lpszFileSpec) {
			*lpszFileSpec = 0;
			bModified = TRUE;
		}
	}
	return bModified;
}

BOOL xPathCanonicalize(LPWSTR lpszBuf, LPCWSTR lpszPath)
{
	LPWSTR lpszDst = lpszBuf;
	LPCWSTR lpszSrc = lpszPath;

	if (lpszBuf)
		*lpszDst = 0;

	if (!lpszBuf || !lpszPath) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (!*lpszPath) {
		*lpszBuf++ = '\\';
		*lpszBuf = 0;
		return TRUE;
	}

	/* Copy path root */
	if (*lpszSrc == '\\') {
		*lpszDst++ = *lpszSrc++;
	}
	else if (*lpszSrc && lpszSrc[1] == ':') {
		/* X:\ */
		*lpszDst++ = *lpszSrc++;
		*lpszDst++ = *lpszSrc++;
		if (*lpszSrc == '\\')
			*lpszDst++ = *lpszSrc++;
	}

	/* Canonicalize the rest of the path */
	while (*lpszSrc) {
		if (*lpszSrc == '.') {
			if (lpszSrc[1] == '\\' && (lpszSrc == lpszPath || lpszSrc[-1] == '\\' || lpszSrc[-1] == ':')) {
				lpszSrc += 2; /* Skip .\ */
			}
			else if (lpszSrc[1] == '.' && (lpszDst == lpszBuf || lpszDst[-1] == '\\')) {
				/* \.. backs up a directory, over the root if it has no \ following X:.
				 * .. is ignored if it would remove a UNC server name or inital \\
				 */
				if (lpszDst != lpszBuf) {
					*lpszDst = 0;					/* Allow PathIsUNCServerShareA test on lpszBuf */
					if (lpszDst > lpszBuf+1 && lpszDst[-1] == '\\' && (lpszDst[-2] != '\\' || lpszDst > lpszBuf+2)) {
						if (lpszDst[-2] == ':' && (lpszDst > lpszBuf+3 || lpszDst[-3] == ':')) {
							lpszDst -= 2;
							while (lpszDst > lpszBuf && *lpszDst != '\\')
								lpszDst--;
							if (*lpszDst == '\\')
								lpszDst++;				/* Reset to last '\' */
							else
								lpszDst = lpszBuf;		/* Start path again from new root */
						}
						else if (lpszDst[-2] != ':' && !xPathIsUNCServerShare(lpszBuf))
							lpszDst -= 2;
					}
					while (lpszDst > lpszBuf && *lpszDst != '\\')
						lpszDst--;
					if (lpszDst == lpszBuf) {
						*lpszDst++ = '\\';
						lpszSrc++;
					}
				}
				lpszSrc += 2;							/* Skip .. in src path */
			}
			else
				*lpszDst++ = *lpszSrc++;
		}
		else
			*lpszDst++ = *lpszSrc++;
	}
	/* Append \ to naked drive specs */
	if (lpszDst - lpszBuf == 2 && lpszDst[-1] == ':')
		*lpszDst++ = '\\';
	*lpszDst++ = 0;
	return TRUE;
}

LPWSTR xPathCombine(LPWSTR lpszDest, LPCWSTR lpszDir, LPCWSTR lpszFile)
{
	WCHAR szTemp[MAX_PATH];
	BOOL bUseBoth = FALSE, bStrip = FALSE;

	if (!lpszDest || (!lpszDir && !lpszFile))
		return NULL;							/* Invalid parameters */

	if ((!lpszFile || !*lpszFile) && lpszDir) {
		/* Use dir only */
		wcsncpy(szTemp, lpszDir, MAX_PATH);
	}
	else if (!lpszDir || !*lpszDir || !xPathIsRelative(lpszFile)) {
		if (!lpszDir || !*lpszDir || *lpszFile != '\\' || xPathIsUNC(lpszFile)) {
			/* Use file only */
			wcsncpy(szTemp, lpszFile, MAX_PATH);
		}
		else {
			bUseBoth = TRUE;
			bStrip = TRUE;
		}
	}
	else
		bUseBoth = TRUE;

	if (bUseBoth) {
		wcsncpy(szTemp, lpszDir, MAX_PATH);
		if (bStrip) {
			xPathStripToRoot(szTemp);
			lpszFile++;							/* Skip '\' */
		}
		if (!xPathAddBackslash(szTemp))
			return NULL;
		if (wcslen(szTemp) + wcslen(lpszFile) >= MAX_PATH)
			return NULL;
		wcscat(szTemp, lpszFile);
	}
	xPathCanonicalize(lpszDest, szTemp);
	return lpszDest;
}

BOOL xPathIsUNC(LPCWSTR lpszPath)
{
	if (lpszPath && (lpszPath[0]=='\\') && (lpszPath[1]=='\\'))
		return TRUE;
	return FALSE;
}

BOOL xPathIsUNCServerShare(LPCWSTR lpszPath)
{
	if (lpszPath && *lpszPath++ == '\\' && *lpszPath++ == '\\') {
		BOOL bSeenSlash = FALSE;
		while (*lpszPath) {
			if (*lpszPath == '\\') {
				if (bSeenSlash)
					return FALSE;
				bSeenSlash = TRUE;
			}
			lpszPath++;
		}
		return bSeenSlash;
	}
	return FALSE;
}

BOOL xPathIsDirectory(LPCWSTR lpszPath)
{
	if (!lpszPath || xPathIsUNCServer(lpszPath))
		return FALSE;

//	if (PathIsUNCServerShareW(lpszPath)) {
//		FIXME("UNC Server Share not yet supported - FAILING\n");
//		return FALSE;
//	}

	DWORD attr = GetFileAttributesW(lpszPath);
	if (attr == INVALID_FILE_ATTRIBUTES)
		return FALSE;
	return ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

BOOL xPathFileExists(LPCWSTR lpszPath)
{
	if (!lpszPath)
		return FALSE;

	UINT iPrevErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD attr = GetFileAttributesW(lpszPath);
	SetErrorMode(iPrevErrMode);
	return (attr != INVALID_FILE_ATTRIBUTES);
}

BOOL xPathUnquoteSpaces(LPWSTR lpszPath)
{
	if (lpszPath && *lpszPath == '"') {
		DWORD dwLen = wcslen(lpszPath) - 1;
		if (lpszPath[dwLen] == '"') {
			lpszPath[dwLen] = 0;
			for (; *lpszPath; lpszPath++)
				*lpszPath = lpszPath[1];
		}
	}
	return TRUE;
}

BOOL WINAPI PathRenameExtensionW(LPWSTR lpszPath, LPCWSTR lpszExt)
{
	LPWSTR lpszExtension = PathFindExtensionW(lpszPath);
	if (!lpszExtension || (lpszExtension - lpszPath + wcslen(lpszExt) >= MAX_PATH))
		return FALSE;

	wcscpy(lpszExtension, lpszExt);
	return TRUE;
}


static BOOL WINAPI SHLWAPI_ChrCmpHelperW(WCHAR ch1, WCHAR ch2, DWORD dwFlags)
{
	WCHAR str1[2], str2[2];

	str1[0] = ch1;
	str1[1] = 0;
	str2[0] = ch2;
	str2[1] = 0;
	return CompareStringW(GetThreadLocale(), dwFlags, str1, 2, str2, 2) - 2;
}

static LPWSTR WINAPI SHLWAPI_StrRChrHelperW(LPCWSTR lpszStr, LPCWSTR lpszEnd, WCHAR ch, BOOL (WINAPI *pChrCmpFn)(WCHAR,WCHAR))
{
	LPCWSTR lpszRet = NULL;

	if (lpszStr) {
		if (!lpszEnd)
			lpszEnd = lpszStr + wcslen(lpszStr);

		while (*lpszStr && lpszStr <= lpszEnd) {
			if (!pChrCmpFn(ch, *lpszStr))
				lpszRet = lpszStr;
			lpszStr = CharNextW(lpszStr);
		}
	}
	return (LPWSTR)lpszRet;
}

LPWSTR WINAPI StrChrW(LPCWSTR lpszStr, WCHAR ch)
{
	LPWSTR lpszRet = NULL;
	if (lpszStr)
		lpszRet = (LPWSTR)wcschr(lpszStr, ch);
	return lpszRet;
}

static BOOL WINAPI SHLWAPI_ChrCmpW(WCHAR ch1, WCHAR ch2)
{
	return SHLWAPI_ChrCmpHelperW(ch1, ch2, 0);
}

LPWSTR WINAPI StrRChrW(LPCWSTR lpszStr, LPCWSTR lpszEnd, WORD ch)
{
	return SHLWAPI_StrRChrHelperW(lpszStr, lpszEnd, ch, SHLWAPI_ChrCmpW);
}

BOOL WINAPI StrToIntExW(LPCWSTR lpszStr, DWORD dwFlags, LPINT lpiRet)
{
	BOOL bNegative = FALSE;
	int iRet = 0;

	if (!lpszStr || !lpiRet)
		return FALSE;

	/* Skip leading space, '+', '-' */
	while (iswspace(*lpszStr))
		lpszStr = CharNextW(lpszStr);

	if (*lpszStr == '-') {
		bNegative = TRUE;
		lpszStr++;
	}
	else if (*lpszStr == '+')
		lpszStr++;

	if (dwFlags & STIF_SUPPORT_HEX && *lpszStr == '0' && towlower(lpszStr[1]) == 'x') {
		/* Read hex number */
		lpszStr += 2;

		if (!iswxdigit(*lpszStr))
			return FALSE;

		while (iswxdigit(*lpszStr)) {
			iRet = iRet * 16;
			if (iswdigit(*lpszStr))
				iRet += (*lpszStr - '0');
			else
				iRet += 10 + (towlower(*lpszStr) - 'a');
			lpszStr++;
		}
		*lpiRet = iRet;
		return TRUE;
	}

	/* Read decimal number */
	if (!iswdigit(*lpszStr))
		return FALSE;

	while (iswdigit(*lpszStr)) {
		iRet = iRet * 10;
		iRet += (*lpszStr - '0');
		lpszStr++;
	}
	*lpiRet = bNegative ? -iRet : iRet;
	return TRUE;
}

int WINAPI StrToIntW(LPCWSTR lpszStr)
{
	if (!lpszStr)
		return 0;

	int iRet = 0;
	if (*lpszStr == '-' || iswdigit(*lpszStr))
		StrToIntExW(lpszStr, 0, &iRet);
	return iRet;
}

static void FillNumberFmt(NUMBERFMTW *fmt, LPWSTR decimal_buffer, int decimal_bufwlen, LPWSTR thousand_buffer, int thousand_bufwlen)
{
	WCHAR grouping[64];
	WCHAR *c;

	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_ILZERO|LOCALE_RETURN_NUMBER, (LPWSTR)&fmt->LeadingZero, sizeof(fmt->LeadingZero)/sizeof(WCHAR));
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER|LOCALE_RETURN_NUMBER, (LPWSTR)&fmt->LeadingZero, sizeof(fmt->NegativeOrder)/sizeof(WCHAR));
	fmt->NumDigits = 0;
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, decimal_buffer, decimal_bufwlen);
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, thousand_buffer, thousand_bufwlen);
	fmt->lpThousandSep = thousand_buffer;
	fmt->lpDecimalSep = decimal_buffer;

	/* 
	 * Converting grouping string to number as described on 
	 * http://blogs.msdn.com/oldnewthing/archive/2006/04/18/578251.aspx
	 */
	fmt->Grouping = 0;
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, grouping, sizeof(grouping)/sizeof(WCHAR));
	for (c = grouping; *c; c++) {
		if (*c >= '0' && *c < '9') {
			fmt->Grouping *= 10;
			fmt->Grouping += *c - '0';
		}
	}

	if (fmt->Grouping % 10 == 0)
		fmt->Grouping /= 10;
	else
	fmt->Grouping *= 10;
}

static int FormatDouble(double value, int decimals, LPWSTR pszBuf, int cchBuf)
{
	static const WCHAR flfmt[] = {'%','f',0};
	WCHAR buf[64];
	NUMBERFMTW fmt;
	WCHAR decimal[8], thousand[8];

	swprintf_s(buf, 64, flfmt, value);

	FillNumberFmt(&fmt, decimal, sizeof decimal / sizeof (WCHAR), thousand, sizeof thousand / sizeof (WCHAR));
	fmt.NumDigits = decimals;
	return GetNumberFormatW(LOCALE_USER_DEFAULT, 0, buf, &fmt, pszBuf, cchBuf);
}

/* Structure for formatting byte strings */
typedef struct tagSHLWAPI_BYTEFORMATS {
	LONGLONG dLimit;
	double   dDivisor;
	double   dNormaliser;
	int      nDecimals;
	WCHAR     wPrefix;
} SHLWAPI_BYTEFORMATS;

LPWSTR WINAPI StrFormatByteSizeW(LONGLONG llBytes, LPWSTR lpszDest, UINT cchMax)
{
#define KB ((ULONGLONG)1024)
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define TB (KB*KB*KB*KB)
#define PB (KB*KB*KB*KB*KB)

	static const SHLWAPI_BYTEFORMATS bfFormats[] = {
		{ 10*KB, 10.24, 100.0, 2, 'K' }, /* 10 KB */
		{ 100*KB, 102.4, 10.0, 1, 'K' }, /* 100 KB */
		{ 1000*KB, 1024.0, 1.0, 0, 'K' }, /* 1000 KB */
		{ 10*MB, 10485.76, 100.0, 2, 'M' }, /* 10 MB */
		{ 100*MB, 104857.6, 10.0, 1, 'M' }, /* 100 MB */
		{ 1000*MB, 1048576.0, 1.0, 0, 'M' }, /* 1000 MB */
		{ 10*GB, 10737418.24, 100.0, 2, 'G' }, /* 10 GB */
		{ 100*GB, 107374182.4, 10.0, 1, 'G' }, /* 100 GB */
		{ 1000*GB, 1073741824.0, 1.0, 0, 'G' }, /* 1000 GB */
		{ 10*TB, 10485.76, 100.0, 2, 'T' }, /* 10 TB */
		{ 100*TB, 104857.6, 10.0, 1, 'T' }, /* 100 TB */
		{ 1000*TB, 1048576.0, 1.0, 0, 'T' }, /* 1000 TB */
		{ 10*PB, 10737418.24, 100.00, 2, 'P' }, /* 10 PB */
		{ 100*PB, 107374182.4, 10.00, 1, 'P' }, /* 100 PB */
		{ 1000*PB, 1073741824.0, 1.00, 0, 'P' }, /* 1000 PB */
		{ 0, 10995116277.76, 100.00, 2, 'E' } /* EB's, catch all */
	};
	WCHAR wszAdd[] = {' ', '?', 'B', 0};
	double dBytes;
	UINT i = 0;

	if (!lpszDest || !cchMax)
		return lpszDest;

	if (llBytes < 1024) {			/* 1K */
		swprintf_s(lpszDest, cchMax, L"%d ", (long)llBytes);
		if (GetOEMCP() == 932)
			wcscat_s(lpszDest, cchMax, L"バイト");
		else
			wcscat_s(lpszDest, cchMax, L"bytes");
		return lpszDest;
	}

	/* Note that if this loop completes without finding a match, i will be
	 * pointing at the last entry, which is a catch all for > 1000 PB
	 */
	while (i < sizeof(bfFormats) / sizeof(SHLWAPI_BYTEFORMATS) - 1) {
		if (llBytes < bfFormats[i].dLimit)
			break;
		i++;
	}

	/* Above 1 TB we encounter problems with FP accuracy. So for amounts above
	 * this number we integer shift down by 1 MB first. The table above has
	 * the divisors scaled down from the '< 10 TB' entry onwards, to account
	 * for this. We also add a small fudge factor to get the correct result for
	 * counts that lie exactly on a 1024 byte boundary.
	 */
	if (i > 8)
		dBytes = (double)(llBytes >> 20) + 0.001;		/* Scale down by I MB */
	else
		dBytes = (double)llBytes + 0.00001;

	dBytes = floor(dBytes / bfFormats[i].dDivisor) / bfFormats[i].dNormaliser;

	if (!FormatDouble(dBytes, bfFormats[i].nDecimals, lpszDest, cchMax))
		return NULL;
	wszAdd[1] = bfFormats[i].wPrefix;
	wcscat_s(lpszDest, cchMax, wszAdd);
	return lpszDest;
}


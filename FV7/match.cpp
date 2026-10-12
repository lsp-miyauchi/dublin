#include "stdafx.h"
#include "..\lscore\atl_special_rx.h"
#include "match.h"
#include "func.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif

//-----------------> 利用しない
#if 0

extern TCHAR gszAst[];

int __fastcall match(LPCTSTR string, LPCTSTR pattern)
{
	if (!StrCmp(pattern, gszAst))
		return 1;
	return PathMatchSpec(string, pattern) ? 1 : 0;
}

static int recmatch(LPCTSTR p, LPCTSTR s)
{
	// Get first character, the pattern for new recmatch calls follows
	UINT c = *p;
	p++;

	// If that was the end of the pattern, match if string empty too
	if (c == 0)
		return *s == 0;

	// '?' matches any character (but not an empty string)
	if (c == '?')
		return *s ? recmatch(p, s + 1) : 0;

	// '*' matches any number of characters, including zero
	if (c == '*') {
		if (*p == 0)
            return 1;
		for (; *s; s++)
			if ((c = recmatch(p, s)) != 0)
				return (int)c;
		return 2;				// 2 means give up--match will return false
    }

	// Parse and process the list of characters and ranges in brackets
	if (c == '[') {
		int e;					// flag true if next char to be taken literally
		LPCTSTR q;				// pointer to end of [-] group
		int r;					// flag true to match anything but the range

		if (*s == 0)			// need a character to match
            return 0;

		int ch = *p;
		p += (r = (ch == '!' || ch == '^'));		// see if reverse
		for (q = p, e = 0; *q; q++)		// find closing bracket
        {
			ch = *q;
			if (e)
				e = 0;
			else if (ch == '\\')		// GRR:  change to ^ for MS-DOS, OS/2?
				e = 1;
			else if (ch == ']')
				break;
		}
		if (*q != ']')			// nothing matches if bad syntax
			return 0;

		// go through the list
		for (c = 0, e = (*p == '-'); p < q; p++)
		{
			ch = *p;
			if (e == 0 && ch == '\\')		// set escape flag if '\'
				e = 1;
			else if (e == 0 && ch == '-')	// set start of range if '-'
				c = *(p - 1);
			else
			{
				UINT cc = *s;
				if (*(p + 1) != '-')
				{
					for (c = c ? c : *p; c <= *p; c++)		// compare range
					{
						if (c == cc)			// typecast for MSC bug
							return r ? 0 : recmatch(q + 1, s + 1);
					}
				}
				c = e = 0;		// clear range, escape flags
			}
		}
		return r ? recmatch(q + 1, s + 1) : 0;	// bracket match failed
	}

	// if escape ('\'), just compare next character
    if (c == '\\')
	{
		c = *p;
		p++;
		if (c == 0)     // if '\' at end, then syntax error
			return 0;
		if (*s == '\\')
			s++;
	}

	// just a character--compare it
	return c == *s ? recmatch(p, s + 1) : 0;
}

int __fastcall match_ex_original(LPCTSTR string, LPCTSTR pattern)
{
	TCHAR dospattern[MAX_PATH];
	lstrcpy(dospattern, pattern);
	int j = lstrlen(pattern);
	if (!StrCmp(dospattern + j - 3, _T("*.*")))
		dospattern[j - 2] = '\0';                    /* nuke the ".*" */
	else if (!StrCmp(dospattern + j - 2, _T("*.")))
	{
		TCHAR *p = StrChr(string, '.');
		if (p)
			return 0;
		dospattern[j - 1] = '\0';
	}

	TCHAR target[MAX_PATH];
	lstrcpy(target, string);
	CharUpper(dospattern);
	CharUpper(target);
	j = recmatch(dospattern, target);
	return j;
}

int __fastcall match_ex(LPCTSTR string, LPCTSTR pattern)
{
	HANDLE h = lccRexInit(pattern);
	if (h) {
		int start;
		BOOL bMatch = lccRexMatch(h, string, &start, NULL);
		lccRexExit(h);
		return (bMatch && start == 0) ? 1 : 0;
	}
	return 0;
}

#endif
// <----------------- 利用しない

////////////////////////////////////////////////////////////////////////
//

static int __fastcall dos_match(LPCTSTR p, LPCTSTR s)
{
	// Get first character, the pattern for new dos_match calls follows
	UINT c = *p;
	p++;

	// If that was the end of the pattern, match if string empty too
	if (c == 0) return (*s == 0);

	// '?' matches any character (but not an empty string)
	if (c == _T('?')) return (*s ? dos_match(p, s + 1) : 0);

	// '*' matches any number of characters, including zero
	if (c == _T('*')) {
		if (*p == 0) return 1;

		for (; *s; s++) {
			int n = dos_match(p, s);
			if (n != 0) return n;
		}
		return -1;				// -1 means give up--match will return false
    }

	// just a character--compare it
	return ((c == *s) ? dos_match(p, s + 1) : 0);
}

BOOL WINAPI PathMatchSpecNoCase(LPCTSTR pszFile, LPCTSTR pszSpec)
{
	TCHAR dospattern[MAX_PATH];
	lstrcpy(dospattern, pszSpec);
	int j = lstrlen(pszSpec);
	if (!StrCmp(dospattern + j - 3, _T("*.*")))
		dospattern[j - 2] = 0;								// nuke the ".*"
	else if (!StrCmp(dospattern + j - 2, _T("*."))) {
		TCHAR *p = StrChr(pszFile, _T('.'));
		if (p)
			return 0;
		dospattern[j - 1] = 0;
	}
	return (dos_match(dospattern, pszFile) == 1);
}


////////////////////////////////////////////////////////////////////////
// class CMatchSpec

CMatchSpec::CMatchSpec(LPCTSTR pszPattern, int matchType, BOOL bCaseSense, TCHAR chDelim)
{
	m_bMatchAll = FALSE;
	m_nMatchType = matchType;
	m_bCaseSense = bCaseSense;
	if (pszPattern && *pszPattern) {
		CString str = pszPattern;
		if (!m_bCaseSense)
			str.MakeLower();

		CSimpleArray<CString> arr;
		SeparateString(str, chDelim, arr);
		m_arrSpec.RemoveAll();

		// 通常一致も正規表現も次のパターンは無条件一致とみなす
		if (arr.GetSize() == 1 &&
			(!StrCmpI(arr[0], _T("*.*")) || !StrCmpI(arr[0], _T("*")))) {
			m_bMatchAll = TRUE;
			return;
		}

		int i;
		if (m_nMatchType == MATCHTYPE_REGEXP) {
			if (arr.GetSize() == 1 &&
				(!StrCmpI(arr[0], _T(".*")) || !StrCmpI(arr[0], _T(".+"))))
				m_bMatchAll = TRUE;
			else {
				for (i = 0; i < arr.GetSize(); i++)
					m_arrSpec.Add(arr[i]);
			}
		}
		else {
			for (i = 0; i < arr.GetSize(); i++) {
				CString mask = arr[i];
				if (mask.FindOneOf(_T("?*")) < 0) {
					// ワイルドカードが指定されていない場合
					CString tmp;
					// 先頭一致パターンは無条件に追加
					tmp.Format(_T("%s*"), (LPCTSTR)mask);
					m_arrSpec.Add(tmp);
					if (m_nMatchType != MATCHTYPE_PREFIX) {
						// *pat*, *patの部分一致・後方一致パターンを追加
						tmp.Format(_T("*%s*"), (LPCTSTR)mask);
						m_arrSpec.Add(tmp);
						tmp.Format(_T("*%s"), (LPCTSTR)mask);
						m_arrSpec.Add(tmp);
					}
				}
				else
					m_arrSpec.Add(arr[i]);
			}
		}
	}
	else {
		m_nMatchType = MATCHTYPE_PARTIAL;
		m_bMatchAll = TRUE;
	}
}

BOOL CMatchSpec::Match(LPCTSTR pszTarget)
{
	if (m_bMatchAll) return TRUE;

	BOOL (WINAPI* pfn)(LPCTSTR, LPCTSTR);
	pfn = PathMatchSpecNoCase;
	BOOL bMatch = FALSE;
	CString str = pszTarget;
	if (!m_bCaseSense) {
		str.MakeLower();
		pfn = PathMatchSpec;
	}

	int cnt = m_arrSpec.GetSize();
	for (int i = 0; i < cnt; i++) {
		if (m_nMatchType == MATCHTYPE_REGEXP) {
			CAtlRegExp<CAtlRECharTraits> rex;
			if (rex.Parse(m_arrSpec[i], FALSE) == REPARSE_ERROR_OK) {
				CAtlREMatchContext<CAtlRECharTraits> rc;
				bMatch = rex.Match(pszTarget, &rc);
			}
		}
		else
			bMatch = (*pfn)(str, m_arrSpec[i]);

		if (bMatch) return TRUE;
	}
	return FALSE;
}


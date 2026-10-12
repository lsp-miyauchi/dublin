#include "stdafx.h"
#include "fopstd.h"
#include "match.h"

extern TCHAR gszAst[];

#if 0

static int __fastcall recmatch(LPTSTR p, LPCTSTR s)
{
	TCHAR c;			// pattern char or start of range in [-] loop

	// Get first character, the pattern for new recmatch calls follows
	c = *p++;

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
        return 2;			// 2 means give up--match will return false
	}

	// just a character--compare it
    return (ToLower(c) == ToLower(*s)) ? recmatch(p, s + 1) : 0;
}

static int __fastcall match(LPCTSTR string, LPCTSTR pattern)
{
	if (!StrCmp(pattern, gszAst))
		return 1;

	TCHAR dospattern[MAX_PATH];
	int j = lstrlen(pattern);

	lstrcpy(dospattern, pattern);
	if (!StrCmp(dospattern + j - 3, gszAst))
		dospattern[j - 2] = '\0';                    /* nuke the ".*" */
	else if (!StrCmp(dospattern + j - 2, _T("*."))) {
		LPTSTR p = StrChr(string, '.');
		if (p)
			return 0;
		dospattern[j - 1] = '\0';
	}
	j = recmatch(dospattern, string);
	return j;
}

#define WILDCHAR   '?'
#define BEG_RANGE  '['
#define END_RANGE  ']'

static int __fastcall recmatch_ex(LPTSTR p, LPCTSTR s)
{
	TCHAR c;		// pattern char or start of range in [-] loop

	// Get first character, the pattern for new recmatch calls follows
	c = *p++;

	// If that was the end of the pattern, match if string empty too
    if (c == 0)
		return (*s == 0);

	// '?' (or '%') matches any character (but not an empty string)
    if (c == '?')
		return *s ? recmatch_ex(p, s + 1) : 0;

	// '*' matches any number of characters, including zero
    if (c == '*') {
		if (*p == 0)
            return 1;

		for (; *s; s++)
            if ((c = recmatch_ex(p, s)) != 0)
                return (int)c;
        return 2;				// means give up--match will return false
    }

	// Parse and process the list of characters and ranges in brackets
    if (c == BEG_RANGE) {
        int e;				// flag true if next char to be taken literally
		LPTSTR q;			// pointer to end of [-] group
        int r;				// flag true to match anything but the range

		if (*s == 0)                   // need a character to match
            return 0;

		r = (*p == '!' || *p == '^');		// see if reverse
		if (r)
			p++;
		for (q = p, e = 0; *q; q++) { // find closing bracket
            if (e)
                e = 0;
			else if (*q == '\\')      // GRR:  change to ^ for MS-DOS, OS/2?
                e = 1;
			else if (*q == END_RANGE)
                break;
        }
		if (*q != END_RANGE)         // nothing matches if bad syntax
            return 0;

		// go through the list
		for (c = 0, e = (*p == '-'); p < q; p++) {
			if (e == 0 && *p == '\\') 		// set escape flag if '\'
                e = 1;
			else if (e == 0 && *p == '-') 	// set start of range if -
				c = *(p - 1);
            else {
				TCHAR cc = ToLower(*s);
                if (*(p + 1) != '-') {
					// compare range
					for (c = c ? c : *p; c <= *p; c++) {
                        if (ToLower(c) == cc)
							return r ? 0 : recmatch_ex(q + 1, s + 1);
					}
				}
                c = e = 0;   // clear range, escape flags
            }
        }
		// bracket match failed
        return r ? recmatch_ex(q + 1, s + 1) : 0;
    }

    // if escape ('\'), just compare next character
    if (c == '\\') {
		c = *p++;
		if (c == 0)     // if '\' at end, then syntax error
			return 0;
		if (*s == '\\')
			s++;
	}

    // just a character--compare it
	if (ToLower(c) == ToLower(*s))
		return recmatch_ex(p, s + 1);
	return 0;
}

static int __fastcall match_ex(LPCTSTR string, LPCTSTR pattern)
{
	TCHAR dospattern[MAX_PATH];
	int j = lstrlen(pattern);

	lstrcpy(dospattern, pattern);
	if (!StrCmp(dospattern + j - 3, _T("*.*")))
		dospattern[j - 2] = '\0';                    /* nuke the ".*" */
	else if (!StrCmp(dospattern + j - 2, _T("*."))) {
		LPTSTR p = StrChr(string, '.');
		if (p)
			return 0;
		dospattern[j - 1] = '\0';
	}
	j = recmatch_ex(dospattern, string);
	return j;
}

#endif

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

CMatchSpec::CMatchSpec(LPCTSTR pszPattern, BOOL bUseRegExp, BOOL bCaseSense)
{
	m_bUseRegExp = bUseRegExp;
	m_bCaseSense = bCaseSense;
	if (pszPattern && *pszPattern) {
		CString str = pszPattern;
		if (!m_bCaseSense)
			str.MakeLower();
		SeparateString(str, _T(';'), m_arrSpec);
		if (m_bUseRegExp) {
			for (int i = 0; i < m_arrSpec.GetSize(); i++) {
				HANDLE h = lccRexInit(m_arrSpec[i]);
				m_arrRex.Add(h);
			}
		}
	}
	else {
		m_bUseRegExp = FALSE;
		m_arrSpec.Add(gszAst);
	}
}

CMatchSpec::~CMatchSpec()
{
	int n = m_arrRex.GetSize();
	for (int i = 0; i < n; i++) {
		HANDLE h = m_arrRex[i];
		if (h)
			lccRexExit(h);
	}
}

BOOL CMatchSpec::Match(LPCTSTR pszTarget)
{
	BOOL bMatch;
	int cnt = m_arrSpec.GetSize();

	BOOL (WINAPI* pfn)(LPCTSTR, LPCTSTR);

	pfn = PathMatchSpecNoCase;
	CString str = pszTarget;
	if (!m_bCaseSense) {
		str.MakeLower();
		pfn = PathMatchSpec;
	}
	for (int i = 0; i < cnt; i++) {
		if (m_bUseRegExp) {
			HANDLE h = m_arrRex[i];
			if (h) {
				int start;
				BOOL ok = lccRexMatch(h, str, &start, NULL);
				bMatch = (ok && start == 0);
			}
			else
				bMatch = (*pfn)(str, m_arrSpec[i]);
		}
		else
			bMatch = (*pfn)(str, m_arrSpec[i]);

		if (bMatch) return TRUE;
	}
	return FALSE;
}


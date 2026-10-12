#include "stdafx.h"
#include "match_exp.h"

#define IS_AST(a)			((*(a)=='*')&&(*((a)+1)==0))
#define IS_ASTDOTAST(a)		((*(a)=='*')&&(*((a)+1)=='.')&&(*((a)+2)=='*')&&(*((a)+3)==0))
#define TOLOWER(ch)			(((ch)>='A'&&(ch)<='Z')?((ch)+0x20):(ch))

int StrToWStr(LPCSTR psz, CStringW& str);


////////////////////////////////////////////////////////////////////////
// matchspec WCHAR ƒo[ƒWƒ‡ƒ“

static int match_coreW(LPCWSTR p, LPCWSTR s)
{
	// Get first character, the pattern for new dos_match calls follows
	WCHAR ch = *p++;

	// If that was the end of the pattern, match if string empty too
	if (ch == 0) {
		return ((*s == 0) ? 1 : 0);
	}

	// '?' matches any character (but not an empty string)
	if (ch == '?') return (*s ? match_coreW(p, s + 1) : 0);

	// '*' matches any number of characters, including zero
	if (ch == '*') {
		if (*p == 0) return 1;

		for (; *s; s++) {
			int n = match_coreW(p, s);
			if (n != 0) return n;
		}
		return -1;				// -1 means give up--match will return false
    }

	// just a character--compare it
	return ((ch == *s) ? match_coreW(p, s + 1) : 0);
}

BOOL match_specW(LPCWSTR name, LPCWSTR mask)
{
	WCHAR pattern[MAX_PATH];
	wcscpy(pattern, mask);
	int j = (int)wcslen(mask);
	if (!wcscmp(pattern + j - 3, L"*.*"))
		pattern[j - 2] = 0;								// nuke the ".*"
	else if (!wcscmp(pattern + j - 2, L"*.")) {
		WCHAR* p = wcschr((WCHAR*)name, '.');
		if (p) return FALSE;

		pattern[j - 1] = 0;
	}
	return (match_coreW(pattern, name) == 1);
}

////////////////////////////////////////////////////////////////////////
// class CMatchSpec

CMatchSpec::CMatchSpec(LPCWSTR pszPattern, BOOL bUseRegExp, BOOL bCaseSense, WCHAR chDelim)
{
	Set(pszPattern, bUseRegExp, bCaseSense, chDelim);
}

CMatchSpec::CMatchSpec(LPCSTR pszPattern, BOOL bUseRegExp, BOOL bCaseSense, char chDelim)
{
	CStringW wstr(pszPattern);
	Set(wstr, bUseRegExp, bCaseSense, (WCHAR)chDelim);
}

void CMatchSpec::Set(LPCWSTR pszPattern, BOOL bUseRegExp, BOOL bCaseSense, WCHAR chDelim)
{
	Clear();

	m_bMatchAll = FALSE;
	m_bUseRegExp = bUseRegExp;
	m_bCaseSense = bCaseSense;
	m_chDelim = chDelim;
	if (pszPattern && *pszPattern) {
		CStringW str = pszPattern;
		if (!m_bCaseSense)
			str.MakeLower();

		int cnt = m_arrSpec.set(str, chDelim);
		if (m_bUseRegExp) {
			for (int i = 0; i < cnt; i++) {
				CRexMatch* prm = new CRexMatch(m_arrSpec[i]);
				m_arrRex.Add(prm);
			}
		}
		else if (cnt == 1) {
			LPCWSTR spec = m_arrSpec[0];
			if (IS_AST(spec) || IS_ASTDOTAST(spec))
				m_bMatchAll = TRUE;
		}
	}
	else {
		m_bUseRegExp = FALSE;
		m_bMatchAll = TRUE;
	}
}

void CMatchSpec::Clear()
{
	int n = m_arrRex.GetSize();
	for (int i = 0; i < n; i++) {
		CRexMatch* prm = m_arrRex[i];
		if (prm)
			delete prm;
	}
	m_arrRex.RemoveAll();
	m_arrSpec.clear();
}

BOOL CMatchSpec::Match(LPCWSTR pszTarget)
{
	if (m_bMatchAll) return TRUE;

	BOOL bMatch = FALSE;

	CStringW name = pszTarget;
	if (!m_bCaseSense)
		name.MakeLower();

	int cnt = m_arrSpec.GetSize();
	for (int i = 0; i < cnt; i++) {
		CStringW mask = m_arrSpec[i];
		if (!m_bCaseSense)
			mask.MakeLower();

		if (m_bUseRegExp) {
			CRexMatch* prm = m_arrRex[i];
			if (prm) {
				int start;
				BOOL ok = prm->Match(name, &start, NULL);
				bMatch = (ok && start == 0);
			}
			else
				bMatch = match_specW(name, mask);
		}
		else
			bMatch = match_specW(name, mask);

		if (bMatch)
			break;
	}
	return bMatch;
}

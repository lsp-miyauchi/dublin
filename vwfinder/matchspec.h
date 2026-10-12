////////////////////////////////////////////////////////////////////////
// matchspec.h

#pragma once
#ifndef __MATCHSPEC_H__
#define __MATCHSPEC_H__

///////////////////////////////////////////////////////////////////////
//

__inline int dos_match(LPCTSTR p, LPCTSTR s)
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

class CMatchSpec
{
private:
	CSimpleArray<HANDLE> m_arrRex;
	CSimpleArray<CString> m_arrSpec;
	BOOL m_bUseRegExp;
	BOOL m_bCaseSense;
public:
	CMatchSpec(LPCTSTR pszPattern, BOOL bUseRegExp=FALSE, BOOL bCaseSense=FALSE)
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
	~CMatchSpec()
	{
		int n = m_arrRex.GetSize();
		for (int i = 0; i < n; i++) {
			HANDLE h = m_arrRex[i];
			if (h)
				lccRexExit(h);
		}
	}
	BOOL Match(LPCTSTR pszTarget)
	{
		BOOL bMatch;
		int cnt = m_arrSpec.GetSize();

		BOOL (WINAPI* pfn)(LPCTSTR, LPCTSTR);

		pfn = MatchSpecNoCase;
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
					bMatch = pfn(str, m_arrSpec[i]);
			}
			else
				bMatch = pfn(str, m_arrSpec[i]);

			if (bMatch) return TRUE;
		}
		return FALSE;
	}

	static BOOL WINAPI MatchSpecNoCase(LPCTSTR pszFile, LPCTSTR pszSpec)
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
};




#endif		// __MATCHSPEC_H__

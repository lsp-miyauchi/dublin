#include "stdafx.h"

TCHAR szAst[] = _T("*.*");

#define IsUpper(c) (c>=0xC0 ? c<=0xDE && c!=0xD7 : c>=0x41 && c<=0x5A)
#define ToLower(c) (IsUpper((BYTE) c) ? (unsigned) c | 0x20 : (unsigned) c)

static int __fastcall recmatch(TCHAR* p, const TCHAR* s)
{
	UINT c;			// pattern TCHAR or start of range in [-] loop

	// Get first TCHARacter, the pattern for new recmatch calls follows
	c = *p++;

	// If that was the end of the pattern, match if string empty too
	if (c == 0)
		return *s == 0;

	// '?' matches any TCHARacter (but not an empty string)
	if (c == '?')
		return *s ? recmatch(p, s + 1) : 0;

	// '*' matches any number of TCHARacters, including zero
	if (c == '*')
	{
		if (*p == 0)
			return 1;
		for (; *s; s++)
			if ((c = recmatch(p, s)) != 0)
				return (int)c;
        return 2;			// 2 means give up--match will return false
	}

	// if escape ('\'), just compare next TCHARacter
	if (c == '\\')
	{
		c = *p++;
		if (c == 0)					// if \ at end, then syntax error
			return 0;
	}

	// just a TCHARacter--compare it
    return (ToLower(c) == ToLower(*s)) ? recmatch(p, s + 1) : 0;
}

int __fastcall match(LPCTSTR string, LPCTSTR pattern)
{
	if (!StrCmp(pattern, szAst))
		return 1;

	TCHAR dospattern[MAX_PATH];
	int j = lstrlen(pattern);

	lstrcpy(dospattern, pattern);
	if (!StrCmp(dospattern + j - 3, szAst))
		dospattern[j - 2] = '\0';                    /* nuke the ".*" */
	else if (!StrCmp(dospattern + j - 2, _T("*.")))
	{
		TCHAR *p = StrChr(string, '.');
		if (p)
			return 0;
		dospattern[j - 1] = '\0';
	}
	j = recmatch(dospattern, string);
	return j;
}

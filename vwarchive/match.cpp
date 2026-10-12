#include "stdafx.h"

extern TCHAR gszAst[];

int match(LPCTSTR string, LPCTSTR pattern)
{
	if (!StrCmp(pattern, gszAst))
		return 1;
	return PathMatchSpec(string, pattern) ? 1 : 0;
}

_inline TCHAR curchar(LPCTSTR pstr)
	{ return *pstr; }
_inline LPCTSTR incmbs(LPCTSTR pstr)
	{ return ++pstr; }


static int recmatch(LPCTSTR p, LPCTSTR s)
{
	// Get first character, the pattern for new recmatch calls follows
	UINT c = curchar(p);
	p = incmbs(p);

	// If that was the end of the pattern, match if string empty too
	if (c == 0)
		return *s == 0;

	// '?' matches any character (but not an empty string)
	if (c == '?')
		return *s ? recmatch(p, incmbs(s)) : 0;

	// '*' matches any number of characters, including zero
	if (c == '*')
	{
		if (*p == 0)
            return 1;
		for (; curchar(s); s = incmbs(s))
			if ((c = recmatch(p, s)) != 0)
				return (int)c;
		return 2;				// 2 means give up--match will return false
    }

	// Parse and process the list of characters and ranges in brackets
	if (c == '[')
	{
		int e;					// flag true if next char to be taken literally
		LPCTSTR q;				// pointer to end of [-] group
		int r;					// flag true to match anything but the range

		if (*s == 0)			// need a character to match
            return 0;

		TCHAR ch = curchar(p);
		p += (r = (ch == '!' || ch == '^'));		// see if reverse
		for (q = p, e = 0; curchar(q); q = incmbs(q))		// find closing bracket
        {
			ch = curchar(q);
			if (e)
				e = 0;
			else if (ch == '\\')		// GRR:  change to ^ for MS-DOS, OS/2?
				e = 1;
			else if (ch == ']')
				break;
		}
		if (curchar(q) != ']')			// nothing matches if bad syntax
			return 0;

		// go through the list
		for (c = 0, e = (curchar(p) == '-'); p < q; p = incmbs(p))
		{
			ch = curchar(p);
			if (e == 0 && ch == '\\')		// set escape flag if '\'
				e = 1;
			else if (e == 0 && ch == '-')	// set start of range if '-'
				c = *(p-1);
			else
			{
				UINT cc = curchar(s);
				if (curchar(incmbs(p)) != '-')
				{
					for (c = c ? c : curchar(p); c <= curchar(p); c++)		// compare range
					{
						if (c == cc)			// typecast for MSC bug
							return r ? 0 : recmatch(incmbs(q), incmbs(s));
					}
				}
				c = e = 0;		// clear range, escape flags
			}
		}
		return r ? recmatch(incmbs(q), incmbs(s)) : 0;	// bracket match failed
	}

	// if escape ('\'), just compare next character
    if (c == '\\')
	{
		c = curchar(p);
		p = incmbs(p);
		if (c == 0)     // if '\' at end, then syntax error
			return 0;
		if (curchar(s) == '\\')
			s = incmbs(s);
	}

	// just a character--compare it
	return c == curchar(s) ? recmatch(p, incmbs(s)) : 0;
}

int match_ex(LPCTSTR string, LPCTSTR pattern)
{
	TCHAR dospattern[MAX_PATH];
	lstrcpy(dospattern, pattern);
	int j = lstrlen(pattern);
	if (!StrCmp(dospattern + j - 3, gszAst))
		dospattern[j - 2] = '\0';                    /* nuke the ".*" */
	else if (!StrCmp(dospattern + j - 2, _T("*.")))
	{
		TCHAR* p = StrChr(string, '.');
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

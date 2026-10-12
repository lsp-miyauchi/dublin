////////////////////////////////////////////////////////////////////////////////
// stringseparator.h

#pragma once
#ifndef __STRINGSEPARATOR_H__
#define __STRINGSEPARATOR_H__


class CStringSeparator : public CSimpleArray<CString>
{
public:
	CStringSeparator()
	{
	}
	~CStringSeparator()
	{
	}
	LPTSTR SkipTabSpace(LPTSTR p)
	{
		if (p) {
			while (*p == _T(' ') || *p == _T('\t'))
				p++;
		}
		return p;
	}
	int Separate(LPCTSTR pszText, int delim)
	{
		RemoveAll();
		LPTSTR baseptr = (LPTSTR)malloc(sizeof(TCHAR) * (lstrlen(pszText) + 1));
		lstrcpy(baseptr, pszText);

		LPTSTR p = baseptr;
		LPTSTR q;
		while (p && *p) {
			if (*p == '"') {
				q = StrChr(++p, '"');
				if (q)
					*q++ = 0;
				else
					break;
			}
			else {
				q = StrChr(p + 1, delim);
				if (q)
					*q++ = 0;
			}
			Add(p);
			p = SkipTabSpace(q);
		}
		free(baseptr);
		return GetSize();
	}
};

#endif		// __STRINGSEPARATOR_H__

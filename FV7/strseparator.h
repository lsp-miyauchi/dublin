//////////////////////////////////////////////////////////////////////
// strseprator

#pragma once
#ifndef __STRSEPARATOR_H__
#define __STRSEPARATOR_H__

class strseparator
{
public:
	CSimpleArray<CString> m_arr;

	strseparator() { }
	~strseparator() { clear(); }

	__inline void clear() { m_arr.RemoveAll(); }

	__inline LPTSTR skip(LPTSTR p)
	{
		if (p) {
			while (*p == ' ' || *p == '\t') p++;
		}
		return p;
	}

	__inline int getcount() { return (int)m_arr.GetSize(); }

	int set(LPCTSTR psz, TCHAR delim)
	{
		clear();
		if (psz && *psz) {
			LPTSTR base = StrDup(psz);
			if (base) {
				LPTSTR p = skip(base);
				LPTSTR q;
				while (p && *p) {
					if (*p == '"') {
						q = StrChr(++p, '"');
						if (q)
							*q++ = '\0';
						else
							break;
					}
					else {
						q = StrChr(p + 1, delim);
						if (q)
							*q++ = '\0';
					}
					m_arr.Add(p);
					p = skip(q);
				}
				LocalFree(base);
			}
		}
		return getcount();
	}

	LPCTSTR getat(int index)
	{
		if (index < 0 || index >= getcount()) return NULL;
		return m_arr[index];
	}
};


#endif			// __STRSEPARATOR_H__

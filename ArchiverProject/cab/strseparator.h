//////////////////////////////////////////////////////////////////////
// strseprator

#pragma once
#ifndef __STRSEPARATOR_H__
#define __STRSEPARATOR_H__

//////////////////////////////////////////////////////////////////////
//

class strseparatorA : public CSimpleArray<CStringA>
{
public:
	strseparatorA() { }
	~strseparatorA() { clear(); }

	inline void clear() { RemoveAll(); }

	inline LPSTR skip(LPSTR p)
	{
		if (p) {
			while (*p == ' ' || *p == '\t')
				p = CharNextA(p);
		}
		return p;
	}

	int set(LPCSTR psz, char delim)
	{
		clear();
		if (psz && *psz) {
			char* base = new char[strlen(psz) + 1];
			if (base) {
				LPSTR p = strcpy(base, skip((LPSTR)psz));
				LPSTR q;
				while (p && *p) {
					if (*p == '"') {
						q = (char*)_mbschr((BYTE*)(++p), '"');
						if (q)
							*q++ = 0;
						else
							break;
					}
					else {
						q = (char*)_mbschr((BYTE*)(p + 1), delim);
						if (q)
							*q++ = 0;
					}
					Add(p);
					p = (q  ? skip(q) : NULL);
				}
				delete [] base;
			}
		}
		return GetSize();
	}
};

//////////////////////////////////////////////////////////////////////
//

class strseparatorW : public CSimpleArray<CStringW>
{
public:
	strseparatorW() { }
	~strseparatorW() { clear(); }

	inline void clear() { RemoveAll(); }

	inline LPWSTR skip(LPWSTR p)
	{
		if (p) {
			while (*p == L' ' || *p == L'\t')
				p++;
		}
		return p;
	}

	int set(LPCWSTR psz, WCHAR delim)
	{
		clear();
		if (psz && *psz) {
			WCHAR* base = new WCHAR[wcslen(psz) + 1];
			if (base) {
				LPWSTR p = wcscpy(base, skip((LPWSTR)psz));
				LPWSTR q;
				while (p && *p) {
					if (*p == L'"') {
						q = wcschr(++p, L'"');
						if (q)
							*q++ = 0;
						else
							break;
					}
					else {
						q = wcschr(p + 1, delim);
						if (q)
							*q++ = 0;
					}
					if (p && *p) Add(p);
					p = (q  ? skip(q) : NULL);
				}
				delete [] base;
			}
		}
		return GetSize();
	}
};


#if !defined(UNICODE)
	#define strseparator		strseparatorA
#else
	#define strseparator		strseparatorW
#endif // !UNICODE

#endif			// __STRSEPARATOR_H__

#pragma once

class JSArray : public CSimpleArray<CString>
{
public:
	int InsertAt(int pos, LPCTSTR p)
	{
		int rblock = GetSize() + 1;
		m_aT = (CString*)realloc(m_aT, rblock * sizeof(CString));
		if (m_aT != NULL) {
			m_nAllocSize = rblock;
			m_nSize++;
			for (int i = pos; i < rblock; i++) {
				m_aT[i + 1] = m_aT[i];
			}
			SetAtIndex(pos, p);
		}
		return GetSize();
	}
	JSArray& concat(const CSimpleArray<CString>& src)
	{
		int n = src.GetSize();
		if (n > 0) {
			int rblock = n + GetSize();
			m_aT = (CString*)realloc(m_aT, rblock * sizeof(CString));
			if (m_aT != NULL) {
				m_nAllocSize = rblock;
				for (int i = 0; i < n; i++)
					Add(src[i]);
			}
		}
		return *this;
	}
	JSArray& operator+=(const CSimpleArray<CString>& src)
	{
		return concat(src);
	}
	JSArray& operator=(const CSimpleArray<CString>& src)
	{
		*this = (const JSArray&)src;
		return *this;
	}
	CSimpleArray<CString> slice(int iStart, int delta = -1) const
	{
		CSimpleArray<CString> dst;
		int cnt = GetSize();
		if (iStart < 0) {
			iStart = cnt - 1 - iStart;
		}
		int l = cnt - 1 - iStart;
		l = (delta < 0) ? l : min(delta, l);
		for (int i = 0; i < l; i++) {
			dst.Add((*this)[iStart + i]);
		}
		return dst;
	}
	CString join(LPCTSTR p) const
	{
		CString s;
		for (int i = 0; i < GetSize(); i++) {
			if (i > 0) {
				s += p;
			}
			s += (*this)[i];
		}
		return s;
	}
	CString join(TCHAR ch) const
	{
		TCHAR s[2] = { ch, 0 };
		return join((LPCTSTR)s);
	}
	JSArray& split(LPCTSTR base, LPCTSTR delim)
	{
		RemoveAll();
		CString src = base;
		int pos = 0;
		CString token = src.Tokenize(delim, pos);
		while (token != _T("")) {
			Add(token);
			token = src.Tokenize(delim, pos);
		}
		return *this;
	}
	JSArray& split(LPCTSTR base, TCHAR ch)
	{
		TCHAR s[2] = { ch, 0 };
		return split(base, (LPCTSTR)s);
	}
	CSimpleArray<CString> filter(bool (*compproc)(LPCTSTR)) const
	{
		CSimpleArray<CString> dst;
		for (int i = 0; i < GetSize(); i++) {
			if (compproc((*this)[i]))
				dst.Add((*this)[i]);
		}
		return dst;
	}
	int push(LPCTSTR p)
	{
		Add(p);
		return GetSize();
	}
	CString pop()
	{
		CString sT;
		int cnt = GetSize();
		if (cnt > 0) {
			sT = (*this)[cnt - 1];
			RemoveAt(cnt - 1);
		}
		return sT;
	}
	int shift(LPCTSTR p)
	{
		return InsertAt(0, p);
	}
	CString unshift()
	{
		CString sT;
		int cnt = GetSize();
		if (cnt > 0) {
			sT = (*this)[0];
			RemoveAt(0);
		}
		return sT;
	}

	JSArray& __cdecl splice(int pos, ...)
	{
		if (pos < 0) {
			pos = GetSize() + pos;
		}

		TCHAR* pOutput = new TCHAR[1024];
		va_list argList;
		va_start(argList, pos);
		int del = 0;
		del = argList ? va_arg(argList, int) : GetSize() - pos;
		while (del > 0 && RemoveAt(pos)) {
			--del;
		}
		while (argList) {
			LPCTSTR p = va_arg(argList, LPCTSTR);
			InsertAt(pos, p);
		}
		va_end(argList);
		delete[] pOutput;

		return *this;
	}
	int indexOf(LPCTSTR p) const
	{
		return Find(p);
	}
	bool includes(LPCTSTR p) const
	{
		return Find(p) >= 0;
	}
	bool some(bool (*compproc)(LPCTSTR)) const
	{
		for (int i = 0; i < GetSize(); i++) {
			if (compproc((*this)[i]))
				return true;
		}
		return false;
	}
	bool every(bool (*compproc)(LPCTSTR)) const
	{
		int match = 0, cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			if (compproc((*this)[i]))
				match++;
		}
		return match == cnt;
	}
};

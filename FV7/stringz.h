/////////////////////////////////////////////////////////////////////////////////
// CStringZ

#pragma once
#ifndef __STRINGZ_H__

template <class TBase>
class CStringZT : public TBase
{
public:
	typedef CStringZT< TBase > CThisStringZ;

	CStringZT(LPCTSTR pstr = NULL) : TBase(pstr) { }
	CStringZT& operator=(const CStringZT< TBase >& src)
	{
		TBase::operator = (src);
		return (*this);
	}

	CStringZT< TBase >& operator = (LPCTSTR pstr)
	{
		TBase::operator= (pstr);
		return *this;
	}
	CStringZT< TBase >& operator += (LPCTSTR pstr)
	{
		TBase::operator+= (pstr);
		return(*this);
	}
	CStringZT< TBase >& operator += (TCHAR ch)
	{
		TBase::operator+= (ch);
		return(*this);
	}

	inline int cmp(LPCTSTR dst) { return TBase::Compare(dst); }
	inline int cmpi(LPCTSTR dst) { return TBase::CompareNoCase(dst); }
	inline int cmpn(LPCTSTR dst, int len) { return TBase::Left(len).Compare(dst); }
	inline int cmpni(LPCTSTR dst, int len) { return TBase::Left(len).CompareNoCase(dst); }

	CSimpleArray< CThisStringZ > split(LPCTSTR delim)
	{
		CSimpleArray< CThisStringZ > arr;
		int pos = 0;
		TBase token = TBase::Tokenize(delim, pos);
		while (token != _T("")) {
			arr.Add(token.GetString());
			token = TBase::Tokenize(delim, pos);
		}
		return arr;
	}

	// path�T�|�[�g
	CStringZT< TBase > FileName()
	{
		int pos = TBase::ReverseFind(_T('\\'));
		LPCTSTR p = TBase::GetString() + (pos < 0 ? 0 : pos);
		return CThisStringZ(p);
	}

	CStringZT< TBase > FileBody()
	{
		CThisStringZ z = FileName();
		int pos = z.ReverseFind(_T('.'));
		if (pos >= 0) {
			z = z.Left(pos);
		}
		return z;
	}

	CStringZT< TBase > FileExtension()
	{
		int pos = TBase::ReverseFind(_T('.'));
		int sep = TBase::ReverseFind(_T('\\'));
		LPCTSTR p = (pos < 0 || pos < sep) ? NULL : TBase::GetString() + pos;
		return CThisStringZ(p);
	}

	static CStringZT< TBase > FileName(LPCTSTR p) { return CThisStringZ(p).FileName(); }
	static CStringZT< TBase > FileBody(LPCTSTR p) { return CThisStringZ(p).FileBody(); }
	static CStringZT< TBase > FileExtension(LPCTSTR p) { return CThisStringZ(p).FileExtension(); }

	void AddBackslash()
	{
		int len = TBase::GetLength();
		if (TBase::GetAt(len -1) != _T('\\')) {
			CThisStringZ::operator+=( _T('\\') );
		}
	}

	CStringZT< TBase > Unquote()
	{
		return CThisStringZ(TBase::Trim(_T("\" ")));
	}

};
typedef CStringZT<CString> CStringZ;

#endif		// __STRINGZ_H__

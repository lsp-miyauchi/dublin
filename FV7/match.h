////////////////////////////////////////////////////////////////////////
// match.h

#pragma once
#ifndef __MATCH_H__
#define __MATCH_H__

enum {
	MATCHTYPE_PARTIAL = 0,
	MATCHTYPE_PREFIX,
	MATCHTYPE_SUFIX,
	MATCHTYPE_REGEXP,
};

class CMatchSpec
{
private:
	CSimpleArray<CString> m_arrSpec;
	int m_nMatchType;
	BOOL m_bCaseSense;
	BOOL m_bMatchAll;
public:
	CMatchSpec(LPCTSTR pszPattern, int matchtype, BOOL bCaseSense = FALSE, TCHAR chDelim = _T(';'));
	~CMatchSpec() { }
	BOOL Match(LPCTSTR pszTarget);
};


#endif			// __MATCH_H__

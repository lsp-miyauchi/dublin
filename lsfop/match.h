////////////////////////////////////////////////////////////////////////
// match.h

#pragma once
#ifndef __MATCH_H__
#define __MATCH_H__

class CMatchSpec
{
private:
	CSimpleArray<HANDLE> m_arrRex;
	CSimpleArray<CString> m_arrSpec;
	BOOL m_bUseRegExp;
	BOOL m_bCaseSense;
public:
	CMatchSpec(LPCTSTR pszPattern, BOOL bUseRegExp=FALSE, BOOL bCaseSense=FALSE);
	~CMatchSpec();
	BOOL Match(LPCTSTR pszTarget);
};


#endif			// __MATCH_H__

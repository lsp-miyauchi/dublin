////////////////////////////////////////////////////////////////////////
// match.h

#pragma once
#ifndef __MATCH_EXP_H__
#define __MATCH_EXP_H__

//@ #include <atlrx.h>			// VS2008では未サポート？
#include "..\..\lscore\atl_special_rx.h"
#include "strseparator.h"

typedef CAtlREMatchContext<CAtlRECharTraitsW>	CREMatchContextW;
typedef CAtlRegExp<CAtlRECharTraitsW>			CRegExpW;

BOOL match_specW(LPCWSTR pszPath, LPCWSTR pszMask);

////////////////////////////////////////////////////////////////////////
//

class CRexMatch
{
public:
	CRegExpW* m_pJre;

	CRexMatch(void)
	{
		m_pJre = NULL;
	}
	CRexMatch(LPCWSTR pszExp)
	{
		m_pJre = NULL;
		Init(pszExp);
	}
	~CRexMatch()
	{
		Clear();
	}
	void Clear()
	{
		if (m_pJre) {
			delete m_pJre;
			m_pJre = NULL;
		}
	}
	BOOL Init(LPCWSTR pszExp)
	{
		Clear();
		if (pszExp && *pszExp) {
			m_pJre = new CRegExpW;
			if (m_pJre) {
				REParseError status = m_pJre->Parse(pszExp);
				if (REPARSE_ERROR_OK != status) {
					delete m_pJre;
					m_pJre = NULL;
				}
			}
		}
		return (m_pJre != NULL);
	}
	BOOL Match(LPCWSTR pszTarget, int* posStart, int* posEnd)
	{
		if (!m_pJre) return FALSE;

		CREMatchContextW mc;
		if (!m_pJre->Match(pszTarget, &mc))
			return FALSE;

#ifdef _WIN64
		if (posStart)	*posStart = (int)(mc.m_Match.szStart - pszTarget);
		if (posEnd)		*posEnd = (int)(mc.m_Match.szEnd - pszTarget);
#else
		if (posStart)	*posStart = PtrToInt(mc.m_Match.szStart - pszTarget);
		if (posEnd)		*posEnd = PtrToInt(mc.m_Match.szEnd - pszTarget);
#endif
		return TRUE;
	}
};

////////////////////////////////////////////////////////////////////////
//

class CMatchSpec
{
private:
	CSimpleArray<CRexMatch*> m_arrRex;
	strseparatorW m_arrSpec;
	BOOL m_bUseRegExp;
	BOOL m_bCaseSense;
	BOOL m_bMatchAll;
	WCHAR m_chDelim;

public:
	CMatchSpec(LPCWSTR pszPattern, BOOL bUseRegExp = FALSE, BOOL bCaseSense = FALSE, WCHAR chDelim = L';');
	CMatchSpec(LPCSTR pszPattern, BOOL bUseRegExp = FALSE, BOOL bCaseSense = FALSE, char chDelim = ';');
	~CMatchSpec()
	{
		Clear();
	}

	void Set(LPCWSTR pszPattern, BOOL bUseRegExp = FALSE, BOOL bCaseSense = FALSE, WCHAR chDelim = L';');
	void Clear();
	BOOL Match(LPCWSTR pszTarget);
};


#endif			// __MATCH_EXP_H__

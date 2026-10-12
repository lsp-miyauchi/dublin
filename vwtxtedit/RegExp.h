///////////////////////////////////////////////////////////
// regexp.h

#pragma once
#ifndef __REGEXP_H__
#define __REGEXP_H__

#define FIS_CASE		0x00000001
#define FIS_NEXT		0x00000002
#define FIS_JRE			0x00000004
#define FIS_FROMTOP		0x00000010
#define FIS_PICKUP		0x00000020
#define FIS_SUBDIR		0x10000000

#define REP_REPLACE		0
#define REP_REPLACEALL	1
#define REP_SKIP		2
#define REP_EXIT		3

///////////////////////////////////////////////////////////
//

class CFind
{
public:
	CString m_strText;
	CString m_strReplace;
	int m_len;
	DWORD m_stat;
	CAtlRegExp<> m_jre;

public:
	CFind()
	{
		m_stat = FIS_CASE | FIS_NEXT;
		m_len = 0;
	}
	~CFind()
	{
	}
	void SetText(const TCHAR* pattern)
	{
		m_len = lstrlen(pattern);
		m_strText = pattern;
	}
	void SetReplaceText(const TCHAR* pattern)
	{
		m_strReplace.Empty();
		m_strReplace = pattern;
	}
	BOOL Find(TCHAR* data, TCHAR* lastpos, int& posFrom, int& posTo)
	{
		int len = lstrlen(data);
		if (len < 1)
			return FALSE;

		if (m_stat & FIS_JRE) {
			CAtlREMatchContext<> mc;
			if (!m_jre.Match(data, &mc)) return FALSE;

			posFrom = (int)(mc.m_Match.szStart - data);
			posTo = (int)(mc.m_Match.szEnd - data);
			return TRUE;
		}

		if (m_len <= len) {
			LPTSTR (WINAPI* pfn)(LPCTSTR, LPCTSTR);
			pfn = (m_stat & FIS_CASE) ? StrStr : StrStrI;

			TCHAR* p = (*pfn)(data, m_strText); 
			if (p && p < lastpos) {
				posFrom = (int)(p - data);
				posTo = (int)(posFrom + m_len);
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL RFind(TCHAR* data, TCHAR* lastpos, int& posFrom, int& posTo)
	{
		int len = lstrlen(data);
		if (len < 1)
			return FALSE;

		if (m_stat & FIS_JRE) {
			CAtlREMatchContext<> mc;

			TCHAR* p = data;
			BOOL bOK = FALSE;
			int from, to;
			const CAtlREMatchContext<>::RECHAR* pszEnd;
			while (p < lastpos && m_jre.Match(p, &mc, &pszEnd)) {
				from = (int)(mc.m_Match.szStart - data);
				to = (int)(mc.m_Match.szEnd - data);
				if (pszEnd - 1 > lastpos) break;

				bOK = TRUE;
				posFrom = from;
				posTo = to;
				p = (TCHAR*)pszEnd;
			}
			return bOK;
		}

		if (m_len <= len && m_len <= (lastpos - data)) {
			LPTSTR (WINAPI* pfn)(LPCTSTR, LPCTSTR);
			pfn = (m_stat & FIS_CASE) ? StrStr : StrStrI;

			TCHAR* p = data;
			TCHAR* q = NULL;
			while (p = (*pfn)(p, m_strText)) {
				if (p >= lastpos)
					break;
				q = p++;
			}
			if (q) {
				posFrom = (int)(q - data);
				posTo = (int)(posFrom + m_len);
				return TRUE;
			}
		}
		return FALSE;
	}
	inline int Len() { return m_len; };
};

///////////////////////////////////////////////////////////
//

class CGrep
{
public:
	BOOL m_bSubdir;
	TCHAR m_filter[MAX_PATH];
	TCHAR m_path[MAX_PATH * 2];
	CFind* m_pFinder;

public:
	CGrep()
	{
		m_filter[0] = 0;
		m_path[0] = 0;
		m_bSubdir = TRUE;
		m_pFinder = NULL;
	}
	~CGrep() {}
	inline CFind* GetFinder() { return m_pFinder; }
};



#endif		// __REGEXP_H__

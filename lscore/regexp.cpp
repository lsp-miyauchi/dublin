///////////////////////////////////////////////////////
// regexp.cpp

#include "stdafx.h"
//@ #include <atlrx.h>			// VS2008では未サポート？
#include "atl_special_rx.h"


extern "C"
HANDLE WINAPI lccRexInit(LPCTSTR pszExp)
{
	CAtlRegExp<>* pJre = new CAtlRegExp<>;
	if (pJre) {
		REParseError status = pJre->Parse(pszExp);
		if (REPARSE_ERROR_OK != status) {
			delete pJre;
			pJre = NULL;
		}
	}
	return (HANDLE)pJre;
}

extern "C"
BOOL WINAPI lccRexMatch(HANDLE h, LPCTSTR pszTarget, int* posStart, int* posEnd)
{
	if (!h) return FALSE;

	CAtlRegExp<>* pJre = (CAtlRegExp<>*)h;
	CAtlREMatchContext<> mc;
	if (!pJre->Match(pszTarget, &mc))
		return FALSE;

#ifdef _WIN64
	if (posStart)	*posStart = (int)(mc.m_Match.szStart - pszTarget);
	if (posEnd)		*posEnd = (int)(mc.m_Match.szEnd - pszTarget);
#else
	if (posStart)	*posStart = PtrToInt(mc.m_Match.szStart - pszTarget);
	if (posEnd)		*posEnd = PtrToInt(mc.m_Match.szEnd - pszTarget);
#endif // _WIN64
	return TRUE;
}

extern "C"
void WINAPI lccRexExit(HANDLE h)
{
	if (h) {
		CAtlRegExp<>* pJre = (CAtlRegExp<>*)h;
		delete pJre;
	}
}

// ExifParser.cpp : CExifParser �̎���

#include "stdafx.h"
#include "exifparse.h"
#include "ExifParser.h"
#include "common.h"

//-----------------------------
// exifparse.cpp
//-----------------------------

BOOL exfIsExifFile(LPCTSTR fname);
BOOL exfGetExifInfo(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, EXIFINFOSTRUCT* info);
BOOL exfGetThumbsnail(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, LPTSTR tip, int cchMax);
BOOL exfGetDateTimeString(LPCTSTR fname, LPTSTR date, int cchMax);
int exfEnumInfo(LPCTSTR fname, CExifCallback* pec);

///////////////////////////////////////////////////////////////////////////
// CExifParser


STDMETHODIMP CExifParser::IsExifFile(LPCWSTR pszFileName)
{
	BOOL ok = exfIsExifFile(pszFileName);
	return (ok ? S_OK : E_FAIL);
}

STDMETHODIMP CExifParser::GetExifInfo(LPCWSTR pszFileName, IStream* pStream, int* pnOrientation, BOOL bLargeImage, BYTE* pInfo)
{
	BOOL ok = exfGetExifInfo(pszFileName, pStream, pnOrientation, bLargeImage, (EXIFINFOSTRUCT*)pInfo);
	return (ok ? S_OK : E_FAIL);
}

STDMETHODIMP CExifParser::GetThumbsnail(LPCWSTR pszFileName, IStream* pStream, int* pnOrientation, BOOL bLargeImage, LPWSTR pszTips, int cchMax)
{
	BOOL ok = exfGetThumbsnail(pszFileName, pStream, pnOrientation, bLargeImage, pszTips, cchMax);
	return (ok ? S_OK : E_FAIL);
}

STDMETHODIMP CExifParser::GetInfoTip(LPCWSTR pszFileName, LPWSTR pszTips, int cchMax)
{
	BOOL ok = exfGetThumbsnail(pszFileName, NULL, NULL, false, pszTips, cchMax);
	return (ok ? S_OK : E_FAIL);
}

STDMETHODIMP CExifParser::GetDateTimeString(LPCWSTR pszFileName, LPWSTR pszDate, int cchMax)
{
	BOOL ok = exfGetDateTimeString(pszFileName, pszDate, cchMax);
	return (ok ? S_OK : E_FAIL);
}

STDMETHODIMP CExifParser::EnumExifInfo(LPCWSTR pszFileName, UINT_PTR pfnCallback, LPARAM param)
{
	CExifCallback ec((EXIFCBPROC)pfnCallback, param);
	exfEnumInfo(pszFileName, &ec);
	return S_OK;
}

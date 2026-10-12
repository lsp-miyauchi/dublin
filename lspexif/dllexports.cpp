////////////////////////////////////////////////////////////////////////
// dllexports.cpp

#include "stdafx.h"
#include "exifparse.h"


BOOL exfIsExifFile(LPCTSTR fname);
BOOL exfGetExifInfo(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, EXIFINFOSTRUCT* info);
BOOL exfGetThumbsnail(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, LPTSTR tip, int cchMax);
BOOL exfGetDateTimeString(LPCTSTR fname, LPTSTR date, int cchMax);
int exfEnumInfo(LPCTSTR fname, CExifCallback* pec);


///////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI ExifGetThumbsnail(LPCTSTR fname, IStream* pstrm, LPTSTR tip, int cchMax)
{
	return exfGetThumbsnail(fname, pstrm, NULL, false, tip, cchMax);
}

extern "C"
BOOL WINAPI ExifGetPrimaryImageEx(LPCTSTR fname, IStream* pstrm, int* pnOrientation)
{
	return exfGetExifInfo(fname, pstrm, pnOrientation, true, NULL);
}

extern "C"
BOOL WINAPI ExifGetPrimaryImage(LPCTSTR fname, IStream* pstrm)
{
	return exfGetExifInfo(fname, pstrm, NULL, true, NULL);
}

extern "C"
BOOL WINAPI ExifGetThumbnailImage(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, LPTSTR tip, int cchMax)
{
	return exfGetThumbsnail(fname, pstrm, pnOrientation, bLargeImage, tip, cchMax);
}

extern "C"
BOOL WINAPI ExifGetInfoTip(LPCTSTR fname, LPTSTR tip, int cchMax)
{
	return exfGetThumbsnail(fname, NULL, NULL, false, tip, cchMax);
}

extern "C"
int WINAPI ExifEnumInfo(LPCTSTR fname, EXIFCBPROC proc, LPARAM param)
{
	CExifCallback ec(proc, param);
	return exfEnumInfo(fname, &ec);
}

extern "C"
BOOL WINAPI ExifIsExifFile(LPCTSTR fname)
{
	return exfIsExifFile(fname);
}

extern "C"
BOOL WINAPI ExifGetDateTimeString(LPCTSTR fname, LPTSTR date, int cchMax)
{
	return exfGetDateTimeString(fname, date, cchMax);
}

extern "C"
BOOL WINAPI ExifGetExifInfo(LPCTSTR fname, IStream* pstrm, EXIFINFOSTRUCT_V1* info_v1_ptr)
{
	EXIFINFOSTRUCT* info_ptr = NULL;
	if (info_v1_ptr) {
		info_ptr = new EXIFINFOSTRUCT;
		memset(info_ptr, 0, sizeof(EXIFINFOSTRUCT));
		memcpy(info_ptr, info_v1_ptr, sizeof(EXIFINFOSTRUCT_V1));
	}
	BOOL bOK = exfGetExifInfo(fname, pstrm, NULL, false, info_ptr);
	if (info_v1_ptr) {
		memcpy(info_v1_ptr, info_ptr, sizeof(EXIFINFOSTRUCT_V1));
		delete info_ptr;
	}
	return bOK;
}

extern "C"
BOOL WINAPI ExifGetExifInfoEx(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, EXIFINFOSTRUCT* info)
{
	return exfGetExifInfo(fname, pstrm, pnOrientation, bLargeImage, info);
}

extern "C"
void WINAPI ExifDrawDigiText(HDC hdc, LPCTSTR pszText, int cchMax, RECT* prc, UINT flags)
{
	// no action
}

//////////////////////////////////////////////////////////////////////
// 共通ソース

#include <shlwapi.h>
#include "imagetypes.h"

extern "C"
int WINAPI imgSpecifyImageType(LPCTSTR lpName)
{
	if (!lpName)
		return 0;

	LPTSTR p = StrRChr(lpName, NULL, '.');
	if (!p)
		return IMG_NOTSUPPORTED;

	p++;
	if (!lstrcmpi(p, _T("bmp")) || !lstrcmpi(p, _T("dib")) || !lstrcmpi(p, _T("rle")))
		return IMG_BMP;
	else if (!lstrcmpi(p, _T("gif")))		return IMG_GIF;
	else if (!lstrcmpi(p, _T("mag")))		return IMG_MAG;
	else if (!lstrcmpi(p, _T("jpg")) ||
			 !lstrcmpi(p, _T("jpeg")) ||
			 !lstrcmpi(p, _T("jpe")) ||
			 !lstrcmpi(p, _T("jif")))		return IMG_JPEG;
	else if (!lstrcmpi(p, _T("png")))		return IMG_PNG;
	else if (!lstrcmpi(p, _T("psd")))		return IMG_PSD;
	else if (!lstrcmpi(p, _T("webp")))		return IMG_WEBP;			// WebP
	else if (!lstrcmpi(p, _T("avif")))		return IMG_AVIF;			// Avif
	else if (!lstrcmpi(p, _T("tif")) ||
			 !lstrcmpi(p, _T("tiff")))		return IMG_TIFF;
	else if (!lstrcmpi(p, _T("tga")))		return IMG_TGA;
	else if (!lstrcmpi(p, _T("pcx")))		return IMG_PCX;
	else if (!lstrcmpi(p, _T("pcd")))		return IMG_PCD;
	else if (!lstrcmpi(p, _T("ras")))		return IMG_RASTER;
	else if (!lstrcmpi(p, _T("xjg")))		return IMG_PNG;
	else if (!lstrcmpi(p, _T("cam")))		return IMG_CAM;
	else if (!lstrcmpi(p, _T("sgi")) ||
			 !lstrcmpi(p, _T("rgb")))		return IMG_SGI;
	else if (!lstrcmpi(p, _T("wmf")))		return IMG_WMF;
	else if (!lstrcmpi(p, _T("emf")))		return IMG_EMF;
	else if (!lstrcmpi(p, _T("pnm")) ||
			 !lstrcmpi(p, _T("pgm")) ||
			 !lstrcmpi(p, _T("ppm")))		return IMG_PNM;
	else if (!lstrcmpi(p, _T("ico")))		return IMG_ICO;
	else if (!lstrcmpi(p, _T("txr")))		return IMG_TXR;
	else if (!lstrcmpi(p, _T("xpm")))		return IMG_XPM;
	else if (!lstrcmpi(p, _T("xbm")))		return IMG_XBM;
	else if (!lstrcmpi(p, _T("fpx")))		return IMG_FPX;
	else if (!lstrcmpi(p, _T("jp2")))		return IMG_JP2;
	else if (!lstrcmpi(p, _T("mki")))		return IMG_MAKI;
	else if (!lstrcmpi(p, _T("crw")) ||
			 !lstrcmpi(p, _T("cr2")))		return IMG_CRW;
	else if (!lstrcmpi(p, _T("nef")))		return IMG_NEF;
	else if (!lstrcmpi(p, _T("raf")))		return IMG_RAF;
	else if (!lstrcmpi(p, _T("mrw")))		return IMG_MRW;
	else if (!lstrcmpi(p, _T("x3f")))		return IMG_X3F;
	else if (!lstrcmpi(p, _T("pef")))		return IMG_PEF;
	else if (!lstrcmpi(p, _T("dng")))		return IMG_DNG;
	else if (!lstrcmpi(p, _T("orf")))		return IMG_ORF;
	else if (!lstrcmpi(p, _T("arw")))		return IMG_ARW;
	else if (!lstrcmpi(p, _T("raw")) ||
			 !lstrcmpi(p, _T("rw2")))		return IMG_RAW;				// panasonic / leica
	else if (!lstrcmpi(p, _T("erf")))		return IMG_ERF;				// epson
	else if (!lstrcmpi(p, _T("mef")))		return IMG_MEF;				// mamiya
	else if (!lstrcmpi(p, _T("srf")) ||
			 !lstrcmpi(p, _T("sr2")))		return IMG_SRF;				// sony
	else if (!lstrcmpi(p, _T("3fr")))		return IMG_3FR;				// hasselblad

	return IMG_NOTSUPPORTED;
}

extern "C"
int WINAPI imgTypeIsExif(int type)
{
	return (
		type == IMG_JPEG ||
		type == IMG_TIFF ||
		type == IMG_CRW ||
		type == IMG_NEF ||
		type == IMG_MRW ||
		type == IMG_RAF ||
		type == IMG_PEF ||
		type == IMG_DNG ||
		type == IMG_ORF ||
		type == IMG_ARW ||
		type == IMG_SRF ||											// sony
		type == IMG_MEF ||											// mamiya
		type == IMG_ERF ||											// epson
		type == IMG_RAW ||											// panasonic / leica
		type == IMG_X3F ||											// sigma
		type == IMG_3FR												// hasselblad
		) ? TRUE : FALSE;
}

//////////////////////////////////////////////////////////////////////
// loader.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"


extern "C"
HDIB WINAPI imgLoadImage(LPCTSTR pszName)
{
	int nType = imgCheckImageType(pszName);
	if (nType == IMG_NOTSUPPORTED)
		return NULL;

	HDIB hdib = NULL;
	switch(nType) {
		case IMG_BMP:	hdib = imgDibLoad(pszName); break;
		case IMG_JPEG:	hdib = imgJpgLoad(pszName, 0, 1); break;
		case IMG_GIF:	hdib = imgGifLoad(pszName); break;
		case IMG_TIFF:	hdib = imgTifLoad(pszName); break;
		case IMG_TGA:	hdib = imgTgaLoad(pszName); break;
		case IMG_PCX:	hdib = imgPcxLoad(pszName); break;
		case IMG_MAG:	hdib = imgMagLoad(pszName); break;
		case IMG_PNG:	hdib = imgPngLoad(pszName); break;
		case IMG_PSD:	hdib = imgPsdLoad(pszName); break;
		case IMG_WEBP:	hdib = imgWebpLoad(pszName); break;
		case IMG_AVIF:	hdib = imgAvifLoad(pszName); break;
		case IMG_MAKI:	hdib = imgMkiLoad(pszName); break;
		case IMG_RASTER:hdib = imgRasLoad(pszName); break;
	//	case IMG_XJG:	hdib = imgXjgLoad(pszName); break;
		case IMG_CAM:	hdib = imgCamLoad(pszName); break;
		case IMG_SGI:	hdib = imgSgiLoad(pszName); break;
		case IMG_WMF:
		case IMG_EMF:	hdib = imgWmfLoad(pszName); break;
		case IMG_PNM:	hdib = imgPnmLoad(pszName); break;
		case IMG_ICO:	hdib = imgIcoLoad(pszName); break;
		case IMG_TXR:	hdib = imgTxrLoad(pszName); break;
		case IMG_XPM:	hdib = imgXpmLoad(pszName); break;
		case IMG_XBM:	hdib = imgXbmLoad(pszName); break;
		case IMG_FPX:	hdib = imgFpxLoad(pszName); break;
		case IMG_JP2:	hdib = imgJp2Load(pszName); break;
		case IMG_PCD:	hdib = imgPcdLoad(pszName); break;
		case IMG_CRW:
		case IMG_NEF:
		case IMG_MRW:
		case IMG_X3F:
		case IMG_PEF:
		case IMG_DNG:
		case IMG_ORF:
		case IMG_ARW:
		case IMG_RAF:	hdib = imgCrwLoad(pszName); break;
		default:
			if (isVer7()) {
				switch (nType) {
					case IMG_CRW:
					case IMG_SRF:
					case IMG_MEF:
					case IMG_ERF:
					case IMG_RAW:
					case IMG_3FR:	hdib = imgCrwLoad(pszName); break;
				}
			}
			break;
	}
	return hdib;
}

extern "C"
HDIB imgLoadThumbnail(LPCTSTR pszName, int width, int height, int* pOrgWidth, int* pOrgHeight, int* pOrgBits)
{
	int x, y, bits;
	HDIB hdib = NULL;
	int nType = imgCheckImageType(pszName);
	if (nType != IMG_NOTSUPPORTED) {
		if (nType == IMG_JPEG) {
			imgJpgGetImageSize(pszName, &x, &y, NULL);
			hdib = imgJpgLoadScaled(pszName, width, height, 0, JC_IFAST, JD_ORDERED, 0);
			bits = 24;
		} else if (nType == IMG_WEBP) {
			imgWebpGetImageSize(pszName, &x, &y);
			hdib = imgWebpLoadScaled(pszName, width, height);
			bits = 24;
		} else if (nType == IMG_AVIF) {
			imgAvifGetImageSize(pszName, &x, &y);
			hdib = imgAvifLoadScaled(pszName, width, height);
			bits = 24;
		} else if (nType == IMG_PCD) {
			hdib = imgPcdLoadEx(pszName, 1);
			x = 192;
			y = 128;
			bits = 24;
		} else if (nType == IMG_FPX) {
			CSize csz;
			hdib = imgFpxLoadThumbnail(pszName, &csz);
			x = csz.cx;
			y = csz.cy;
			bits = 24;
		}

		if (!hdib) {
			HDIB h = imgLoadImage(pszName);
			if (h) {
				hdib = imgScaleInFrame(h, width, height);
				BITMAPINFOHEADER* pb = dib_lockheader(h);
				x = pb->biWidth;
				y = pb->biHeight;
				bits = pb->biBitCount;
				dib_unlock(h);
				dib_destroy(h);
			}
		}
	}

	if (hdib) {
		if (pOrgWidth)	*pOrgWidth = x;
		if (pOrgHeight)	*pOrgHeight = y;
		if (pOrgBits)	*pOrgBits = bits;
	}
	return hdib;
}

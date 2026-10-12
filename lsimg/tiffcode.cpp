#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

#include "..\gnu_libs\tiff\libtiff\tiffio.h"

#ifdef __cplusplus
}
#endif

#define CVT4(x)				(BYTE)(((((x)&0xff)>>4)*255)/15)
#define CVT(x)				(BYTE)(((x)*255L)/((1L<<16)-1))
#define	SCALE(x)			(((x)*((1L<<16)-1))/255)


//////////////////////////////////////////////////////////////////////
//

static void cmyk_rgb(void* pdata, BYTE* pbBgr, BOOL b16)
{
	int cyan, magenta, yellow, black;
	if (b16) {
		uint32* pwd = (uint32*)pdata;
		cyan = CVT(pwd[0]);
		magenta = CVT(pwd[1]);
		yellow = CVT(pwd[2]);
		black = CVT(pwd[3]);
	} else {
		u_char* pd = (u_char*)pdata;
		cyan = pd[0];
		magenta = pd[1];
		yellow = pd[2];
		black = pd[3];
	}
	*pbBgr++ = (yellow + black) > MAXRGB ? 0 : MAXRGB - (yellow + black);
	*pbBgr++ = (magenta + black) > MAXRGB ? 0 : MAXRGB - (magenta + black);
	*pbBgr = (cyan + black) > MAXRGB ? 0 : MAXRGB - (cyan + black);
}

static TIFFYCbCrToRGB* initializeYCrCb(TIFF* tif)
{
	/* Initialize structures */
	TIFFYCbCrToRGB* ycbcr = (TIFFYCbCrToRGB*)_TIFFmalloc(sizeof(TIFFYCbCrToRGB) + sizeof(long) 
		+ 4 * 256 * sizeof(TIFFRGBValue)
		+ 2 * 256 * sizeof(int)
		+ 3 * 256 * sizeof(int32));
	if (ycbcr) {
		float* luma, * refBlackWhite;
		TIFFGetFieldDefaulted(tif, TIFFTAG_YCBCRCOEFFICIENTS, &luma);
		TIFFGetFieldDefaulted(tif, TIFFTAG_REFERENCEBLACKWHITE, &refBlackWhite);
		if (TIFFYCbCrToRGBInit(ycbcr, luma, refBlackWhite) < 0) {
			_TIFFfree(ycbcr);
			ycbcr = NULL;
		}
	}
	return ycbcr;
}

static TIFFCIELabToRGB* initializeCIELab(TIFF* tif)
{
	/* Initialize structures */
	TIFFCIELabToRGB* cielab = (TIFFCIELabToRGB*)_TIFFmalloc(sizeof(TIFFCIELabToRGB));
	if (cielab) {
		float* whitePoint;
		TIFFGetFieldDefaulted(tif, TIFFTAG_WHITEPOINT, &whitePoint);
		float refWhite[3];
		refWhite[1] = 100.0F;
		refWhite[0] = whitePoint[0] / whitePoint[1] * refWhite[1];
		refWhite[2] = (1.0F - whitePoint[0] - whitePoint[1]) / whitePoint[1] * refWhite[1];
		TIFFDisplay display_sRGB;
		if (TIFFCIELabToRGBInit(cielab, &display_sRGB, refWhite) < 0) {
			_TIFFfree(cielab);
			cielab = NULL;
		}
	}
	return cielab;
}

static int __fastcall checkcmap(int n, uint16* r, uint16* g, uint16* b)
{
	while (n-- > 0) {
		if (*r & 0xff00 || *g & 0xff00 || *b & 0xff00)
			return 16;
		r++;
		g++;
		b++;
	}
	return 8;
}

static BYTE* tiff_decode(TIFF* tif)
{
	// 全データをデコード
	int rowsperstrip, height;
	TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

	int maxReadCount = height / rowsperstrip;
	if (height % rowsperstrip) {
		maxReadCount++;
	}

	int stripsize = TIFFStripSize(tif);
	int scanlinesize = TIFFScanlineSize(tif); // Number of byte in ine line
// dbg.printf(_T("scanline-size=%d, rows/strip=%d, strip-size=%d\n\n"), scanlinesize, rowsperstrip, stripsize);

	BYTE* pData = (BYTE*)malloc(stripsize * maxReadCount);
	BYTE* ps = pData;
	for (int y = 0; y < height; y += rowsperstrip) {
		uint32 pos = TIFFComputeStrip(tif, y, 0);
		int j = min(rowsperstrip, height - y);
		if (TIFFReadEncodedStrip(tif, pos, ps, j * scanlinesize) == -1) {
			free(pData);
			return NULL;
		}
		ps += stripsize;
	}
	return pData;
}

extern "C"
HDIB WINAPI imgTifLoad(LPCTSTR pszFile)
{
#ifdef UNICODE
	char sbname[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pszFile, -1, sbname, _COUNTOF(sbname), NULL, NULL);
	TIFF* tif = TIFFOpen(sbname, "r");
#else
	TIFF* tif = TIFFOpen(pszFile, "r");
#endif
	if (!tif)
		return NULL;


	// ファイル「F:\W32\TIFF\contrib\ras\tif2ras.c」を参考にせよ！
	int width, height;
	uint16 bitspersample, photometric, sampleperpixel;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &sampleperpixel);

// dbg.printf(_T("photometric=%d, width=%d, height=%d, sample/pixel=%d, bits/sample=%d\n\n"), photometric, width, height, sampleperpixel, bitspersample);

	int depth;
	switch (photometric) {
		case PHOTOMETRIC_MINISBLACK:
		case PHOTOMETRIC_MINISWHITE:
			depth = (bitspersample <= 8) ? bitspersample : 24;
			break;

		case PHOTOMETRIC_PALETTE:
			depth = min(bitspersample, 8);
			break;

		case PHOTOMETRIC_RGB:
		case PHOTOMETRIC_SEPARATED:				// 5 CMYK
		case PHOTOMETRIC_YCBCR:					// 6 !CCIR 601
		case PHOTOMETRIC_CIELAB:				// 8 L*a*b*
		case PHOTOMETRIC_LOGLUV:
			depth = (bitspersample == 16) ? 24 : max(bitspersample, 24);
			break;

		default:
			TIFFClose(tif);
			return NULL;
	}

	int numcolors = (1 << bitspersample);

	// DIB作成
	HDIB hdib = dib_create(width, height, depth);
	BITMAPINFOHEADER* pbh  = dib_lockheader(hdib);
	if (!pbh) {
		TIFFClose(tif);
		return NULL;
	}
	BITMAPINFO* pbi = (BITMAPINFO*)pbh;

	// 全データをデコード
	BYTE* pStrip = tiff_decode(tif);
	if (!pStrip) {
		goto err_tiffload;
	}

	pbh->biClrUsed = pbh->biClrImportant = (depth <= 8) ? numcolors : 0;
	int i;
	switch (photometric) {
		case PHOTOMETRIC_MINISBLACK:
		case PHOTOMETRIC_MINISWHITE:
			if (bitspersample <= 8) {
				for (i = 0; i < numcolors; i++) {
					int j = (photometric == PHOTOMETRIC_MINISBLACK) ? (255 * i) / (numcolors - 1): 255 - (255 * i) / (numcolors - 1);
					pbi->bmiColors[i].rgbRed = (BYTE)j;
					pbi->bmiColors[i].rgbGreen = (BYTE)j;
					pbi->bmiColors[i].rgbBlue = (BYTE)j;
				}
			} 
			break;

		case PHOTOMETRIC_PALETTE: {
			// パレットが16ビットか8ビットかを判定する
			uint16 *rm, *gm, *bm;
			TIFFGetField(tif, TIFFTAG_COLORMAP, &rm, &gm, &bm);
//DumpBox((BYTE*)rm, colors * sizeof(WORD));
//DumpBox((BYTE*)gm, colors * sizeof(WORD));
//DumpBox((BYTE*)bm, colors * sizeof(WORD));

			int palsize = checkcmap(numcolors, rm, gm, bm);
			// パレットデータをDIBへ格納
			for (i = numcolors - 1; i >= 0; i--) {
				if (palsize == 16) {
					pbi->bmiColors[i].rgbRed = CVT(rm[i]);
					pbi->bmiColors[i].rgbGreen = CVT(gm[i]);
					pbi->bmiColors[i].rgbBlue = CVT(bm[i]);
				} else {
					pbi->bmiColors[i].rgbRed = (BYTE)rm[i];
					pbi->bmiColors[i].rgbGreen = (BYTE)gm[i];
					pbi->bmiColors[i].rgbBlue = (BYTE) bm[i];
				}
			}
			break;
		}

		case PHOTOMETRIC_RGB:
		case PHOTOMETRIC_SEPARATED:				// CMYK
		case PHOTOMETRIC_YCBCR:					// !CCIR 601
		case PHOTOMETRIC_CIELAB:				// L*a*b*
		case PHOTOMETRIC_LOGLUV:				// CIE Log2(L) (u',v')
			break;

		default:
			goto err_tiffload;
	}

	int scanlinesize = TIFFScanlineSize(tif);	// Number of byte in ine line

	BYTE* pBase = dib_findbits(pbh);
	for (int y = 0; y < height; y++) {
		BYTE* ps = pStrip + y * scanlinesize;
		BYTE* pd = pBase + WIDTHBYTES(width * depth) * (height - y - 1);

// if (y == 0) dbg.dump(ps, scanlinesize);

		switch (photometric) {
			case PHOTOMETRIC_RGB:
				if (bitspersample == 16) {
					WORD* pws = (WORD*)ps;
					for (i = 0; i < width; i++) {
						*pd++ = CVT(pws[2]);
						*pd++ = CVT(pws[1]);
						*pd++ = CVT(pws[0]);
						pws += sampleperpixel;
					}
				} else {
					for (i = 0; i < width; i++) {
						*pd++ = ps[2];
						*pd++ = ps[1];
						*pd++ = ps[0];
						ps += sampleperpixel;
					}
				}
				break;

			case PHOTOMETRIC_MINISWHITE:
			case PHOTOMETRIC_MINISBLACK:
				if (bitspersample <= 8) {
					memcpy(pd, ps, scanlinesize);
				} else if (bitspersample == 16) {
					WORD* pws = (WORD*)ps;
					for (i = 0; i < width; i++) {
						BYTE b = CVT(*pws);
						*pd++ = b;
						*pd++ = b;
						*pd++ = b;
						pws += sampleperpixel;
					}
				}
				break;

			case PHOTOMETRIC_PALETTE:
				memcpy(pd, ps, scanlinesize);
				break;

			case PHOTOMETRIC_SEPARATED:				// CMYK
				if (bitspersample == 16) {
					WORD* pws = (WORD*)ps;
					for (i = 0; i < width; i++) {
						cmyk_rgb(pws, pd, TRUE);
						pd += 3;
						pws += sampleperpixel;
					}
				} else {
					for (i = 0; i < width; i++) {
						cmyk_rgb(ps, pd, FALSE);
						pd += 3;
						ps += sampleperpixel;
					}
				}
				break;

			case PHOTOMETRIC_YCBCR: {				// YCbCr
				TIFFYCbCrToRGB* ycbcr = initializeYCrCb(tif);
				if (ycbcr) {
					/* Start conversion */
					uint32 r, g, b;
					uint32 Y;
					int32 Cb, Cr;
					for (i = 0; i < width; i++) {
						Y = ps[0];
						Cb = ps[1];
						Cr = ps[2];
						TIFFYCbCrtoRGB(ycbcr, Y, Cb, Cr, &r, &g, &b);
						*pd++ = (BYTE)b;
						*pd++ = (BYTE)g;
						*pd++ = (BYTE)r;
						ps += sampleperpixel;
					}
					_TIFFfree(ycbcr);
				}
				break;
			}

			case PHOTOMETRIC_CIELAB: {
				TIFFCIELabToRGB* cielab = initializeCIELab(tif);
				if (cielab) {
					/* Now we can start to convert */
					uint32 r, g, b;
					uint32 L;
					int32 la, lb;
					float X, Y, Z;
					for (i = 0; i < width; i++) {
						L = ps[0];
						la = ps[1];
						lb = ps[2];
						TIFFCIELabToXYZ(cielab, L, la, lb, &X, &Y, &Z);
						TIFFXYZToRGB(cielab, X, Y, Z, &r, &g, &b);
						*pd++ = (BYTE)b;
						*pd++ = (BYTE)g;
						*pd++ = (BYTE)r;
						ps += sampleperpixel;
					}
					_TIFFfree(cielab);
				}
				break;
			}

			default:
				break;
		}
	}
	free(pStrip);
	dib_unlock(hdib); 
	TIFFClose(tif);
	return hdib;

err_tiffload:
	if (pStrip) {
		free(pStrip);
	}
	dib_unlock(hdib); 
	TIFFClose(tif);
	return NULL;
}

extern "C"
BOOL WINAPI imgTifSaveParams(LPCTSTR pszFile, HDIB hdib, int compression)
{
	int x, y;

	TIFF *tif = NULL;
#ifdef UNICODE
	char sbname[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pszFile, -1, sbname, _COUNTOF(sbname), NULL, NULL);
	tif = TIFFOpen(sbname, "w");
#else
	tif = TIFFOpen(pszFile, "w");
#endif
	if (tif == NULL)
		return FALSE;

	mif image;
	if (!image.attachdib(hdib)) {
		TIFFClose(tif);
		DeleteFile(pszFile);
		return FALSE;
	}

	short metric;
	short samplesperpixel;
	switch (image.m_bits) {
		case 1:
		case 4:
		case 8:
			samplesperpixel = 1;
			metric = PHOTOMETRIC_PALETTE;
			break;

		case 24:
			samplesperpixel = 3;
			metric = PHOTOMETRIC_RGB;
			break;

		default:
			return FALSE;
	}

	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, image.m_width);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, image.m_height);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, metric);
	TIFFSetField(tif, TIFFTAG_DOCUMENTNAME, "");
	TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "KMIMG");
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, image.m_height);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_SOFTWARE, "KMIMG 1.5");
	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(tif, TIFFTAG_XRESOLUTION, 72);
	TIFFSetField(tif, TIFFTAG_YRESOLUTION, 72);

	WORD compress_tag = COMPRESSION_NONE;
	if (compression == TIFF_COMPRESS_JPEG)
		compress_tag = COMPRESSION_JPEG;
	if (compression == TIFF_COMPRESS_LZW)
		compress_tag = COMPRESSION_LZW;
	if (compression == TIFF_COMPRESS_PACKBITS)
		compress_tag = COMPRESSION_PACKBITS;
	if (compression == TIFF_COMPRESS_ZIP)
		compress_tag = COMPRESSION_DEFLATE;
	TIFFSetField(tif, TIFFTAG_COMPRESSION, compress_tag);

	char buf[MAX_PATH];
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintfA(buf, "%04d/%02d/%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	TIFFSetField(tif, TIFFTAG_DATETIME, buf);

	if (metric == PHOTOMETRIC_PALETTE) {
		WORD red[256], green[256], blue[256];
		memset(red, 0, sizeof(red));
		memset(green, 0, sizeof(green));
		memset(blue, 0, sizeof(blue));
		for (x = 0; x < image.m_colors; x++) {
			red[x] = SCALE(image.m_map[x].red);
			green[x] = SCALE(image.m_map[x].green);
			blue[x] = SCALE(image.m_map[x].blue);
		}
		TIFFSetField(tif, TIFFTAG_COLORMAP, red, green, blue);
	}

	BYTE* pline = (BYTE*)malloc(samplesperpixel * image.m_width);
	register RGBA* p = image.m_pixels;
	for (y = 0; y < image.m_height; y++) {
		for (x = 0; x < image.m_width; x++) {
			if (metric == PHOTOMETRIC_PALETTE)
				pline[x] = p->alpha;
			else {
				pline[x * 3] = p->red;
				pline[x * 3 + 1] = p->green;
				pline[x * 3 + 2] = p->blue;
			}
			p++;
		}

		if (TIFFWriteScanline(tif, pline, y, 0) < 0)
			break;
	}
	free(pline);

	TIFFFlushData(tif);
	TIFFClose(tif);
	return TRUE;
}

extern "C"
BOOL WINAPI imgTifSave(LPCTSTR pszFile, HDIB hdib)
{
	return imgTifSaveParams(pszFile, hdib, TIFF_COMPRESS_LZW);
}


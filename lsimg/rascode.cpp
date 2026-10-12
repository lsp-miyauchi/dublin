/*-
 * ras.c - Converts from a Sun Rasterfile to a DIB
 *
 */
#define STRICT
#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

#define ROLD			0 /* old format, see /usr/include/rasterfile.h */
#define RSTANDARD		1 /* standard format */
#define RRLENCODED		2 /* run length encoding to compress the image */
#define RRGB			3 /* RGB-format instead of BGR in 24 or 32-bit mode */
#define RTIFF			4 /* TIFF <-> rasterfile */
#define RIFF			5 /* IFF (TAAC) <-> rasterfile */

/* these are the possible colormap types.  if it's in RGB format,
 * the map is made up of three byte arrays (red, green, then blue)
 * that are each 1/3 of the colormap length.
 */

#define RNOMAP  0 /* no colormap follows the header */
#define RRGBMAP 1 /* rgb colormap */
#define RRAWMAP 2 /* raw colormap; good luck */
#define RESC 128 /* run-length encoding escape character */

#pragma pack(1)
typedef struct tagRASHDR {
	DWORD magic;
	BYTE width[4];
	BYTE height[4];
	BYTE depth[4];
	BYTE length[4];
	BYTE type[4];
	BYTE cmaptype[4];
	BYTE cmaplength[4];
} RASHDR;
typedef RASHDR* LPRASHDR;
#pragma pack()


static void sunread(MemStream* pmf, BYTE* buf, int len, BOOL enc)
{
	static BYTE repchar, remaining = 0;

	if (enc) {		/* rl-encoded read */
		while (len--) {
			if (remaining) {
				remaining--;
				*(buf++)= repchar;
			}
			else {
				repchar = _mfgetc(pmf);
				if (repchar == RESC) {
					remaining = _mfgetc(pmf);
					if (remaining == 0)
						*(buf++)= RESC;
					else {
						repchar = _mfgetc(pmf);
						*(buf++)= repchar;
					}
				}
				else
					*(buf++)= repchar;
			}
		}
	}
	else		/* normal read */
		_mfread(pmf, buf, len);
}

static HDIB load_ras(MemStream* pms)
{
	LPBITMAPINFO pbi;
	LPBITMAPINFOHEADER pbh;
	RASHDR hdr;
	LPBYTE pixels, lpw;
	DWORD nWidth, nHeight, bpp, nType, maptype, mapsize;
	HDIB hdib = NULL;
	DWORD fill;			/* # of fill bytes per raster line */
	BOOL enc;
	DWORD rgb_format;
	BYTE fillchar;
	DWORD x, y;
	DWORD linelen;		/* length of raster line in bytes */

	// ヘッダ情報の取得とRASファイルチェック
	if (sizeof(hdr) != _mfread(pms, &hdr, sizeof(hdr)) ||
		hdr.magic != 0x956aa659)
		goto red_exit;

	bpp = (DWORD)hdr.depth[3];
	nWidth = (DWORD)((WORD)hdr.width[2] * 256 + hdr.width[3]);
	nHeight = (DWORD)((WORD)hdr.height[2] * 256 + hdr.height[3]);
	nType = (DWORD)hdr.type[3];
	maptype = (DWORD)hdr.cmaptype[3];
	mapsize = (DWORD)((WORD)hdr.cmaplength[2] * 256 + hdr.cmaplength[3]);
	mapsize /= 3;

	enc = 0;
	rgb_format = 0;
	switch (nType) {
		case ROLD:
		case RSTANDARD:
			break;

		case RRLENCODED:
			enc = 1;
			break;

		case RRGB:
			rgb_format = 1;
			break;

		case RTIFF: /* sorry, don't even know what these are */
		case RIFF:
		default:
			goto red_exit;
	}

	hdib = dib_create(nWidth, nHeight, bpp);
	pbi = dib_lockinfo(hdib);
	pbh = (LPBITMAPINFOHEADER)pbi;

	linelen = WIDTHBYTES(pbh->biWidth * pbh->biBitCount);
	fill = (linelen % 2 ? 1 : 0);

	// パレットの設定
	if (maptype == RRGBMAP) {
		pbh->biClrUsed = mapsize;
		pbh->biClrImportant = mapsize;
		for (x = 0; x < mapsize; x++)	pbi->bmiColors[x].rgbRed = _mfgetc(pms);
		for (x = 0; x < mapsize; x++)	pbi->bmiColors[x].rgbGreen = _mfgetc(pms);
		for (x = 0; x < mapsize; x++)	pbi->bmiColors[x].rgbBlue = _mfgetc(pms);
	}

	pixels = dib_findbits(pbi);
	if (bpp >= 24) {
		BYTE* buf, * bp;
		buf = (BYTE*)malloc(nWidth * (bpp == 24 ? 3 : 4));

		for (y = 0; y < nHeight; y++) {
			sunread(pms, buf, nWidth * (bpp == 24 ? 3 : 4), enc);
			bp = buf;

			lpw = pixels + linelen * (nHeight - y - 1);
			if (bpp == 24) {
				if (rgb_format) {
					for (x = 0; x < nWidth; x++) {
						*lpw++ = *(bp + 2); /* red */
						*lpw++ = *(bp + 1); /* green */
						*lpw++ = *bp;       /* blue */
						bp += 3;
					}
				}
				else {
					for (x = 0; x < nWidth; x++) {
						*lpw++ = *bp++; /* red */
						*lpw++ = *bp++; /* green */
						*lpw++ = *bp++; /* blue */
					}
				}
			}
			else {
				if (rgb_format) {
					for (x = 0; x < nWidth; x++) {
						*lpw++ = *(bp + 3); /* red */
						*lpw++ = *(bp + 2); /* green */
						*lpw++ = *(bp + 1); /* blue */
						bp += 4;
					}
				}
				else {
					for (x = 0; x < nWidth; x++) {
						bp++;                  /* skip byte */
						*lpw++ = *bp++; /* red */
						*lpw++ = *bp++; /* green */
						*lpw++ = *bp++; /* blue */
					}
				}
			}
			if (fill)
				sunread(pms, &fillchar, fill, enc);
		}
		free(buf);
	}
	else {
		for (y = 0; y < nHeight; y++) {
			lpw = pixels + linelen * (nHeight - y - 1);
			sunread(pms, lpw, nWidth, enc);
			lpw += nWidth;
			if (fill)
				sunread(pms, &fillchar, fill, enc);
		}
	}

red_exit:

	if (hdib)
		dib_unlock(hdib);

	return hdib;
}

extern "C"
HDIB WINAPI imgRasLoadMem(HANDLE hmem)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(hmem);
	if (pms) {
		hdib = load_ras(pms);
		_mfclose(pms);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgRasLoad(LPCTSTR pszName)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(pszName);
	if (pms) {
		hdib = load_ras(pms);
		_mfclose(pms);
	}
	return hdib;
}


////////////////////////////////////////////////////////////////////////
// 注意：保存は8bppまたは24bppのみ

static BOOL ras_save_core(HANDLE hf, HDIB hdib)
{
	LPBITMAPINFO pbi;
	LPBITMAPINFOHEADER pbh;
	RASHDR hdr;
	LPBYTE pixels, lpw, pBase;
	DWORD mapsize, colors;
	DWORD fill;			/* # of fill bytes per raster line */
	BYTE fillchar;
	DWORD x, y, dw, linelen;

	pbi = dib_lockinfo(hdib);
	pbh = (BITMAPINFOHEADER*)pbi;
	pBase = (BYTE*)pbi;

	// ヘッダ情報の設定と書き込み
	memset(&hdr, 0, sizeof(hdr));
	hdr.magic = 0x956aa659;
	hdr.depth[3] = (BYTE)pbh->biBitCount;
	hdr.width[2] = HIBYTE((WORD)pbh->biWidth);
	hdr.width[3] = (BYTE)pbh->biWidth;
	hdr.height[2] = HIBYTE((WORD)pbh->biHeight);
	hdr.height[3] = (BYTE)pbh->biHeight;
	hdr.type[3] = RSTANDARD;
	colors = dib_numcolors(pBase);
	mapsize = (colors > 0) ? 256 : 0;
	hdr.cmaptype[3] = (mapsize > 0) ? RRGBMAP : RNOMAP;
	hdr.cmaplength[2] = HIBYTE((WORD)(mapsize * 3));
	hdr.cmaplength[3] = (BYTE)(mapsize * 3);
	linelen = WIDTHBYTES(pbh->biWidth * pbh->biBitCount);
	fill = (linelen % 2 ? 1 : 0);
	if (pbh->biBitCount == 24)
		dw = (pbh->biWidth * 3 + fill) * pbh->biHeight;
	else
		dw = (pbh->biWidth + fill) * pbh->biHeight;
	hdr.length[3] = (BYTE)dw;
	hdr.length[2] = (BYTE)(dw >> 8);
	hdr.length[1] = (BYTE)(dw >> 16);
	hdr.length[0] = (BYTE)(dw >> 24);
	WriteFile(hf, &hdr, sizeof(hdr), &dw, NULL);

	// カラーマップの書き込み
	if (mapsize > 0) {
		BYTE red[256], green[256], blue[256];
		ZeroMemory(&red, 256);
		ZeroMemory(&green, 256);
		ZeroMemory(&blue, 256);

		for (x = 0; x < colors; x++) {
			red[x] = pbi->bmiColors[x].rgbRed;
			green[x] = pbi->bmiColors[x].rgbGreen;
			blue[x] = pbi->bmiColors[x].rgbBlue;
		}
		WriteFile(hf, &red, mapsize, &dw, NULL);
		WriteFile(hf, &green, mapsize, &dw, NULL);
		WriteFile(hf, &blue, mapsize, &dw, NULL);
	}

	fillchar = 0;
	pixels = dib_findbits(pbi);
	for (y = 0; y < pbh->biHeight; y++) {
		lpw = pixels + linelen * (pbh->biHeight - y - 1);
		if (pbh->biBitCount == 24)
			WriteFile(hf, lpw, pbh->biWidth * 3, &dw, NULL);
		else
			WriteFile(hf, lpw, pbh->biWidth, &dw, NULL);
		if (fill)
			WriteFile(hf, &fillchar, 1, &dw, NULL);
	}
	dib_unlock(hdib);
	return TRUE;
}

extern "C"
BOOL WINAPI imgRasSave(LPCTSTR pszName, HDIB hdib)
{
	BOOL bOK = FALSE;
	if (hdib && pszName && *pszName) {
		HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			bOK = ras_save_core(hf, hdib);
			CloseHandle(hf);
		}
	}
	return bOK;
}

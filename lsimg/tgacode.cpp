#define STRICT
#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

////////////////////////////////////////////////////////////////
//

#pragma pack(1)
typedef struct tagTGAHEADER
{
	BYTE idlen; 			/* Length of Identifier String */
	BYTE cmaptype;			/* 0 = NoMap */
	BYTE imagetype;			/* Image Type (1,2,3,9,10,11) */
	WORD cmapstart;			/* Index of first colour map entry */
	WORD cmaplen;			/* Length of colour map (number of entries) */
	BYTE cmapdepth;			/* Length of colour map entry */
	WORD xoff;				/* X Origin of Image */
	WORD yoff;				/* Y Origin of Image */
	WORD width;				/* width of Image */
	WORD height; 			/* height of Image */
	BYTE depth;				/* PixelSize (8,16,24) */
	BYTE descriptor;		// 00000000
							//     1111 ---- # of Attribute bits / pixel
							//   11     ---- orign location
							// 11       ---- unused (Interlieve)
} TGAHEADER;
typedef TGAHEADER* LPTGAHEADER;
#pragma pack()

char TGA_ImageIDField[256];

/* Definitions for Image Types */

#define TGA_MAPRGB			1
#define TGA_RAWRGB	 		2
#define TGA_RAWMONO			3
#define TGA_MAPENCODE		9
#define TGA_RAWENCODE		10
#define TGA_MONOENCODE		11

// Define flags for mode - these indicate special conditions
#define GREYSC				0
#define COLOUR				1
#define MAPPED				2
#define RLENCD				4
#define INTERL				8
#define FOURWY				16

int RLE_count = 0, RLE_flag = 0;


////////////////////////////////////////////////////////////////
//

static RGBQUAD* lpColorMap = NULL;





////////////////////////////////////////////////////////////////
//

static BOOL __fastcall get_map_entry(MemStream* pmf, RGBQUAD* prgbq, int depth, int /*mode*/)
{
	UINT l;
	BYTE r, g, b;

	// read appropriate number of bytes, break into rgb & put in map
	switch (depth) {
		case 8:						// Grey Scale already, read and triplicate
			r = g = b = _mfgetc(pmf);
			break;

		case 16: 					// 5 bits each of red green and blue
		case 15: 					// Watch for byte order
			l = _mfgetw(pmf);
			r = ((l >> 10) & 0x1F) << 3;
			g = ((l >> 5) & 0x1F) << 3;
			b = (l & 0x1F) << 3;
			break;

		case 32: 					// Read alpha byte & throw away
		case 24: 					// Eight bits each of red green and blue
			b = _mfgetc(pmf);
			g = _mfgetc(pmf);
			r = _mfgetc(pmf);
			if (depth == 32)
				_mfgetc(pmf);
			break;

		default:
			return FALSE;
	}
	prgbq->rgbRed = r;
	prgbq->rgbGreen = g;
	prgbq->rgbBlue = b;
	return TRUE;
}

static BOOL __fastcall get_pixel(MemStream* pmf, RGBQUAD* prgbq, int depth, int mode)
{
	static int Red, Grn, Blu;
	BYTE i;
	UINT l;

	// Check if run length encoded.
	if ((mode & RLENCD) != 0)
	{
		if (RLE_count == 0)				// Have to restart run
		{
			i = _mfgetc(pmf);
			RLE_flag = (i & 0x80) >> 7;
			if (RLE_flag == 0)			// Stream of unencoded pixels
				RLE_count = i + 1;
			else						// Single pixel replicated
				RLE_count = i - 127;
			RLE_count--;				// Decrement count & get pixel
		}
		else
		{
			// Have already read count & (at least) first pixel
			RLE_count--;
			if (RLE_flag != 0)			// Replicated pixels
				goto PixEncode;
		}
	}

	// Read appropriate number of bytes, break into RGB
	switch (depth)
	{
		case 8:							// Grey Scale - read a byte and triplicate
			i = _mfgetc(pmf);
			Red = i;
			Grn = i;
			Blu = i;
			l = i;
			break;

		case 16: 						// Five bits each of red green and blue
		case 15: 						// Watch byte order
			l = _mfgetw(pmf);
			Red = ((l >> 10) & 0x1F) << 3;
			Grn = ((l >> 5) & 0x1F) << 3;
			Blu = (l & 0x1F) << 3;
			break;

		case 32: 						// Read alpha byte & throw away
		case 24:				 		// Eight bits each of red green and blue
			Blu = _mfgetc(pmf);
			Grn = _mfgetc(pmf);
			Red = _mfgetc(pmf);
			if (depth == 32)
				_mfgetc(pmf);
			l = 0;
			break;

		default:
			return FALSE;
	}

PixEncode:
	if ((mode & MAPPED) == MAPPED)
	{
		if (depth == 8)
			prgbq->rgbBlue = prgbq->rgbGreen = prgbq->rgbRed = l;
		else
		{
			prgbq->rgbBlue = lpColorMap[l].rgbBlue;
			prgbq->rgbGreen = lpColorMap[l].rgbGreen;
			prgbq->rgbRed = lpColorMap[l].rgbRed;
		}
	}
	else
	{
		prgbq->rgbBlue = Blu;
		prgbq->rgbGreen = Grn;
		prgbq->rgbRed = Red;
	}
	return TRUE;
}

static BOOL __fastcall read_tga(MemStream* pmf, BYTE* pixels, TGAHEADER* tga, int bits)
{
	register UINT i, j, iloop;
	UINT mode;
	ULONG k, width;
	RGBQUAD rgbq;
	LPBYTE lpTo;

	switch (tga->imagetype)
	{
		case TGA_MAPRGB:
		case TGA_RAWRGB:
		case TGA_RAWMONO:
		case TGA_MAPENCODE:
		case TGA_RAWENCODE:
		case TGA_MONOENCODE:
			break;

		default:
			return FALSE;
	}

	if (tga->imagetype == TGA_RAWMONO || tga->imagetype == TGA_MONOENCODE)
		mode = GREYSC;
	else
		mode = COLOUR;

	// ID文字列があれば、それを読みとばす
	if (tga->idlen != 0)
		_mfseek(pmf, (long)tga->idlen, FILE_CURRENT);

	// カラーマップ
	BOOL bOK;
	if (tga->cmaptype != 0)
	{
		for (i = 0;  i < tga->cmaplen; i++)
		{
			bOK = get_map_entry(pmf, &lpColorMap[i], tga->cmapdepth, mode);
			if (!bOK)
				return FALSE;
		}

		if ((tga->imagetype != TGA_RAWRGB) &&
			(tga->imagetype != TGA_RAWMONO) &&
			(tga->imagetype != TGA_MONOENCODE) &&
			(tga->imagetype != TGA_RAWENCODE))
			mode = mode | MAPPED;
	}

	// Check Run Length Encoding
	if ((tga->imagetype == TGA_MAPENCODE) ||
		 (tga->imagetype == TGA_MONOENCODE)||
		 (tga->imagetype == TGA_RAWENCODE))
		mode = mode | RLENCD;

	// Check for interlacing of the Targa file
	switch (tga->descriptor >> 6)
	{
		case 2:						// Four way interlace
			mode = mode | FOURWY;
		case 1:						// Two way interlace
			mode = mode | INTERL;
		case 0:						// No interlace
			break;

		default: 					// Reserved - we'll let it pass
			break;
	}

	// DIBに配置する際の１ラインあたりのバイト数を求める
	width = WIDTHBYTES(tga->width * bits);

	// TGAデータの展開
	for (i = 0; i < tga->height;  i++) {
		if ((mode & INTERL) == 0)				// No interlace
			j = i;
		else if ((mode & FOURWY) != 0) {		// Two way interlace
			if ((i << 1) < tga->height)
				j = i << 1;
			else {
				j = i - (tga->height >> 1);
				j = (j << 1) + 1;
			}
		}
		else {									// Four way interlace
			if ((i << 2) < tga->height)
				j = i << 2;
			else if ((i << 1) < tga->height) {
				j = i - (tga->height >> 2);
				j = (j << 2) + 1;
			}
			else if ((i << 2) < 3 * tga->height) {
				j = i - (tga->height >> 1);
				j = (j << 2) + 2;
			}
			else {
				j = i - (tga->height >> 1) - (tga->height >> 2);
				j = (j << 2) + 3;
			}
		}

		if (tga->descriptor & 0x30)
			k = (tga->height - 1 - j) * width;
		else
			k = j * width;

		lpTo = pixels + k;
		for (iloop = 0; iloop < tga->width; iloop++)
		{
			bOK = get_pixel(pmf, &rgbq, tga->depth, mode);
			if (!bOK)
				return FALSE;

			*lpTo++ = rgbq.rgbBlue;
			if (tga->depth != 8)
			{
				*lpTo++ = rgbq.rgbGreen;
				*lpTo++ = rgbq.rgbRed;
			}
		}
	}
	return TRUE;
}

static HDIB WINAPI load_tga(MemStream* pms)
{
	TGAHEADER header;
	if ((_mfread(pms, &header, 18)) != 18) {
		return NULL;
	}
	if (header.imagetype < 1 || header.imagetype > 11 || header.depth > 32) {
		return NULL;
	}

	// DIB作成のためのビットカウントの補正
	int bits;
	if (header.depth == 8)
		bits = 8;
	else if (header.depth == 15 ||
			 header.depth == 16 ||
			 header.depth == 24 ||
			 header.depth == 32)
		bits = 24;
    else
		return NULL;

	// Create the DIB structures
	HDIB hdib = dib_create(header.width, header.height, bits);
	if (!hdib)
		return NULL;

	LPBITMAPINFO pbi = dib_lockinfo(hdib);
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pbi;
	pbh->biPlanes = 1;
	pbh->biClrUsed = pbh->biClrImportant = header.cmaplen;

	// カラーマップの保存先
	lpColorMap = (RGBQUAD*)pbi->bmiColors;

	BYTE* pixels = dib_findbits(pbi);
	BOOL bOK = read_tga(pms, pixels, &header, bits);
	dib_unlock(hdib);

	if (!bOK) {
		GlobalFree(hdib);
		hdib = NULL;
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgTgaLoadMem(HANDLE hmem)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(hmem);
	if (pms) {
		hdib = load_tga(pms);
		_mfclose(pms);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgTgaLoad(LPCTSTR pszFile)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(pszFile);
	if (pms) {
		hdib = load_tga(pms);
		_mfclose(pms);
	}
	return hdib;
}


//////////////////////////////////////////////////////////////////////
//

static BOOL __fastcall write_header(MemStream* pmf, LPBITMAPINFOHEADER pbh)
{
	TGAHEADER header;
	memset(&header, 0, sizeof(header));

	int colors = dib_numcolors(pbh);
	if (colors > 0) {
		header.cmaptype = 1;			// color map type 1
		header.cmaplen = colors;		// color map length
		header.cmapdepth = 24;			// 24 bits per cmap entry
	}

	header.width = (WORD)pbh->biWidth;
	header.height = (WORD)pbh->biHeight;
	header.descriptor = 0x20;			// Top-down, non-interlaced

	if (colors > 0) {
		header.imagetype = 1;			// image type = colormapped RGB
		header.depth = 8;
	}
	else {
		header.imagetype = 2;			// image type = uncompressed RGB
		header.depth = 24;
	}

	return (_mfwrite(pmf, &header, sizeof(header)) == sizeof(header));
}

static BOOL __fastcall write_colormap(MemStream* pmf, LPRGBQUAD prq, int colors)
{
	if (colors == 0)
		return TRUE;

	DWORD dw = sizeof(BYTE) * 3 * colors;
	LPBYTE pb = (LPBYTE)malloc(dw);
	if (pb) {
		register int i;
		register LPBYTE pw = pb;
		for (i = 0; i < colors; i++) {
			*pw++ = prq[i].rgbBlue;
			*pw++ = prq[i].rgbGreen;
			*pw++ = prq[i].rgbRed;
		}
		BOOL bOK = (_mfwrite(pmf, pb, dw) == dw);
		free(pb);
		return bOK;
	}
	return FALSE;
}

extern "C"
HANDLE WINAPI imgTgaSaveMem(HDIB hdib, DWORD* pdwSize)
{
	if (!hdib)
		return NULL;

	LPBITMAPINFOHEADER pbh = dib_lockheader(hdib);
	DWORD dw = pbh->biWidth * pbh->biHeight * 3 + sizeof(RGBQUAD) * 256 + sizeof(TGAHEADER);
	MemStream* pmf = _mfcreate(dw);
	if (!pmf) {
		dib_unlock(hdib);
		return NULL;
	}

	// Write the colormap.  Note Targa uses BGR byte order
	if (!write_header(pmf, pbh)) {
		_mfclose(pmf);
		dib_unlock(hdib);
		return NULL;
	}

	int colors = dib_numcolors(pbh);
	if (colors > 0) {
		if (!write_colormap(pmf, ((BITMAPINFO*)pbh)->bmiColors, colors)) {
			_mfclose(pmf);
			GlobalUnlock(hdib);
			return NULL;
		}
	}

	DWORD width = WIDTHBYTES(pbh->biWidth * pbh->biBitCount);
	dw = pbh->biWidth;
	if (colors == 0)
		dw *= 3;

	register LPBYTE pb = dib_findbits(pbh) + width * (pbh->biHeight - 1);
	BOOL bOK = FALSE;
	for (int i = 0; i < pbh->biHeight; i++) {
		bOK = (_mfwrite(pmf, pb, dw) == dw);
		pb -= width;
		if (!bOK)
			break;
	}
	dib_unlock(hdib);

	HANDLE hmem = NULL;
	if (bOK) {
		size_t size;
		hmem = _mfdetach(pmf, &size);
		*pdwSize = (DWORD)size;
	}
	else
		_mfclose(pmf);
	return hmem;
}

extern "C"
BOOL WINAPI imgTgaSave(LPCTSTR pszFile, HDIB hdib)
{
	if (!hdib || !pszFile || *pszFile == '\0')
		return FALSE;

	DWORD dwSize;
	HANDLE hmem = imgTgaSaveMem(hdib, &dwSize);
	return SaveMemToFileAndWep(pszFile, hmem, dwSize);
}

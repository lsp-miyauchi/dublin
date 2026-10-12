#define STRICT
#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

static BYTE mono[6]= {0, 0, 0, 255, 255, 255};

#pragma pack(1)
static struct {
    BYTE manufacturer;
    BYTE version;
    BYTE compression;
    BYTE bpp;
    short x1, y1;
    short x2, y2;
    short prn_hdpi;
    short prn_vdpi;
    BYTE colormap[48];
    BYTE reserved;
    BYTE planes;
    short bytesperline;
    short color;
	short video_hdpi;
	short video_vdpi;
    BYTE filler[54];
} pcx_header;
#pragma pack()


static void __fastcall set_cmap(LPBITMAPINFO pbi, BYTE* cmap, int cnt)
{
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pbi;
	pbh->biClrUsed = cnt;
	pbh->biClrImportant = cnt;
	for (int i = 0; i < cnt; i++)
	{
		pbi->bmiColors[i].rgbRed = *cmap++;
		pbi->bmiColors[i].rgbGreen = *cmap++;
		pbi->bmiColors[i].rgbBlue = *cmap++;
	}
}

static void __fastcall readline(MemStream* pmf, BYTE* buffer, int bytes)
{
	static BYTE count= 0, value= 0;
	if (pcx_header.compression) {
		while (bytes--) {
			if (count == 0) {
				value= _mfgetc(pmf);
				if (value < 0xc0)
					count= 1;
				else {
					count= value - 0xc0;
					value= _mfgetc(pmf);
				}
			}
			count--;
			*(buffer++)= value;
		}
	}
	else
		_mfread(pmf, buffer, bytes);
}

static void __fastcall load_8(MemStream* pmf, int width, int height, BYTE *buffer, int bytes)
{
	BYTE* line = (BYTE*)malloc(bytes);
	for (int row = 0; row < height; buffer+= width, ++row) {
		readline(pmf, line, bytes);
		memcpy(buffer, line, width);
	}
	free(line);
}

static void __fastcall load_24(MemStream* pmf, int width, int height, BYTE *buffer, int bytes)
{
	register int x, y, c;
	BYTE* line = (BYTE*)malloc(bytes * 3);
	for (y = 0; y < height; buffer+= width * 3, ++y) {
		for (c = 0; c < 3; ++c) {
			readline(pmf, line, bytes);
			for (x = 0; x < width; ++x)
				buffer[x * 3 + c]= line[x];
		}
	}
	free(line);
}

static void __fastcall load_1(MemStream* pmf, int width, int height, BYTE *buffer, int bytes)
{
	register int x, y;
	BYTE* line = (BYTE*)malloc(bytes);
	for (y = 0; y < height; buffer+= width, ++y) {
		readline(pmf, line, bytes);
		for (x = 0; x < width; ++x) {
			if (line[x / 8] & (128 >> (x % 8)))
				buffer[x] = 1;
			else
				buffer[x] = 0;
		}
	}
	free(line);
}

static void __fastcall load_4(MemStream* pms, int width, int height, BYTE *buffer, int bytes)
{
	register int x, y, c;
	BYTE* line = (BYTE*)malloc(bytes);
	for (y = 0; y < height; buffer += width, ++y) {
		memset(buffer, 0, width);
		for (c = 0; c < 4; ++c) {
			readline(pms, line, bytes);
			for (x = 0; x < width; ++x) {
				if (line[x / 8] & (128 >> (x % 8)))
					buffer[x] += (1 << c);
			}
		}
	}
	free(line);
}


static HDIB load_pcx(MemStream* pms)
{
	if (_mfread(pms, &pcx_header, 128) != 128 || pcx_header.manufacturer != 10) {
		return NULL;
	}

	int offset_x = (int)pcx_header.x1;
	int offset_y = (int)pcx_header.y1;
	int width = (int)pcx_header.x2 - offset_x + 1;
	int height = (int)pcx_header.y2 - offset_y + 1;

	HDIB hdib = NULL;
	if (pcx_header.planes == 3 && pcx_header.bpp == 8)
		hdib = dib_create(width, height, 24);
	else
		hdib = dib_create(width, height, 8);

	if (!hdib) {
		return NULL;
	}

	BITMAPINFO* pbi = dib_lockinfo(hdib);
	BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)pbi;

	BYTE *dest;
	if (pcx_header.planes == 1 && pcx_header.bpp == 1) {
		dest = (BYTE*)malloc(width * height);
		load_1(pms, width, height, dest, (int)pcx_header.bytesperline);
		set_cmap(pbi, mono, 2);
	}
	else if (pcx_header.planes == 4 && pcx_header.bpp == 1) {
		dest = (BYTE*)malloc(width * height);
		load_4(pms, width, height, dest, (int)pcx_header.bytesperline);
		set_cmap(pbi, pcx_header.colormap, 16);
	}
	else if (pcx_header.planes == 1 && pcx_header.bpp == 8) {
		dest = (BYTE*)malloc(width * height);
		load_8(pms, width, height, dest, (int)pcx_header.bytesperline);

		BYTE cmap[769];
		_mfseek(pms, -769, FILE_END);
		_mfread(pms, cmap, 769);
		if (cmap[0] == 0x0c)
			set_cmap(pbi, &cmap[1], 256);
	}
	else if (pcx_header.planes == 3 && pcx_header.bpp == 8) {
		dest = (BYTE*)malloc (width * height * 3);
		load_24(pms, width, height, dest, (int)pcx_header.bytesperline);
	}
	else {
		GlobalUnlock(hdib);
		GlobalFree(hdib);
		return NULL;
	}

	BYTE* pBase = dib_findbits(pbh);
	register BYTE* pixels = dest;
	int bits = (pcx_header.planes == 3 && pcx_header.bpp == 8) ? 24 : 8;
	int widthbytes = WIDTHBYTES(width * bits);
	for (int y = 0; y < height; y++) {
		register BYTE* ps = pBase + widthbytes * (height - y - 1);
		for (int x = 0; x < width; x++) {
			if (bits == 24) {
				*ps++ = pixels[2];
				*ps++ = pixels[1];
				*ps++ = pixels[0];
				pixels += 3;
			}
			else
				*ps++ = *pixels++;
		}
	}
	free(dest);
	dib_unlock(hdib);

	if (bits == 8) {
		HDIB h = NULL;
		if (pcx_header.planes == 1 && pcx_header.bpp == 1)
			h = mifDecreaseColorDirect(hdib, 1);
		else if (pcx_header.planes == 4 && pcx_header.bpp == 1)
			h = mifDecreaseColorDirect(hdib, 4);

		if (h) {
			dib_destroy(hdib);
			hdib = h;
		}
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgPcxLoadMem(HANDLE hmem, DWORD dwSize)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(hmem);
	if (pms) {
		hdib = load_pcx(pms);
		_mfclose(pms);
	}	
	return hdib;
}

extern "C"
HDIB WINAPI imgPcxLoad(LPCTSTR fname)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(fname);
	if (pms) {
		hdib = load_pcx(pms);
		_mfclose(pms);
	}
	return hdib;
}

//////////////////////////////////////////////////////////////////////
//


static void __fastcall writeline(MemStream* pmf, BYTE *buffer, int bytes)
{
	BYTE value, count;
	BYTE *finish = buffer + bytes;

	while (buffer < finish) {
		value = *(buffer++);
		count = 1;

		while (buffer < finish && count < 63 && *buffer == value) {
			count++;
			buffer++;
		}

		if (value < 0xc0 && count == 1)
			_mfputc(pmf, value);
		else {
			_mfputc(pmf, 0xc0 + count);
			_mfputc(pmf, value);
		}
	}
}

static void __fastcall save_8(MemStream* pmf, int width, int height, BYTE *buffer)
{
	int widthbytes = WIDTHBYTES(width * 8);
	BYTE* pb = buffer + widthbytes * (height - 1);
	for (int row = 0; row < height; row++) {
		writeline(pmf, pb, width);
		pb -= widthbytes;
	}
}

static void __fastcall save_24(MemStream* pmf, int width, int height, BYTE *buffer)
{
	int widthbytes = WIDTHBYTES(width * 24);
	int x, y;
	BYTE *line = (BYTE*)malloc(width * 3);
	for (y = 0; y < height; y++) {
		BYTE* pb = buffer + widthbytes * (height - y - 1);
		for (x = 0; x < width; x++) {
			line[x] = pb[2];
			line[width + x] = pb[1];
			line[width * 2 + x] = pb[0];
			pb += 3;
		}
		writeline(pmf, line, width);
		writeline(pmf, &line[width], width);
		writeline(pmf, &line[width * 2], width);
	}
	free(line);
}

extern "C"
HANDLE WINAPI imgPcxSaveMem(HDIB hdib, DWORD* pdwSize)
{
	if (!hdib)
		return NULL;

	int i, width, height, bits, colors;
	HDIB hout = NULL;
	dib_getinfo(hdib, &width, &height, &colors, &bits);
	if (bits < 24) {
		hout = mifIncreaseColor(hdib, 8);
		if (!hout)
			return NULL;
		hdib = hout;
	}

	MemStream* pmf = _mfcreate(GlobalSize(hdib));
	if (!pmf) {
		if (hdib == hout)
			GlobalFree(hdib);
		*pdwSize = 0;
		return NULL;
	}

	memset(&pcx_header, 0, sizeof(pcx_header));
	pcx_header.manufacturer = 0x0a;
	pcx_header.version = 5;
	pcx_header.compression = 1;

	switch (bits) {
		case 8:
			pcx_header.bpp = 8;
			pcx_header.bytesperline = (short)width;
			pcx_header.planes = 1;
			pcx_header.color = 1;
			break;

		case 24:
			pcx_header.bpp = 8;
			pcx_header.planes = 3;
			pcx_header.color = 1;
			pcx_header.bytesperline = width;
			break;

		default:
			break;
	}

	pcx_header.x1 = 0;
	pcx_header.y1 = 0;
	pcx_header.x2 = width - 1;
	pcx_header.y2 = height - 1;
	pcx_header.prn_hdpi = 300;
	pcx_header.prn_vdpi = 300;
	pcx_header.reserved = 0;
	_mfwrite(pmf, &pcx_header, 128);

	// ヘッダの書き出し
	BITMAPINFO* pbi = dib_lockinfo(hdib);
	BYTE* pBase = dib_findbits(pbi);
	switch (bits) {
		case 8:
			save_8(pmf, width, height, pBase);
			_mfputc(pmf, 0x0c);
			for (i = 0; i < 256; i++) {
				if (i < colors) {
					_mfputc(pmf, pbi->bmiColors[i].rgbRed);
					_mfputc(pmf, pbi->bmiColors[i].rgbGreen);
					_mfputc(pmf, pbi->bmiColors[i].rgbBlue);
				}
				else {
					_mfputc(pmf, 0);
					_mfputc(pmf, 0);
					_mfputc(pmf, 0);
				}
			}
			break;

		case 24:
			save_24(pmf, width, height, pBase);
			break;

		default:
			break;
	}
	dib_unlock(hdib);

	if (hdib == hout)
		dib_destroy(hdib);
	size_t size;
	HGLOBAL hmem = _mfdetach(pmf, &size);
	*pdwSize = (DWORD)size;
	return hmem;
}

extern "C"
BOOL WINAPI imgPcxSave(LPCTSTR pszFile, HDIB hdib)
{
	DWORD dwSize;
	HGLOBAL hmem = imgPcxSaveMem(hdib, &dwSize);
	return SaveMemToFileAndWep(pszFile, hmem, dwSize);
}

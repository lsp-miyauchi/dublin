////////////////////////////////////////////////////////////////////////
// dibcode.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define IS_WIN30_DIB(a)		((*(LPDWORD)(a)) == sizeof(BITMAPINFOHEADER))

#pragma pack(1)

typedef struct Bitmap_File_Head_Struct {
	char	zzMagic[2];		// "BM"
	DWORD	bfSize;
	WORD	zzHotX;
	WORD	zzHotY;
	DWORD	bfOffs;
	DWORD	biSize;
} Bitmap_File_Head;

typedef struct Bitmap_Head_Struct {
	DWORD	biWidth;
	DWORD	biHeight;
	WORD	biPlanes;
	WORD	biBitCnt;
	DWORD	biCompr;
	DWORD	biSizeIm;
	DWORD	biXPels;
	DWORD	biYPels;
	DWORD	biClrUsed;
	DWORD	biClrImp;
} Bitmap_Head;

#pragma pack()


extern "C"
HANDLE WINAPI imgDibSaveMem(HDIB hdib, DWORD* pdwSizeMem)
{
	if (hdib == NULL)
		return NULL;

	BITMAPINFOHEADER* pbh = dib_lockheader(hdib);
	int pals = dib_palsize(pbh);

	BITMAPFILEHEADER bfh;
	memset(&bfh, 0, sizeof(bfh));

	bfh.bfType = DIB_HEADER_MARKER;
	DWORD dwSize = pbh->biSize + pals;
	if (pbh->biCompression == BI_RLE8 || pbh->biCompression == BI_RLE4)
		dwSize += pbh->biSizeImage;
	else {
		DWORD dw = WIDTHBYTES(pbh->biWidth  * pbh->biBitCount) * pbh->biHeight;
		dwSize += dw;
		pbh->biSizeImage = dw;
	}

	bfh.bfSize = dwSize + sizeof(BITMAPFILEHEADER);
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + pbh->biSize + pals;

	HANDLE hmem = GlobalAlloc(GMEM_MOVEABLE, bfh.bfSize);
	if (!hmem) {
		GlobalUnlock(hdib);
		return NULL;
	}

	BYTE* pb = (BYTE*)GlobalLock(hmem);
	memcpy(pb, &bfh, sizeof(bfh));
	pb += sizeof(bfh);
	memcpy(pb, pbh, dwSize);

	GlobalUnlock(hmem);
	dib_unlock(hdib);
	if (pdwSizeMem) *pdwSizeMem = bfh.bfSize;
	return hmem;
}

extern "C"
BOOL WINAPI imgDibSave(LPCTSTR pszFile, HDIB hdib)
{
	BOOL bOK = FALSE;
	DWORD dwSize;
	HANDLE hmem = imgDibSaveMem(hdib, &dwSize);
	if (hmem) {
		bOK = SaveMemToFileAndWep(pszFile, hmem, dwSize);
		GlobalFree(hmem);
	}
	return bOK;
}

static _inline DWORD ToL(BYTE* puffer)
	{ return (puffer[0] | puffer[1]<<8 | puffer[2]<<16 | puffer[3]<<24); }

static _inline WORD ToS(BYTE* puffer)
	{ return (puffer[0] | puffer[1]<<8); }


static BOOL readcmap(MemStream* pmf, RGBA* cmap, int number, int size)
{
	BYTE rgb[4];
	for (int i = 0; i < number ; i++) {
		if (!_mfread(pmf, rgb, size))
			return FALSE;

		// Bitmap save the colors in another order! But change only once!
		if (cmap) {
			cmap->red = rgb[2];
			cmap->green = rgb[1];
			cmap->blue = rgb[0];
			cmap++;
		}
	}
	return TRUE;
}

static BOOL readdib(MemStream* pmf, mif& image, int bpp, int compression, DWORD* masks)
{
	BYTE v,wieviel;
	int width = image.m_width, height = image.m_height;
	WORD rgb;
	RGBA* cmap = image.m_map;

	int rowbytes= ((image.m_width * bpp - 1) / 32) * 4 + 4;
	RGBA* pixels = image.m_pixels;
	long offset = pmf->tellp();
	switch (bpp) {
		case 32:
			try {
				parallel_for (0, height, [&](int y) {
					uint32_t num;
					BYTE* data = (BYTE*)_mfdataptr(pmf, offset + y * rowbytes, rowbytes, &num);
					if (data == NULL || (int)num != rowbytes)
						throw y;
					RGBA* p = pixels + (height - y - 1) * width;
					for (int x = 0; x < width; ++x) {
						p->blue  = *data++;
						p->green = *data++;
						p->red   = *data++;
						p->alpha = *data++;
						p++;
					}
				});
			} catch(int) { }
			break;

		case 24:
			try {
				parallel_for (0, height, [&](int y) {
					uint32_t num;
					BYTE* data = (BYTE*)_mfdataptr(pmf, offset + y * rowbytes, rowbytes, &num);
					if (data == NULL || (int)num != rowbytes)
						throw y;
					RGBA* p = pixels + (height - y - 1) * width;
					for (int x = 0; x < width; ++x) {
						p->blue  = *data++;
						p->green = *data++;
						p->red   = *data++;
						p++;
					}
				});
			} catch(int) { }
			break;

		case 16:
		{
			bool type565 = (masks[0] | masks[1] | masks[2]) == 0xffff;
			try {
				parallel_for (0, height, [&](int y) {
					uint32_t num;
					BYTE* data = (BYTE*)_mfdataptr(pmf, offset + y * rowbytes, rowbytes, &num);
					if (data == NULL || (int)num != rowbytes)
						throw y;
					RGBA* p = pixels + (height - y - 1) * width;
					for (int x = 0; x < width; ++x) {
						rgb = ToS(&data[x * 2]);
						if (type565) {
							p->red   = ((rgb >> 11) & 0x1f) << 3;
							p->green = ((rgb >> 5) & 0x3f) << 2;
							p->blue  = (rgb & 0x1f) << 3;
						} else {
							p->red   = ((rgb >> 10) & 0x1f) << 3;
							p->green = ((rgb >> 5) & 0x1f) << 3;
							p->blue  = (rgb & 0x1f) << 3;
						}
						p++;
					}
				});
			} catch(int) { }
			break;
		}

		case 8:
		case 4:
		case 2:
		case 1:
		{
			int y = height - 1;		// Bitmaps begin in the lower left corner
			int x = 0;
			BYTE* buffer = (BYTE*)malloc(rowbytes);
			RGBA* p;
			if (compression == 0) {
				while (_mfread(pmf, &v, 1)) {
					for (int i = 1; (i <= (8 / bpp)) && (x < width); i++, x++) {
						p = pixels + (y * width) + x;
						p->alpha = (v & (((1 << bpp) - 1) << (8 - (i * bpp)))) >> (8 - (i * bpp));
						p->red = cmap[p->alpha].red;
						p->green = cmap[p->alpha].green;
						p->blue = cmap[p->alpha].blue;
					}
					if (x == width) {
						_mfread(pmf, buffer, rowbytes - 1 - (width * bpp - 1) / 8);
						y--;
						x = 0;
					}
					if (y < 0)
						break;
				}
			}
			else {
				while (y >= 0 && x <= width) {
					_mfread(pmf, buffer, 2);
					if (buffer[0] != 0) {
						// Count + Color - record
						for (int j = 0; ((BYTE)j < buffer[0]) && (x < width);) {
							for (int i = 1;
									((i <= (8 / bpp)) &&
									(x < width) &&
									((BYTE)j < buffer[0]));
										i++, x++, j++)
							{
								p = pixels + (y * width) + x;
								p->alpha = (buffer[1] & (((1 << bpp) - 1) << (8 - (i * bpp)))) >> (8 - (i * bpp));
								p->red = cmap[p->alpha].red;
								p->green = cmap[p->alpha].green;
								p->blue = cmap[p->alpha].blue;
							}
						}
					}
					if (buffer[0] == 0 && buffer[1] > 2) {
						// uncompressed record
						wieviel = buffer[1];
						for (int j = 0; j < wieviel; j += (8 / bpp)) {
							_mfread(pmf, &v, 1);
							int i = 1;
							while (i <= (8 / bpp) && x < width) {
								p = pixels + (y * width) + x;
								p->alpha = (v & (((1 << bpp) - 1) << (8 - (i * bpp)))) >> (8 - (i * bpp));
								p->red = cmap[p->alpha].red;
								p->green = cmap[p->alpha].green;
								p->blue = cmap[p->alpha].blue;
								i++;
								x++;
							}
						}

						if ((wieviel % 2) && (bpp == 4))
							wieviel++;
						if ((wieviel / (8 / bpp)) % 2)
							_mfread(pmf, &v, 1);
					}
					if (buffer[0] == 0 && buffer[1] == 0) {
						// Line end
						y--;
						x = 0;
					}
					if (buffer[0] == 0 && buffer[1] == 1) {
						// Bitmap end
						break;
					}

					if (buffer[0] == 0 && buffer[1] == 2) {
						// Deltarecord
						_mfread(pmf, buffer, 2);
						x += buffer[0];
						y -= buffer[1];
					}
				}
			}
			free(buffer);
			break;
		}

		default:
			// This is very bad, we should not be here
			break;
	}

	return TRUE;
}

extern "C"
HDIB WINAPI imgDibLoadMem(HANDLE hmem)
{
	MemStream* pmf = _mfcreate(hmem);
	if (!pmf)
		return NULL;

	BYTE buffer[64];

	if (!_mfread(pmf, buffer, 18) || buffer[0] != 'B' || buffer[1] != 'M')
		return NULL;

	Bitmap_File_Head bfh;

	bfh.bfSize = ToL (&buffer[0x02]);
	bfh.zzHotX = ToS (&buffer[0x06]);
	bfh.zzHotY = ToS (&buffer[0x08]);
	bfh.bfOffs = ToL (&buffer[0x0a]);
	bfh.biSize = ToL (&buffer[0x0e]);

	int Maps;
	Bitmap_Head bh;
	if (bfh.biSize == 12) {			// OS/2 1.x ?
		if (!_mfread(pmf, buffer, 8)) {
			return NULL;
        }

		bh.biWidth = ToS(&buffer[0x00]);       /* 12 */
		bh.biHeight = ToS(&buffer[0x02]);       /* 14 */
		bh.biPlanes = ToS(&buffer[0x04]);       /* 16 */
		bh.biBitCnt = ToS(&buffer[0x06]);       /* 18 */
		bh.biCompr = 0;
		bh.biSizeIm = 0;
		bh.biXPels = bh.biYPels = 0;
		bh.biClrUsed = 0;
		Maps = 3;
	} else if (bfh.biSize == 40) {	// Windows 3.x
		if (!_mfread(pmf, buffer, bfh.biSize - 4)) {
			return NULL;
		}
		bh.biWidth   = ToL(&buffer[0x00]);	/* 12 */
		bh.biHeight  = ToL(&buffer[0x04]);	/* 16 */
		bh.biPlanes  = ToS(&buffer[0x08]);       /* 1A */
		bh.biBitCnt  = ToS(&buffer[0x0A]);	/* 1C */
		bh.biCompr   = ToL(&buffer[0x0C]);	/* 1E */
		bh.biSizeIm  = ToL(&buffer[0x10]);	/* 22 */
		bh.biXPels   = ToL(&buffer[0x14]);	/* 26 */
		bh.biYPels   = ToL(&buffer[0x18]);	/* 2A */
		bh.biClrUsed = ToL(&buffer[0x1C]);	/* 2E */
		bh.biClrImp  = ToL(&buffer[0x20]);	/* 32 */
		Maps = 4;
	} else if (bfh.biSize <= 64) {	// Probably OS/2 2.x
		if (!_mfread(pmf, buffer, bfh.biSize - 4)) {
			return NULL;
		}
		bh.biWidth   = ToL(&buffer[0x00]);       /* 12 */
		bh.biHeight  = ToL(&buffer[0x04]);       /* 16 */
		bh.biPlanes  = ToS(&buffer[0x08]);       /* 1A */
		bh.biBitCnt  = ToS(&buffer[0x0A]);       /* 1C */
		bh.biCompr   = ToL(&buffer[0x0C]);       /* 1E */
		bh.biSizeIm  = ToL(&buffer[0x10]);       /* 22 */
		bh.biXPels   = ToL(&buffer[0x14]);       /* 26 */
		bh.biYPels   = ToL(&buffer[0x18]);       /* 2A */
		bh.biClrUsed = ToL(&buffer[0x1C]);       /* 2E */
		bh.biClrImp  = ToL(&buffer[0x20]);       /* 32 */
		Maps = 3;
	} else {
		return NULL;
	}

	// Valid bitpdepthis 1, 4, 8, 16, 24, 32
	// 16 is awful, we should probably shoot whoever invented it

	// There should be some colors used!
	int numcolors = (bfh.bfOffs - bfh.biSize - 14) / Maps;

	if (bh.biClrUsed == 0 && bh.biBitCnt <= 8)
		bh.biClrUsed = numcolors;

	if (bh.biHeight == 0 || bh.biWidth == 0) {
		return NULL;
	}

	if (bh.biPlanes != 1) {
		return NULL;
	}

	if (bh.biBitCnt <= 8 && (numcolors > 256 || bh.biClrUsed > 256)) {
		return NULL;
	}

	mif image;
	if (bh.biBitCnt == 16)
		image.create(bh.biWidth, bh.biHeight, 24);
	else
		image.create(bh.biWidth, bh.biHeight, bh.biBitCnt != 2 ? bh.biBitCnt : 4);

	// Windows and OS/2 declare filler so that rows are a multiple of
	// word length (32 bits == 4 bytes)

	// Get the Colormap
	DWORD masks[3];
	masks[0] = masks[1] = masks[2] = 0;
	if (bh.biCompr == BI_BITFIELDS && (bh.biBitCnt == 16 || bh.biBitCnt == 32)) {
		masks[0] = _mfgetl(pmf);
		masks[1] = _mfgetl(pmf);
		masks[2] = _mfgetl(pmf);
	} else {
		if (!readcmap(pmf, image.m_map, numcolors, Maps)) {
			return NULL;
		}
	}

	HDIB hdib = NULL;
	if (readdib(pmf, image, bh.biBitCnt, bh.biCompr, masks)) {
		/* 32bit alpha -> 24bit */
		image.applyalpha(RGB(255,255,255));
		hdib = image.createdib();
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgDibLoad(LPCTSTR fname)
{
	MemStream* pms = _mfcreate(fname);
	if (pms) {
		size_t size;
		HGLOBAL hmem = _mfdetach(pms, &size);
		HDIB hdib = imgDibLoadMem(hmem);
		GlobalFree(hmem);
		return hdib;
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgDibFromClipboard(HWND hwnd)
{
	// クリップボードからのデータをBI_RGB形式で読み込む
	HDIB hdib = NULL;
	if (OpenClipboard(hwnd)) {
		HANDLE hf;
		if (hf = GetClipboardData(CF_DIB)) {
			mif image;
			if (image.attachdib(hf))
				hdib = image.createdib();
		}
		else if (hf = GetClipboardData(CF_BITMAP)) {
			HPALETTE hpal = (HPALETTE)GetClipboardData(CF_PALETTE);
			hdib = dib_createfrombitmap((HBITMAP)hf, hpal);
		}
		CloseClipboard();
	}
	return hdib;
}

extern "C"
BOOL WINAPI imgDibToClipboard(HWND hwnd, HDIB hdib)
{
	// クリップボードへDIBデータをコピー
	BOOL bOK = FALSE;
	if (hdib) {
		if (OpenClipboard(hwnd)) {
			EmptyClipboard();

			HPALETTE hpal;
			HBITMAP hbm;
			imgCreateBitmapFromDIB(hdib, &hbm, &hpal);

			SetClipboardData(CF_DIB, hdib);
			SetClipboardData(CF_PALETTE, hpal);
			SetClipboardData(CF_BITMAP, hbm);
			CloseClipboard();
			bOK = TRUE;
		}
	}
	return bOK;
}

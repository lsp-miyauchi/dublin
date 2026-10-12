// png2bmp.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "mfmem.h"


#define USE_LIBPNG_16


#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

#include <png.h>
#ifdef USE_LIBPNG_16
	#include "pngpriv.h"
	#include "pngstruct.h"
#endif

#ifdef __cplusplus
}
#endif


#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define IS_WIN30_DIB(a)		((*(LPDWORD)(a)) == sizeof(BITMAPINFOHEADER))
#define ADJRGB(a)			(max(0, min((a), 255)))
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

static __inline int do_mix(int src, int dst, int opaque)
{
	if (opaque == 0) return src;
	if (opaque == 255) return dst;
	return ADJRGB((src * (255 - opaque) + dst * opaque) / 255);
}

static int __fastcall dib_numcolors(void* pv)
{
	int colors = ((LPBITMAPINFOHEADER)pv)->biClrUsed;
	if (!colors) {
		int bits = ((LPBITMAPINFOHEADER)pv)->biBitCount;
		colors = (bits <= 8) ? (1 << bits) : 0;
	}
	return colors;
}

static int __fastcall dib_palsize(void* pdib)
{
	int size = dib_numcolors(pdib) * sizeof(RGBQUAD);
	if (((BITMAPINFOHEADER*)pdib)->biCompression == BI_BITFIELDS)
		size += sizeof(DWORD) * 3;
	return size;
}

static BOOL mem2file(LPCTSTR pszName, HANDLE hmem, DWORD size)
{
	BOOL bOK = FALSE;
	if (hmem && size) {
		void* pv = GlobalLock(hmem);
		HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			DWORD dw;
			WriteFile(hf, pv, size, &dw, NULL);
			CloseHandle(hf);
		}
		else
			DeleteFile(pszName);
		GlobalUnlock(hmem);
	}
	return bOK;
}

static HGLOBAL dib_save_mem(HGLOBAL hdib, DWORD* pdwSizeMem)
{
	if (hdib == NULL)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
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

	LPBYTE pb = (LPBYTE)GlobalLock(hmem);
	memcpy(pb, &bfh, sizeof(bfh));
	pb += sizeof(bfh);
	memcpy(pb, pbh, dwSize);

	GlobalUnlock(hmem);
	GlobalUnlock(hdib);
	if (pdwSizeMem)
		*pdwSizeMem = bfh.bfSize;
	return hmem;
}

BOOL dib_save(LPTSTR pszFile, HGLOBAL hdib)
{
	BOOL bOK = FALSE;
	DWORD size;
	HGLOBAL hmem = dib_save_mem(hdib, &size);
	if (hmem) {
		bOK = mem2file(pszFile, hmem, size);
		GlobalFree(hmem);
	}
	return bOK;
}
#ifdef USE_LIBPNG_16
	void readstream(png_structp png_ptr, png_bytep pbuf, png_size_t len)
#else
	void readstream(png_struct* png_ptr, png_bytep pbuf, png_size_t len)
#endif
{
	_mfread((LPMF)png_ptr->io_ptr, pbuf, (DWORD)len);
}

static void __fastcall expand_bits(png_bytep ps, RGBA* pd, int width, int bits, BOOL alpha)
{
	if (bits == 24) {
		for (int x = 0; x < width; x++) {
			pd->red = *ps++;
			pd->green = *ps++;
			pd->blue = *ps++;
			if (alpha)
				pd->alpha = *ps++;
			pd++;
		}
	}
	else if (bits == 8) {
		for (int x = 0; x < width; x++) {
			pd->alpha = *ps++;
			pd++;
		}
	}
	else if (bits == 4) {
		for (int x = 0; x < width; x++) {
			if (x & 1) {
				pd->alpha = (*ps) & 0xf;
				ps++;
			}
			else
				pd->alpha = ((*ps) >> 4) & 0xf;
			pd++;
		}
	}
	else if (bits == 2) {
		for (int x = 0; x < width; x++) {
			pd->alpha = ((*ps) >> ((3 - (x % 4)) * 2)) & 3;
			if ((x % 4) == 3)
				ps++;
			pd++;
		}
	}
	else if (bits == 1) {
		for (int x = 0; x < width; x++) {
			pd->alpha = ((*ps) >> (7 - (x % 8))) & 1;
			if ((x % 8) == 7)
				ps++;
			pd++;
		}
	}
}

HGLOBAL png_load_mem(HGLOBAL hmem)
{
	png_struct* png_ptr;
	png_infop info_ptr;
	LPMF pmf;
	DWORD y;
	png_uint_32 cx, cy;
	int depth, coltype, interlace;
	BYTE head[128];

	pmf = _mfmemopen(hmem, 0);
	if (!pmf)
		return NULL;

	_mfread(pmf, head, sizeof(head));
	if (!png_check_sig(head, sizeof(head))) {
		_mfclose(pmf);
		return NULL;
	}
	_mfseek(pmf, 0, FILE_BEGIN);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		_mfclose(pmf);
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		_mfclose(pmf);
		return NULL;
	}
	png_set_read_fn(png_ptr, pmf, readstream);
	png_read_info(png_ptr, info_ptr);

	// 画像情報の取得
	png_get_IHDR(png_ptr, info_ptr, &cx, &cy, &depth, &coltype, &interlace, 0, 0);

	if (!cx || !cy ||
		(depth != 1 && depth != 2 && depth != 4 && depth != 8 && depth != 16) ||
		(coltype != PNG_COLOR_TYPE_GRAY && coltype != PNG_COLOR_TYPE_GRAY_ALPHA &&
		 coltype != PNG_COLOR_TYPE_PALETTE && coltype != PNG_COLOR_TYPE_RGB &&
		 coltype != PNG_COLOR_TYPE_RGB_ALPHA))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		_mfclose(pmf);
		return NULL;
	}

	if (depth == 16) {
		png_set_strip_16(png_ptr);
		depth = 8;
	}

	BOOL GA2RGBA = FALSE;
	if (coltype == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
		depth = 24;
		coltype = PNG_COLOR_TYPE_RGB_ALPHA;
		GA2RGBA = TRUE;
	}

	// Set the background color to draw transparent and alpha images over.
	// It is possible to set the red, green, and blue components directly
	// for paletted images instead of supplying a palette index.  Note that
	// even if the PNG file supplies a background, you are not required to
	// use it - you should use the (solid) application background if it has one.

	png_color_16 my_background = { 0, 255, 255, 255, 0 };
	if (coltype != PNG_COLOR_TYPE_RGB_ALPHA) {
		png_color_16 *image_background;

		if (png_get_bKGD(png_ptr, info_ptr, &image_background)) {
			png_set_background(png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
			my_background = *image_background;
		}
		else
			png_set_background(png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
	}

	// if this image has transparency, store the trns values

	png_bytep trans               = NULL;
	int num_trans                 = 0;
	png_color_16p trans_values    = NULL;
	png_uint_32 transparent_value = png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values);

	// unlike the example in the libpng documentation, we have *no* idea where
	// this file may have come from--so if it doesn't have a file gamma, don't
	// do any correction ("do no harm")

	double gamma = 0;
	double screen_gamma = 2.2;
	if (png_get_gAMA(png_ptr, info_ptr, &gamma))
		png_set_gamma(png_ptr, screen_gamma, gamma);

	// All transformations have been registered; now update info_ptr data
	png_read_update_info(png_ptr, info_ptr);

	int bits = depth;
	mif image;
	if (coltype == PNG_COLOR_TYPE_PALETTE) {
		image.create(cx, cy, depth == 2 ? 4 : depth);
		png_colorp png_palette;
		png_get_PLTE(png_ptr, info_ptr, &png_palette, &image.m_colors);
		int i;
		for (i = 0; i < image.m_colors; i++) {
			image.m_map[i].red   = png_palette[i].red;
			image.m_map[i].green = png_palette[i].green;
			image.m_map[i].blue  = png_palette[i].blue;
		}
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
			// 透明色処理
			for (i = 0; i < num_trans; i++) {
				image.m_map[i].red   = do_mix(my_background.red,   image.m_map[i].red,   trans[i]);
				image.m_map[i].green = do_mix(my_background.green, image.m_map[i].green, trans[i]);
				image.m_map[i].blue  = do_mix(my_background.blue,  image.m_map[i].blue,  trans[i]);
			}
		}
	}
	else if (coltype == PNG_COLOR_TYPE_RGB ||
			 coltype == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		image.create(cx, cy, 8);		// カラーマップ領域を作成するため
		image.m_bits = (coltype == PNG_COLOR_TYPE_RGB) ? 24 : 32;
		bits = 24;
	}
	else if (coltype == PNG_COLOR_TYPE_GRAY ||
			 coltype == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		image.create(cx, cy, depth == 2 ? 4 : depth);
		image.m_colors = 1 << min(8, max(1, depth));
		for (int i = 0; i < image.m_colors; i++) {
			image.m_map[i].red   =
			image.m_map[i].green =
			image.m_map[i].blue  = (i * 255) / max(image.m_colors - 1, 1);
		}

		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
			; // 透明色処理をここで...
		}
	}

	png_bytepp row_pointers = (png_bytepp)malloc(cy * sizeof(png_bytep));
	for (y = 0; y < cy; y++)
		row_pointers[y] = (png_bytep)malloc(4 * cx);

	png_read_image(png_ptr, row_pointers);

	BOOL alpha = ((coltype & PNG_COLOR_MASK_ALPHA) != 0);
	for (y = 0; y < cy; y++)
		expand_bits(row_pointers[y], image.pos(0, y), cx, bits, alpha);

	for (y = 0; y < cy; y++)
		free(row_pointers[y]);
	free(row_pointers);

	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	if (coltype == PNG_COLOR_TYPE_RGB_ALPHA) {
		RGBA* p = image.m_pixels;
		int num = image.m_packets;
		while (num-- > 0) {
			if (!p->alpha) {
				p->red   = (BYTE)my_background.red;
				p->green = (BYTE)my_background.green;
				p->blue  = (BYTE)my_background.blue;
			}
			p++;
		}
	}

	if (GA2RGBA) {
		for (int i = 0; i < 256; i++) {
			image.m_map[i].red   = 
			image.m_map[i].green = 
			image.m_map[i].blue  = i;
		}

		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
			// 透明色処理をここで...
			int i = 0;
			RGBA* p = image.m_pixels;
			int num = image.m_packets;
			while (num-- > 0) {
				p->alpha = p->red;
				p++;
			}
		}
	}
	_mfclose(pmf);

	return image.createdib();
}

void png2bmp(LPCTSTR pszPng, LPTSTR pszBmp)
{
	DWORD dwSize;
	HGLOBAL hmem = readfileall(pszPng, &dwSize);
	if (!hmem) {
		_tprintf(_T("png2bmp: read file error %s\n"), pszPng);
		return;
	}

	HGLOBAL hdib = png_load_mem(hmem);
	GlobalFree(hmem);
	if (hdib) {
		DWORD size;
		hmem = dib_save_mem(hdib, &size);
		if (hmem) {
			mem2file(pszBmp, hmem, size);
			GlobalFree(hmem);
		}
		else
			_tprintf(_T("png2bmp: save to mem error\n"));
		GlobalFree(hdib);
	}
	else
		_tprintf(_T("png2bmp: load png error %s\n"), pszPng);
}


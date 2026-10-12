/*-
 * pngcode.cpp
 *
 */
#define STRICT
#include "stdafx.h"
#include <time.h>
#include "lsimg.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

#define USE_LIBPNG_16

#ifdef USE_LIBPNG_16
	#include "pngpriv.h"
	#include "pngstruct.h"
#endif
#include "png.h"

#ifdef __cplusplus
}
#endif


#ifdef USE_LIBPNG_16
	void readstream(png_structp png_ptr, png_bytep pbuf, png_size_t len)
#else
	void readstream(png_struct* png_ptr, png_bytep pbuf, png_size_t len)
#endif
{
	_mfread((MemStream*)png_ptr->io_ptr, pbuf, (DWORD)len);
}

static void __fastcall expand_bits(png_bytep ps, RGBA* pd, int width, int bits, bool alpha)
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
	} else if (bits == 8) {
		for (int x = 0; x < width; x++) {
			pd->alpha = *ps++;
			pd++;
		}
	} else if (bits == 4) {
		for (int x = 0; x < width; x++) {
			if (x & 1) {
				pd->alpha = (*ps) & 0xf;
				ps++;
			}
			else
				pd->alpha = ((*ps) >> 4) & 0xf;
			pd++;
		}
	} else if (bits == 2) {
		for (int x = 0; x < width; x++) {
			pd->alpha = ((*ps) >> ((3 - (x % 4)) * 2)) & 3;
			if ((x % 4) == 3)
				ps++;
			pd++;
		}
	} else if (bits == 1) {
		for (int x = 0; x < width; x++) {
			pd->alpha = ((*ps) >> (7 - (x % 8))) & 1;
			if ((x % 8) == 7)
				ps++;
			pd++;
		}
	}
}

static HDIB load_png(MemStream* pms)
{
	png_struct* png_ptr;
	png_infop info_ptr;
	DWORD y;
	png_uint_32 cx, cy;
	int depth, coltype, interlace;
	BYTE head[128];

	_mfread(pms, head, sizeof(head));
	if (!png_check_sig(head, sizeof(head))) {
		return NULL;
	}
	_mfseek(pms, 0, FILE_BEGIN);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	png_set_read_fn(png_ptr, pms, readstream);
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
		png_color_16* image_background;

		if (png_get_bKGD(png_ptr, info_ptr, &image_background)) {
			png_set_background(png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
			my_background = *image_background;
		} else {
			png_set_background(png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
		}
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
	} else if (coltype == PNG_COLOR_TYPE_RGB || coltype == PNG_COLOR_TYPE_RGB_ALPHA) {
		image.create(cx, cy, 8);		// カラーマップ領域を作成するため
		image.m_bits = 24;
		bits = 24;
	} else if (coltype == PNG_COLOR_TYPE_GRAY || coltype == PNG_COLOR_TYPE_GRAY_ALPHA) {
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

	bool alpha = ((coltype & PNG_COLOR_MASK_ALPHA) != 0);
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
			p->red   = do_mix(my_background.red,   p->red,   p->alpha);
			p->green = do_mix(my_background.green, p->green, p->alpha);
			p->blue  = do_mix(my_background.blue,  p->blue,  p->alpha);
			p++;
		}
	}

	if (GA2RGBA) {
		for (int i = 0; i < 256; i++) {
			image.m_map[i].red   = 
			image.m_map[i].green = 
			image.m_map[i].blue  = i;
		}

		RGBA* p = image.m_pixels;
		int num = image.m_packets;
		while (num-- > 0) {
			p->alpha = p->red;
			p++;
		}
		image.m_bits = 8;
	}
	return image.createdib();
}

extern "C"
HDIB WINAPI imgPngLoadMem(HANDLE hmem)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(hmem);
	if (pms) {
		hdib = load_png(pms);
		_mfclose(pms);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgPngLoad(LPCTSTR pszName)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(pszName);
	if (pms) {
		hdib = load_png(pms);
		_mfclose(pms);
	}
	return hdib;
}

#ifdef USE_LIBPNG_16
	void writestream(png_structp png_ptr, png_bytep pbuf, png_size_t len)
#else
	void writestream(png_struct* png_ptr, png_bytep pbuf, png_size_t len)
#endif
{
	_mfwrite((MemStream*)png_ptr->io_ptr, pbuf, (DWORD)len);
}

void flushstream(png_structp png_ptr)
{
}

static HANDLE __fastcall png_save(HDIB hdib, PNG_CPARAMS* ppi, DWORD* pdwSize)
{
	png_struct* png_ptr;
	png_info* info_ptr;
	DWORD y, cx, cy;
	int depth, coltype, width;
	LPBYTE pBase;
	png_bytep* line_ptr;
	LPBITMAPINFO pbi;

	if (!hdib || !pdwSize || !ppi)
		return NULL;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
		return NULL;

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return NULL;
	}

	MemStream* pmf = _mfcreate((DWORD)GlobalSize(hdib));
	if (!pmf) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return NULL;
	}
	png_set_write_fn(png_ptr, pmf, writestream, flushstream);

	// 圧縮レベル 0:非圧縮  1:best_speed  9:smallest_size
	if (ppi->mask & PNGF_COMPRESSIONLEVEL)
		png_set_compression_level(png_ptr, ppi->compression_level);

	// 色深度とカラータイプ
	pbi = dib_lockinfo(hdib);
/*
	if (pbi->bmiHeader.biBitCount == 1) {
		depth = 1;
		coltype = PNG_COLOR_TYPE_GRAY;
	}
	else if (pbi->bmiHeader.biBitCount == 4) {
*/
	if (pbi->bmiHeader.biBitCount <= 4) {
		depth = 4;
		coltype = PNG_COLOR_TYPE_PALETTE;
	}
	else {
		depth = 8;
		if (pbi->bmiHeader.biBitCount > 24)
			coltype = PNG_COLOR_TYPE_RGB_ALPHA;
		else if (pbi->bmiHeader.biBitCount > 8)
			coltype = PNG_COLOR_TYPE_RGB;
		else
			coltype = PNG_COLOR_TYPE_PALETTE;
	}
	cx = pbi->bmiHeader.biWidth;
	cy = pbi->bmiHeader.biHeight;

	png_set_IHDR(png_ptr, info_ptr, cx, cy, depth, coltype,
			(ppi->mask & PNGF_INTERLACE) ?
			((ppi->interlace) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE) : PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// パレットデータの設定
	if (coltype == PNG_COLOR_TYPE_PALETTE) {
		png_color palette[256];
		DWORD colors;

		colors = dib_numcolors(pbi);
		for (y = 0; y < colors; y++) {
			palette[y].red   = pbi->bmiColors[y].rgbRed;
			palette[y].green = pbi->bmiColors[y].rgbGreen;
			palette[y].blue  = pbi->bmiColors[y].rgbBlue;
        }
		png_set_PLTE(png_ptr, info_ptr, palette, colors);
	}

	// 背景色
	if (ppi->mask & PNGF_BKGNDCOLOR) {
		png_color_16 background;
		background.red = GetRValue(ppi->clr_bkgnd);
		background.green = GetGValue(ppi->clr_bkgnd);
		background.blue = GetBValue(ppi->clr_bkgnd);
		png_set_bKGD(png_ptr, info_ptr, &background);
	}

	// 透明色
	if (ppi->mask & PNGF_XPARENTCOLOR && ppi->num_trans > 0) {
		if (coltype == PNG_COLOR_TYPE_RGB || coltype == PNG_COLOR_TYPE_GRAY) {
			png_color_16 xparent;
			xparent.red = GetRValue(ppi->clr_trans);
			xparent.green = GetGValue(ppi->clr_trans);
			xparent.blue = GetBValue(ppi->clr_trans);
			png_set_tRNS(png_ptr, info_ptr, NULL, 1, &xparent);
		}
		else if (coltype == PNG_COLOR_TYPE_PALETTE && ppi->alpha_trans_ptr) {
			png_set_tRNS(png_ptr, info_ptr, ppi->alpha_trans_ptr, ppi->num_trans, NULL);
		}
	}

	// 作成時刻
	if (ppi->mask & PNGF_TIME) {
		time_t cutime = time(NULL);		// time right NOW
		struct tm* gmt = gmtime(&cutime);

		png_time mod_time;
		mod_time.year = gmt->tm_year + 1900;
		mod_time.month = gmt->tm_mon + 1;
		mod_time.day = gmt->tm_mday;
		mod_time.hour = gmt->tm_hour;
		mod_time.minute = gmt->tm_min;
		mod_time.second = gmt->tm_sec;
		png_set_tIME(png_ptr, info_ptr, &mod_time);
	}

	png_write_info(png_ptr, info_ptr);

	// BGR->RGBへ
	png_set_bgr(png_ptr);

	// 画像データ読み込み
	pBase = dib_findbits(pbi);
	width = WIDTHBYTES(cx * pbi->bmiHeader.biBitCount);
	line_ptr = (png_bytep*)malloc(cy * sizeof(png_bytep));
	for (y = 0; y < cy; y++)
		line_ptr[y] = pBase + (cy - y - 1) * width;
	png_write_image(png_ptr, line_ptr);
	free(line_ptr);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	dib_unlock(hdib);
	size_t size;
	HGLOBAL hmem = _mfdetach(pmf, &size);
	*pdwSize = (DWORD)size;
	return hmem;
}

extern "C"
HANDLE WINAPI imgPngSaveMem(HDIB hdib, DWORD* pdwSize)
{
	PNG_CPARAMS cpi;
	cpi.mask = 0;
	return png_save(hdib, &cpi, pdwSize);
}

extern "C"
BOOL WINAPI imgPngSave(LPCTSTR pszName, HDIB hdib)
{
	DWORD size;
	HANDLE hmem = imgPngSaveMem(hdib, &size);
	return SaveMemToFileAndWep(pszName, hmem, size);
}

extern "C"
HANDLE WINAPI imgPngSaveMemParams(HDIB hdib, PNG_CPARAMS* pcpi, DWORD* pdwSize)
{
	return png_save(hdib, pcpi, pdwSize);
}

extern "C"
BOOL WINAPI imgPngSaveParams(LPCTSTR pszName, HDIB hdib, PNG_CPARAMS* pcpi)
{
	DWORD size;
	HANDLE hmem = png_save(hdib, pcpi, &size);
	return SaveMemToFileAndWep(pszName, hmem, size);
}

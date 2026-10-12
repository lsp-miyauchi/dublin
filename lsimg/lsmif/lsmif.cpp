//////////////////////////////////////////////////////////////////////
// lspmif.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "dib_sup.h"
#include "common.h"
#include "palette_table.h"
#include "colorspace.h"


BOOL adjust_rgb(mif& src, LPRECT prc, int red, int green, int blue);
BOOL bright_contrast(mif& src, LPRECT prc, int brightness, int contrast);
BOOL bumpmap(mif& src, LPRECT prc, MIF_BUMPMAP* pbp);
BOOL bumpmap_merge(mif& src, mif& bmp, POINT pt, MIF_BUMPMAP* pbp);
BOOL autocontrast(mif& image, LPRECT prc);
void blur(mif& src, LPRECT prc, int x_factor, int y_factor);
BOOL blur_simple(mif& src, LPRECT prc, int radius);
BOOL blur_gauss_rle(mif& src, LPRECT prc, double radius);
BOOL blur_gauss_rle_plane(mif& msrc, LPRECT prc, double radius, UINT plane);
BOOL blur_gauss_iir(mif& src, LPRECT prc, double horz, double vert);
BOOL blur_gauss_sel(mif& image, LPRECT prc, double radius, int maxdelta);
BOOL gamma_rgb(mif& src, LPRECT prc, double r_amt, double g_amt, double b_amt, double rgb_amt);
BOOL modulate(mif& src, LPRECT prc, int per_h, int per_s, int per_l);
BOOL labmodulate(mif& src, LPRECT prc, int per_L, int per_a, int per_b);
BOOL hue_saturation(mif& src, LPRECT prc, int hue_range, int per_h, int per_s, int per_l);
BOOL autocolorenhance(mif& src, LPRECT prc);
BOOL colorbalance(mif& image, LPRECT prc, MIF_COLORBALANCE* pcb);
BOOL colorify(mif& image, LPRECT prc, COLORREF rgb);
BOOL colorcut(mif& src, LPRECT prc, MIF_COLORCUT* pcp);
DWORD countcolors(mif& image);
BOOL edge(mif& src, LPRECT prc, double amount);
BOOL emboss(mif& src, LPRECT prc, MIF_EMBOSS* pep);
BOOL enhance(mif& src, LPRECT prc, int threshold);
BOOL equalize(mif& src, LPRECT prc);
BOOL fadeinrect(mif& image, LPRECT prc, int delta, COLORREF cr);
BOOL fadepaste(mif& src, mif& dst, int delta, int xpos, int ypos);
BOOL filter_formal3x3(mif& src, LPRECT prc, UINT nType);
BOOL filter_custom3x3(mif& src, LPRECT prc, int* pFilter, int nDivide);
BOOL mblur_linear(mif& src, LPRECT prc, int length, int angle);
BOOL mblur_radial(mif& src, LPRECT prc, int angle);
BOOL mblur_zoom(mif& src, LPRECT prc, int length);
BOOL mblur_round(mif& src, LPRECT prc, int nRotate);
BOOL addnoise(mif& src, LPRECT prc, int noise_type);
BOOL normalize(mif& m, LPRECT prc);
BOOL oilpaint(mif& src, LPRECT prc, int rad);
BOOL pixelize(mif& src, LPRECT prc, int xblock, int yblock, BOOL renga, BOOL cross);
BOOL posterize(mif& src, LPRECT prc, int levels);
BOOL pyramid(mif& src, LPRECT prc, int block);
void quantize(mif& image, int dither_type, int numcolors, LOGPALETTE* plgp);
void quantize_simple(mif& image, int num_colors, BOOL cie_luv, BOOL principal);
BOOL raise_edge(mif& src, LPRECT prc, int delta);
BOOL scroll(mif& src, int sx, int sy, BOOL bWrap);
BOOL sharpen2(mif& src, LPRECT prc, int per);
BOOL spread(mif& src, LPRECT prc, int amount);
BOOL sblur_wind(mif& src, LPRECT prc, int threshold, int strength, int direction, int edge);
BOOL sblur_blast(mif& src, LPRECT prc, int threshold, int strength, int direction, int edge);
BOOL tile_divide(mif& src, LPRECT prc, int divide);
BOOL tile(mif& src, LPRECT prc, int xblock, int yblock);
BOOL texturize(mif& src, LPRECT prc, char* texture, int twidth, int theight, int dir, int depth);
BOOL texturize_by_file(mif& src, LPRECT prc, LPCSTR txrfile, int direction, int depth);
BOOL texturize_by_resource(mif& src, LPRECT prc, HINSTANCE hInst, UINT typeResource, UINT idResource, int direction, int depth);
BOOL texture_to_file(mif& src, LPCSTR title, LPCSTR txrfile, BOOL gray);
BOOL zoom_filter(mif& src, int cx, int cy, int method, BOOL ondisk);
BOOL zoom_pixel(mif& src, int cx, int cy);
BOOL zoom_near(mif& src, int cx, int cy);
BOOL zoom_linear(mif& src, int cx, int cy);
BOOL zoom_inframe(mif& src, int cx, int cy);
BOOL unsharp_mask(mif& src, LPRECT prc, MIF_UNSHARPMASK* pup);
BOOL unsharp_mask2(mif& src, LPRECT prc, double radius, double depth);
BOOL video(mif& src, LPRECT prc, MIF_VIDEO* pvp);

mif::mif()
{
	clearparam();
}

mif::mif(int width, int height, int bits)
{
	clearparam();
	create(width, height, bits);
}

mif::mif(mif& src)
{
	clearparam();
	copyfrom(&src);
}

mif::mif(HANDLE hdib)
{
	clearparam();
	attachdib(hdib);
}

mif::mif(HBITMAP hbm, HPALETTE hpal)
{
	clearparam();
	attachbitmap(hbm, hpal);
}

void mif::destroy()
{
	if (m_map)
		free(m_map);

	if (m_mapfile)
		delete m_mapfile;
	else if (m_pixels)
		free(m_pixels);
	clearparam();
}

void mif::clearparam()
{
	m_width = 0;
	m_height = 0;
	m_bits = 0;
	m_colors = 0;
	m_packets = 0;
	m_map = NULL;
	m_pixels = NULL;
	m_packets = 0;
	m_mapfile = NULL;
	m_progress = NULL;
}

BOOL mif::create(int width, int height, int bits)
{
	destroy();

	m_width = width;
	m_height = height;
	m_bits = bits;
	m_packets = width * height;

	if (m_bits < 24)
		alloccmap(1 << m_bits);

	m_pixels = (RGBA*)malloc(m_packets * sizeof(RGBA));
	if (!m_pixels)
	{
		destroy();
		return FALSE;
	}
	memset(m_pixels, 0, m_packets * sizeof(RGBA));
	return TRUE;
}

BOOL mif::create_width_mapfile(int width, int height, int bits)
{
	destroy();

	m_width = width;
	m_height = height;
	m_bits = bits;
	m_packets = width * height;

	if (m_bits < 24)
		alloccmap(1 << m_bits);

	m_mapfile = new mapfile();
	m_pixels = (RGBA*)m_mapfile->create(m_packets * sizeof(RGBA));
	if (!m_pixels) {
		delete m_mapfile;
		m_mapfile = NULL;

		m_pixels = (RGBA*)malloc(m_packets * sizeof(RGBA));
		if (!m_pixels) {
			destroy();
			return FALSE;
		}
		memset(m_pixels, 0, m_packets * sizeof(RGBA));
	}
	return TRUE;
}

BOOL mif::_attachdib(HANDLE hdib, BOOL ondisk)
{
	if (!hdib)
		return FALSE;

	// エンコードされている画像は対象外
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	if (pbh->biCompression != BI_RGB) {
		GlobalUnlock(hdib);
		return FALSE;
	}

	int bits = (pbh->biBitCount > 8) ? 24 : ((pbh->biBitCount == 2) ? 4 : pbh->biBitCount);
	if (ondisk) {
		if (!create_width_mapfile(pbh->biWidth, pbh->biHeight, bits))
			return FALSE;
	}
	else {
		if (!create(pbh->biWidth, pbh->biHeight, bits))
			return FALSE;
	}

	if (bits <= 8) {
		// インデクスカラーのコピー
		LPBITMAPINFO pbi = (LPBITMAPINFO)pbh;
		m_colors = pbh->biClrUsed ? pbh->biClrUsed : 1 << pbh->biBitCount;
		m_colors = min(256, m_colors);

		for (int x = 0; x < m_colors; x++) {
			m_map[x].red = pbi->bmiColors[x].rgbRed;
			m_map[x].green = pbi->bmiColors[x].rgbGreen;
			m_map[x].blue = pbi->bmiColors[x].rgbBlue;
		}
	}

	BYTE* base = dib_findbits(pbh);
	int widthbytes = WIDTHBYTES(m_width * pbh->biBitCount);
	DWORD mr, mg, mb;
	DWORD sr, sg, sb;

	// If not using BI_BITFIELDS then set masks to default values
	if (pbh->biBitCount == 32 || pbh->biBitCount == 16) {
		if (pbh->biCompression == BI_RGB) {
			if (pbh->biBitCount == 16) {
				mr = MAKE555WORD(0xFF, 0, 0);   // Make 5, 5, 5 mask
				mg = MAKE555WORD(0, 0xFF, 0);
				mb = MAKE555WORD(0, 0, 0xFF);
			}
			else {
				mr = 0xFF0000;                  // Make 8, 8, 8 mask
				mg = 0x00FF00;
				mb = 0x0000FF;
			}
		}
		else {
			DWORD* pmask = (DWORD*)((BYTE*)pbh + (WORD)pbh->biSize);
			mr = pmask[0]; 						// Use the specified mask
			mg = pmask[1];
			mb = pmask[2];
		}

		sr = mr;
		while (!(sr & 1)) sr >>= 1;
		sg = mg;
		while (!(sg & 1)) sg >>= 1;
		sb = mb;
		while (!(sb & 1)) sb >>= 1;
	}

	if (pbh->biBitCount == 32) {
		// 32bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + widthbytes * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				DWORD dw = *(DWORD*)ps;
				pixels->red = (BYTE)((dw & mr) >> sr);
				pixels->green = (BYTE)((dw & mg) >> sg);
				pixels->blue = (BYTE)((dw & mb) >> sb);
				ps += sizeof(DWORD);
				pixels++;
			}
		});
	}
	else if (pbh->biBitCount == 24) {
		// 24bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + widthbytes * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				pixels->blue = *ps++;
				pixels->green = *ps++;
				pixels->red = *ps++;
				pixels++;
			}
		});
	}
	else if (pbh->biBitCount == 16) {
		// 16bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + widthbytes * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				WORD w = *(WORD*)ps;
				pixels->red = (BYTE)((w & mr) >> sr);
				pixels->green = (BYTE)((w & mg) >> sg);
				pixels->blue = (BYTE)((w & mb) >> sb);
				ps += sizeof(WORD);
				pixels++;
			}
		});
	}
	else if (pbh->biBitCount == 8) {
		// 8bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + widthbytes * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				pixels->alpha = *ps++;
				pixels->red = m_map[pixels->alpha].red;
				pixels->green = m_map[pixels->alpha].green;
				pixels->blue = m_map[pixels->alpha].blue;
				pixels++;
			}
		});
	}
	else if (pbh->biBitCount == 4) {
		// 4bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + widthbytes * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				if (x & 1) {
					pixels->alpha = (*ps) & 0xf;
					ps++;
				}
				else
					pixels->alpha = ((*ps) >> 4) & 0xf;
				pixels->red = m_map[pixels->alpha].red;
				pixels->green = m_map[pixels->alpha].green;
				pixels->blue = m_map[pixels->alpha].blue;
				pixels++;
			}
		});
	}
	else if (pbh->biBitCount == 2) {
		// 2bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + widthbytes * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				pixels->alpha = ((*ps) >> ((3 - (x % 4)) * 2)) & 3;
				pixels->red = m_map[pixels->alpha].red;
				pixels->green = m_map[pixels->alpha].green;
				pixels->blue = m_map[pixels->alpha].blue;
				if ((x % 4) == 3)
					ps++;
				pixels++;
			}
		});
	}
	else if (pbh->biBitCount == 1) {
		// 1bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + widthbytes * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				pixels->alpha = ((*ps) >> (7 - (x % 8))) & 1;
				pixels->red = m_map[pixels->alpha].red;
				pixels->green = m_map[pixels->alpha].green;
				pixels->blue = m_map[pixels->alpha].blue;
				if ((x % 8) == 7)
					ps++;
				pixels++;
			}
		});
	}

	GlobalUnlock(hdib);
	return TRUE;
}

BOOL mif::attachdib(HANDLE hdib)
{
	return _attachdib(hdib, FALSE);
}

BOOL mif::attachdib_width_mapfile(HANDLE hdib)
{
	return _attachdib(hdib, TRUE);
}

BOOL mif::attachdib32(int width, int height, void* pixels)
{
	if (!pixels || !create(width, height, 24))
		return FALSE;

	parallel_for (0, height, [&](int y) {
		COLORREF* p = (COLORREF*)pixels + (y * m_width);
		RGBA* q = pos(0, height - y - 1);
		int cnt = m_width;
		while (cnt-- > 0) {
			q->red = GetRValue(*p);
			q->green = GetGValue(*p);
			q->blue = GetBValue(*p);
			p++;
			q++;
		}
	});
	return TRUE;
}

BOOL mif::attachbitmap(HBITMAP hbm, HPALETTE hpal)
{
	BOOL bOK = FALSE;
	HANDLE h = dib_createfrombitmap(hbm, hpal);
	if (h) {
		bOK = attachdib(h);
		GlobalFree(h);
	}
	return bOK;
}

BOOL mif::copyto(mif* copyto)
{
	if (!copyto->create(m_width, m_height, m_bits))
		return FALSE;

	copyto->m_colors = m_colors;
	if (m_map)
		memcpy(copyto->m_map, m_map, m_colors * sizeof(RGBA));
	if (m_pixels)
		memcpy(copyto->m_pixels, m_pixels, m_packets * sizeof(RGBA));
	return TRUE;
}

BOOL mif::copyfrom(mif* copyfrom)
{
	if (!create(copyfrom->m_width, copyfrom->m_height, copyfrom->m_bits))
		return FALSE;

	m_colors = copyfrom->m_colors;
	if (copyfrom->m_map)
		memcpy(m_map, copyfrom->m_map, copyfrom->m_colors * sizeof(RGBA));
	if (copyfrom->m_pixels)
		memcpy(m_pixels, copyfrom->m_pixels, copyfrom->m_packets * sizeof(RGBA));
	return TRUE;
}

BOOL mif::movefrom(mif* movefrom)
{
	if (!movefrom)
		return FALSE;

	destroy();
	m_width = movefrom->m_width;
	m_height = movefrom->m_height;
	m_bits = movefrom->m_bits;
	m_colors = movefrom->m_colors;
	m_map = movefrom->m_map;
	m_packets = movefrom->m_packets;
	m_pixels = movefrom->m_pixels;
	m_mapfile = movefrom->m_mapfile;

	movefrom->clearparam();
	return TRUE;
}

void mif::alloccmap(int colors)
{
	if (m_map)
		free(m_map);
	m_map = NULL;
	m_colors = colors;
	if (colors > 0) {
		m_map = (RGBA*)malloc(colors * sizeof(RGBA));
		memset(m_map, 0, colors * sizeof(RGBA));
	}
}

void mif::blt(int dx, int dy, int width, int height, mif* src, int sx, int sy)
{
	if (!src ||
		width < 0 || height < 0 ||
		dx >= m_width || dy >= m_height ||
		dx + width < 0 || dy + height < 0 ||
		sx >= src->m_width || sy >= src->m_height)
		return;

	parallel_for (0, height, [&](int y) {
		if (sy + y >= 0 && sy + y < src->m_height &&
			dy + y >= 0 && dy + y < m_height) {
			RGBA* ps = src->pos(sx, sy + y);
			RGBA* pd = pos(dx, dy + y);
			for (int x = 0; x < width; x++) {
				if (sx + x >= 0 && sx + x < src->m_width && dx + x >= 0 && dx + x < m_width)
					*pd = *ps;
				ps++;
				pd++;
			}
		}
	});
}

void mif::blt(int dx, int dy, int width, int height, HANDLE hdib, int sx, int sy)
{
	mif dib(hdib);
	blt(dx, dy, width, height, &dib, sx, sy);
}

void mif::increasecolor(int bits)
{
	if (bits <= 4)
		bits = 4;
	else if (bits <= 8)
		bits = 8;
	else
		bits = 24;

	// 指定されたビットカウントが現在のDIBのビットカウント以下ならば何もしない
	if (m_bits >= bits)
		return;

	m_bits = bits;
	if (m_bits == 24) {
		if (m_map) {
			m_colors = 0;
			free(m_map);
			m_map = NULL;
		}
	}
}

static _inline unsigned char applyalpha(int v, int a, int b)
{
	int n = (int)ceil((double)v * (double)a / 255.0 + (double)b * (double)(255 - a) / 255.0);
	return (unsigned char)min(255, n);
}

static _inline void alphablend(RGBA& pixel, COLORREF bkcolor)
{
	pixel.red = applyalpha(pixel.red, pixel.alpha, GetRValue(bkcolor));
	pixel.green = applyalpha(pixel.green, pixel.alpha, GetGValue(bkcolor));
	pixel.blue = applyalpha(pixel.blue, pixel.alpha, GetBValue(bkcolor));
}

#ifndef CLR_NONE
#define CLR_NONE                0xFFFFFFFFL
#define CLR_DEFAULT             0xFF000000L
#endif

void mif::applyalpha(COLORREF bkcolor = CLR_NONE)
{
	if (bkcolor == CLR_NONE || bkcolor == CLR_DEFAULT) {
		bkcolor = RGB(0, 0, 0);
	}
	/* 32bit alpha -> 24bit */
	if (m_bits == 32) {
		m_bits = 24;
		RGBA* p = m_pixels;
		for (int i = 0; i < m_packets; i++) {
			alphablend(*p++, bkcolor);
		}
	}
}

HANDLE mif::createdib()
{
	if (!m_pixels)
		return NULL;

	HANDLE hdib = dib_create(m_width, m_height, m_bits);
	if (!hdib)
		return NULL;

	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	if (m_bits < 24) {
		LPBITMAPINFO pbi = (LPBITMAPINFO)pbh;
		pbh->biClrUsed = m_colors;
		for (int x = 0; x < m_colors; x++) {
			pbi->bmiColors[x].rgbRed = m_map[x].red;
			pbi->bmiColors[x].rgbGreen = m_map[x].green;
			pbi->bmiColors[x].rgbBlue = m_map[x].blue;
		}
	}

	int width = WIDTHBYTES(m_width * m_bits);
	BYTE* base = dib_findbits(pbh);

	if (m_bits == 32) {
		// 32bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + width * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				*ps++ = pixels->blue;
				*ps++ = pixels->green;
				*ps++ = pixels->red;
				*ps++ = pixels->alpha;
				pixels++;
			}
		});
	}
	else if (m_bits == 24) {
		// 24bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + width * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				*ps++ = pixels->blue;
				*ps++ = pixels->green;
				*ps++ = pixels->red;
				pixels++;
			}
		});
	}
	else if (m_bits == 8) {
		// 8bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + width * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				*ps++ = pixels->alpha;
				pixels++;
			}
		});
	}
	else if (m_bits == 4) {
		// 4bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + width * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				if (x & 1)
					*ps++ |= pixels->alpha & 0x0f;
				else
					*ps = (pixels->alpha << 4) & 0xf0;
				pixels++;
			}
		});
	}
	else if (m_bits == 1) {
		// 1bit
		parallel_for (0, m_height, [&](int y) {
			RGBA* pixels = pos(0, y);
			BYTE* ps = base + width * (m_height - y - 1);
			for (int x = 0; x < m_width; x++) {
				if ((x & 7) == 0)
					*ps = 0;
				*ps |= (pixels->alpha & 1) << (7 - (x & 7));
				if ((x & 7) == 7)
					ps++;
				pixels++;
			}
		});
	}

	GlobalUnlock(hdib);
	return hdib;
}

HBITMAP mif::createbitmap(HPALETTE* phpal)
{
	HPALETTE hpal = NULL;
	HBITMAP hbm = NULL;

	HANDLE hdib = createdib();
	if (hdib) {
		LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
		LPBYTE pb = dib_findbits(pbh);
		hpal = dib_createpalette(hdib);
		HDC hdc = GetDC(NULL);
		HPALETTE hpw = NULL;
		if (hpal) {
			hpw = SelectPalette(hdc, hpal, FALSE);
			RealizePalette(hdc);
		}
		hbm = CreateDIBitmap(hdc, pbh, CBM_INIT, pb, (BITMAPINFO*)pbh, DIB_RGB_COLORS);
		if (!hbm) {
			// Windows Meでは16M以上のビットマップはCreateDIBitmapでは作成できない
			void* pv;
			hbm = CreateDIBSection(hdc, (BITMAPINFO*)pbh, DIB_RGB_COLORS, &pv, NULL, 0);
			if (hbm)
				SetDIBits(hdc, hbm, 0, pbh->biHeight, pb, (BITMAPINFO*)pbh, DIB_RGB_COLORS);
		}
		if (hpal)
			SelectPalette(hdc, hpw, FALSE);
		GlobalUnlock(hdib);
		ReleaseDC(NULL, hdc);
		GlobalFree(hdib);
	}
	if (phpal)
		*phpal = hpal;
	else if (hpal)
		DeletePalette(hpal);
	return hbm;
}

LOGPALETTE* mif::createlogpalette()
{
	if (!m_map || !m_colors)
		return NULL;

	LOGPALETTE* plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * m_colors);
	plp->palVersion = 0x300;
	plp->palNumEntries = m_colors;
	PALETTEENTRY* ppe = plp->palPalEntry;
	for (int i = 0; i < m_colors; i++) {
		ppe->peRed = m_map[i].red;
		ppe->peGreen = m_map[i].green;
		ppe->peBlue = m_map[i].blue;
		ppe->peFlags = 0;
		ppe++;
	}
	return plp;
}

HPALETTE mif::createpalette()
{
	HPALETTE hpal = NULL;
	LOGPALETTE* plp = createlogpalette();
	if (plp) {
		hpal = CreatePalette(plp);
		free(plp);
	}
	return hpal;
}

BOOL mif::replace_palette(HPALETTE hpal)
{
	if (!hpal)
		return FALSE;

	WORD dst_colors;
	GetObject(hpal, sizeof(WORD), &dst_colors);
	if (m_colors != dst_colors)
		return FALSE;

	PALETTEENTRY* ppe = (PALETTEENTRY*)malloc(sizeof(PALETTEENTRY) * dst_colors);
	GetPaletteEntries(hpal, 0, dst_colors, ppe);
	for (int i = 0; i < m_colors; i++) {
		m_map[i].red = ppe[i].peRed;
		m_map[i].green = ppe[i].peGreen;
		m_map[i].blue = ppe[i].peBlue;
	}
	free(ppe);
	validate_rgb();
	return TRUE;
}


BOOL mif::addlayer(mif* lay, int x, int y, int blendtype, int opaque)
{
	CRect rcs(0, 0, m_width, m_height);
	CRect rcd(x, y, lay->m_width + x, lay->m_height + y);
	rcs &= rcd;
	if (rcs.IsRectEmpty())
		return FALSE;

	BLENDFUNC pfn = get_blendfunction(blendtype);
	parallel_for (rcs.top, rcs.bottom, [&](int sy) {
		RGBA* ps = pos(rcs.left, sy);
		RGBA* pd = lay->pos(rcs.left - x, sy - y);
		for (int sx = rcs.left; sx < rcs.right; sx++) {
			if (pd->alpha) {
				RGBA pix = (*pfn)(ps, pd);
				pix = blend_pixel(ps, &pix, pd->alpha);
				*ps = blend_pixel(ps, &pix, opaque);
			}
			ps++;
			pd++;
		}
	});
	return TRUE;
}

BOOL mif::desaturate(LPRECT prc)
{
	if (m_bits < 24) {
		int i = m_colors;
		RGBA* p = m_map;
		while (i-- > 0) {
			int v = rgba_to_l(p);
			p->red = p->green = p->blue = v;
			p++;
		}
		validate_rgb();
	}
	else {
		CRect rc;
		if (!validaterect(prc, &rc))
			return FALSE;

		int cx = rc.Width();
		int cy = rc.Height();

		parallel_for (0, cy, [&](int y) {
			RGBA* p = pos(rc.left, y + rc.top);
			int i = cx;
			while (i-- > 0) {
				int v = rgba_to_l(p);
				p->red = p->green = p->blue = v;
				p++;
			}
		});
	}
	return TRUE;
}

void mif::grayscale()
{
	register RGBA* p = m_pixels;
	register int cnt = m_packets;
	int gray;
	while (cnt-- > 0) {
		gray = rgb_to_l(p->red, p->green, p->blue);
		p->red = p->green = p->blue = p->alpha = gray;
		p++;
	}

	alloccmap(256);
	m_bits = 8;
	p = m_map;
	cnt = 255;
	while (cnt >= 0) {
		p->red = p->green = p->blue = 255 - cnt;
		p++;
		cnt--;
	}
}

void mif::validate_rgb()
{
	if (m_bits < 24) {
		register RGBA* p = m_pixels;
		register int cnt = m_packets;
		while (cnt-- > 0) {
			p->red = m_map[p->alpha].red;
			p->green = m_map[p->alpha].green;
			p->blue = m_map[p->alpha].blue;
			p++;
		}
	}
}

void mif::solidfill(int sx, int sy, int width, int height, COLORREF rgb, int opaque, int blendmode)
{
	if (width < 1 || height < 1 || opaque == 0)
		return;

	if (m_bits < 24)
		m_bits = 24;

	int ex = sx + width;
	int ey = sy + height;

	sx = min(m_width, max(sx, 0));
	sy = min(m_height, max(sy, 0));
	ex = min(ex, m_width);
	ey = min(ey, m_height);

	width = ex - sx;
	height = ey - sy;
	if (width < 1 || height < 1)
		return;

	BLENDFUNC pfn = get_blendfunction(blendmode);
	RGBA pix = { GetRValue(rgb), GetGValue(rgb), GetBValue(rgb), 0 };
	parallel_for (sy, ey, [&](int y) {
		RGBA* p = pos(sx, y);
		for (int x = sx; x < ex; x++) {
			RGBA pixw = (*pfn)(p, &pix);
			blend_to(p, &pixw, opaque);
			p++;
		}
	});
}

void mif::solidfill(LPRECT prc, COLORREF rgb, int opaque, int blendmode)
{
	CRect rc;
	if (prc && !IsRectEmpty(prc))
		rc.CopyRect(prc);
	else
		rc.SetRect(0, 0, m_width, m_height);
	solidfill(rc.left, rc.top, rc.Width(), rc.Height(), rgb, opaque, blendmode);
}

void mif::fillsolidrect(int sx, int sy, int width, int height, COLORREF rgb, int opaque)
{
	solidfill(sx, sy, width, height, rgb, opaque);
}

void mif::fillsolidrect(LPRECT prc, COLORREF rgb, int opaque)
{
	CRect rc;
	if (prc && !IsRectEmpty(prc))
		rc.CopyRect(prc);
	else
		rc.SetRect(0, 0, m_width, m_height);
	solidfill(rc.left, rc.top, rc.Width(), rc.Height(), rgb, opaque);
}

void mif::shadefill(int sx, int sy, int width, int height, COLORREF* rgbs, int colors, int bounds, int opaque, int blendmode)
{
	if (width < 1 || height < 1 || opaque == 0)
		return;
	m_bits = 24;

	BOOL swap = FALSE;
	if (bounds == strokeUp) {
		bounds = strokeDown;
		swap = TRUE;
	}
	else if (bounds == strokeUpLeft) {
		bounds = strokeDownRight;
		swap = TRUE;
	}
	else if (bounds == strokeLeft) {
		bounds = strokeRight;
		swap = TRUE;
	}
	else if (bounds == strokeDownLeft) {
		bounds = strokeUpRight;
		swap = TRUE;
	}

	COLORREF rgbw[16];
	memcpy(rgbw, rgbs, colors * sizeof(COLORREF));
	if (swap) {
		for (int i = 0; i < colors; i++)
			rgbw[i] = rgbs[colors - i - 1];
	}

	PALETTEENTRY tbl[512];
	grad_table(rgbw, colors, tbl, 512);

	BLENDFUNC pfn = get_blendfunction(blendmode);

	int x_div = (width > 1) ? width - 1 : 1;
	int y_div = (height > 1) ? height - 1 : 1;
	if (bounds == strokeDown) {
		parallel_for (0, height, [&](int y) {
			RGBA* p = pos(sx, sy + y);
			int r = y * 511 / y_div;
			for (int x = 0; x < width; x++) {
				RGBA pix = (*pfn)(p, (RGBA*)&tbl[r]);
				blend_to(p, &pix, opaque);
				p++;
			}
		});
	}
	else if (bounds == strokeRight) {
		parallel_for (0, width, [&](int x) {
			RGBA* p = pos(sx + x, sy);
			int r = x * 511 / x_div;
			for (int y = 0; y < height; y++) {
				RGBA pix = (*pfn)(p, (RGBA*)&tbl[r]);
				blend_to(p, &pix, opaque);
				p += m_width;
			}
		});
	}
	else if (bounds == strokeDownRight) {
		parallel_for (0, height, [&](int y) {
			RGBA* p = pos(sx, sy + y);
			int yr = y * 511 / y_div;
			for (int x = 0; x < width; x++) {
				int xr = x * 511 / x_div;
				int r = (xr + yr) >> 1;
				RGBA pix = (*pfn)(p, (RGBA*)&tbl[r]);
				blend_to(p, &pix, opaque);
				p++;
			}
		});
	}
	else if (bounds == strokeUpRight) {
		parallel_for (0, height, [&](int y) {
			RGBA* p = pos(sx, sy + y);
			int yr = (height - y - 1) * 511 / y_div;
			for (int x = 0; x < width; x++) {
				int xr = x * 511 / x_div;
				int r = (xr + yr) >> 1;
				RGBA pix = (*pfn)(p, (RGBA*)&tbl[r]);
				blend_to(p, &pix, opaque);
				p++;
			}
		});
	}
}

void mif::shadefill(LPRECT prc, COLORREF* rgbs, int colors, int bounds, int opaque, int blendmode)
{
	CRect rc;
	if (prc && !IsRectEmpty(prc))
		rc.CopyRect(prc);
	else
		rc.SetRect(0, 0, m_width, m_height);
	shadefill(rc.left, rc.top, rc.Width(), rc.Height(), rgbs, colors, bounds, opaque, blendmode);
}

void mif::fillshaderect(int sx, int sy, int width, int height, COLORREF* rgbs, int colors, int bounds, int opaque)
{
	shadefill(sx, sy, width, height, rgbs, colors, bounds, opaque);
}

void mif::fillshaderect(LPRECT prc, COLORREF* rgbs, int colors, int bounds, int opaque)
{
	CRect rc;
	if (prc && !IsRectEmpty(prc))
		rc.CopyRect(prc);
	else
		rc.SetRect(0, 0, m_width, m_height);
	shadefill(rc.left, rc.top, rc.Width(), rc.Height(), rgbs, colors, bounds, opaque);
}

LOGPALETTE* mif::make_gradtable(GRADCOLORS* pcol, int colors, int numtables)
{
	if (!pcol)
		return NULL;

	GRADCOLORS* pgc = new GRADCOLORS[colors + 2];
	if (pcol[0].pos > 0) {
		pgc[0].pos = 0;
		pgc[0].cr = pcol[0].cr;
		CopyMemory(&pgc[1], &pcol[0], sizeof(GRADCOLORS) * colors);
		colors++;
	}
	else
		CopyMemory(&pgc[0], &pcol[0], sizeof(GRADCOLORS) * colors);

	if (pgc[colors - 1].pos < 1000) {
		pgc[colors].pos = 1000;
		pgc[colors].cr = pgc[colors - 1].cr;
		colors++;
	}
	LOGPALETTE* plgp = grad_table_ex(pgc, colors, numtables);
	delete [] pgc;
	return plgp;
}

void mif::free_gradtable(LOGPALETTE* plgp)
{
	if (plgp)
		free(plgp);
}

void mif::gradientfill(int sx, int sy, int width, int height, GRADCOLORS* pcol, int colors, int bounds, int opaque, int blendmode)
{
	if (!pcol || width < 1 || height < 1 || colors < 1 || opaque == 0)
		return;

	if (colors == 1) {
		solidfill(sx, sy, width, height, pcol->cr, opaque, blendmode);
		return;
	}

	LOGPALETTE* plgp = make_gradtable(pcol, colors, 1000);
	if (!plgp)
		return;

	m_bits = 24;
	BOOL swap = FALSE;
	if (bounds == strokeUp) {
		bounds = strokeDown;
		swap = TRUE;
	}
	else if (bounds == strokeUpLeft) {
		bounds = strokeDownRight;
		swap = TRUE;
	}
	else if (bounds == strokeLeft) {
		bounds = strokeRight;
		swap = TRUE;
	}
	else if (bounds == strokeDownLeft) {
		bounds = strokeUpRight;
		swap = TRUE;
	}

	if (swap) {
		int middle = plgp->palNumEntries / 2;
		PALETTEENTRY tmp;
		for (int i = 0; i < middle; i++) {
			tmp = plgp->palPalEntry[i];
			plgp->palPalEntry[i] = plgp->palPalEntry[plgp->palNumEntries - i - 1];
			plgp->palPalEntry[plgp->palNumEntries - i - 1] = tmp;
		}
	}

	BLENDFUNC pfn = get_blendfunction(blendmode);

	int x_div = (width > 1) ? width - 1 : 1;
	int y_div = (height > 1) ? height - 1 : 1;
	if (bounds == strokeDown) {
		parallel_for (0, height, [&](int y) {
			RGBA* p = pos(sx, sy + y);
			int r = y * 999 / y_div;
			for (int x = 0; x < width; x++) {
				RGBA pix = (*pfn)(p, (RGBA*)&plgp->palPalEntry[r]);
				blend_to(p, &pix, opaque);
				p++;
			}
		});
	}
	else if (bounds == strokeRight) {
		parallel_for (0, width, [&](int x) {
			RGBA* p = pos(sx + x, sy);
			int r = x * 999 / x_div;
			for (int y = 0; y < height; y++) {
				RGBA pix = (*pfn)(p, (RGBA*)&plgp->palPalEntry[r]);
				blend_to(p, &pix, opaque);
				p += m_width;
			}
		});
	}
	else if (bounds == strokeDownRight) {
		parallel_for (0, height, [&](int y) {
			RGBA* p = pos(sx, sy + y);
			int yr = y * 999 / y_div;
			for (int x = 0; x < width; x++) {
				int xr = x * 999 / x_div;
				int r = (xr + yr) >> 1;
				RGBA pix = (*pfn)(p, (RGBA*)&plgp->palPalEntry[r]);
				blend_to(p, &pix, opaque);
				p++;
			}
		});
	}
	else if (bounds == strokeUpRight) {
		parallel_for (0, height, [&](int y) {
			RGBA* p = pos(sx, sy + y);
			int yr = (height - y - 1) * 999 / y_div;
			for (int x = 0; x < width; x++) {
				int xr = x * 999 / x_div;
				int r = (xr + yr) >> 1;
				RGBA pix = (*pfn)(p, (RGBA*)&plgp->palPalEntry[r]);
				blend_to(p, &pix, opaque);
				p++;
			}
		});
	}
	free_gradtable(plgp);
}

void mif::gradientfill(LPRECT prc, GRADCOLORS* pcol, int colors, int bounds, int opaque, int blendmode)
{
	CRect rc;
	if (prc && !IsRectEmpty(prc))
		rc.CopyRect(prc);
	else
		rc.SetRect(0, 0, m_width, m_height);
	gradientfill(rc.left, rc.top, rc.Width(), rc.Height(), pcol, colors, bounds, opaque, blendmode);
}

void mif::fillgradientrect(int sx, int sy, int width, int height, GRADCOLORS* pcol, int colors, int bounds, int opaque)
{
	gradientfill(sx, sy, width, height, pcol, colors, bounds, opaque);
}

void mif::fillgradientrect(LPRECT prc, GRADCOLORS* pcol, int colors, int bounds, int opaque)
{
	CRect rc;
	if (prc && !IsRectEmpty(prc))
		rc.CopyRect(prc);
	else
		rc.SetRect(0, 0, m_width, m_height);
	gradientfill(rc.left, rc.top, rc.Width(), rc.Height(), pcol, colors, bounds, opaque);
}

void mif::settoalpha()
{
	m_bits = 24;
	desaturate(NULL);

	register RGBA* p = m_pixels;
	for (int x = 0; x < m_packets; x++) {
		p->alpha = p->red;
		p++;
	}
}

void mif::copyplane(int planeFrom, int planeTo)
{
	if (planeFrom == planeTo)
		return;

	register RGBA* p = m_pixels;
	for (int x = 0; x < m_packets; x++)
	{
		*((BYTE*)p + planeTo) = *((BYTE*)p + planeFrom);
		p++;
	}
}

void mif::invertrect(int sx, int sy, int width, int height)
{
	if (width < 1 || height < 1)
		return;

	register RGBA* p;
	if (m_bits < 24) {
		p = m_map;
		for (int i = 0; i < m_colors; i++) {
			p->red = ~p->red;
			p->green = ~p->green;
			p->blue = ~p->blue;
			p++;
		}
		sx = 0;
		sy = 0;
		width = m_width;
		height = m_height;
	}

	int ex = sx + width;
	int ey = sy + height;

	sx = max(0, min(sx, m_width));
	sy = max(0, min(sy, m_height));
	ex = min(ex, m_width);
	ey = min(ey, m_height);

	width = ex - sx;
	height = ey - sy;
	if (width < 1 || height < 1)
		return;

	for (int y = sy; y < ey; y++) {
		p = m_pixels + y * m_width + sx;
		for (int x = sx; x < ex; x++) {
			p->red = ~p->red;
			p->green = ~p->green;
			p->blue = ~p->blue;
			p++;
		}
	}
}

void mif::invertrect(LPRECT prc)
{
	CRect rc;
	if (prc && !IsRectEmpty(prc))
		rc.CopyRect(prc);
	else
		rc.SetRect(0, 0, m_width, m_height);
	invertrect(rc.left, rc.top, rc.Width(), rc.Height());
}

void mif::texturefill(mif* texture, int sx, int sy, int width, int height, int opaque, int blendmode)
{
	if (width < 1 || height < 1 || opaque == 0)
		return;

	int ex = sx + width;
	int ey = sy + height;

	sx = max(0, min(sx, m_width));
	sy = max(0, min(sy, m_height));
	ex = min(ex, m_width);
	ey = min(ey, m_height);

	width = ex - sx;
	height = ey - sy;
	if (width < 1 || height < 1)
		return;

	if (m_bits < 24)
		m_bits = 24;

	BLENDFUNC pfn = get_blendfunction(blendmode);

	register RGBA* p;
	register RGBA* q;
	register RGBA* qw;
	RGBA pix;
	for (int y = sy; y < ey; y++) {
		p = m_pixels + y * m_width + sx;
		q = texture->m_pixels + ((y - sy) % texture->m_height) * texture->m_width;
		for (int x = sx; x < ex; x++) {
			qw = q + ((x - sx) % texture->m_width);

			pix = (*pfn)(p, qw);
			blend_to(p, &pix, opaque);
			p++;
		}
	}
}

void mif::texturefill(mif* texture, LPRECT prc, int opaque, int blendmode)
{
	CRect rc;
	if (validaterect(prc, &rc))
		texturefill(texture, rc.left, rc.top, rc.Width(), rc.Height(), opaque, blendmode);
}

void mif::getcol(RGBA* data, int sx, int sy, int height)
{
	register RGBA* pixels = m_pixels + sy * m_width + sx;
	for (int y = 0; y < height; y++) {
		if (y + sy >= m_height)
			break;

		*data = *pixels;
		pixels += m_width;
		data++;
	}
}

void mif::setcol(RGBA* data, int sx, int sy, int height)
{
	register RGBA* pixels = m_pixels + sy * m_width + sx;
	for (int y = 0; y < height; y++) {
		if (y + sy >= m_height)
			break;

		*pixels = *data;
		pixels += m_width;
		data++;
	}
}

void mif::getcol_rgb(BYTE* data, int sx, int sy, int height)
{
	register RGBA* p = m_pixels + sy * m_width + sx;
	for (int y = 0; y < height; y++) {
		if (y + sy >= m_height)
			break;

		*data++ = p->red;
		*data++ = p->green;
		*data++ = p->blue;
		p += m_width;
	}
}

void mif::setcol_rgb(BYTE* data, int sx, int sy, int height)
{
	register RGBA* p = m_pixels + sy * m_width + sx;
	for (int y = 0; y < height; y++) {
		if (y + sy >= m_height)
			break;

		p->red = *data++;
		p->green = *data++;
		p->blue = *data++;
		p += m_width;
	}
}

void mif::getrow(RGBA* data, int sx, int sy, int width)
{
	RGBA* pixels = m_pixels + sy * m_width + sx;
	memcpy(data, pixels, min(width, m_width - sx) * sizeof(RGBA));
}

void mif::setrow(RGBA* data, int sx, int sy, int width)
{
	RGBA* pixels = m_pixels + sy * m_width + sx;
	memcpy(pixels, data, min(width, m_width - sx) * sizeof(RGBA));
}

void mif::getrow_rgb(BYTE* data, int sx, int sy, int width)
{
	register RGBA* p = m_pixels + sy * m_width + sx;
	for (int x = 0; x < width; x++) {
		if (x + sx >= m_width)
			break;

		*data++ = p->red;
		*data++ = p->green;
		*data++ = p->blue;
		p++;
	}
}

void mif::setrow_rgb(BYTE* data, int sx, int sy, int width)
{
	register RGBA* p = m_pixels + sy * m_width + sx;
	for (int x = 0; x < width; x++) {
		if (x + sx >= m_width)
			break;

		p->red = *data++;
		p->green = *data++;
		p->blue = *data++;
		p++;
	}
}

void mif::getpacket(RGBA* data, int sx, int sy, int dx, int dy)
{
	int size = min(dx, m_width - sx);
	register RGBA* p = m_pixels + sy * m_width + sx;
	register RGBA* q = data;
	for (int y = 0; (y < dy) && (sy + y < m_height); y++) {
		memcpy(q, p, size * sizeof(RGBA));
		p += m_width;
		q += size;
	}
}

void mif::getplane(BYTE* data, int sx, int sy, int dx, int dy, int plane)
{
	dx = min(dx, m_width - sx);
	dy = min(dy, m_height - sy);

	register RGBA* p;
	register BYTE* q = data;
	for (int y = 0; y < dy; y++) {
		p = pos(sx, sy + y);
		if (plane == planeRed) {
			for (int x = 0; x < dx; x++) {
				*q++ = p->red;
				p++;
			}
		}
		else if (plane == planeGreen) {
			for (int x = 0; x < dx; x++) {
				*q++ = p->green;
				p++;
			}
		}
		else if (plane == planeBlue) {
			for (int x = 0; x < dx; x++) {
				*q++ = p->blue;
				p++;
			}
		}
		else if (plane == planeAlpha) {
			for (int x = 0; x < dx; x++) {
				*q++ = p->alpha;
				p++;
			}
		}
	}
}

void mif::setplane(BYTE* data, int sx, int sy, int dx, int dy, int plane)
{
	dx = min(dx, m_width - sx);
	dy = min(dy, m_height - sy);

	register RGBA* p;
	register BYTE* q = data;
	for (int y = 0; y < dy; y++) {
		p = pos(sx, sy + y);
		if (plane == planeRed) {
			for (int x = 0; x < dx; x++) {
				p->red = *q++;
				p++;
			}
		}
		else if (plane == planeGreen) {
			for (int x = 0; x < dx; x++) {
				p->green = *q++;
				p++;
			}
		}
		else if (plane == planeBlue) {
			for (int x = 0; x < dx; x++) {
				p->blue = *q++;
				p++;
			}
		}
		else if (plane == planeAlpha) {
			for (int x = 0; x < dx; x++) {
				p->alpha = *q++;
				p++;
			}
		}
	}
}


void mif::setpacket(RGBA* data, int sx, int sy, int dx, int dy)
{
	int size = min(dx, m_width - sx);
	register RGBA* p = m_pixels + sy * m_width + sx;
	register RGBA* q = data;
	for (int y = 0; (y < dy) && (sy + y < m_height); y++) {
		memcpy(p, q, size * sizeof(RGBA));
		p += m_width;
		q += size;
	}
}


BOOL mif::validaterect(LPRECT prcOrg, LPRECT prcVld)
{
	if (!prcOrg || IsRectEmpty(prcOrg)) {
		prcVld->left = 0;
		prcVld->right = m_width;
		prcVld->top = 0;
		prcVld->bottom = m_height;
	}
	else {
		prcVld->left = max(0, prcOrg->left);
		prcVld->right = min(m_width, prcOrg->right);
		prcVld->top = max(0, prcOrg->top);
		prcVld->bottom = min(m_height, prcOrg->bottom);
		if (prcVld->left >= prcVld->right || prcVld->top >= prcVld->bottom)
			return FALSE;
	}
	return TRUE;
}

void __fastcall rgba_reverse_buffer(RGBA *buffer, int length)
{
	int midpoint = length / 2;
	register RGBA* ps = buffer;
	register RGBA* pe = &buffer[length - 1];
	for (int i = 0; i < midpoint; i++) {
		RGBA temp = *ps;
		*ps = *pe;
		*pe = temp;
		ps++;
		pe--;
	}
}

void mif::rotate(BOOL rotate_left)
{
	mif dst;
	dst.create(m_height, m_width, m_bits);

	if (m_bits < 24) {
		memcpy(dst.m_map, m_map, m_colors * sizeof(RGBA));
		dst.m_colors = m_colors;
	}

	if (rotate_left) {
		parallel_for (0, m_height, [&](int y) {
			RGBA* p = pos(0, y);
			RGBA* q = dst.pos(y, dst.m_height - 1);
			for (int x = 0; x < m_width; x++) {
				*q = *p++;
				q -= dst.m_width;
			}
		});
	}
	else {
		parallel_for (0, m_height, [&](int y) {
			RGBA* p = pos(0, y);
			RGBA* q = dst.pos(dst.m_width - y - 1, 0);
			for (int x = 0; x < m_width; x++) {
				*q = *p++;
				q += dst.m_width;
			}
		});
	}
	movefrom(&dst);
}

BOOL mif::rotate180(int sx, int sy, int dx, int dy)
{
	CRect rc, rcw(sx, sy, sx + dx, sy + dy);
	if (!validaterect(&rcw, &rc))
		return FALSE;

	int width = rc.Width();
	int height = rc.Height();
	parallel_for (0, height / 2, [&](int y) {
		RGBA* p = pos(rc.left, rc.top + y);
		RGBA* q = pos(rc.right - 1, rc.bottom - y - 1);
		for (int x = 0; x < width; x++) {
			RGBA pix = *p; *p = *q; *q = pix; p++; q--;
		}
	});
	return TRUE;
}

BOOL mif::mirror(int sx, int sy, int dx, int dy)
{
	CRect rc, rcw(sx, sy, sx + dx, sy + dy);
	if (!validaterect(&rcw, &rc))
		return FALSE;

	int width = rc.Width();
	parallel_for (rc.top, rc.bottom, [&](int y) {
		RGBA* p = pos(rc.left, y);
		RGBA* q = pos(rc.right - 1, y);
		for (int x = 0; x < width / 2; x++) {
			RGBA pix = *p; *p = *q; *q = pix; p++; q--;
		}
	});
	return TRUE;
}

BOOL mif::flip(int sx, int sy, int dx, int dy)
{
	CRect rc, rcw(sx, sy, sx + dx, sy + dy);
	if (!validaterect(&rcw, &rc))
		return FALSE;

	int width = rc.Width();
	int height = rc.Height();
	parallel_for (0, height / 2, [&](int y) {
		RGBA* p = pos(rc.left, rc.top + y);
		RGBA* q = pos(rc.left, rc.bottom - y - 1);
		for (int x = 0; x < width; x++) {
			RGBA pix = *p; *p = *q; *q = pix; p++; q++;
		}
	});
	return TRUE;
}

BOOL mif::crop(LPRECT prc)
{
	CRect rc;
	if (!validaterect(prc, &rc))
		return FALSE;

	mif dst;
	dst.create(rc.Width(), rc.Height(), m_bits);
	if (m_bits < 24) {
		dst.m_colors = m_colors;
		memcpy(dst.m_map, m_map, sizeof(RGBA) * m_colors);
	}
	dst.blt(0, 0, rc.Width(), rc.Height(), this, rc.left, rc.top);

	movefrom(&dst);
	return TRUE;
}

void mif::addborder(int left, int right, int top, int bottom, COLORREF rgb)
{
	if (m_bits < 24)
		m_bits = 24;

	mif dst;
	dst.create(m_width + left + right, m_height + top + bottom, m_bits);
	dst.solidfill(0, 0, dst.m_width, dst.m_height, rgb, 255);
	dst.blt(left, top, m_width, m_height, this, 0, 0);
	movefrom(&dst);
}

static int arSharpen[9] = { -1, -1, -1, -1, 24, -1, -1, -1, -1 };

static void __fastcall do_sharpen(RGBA* p1, RGBA* p2, RGBA* p3, RGBA* pb, int x, int width)
{
	register int red, green, blue;
	red = 0;
	green = 0;
	blue = 0;

	for (int i = -1; i < 2; i++) {
		int n;
		if (x + i < 0)				n = 0;
		else if (x + i >= width)	n = width - 1;
		else						n = x + i;

		red +=		p1[n].red * arSharpen[i + 1] +
					p2[n].red * arSharpen[i + 4] +
					p3[n].red * arSharpen[i + 7];
		green +=	p1[n].green * arSharpen[i + 1] +
					p2[n].green * arSharpen[i + 4] +
					p3[n].green * arSharpen[i + 7];
		blue +=		p1[n].blue * arSharpen[i + 1] +
					p2[n].blue * arSharpen[i + 4] +
					p3[n].blue * arSharpen[i + 7];
	}
	pb->blue = ADJRGB(blue >> 4);
	pb->green = ADJRGB(green >> 4);
	pb->red = ADJRGB(red >> 4);
}

void mif::sharpen(LPRECT prc)
{
	CRect rc;
	if (!validaterect(prc, &rc))
		return;

	register int x, y;
	int cx = rc.Width();
	int cy = rc.Height();

	RGBA* line1 = (RGBA*)malloc(cx * sizeof(RGBA));
	RGBA* line2 = (RGBA*)malloc(cx * sizeof(RGBA));
	RGBA* line3 = (RGBA*)malloc(cx * sizeof(RGBA));

	RGBA* pb = m_pixels + rc.top * m_width + rc.left;
	memcpy(line1, pb, cx * sizeof(RGBA));
	memcpy(line2, pb, cx * sizeof(RGBA));
	for (y = 0; y < cy; y++) {
		pb = m_pixels + (y + rc.top) * m_width + rc.left;
		if (y == cy - 1)
			memcpy(line3, pb, cx * sizeof(RGBA));
		else
			memcpy(line3, pb + m_width, cx * sizeof(RGBA));

		for (x = 0; x < cx; x++) {
			do_sharpen(line1, line2, line3, pb, x, cx);
			pb++;
		}
		RGBA* tmp = line1;
		line1 = line2;
		line2 = line3;
		line3 = tmp;
	}
	free(line1);
	free(line2);
	free(line3);
}

BOOL mif::sharpen2(LPRECT prc, int per)
{
	return ::sharpen2(*this, prc, per);
}

void mif::negative(LPRECT prc)
{
	CRect rc;
	register RGBA* p;
	register int cnt;
	if (m_bits < 24) {
		p = m_map;
		cnt = m_colors;
		while (cnt-- > 0) {
			p->red = ~p->red;
			p->green = ~p->green;
			p->blue = ~p->blue;
			p++;
		}
	}
	else if (validaterect(prc, &rc)) {
		for (int y = rc.top; y < rc.bottom; y++) {
			p = pos(rc.left, y);
			cnt = rc.Width();
			while (cnt-- > 0) {
				p->red = ~p->red;
				p->green = ~p->green;
				p->blue = ~p->blue;
				p++;
			}
		}
	}
}

BOOL mif::merge(mif* dst, BOOL horz, int span, COLORREF rgb)
{
	if (!dst)
		return FALSE;

	span = max(0, span);

	int sx = m_width;
	int sy = m_height;
	int dx = dst->m_width;
	int dy = dst->m_height;

	int mx, my;
	if (horz) {
		mx = sx + dx + span;
		my = max(sy, dy);
	}
	else {
		mx = max(sx, dx);
		my = sy + dy + span;
	}

	mif mrg;
	if (!mrg.create(mx, my, 24))
		return FALSE;

	mrg.solidfill(0, 0, mx, my, rgb, 255);
	mrg.blt(0, 0, m_width, m_height, this, 0, 0);
	movefrom(&mrg);

	if (horz)
		blt(sx + span, 0, dx, dy, dst, 0, 0);
	else
		blt(0, sy + span, dx, dy, dst, 0, 0);
	return TRUE;
}

/* @@@@@ インデクスカラーにせず減色 256色以上でもOK
void mif::quantize_simple(int numcolors, BOOL cie_luv, BOOL principal)
{
	::quantize_simple(*this, numcolors, cie_luv, principal);
}
@@@@@ */

void mif::quantize(int dither_type, int numcolors, LOGPALETTE* plgp)
{
	::quantize(*this, dither_type, numcolors, plgp);
}

void mif::quantize_wpal(int dither_type, const SYSCOL* sc, int numpal)
{
	LOGPALETTE* plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * numpal);
	plp->palVersion = 0x300;
	plp->palNumEntries = numpal;
	PALETTEENTRY* ppe = plp->palPalEntry;
	for (int i = 0; i < numpal; i++) {
		ppe->peRed = sc->red;
		ppe->peGreen = sc->green;
		ppe->peBlue = sc->blue;
		ppe->peFlags = 0;
		ppe++;
		sc++;
	}
	::quantize(*this, dither_type, 0, plp);
	free(plp);
}

BOOL mif::quantize_wdp(int dither_type, int wdpal)
{
	BOOL bOK = TRUE;
	switch (wdpal) {
		case wdpalWin16:
			quantize_wpal(dither_type, pal_win16, sizeof(pal_win16) / sizeof(SYSCOL));
			break;

		case wdpalWin256:
			quantize_wpal(dither_type, pal_win256, sizeof(pal_win256) / sizeof(SYSCOL));
			break;

		case wdpalMac:
			quantize_wpal(dither_type, pal_mac, sizeof(pal_mac) / sizeof(SYSCOL));
			break;

		case wdpalWeb:
			quantize_wpal(dither_type, pal_web, sizeof(pal_web) / sizeof(SYSCOL));
			break;

		case wdpalSpect:
			quantize_wpal(dither_type, pal_spc, sizeof(pal_spc) / sizeof(SYSCOL));
			break;

		case wdpalPastel:
			quantize_wpal(dither_type, pal_pastel, sizeof(pal_pastel) / sizeof(SYSCOL));
			break;

		case wdpalHot:
			quantize_wpal(dither_type, pal_kok, sizeof(pal_kok) / sizeof(SYSCOL));
			break;

		case wdpalCold:
			quantize_wpal(dither_type, pal_cold, sizeof(pal_cold) / sizeof(SYSCOL));
			break;

		case wdpalGold:
			quantize_wpal(dither_type, pal_gold, sizeof(pal_gold) / sizeof(SYSCOL));
			break;

		default:
			bOK = FALSE;
			break;
	}
	return bOK;
}

BOOL mif::bright_contrast(LPRECT prc, int brightness, int contrast)
{
	return ::bright_contrast(*this, prc, brightness, contrast);
}

BOOL mif::bumpmap(LPRECT prc, MIF_BUMPMAP* pbp)
{
	return ::bumpmap(*this, prc, pbp);
}

BOOL mif::bumpmap_merge(mif& bmp, POINT pt, MIF_BUMPMAP* pbp)
{
	return ::bumpmap_merge(*this, bmp, pt, pbp);
}

BOOL mif::autocontrast(LPRECT prc)
{
	return ::autocontrast(*this, prc);
}

BOOL mif::blur(LPRECT prc, int xfactor, int yfactor)
{
	CRect rc;
	if (validaterect(prc, &rc) && (xfactor > 0 || yfactor > 0)) {
		::blur(*this, &rc, xfactor, yfactor);
		return TRUE;
	}
	return FALSE;
}

BOOL mif::blur_simple(LPRECT prc, int radius)
{
	return ::blur_simple(*this, prc, radius);
}

BOOL mif::blur_gauss_rle(LPRECT prc, double radius)
{
	return ::blur_gauss_rle(*this, prc, radius);
}

BOOL mif::blur_gauss_rle_plane(LPRECT prc, double radius, UINT planes)
{
	return ::blur_gauss_rle_plane(*this, prc, radius, planes);
}

BOOL mif::blur_gauss_iir(LPRECT prc, double horz, double vert)
{
	return ::blur_gauss_iir(*this, prc, horz, vert);
}

BOOL mif::blur_gauss_sel(LPRECT prc, double radius, int maxdelta)
{
	return ::blur_gauss_sel(*this, prc, radius, maxdelta);
}

BOOL mif::gamma_rgb(LPRECT prc, double r_amt, double g_amt, double b_amt, double rgb_amt)
{
	return ::gamma_rgb(*this, prc, r_amt, g_amt, b_amt, rgb_amt);
}

BOOL mif::gamma(LPRECT prc, double rgb_amt)
{
	return ::gamma_rgb(*this, prc, 1.0, 1.0, 1.0, rgb_amt);
}

BOOL mif::modulate(LPRECT prc, int per_h, int per_s, int per_l)
{
	return ::modulate(*this, prc, per_h, per_s, per_l);
}

BOOL mif::hue_saturation(LPRECT prc, int hue_range, int per_h, int per_s, int per_l)
{
	return ::hue_saturation(*this, prc, hue_range, per_h, per_s, per_l);
}

BOOL mif::autocolorenhance(LPRECT prc)
{
	return ::autocolorenhance(*this, prc);
}

BOOL mif::adjust_rgb(LPRECT prc, int red, int green, int blue)
{
	return ::adjust_rgb(*this, prc, red, green, blue);
}

BOOL mif::colorbalance(LPRECT prc, MIF_COLORBALANCE* pcb)
{
	return ::colorbalance(*this, prc, pcb);
}

BOOL mif::colorify(LPRECT prc, COLORREF cr)
{
	return ::colorify(*this, prc, cr);
}

BOOL mif::colorcut(LPRECT prc, MIF_COLORCUT* pcp)
{
	return ::colorcut(*this, prc, pcp);
}

DWORD mif::countcolors()
{
	return ::countcolors(*this);
}

BOOL mif::edge(LPRECT prc, double amount)
{
	return ::edge(*this, prc, amount);
}

BOOL mif::emboss(LPRECT prc, MIF_EMBOSS* pep)
{
	return ::emboss(*this, prc, pep);
}

BOOL mif::enhance(LPRECT prc, int threshold)
{
	return ::enhance(*this, prc, threshold);
}

BOOL mif::equalize(LPRECT prc)
{
	return ::equalize(*this, prc);
}

BOOL mif::fadeinrect(LPRECT prc, int delta, COLORREF cr)
{
	return ::fadeinrect(*this, prc, delta, cr);
}

BOOL mif::fadepaste(mif& paste, int delta, int xpos, int ypos)
{
	return ::fadepaste(*this, paste, delta, xpos, ypos);
}

BOOL mif::filter_3x3(LPRECT prc, UINT nType)
{
	return ::filter_formal3x3(*this, prc, nType);
}

BOOL mif::filter_custom3x3(LPRECT prc, int* pFilter, int nDivide)
{
	return ::filter_custom3x3(*this, prc, pFilter, nDivide);
}

BOOL mif::filter_sharpen(LPRECT prc, int nValue)
{
	int filter[9];
	if (nValue < 1 || nValue > 40)
		nValue = 10;

	for (int i = 0; i < 9; i++)
		filter[i] = -1;
	filter[4] = nValue + 8;
	return filter_custom3x3(prc, filter, nValue);
}

BOOL mif::filter_soften(LPRECT prc, int nValue)
{
	int filter[9];
	if (nValue < 1 || nValue > 40)
		nValue = 10;

	for (int i = 0; i < 9; i++)
		filter[i] = 1;
	filter[4] = nValue;
	return filter_custom3x3(prc, filter, nValue + 8);
}

BOOL mif::mblur_linear(LPRECT prc, int length, int angle)
{
	return ::mblur_linear(*this, prc, length, angle);
}

BOOL mif::mblur_radial(LPRECT prc, int angle)
{
	return ::mblur_radial(*this, prc, angle);
}

BOOL mif::mblur_zoom(LPRECT prc, int length)
{
	return ::mblur_zoom(*this, prc, length);
}

BOOL mif::mblur_round(LPRECT prc, int rot)
{
	return ::mblur_round(*this, prc, rot);
}

BOOL mif::addnoise(LPRECT prc, int noise_type)
{
	return ::addnoise(*this, prc, noise_type);
}

BOOL mif::normalize(LPRECT prc)
{
	return ::normalize(*this, prc);
}

BOOL mif::oilpaint(LPRECT prc, int rad)
{
	return ::oilpaint(*this, prc, rad);
}

BOOL mif::pixelize(LPRECT prc, int xblock, int yblock, BOOL renga, BOOL cross)
{
	return ::pixelize(*this, prc, xblock, yblock, renga, cross);
}

BOOL mif::posterize(LPRECT prc, int levels)
{
	return ::posterize(*this, prc, levels);
}

BOOL mif::pyramid(LPRECT prc, int block)
{
	return ::pyramid(*this, prc, block);
}

BOOL mif::raise_edge(LPRECT prc, int delta)
{
	return ::raise_edge(*this, prc, delta);
}

BOOL mif::scroll(int sx, int sy, BOOL wrap)
{
	return ::scroll(*this, sx, sy, wrap);
}

BOOL mif::spread(LPRECT prc, int amount)
{
	return ::spread(*this, prc, amount);
}

BOOL mif::sblur_blast(LPRECT prc, int threshold, int strength, int direction, int edge)
{
	return ::sblur_blast(*this, prc, threshold, strength, direction, edge);
}

BOOL mif::sblur_wind(LPRECT prc, int threshold, int strength, int direction, int edge)
{
	return ::sblur_wind(*this, prc, threshold, strength, direction, edge);
}

BOOL mif::tile_divide(LPRECT prc, int numdiv)
{
	return ::tile_divide(*this, prc, numdiv);
}

BOOL mif::tile(LPRECT prc, int xblock, int yblock)
{
	return ::tile(*this, prc, xblock, yblock);
}


BOOL mif::texturize(LPRECT prc, char* texture, int twidth, int theight, int dir, int depth)
{
	return ::texturize(*this, prc, texture, twidth, theight, dir, depth);
}

BOOL mif::texturize_by_file(LPRECT prc, LPCSTR txrfile, int direction, int depth)
{
	return ::texturize_by_file(*this, prc, txrfile, direction, depth);
}

BOOL mif::texturize_by_resource(LPRECT prc, HINSTANCE hInst, UINT typeResource, UINT idResource, int direction, int depth)
{
	return ::texturize_by_resource(*this, prc, hInst, typeResource, idResource, direction, depth);
}

BOOL mif::texture_save(LPCSTR title, LPCSTR txrfile, BOOL gray)
{
	return texture_to_file(*this, title, txrfile, gray);
}

BOOL mif::unsharp_mask(LPRECT prc, MIF_UNSHARPMASK* pup)
{
	return ::unsharp_mask(*this, prc, pup);
}

BOOL mif::unsharp_mask2(LPRECT prc, double radius, double depth)
{
	return ::unsharp_mask2(*this, prc, radius, depth);
}

BOOL mif::video(LPRECT prc, MIF_VIDEO* pvp)
{
	return ::video(*this, prc, pvp);
}

BOOL mif::zoom(int cx, int cy, int zoom_method, BOOL ondisk)
{
	if ((m_width == cx && m_height == cy) ||
		cx < 1 || cy < 1 || zoom_method < 0 || zoom_method >= zoomLast)
		return FALSE;

	if (m_bits < 24)
		m_bits = 24;

	int orgwidth = m_width;
	int orgheight = m_height;
	if (zoom_filter(*this, cx, cy, zoom_method, ondisk))
	{
		if (cy < orgheight && cx < orgwidth)
			sharpen(NULL);
		return TRUE;
	}
	return FALSE;
}

BOOL mif::zoom_pixel(int cx, int cy)
{
	return ::zoom_pixel(*this, cx, cy);
}

BOOL mif::zoom_near(int cx, int cy)
{
	return ::zoom_near(*this, cx, cy);
}

BOOL mif::zoom_linear(int cx, int cy)
{
	return ::zoom_linear(*this, cx, cy);
}

BOOL mif::zoom_inframe(int cx, int cy)
{
	return ::zoom_inframe(*this, cx, cy);
}

void mif::convert_colorspace(int color_space)
{
	rgb2any ra(color_space);
	register RGBA* p = m_pixels;
	int cnt = m_packets;
	while (cnt-- > 0)
	{
		ra.xfer((BYTE*)p);
		p++;
	}
}

void mif::convert_rgb(int color_space)
{
	any2rgb ar(color_space);
	register RGBA* p = m_pixels;
	int cnt = m_packets;
	while (cnt-- > 0)
	{
		ar.xfer((BYTE*)p);
		p++;
	}
}

void mif::progress_init(int maxprogress)
{
	if (m_progress) (*m_progress)(PROG_INITIALIZE, maxprogress);
}

void mif::progress_continue()
{
	if (m_progress) (*m_progress)(PROG_CONTINUE, 0);
}

void mif::progress_term()
{
	if (m_progress) (*m_progress)(PROG_DONE, 0);
}

void mif::progress_reset()
{
	if (m_progress) (*m_progress)(PROG_RESET, 0);
}

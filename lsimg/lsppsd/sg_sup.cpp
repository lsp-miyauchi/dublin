//////////////////////////////////////////////////////////////////////
// sg_sup.cpp

#include "stdafx.h"
#include "dib_sup.h"
#include "sg_sup.h"


SGIMAGE* __fastcall sg_create(int width, int height, int bits, BOOL bOnDisk)
{
	SGIMAGE* image = (SGIMAGE*)malloc(sizeof(SGIMAGE));
	if (!image)
		return NULL;

	memset(image, 0, sizeof(SGIMAGE));
	image->width = width;
	image->height = height;
	image->packets = width * height;
	image->bits = bits;
	if (image->bits < 24)
		image->colors = min(256, 1 << bits);

	DWORD needs = image->packets * sizeof(PIXELPACKET);
	if (bOnDisk)
	{
		char tmp[MAX_PATH];
		GetTempPath(MAX_PATH, tmp);
		GetTempFileName(tmp, "~sg", 0, image->tmpfile);
		image->hfile = CreateFile(image->tmpfile, GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
								NULL);

		BOOL bOK = FALSE;
		if (image->hfile != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(image->hfile, needs, NULL, FILE_BEGIN);
			SetEndOfFile(image->hfile);
			SetFilePointer(image->hfile, 0, NULL, FILE_BEGIN);

			if (GetFileSize(image->hfile, NULL) == needs)
			{
				image->hmap = CreateFileMapping(image->hfile, NULL,
											PAGE_READWRITE, 0, 0, NULL);
				if (image->hmap)
				{
					image->pixels = (PIXELPACKET*)MapViewOfFile(image->hmap,
														FILE_MAP_WRITE, 0, 0, 0);
					if (image->pixels)
						bOK = TRUE;
					else
						CloseHandle(image->hmap);
				}
			}
			if (!bOK)
			{
				CloseHandle(image->hfile);
				DeleteFile(image->tmpfile);
			}
		}

		if (!bOK)
		{
			free(image);
			image = NULL;
		}
	}
	else
	{
		image->pixels = (PIXELPACKET*)malloc(needs);
		if (!image->pixels)
		{
			free(image);
			image = NULL;
		}
		else
			memset(image->pixels, 0, needs);
	}
	return image;
}

BOOL __fastcall sg_destroy(SGIMAGE* image)
{
	if (!image)
		return FALSE;

	if (image->hfile)
	{
		UnmapViewOfFile(image->pixels);
		CloseHandle(image->hmap);
		CloseHandle(image->hfile);
		DeleteFile(image->tmpfile);
	}
	else
	{
		if (image->pixels)
			free(image->pixels);
	}
	free(image);
	return TRUE;
}

BOOL __fastcall sg_move(SGIMAGE* dst, SGIMAGE* src)
{
	if (!src || !dst)
		return FALSE;

	if (dst->pixels)
	{
		if (dst->hfile)
		{
			UnmapViewOfFile(dst->pixels);
			CloseHandle(dst->hmap);
			CloseHandle(dst->hfile);
			DeleteFile(dst->tmpfile);
		}
		else
			free(dst->pixels);
	}
	memcpy(dst, src, sizeof(SGIMAGE));
	memset(src, 0, sizeof(SGIMAGE));
	return TRUE;
}

BOOL __fastcall sg_convert2bmi(SGIMAGE* image, BITMAPINFO* pbi)
{
	if (!image || !image->pixels || !pbi)
		return FALSE;

	if (image->bits <= 8)
	{
		pbi->bmiHeader.biClrUsed = image->colors;
		pbi->bmiHeader.biClrImportant = image->colors;
		for (int x = 0; x < image->colors; x++)
		{
			pbi->bmiColors[x].rgbRed = image->colormap[x].red;
			pbi->bmiColors[x].rgbGreen = image->colormap[x].green;
			pbi->bmiColors[x].rgbBlue = image->colormap[x].blue;
		}
	}

	PIXELPACKET* pixels = image->pixels;
	int width = WIDTHBYTES(image->width * image->bits);
	unsigned char* base = dib_findbits(pbi);
	for (int y = 0; y < image->height; y++)
	{
		unsigned char* ps = base + width * (image->height - y - 1);
		for (int x = 0; x < image->width; x++)
		{
			if (image->bits == 24)
			{
				*ps++ = pixels->blue;
				*ps++ = pixels->green;
				*ps++ = pixels->red;
			}
			else if (image->bits == 8)
				*ps++ = pixels->index;
			else if (image->bits == 4)
			{
				if (x & 1)
					*ps++ |= pixels->index & 0x0f;
				else
					*ps = (pixels->index << 4) & 0xf0;
			}
			else if (image->bits == 1)
			{
				if ((x & 7) == 0)
					*ps = 0;
				*ps |= (pixels->index & 1) << (7 - (x & 7));
				if ((x & 7) == 7)
					ps++;
			}
			pixels++;
		}
	}
	return TRUE;
}

HDIB __fastcall sg_getdib(SGIMAGE* image)
{
	HDIB hdib = dib_create(image->width, image->height, image->bits);
	if (hdib)
	{
		BITMAPINFO* pbi = (BITMAPINFO*)GlobalLock(hdib);
		BOOL bOK = sg_convert2bmi(image, pbi);
		GlobalUnlock(hdib);
		if (!bOK)
		{
			GlobalFree(hdib);
			hdib = NULL;
		}
	}
	return hdib;
}

SGIMAGE* __fastcall sg_fromdib(HANDLE hdib)
{
	if (!hdib)
		return NULL;

	BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)GlobalLock(hdib);
	SGIMAGE* image = sg_create(pbh->biWidth, pbh->biHeight, pbh->biBitCount);
	if (!image)
	{
		GlobalUnlock(hdib);
		return NULL;
	}

	int x, y;
	if (image->bits < 24)
	{
		// インデクスカラーのコピー
		image->colors = pbh->biClrUsed ? pbh->biClrUsed : 1 << image->bits;
		image->colors = min(256, image->colors);

		BITMAPINFO* pbi = (BITMAPINFO*)pbh;
		for (x = 0; x < image->colors; x++)
		{
			image->colormap[x].red = pbi->bmiColors[x].rgbRed;
			image->colormap[x].green = pbi->bmiColors[x].rgbGreen;
			image->colormap[x].blue = pbi->bmiColors[x].rgbBlue;
		}
	}

	PIXELPACKET* pixels = image->pixels;
	BYTE* base = dib_findbits(pbh);
	int width = WIDTHBYTES(image->width * image->bits);
	for (y = 0; y < image->height; y++)
	{
		BYTE* ps = base + width * (image->height - y - 1);
		for (x = 0; x < image->width; x++)
		{
			if (image->bits > 8)
			{
				pixels->blue = *ps++;
				pixels->green = *ps++;
				pixels->red = *ps++;
			}
			else if (image->bits == 8)
			{
				pixels->index = *ps++;
				pixels->red = image->colormap[pixels->index].red;
				pixels->green = image->colormap[pixels->index].green;
				pixels->blue = image->colormap[pixels->index].blue;
			}
			else if (image->bits == 4)
			{
				if (x & 1)
				{
					pixels->index = (*ps) & 0xf;
					ps++;
				}
				else
					pixels->index = ((*ps) >> 4) & 0xf;
				pixels->red = image->colormap[pixels->index].red;
				pixels->green = image->colormap[pixels->index].green;
				pixels->blue = image->colormap[pixels->index].blue;
			}
			else if (image->bits == 1)
			{
				pixels->index = ((*ps) >> (7 - (x % 8))) & 1;
				pixels->red = image->colormap[pixels->index].red;
				pixels->green = image->colormap[pixels->index].green;
				pixels->blue = image->colormap[pixels->index].blue;

				if ((x % 8) == 7)
					ps++;
			}
			pixels++;
		}
	}

	GlobalUnlock(hdib);
	return image;
}

SGIMAGE* __fastcall sg_get(SGIMAGE* src, int sx, int sy, int dx, int dy)
{
	register PIXELPACKET *ps, *pd;
	register int y;

	if (!src || !src->pixels || !dx || !dy)
		return NULL;
	if (dx < 0)
	{
		sx -= dx;
		dx *= -1;
	}
	if (dy < 0)
	{
		sy -= dy;
		dy *= -1;
	}
	sx = min(src->width, max(0, sx));
	sy = min(src->height, max(0, sy));
	if (sx + dx > src->width)
		dx = src->width - sx;
	if (sy + dy > src->height)
		dy = src->height - sy;

	SGIMAGE* dst = sg_create(dx, dy, src->bits);
	if (!dst)
		return NULL;
	memcpy(dst->colormap, src->colormap, sizeof(COLORPACKET) * 256);
	dst->colors = src->colors;

	for (y = 0; y < dy; y++)
	{
		ps = src->pixels + (y + sy) * src->width + sx;
		pd = dst->pixels + y * dst->width;
		memcpy(pd, ps, (dx - sx) * sizeof(PIXELPACKET));
	}
	return dst;
}

BOOL __fastcall sg_fill(SGIMAGE* dst, int sx, int sy, int dx, int dy, COLORREF rgb)
{
	if (!dst || !dst->pixels)
		return FALSE;

	CRect rc(0, 0, dst->width, dst->height);
	CRect rcw(sx, sy, sx + dx, sy + dy);
	rc &= rcw;
	if (rc.IsRectEmpty())
		return TRUE;

	dx = rc.Width();
	dy = rc.Height();
	if (!dx || !dy)
		return TRUE;

	register PIXELPACKET* pd;
	for (int y = 0; y < dy; y++)
	{
		pd = dst->pixels + (y + rc.top) * dst->width + rc.left;
		for (int x = 0; x < dx; x++)
		{
			pd->red = GetRValue(rgb);
			pd->green = GetGValue(rgb);
			pd->blue = GetBValue(rgb);
			pd++;
		}
	}
	return TRUE;
}

BOOL __fastcall sg_blt(SGIMAGE* dst, int xdst, int ydst, int dx, int dy, SGIMAGE* src, int xsrc, int ysrc, DWORD rop)
{
	if (!src || !src->pixels || !dst || !dst->pixels)
		return FALSE;

	CRect rcd(0, 0, dst->width, dst->height);
	CRect rcw(xdst, ydst, xdst + dx, ydst + dy);
	rcd &= rcw;

	CRect rcs(0, 0, src->width, src->height);
	if (rop != BLACKNESS && rop != WHITENESS && rop != DSTINVERT)
	{
		rcw.SetRect(xsrc, ysrc, xsrc + dx, ysrc + dy);
		rcs &= rcw;
	}

	int xdelta = min(rcd.Width(), rcs.Width());
	rcd.right = rcd.left + xdelta;
	rcs.right = rcs.left + xdelta;
	int ydelta = min(rcd.Height(), rcs.Height());
	rcd.bottom = rcd.top + ydelta;
	rcs.bottom = rcs.top + ydelta;

	if (rcd.IsRectEmpty() || rcs.IsRectEmpty() || !xdelta || !ydelta)
		return TRUE;

	register PIXELPACKET *ps, *pd;
	for (int y = 0; y < ydelta; y++)
	{
		pd = dst->pixels + (y + rcd.top) * dst->width + rcd.left;
		ps = src->pixels + (y + rcs.top) * src->width + rcs.left;
		for (int x = 0; x < xdelta; x++)
		{
			switch (rop)
			{
				case SRCCOPY:
					*pd++ = *ps++;
					break;

				case SRCAND:
					pd->red &= ps->red;
					pd->green &= ps->green;
					pd->blue &= ps->blue;
					ps++;
					pd++;
					break;

				case SRCINVERT:
					pd->red ^= ps->red;
					pd->green ^= ps->green;
					pd->blue ^= ps->blue;
					ps++;
					pd++;
					break;

				case SRCPAINT:
					pd->red |= ps->red;
					pd->green |= ps->green;
					pd->blue |= ps->blue;
					ps++;
					pd++;
					break;

				case BLACKNESS:
					pd->red = 0;
					pd->green = 0;
					pd->blue = 0;
					pd++;
					break;

				case WHITENESS:
					pd->red = 255;
					pd->green = 255;
					pd->blue = 255;
					pd++;
					break;

				case DSTINVERT:
					pd->red = 255 - pd->red;
					pd->green = 255 - pd->green;
					pd->blue = 255 - pd->blue;
					pd++;
					break;
			}
		}
	}
	return TRUE;
}

BOOL __fastcall sg_mirror(SGIMAGE* src, int sx, int sy, int dx, int dy)
{
	if (!src || !src->pixels)
		return FALSE;

	CRect rc(0, 0, src->width, src->height);
	CRect rcw(sx, sy, sx + dx, sy + dy);
	rc &= rcw;
	dx = rc.Width();
	dy = rc.Height();

	if (rc.IsRectEmpty() || !dx || !dy)
		return TRUE;

	register PIXELPACKET *ps;
	PIXELPACKET* pd = (PIXELPACKET*)malloc(dx * sizeof(PIXELPACKET));
	for (int y = 0; y < dy; y++)
	{
		ps = src->pixels + (y + rc.top) * src->width + rc.left;
		memcpy(pd, ps, dx * sizeof(PIXELPACKET));
		for (int x = 0; x < dx; x++)
			*ps++ = pd[dx - x - 1];
	}
	free(pd);
	return TRUE;
}

BOOL __fastcall sg_flip(SGIMAGE* src, int sx, int sy, int dx, int dy)
{
	if (!src || !src->pixels)
		return FALSE;

	CRect rc(0, 0, src->width, src->height);
	CRect rcw(sx, sy, sx + dx, sy + dy);
	rc &= rcw;
	dx = rc.Width();
	dy = rc.Height();

	if (rc.IsRectEmpty() || !dx || !dy)
		return TRUE;

	register PIXELPACKET *ps;
	PIXELPACKET* pd = (PIXELPACKET*)malloc(dy * sizeof(PIXELPACKET));
	int x;
	for (x = 0; x < dx; x++) {
		ps = src->pixels + rc.top * src->width + rc.left + x;
		int y;
		for (y = 0; y < dy; y++)
			pd[y] = ps[y * src->width];
		for (y = 0; y < dy; y++)
			ps[y * src->width] = pd[dy - y - 1];
	}
	free(pd);
	return TRUE;
}

BOOL __fastcall sg_getrow(SGIMAGE* image, PIXELPACKET* line, int row)
{
	if (!image || !line || row < 0 || row >= image->height)
		return FALSE;

	memcpy(line, image->pixels + image->width * row,
			image->width * sizeof(PIXELPACKET));
	return TRUE;
}

BOOL __fastcall sg_setrow(SGIMAGE* image, PIXELPACKET* line, int row)
{
	if (!image || !line || row < 0 || row >= image->height)
		return FALSE;

	memcpy(image->pixels + image->width * row, line,
			image->width * sizeof(PIXELPACKET));
	return TRUE;
}

BOOL __fastcall sg_getcol(SGIMAGE* image, PIXELPACKET* line, int col)
{
	if (!image || !line || col < 0 || col >= image->width)
		return FALSE;

	register PIXELPACKET* ps = image->pixels + col;
	for (int y = 0; y < image->height; y++)
	{
		*line++ = *ps;
		ps += image->width;
	}
	return TRUE;
}

BOOL __fastcall sg_setcol(SGIMAGE* image, PIXELPACKET* line, int col)
{
	if (!image || !line || col < 0 || col >= image->width)
		return FALSE;

	register PIXELPACKET* ps = image->pixels + col;
	for (int y = 0; y < image->height; y++)
	{
		*ps = *line++;
		ps += image->width;
	}
	return TRUE;
}

BOOL __fastcall sg_getplane(SGIMAGE* image, UINT plane, unsigned char* data)
{
	if (!image || !data)
		return FALSE;

	register PIXELPACKET* p = image->pixels;
	register int cnt = image->packets;
	if (plane == PLANE_RED)
		while (cnt-- > 0)
		{
			*data++ = p->red;
			p++;
		}
	else if (plane == PLANE_GREEN)
		while (cnt-- > 0)
		{
			*data++ = p->green;
			p++;
		}
	else if (plane == PLANE_BLUE)
		while (cnt-- > 0)
		{
			*data++ = p->blue;
			p++;
		}
	else if (plane == PLANE_INDEX)
		while (cnt-- > 0)
		{
			*data++ = p->index;
			p++;
		}
	else if (plane == PLANE_ALPHA)
		while (cnt-- > 0)
		{
			*data++ = p->alpha;
			p++;
		}
	else
		return FALSE;

	return TRUE;
}

BOOL __fastcall sg_setplane(SGIMAGE* image, UINT plane, unsigned char* data)
{
	if (!image || !data)
		return FALSE;

	register PIXELPACKET* p = image->pixels;
	register int cnt = image->packets;
	if (plane == PLANE_RED)
		while (cnt-- > 0)
		{
			p->red = *data++;
			p++;
		}
	else if (plane == PLANE_GREEN)
		while (cnt-- > 0)
		{
			p->green = *data++;
			p++;
		}
	else if (plane == PLANE_BLUE)
		while (cnt-- > 0)
		{
			p->blue = *data++;
			p++;
		}
	else if (plane == PLANE_INDEX)
		while (cnt-- > 0)
		{
			p->index = *data++;
			p++;
		}
	else if (plane == PLANE_ALPHA)
		while (cnt-- > 0)
		{
			p->alpha = *data++;
			p++;
		}
	else
		return FALSE;

	return TRUE;
}

SGIMAGE* __fastcall sg_rotate(SGIMAGE* src, BOOL rotate_left)
{
	if (!src)
		return FALSE;

	SGIMAGE* dst = sg_create(src->height, src->width, src->bits);
	if (src->bits < 24)
	{
		memcpy(dst->colormap, src->colormap, src->colors * sizeof(COLORPACKET));
		dst->colors = src->colors;
	}

	PIXELPACKET* p = (PIXELPACKET*)malloc(src->width * sizeof(PIXELPACKET));
	PIXELPACKET* q = (PIXELPACKET*)malloc(src->width * sizeof(PIXELPACKET));
	for (int i = 0; i < src->height; i++)
	{
		if (sg_getrow(src, p, i))
		{
			if (rotate_left)
			{
				for (int x = 0; x < src->width; x++)
					q[src->width - x - 1] = p[x];
				sg_setcol(dst, q, i);
			}
			else
				sg_setcol(dst, p, dst->width - i - 1);
		}
	}
	free(p);
	free(q);
	return dst;
}

void __fastcall sg_rgb2cmyk(PIXELPACKET* pkt)
{
	register int c, m, y, k;
	c = 255 - pkt->red;
	m = 255 - pkt->green;
	y = 255 - pkt->blue;
	if (c < m)
		k = c;
	else
		k = m;
	if (y < k)
		k = y;
	if (k > 0)
	{
		c = c - k;
		m = m - k;
		y = y - k;
	}
	pkt->red = ADJRGB(c);
	pkt->green = ADJRGB(m);
	pkt->blue = ADJRGB(y);
	pkt->index = ADJRGB(k);
}

void __fastcall sg_cmyk2rgb(PIXELPACKET* pkt)
{
	register int c = pkt->red;
	register int m = pkt->green;
	register int y = pkt->blue;
	register int k = pkt->index;
	if (c + k < 255)
		pkt->red = 255 - (c + k);
	else
		pkt->red = 0;

	if (m + k < 255)
		pkt->green = 255 - (m + k);
	else
		pkt->green = 0;

	if (y + k < 255)
		pkt->blue = 255 - (y + k);
	else
		pkt->blue = 0;
}

static int arSharpen[9] = { -1, -1, -1, -1, 24, -1, -1, -1, -1 };

static void __fastcall do_sharpen(PIXELPACKET* p1, PIXELPACKET* p2, PIXELPACKET* p3, PIXELPACKET* pb, int x, int width)
{
	register int red, green, blue;
	red = 0;
	green = 0;
	blue = 0;

	for (int i = -1; i < 2; i++)
	{
		int n;
		if (x + i < 0)
			n = 0;
		else if (x + i >= width)
			n = width - 1;
		else
			n = x + i;

		red += p1[n].red * arSharpen[i + 1] +
					p2[n].red * arSharpen[i + 4] +
					p3[n].red * arSharpen[i + 7];
		green += p1[n].green * arSharpen[i + 1] +
					p2[n].green * arSharpen[i + 4] +
					p3[n].green * arSharpen[i + 7];
		blue += p1[n].blue * arSharpen[i + 1] +
					p2[n].blue * arSharpen[i + 4] +
					p3[n].blue * arSharpen[i + 7];
	}
	pb->blue = ADJRGB(blue >> 4);
	pb->green = ADJRGB(green >> 4);
	pb->red = ADJRGB(red >> 4);
}

void __fastcall sg_sharpen(SGIMAGE* image)
{
	CRect rc(0, 0, image->width, image->height);
	register int x, y;
	int cx = rc.Width();
	int cy = rc.Height();

	PIXELPACKET* line1 = (PIXELPACKET*)malloc(cx * sizeof(PIXELPACKET));
	PIXELPACKET* line2 = (PIXELPACKET*)malloc(cx * sizeof(PIXELPACKET));
	PIXELPACKET* line3 = (PIXELPACKET*)malloc(cx * sizeof(PIXELPACKET));

	PIXELPACKET* pb = image->pixels + rc.top * image->width + rc.left;
	memcpy(line1, pb, cx * sizeof(PIXELPACKET));
	memcpy(line2, pb, cx * sizeof(PIXELPACKET));
	for (y = 0; y < cy; y++)
	{
		pb = image->pixels + (y + rc.top) * image->width + rc.left;
		if (y == cy - 1)
			memcpy(line3, pb, cx * sizeof(PIXELPACKET));
		else
			memcpy(line3, pb + image->width, cx * sizeof(PIXELPACKET));

		for (x = 0; x < cx; x++)
		{
			do_sharpen(line1, line2, line3, pb, x, cx);
			pb++;
		}
		PIXELPACKET* tmp = line1;
		line1 = line2;
		line2 = line3;
		line3 = tmp;
	}
	free(line1);
	free(line2);
	free(line3);
}


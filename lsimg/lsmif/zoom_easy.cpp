///////////////////////////////////////////////////////////////////////
// zoom_easy.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static void __fastcall do_pixel(mif& src, mif& dst, int sx, int sy, int dx, int dy)
{
	parallel_for (0, dy, [&](int y) {
		RGBA* ps = src.m_pixels + MulDiv(y, sy, dy) * sx;
		RGBA* pd = dst.m_pixels + y * dx;
		for (int x = 0; x < dx; x++)
			pd[x] = ps[(x * sx / dx)];
	});
}

static void __fastcall do_near(mif& src, mif& dst, int sx, int sy, int dx, int dy)
{
	RGBA** pa = (RGBA**)malloc(sizeof(RGBA*) * sy);
	for (int y = 0; y < sy; y++)
		pa[y] = src.m_pixels + y * sx;

	parallel_for (0, dy, [&](int y) {
		RGBA* pd = dst.m_pixels + y * dx;
		for (int x = 0; x < dx; x++) {
			float fx = (float)x * (float)sx / (float)dx;
			float fy = (float)y * (float)sy / (float)dy;

			int nx = (int)fx;
			int ny = (int)fy;
			float p = fx - nx;
			float q = fy - ny;
			if (nx >= sx - 1) {
				p = 0.5;
				nx = sx - 2;
			}
			if (ny >= sy - 1) {
				q = 0.5;
				ny = sy - 2;
			}
			if (p >= 0.5)
				nx++;
			if (q >= 0.5)
				ny++;
			pd[x] = pa[ny][nx];
		}
	});
	free(pa);
}

static void __fastcall do_linear(mif& src, mif& dst, int sx, int sy, int dx, int dy)
{
	RGBA** pa = (RGBA**)malloc(sizeof(RGBA*) * sy);
	for (int y = 0; y < sy; y++)
		pa[y] = src.m_pixels + y * sx;

	parallel_for (0, dy, [&](int y) {
		RGBA* pd = dst.m_pixels + y * dx;
		for (int x = 0; x < dx; x++) {
			float fx = (float)x * (float)sx / (float)dx;
			float fy = (float)y * (float)sy / (float)dy;

			int nx = (int)fx;
			int ny = (int)fy;
			float p = fx - nx;
			float q = fy - ny;
			if (nx >= sx - 1) {
				p = 0.5;
				nx = sx - 2;
			}
			if (ny >= sy - 1) {
				q = 0.5;
				ny = sy - 2;
			}

			RGBA s0 = pa[ny][nx];
			RGBA s1 = pa[ny][nx + 1];
			RGBA s2 = pa[ny + 1][nx];
			RGBA s3 = pa[ny + 1][nx + 1];
			pd[x].red = (BYTE)((1 - q) * ((1 - p) * s0.red + p * s1.red) + q * ((1 - p) * s2.red + p * s3.red));
			pd[x].green = (BYTE)((1 - q) * ((1 - p) * s0.green + p * s1.green) + q * ((1 - p) * s2.green + p * s3.green));
			pd[x].blue = (BYTE)((1 - q) * ((1 - p) * s0.blue + p * s1.blue) + q * ((1 - p) * s2.blue + p * s3.blue));
		}
	});
	free(pa);
}

BOOL zoom_pixel(mif& src, int cx, int cy)
{
	mif dst;
	dst.create(cx, cy, src.m_bits);
	if (src.m_bits <= 8) {
		dst.m_colors = src.m_colors;
		memcpy(dst.m_map, src.m_map, src.m_colors * sizeof(RGBA));
	}
	do_pixel(src, dst, src.m_width, src.m_height, cx, cy);
	dst.validate_rgb();
	src.movefrom(&dst);
	return TRUE;
}

BOOL zoom_near(mif& src, int cx, int cy)
{
	mif dst;
	dst.create(cx, cy, src.m_bits);
	if (src.m_bits <= 8) {
		dst.m_colors = src.m_colors;
		memcpy(dst.m_map, src.m_map, src.m_colors * sizeof(RGBA));
	}
	do_near(src, dst, src.m_width, src.m_height, cx, cy);
	dst.validate_rgb();
	src.movefrom(&dst);
	return TRUE;
}

BOOL zoom_linear(mif& src, int cx, int cy)
{
	src.m_bits = 24;
	mif dst;
	dst.create(cx, cy, 24);
	do_linear(src, dst, src.m_width, src.m_height, cx, cy);
	src.movefrom(&dst);
	return TRUE;
}

BOOL zoom_inframe(mif& src, int cx, int cy)
{
	int width = src.m_width;
	int height = src.m_height;

	if (cx >= width && cy >= height)
		return TRUE;

/* @@@
	int dx, dy;
	for (float f = 1.05F; ; f += 0.05F)
	{
		dx = (int)((float)width / f);
		dy = (int)((float)height / f);
		if (dx <= cx && dy <= cy)
			break;
	}
@@@ */
	double xpar = (double)cx / (double)width;
	double ypar = (double)cy / (double)height;
	if (xpar > 1.0) xpar = 1.0;
	if (ypar > 1.0) ypar = 1.0;
	xpar = (xpar < ypar) ? xpar : ypar;
	int dx = (int)floor(width * xpar);
	int dy = (int)floor(height * xpar);
	if (src.m_bits < 24)
		src.m_bits = 24;
	return zoom_linear(src, dx, dy);
}

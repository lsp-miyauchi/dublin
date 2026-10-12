///////////////////////////////////////////////////////////////////////
// mblur.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define MBLUR_LINEAR	0
#define MBLUR_RADIAL	1
#define MBLUR_ZOOM		2

static void __fastcall get_pixel(mif* image, int x, int y, RGBA* pixel)
{
	if (x < 0 || x >= image->m_width || y < 0 || y >= image->m_height)
		pixel->red = pixel->green = pixel->blue = 0;
	else
		*pixel = *(image->pos(x, y));
}

static void __fastcall halftonepacket(RGBA* pd, RGBA* ps)
{
	pd->red = (pd->red + ps->red) >> 1;
	pd->green = (pd->green + ps->green) >> 1;
	pd->blue = (pd->blue + ps->blue) >> 1;
}

BOOL mblur_linear(mif& src, LPRECT prc, int length, int angle)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	RGBA pixel;
	int	sum_r, sum_g, sum_b;

	int x, y, i, xx, yy;
	int dx, dy, swapdir, err, e, s1, s2;

	int n = length;
	int px = (int)(n * cos(angle / 180.0 * M_PI));
	int py = (int)(n * sin(angle / 180.0 * M_PI));

	// Initialization for Bresenham algorithm:
	// dx = abs(x2 - x1), s1 = sign(x2 - x1)
	// dy = abs(y2 - y1), s2 = sign(y2 - y1)
	if ((dx = px) != 0)
	{
		if (dx < 0)
		{
			dx = -dx;
			s1 = -1;
		}
		else
			s1 = 1;
	}
	else
		s1 = 0;

	if ((dy = py) != 0)
	{
		if (dy < 0)
		{
			dy = -dy;
			s2 = -1;
		}
		else
			s2 = 1;
	}
	else
		s2 = 0;

	if (dy > dx)
	{
		swapdir = dx;
		dx = dy;
		dy = swapdir;
		swapdir = 1;
	}
	else
		swapdir = 0;

	dy <<= 1;
	err = dy - dx;			// Initial error term
	dx <<= 1;

	mif dst;
	dst.create(rc.Width(), rc.Height(), 24);

	RGBA* pd = dst.m_pixels;
	for (y = rc.top; y < rc.bottom; y++)
	{
		for (x = rc.left; x < rc.right; x++)
		{
			xx = x;
			yy = y;
			e = err;
			sum_r = sum_g = sum_b = 0;

			for (i = 0; i < n; )
			{
				get_pixel(&src, xx, yy, &pixel);
				sum_r += pixel.red;
				sum_g += pixel.green;
				sum_b += pixel.blue;
				i++;

				while (e >= 0)
				{
					if (swapdir)
						xx += s1;
					else
						yy += s2;
					e -= dx;
				}
				if (swapdir)
					yy += s2;
				else
					xx += s1;
				e += dy;
				if (xx < 0 || xx >= src.m_width || yy < 0 || yy >= src.m_height)
					break;
			}

			if (i == 0)
				get_pixel(&src, xx, yy, pd);
			else
			{
				pd->red = sum_r / i;
				pd->green = sum_g / i;
				pd->blue = sum_b / i;
			}
			pd++;
		}
	}
	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
	return TRUE;
}

BOOL mblur_radial(mif& src, LPRECT prc, int angle)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	double cen_x = (double)(rc.left + rc.right - 1.0) / 2.0;
	double cen_y = (double)(rc.top + rc.bottom - 1.0) / 2.0;

	RGBA *pd;
	RGBA pixel;
	int	sum_r, sum_g, sum_b;

	int x, y;
	int count, r, step;

	double xx = 0.0;
	double yy = 0.0;

	double xangle = angle / 180.0 * M_PI;
	int w = (int)MAX(src.m_width - cen_x, cen_x);
	int h = (int)MAX(src.m_height - cen_y, cen_y);
	int R = (int)sqrt((double)(w * w + h * h));
	int n = (int)(4 * xangle * sqrt((double)R) + 2);
	double theta = xangle / ((double)(n - 1));

	double* ct = (double*)malloc(n * sizeof(double));
	double* st = (double*)malloc(n * sizeof(double));

	double offset = theta * (n - 1) / 2;
	int i;
	for (i = 0; i < n; ++i) {
		ct[i] = cos(theta * i - offset);
		st[i] = sin(theta * i - offset);
	}

	mif dst;
	dst.create(rc.Width(), rc.Height(), 24);

	pd = dst.m_pixels;
	for (y = rc.top; y < rc.bottom; y++) {
		for (x = rc.left; x < rc.right; x++) {
			double xr = x - cen_x;
			double yr = y - cen_y;
			r = (int)sqrt(xr * xr + yr * yr);
			if (r == 0)
				step = 1;
			else if ((step = R / r) == 0)
				step = 1;
			else if (step > n - 1)
				step = n - 1;

			sum_r = sum_g = sum_b = 0;
			for (i = 0, count = 0; i < n; i += step) {
				xx = cen_x + xr * ct[i] - yr * st[i];
				yy = cen_y + xr * st[i] + yr * ct[i];
				if ((yy < 0) || (yy >= src.m_height) ||
					(xx < 0) || (xx >= src.m_width))
					continue;

				count++;
				get_pixel(&src, (int)xx, (int)yy, &pixel);
				sum_r += pixel.red;
				sum_g += pixel.green;
				sum_b += pixel.blue;
			}

			if (count == 0)
				get_pixel(&src, (int)xx, (int)yy, pd);
			else {
				pd->red = sum_r / count;
				pd->green = sum_g / count;
				pd->blue = sum_b / count;
			}
			pd++;
		}
	}
	free(ct);
	free(st);

	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
	return TRUE;
}

BOOL mblur_zoom(mif& src, LPRECT prc, int length)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	RGBA *pd;
	RGBA pixel;
	int	sum_r, sum_g, sum_b;

	double cen_x = (double)(rc.left + rc.right - 1.0) / 2.0;
	double cen_y = (double)(rc.top + rc.bottom - 1.0) / 2.0;

	int x, y, i, xx, yy;

	// initialize
	xx = 0;
	yy = 0;

	int n = length;
	double f = 0.02;

	mif dst;
	dst.create(rc.Width(), rc.Height(), 24);

	pd = dst.m_pixels;
	for (y = rc.top; y < rc.bottom; y++)
	{
		for (x = rc.left; x < rc.right; x++)
		{
			sum_r = sum_g = sum_b = 0;

			for (i = 0; i < n; ++i)
			{
				xx = (int)(cen_x + (x - cen_x) * (1.0 + f * i));
				yy = (int)(cen_y + (y - cen_y) * (1.0 + f * i));

				if (yy < 0 || yy >= src.m_height || xx < 0 || xx >= src.m_width)
					break;

				get_pixel(&src, xx, yy, &pixel);
				sum_r += pixel.red;
				sum_g += pixel.green;
				sum_b += pixel.blue;
			}

			if (i == 0)
				get_pixel(&src, xx, yy, pd);
			else
			{
				pd->red = sum_r / i;
				pd->green = sum_g / i;
				pd->blue = sum_b / i;
			}
			pd++;
		}
	}
	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
	return TRUE;
}

BOOL mblur_round(mif& src, LPRECT prc, int nRotate)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;

	src.m_bits = 24;

	int dx = rc.Width();
	int dy = rc.Height();

	register int x, y;

	int x_center = dx / 2;
	int y_center = dy / 2;

	double degrees, sIn, cOs, rot_x, rot_y;

	register RGBA* pd;
	int r = 0;
	while (r != nRotate)
	{
		r += (nRotate < 0) ? -1 : 1;
		degrees = DEG2RAD(r);
		sIn = sin(degrees);
		cOs = cos(degrees);
		for (y = 0; y < dy; y++)
		{
			pd = src.pos(rc.left, y + rc.top);
			for (x = 0; x < dx; x++)
			{
				rot_x = (x - x_center) * cOs - (y - y_center) * sIn;
				rot_y = (x - x_center) * sIn + (y - y_center) * cOs;
				rot_x += dx / 2;
				rot_y += dy / 2;
				if (0 <= (int)rot_x && (int)rot_x < dx &&
					0 <= (int)rot_y && (int)rot_y < dy)
					halftonepacket(pd, src.pos(rc.left + (int)rot_x, rc.top + (int)rot_y));
				pd++;
			}
		}
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// gauss_rle.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

/*
 * The equations: g(r) = exp (- r^2 / (2 * sigma^2))
 *                   r = sqrt (x^2 + y ^2)
 */

static int* make_curve(double sigma, int* length, int** curve_base)
{
	int temp;
	int i, n;

	double sigma2 = 2 * sigma * sigma;
	double l = sqrt(-sigma2 * log(1.0 / 255.0));

	n = (int)(ceil(l) * 2);
	if ((n % 2) == 0)
		n += 1;

	*curve_base = (int*)malloc(sizeof(int) * n);
	int* curve = *curve_base;
	*length = n / 2;
	curve += *length;
	curve[0] = 255;

	for (i = 1; i <= *length; i++) {
		temp = (int)(exp (- (i * i) / sigma2) * 255);
		curve[-i] = temp;
		curve[i] = temp;
	}
	return curve;
}

static void run_length_encode(BYTE* src, int* dest, int width)
{
	BYTE last = *src;
	src += 4;
	int start = 0;

	int i, j;
	for (i = 1; i < width; i++) {
		if (*src != last) {
			for (j = start; j < i; j++) {
				*dest++ = (i - j);
				*dest++ = last;
			}
			start = i;
			last = *src;
		}
		src += 4;
	}

	for (j = start; j < i; j++) {
		*dest++ = (i - j);
		*dest++ = last;
	}
}

BOOL blur_gauss_rle_plane(mif& msrc, LPRECT prc, double radius, UINT plane)
{
	CRect rc;
	if (!msrc.validaterect(prc, &rc))
		return FALSE;

	if (msrc.m_bits < 24)
		msrc.m_bits = 24;

	mif mdst;
	mdst.copyfrom(&msrc);

	int i, length, *curve_base;
	int* curve = make_curve(radius, &length, &curve_base);
	int* sum_p = (int*)malloc(sizeof(int) * (2 * length + 1));
	sum_p[0] = 0;

	for (i = 1; i <= length * 2; i++)
		sum_p[i] = curve[i - length - 1] + sum_p[i - 1];
	int* sum = sum_p + length;
	free(curve_base);

	int width = rc.Width();
	int height = rc.Height();

	int* buf = (int*)malloc(sizeof(int) * max(width, height) * 2);
	int total = sum[length] - sum[-length];

	// allocate buffers for source and destination pixels
	RGBA* src = (RGBA*)malloc(max(width, height) * sizeof(RGBA));
	RGBA* dst = (RGBA*)malloc(max(width, height) * sizeof(RGBA));
	BYTE* sp;
	BYTE* dp;
	int* bb;
	int initial_p, initial_m;
	int col, row, b, start, end, val, pixels;

	//
	// Blur in horizontal direction
	// img_s --> img_d
	//

msrc.progress_init((width + height) >> 3);
int prog = 0;

	for (col = 0; col < width; col++) {
		msrc.getcol(src, col + rc.left, rc.top, height);
		memcpy(dst, src, sizeof(RGBA) * height);
		sp = (BYTE*)src;
		dp = (BYTE*)dst;

		for (b = 0; b < sizeof(RGBA); b++) {
			if (b == 0 && !(plane & bltRed))	continue;
			if (b == 1 && !(plane & bltGreen))	continue;
			if (b == 2 && !(plane & bltBlue))	continue;
			if (b == 3 && !(plane & bltAlpha))	continue;

			initial_p = sp[b];
			initial_m = sp[(height - 1) * sizeof(RGBA) + b];

			// Determine a run-length encoded version of the row
			run_length_encode(sp + b, buf, height);

			for (row = 0; row < height; row++) {
				start = (row < length) ? -row : -length;
				end = (height <= (row + length)) ? (height - row - 1) : length;

				val = 0;
				i = start;
				bb = buf + (row + i) * 2;

				if (start != -length)
					val += initial_p * (sum[start] - sum[-length]);

				while (i < end) {
					pixels = bb[0];
					i += pixels;
					if (i > end)
						i = end;
					val += bb[1] * (sum[i] - sum[start]);
						bb += (pixels * 2);
					start = i;
				}

				if (end != length)
					val += initial_m * (sum[length] - sum[end]);

				dp[row * sizeof(RGBA) + b] = val / total;
			}
		}
		mdst.setcol(dst, col + rc.left, rc.top, height);
if ((++prog & 7) == 0) msrc.progress_continue();
	}

	//
	// Blur in vertical direction
	// img_d --> img_s
	//
	for (row = 0; row < height; row++) {
		mdst.getrow(src, rc.left, row + rc.top, width);
		memcpy(dst, src, sizeof(RGBA) * width);
		sp = (BYTE*)src;
		dp = (BYTE*)dst;

		for (b = 0; b < sizeof(RGBA); b++) {
			if (b == 0 && !(plane & bltRed))	continue;
			if (b == 1 && !(plane & bltGreen))	continue;
			if (b == 2 && !(plane & bltBlue))	continue;
			if (b == 3 && !(plane & bltAlpha))	continue;

			initial_p = sp[b];
			initial_m = sp[(width-1) * sizeof(RGBA) + b];

			// Determine a run-length encoded version of the row
			run_length_encode(sp + b, buf, width);

			for (col = 0; col < width; col++) {
				start = (col < length) ? -col : -length;
				end = (width <= (col + length)) ? (width - col - 1) : length;

				val = 0;
				i = start;
				bb = buf + (col + i) * 2;

				if (start != -length)
					val += initial_p * (sum[start] - sum[-length]);

				while (i < end) {
					pixels = bb[0];
					i += pixels;
					if (i > end)
						i = end;

					val += bb[1] * (sum[i] - sum[start]);
					bb += (pixels * 2);
					start = i;
				}

				if (end != length)
					val += initial_m * (sum[length] - sum[end]);

				dp[col * sizeof(RGBA) + b] = val / total;
			}
		}
		msrc.setrow(dst, rc.left, row + rc.top, width);
if ((++prog & 7) == 0) msrc.progress_continue();
	}
msrc.progress_term();

	// free buffers
	free(sum_p);
	free(buf);
	free(src);
	free(dst);

	return TRUE;
}

BOOL blur_gauss_rle(mif& msrc, LPRECT prc, double radius)
{
	return blur_gauss_rle_plane(msrc, prc, radius, bltRed | bltGreen | bltBlue);
}

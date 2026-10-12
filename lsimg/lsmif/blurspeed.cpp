////////////////////////////////////////////////////////////////////////
// blurspeed.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"
#include <stdlib.h>


#define COMPARE_WIDTH	3

/* @@@
#define MIN_THRESHOLD	0
#define MAX_THRESHOLD	50
#define MIN_STRENGTH	1
#define MAX_STRENGTH	50

#define NEGATIVE_STRENGTH_TEXT "\n   Wind Strength must be greater than 0.   \n"
#define THRESHOLD_TEXT "Higher values restrict the effect to fewer areas of the image"
#define STRENGTH_TEXT "Higher values increase the magnitude of the effect"
#define WIND_TEXT "A fine grained algorithm"
#define BLAST_TEXT "A coarse grained algorithm"
#define LEFT_TEXT "Makes the wind come from the left"
#define RIGHT_TEXT "Makes the wind come from the right"
#define LEADING_TEXT "The effect is applied at the leading edge of objects"
#define TRAILING_TEXT "The effect is applied at the trailing edge of objects"
#define BOTH_TEXT "The effect is applied at both edges of objects"

typedef struct {
	int threshold;			// 0 - 50 derivative comparison for edge detection
							// Controls where blending will be done
	int direction;			// of wind, LEFT or RIGHT
	int strength;			// 1 - 50 how many pixels to bleed
							// Controls the extent of the blending
	int alg;				// which algorithm
	int edge;				// controls abs, negation of derivative
} config_t;

config_t config =
{
	10,							// threshold for derivative edge detection
	WINDBLUR_LEFT,				// bleed to the right
	10,							// how many pixels to bleed
	WINDBLUR_WIND,				// default algorithm
	WINDBLUR_LEADING			// abs(derivative)
};

@@@ */

extern void __fastcall rgba_reverse_buffer(RGBA *buffer, int length);


static void get_derivative(RGBA *ps, RGBA *pd, int edge, int& r, int& g, int& b)
{
	r = (int)pd->red - (int)ps->red;
	g = (int)pd->green - (int)ps->green;
	b = (int)pd->blue - (int)ps->blue;

	if (edge == sblurBoth) {
		r = abs(r);
		g = abs(g);
		b = abs(b);
	}
	else if (edge == sblurLeading) {
		r = -r;
		g = -g;
		b = -b;
	}
	else if (edge == sblurTrailing) {
		// no change needed
	}
	return;
}

static int exceeded(RGBA* ps, RGBA* pd, int edge, int threshold)
{
	int r, g, b;
	get_derivative(ps, pd, edge, r, g, b);
	return (((r + g + b) / 3) > threshold) ? 1 : 0;
}

static int blast_row(RGBA* pixels, int lpi, int threshold, int strength, int edge)
{
	int skip = 0;
	for (int j = 0; j < lpi; j++) {
		if (exceeded(pixels + j, pixels + j + 1, edge, threshold)) {
			// we have found an edge, do bleeding
			int weight = rand() % 10;
			int random_factor = (weight > 5) ? 2 : ((weight > 3) ? 3 : 4);
			int bleed_length = 0;
			switch (rand() % random_factor) {
				case 3:
					bleed_length += strength;
					// fall through to add up multiples of strength
				case 2:
					bleed_length += strength;
					// fall through
				case 1:
					bleed_length += strength;
					// fall through
				case 0:
					bleed_length += strength;
					// fall through
			}

			int lbi = j + bleed_length;
			if (lbi > lpi)
				lbi = lpi;

			for (int i = j; i < lbi; i++)
				pixels[i] = pixels[j];

			j = lbi - 1;
			if ((rand() % 10) > 7)
				skip = 1;
		}
	}
	return skip;
}

static void wind_row(RGBA *sb, int lpi, int threshold, int strength, int edge)
{
	const int comp_stride = COMPARE_WIDTH;
	for (int j = 0; j < lpi; j++) {
		if (exceeded(sb + j, sb + j + comp_stride, edge, threshold)) {
			// we have found an edge, do bleeding
			int sbi = j + comp_stride;				// starting bleed index
			int blend_color_R = sb[j].red;
			int blend_color_G = sb[j].green;
			int blend_color_B = sb[j].blue;
			int target_color_R = sb[sbi].red;
			int target_color_G = sb[sbi].green;
			int target_color_B = sb[sbi].blue;
			double bleed_length_max = strength;

			if (rand() % 3)							// introduce weighted randomness
				bleed_length_max = strength;
			else
				bleed_length_max = 4 * strength;

			int bleed_variation = 1 + (int)(bleed_length_max * rand() / (RAND_MAX + 1.0));
			int lbi = sbi + bleed_variation;		// last bleed index
			if (lbi > lpi)
				lbi = lpi;							// stop overunning the buffer

			int bleed_length = bleed_variation;
			int blend_amt_R = target_color_R - blend_color_R;
			int blend_amt_G = target_color_G - blend_color_G;
			int blend_amt_B = target_color_B - blend_color_B;
			double denominator = bleed_length * bleed_length + bleed_length;
			denominator = 2.0 / denominator;
			int n = bleed_length;
			for (int i = sbi; i < lbi; i++) {
				// check against original colour
				if (!exceeded(sb + j, sb + i, edge, threshold) && (rand() % 2))
					break;

				blend_color_R += (int)(blend_amt_R * n * denominator);
				blend_color_G += (int)(blend_amt_G * n * denominator);
				blend_color_B += (int)(blend_amt_B * n * denominator);

				blend_color_R = ADJRGB(blend_color_R);
				blend_color_G = ADJRGB(blend_color_G);
				blend_color_B = ADJRGB(blend_color_B);

				sb[i].red = (BYTE)((blend_color_R * 2 + sb[i].red) / 3);
				sb[i].green = (BYTE)((blend_color_G * 2 + sb[i].green) / 3);
				sb[i].blue = (BYTE)((blend_color_B * 2 + sb[i].blue) / 3);

				if (exceeded(sb + i, sb + i + comp_stride, sblurBoth, threshold)) {
					target_color_R = sb[i+comp_stride].red;
					target_color_G = sb[i+comp_stride].green;
					target_color_B = sb[i+comp_stride].blue;
					blend_amt_R = target_color_R - blend_color_R;
					blend_amt_G = target_color_G - blend_color_G;
					blend_amt_B = target_color_B - blend_color_B;
					denominator = n * n + n;
					denominator = 2.0 / denominator;
				}
				n--;
			}
		}
	}
}

BOOL sblur_blast(mif& src, LPRECT prc, int threshold, int strength, int direction, int edge)
{
	srand(0);

	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	int width = rc.Width();
	int height = rc.Height();
	mif dst(width, height, 24);

	RGBA* pix_ptr = new RGBA[width * height];
	if (direction == sblurLeft || direction == sblurRight) {
		parallel_for (rc.top, rc.bottom, [&](int row) {
			RGBA* pix = pix_ptr + (row * width);
			src.getrow(pix, rc.left, row, width);
			if (direction == sblurRight)
				rgba_reverse_buffer(pix, width);

			int marker = blast_row(pix, width - 1, threshold, strength, edge);
			if (direction == sblurRight)
				rgba_reverse_buffer(pix, width);

			dst.setrow(pix, 0, row - rc.top, width);
			if (marker) {
				int limit = 1 + rand() % 2;
				for (int j = 0; (j < limit) && (row < rc.bottom); j++) {
					row++;
					if (row < rc.bottom) {
						src.getrow(pix, rc.left, row, rc.Width());
						dst.setrow(pix, 0, row - rc.top, rc.Width());
					}
				}
			}
		});
	}
	else {
		parallel_for (rc.left, rc.right, [&](int col) {
			RGBA* pix = pix_ptr + (col * height);
			src.getcol(pix, col, rc.top, height);
			if (direction == sblurBottom)
				rgba_reverse_buffer(pix, height);

			int marker = blast_row(pix, height - 1, threshold, strength, edge);
			if (direction == sblurBottom)
				rgba_reverse_buffer(pix, height);

			dst.setcol(pix, col - rc.left, 0, height);
			if (marker) {
				int limit = 1 + rand() % 2;
				for (int j = 0; (j < limit) && (col < rc.bottom); j++) {
					col++;
					if (col < rc.right) {
						src.getcol(pix, col, rc.top, height);
						dst.setcol(pix, col - rc.left, 0, height);
					}
				}
			}
		});
	}
	delete [] pix_ptr;

	src.blt(rc.left, rc.top, width, height, &dst, 0, 0);
	return TRUE;
}

BOOL sblur_wind(mif& src, LPRECT prc, int threshold, int strength, int direction, int edge)
{
	srand(0);

	CRect rc;
	if (!src.validaterect(prc, &rc))
		return NULL;
	src.m_bits = 24;

	int width = rc.Width();
	int height = rc.Height();

	RGBA* pix_ptr = new RGBA[width * height];
	if (direction == sblurLeft || direction == sblurRight) {
		int lpi = width - COMPARE_WIDTH;
		parallel_for (rc.top, rc.bottom, [&](int row) {
			RGBA* pix = pix_ptr + (row * width);
			src.getrow(pix, rc.left, row, width);
			if (direction == sblurRight)
				rgba_reverse_buffer(pix, width);

			wind_row(pix, lpi, threshold, strength, edge);
			if (direction == sblurRight)
				rgba_reverse_buffer(pix, width);

			src.setrow(pix, rc.left, row, width);
		});
	}
	else {
		int lpi = height - COMPARE_WIDTH;
		parallel_for (rc.left, rc.right, [&](int col) {
			RGBA* pix = pix_ptr + (col * height);
			src.getcol(pix, col, rc.top, height);
			if (direction == sblurBottom)
				rgba_reverse_buffer(pix, height);

			wind_row(pix, lpi, threshold, strength, edge);
			if (direction == sblurBottom)
				rgba_reverse_buffer(pix, height);

			src.setcol(pix, col, rc.top, height);
		});
	}
	delete [] pix_ptr;
	return TRUE;
}

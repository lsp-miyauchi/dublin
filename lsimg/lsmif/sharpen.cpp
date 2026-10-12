////////////////////////////////////////////////////////////////////////
// sharpen.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static BYTE neg_lut[256];					// Negative coefficient LUT
static int	pos_lut[256];					// Positive coefficient LUT

static void compute_luts(int sharpen_percent)
{
	int fact = 100 - sharpen_percent;
	if (fact < 1)
		fact = 1;

	for (int i = 0; i < 256; i ++)
	{
		pos_lut[i] = 100 * i / fact;
		neg_lut[i] = sharpen_percent * i / 8 / fact;
	}
}

static void rgb_filter(int    width,	/* I - Width of line in pixels */
           BYTE *src,		/* I - Source line */
           BYTE *dst,		/* O - Destination line */
           BYTE *neg0,	/* I - Top negative coefficient line */
           BYTE *neg1,	/* I - Middle negative coefficient line */
           BYTE *neg2)	/* I - Bottom negative coefficient line */
{
	int pixel;		/* New pixel value */

	*dst++ = *src++;
	*dst++ = *src++;
	*dst++ = *src++;
	width -= 2;

	while (width > 0)
	{
		pixel = pos_lut[*src++] - neg0[-3] - neg0[0] - neg0[3] -
							neg1[-3] - neg1[3] - neg2[-3] - neg2[0] - neg2[3];
		if (pixel < 0)
			*dst++ = 0;
		else if (pixel < 255)
			*dst++ = pixel;
		else
			*dst++ = 255;

		pixel = pos_lut[*src++] - neg0[-2] - neg0[1] - neg0[4] -
							neg1[-2] - neg1[4] - neg2[-2] - neg2[1] - neg2[4];
		if (pixel < 0)
			*dst++ = 0;
		else if (pixel < 255)
			*dst++ = pixel;
		else
			*dst++ = 255;

		pixel = pos_lut[*src++] - neg0[-1] - neg0[2] - neg0[5] -
							neg1[-1] - neg1[5] - neg2[-1] - neg2[2] - neg2[5];
		if (pixel < 0)
			*dst++ = 0;
		else if (pixel < 255)
			*dst++ = pixel;
		else
			*dst++ = 255;

		neg0 += 3;
		neg1 += 3;
		neg2 += 3;
		width--;
	}

	*dst++ = *src++;
	*dst++ = *src++;
	*dst++ = *src++;
}

BOOL sharpen2(mif& src, LPRECT prc, int per)
{
	CRect rc;
	if (per < 1 || per > 99 || !src.validaterect(prc, &rc))
		return FALSE;

	BYTE* src_rows[4], *src_ptr, *dst_row, *neg_rows[4], *neg_ptr;

	compute_luts(per);

	int sel_width = rc.Width();
	int width = sel_width * 3;

	BYTE* work_base = (BYTE*)malloc(width * 9 * sizeof(BYTE));
	int row;
	for (row = 0; row < 4; row ++) {
		src_rows[row] = work_base + width * row * 2;
		neg_rows[row] = work_base + width * (row * 2 + 1);
	}
	dst_row = work_base + width * 8;

	// Pre-load the first row for the filter...
	src.getrow_rgb(src_rows[0], rc.left, rc.top, sel_width);
	src_ptr = src_rows[0];
	neg_ptr = neg_rows[0];
	int i;
	for (i = width; i > 0; i--) {
		*neg_ptr = neg_lut[*src_ptr];
		src_ptr++;
		neg_ptr++;
	}

	row = 1;
	int count = 1;

	int y;
	for (y = rc.top; y < rc.bottom; y ++) {
		if ((y + 1) < rc.bottom) {
			// Check to see if our src_rows[] array is overflowing yet...
			if (count >= 3)
				count--;

			// Grab the next row...
			src.getrow_rgb(src_rows[row], rc.left, y + 1, sel_width);
			src_ptr = src_rows[row];
			neg_ptr = neg_rows[row];
			for (i = width; i > 0; i--) {
				*neg_ptr = neg_lut[*src_ptr];
				src_ptr++;
				neg_ptr++;
			}
			count++;
			row = (row + 1) & 3;
		}
		else {
			// No more pixels at the bottom...  Drop the oldest samples...
			count--;
		}

		// Now sharpen pixels and save the results...
		if (count == 3) {
			rgb_filter(sel_width,
						src_rows[(row + 2) & 3], dst_row,
						neg_rows[(row + 1) & 3] + 3,
						neg_rows[(row + 2) & 3] + 3,
						neg_rows[(row + 3) & 3] + 3);

			// Set the row...
			src.setrow_rgb(dst_row, rc.left, y, sel_width);
		}
		else if (count == 2) {
			if (y == rc.top)
				src.setrow_rgb(src_rows[0], rc.left, y, sel_width);
			else
				src.setrow_rgb(src_rows[2], rc.left, y, sel_width);
		}
	}
	free(work_base);
	return TRUE;
}

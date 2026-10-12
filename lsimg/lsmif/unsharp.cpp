/* $Id: unsharp.c,v 1.3 1999/07/11 18:53:53 neo Exp $
 * unsharp.c 0.8 -- This is a plug-in for the GIMP 1.0
 *  http://www.steppe.com/~winston/gimp/unsharp.html
 *
 * Copyright (C) 1999 Winston Chang
 */

////////////////////////////////////////////////////////////////////////
// unsharp.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static _inline int round2int(double i)
	{ return (int)(i + 0.5); }

/* generates a 1-D convolution matrix to be used for each pass of 
 * a two-pass gaussian blur.  Returns the length of the matrix.
 */
static int __fastcall gen_convolve_matrix(double radius, double** cmatrix_p)
{
	int matrix_length;
	int matrix_midpoint;
	double* cmatrix;
	int i,j;
	double std_dev;
	double sum;
	
	/* we want to generate a matrix that goes out a certain radius
	 * from the center, so we have to go out ceil(rad-0.5) pixels,
	 * inlcuding the center pixel.  Of course, that's only in one direction,
	 * so we have to go the same amount in the other direction, but not count
	 * the center pixel again.  So we double the previous result and subtract
	 * one.
     * The radius parameter that is passed to this function is used as
	 * the standard deviation, and the radius of effect is the
	 * standard deviation * 2.  It's a little confusing.
	 */
	radius = fabs(radius) + 1.0;

	std_dev = radius;
	radius = std_dev * 2;

	/* go out 'radius' in each direction */
	matrix_length = (int)(2 * ceil(radius - 0.5) + 1);
	if (matrix_length <= 0)
		matrix_length = 1;
	matrix_midpoint = matrix_length/2 + 1;
	*cmatrix_p = new double[matrix_length];
	cmatrix = *cmatrix_p;

	/*  Now we fill the matrix by doing a numeric integration approximation
	 * from -2*std_dev to 2*std_dev, sampling 50 points per pixel.
	 * We do the bottom half, mirror it to the top half, then compute the
	 * center point.  Otherwise asymmetric quantization errors will occur.
	 *  The formula to integrate is e^-(x^2/2s^2).
	 */

	/* first we do the top (right) half of matrix */
	for (i=matrix_length/2 + 1; i<matrix_length; i++) {
		double base_x = i - floor((double)(matrix_length/2)) - 0.5;
		sum = 0;
		for (j=1; j<=50; j++) {
			if ( base_x+0.02*j <= radius ) 
				sum += exp( -(base_x+0.02*j)*(base_x+0.02*j) / (2*std_dev*std_dev) );
		}
		cmatrix[i] = sum/50;
	}

	/* mirror the thing to the bottom half */
	for (i=0; i<=matrix_length/2; i++) {
		cmatrix[i] = cmatrix[matrix_length-1-i];
	}
	
	/* find center val -- calculate an odd number of quanta to make it symmetric,
	 * even if the center point is weighted slightly higher than others. */
	sum = 0;
	for (j=0; j<=50; j++) {
		sum += exp( -(0.5+0.02*j)*(0.5+0.02*j) / (2*std_dev*std_dev) );
	}
	cmatrix[matrix_length/2] = sum/51;

	
	/* normalize the distribution by scaling the total sum to one */
	sum=0;
	for (i=0; i<matrix_length; i++) sum += cmatrix[i];
	for (i=0; i<matrix_length; i++) cmatrix[i] = cmatrix[i] / sum;

	return matrix_length;
}

/* ----------------------- gen_lookup_table ----------------------- */
/* generates a lookup table for every possible product of 0-255 and
   each value in the convolution matrix.  The returned array is
   indexed first by matrix position, then by input multiplicand (?)
   value.
*/
static double* gen_lookup_table(double* cmatrix, int cmatrix_length)
{
	double* lookup_table = new double[cmatrix_length * 256];
	register double* lookup_table_p = lookup_table;
	register double* cmatrix_p      = cmatrix;
	register int i, j;
	for (i = 0; i < cmatrix_length; i++) {
		for (j = 0; j < 256; j++)
			*(lookup_table_p++) = *cmatrix_p * (double)j;
		cmatrix_p++;
	}
	return lookup_table;
}

/* this function is written as if it is blurring a column at a time,
   even though it can operate on rows, too.  There is no difference
   in the processing of the lines, at least to the blur_line function. */
static _inline void blur_line(double* ctable, double* cmatrix,
         int cmatrix_length, RGBA* cur_col, RGBA* dest_col, int y)
{
	double scale;
	double sum_r, sum_g, sum_b;
	int j;
	int row;

	if (cmatrix_length > y) {
		for (row = 0; row < y ; row++) {
			scale = 0;
			for (j = 0; j < y ; j++) {
				if ((j + cmatrix_length/2 - row >= 0) &&
					(j + cmatrix_length/2 - row < cmatrix_length))
					scale += cmatrix[j + cmatrix_length/2 - row];
			}

			sum_r = sum_g = sum_b = 0;
			for (j = 0; j < y; j++) {
				if ((j >= row - cmatrix_length/2) &&
					(j <= row + cmatrix_length/2))
				{
					sum_r += cur_col[j].red * cmatrix[j];
					sum_g += cur_col[j].green * cmatrix[j];
					sum_b += cur_col[j].blue * cmatrix[j];
				}
			}
			dest_col[row].red = (BYTE)round2int(sum_r / scale);
			dest_col[row].green = (BYTE)round2int(sum_g / scale);
			dest_col[row].blue = (BYTE)round2int(sum_b / scale);
		}
	}
	else {
		/* when the cmatrix is smaller than row length */
		/* for the edge condition, we only use available info, and scale to one */
		for (row = 0; row < cmatrix_length/2; row++) {
			/* find scale factor */
			scale=0;
			for (j = cmatrix_length/2 - row; j<cmatrix_length; j++)
				scale += cmatrix[j];

			sum_r = sum_g = sum_b = 0;
			for (j = cmatrix_length/2 - row; j<cmatrix_length; j++) {
				sum_r += cur_col[(row + j-cmatrix_length/2)].red * cmatrix[j];
				sum_g += cur_col[(row + j-cmatrix_length/2)].green * cmatrix[j];
				sum_b += cur_col[(row + j-cmatrix_length/2)].blue * cmatrix[j];
			}
			dest_col[row].red = (BYTE)round2int(sum_r / scale);
			dest_col[row].green = (BYTE)round2int(sum_g / scale);
			dest_col[row].blue = (BYTE)round2int(sum_b / scale);
		}

		/* go through each pixel in each col */
		for (; row < y-cmatrix_length/2; row++) {
			sum_r = sum_g = sum_b = 0;
			for (j = 0; j<cmatrix_length; j++)
			{
				sum_r += cur_col[(row + j-cmatrix_length/2)].red * cmatrix[j];
				sum_g += cur_col[(row + j-cmatrix_length/2)].green * cmatrix[j];
				sum_b += cur_col[(row + j-cmatrix_length/2)].blue * cmatrix[j];
			}
			dest_col[row].red = (BYTE)round2int(sum_r);
			dest_col[row].green = (BYTE)round2int(sum_g);
			dest_col[row].blue = (BYTE)round2int(sum_b);
		}

		/* for the edge condition , we only use available info, and scale to one */
		for (; row < y; row++) {
			/* find scale factor */
			scale=0;
			for (j = 0; j< y-row + cmatrix_length/2; j++)
				scale += cmatrix[j];

			sum_r = sum_g = sum_b = 0;
			for (j = 0; j<y-row + cmatrix_length/2; j++) {
				sum_r += cur_col[(row + j-cmatrix_length/2)].red * cmatrix[j];
				sum_g += cur_col[(row + j-cmatrix_length/2)].green * cmatrix[j];
				sum_b += cur_col[(row + j-cmatrix_length/2)].blue * cmatrix[j];
			}
			dest_col[row].red = (BYTE)round2int(sum_r / scale);
			dest_col[row].green = (BYTE)round2int(sum_g / scale);
			dest_col[row].blue = (BYTE)round2int(sum_b / scale);
		}
	}
}

/* perform an unsharp mask on the region, given a source region, dest.
   region, width and height of the regions, and corner coordinates of
   a subregion to act upon.  Everything outside the subregion is unaffected.
  */
BOOL unsharp_mask(mif& src, LPRECT prc, MIF_UNSHARPMASK* pup)
{
	CRect rc;
	if (!src.validaterect(prc, &rc)) return FALSE;

	MIF_UNSHARPMASK up;
	up.radius = 5.0;
	up.amount = 0.5;
	up.threshold = 0;
	if (pup)
		memcpy(&up, pup, sizeof(up));

	if (up.radius < 0.0 || up.amount < 0.0 || up.threshold < 0)
		return FALSE;

	// these are used for the merging step
	int u;
	int row, col;

	// generate convolution matrix and make sure it's smaller than each dimension
	double* cmatrix;
	int cmatrix_length = gen_convolve_matrix(up.radius, &cmatrix);

	// generate lookup table
	double* ctable = gen_lookup_table(cmatrix, cmatrix_length);

	// allocate row buffers
	int x = rc.Width();
	int y = rc.Height();

	RGBA* cur_row = new RGBA[x];
	RGBA* dst_row = new RGBA[x];

	mif dst;
	dst.create(x, y, 24);

	// blur the rows
int prog = (x + y * 2) >> 3;
src.progress_init(prog);
prog = 0;
	for (row = 0; row < y; row++) {
		src.getrow(cur_row, rc.left, rc.top + row, x);
		dst.getrow(dst_row, 0, row, x);
		blur_line(ctable, cmatrix, cmatrix_length, cur_row, dst_row, x);
		dst.setrow(dst_row, 0, row, x);
if ((++prog & 7) == 0) src.progress_continue();
	}

	// allocate column buffers
	RGBA* cur_col = new RGBA[y];
	RGBA* dst_col = new RGBA[y];

	// blur the cols
	for (col = 0; col < x; col++) {
		dst.getcol(cur_col, col, 0, y);
		dst.getcol(dst_col, col, 0, y);
		blur_line(ctable, cmatrix, cmatrix_length, cur_col, dst_col, y);
		dst.setcol(dst_col, col, 0, y);
if ((++prog & 7) == 0) src.progress_continue();
	}

	// merge the source and destination (which currently contains
	// the blurred version) images
	for (row = 0; row < y; row++) {
		src.getrow(cur_row, rc.left, rc.top + row, x);
		dst.getrow(dst_row, 0, row, x);

		// combine the two
		for (u = 0; u < x; u++) {
			int diff_r = cur_row[u].red - dst_row[u].red;
			int diff_g = cur_row[u].green - dst_row[u].green;
			int diff_b = cur_row[u].blue - dst_row[u].blue;

			// do tresholding
			if (abs(2 * diff_r) < up.threshold) diff_r = 0;
			if (abs(2 * diff_g) < up.threshold) diff_g = 0;
			if (abs(2 * diff_b) < up.threshold) diff_b = 0;

			int value_r = (int)(cur_row[u].red + up.amount * diff_r);
			int value_g = (int)(cur_row[u].green + up.amount * diff_g);
			int value_b = (int)(cur_row[u].blue + up.amount * diff_b);

			dst_row[u].red = ADJRGB(value_r);
			dst_row[u].green = ADJRGB(value_g);
			dst_row[u].blue = ADJRGB(value_b);
		}
		dst.setrow(dst_row, 0, row, x);
if ((++prog & 7) == 0) src.progress_continue();
	}
	// free the memory we took
	delete [] cmatrix;
	delete [] ctable;
	delete [] cur_row;
	delete [] dst_row;
	delete [] cur_col;
	delete [] dst_col;
src.progress_term();

	src.blt(rc.left, rc.top, x, y, &dst, 0, 0);
	return TRUE;
}

BOOL unsharp_mask2(mif& src, LPRECT prc, double radius, double depth)
{
	mif dst(src);
dst.m_progress = src.m_progress;

	CRect rc;
	if (!dst.validaterect(prc, &rc)) return FALSE;
	if (radius < 0.0 || depth < 0.0) return FALSE;

	// Blur
	if (!dst.blur_gauss_iir(prc, radius, radius)) return FALSE;

	int width = rc.Width();
	int height = rc.Height();

src.progress_init(height >> 3);
int prog = 0;

	double depthPlus = depth + 1.0;
	int v;
	for (int y = 0; y < height; y++) {
		RGBA* ps = src.pos(rc.left, rc.top + y);
		RGBA* pd = dst.pos(rc.left, rc.top + y);
		for (int x = 0; x < width; x++) {
			v = (int)floor((depthPlus * ps->red - depth * pd->red) + 0.5);
			ps->red = ADJRGB(v);
			v = (int)floor((depthPlus * ps->green - depth * pd->green) + 0.5);
			ps->green = ADJRGB(v);
			v = (int)floor((depthPlus * ps->blue - depth * pd->blue) + 0.5);
			ps->blue = ADJRGB(v);
			ps++;
			pd++;
		}
if ((++prog & 7) == 0) src.progress_continue();
	}
src.progress_term();
	return TRUE;
}


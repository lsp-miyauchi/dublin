////////////////////////////////////////////////////////////////////////
// gauss_iir.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


static void transfer_pixels(double *src1, double *src2, BYTE *dst, int bpp, int width)
{
	int bend = bpp * width;
	for (int i = 0; i < bend; i++) {
		double sum = *src1++ + *src2++;
 		if (sum > 255)
			sum = 255;
		else if(sum < 0)
			sum = 0;
		*dst++ = (BYTE)sum;
	}
}

static void find_constants(double n_p[], double n_m[], double d_p[], double d_m[], double bd_p[], double bd_m[], double radius)
{
	int i;
	double constants [8];
	double div;

	/*  The constants used in the implemenation of a casual sequence
	 *  using a 4th order approximation of the gaussian operator
	 */

	div = sqrt(2 * M_PI) * radius;
	constants[0] = -1.783 / radius;
	constants[1] = -1.723 / radius;
	constants[2] = 0.6318 / radius;
	constants[3] = 1.997  / radius;
	constants[4] = 1.6803 / div;
	constants[5] = 3.735 / div;
	constants[6] = -0.6803 / div;
	constants[7] = -0.2598 / div;

	n_p[0] = constants[4] + constants[6];
	n_p[1] = exp(constants[1]) *
				 	(constants[7] * sin (constants[3]) -
					(constants[6] + 2 * constants[4]) * cos (constants[3])) +
					exp(constants[0]) *
					(constants[5] * sin (constants[2]) -
					(2 * constants[6] + constants[4]) * cos (constants[2]));
	n_p[2] = 2 * exp (constants[0] + constants[1]) *
					((constants[4] + constants[6]) * 
					cos (constants[3]) * cos (constants[2]) -
					constants[5] * cos (constants[3]) * sin (constants[2]) -
					constants[7] * cos (constants[2]) * sin (constants[3])) +
					constants[6] * exp (2 * constants[0]) +
					constants[4] * exp (2 * constants[1]);
	n_p[3] = exp (constants[1] + 2 * constants[0]) *
					(constants[7] * sin (constants[3]) -
					constants[6] * cos (constants[3])) +
					exp (constants[0] + 2 * constants[1]) *
					(constants[5] * sin (constants[2]) -
					constants[4] * cos (constants[2]));
	n_p[4] = 0.0;

	d_p[0] = 0.0;
	d_p[1] = -2 * exp (constants[1]) * cos (constants[3]) -
				2 * exp (constants[0]) * cos (constants[2]);
	d_p[2] = 4 * cos (constants[3]) * cos (constants[2]) *
				exp (constants[0] + constants[1]) +
				exp (2 * constants[1]) + exp (2 * constants[0]);
	d_p[3] = -2 * cos (constants[2]) * exp (constants[0] + 2 * constants[1]) -
				2 * cos (constants[3]) * exp (constants[1] + 2 * constants[0]);
	d_p[4] = exp (2 * constants[0] + 2 * constants[1]);

	for (i = 0; i <= 4; i++)
		d_m[i] = d_p[i];

	n_m[0] = 0.0;
	for (i = 1; i <= 4; i++)
		n_m[i] = n_p[i] - d_p[i] * n_p[0];

	double sum_n_p = 0.0;
	double sum_n_m = 0.0;
	double sum_d = 0.0;
	for (i = 0; i <= 4; i++)
	{
		sum_n_p += n_p[i];
		sum_n_m += n_m[i];
		sum_d += d_p[i];
	}

	double a = sum_n_p / (1 + sum_d);
	double b = sum_n_m / (1 + sum_d);

	for (i = 0; i <= 4; i++)
	{
		bd_p[i] = d_p[i] * a;
		bd_m[i] = d_m[i] * b;
	}
}

BOOL blur_gauss_iir(mif& msrc, LPRECT prc, double horz, double vert)
{
	CRect rc;
	if (!msrc.validaterect(prc, &rc)) return FALSE;

	if (msrc.m_bits < 24) msrc.m_bits = 24;

	mif mdst;
	mdst.copyfrom(&msrc);

	double n_p[5], n_m[5];
	double d_p[5], d_m[5];
	double bd_p[5], bd_m[5];
	int i, j;
	int row, col, b;
	int terms;
	int initial_p[4];
	int initial_m[4];

	int width = rc.Width();
	int height = rc.Height();

	double* val_p = (double*)malloc(max(width, height) * 3 * sizeof(double));
	double* val_m = (double*)malloc(max(width, height) * 3 * sizeof(double));

	BYTE* src = (BYTE*)malloc(max(width, height) * 3);
	BYTE* dst = (BYTE*)malloc(max(width, height) * 3);

	//
	// First the vertical pass
	//

	// derive the constants for calculating the gaussian from the std dev
	find_constants(n_p, n_m, d_p, d_m, bd_p, bd_m, vert);

msrc.progress_init((width + height) >> 3);
int prog = 0;

	BYTE *sp_p, *sp_m;
	double *vp, *vm;
	for (col = 0; col < width; col++) {
		memset(val_p, 0, height * 3 * sizeof(double));
		memset(val_m, 0, height * 3 * sizeof(double));
		msrc.getcol_rgb(src, col + rc.left, rc.top, height);

		sp_p = src;
		sp_m = src + (height - 1) * 3;
		vp = val_p;
		vm = val_m + (height - 1) * 3;

		// Set up the first vals
		for (i = 0; i < 3; i++) {
			initial_p[i] = sp_p[i];
			initial_m[i] = sp_m[i];
		}

		for (row = 0; row < height; row++) {
			double *vpptr, *vmptr;
			terms = (row < 4) ? row : 4;

			for (b = 0; b < 3; b++) {
				vpptr = vp + b; vmptr = vm + b;
				for (i = 0; i <= terms; i++) {
					*vpptr += n_p[i] * sp_p[(-i * 3) + b] - d_p[i] * vp[(-i * 3) + b];
					*vmptr += n_m[i] * sp_m[(i * 3) + b] - d_m[i] * vm[(i * 3) + b];
				}
				for (j = i; j <= 4; j++) {
					*vpptr += (n_p[j] - bd_p[j]) * initial_p[b];
					*vmptr += (n_m[j] - bd_m[j]) * initial_m[b];
				}
			}

			sp_p += 3;
			sp_m -= 3;
			vp += 3;
			vm -= 3;
		}

		transfer_pixels(val_p, val_m, dst, 3, height);
		mdst.setcol_rgb(dst, col + rc.left, rc.top, height);
if ((++prog & 7) == 0) msrc.progress_continue();
	}


	//
	// Next the horizontal pass
	//

	// derive the constants for calculating the gaussian from the std dev
	if (horz != vert) find_constants(n_p, n_m, d_p, d_m, bd_p, bd_m, horz);

	for (row = 0; row < height; row++) {
		memset(val_p, 0, width * 3 * sizeof(double));
		memset(val_m, 0, width * 3 * sizeof(double));
		mdst.getrow_rgb(src, rc.left, row + rc.top, width);

		sp_p = src;
		sp_m = src + (width - 1) * 3;
		vp = val_p;
		vm = val_m + (width - 1) * 3;

		// Set up the first vals
		for (i = 0; i < 3; i++) {
			initial_p[i] = sp_p[i];
			initial_m[i] = sp_m[i];
		}

		for (col = 0; col < width; col++) {
			double *vpptr, *vmptr;
			terms = (col < 4) ? col : 4;

			for (b = 0; b < 3; b++) {
				vpptr = vp + b; vmptr = vm + b;
				for (i = 0; i <= terms; i++) {
					*vpptr += n_p[i] * sp_p[(-i * 3) + b] - d_p[i] * vp[(-i * 3) + b];
					*vmptr += n_m[i] * sp_m[(i * 3) + b] - d_m[i] * vm[(i * 3) + b];
				}
				for (j = i; j <= 4; j++) {
					*vpptr += (n_p[j] - bd_p[j]) * initial_p[b];
					*vmptr += (n_m[j] - bd_m[j]) * initial_m[b];
				}
			}

			sp_p += 3;
			sp_m -= 3;
			vp += 3;
			vm -= 3;
		}

		transfer_pixels(val_p, val_m, dst, 3, width);
		msrc.setrow_rgb(dst, rc.left, row + rc.top, width);
if ((++prog & 7) == 0) msrc.progress_continue();
	}

msrc.progress_term();

	// free up buffers
	free(val_p);
	free(val_m);
	free(src);
	free(dst);
	return TRUE;
}

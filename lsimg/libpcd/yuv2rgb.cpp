#include "stdafx.h"
#include "pcd.h"
#include "pcdlocal.h"

#define RED_NUL     137
#define BLUE_NUL    156

#define LUN_MUL     360
#define RED_MUL     512
#define BLUE_MUL    512
#define GREEN1_MUL  (-RED_MUL/2)
#define GREEN2_MUL  (-BLUE_MUL/6)

#define RED_ADD     (-RED_NUL*RED_MUL)
#define BLUE_ADD    (-BLUE_NUL*BLUE_MUL)
#define GREEN1_ADD  (-RED_ADD/2)
#define GREEN2_ADD  (-BLUE_ADD/6)

/* ------------------------------------------------------------------------ */

void pcd_get_LUT_init(PCD_IMAGE* img)
{
	// init Lookup tables
	register int i;
	for (i = 0; i < 256; i++) {
		img->LUT_gray_int[i] = i * LUN_MUL >> 8;
		img->LUT_red[i] = (RED_ADD + i * RED_MUL) >> 8;
		img->LUT_blue[i] = (BLUE_ADD + i * BLUE_MUL) >> 8;
		img->LUT_green1[i] = (GREEN1_ADD + i * GREEN1_MUL) >> 8;
		img->LUT_green2[i] = (GREEN2_ADD + i * GREEN2_MUL) >> 8;
	}
}

/* ------------------------------------------------------------------------ */

#define GET_RED		(LUT_RANGE(gray + img->LUT_red[img->exp_red[x]]))
#define GET_GREEN	(LUT_RANGE(gray + img->LUT_green1[img->exp_red[x]] + img->LUT_green2[img->exp_blue[x]]))
#define GET_BLUE	(LUT_RANGE(gray + img->LUT_blue[img->exp_blue[x]]))

static int pcd_get_image_line_0(PCD_IMAGE* img, int y, BYTE* dest)
{
    int maxx;

	int bytes = 3;
	if (img->rot & 2) {
		y = img->height - y - 1;
		dest += (img->width - 1) * bytes;
		bytes = -bytes;
	}

	{
		register int    x;
		register BYTE *src1, *src2;

		maxx = (img->width >> 1) - 1;
		if (y & 1) {
			src1 = img->blue[y >> 1];
			src2 = img->blue[(y + 1 == img->height ? y : y + 1) >> 1];
			for (x = 0; x < maxx; x++) {
				img->exp_blue[x * 2] = (src1[x] + src2[x] + 1) >> 1;
				img->exp_blue[x * 2 + 1] = (src1[x] + src1[x + 1] + src2[x] + src2[x + 1] + 2) >> 2;
			}
			img->exp_blue[x * 2 + 1] = img->exp_blue[x * 2] = (src1[x] + src2[x] + 1) >> 1;

			src1 = img->red[y >> 1];
			src2 = img->red[(y + 1 == img->height ? y : y + 1) >> 1];
			for (x = 0; x < maxx; x++) {
				img->exp_red[x * 2] = (src1[x] + src2[x] + 1) >> 1;
				img->exp_red[x * 2 + 1] = (src1[x] + src1[x + 1] + src2[x] + src2[x + 1] + 2) >> 2;
			}
			img->exp_red[x * 2 + 1] = img->exp_red[x * 2] = (src1[x] + src2[x] + 1) >> 1;
		}
		else {
			src1 = img->blue[y >> 1];
			for (x = 0; x < maxx; x++) {
				img->exp_blue[x * 2] = src1[x];
				img->exp_blue[x * 2 + 1] = (src1[x] + src1[x + 1] + 1) >> 1;
			}
			img->exp_blue[x * 2 + 1] = img->exp_blue[x * 2] = src1[x];

			src1 = img->red[y >> 1];
			for (x = 0; x < maxx; x++) {
				img->exp_red[x * 2] = src1[x];
				img->exp_red[x * 2 + 1] = (src1[x] + src1[x + 1] + 1) >> 1;
			}
			img->exp_red[x * 2 + 1] = img->exp_red[x * 2] = src1[x];
		}
	}
	maxx = img->width;

	{
		register int x, gray;
		register BYTE *luma = img->luma[y];
		for (x = 0; x < maxx; x++, dest += bytes) {
			gray = img->LUT_gray_int[luma[x]];
			dest[2] = GET_RED;
			dest[1] = GET_GREEN;
			dest[0] = GET_BLUE;
		}
	}
	return 0;
}

static int pcd_get_image_line_90(PCD_IMAGE* img, int y, BYTE *dest)
{
	BYTE** luma = img->luma;
	int maxx, y1, y2;

	int bytes = 3;

	if (!(img->rot & 2)) {
		y = img->width - y - 1;
	}
	else {
		dest += (img->height - 1) * bytes;
		bytes = -bytes;
	}

	{
		register int    x;
		register BYTE **src;

		y1 = y >> 1;
		y2 = (y + 1 == img->width ? y : y + 1) >> 1;
		maxx = (img->height >> 1) - 1;

		if (y & 1) {
			src = img->blue;
			for (x = 0; x < maxx; x++) {
				img->exp_blue[x * 2] = (src[x][y1] + src[x][y2] + 1) >> 1;
				img->exp_blue[x * 2 + 1] = (src[x][y1] + src[x][y2] + src[x + 1][y1] + src[x + 1][y2] + 2) >> 2;
			}
			img->exp_blue[x * 2 + 1] = img->exp_blue[x * 2] = (src[x][y1] + src[x][y2] + 1) >> 1;

			src = img->red;
			for (x = 0; x < maxx; x++) {
				img->exp_red[x * 2] = (src[x][y1] + src[x][y2] + 1) >> 1;
				img->exp_red[x * 2 + 1] = (src[x][y1] + src[x][y2] + src[x + 1][y1] + src[x + 1][y2] + 2) >> 2;
			}
			img->exp_red[x * 2 + 1] = img->exp_red[x * 2] = (src[x][y1] + src[x][y2] + 1) >> 1;
		}
		else {
			src = img->blue;
			for (x = 0; x < maxx; x++) {
				img->exp_blue[x * 2] = src[x][y1];
				img->exp_blue[x * 2 + 1] = (src[x][y1] + src[x + 1][y1] + 1) >> 1;
			}
			img->exp_blue[x * 2 + 1] = img->exp_blue[x * 2] = src[x][y1];

			src = img->red;
			for (x = 0; x < maxx; x++) {
				img->exp_red[x * 2] = src[x][y1];
				img->exp_red[x * 2 + 1] = (src[x][y1] + src[x + 1][y1] + 1) >> 1;
			}
			img->exp_red[x * 2 + 1] = img->exp_red[x * 2] = src[x][y1];
		}
	}

	maxx = img->height;
	{
		register int    x, gray;
		for (x = 0; x < maxx; x++, dest += bytes) {
			gray = img->LUT_gray_int[luma[x][y]];
			dest[2] = GET_RED;
			dest[1] = GET_GREEN;
			dest[0] = GET_BLUE;
	    }
	}
	return 0;
}

int pcd_get_image_line(PCD_IMAGE* img, int y, BYTE *dest)
{
	if (img->res == 0) return -1;
	if (img->rot & 1)
		return pcd_get_image_line_90(img, y, dest);
	return pcd_get_image_line_0(img, y, dest);
}

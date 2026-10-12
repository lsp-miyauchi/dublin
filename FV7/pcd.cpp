#include "stdafx.h"
#include "pcd.h"
#include "pcdlocal.h"


#ifndef _REENTRANT
#define SPEED_STATIC static	/* a bit faster, but not thread-save */
#endif

#define RANGE       320

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

static UINT LUT_flag = 0;
static UINT LUT_gray_int[256];
static UINT LUT_red[256];
static UINT LUT_blue[256];
static UINT LUT_green1[256];
static UINT LUT_green2[256];
UINT LUT_range[256 + 2 * RANGE];

int pcd_img_start[]  = { 0 /*dummy */ , 8192, 47104, 196608 };
int pcd_def_width[]  = { 0 /*dummy */ , 192, 384, 768, 1536, 3072, 6144 };
int pcd_def_height[] = { 0 /*dummy */ , 128, 256, 512, 1024, 2048, 4096 };

void pcd_get_LUT_init()
{
	register int i;

	// only once needed
	if (LUT_flag) return;

	LUT_flag = 1;
	// init Lookup tables
	for (i = 0; i < 256; i++) {
		LUT_gray_int[i] = i * LUN_MUL >> 8;
		LUT_red[i] = (RED_ADD + i * RED_MUL) >> 8;
		LUT_blue[i] = (BLUE_ADD + i * BLUE_MUL) >> 8;
		LUT_green1[i] = (GREEN1_ADD + i * GREEN1_MUL) >> 8;
		LUT_green2[i] = (GREEN2_ADD + i * GREEN2_MUL) >> 8;
	}
	for (i = 0; i < RANGE; i++)			LUT_range[i] = 0;
	for (; i < RANGE + 256; i++)		LUT_range[i] = i - RANGE;
	for (; i < 2 * RANGE + 256; i++)	LUT_range[i] = 255;
}

/* ------------------------------------------------------------------------ */

#define GET_RED   (LUT_range[RANGE + gray + LUT_red[red[x]]])
#define GET_GREEN (LUT_range[RANGE + gray + LUT_green1[red[x]] + LUT_green2[blue[x]]])
#define GET_BLUE  (LUT_range[RANGE + gray + LUT_blue[blue[x]]])

static int pcd_get_image_line_0(PCD_IMAGE* img, int y, BYTE* dest)
{
    SPEED_STATIC BYTE red[3072];
    SPEED_STATIC BYTE blue[3072];
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
				blue[x * 2] = (src1[x] + src2[x] + 1) >> 1;
				blue[x * 2 + 1] = (src1[x] + src1[x + 1] + src2[x] + src2[x + 1] + 2) >> 2;
			}
			blue[x * 2 + 1] = blue[x * 2] = (src1[x] + src2[x] + 1) >> 1;

			src1 = img->red[y >> 1];
			src2 = img->red[(y + 1 == img->height ? y : y + 1) >> 1];
			for (x = 0; x < maxx; x++) {
				red[x * 2] = (src1[x] + src2[x] + 1) >> 1;
				red[x * 2 + 1] = (src1[x] + src1[x + 1] + src2[x] + src2[x + 1] + 2) >> 2;
			}
			red[x * 2 + 1] = red[x * 2] = (src1[x] + src2[x] + 1) >> 1;
		}
		else {
			src1 = img->blue[y >> 1];
			for (x = 0; x < maxx; x++) {
				blue[x * 2] = src1[x];
				blue[x * 2 + 1] = (src1[x] + src1[x + 1] + 1) >> 1;
			}
			blue[x * 2 + 1] = blue[x * 2] = src1[x];

			src1 = img->red[y >> 1];
			for (x = 0; x < maxx; x++) {
				red[x * 2] = src1[x];
				red[x * 2 + 1] = (src1[x] + src1[x + 1] + 1) >> 1;
			}
			red[x * 2 + 1] = red[x * 2] = src1[x];
		}
	}
	maxx = img->width;

	{
		register int x, gray;
		register BYTE *luma = img->luma[y];
		for (x = 0; x < maxx; x++, dest += bytes) {
			gray = LUT_gray_int[luma[x]];
			dest[2] = GET_RED;
			dest[1] = GET_GREEN;
			dest[0] = GET_BLUE;
		}
	}
	return 0;
}

static int pcd_get_image_line_90(PCD_IMAGE* img, int y, BYTE *dest)
{
	SPEED_STATIC BYTE red[3072];
	SPEED_STATIC BYTE blue[3072];
	BYTE **luma = img->luma;
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
				blue[x * 2] = (src[x][y1] + src[x][y2] + 1) >> 1;
				blue[x * 2 + 1] = (src[x][y1] + src[x][y2] + src[x + 1][y1] + src[x + 1][y2] + 2) >> 2;
			}
			blue[x * 2 + 1] = blue[x * 2] = (src[x][y1] + src[x][y2] + 1) >> 1;

			src = img->red;
			for (x = 0; x < maxx; x++) {
				red[x * 2] = (src[x][y1] + src[x][y2] + 1) >> 1;
				red[x * 2 + 1] = (src[x][y1] + src[x][y2] + src[x + 1][y1] + src[x + 1][y2] + 2) >> 2;
			}
			red[x * 2 + 1] = red[x * 2] = (src[x][y1] + src[x][y2] + 1) >> 1;
		}
		else {
			src = img->blue;
			for (x = 0; x < maxx; x++) {
				blue[x * 2] = src[x][y1];
				blue[x * 2 + 1] = (src[x][y1] + src[x + 1][y1] + 1) >> 1;
			}
			blue[x * 2 + 1] = blue[x * 2] = src[x][y1];

			src = img->red;
			for (x = 0; x < maxx; x++) {
				red[x * 2] = src[x][y1];
				red[x * 2 + 1] = (src[x][y1] + src[x + 1][y1] + 1) >> 1;
			}
			red[x * 2 + 1] = red[x * 2] = src[x][y1];
		}
	}

	maxx = img->height;
	{
		register int    x, gray;
		for (x = 0; x < maxx; x++, dest += bytes) {
			gray = LUT_gray_int[luma[x][y]];
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

int pcd_open(PCD_IMAGE* img, LPCTSTR filename)
{
	pcd_get_LUT_init();
	memset(img, 0, sizeof(PCD_IMAGE));

	HANDLE hf = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return -1;
	img->size = GetFileSize(hf, NULL);
	img->maphandle = CreateFileMapping(hf, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!img->maphandle) {
		CloseHandle(hf);
		return -1;
	}
	img->mmap = (BYTE*)MapViewOfFile(img->maphandle, FILE_MAP_READ, 0, 0, 0); 
	if (!img->mmap) {
		CloseHandle(img->maphandle);
		img->maphandle = NULL;
		CloseHandle(hf);
		return -1;
	}
	CloseHandle(hf);

	if (0 == memcmp("PCD_OPA", img->mmap, 7)) {
		// this is the thumbnails file
		img->thumbnails = (int)img->mmap[10] << 8 | (int)img->mmap[11];
	}
	else {
		if (img->size < 786432) {
			pcd_close(img);
			return -1;
		}
	}
	return img->thumbnails;
}

int pcd_get_rot(PCD_IMAGE* img, int nr)
{
    if (img->thumbnails)
		return img->mmap[12 + nr] & 3;
	return img->mmap[0x48] & 3;
}

int pcd_get_maxres(PCD_IMAGE* img)
{
	return (img->thumbnails) ? 1 : ((img->size == 786432) ? 3 : 5);
}

int pcd_select(PCD_IMAGE* img, int res, int nr, int rot, int *width, int *height)
{
	// free old stuff
    pcd_free(img);

	// sanity checks...
	if (0 == img->thumbnails) {
		if (res < 1 || res > 5) return -1;
		if (img->size == 786432 && res > 3) return -1;
	}
	else {
		if (nr < 0 || nr >= img->thumbnails) return -1;
	}

	/* width/height == 0: fill in default image size */
	*width = PCD_WIDTH(res, rot);
	*height = PCD_HEIGHT(res, rot);

	/* recalc coordinates (rotation) */
	switch (rot) {
	case 0:			/* none */
		img->width = *width;
		img->height = *height;
		break;
	case 1:			/* 90� ccw */
		img->width = *height;
		img->height = *width;
		break;
	case 2:			/* 180� */
		img->width = *width;
		img->height = *height;
		break;
	case 3:			/* 90� cw */
		img->width = *height;
		img->height = *width;
		break;
	default:
		return -1;
	}

	/* prepeare */
	img->res = res;
	img->nr = nr;
	img->rot = rot;
	img->luma = (BYTE**)malloc(img->height * sizeof(BYTE*));
	img->red = (BYTE**)malloc(img->height * sizeof(BYTE*) >> 1);
	img->blue = (BYTE**)malloc(img->height * sizeof(BYTE*) >> 1);

	if (img->luma == NULL ||
		img->red == NULL ||
		img->blue == NULL) {
		pcd_free(img);
		return -1;
	}

	int y;
	BYTE* ptr;
	if (res <= 3) {
		// just fill in pointers
		if (img->thumbnails) {
			ptr = img->mmap + 10240 + 36864 * nr;
		}
		else {
			ptr = img->mmap + pcd_img_start[res];
		}
		for (y = 0; y < img->height; y += 2, ptr += (pcd_def_width[res] >> 1) * 6) {
			img->luma[y] = ptr;
			img->luma[y + 1] = ptr + (pcd_def_width[res] >> 1) * 2;
			img->blue[y >> 1] = ptr + (pcd_def_width[res] >> 1) * 4;
			img->red[y >> 1] = ptr + (pcd_def_width[res] >> 1) * 5;
		}
	}
	else {
		// high res, have to malloc memory
		img->data = (BYTE*)malloc(img->width * img->height * 3 / 2);
		if (img->data == NULL) {
			pcd_free(img);
			return -1;
		}
		ptr = img->data;
		for (y = 0; y < img->height; y++, ptr += img->width)
			img->luma[y] = ptr;
		for (y = 0; y < img->height >> 1; y++, ptr += img->width >> 1)
			img->blue[y] = ptr;
		for (y = 0; y < img->height >> 1; y++, ptr += img->width >> 1)
			img->red[y] = ptr;
	}
	return 0;
}

int pcd_free(PCD_IMAGE* img)
{
	img->res = 0;
    if (img->data) free(img->data);
    if (img->luma) free(img->luma);
    if (img->red)  free(img->red);
    if (img->blue) free(img->blue);
    if (img->seq1) free(img->seq1);
    if (img->len1) free(img->len1);
    if (img->seq2) free(img->seq2);
    if (img->len2) free(img->len2);
    if (img->seq3) free(img->seq3);
    if (img->len3) free(img->len3);
    img->data = NULL;
    img->luma = img->red = img->blue = NULL;
    img->seq1 = img->seq2 = img->seq3 = NULL;
    img->len1 = img->len2 = img->len3 = NULL;
    return 0;
}

int pcd_close(PCD_IMAGE* img)
{
	UnmapViewOfFile(img->mmap);
	CloseHandle(img->maphandle);
	pcd_free(img);
	memset(img, 0, sizeof(PCD_IMAGE));
	return 0;
}

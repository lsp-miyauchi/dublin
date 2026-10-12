#include "stdafx.h"
#include "pcd.h"
#include "pcdlocal.h"


int pcd_img_start[]  = { 0 /*dummy */ , 8192, 47104, 196608 };
int pcd_def_width[]  = { 0 /*dummy */ , 192, 384, 768, 1536, 3072, 6144 };
int pcd_def_height[] = { 0 /*dummy */ , 128, 256, 512, 1024, 2048, 4096 };

int pcd_open(PCD_IMAGE* img, LPCTSTR filename)
{
	memset(img, 0, sizeof(PCD_IMAGE));
	pcd_get_LUT_init(img);

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
		if (img->size < 786432 || 0 != memcmp("PCD", &img->mmap[0x800], 3)) {
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
	img->exp_red = (BYTE*)malloc(3072);
	img->exp_blue = (BYTE*)malloc(3072);

	if (img->luma == NULL ||
		img->red == NULL ||
		img->blue == NULL ||
		img->exp_red == NULL ||
		img->exp_blue == NULL) {
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
	if (img->exp_red) free(img->exp_red);
	if (img->exp_blue) free(img->exp_blue);
    img->data = NULL;
    img->luma = img->red = img->blue = NULL;
    img->seq1 = img->seq2 = img->seq3 = NULL;
    img->len1 = img->len2 = img->len3 = NULL;
	img->exp_red = img->exp_red = NULL;
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

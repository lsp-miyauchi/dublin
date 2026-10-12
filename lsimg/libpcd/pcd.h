////////////////////////////////////////////////////////////////////////
// pcd.h

#pragma once
#ifndef __PCD_H__
#define __PCD_H__

typedef struct _PCD_IMAGE {
    int size;
	HANDLE maphandle;
    BYTE *mmap;
    int thumbnails;			// # of thumbnails, 0 for normal image file
    int res, nr;
    int width, height, rot;
    BYTE** luma;
    BYTE** red;
    BYTE** blue;
    BYTE* data;
    BYTE* seq1;				// huffman tables
    BYTE* len1;
    BYTE* seq2;
    BYTE* len2;
    BYTE* seq3;
    BYTE* len3;

	BYTE* exp_red;
	BYTE* exp_blue;

	UINT LUT_gray_int[256];
	UINT LUT_red[256];
	UINT LUT_blue[256];
	UINT LUT_green1[256];
	UINT LUT_green2[256];
} PCD_IMAGE;

/* --- file.c --- */

int pcd_open(PCD_IMAGE* img, LPCTSTR filename);
int pcd_get_rot(PCD_IMAGE* img, int nr);
int pcd_get_maxres(PCD_IMAGE* img);
int pcd_select(PCD_IMAGE* img, int res, int nr, int rot, int *width, int *height);
int pcd_close(PCD_IMAGE* img);

/* --- yuv2rgb.c --- */

int pcd_get_image_line(PCD_IMAGE* img, int line, BYTE *dest);

/* --- huff.c --- */
int pcd_decode(PCD_IMAGE* img);


#endif			// __PCD_H__

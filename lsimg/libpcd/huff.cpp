#include "stdafx.h"
#include "pcd.h"
#include "pcdlocal.h"

#define HUFF1 0xc2000

static int pcd_read_htable(BYTE* src, BYTE** pseq, BYTE** pbits)
{
    int len, seq, seq2, bits, i, j;

	if (*pseq) free(*pseq);
	if (*pbits) free(*pbits);
	*pseq = NULL;
	*pbits = NULL;

	*pseq = (BYTE*)malloc(0x10000 * sizeof(char));
	if (*pseq == NULL) return -1;
	memset(*pseq, 0, 0x10000 * sizeof(char));

	*pbits = (BYTE*)malloc(0x10000 * sizeof(char));
	if (*pbits == NULL) {
		free(*pseq);
		*pseq = NULL;
		return -1;
	}
	memset(*pbits, 0, 0x10000 * sizeof(char));

	for (i = 1, len = *src; len >= 0; i += 4, len--) {
		seq = ((int) src[i + 1] << 8 | (int) src[i + 2]);
		bits = src[i] + 1;
		seq2 = seq + (0x10000 >> bits);
		for (j = seq; j < seq2; j++) {
			if ((*pbits)[j]) {
				return -1;
			}
			(*pseq)[j] = src[i + 3];
			(*pbits)[j] = bits;
		}
	}
	return i;
}

#define SETSHIFT       { shiftreg=(((stream[0]<<16) | (stream[1]<<8 ) | (stream[2])) >> (8-bit)) & 0xffff; }
#define LEFTSHIFT      { shiftreg=((shiftreg<<1) & 0xffff) | ((stream[2]>>(7-bit++))&1); stream += bit>>3; bit &= 7; }

static int pcd_un_huff(PCD_IMAGE* img, BYTE *start, int run)
{
	register int shiftreg, bit;
	BYTE *stream = start;
	int type, shift;

	int height, y1, y2;
	switch (run) {
	case 1:
		height = pcd_def_height[4];
		y1 = 0;
		y2 = img->height >> (img->res - 4);
		break;
	case 2:
		height = pcd_def_height[5];
		y1 = 0;
		y2 = img->height >> (img->res - 5);
		break;
	default:
		return -1;
	}

	for (int y = 0; y < height;) {
		for (;;) {
			bit = 0;
			stream = (BYTE*)memchr(stream, 0xff, 10240);
			if (stream[1] == 0xff)
				break;
			stream++;
		}
		SETSHIFT;
		while (shiftreg != 0xfffe)
			LEFTSHIFT;
		stream += 2;
		SETSHIFT;
		y = (shiftreg >> 1) & 0x1fff;
		type = (shiftreg >> 14);
		stream += 2;
		SETSHIFT;

		if (y > height) {
			return -1;
		}
		if (y < y1 || y >= y2)
			continue;

		{
			register BYTE *data;
			register int    x;
			BYTE  *seq;
			BYTE  *bits;
			int             x1, x2;

			switch (type) {
			case 0:
				shift = 0;
				seq = img->seq1;
				bits = img->len1;
				data = img->luma[(y - y1) >> shift];
				break;
			case 2:
				shift = 1;
				seq = img->seq2;
				bits = img->len2;
				data = img->blue[(y - y1) >> shift];
				break;
			case 3:
				shift = 1;
				seq = img->seq3;
				bits = img->len3;
				data = img->red[(y - y1) >> shift];
				break;
			default:
				return -1;
			}

			if (run == 1) {
				x1 = 0;
				x2 = (img->width) >> (img->res - 4 + shift);
			}
			else {
				x1 = 0;
				x2 = (img->width) >> (img->res - 5 + shift);
			}
			for (x = 0; x < x1; x++) {
				bit += bits[shiftreg];
				stream += bit >> 3, bit &= 7;
				SETSHIFT;
			}
			for (x = 0; x < x2; x++) {
				data[x] = LUT_RANGE((int)data[x] + (signed char) seq[shiftreg]);
				bit += bits[shiftreg];
				stream += bit >> 3, bit &= 7;
				SETSHIFT;
			}
		}
	}
	return ((stream - start) + 0x6000 + 2047) & ~0x7ff;
}

int pcd_decode(PCD_IMAGE* img)
{
	int pos = HUFF1, rc;

	switch (img->res) {
	case 1:
	case 2:
	case 3:
		break;				// nothing to do
	case 4:
		pcd_inter_m2(img);
		pcd_inter_lines(img->blue, img->width >> 1, img->height >> 1);
		pcd_inter_lines(img->red, img->width >> 1, img->height >> 1);
		pcd_inter_lines(img->luma, img->width, img->height);
		if (-1 == (rc = pcd_read_htable(img->mmap + pos, &img->seq1, &img->len1)))
			return -1;
		pos += rc;
		pos = (pos + 2047) & ~0x3ff;
		if (-1 == pcd_un_huff(img, img->mmap + pos, 1))
			return (-1);
		break;
	case 5:
		pcd_inter_m2(img);
		pcd_inter_lines(img->blue, img->width >> 2, img->height >> 2);
		pcd_inter_pixels(img->blue, img->width >> 1, img->height >> 1);
		pcd_inter_lines(img->blue, img->width >> 1, img->height >> 1);
		pcd_inter_lines(img->red, img->width >> 2, img->height >> 2);
		pcd_inter_pixels(img->red, img->width >> 1, img->height >> 1);
		pcd_inter_lines(img->red, img->width >> 1, img->height >> 1);
		pcd_inter_lines(img->luma, img->width >> 1, img->height >> 1);
		if (-1 == (rc = pcd_read_htable(img->mmap + pos, &img->seq1, &img->len1)))
			return -1;
		pos += rc;
		pos = (pos + 2047) & ~0x3ff;
		if (-1 == (rc = pcd_un_huff(img, img->mmap + pos, 1)))
			return (-1);
		pos += rc;
		pcd_inter_pixels(img->luma, img->width, img->height);
		pcd_inter_lines(img->luma, img->width, img->height);
		if (-1 == (rc = pcd_read_htable(img->mmap + pos, &img->seq1, &img->len1)))
			return -1;
		pos += rc;
		if (-1 == (rc = pcd_read_htable(img->mmap + pos, &img->seq2, &img->len2)))
			return -1;
		pos += rc;
		if (-1 == (rc = pcd_read_htable(img->mmap + pos, &img->seq3, &img->len3)))
			return -1;
		pos += rc;
		pos = (pos + 2047) & ~0x3ff;
		if (-1 == pcd_un_huff(img, img->mmap + pos, 2))
			return -1;
		break;
	default:
		return -1;
	}
	return 0;
}

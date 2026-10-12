//////////////////////////////////////////////////////////////////////
// sgi

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"
#include "filemap.h"

/* GIMP
 * "$Id: sgilib.c,v 1.8 2003/04/07 11:59:33 neo Exp $"
 *   SGI image file format library routines.
 *   Copyright 1997-1998 Michael Sweet (mike@easysw.com)
 */

#define SGI_MAGIC	474	/* Magic number in image file */
#define SGI_COMP_NONE	0	/* No compression */
#define SGI_COMP_RLE	1	/* Run-length encoding */
#define SGI_COMP_ARLE	2	/* Agressive run-length encoding */


////////////////////////////////////////////////////////////////////////
// Image structure...

typedef struct {
	filemap	*file;			/* Image file */
	int		bpp,			/* Bytes per pixel/channel */
			comp;			/* Compression */
	USHORT	dim;			/* Dimension */
	USHORT	xsize,			/* Width in pixels */
			ysize,			/* Height in pixels */
			zsize;			/* Number of channels */
	long	pixMin,
			pixMax;
	long	colMap;
	long	*table;			/* Offset table for compression */
} sgi_t;

////////////////////////////////////////////////////////////////////////
// 'read_rle8()' - Read 8-bit RLE data.

static int sgiread_rle8(sgi_t* sgip, BYTE* row, int width)
{
	BYTE* rowBase = row;
	while (width > 0) {
		int ch = sgip->file->getc();
		if (ch == EOF) return (-1);

		int count = ch & 127;
		if (count == 0) break;

		if (ch & 128) {
			while (count--) {
				*row++ = sgip->file->getc();
				width--;
			}
		} else {
			ch = sgip->file->getc();
			while (count--) {
				*row++ = ch;
				width--;
			}
		}
	}
	int len = row - rowBase;
	return (width > 0 ? -1 : len);
}

////////////////////////////////////////////////////////////////////////
// 'read_rle16()' - Read 16-bit RLE data.

static int sgiread_rle16(sgi_t* sgip, BYTE* row, int width)
{
	int i;
	int len = 0;
	while (width > 0) {
		int ch = sgip->file->getmsbshort();
		if (ch == EOF)
			return (-1);
		len++;

		int count = ch & 127;
		if (count == 0) break;

		if (ch & 128) {
			for (i = 0; i < count; i++, row++, width--, len++)
				*row = (sgip->file->getmsbshort() >> 8);
		}
		else {
			ch = (sgip->file->getmsbshort() >> 8);
			len++;
			for (i = 0; i < count; i++, row++, width--)
				*row = ch;
		}
	}
	return (width > 0 ? -1 : len * 2);
}

////////////////////////////////////////////////////////////////////////
// 'sgiClose()' - Close an SGI image file.

static int sgiClose(sgi_t* sgip)
{
	if (sgip == NULL) return (-1);
	if (sgip->table != NULL) free(sgip->table);
	if (sgip->file) {
		sgip->file->close();
		delete sgip->file;
	}
	free(sgip);
	return 0;
}

////////////////////////////////////////////////////////////////////////
// 'sgiGetRow()' - Get a row of image data from a file.

static int sgiGetRow(sgi_t* sgip, BYTE* row, int y, int z)
{
	long offset;		/* File offset */
	if (sgip == NULL || row == NULL || y < 0 || y >= sgip->ysize || z < 0 || z >= sgip->zsize)
		return (-1);

	switch (sgip->comp) {
    case SGI_COMP_NONE:
		// Seek to the image row - optimize buffering by only seeking if necessary...
		offset = 512 + (y + z * sgip->ysize) * sgip->xsize * sgip->bpp;
		if (offset != sgip->file->seek(0, SEEK_CUR))
			sgip->file->seek(offset, SEEK_SET);

		if (sgip->bpp == 1)
			sgip->file->read(row, sgip->xsize);
		else {
			for (int x = 0; x < sgip->xsize; x++)
				*row++ = (sgip->file->getmsbshort() >> 8);
		}
		break;

	case SGI_COMP_RLE:
		offset = sgip->table[y + sgip->ysize * z];
		if (offset != sgip->file->seek(0, SEEK_CUR))
			sgip->file->seek(offset, SEEK_SET);

		if (sgip->bpp == 1)
			return (sgiread_rle8(sgip, row, sgip->xsize));
		else
			return (sgiread_rle16(sgip, row, sgip->xsize));
		break;
	}
	return (0);
}

////////////////////////////////////////////////////////////////////////
// 'sgiOpenFile()' - Open an SGI image file for reading or writing.

static sgi_t* sgiOpen(LPCTSTR filename)
{
	sgi_t* sgip = (sgi_t*)calloc(sizeof(sgi_t), 1);
	if (sgip == NULL) return NULL;

	filemap* file = new filemap();
	if (file == NULL || !file->open(filename)) {
		free(sgip);
		return NULL;
	}

	sgip->file = file;
	short magic = sgip->file->getmsbshort();
	if (magic != SGI_MAGIC) {
		delete file;
		free(sgip);
		return NULL;
	}

	sgip->comp   = sgip->file->getc();
	sgip->bpp    = sgip->file->getc();
	sgip->dim    = sgip->file->getmsbshort();
	sgip->xsize  = sgip->file->getmsbshort();
	sgip->ysize  = sgip->file->getmsbshort();
	sgip->zsize  = sgip->file->getmsbshort();
	sgip->pixMin = sgip->file->getmsblong();
	sgip->pixMax = sgip->file->getmsblong();
	sgip->file->seek(84, SEEK_CUR);
	sgip->colMap = sgip->file->getmsblong();

	if (sgip->comp) {
		// This file is compressed; read the scanline tables...
		sgip->file->seek(512, SEEK_SET);
		int tblcount = sgip->ysize * sgip->zsize;
		sgip->table = (long*)malloc(tblcount * sizeof(long));
		for (int i = 0; i < tblcount; i++)
			sgip->table[i] = sgip->file->getmsblong();
	}
	return sgip;
}

////////////////////////////////////////////////////////////////////////
//

HDIB WINAPI xxxSgiLoad(LPCTSTR pszName)
{
	sgi_t* sgip = sgiOpen(pszName);
	if (sgip == NULL) return NULL;

	int i;
	mif image;
	switch (sgip->zsize) {
	case 1:		// Grayscale
	case 2:		// Grayscale + alpha
		image.create(sgip->xsize, sgip->ysize, 8);
		for (i = 0; i < image.m_colors; i++) {
			image.m_map[i].red = image.m_map[i].green = image.m_map[i].blue  = i;
		}
		break;

	case 3:		// RGB
		image.create(sgip->xsize, sgip->ysize, 24);
		break;

	case 4:		// RGBA
		image.create(sgip->xsize, sgip->ysize, 24);
		image.m_bits = 32;
		break;
	}

	BYTE* row[4];
	row[0] = (BYTE*)malloc(sizeof(BYTE) * sgip->xsize * sgip->zsize);
	for (i = 1; i < sgip->zsize; i++)
		row[i] = row[0] + i * sgip->xsize;

	bool bOK = true;
	int x, y;
	for (y = 0; y < sgip->ysize; y++) {
		RGBA* p = image.pos(0, y);
		for (i = 0; i < sgip->zsize; i++) {
			if (sgiGetRow(sgip, row[i], sgip->ysize - 1 - y, i) < 0) {
				bOK = false;
				break;
			}
		}
		if (!bOK) break;

		for (x = 0; x < sgip->xsize; x++) {
			switch (sgip->zsize) {
			case 1:
			case 2:
				p->alpha = row[0][x];
				break;

			case 4:
				p->alpha = row[3][x];
				// fall through...

			case 3:
				p->red = row[0][x];
				p->green = row[1][x];
				p->blue = row[2][x];
				break;
			}
			p++;
		}
	}
	sgiClose(sgip);
	free(row[0]);
	return (bOK ? image.createdib() : NULL);
}

//////////////////////////////////////////////////////////////////////
// sgicode.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

/* GIMP
 * "$Id: sgilib.c,v 1.8 2003/04/07 11:59:33 neo Exp $"
 *   SGI image file format library routines.
 *   Copyright 1997-1998 Michael Sweet (mike@easysw.com)
 */

#define SGI_MAGIC	474	/* Magic number in image file */
#define SGI_READ	0	/* Read from an SGI image file */
#define SGI_WRITE	1	/* Write to an SGI image file */
#define SGI_COMP_NONE	0	/* No compression */
#define SGI_COMP_RLE	1	/* Run-length encoding */
#define SGI_COMP_ARLE	2	/* Agressive run-length encoding */


////////////////////////////////////////////////////////////////////////
// Image structure...

typedef struct {
	FILE	*file;			/* Image file */
	int		mode,			/* File open mode */
			bpp,			/* Bytes per pixel/channel */
			comp,			/* Compression */
			swapBytes;		/* SwapBytes flag */
	USHORT	dim;			/* Dimension */
	USHORT	xsize,			/* Width in pixels */
			ysize,			/* Height in pixels */
			zsize;			/* Number of channels */
	long	pixMin,
			pixMax;
	long	colMap;
	long	firstrow,		/* File offset for first row */
			nextrow,		/* File offset for next row */
			*table,			/* Offset table for compression */
			*length;		/* Length table for compression */
	USHORT*	arle_row;		/* Advanced RLE compression buffer */
	long	arle_offset,	/* Advanced RLE buffer offset */
			arle_length;	/* Advanced RLE buffer length */
} sgi_t;


////////////////////////////////////////////////////////////////////////
// Local functions...

static int	getlong(sgi_t*);
static int	getshort(sgi_t*);
static int	putlong(long, sgi_t*);
static int	putshort(USHORT, sgi_t*);
static int	read_rle8(sgi_t*, USHORT*, int);
static int	read_rle16(sgi_t*, USHORT*, int);
static int	write_rle8(sgi_t*, USHORT*, int);
static int	write_rle16(sgi_t*, USHORT*, int);


////////////////////////////////////////////////////////////////////////
// 'sgiClose()' - Close an SGI image file.

int sgiClose(sgi_t* sgip)
{
	int i;			/* Return status */
	if (sgip == NULL) return (-1);

	if (sgip->mode == SGI_WRITE && sgip->comp != SGI_COMP_NONE) {
		// Write the scanline offset table to the file...
		fseek(sgip->file, 512, SEEK_SET);
		int tblcount = sgip->ysize * sgip->zsize;
		for (i = 0; i < tblcount; i++)
			if (putlong(sgip->table[i], sgip) < 0)
				return (-1);
		for (i = 0; i < tblcount; i++)
			if (putlong(sgip->length[i], sgip) < 0)
				return (-1);
	}

	if (sgip->table != NULL) free(sgip->table);
	if (sgip->length != NULL) free(sgip->length);
	if (sgip->comp == SGI_COMP_ARLE) free(sgip->arle_row);

	i = fclose(sgip->file);
	free(sgip);
	return (i);
}

////////////////////////////////////////////////////////////////////////
// 'sgiGetRow()' - Get a row of image data from a file.

int sgiGetRow(sgi_t* sgip, USHORT* row, int y, int z)
{
	int x;				/* X coordinate */
	long offset;		/* File offset */
	long len;

	if (sgip == NULL || row == NULL || y < 0 || y >= sgip->ysize || z < 0 || z >= sgip->zsize)
		return (-1);

	switch (sgip->comp) {
    case SGI_COMP_NONE:
		// Seek to the image row - optimize buffering by only seeking if necessary...
		offset = 512 + (y + z * sgip->ysize) * sgip->xsize * sgip->bpp;
		if (offset != ftell(sgip->file))
			fseek(sgip->file, offset, SEEK_SET);

		if (sgip->bpp == 1) {
			for (x = 0; x < sgip->xsize; x++)
				*row++ = getc(sgip->file);
		}
		else {
			for (x = 0; x < sgip->xsize; x++)
				*row++ = getshort(sgip);
		}
		break;

	case SGI_COMP_RLE:
		offset = sgip->table[y + sgip->ysize * z];
		len = sgip->length[y + sgip->ysize * z];
		if (offset != ftell(sgip->file))
			fseek(sgip->file, offset, SEEK_SET);

		if (sgip->bpp == 1)
			return (read_rle8(sgip, row, sgip->xsize));
		else
			return (read_rle16(sgip, row, sgip->xsize));
		break;
	}
	return (0);
}

////////////////////////////////////////////////////////////////////////
// 'sgiOpenFile()' - Open an SGI image file for reading or writing.

sgi_t* sgiOpenFile(FILE* file, int mode, int comp, int bpp, int xsize, int ysize, int zsize)
{
	int i, tblcount;	/* Looping var */
	char* dummy;		/* Name of file in image header */
	short magic;		/* Magic number */

	sgi_t* sgip = (sgi_t*)calloc(sizeof(sgi_t), 1);
	if (sgip == NULL) return (NULL);

	sgip->file = file;
	sgip->swapBytes = 0;
	switch (mode) {
	case SGI_READ:
		sgip->mode = SGI_READ;
		magic = getshort(sgip);
		if (magic != SGI_MAGIC) {
			// try little endian format
			magic = ((magic >> 8) & 0x00ff) | ((magic << 8) & 0xff00);
			if(magic != SGI_MAGIC) {
				free(sgip);
				return (NULL);
			}
			sgip->swapBytes = 1;
		}

		sgip->comp   = getc(sgip->file);
		sgip->bpp    = getc(sgip->file);
		sgip->dim    = getshort(sgip);
		sgip->xsize  = getshort(sgip);
		sgip->ysize  = getshort(sgip);
		sgip->zsize  = getshort(sgip);
		sgip->pixMin = getlong(sgip);
		sgip->pixMax = getlong(sgip);
		fseek(sgip->file, 84, SEEK_CUR);
		sgip->colMap = getlong(sgip);

		if (sgip->comp) {
			// This file is compressed; read the scanline tables...
			fseek(sgip->file, 512, SEEK_SET);
			tblcount = sgip->ysize * sgip->zsize;
			sgip->table = (long*)malloc(tblcount * sizeof(long));
			for (i = 0; i < tblcount; i++)
				sgip->table[i] = getlong(sgip);
			sgip->length = (long*)malloc(tblcount * sizeof(long));
			for (i = 0; i < tblcount; i++)
				sgip->length[i] = getlong(sgip);
		}
		break;

	case SGI_WRITE:
		if (xsize < 1 || ysize < 1 || zsize < 1 || bpp < 1 || bpp > 2 || comp < SGI_COMP_NONE || comp > SGI_COMP_ARLE) {
			free(sgip);
			return (NULL);
		}

		sgip->mode = SGI_WRITE;
		putshort(SGI_MAGIC, sgip);
		putc((sgip->comp = comp) != 0, sgip->file);
		putc(sgip->bpp = bpp, sgip->file);
		putshort(3, sgip);			// Dimensions
		putshort(sgip->xsize = xsize, sgip);
		putshort(sgip->ysize = ysize, sgip);
		putshort(sgip->zsize = zsize, sgip);
		if (bpp == 1) {
			putlong(0, sgip);		// Minimum pixel
			putlong(255, sgip);		// Maximum pixel
		}
		else {
			putlong(-32768, sgip);	// Minimum pixel
			putlong(32767, sgip);	// Maximum pixel
		}
		putlong(0, sgip);			// Reserved

		dummy = (char*)malloc(512);
		memset(dummy, 0, 512);
		// fill image_name field
		fwrite(dummy, 80, 1, sgip->file);

		// colormap
		putlong(0, sgip);

		// fill dummy field
		fwrite(dummy, 404, 1, sgip->file);
		free(dummy);

		switch (comp) {
		case SGI_COMP_NONE:			// No compression
			// This file is uncompressed.  To avoid problems with sparse files,
			// we need to write blank pixels for the entire image...
			break;

		case SGI_COMP_ARLE:			// Aggressive RLE
			sgip->arle_row    = (USHORT*)calloc(xsize, sizeof(USHORT));
			sgip->arle_offset = 0;

		case SGI_COMP_RLE:			// Run-Length Encoding
			tblcount = sgip->ysize * sgip->zsize;
			// This file is compressed; write the (blank) scanline tables...
			for (i = 2 * tblcount; i > 0; i--)
				putlong(0, sgip);

			sgip->firstrow = ftell(sgip->file);
			sgip->nextrow  = ftell(sgip->file);
			sgip->table    = (long*)calloc(tblcount, sizeof(long));
			sgip->length   = (long*)calloc(tblcount, sizeof(long));
			break;
		}
		break;

	default:
		free(sgip);
		return (NULL);
	}
	return (sgip);
}

////////////////////////////////////////////////////////////////////////
// 'sgiOpen()' - Open an SGI image file for reading or writing.

sgi_t* sgiOpen(LPCTSTR filename, int mode, int comp, int bpp, int xsize, int ysize, int zsize)
{
	FILE* file;
	if (mode == SGI_READ)
		file = _tfopen(filename, _T("rb"));
	else
		file = _tfopen(filename, _T("wb+"));
	if (file == NULL) return (NULL);

	sgi_t* sgip = sgiOpenFile(file, mode, comp, bpp, xsize, ysize, zsize);
	if (sgip == NULL)
		fclose(file);
	return (sgip);
}

////////////////////////////////////////////////////////////////////////
// 'sgiPutRow()' - Put a row of image data to a file.

int sgiPutRow(sgi_t* sgip, USHORT* row, int y, int z)
{
	int x;				/* X coordinate */
	long offset;		/* File offset */
	long pos;

	if (sgip == NULL || row == NULL || y < 0 || y >= sgip->ysize || z < 0 || z >= sgip->zsize)
		return (-1);

	switch (sgip->comp) {
    case SGI_COMP_NONE:
		// Seek to the image row - optimize buffering by only seeking if necessary...
		offset = 512 + (y + z * sgip->ysize) * sgip->xsize * sgip->bpp;
		if (offset != ftell(sgip->file))
			fseek(sgip->file, offset, SEEK_SET);

		if (sgip->bpp == 1) {
			for (x = 0; x < sgip->xsize; x++) {
				putc(*row, sgip->file);
				row++;
			}
		}
		else {
			for (x = 0; x < sgip->xsize; x++) {
				putshort(*row, sgip);
				row++;
			}
		}
		break;

	case SGI_COMP_ARLE:
		offset = y + sgip->ysize * z;
		// First check the last row written...
		if (sgip->arle_offset > 0) {
			for (x = 0; x < sgip->xsize; x++)
				if (row[x] != sgip->arle_row[x])
					break;

			if (x == sgip->xsize) {
				sgip->table[offset] = sgip->arle_offset;
				sgip->length[offset] = sgip->arle_length;
				return (0);
			}
		}

		// If that didn't match, search all the previous rows...
		fseek(sgip->file, sgip->firstrow, SEEK_SET);
		if (sgip->bpp == 1) {
			do {
				sgip->arle_offset = ftell(sgip->file);
				if ((sgip->arle_length = read_rle8(sgip, sgip->arle_row, sgip->xsize)) < 0) {
					x = 0;
					break;
				}
				for (x = 0; x < sgip->xsize; x++)
					if (row[x] != sgip->arle_row[x])
						break;
			} while (x < sgip->xsize);
		}
		else {
			do {
				sgip->arle_offset = ftell(sgip->file);
				if ((sgip->arle_length = read_rle16(sgip, sgip->arle_row, sgip->xsize)) < 0) {
					x = 0;
					break;
				}

				for (x = 0; x < sgip->xsize; x++)
					if (row[x] != sgip->arle_row[x])
						break;
			} while (x < sgip->xsize);
		}

		if (x == sgip->xsize) {
			sgip->table[offset]  = sgip->arle_offset;
			sgip->length[offset] = sgip->arle_length;
			return (0);
		}
		else
			fseek(sgip->file, 0, SEEK_END);	/* Clear EOF */

	case SGI_COMP_RLE:
		offset = y + sgip->ysize * z;
		pos = sgip->table[offset] = sgip->nextrow;
		if (pos != ftell(sgip->file))
			fseek(sgip->file, pos, SEEK_SET);

		if (sgip->bpp == 1)
			x = write_rle8(sgip, row, sgip->xsize);
		else
			x = write_rle16(sgip, row, sgip->xsize);

		if (sgip->comp == SGI_COMP_ARLE) {
			sgip->arle_offset = pos;
			sgip->arle_length = x;
			memcpy(sgip->arle_row, row, sgip->xsize * sizeof(short));
		}
		sgip->nextrow = ftell(sgip->file);
		sgip->length[offset] = x;
		return (x);
	}
	return (0);
}

////////////////////////////////////////////////////////////////////////
// 'getlong()' - Get a 32-bit big-endian integer.

static int getlong(sgi_t* sgip)
{
	BYTE b[4];
	fread(b, 4, 1, sgip->file);
	if (sgip->swapBytes)
		return ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]);
	else
		return ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
}


////////////////////////////////////////////////////////////////////////
// 'getshort()' - Get a 16-bit big-endian integer.

static int getshort(sgi_t* sgip)
{
	BYTE b[2];
	fread(b, 2, 1, sgip->file);
	if (sgip->swapBytes)
		return ((b[1] << 8) | b[0]);
	else
		return ((b[0] << 8) | b[1]);
}

////////////////////////////////////////////////////////////////////////
// 'putlong()' - Put a 32-bit big-endian integer.

static int putlong(long n, sgi_t* sgip)
{
	if (putc(n >> 24, sgip->file) == EOF) return (EOF);
	if (putc(n >> 16, sgip->file) == EOF) return (EOF);
	if (putc(n >> 8, sgip->file) == EOF) return (EOF);
	if (putc(n, sgip->file) == EOF) return (EOF);
	return (0);
}

////////////////////////////////////////////////////////////////////////
// 'putshort()' - Put a 16-bit big-endian integer.

static int putshort(USHORT n, sgi_t* sgip)
{
	if (putc(n >> 8, sgip->file) == EOF) return (EOF);
	if (putc(n, sgip->file) == EOF) return (EOF);
	return (0);
}

////////////////////////////////////////////////////////////////////////
// 'read_rle8()' - Read 8-bit RLE data.

static int read_rle8(sgi_t* sgip, USHORT* row, int width)
{
	USHORT* rowBase = row;
	while (width > 0) {
		int ch = getc(sgip->file);
		if (ch == EOF) return (-1);

		int count = ch & 127;
		if (count == 0) break;

		if (ch & 128) {
			while (count--) {
				*row++ = getc(sgip->file);
				width--;
			}
		}
		else {
			ch = getc(sgip->file);
			while (count--) {
				*row++ = ch;
				width--;
			}
		}
	}

	int len = (row - rowBase) / sizeof(USHORT);
	return (width > 0 ? -1 : len);
}

////////////////////////////////////////////////////////////////////////
// 'read_rle16()' - Read 16-bit RLE data.

static int read_rle16(sgi_t* sgip, USHORT* row, int width)
{
	int i;
	int len = 0;
	while (width > 0) {
		int ch = getshort(sgip);
		if (ch == EOF)
			return (-1);
		len++;

		int count = ch & 127;
		if (count == 0) break;

		if (ch & 128) {
			for (i = 0; i < count; i++, row++, width--, len++)
				*row = getshort(sgip);
		}
		else {
			ch = getshort(sgip);
			len++;
			for (i = 0; i < count; i++, row++, width--)
				*row = ch;
		}
	}
	return (width > 0 ? -1 : len * 2);
}

////////////////////////////////////////////////////////////////////////
// 'write_rle8()' - Write 8-bit RLE data.

static int write_rle8(sgi_t* sgip, USHORT* row, int width)
{
	int		len,	/* Length of output line */
			count,	/* Number of repeated/non-repeated pixels */
			i,		/* Looping var */
			x;		/* Looping var */
	USHORT	*start,	/* Start of sequence */
			repeat;	/* Repeated pixel */

	for (x = width, len = 0; x > 0; ) {
		start = row;
		row   += 2;
		x     -= 2;
		while (x > 0 && (row[-2] != row[-1] || row[-1] != row[0])) {
			row++;
			x--;
		}
		row -= 2;
		x   += 2;

		count = row - start;
		while (count > 0) {
			i     = count > 126 ? 126 : count;
			count -= i;

			if (putc(128 | i, sgip->file) == EOF)
				return (-1);
			len++;

			while (i > 0) {
				if (putc(*start, sgip->file) == EOF)
					return (-1);
				start++;
				i--;
				len++;
			}
		}

		if (x <= 0) break;

		start  = row;
		repeat = row[0];
		row++;
		x--;
		while (x > 0 && *row == repeat) {
			row++;
			x--;
		}

		count = row - start;
		while (count > 0) {
			i     = count > 126 ? 126 : count;
			count -= i;

			if (putc(i, sgip->file) == EOF)
				return (-1);
			len++;

			if (putc(repeat, sgip->file) == EOF)
				return (-1);
			len++;
		}
	}
	len++;
	if (putc(0, sgip->file) == EOF)
		return (-1);
	return (len);
}

////////////////////////////////////////////////////////////////////////
// 'write_rle16()' - Write 16-bit RLE data.

static int write_rle16(sgi_t* sgip, USHORT* row, int width)
{
  int		len,	/* Length of output line */
			count,	/* Number of repeated/non-repeated pixels */
			i,		/* Looping var */
			x;		/* Looping var */
  USHORT	*start,	/* Start of sequence */
			repeat;	/* Repeated pixel */

	for (x = width, len = 0; x > 0;) {
		start = row;
		row   += 2;
		x     -= 2;

		while (x > 0 && (row[-2] != row[-1] || row[-1] != row[0])) {
			row++;
			x--;
		}
		row -= 2;
		x   += 2;

		count = row - start;
		while (count > 0) {
			i     = count > 126 ? 126 : count;
			count -= i;

			if (putshort(128 | i, sgip) == EOF)
				return (-1);
			len++;

			while (i > 0) {
				if (putshort(*start, sgip) == EOF)
					return (-1);
				start++;
				i--;
				len++;
			}
		}

		if (x <= 0) break;

		start  = row;
		repeat = row[0];
		row++;
		x--;
		while (x > 0 && *row == repeat) {
			row++;
			x--;
		}

		count = row - start;
		while (count > 0) {
			i     = count > 126 ? 126 : count;
			count -= i;

			if (putshort(i, sgip) == EOF)
				return (-1);
			len++;

			if (putshort(repeat, sgip) == EOF)
				return (-1);
			len++;
		}
	}
	len++;
	if (putshort(0, sgip) == EOF)
		return (-1);
	return (2 * len);
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
HDIB WINAPI imgSgiLoad(LPCTSTR pszName)
{
	sgi_t* sgip = sgiOpen(pszName, SGI_READ, 0, 0, 0, 0, 0);
	if (sgip == NULL) return NULL;

	int i;
	int bits;
	switch (sgip->zsize) {
		case 1:		// Grayscale
		case 2:		// Grayscale + alpha
			bits = 8; break;
		case 3:		// RGB
			bits = 24; break;
		case 4:		// RGBA
			bits = 32;
			break;
	}

	HDIB hdib = dib_create(sgip->xsize, sgip->ysize, bits);
	if (!hdib) {
		sgiClose(sgip);
		return NULL;
	}

	CDibHandle dib(hdib);
	if (bits == 8) {
		RGBQUAD* map = dib.ColorMap();
		for (i = 0; i < 256; i++) {
			map->rgbRed = map->rgbGreen = map->rgbBlue  = i;
			map++;
		}
	}

	USHORT* row[4];
	row[0] = (USHORT*)malloc(sizeof(USHORT) * sgip->xsize * sgip->zsize);
	for (i = 1; i < sgip->zsize; i++)
		row[i] = row[0] + i * sgip->xsize;

	BYTE* pixels = (BYTE*)dib.BitmapBits();
	bool bOK = true;
	int x, y;
	for (y = 0; y < sgip->ysize; y++) {
		BYTE* p = pixels + dib.WidthBytes() * y;
		for (i = 0; i < sgip->zsize; i++) {
			if (sgiGetRow(sgip, row[i], y, i) < 0) {
				bOK = false;
				break;
			}
		}
		if (!bOK) break;

		if (sgip->bpp == 1) {
			// 8-bit (unsigned) pixels...
			for (x = 0; x < sgip->xsize; x++) {
				switch (sgip->zsize) {
				case 1:
				case 2:
					*p++ = (BYTE)row[0][x];
					break;

				case 4:
					*p++ = (BYTE)row[3][x];
					// fall through...

				case 3:
					*p++ = (BYTE)row[2][x];
					*p++ = (BYTE)row[1][x];
					*p++ = (BYTE)row[0][x];
					break;
				}
			}
		}
		else {
			// 16-bit (unsigned) pixels...
			for (x = 0; x < sgip->xsize; x++) {
				switch (sgip->zsize) {
				case 1:
				case 2:
					*p++ = (BYTE)(row[0][x] >> 8);
					break;

				case 4:
					*p++ = (BYTE)(row[3][x] >> 8);
					// fall through...

				case 3:
					*p++ = (BYTE)(row[2][x] >> 8);
					*p++ = (BYTE)(row[1][x] >> 8);
					*p++ = (BYTE)(row[0][x] >> 8);
					break;
				}
			}
		}
	}
	sgiClose(sgip);
	free(row[0]);

	dib.Detach();
	if (!bOK) {
		GlobalFree(hdib);
		return NULL;
	}
	return hdib;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
HANDLE WINAPI imgSgiSaveMem(HDIB hdib, int nCompression, DWORD* pdwSize)
{
	return NULL;
}

extern "C"
BOOL WINAPI imgSgiSave(LPCTSTR pszFile, HDIB hdib, int nCompression)
{
	mif image;
	if (!image.attachdib(hdib)) return FALSE;

	sgi_t* sgip = sgiOpen(pszFile, SGI_WRITE, nCompression, 1, image.m_width, image.m_height, 3);
	if (sgip == NULL) return FALSE;

	USHORT* row[3];
	row[0] = (USHORT*)malloc(sizeof(USHORT) * image.m_width * 3);
	row[1] = row[0] + image.m_width;
	row[2] = row[1] + image.m_width;

	for (int y = 0; y < image.m_height; y++) {
		RGBA* p = image.pos(0, image.m_height - y - 1);
		for (int x = 0; x < image.m_width; x++) {
			row[0][x] = p->red;
			row[1][x] = p->green;
			row[2][x] = p->blue;
			p++;
		}
		for (int i = 0; i < 3; i++)
			sgiPutRow(sgip, row[i], y, i);
	}
	sgiClose(sgip);
	free(row[0]);
	return TRUE;
}


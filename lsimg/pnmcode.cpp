////////////////////////////////////////////////////////////////////////
// pnmread.cpp - Converts from/to Adobe PNM/PGM image

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"
#include "filemap.h"

// format = '1' PBM		-- 2bit color (ascii)
// format = '2'	PGM		-- gray scale (ascii)
// format = '3'	PPM		-- full color (ascii)
// format = '4'	PBM		-- 2bit color (bin)
// format = '5'	PGM		-- gray scale (bin)
// format = '6'	PPM		-- full color (bin)
// format = '7'			-- indexed color with 332 color map

#define MagickEpsilon		1.0e-12
#define Intensity(a)  \
  ((0.299*(a).red+0.587*(a).green+(0.114+MagickEpsilon)*(a).blue))

static const short int ditherR[2][16]=
{
	{-16,  4, -1, 11,-14,  6, -3,  9,-15,  5, -2, 10,-13,  7, -4,  8},
	{ 15, -5,  0,-12, 13, -7,  2,-10, 14, -6,  1,-11, 12, -8,  3, -9}
};
static const short int ditherG[2][16]=
{
	{ 11,-15,  7, -3,  8,-14,  4, -2, 10,-16,  6, -4,  9,-13,  5, -1},
	{-12, 14, -8,  2, -9, 13, -5,  1,-11, 15, -7,  3,-10, 12, -6,  0}
};
static const short int ditherB[2][16]=
{
	{ -3,  9,-13,  7, -1, 11,-15,  5, -4,  8,-14,  6, -2, 10,-16,  4},
	{  2,-10, 12, -8,  0,-12, 14, -6,  3, -9, 13, -7,  1,-11, 15, -5}
};


static UINT integer(filemap& fm, UINT base)
{
	int c;
	UINT value;

	// Skip any leading whitespace.
	do {
		c = fm.getc();
		if (c == EOF) return 0;

		if (c == '#') {
			// skip comment.
			for ( ; (c != EOF) && (c != '\n');)
				c = fm.getc();

			continue;
		}
	} while (!isdigit(c));

	if (base == 2) return (c - '0');

	// Evaluate number.
	value = 0;
	do {
		value *= 10;
		value += c - '0';
		c = fm.getc();
		if (c == EOF)
			return 0;
	} while (isdigit(c));
	return value;
}

static HDIB pnm_load_core(filemap& fm)
{
	char format;
	int y;
	BYTE* scale;
	register int i, x;
	register RGBA* q;
	UINT max_value, status;
	USHORT blue, green, index, red;

	mif image;

	// Read PNM image.
	status = fm.read(&format, 1);
	do {
		// Verify PNM identifier.
		if (!status || format != 'P') return NULL;

		// Initialize image structure.
		format = fm.getc();
		if (format == '7')
			integer(fm, 10);

		int columns = integer(fm, 10);
		int rows = integer(fm, 10);
		if (format == '1' || format == '4')
			max_value = 1;			// bitmap
		else
			max_value = integer(fm, 10);

		BOOL indexed_color = FALSE;
		int colors = 0;
		if (format != '3' && format != '6') {
			indexed_color = TRUE;
			colors = min(max_value, 255) + 1;
		}

		if (columns * rows == 0)
			return NULL;

		image.create(columns, rows, indexed_color ? 8 : 24);

		scale = NULL;
		if (indexed_color) {
			// Create colormap.
			if (format != '7') {
				for (i = 0; i < image.m_colors; i++) {
					image.m_map[i].red = (BYTE)((255 * i) / (colors - 1));
					image.m_map[i].green = (BYTE)((255 * i) / (colors - 1));
					image.m_map[i].blue = (BYTE)((255 * i) / (colors - 1));
				}
			}
			else {
				// Initialize 332 colormap.
				i = 0;
				for (red = 0; red < 8; red++)
					for (green = 0; green < 8; green++)
						for (blue = 0; blue < 4; blue++) {
							image.m_map[i].red = (BYTE)((int)(red*255)/7);
							image.m_map[i].green = (BYTE)((int)(green*255)/7);
							image.m_map[i].blue = (BYTE)((int)(blue*255)/3);
							i++;
						}
			}
		}
		else if (max_value != 255) {
			// Compute pixel scaling table.
			scale = (BYTE*)malloc((max_value + 1) * sizeof(BYTE));
			for (i = 0; i <= (int)max_value; i++)
				scale[i] = (BYTE)((i * 255 + (max_value >> 1)) / max_value);
		}

		// Convert PNM pixels to runlength-encoded MIFF packets.
		q = image.m_pixels;
		switch (format) {
			case '1':
				for (y = 0; y < image.m_height; y++) {
					for (x = 0; x < image.m_width; x++) {
						index = !integer(fm, 2);
						q->alpha = (BYTE)index;
						q++;
					}
				}
				break;

			case '2':
				for (y = 0; y < image.m_height; y++) {
					for (x = 0; x < image.m_width; x++) {
						index = integer(fm, 10);
						q->alpha = (BYTE)index;
						q++;
					}
				}
				break;

			case '3':
				for (y = 0; y < image.m_height; y++) {
					for (x = 0; x < image.m_width; x++) {
						red = integer(fm, 10);
						green = integer(fm, 10);
						blue = integer(fm, 10);
						if (scale) {
//@							red = scale[red];
//@							green = scale[green];
//@							blue = scale[blue];
							red = scale[min(max_value, red)];
							green = scale[min(max_value, green)];
							blue = scale[min(max_value, blue)];
						}
						q->red = (BYTE)red;
						q->green = (BYTE)green;
						q->blue = (BYTE)blue;
						q++;
					}
				}
				break;

			case '4':
				for (y = 0; y < image.m_height; y++) {
					BYTE bit = 0;
					BYTE byte = 0;
					for (x = 0; x < image.m_width; x++) {
						if (bit == 0)
							byte = fm.getc();
						index = (byte & 0x80) ? 0 : 1;
						q->alpha = (BYTE)index;
						bit++;
						if (bit == 8)
							bit = 0;
						byte <<= 1;
						q++;
					}
				}
				break;

			case '5':
			case '7':
				for (y = 0; y < image.m_height; y++) {
					for (x = 0; x < image.m_width; x++) {
						if (max_value <= 255)
							index = fm.getc();
						else
							index = fm.getshort();

						if (index > max_value)
							index = max_value;
						q->alpha = (BYTE)index;
						q++;
					}
				}
				break;

			case '6':
				for (y = 0; y < image.m_height; y++) {
					for (x = 0; x < image.m_width; x++) {
						if (max_value <= 255) {
							red = fm.getc();
							green = fm.getc();
							blue = fm.getc();
						}
						else {
							red = fm.getshort();
							green = fm.getshort();
							blue = fm.getshort();
						}

						if (scale) {
//@							red = scale[red];
//@							green = scale[green];
//@							blue = scale[blue];
							red = scale[min(max_value, red)];
							green = scale[min(max_value, green)];
							blue = scale[min(max_value, blue)];
						}
						q->red = (BYTE)red;
						q->green = (BYTE)green;
						q->blue = (BYTE)blue;
						q++;
					}
				}
				break;

			default:
				return NULL;
		}

		if (scale)
			free(scale);

		break;			// もし次の画像データがあっても読まない
	} while (status && format == 'P');

	return image.createdib();
}

extern "C"
HDIB WINAPI imgPnmLoad(LPCTSTR pName)
{
	HDIB hdib = NULL;
	filemap fm;
	if (fm.open(pName))
		hdib = pnm_load_core(fm);
	return hdib;
}


static void pnmsave_p1(mif& image, FILE* fp)
{
	fputs("P1\n", fp);
	fprintf(fp, "%u %u\n", image.m_width, image.m_height);
	BYTE polarity = Intensity(image.m_map[0]) > (0.5 * 255);
	if (image.m_colors == 2)
		polarity = Intensity(image.m_map[0]) > Intensity(image.m_map[1]);
	int i = 0;
	RGBA* p = image.m_pixels;
	for (int j = 0; j < image.m_packets; j++)
	{
		fprintf(fp, "%d ", (p->alpha == polarity));
		i++;
		if (i == 36)
		{
			fputc('\n', fp);
			i = 0;
		}
	}
}

static void pnmsave_p2(mif& image, FILE* fp)
{
	fputs("P2\n", fp);
	fprintf(fp, "%u %u\n", image.m_width, image.m_height);
	fputs("255\n", fp);
	int i = 0;
	RGBA* p = image.m_pixels;
	for (int j = 0; j < image.m_packets; j++)
	{
		fprintf(fp, "%d ", (int)Intensity(*p));
		i++;
		if (i == 12)
		{
			fputc('\n', fp);
			i = 0;
		}
		p++;
	}
}

static void pnmsave_p3(mif& image, FILE* fp)
{
	fputs("P3\n", fp);
	fprintf(fp, "%u %u\n", image.m_width, image.m_height);
	fputs("255\n", fp);
	int i = 0;
	RGBA* p = image.m_pixels;
	for (int j = 0; j < image.m_packets; j++)
	{
		fprintf(fp, "%d %d %d ", p->red, p->green, p->blue);
		i++;
		if (i == 4)
		{
			fputc('\n', fp);
			i = 0;
		}
		p++;
	}
}

static void pnmsave_p4(mif& image, FILE* fp)
{
	register BYTE bit, byte, polarity;

	fputs("P4\n", fp);
	fprintf(fp, "%u %u\n", image.m_width, image.m_height);
	polarity = Intensity(image.m_map[0]) > (0.5 * 255);
	if (image.m_colors == 2)
		polarity = Intensity(image.m_map[0]) > Intensity(image.m_map[1]);

	RGBA* p = image.m_pixels;
	for (int y = 0; y < image.m_height; y++)
	{
		bit = 0;
		byte = 0;
		for (int x = 0; x < image.m_width; x++)
		{
			byte <<= 1;
			if (p->alpha == polarity)
				byte |= 0x01;
			bit++;
			if (bit == 8)
			{
				fputc(byte, fp);
				bit = 0;
				byte = 0;
			}
			p++;
		}
		if (bit != 0)
			fputc(byte << (8 - bit), fp);
	}
}

static void pnmsave_p5(mif& image, FILE* fp)
{
	fputs("P5\n", fp);
	fprintf(fp, "%u %u\n", image.m_width, image.m_height);
	fputs("255\n", fp);
	RGBA* p = image.m_pixels;
	for (int i = 0; i < image.m_packets; i++)
	{
		fputc((BYTE)(Intensity(*p) + 0.5), fp);
		p++;
	}
}

static void pnmsave_p6(mif& image, FILE* fp)
{
	fputs("P6\n", fp);
	fprintf(fp, "%u %u\n", image.m_width, image.m_height);
	fputs("255\n", fp);

	RGBA* p = image.m_pixels;
	for (int i = 0; i < image.m_packets; i++)
	{
		fputc(p->red, fp);
		fputc(p->green, fp);
		fputc(p->blue, fp);
		p++;
	}
}

static void pnmsave_p7(mif& image, FILE* fp, BOOL dither)
{
	fputs("P7 332\n", fp);

	int value, i, j, x, y;
	USHORT *b_map[2][16], *g_map[2][16], *r_map[2][16];

	for (i = 0; i < 2; i++)
		for (j = 0; j < 16; j++)
		{
			r_map[i][j] = (USHORT*)malloc(256*sizeof(unsigned short));
			g_map[i][j] = (USHORT*)malloc(256*sizeof(unsigned short));
			b_map[i][j] = (USHORT*)malloc(256*sizeof(unsigned short));
		}

	// Initialize dither tables.
	for (i = 0; i < 2; i++)
		for (j = 0; j < 16; j++)
			for (x=0; x < 256; x++)
			{
				value = x - 16;
				if (x < 48)
					value = x / 2 + 8;
				value += ditherR[i][j];
				r_map[i][j][x] =
						(USHORT)((value < 0) ? 0 : (value > 255) ? 255 : value);
				value = x - 16;
				if (x < 48)
					value = x / 2 + 8;
				value += ditherG[i][j];
				g_map[i][j][x] =
						(USHORT)((value < 0) ? 0 : (value > 255) ? 255 : value);
				value = x - 32;
				if (x < 112)
					value = x / 2 + 24;
				value += 2 * ditherB[i][j];
				b_map[i][j][x] =
						(USHORT)((value < 0) ? 0 : (value > 255) ? 255 : value);
			}

	// Convert image to a P7 image.
	fputs("#END_OF_COMMENTS\n", fp);
	fprintf(fp,"%u %u 255\n",image.m_width, image.m_height);
	i = 0;
	j = 0;
	RGBA* p = image.m_pixels;
	BYTE pixel;
	for (y = 0; y < image.m_height; y++)
	{
		for (x = 0; x < image.m_width; x++)
		{
			if (!dither)
				pixel = ((p->red & 0xe0) |
						((p->green & 0xe0) >> 3) |
						((p->blue & 0xc0) >> 6));
			else
				pixel = ((r_map[i][j][p->red] & 0xe0) |
						((g_map[i][j][p->green] & 0xe0) >> 3) |
						((b_map[i][j][p->blue] & 0xc0) >> 6));
			fputc(pixel, fp);
			p++;
			j++;
			if (j == 16)
				j=0;
		}
		i++;
		if (i == 2)
			i=0;
	}

	// Free allocated memory.
	for (i = 0; i < 2; i++)
		for (j = 0; j < 16; j++)
		{
			free(g_map[i][j]);
			free(b_map[i][j]);
			free(r_map[i][j]);
		}
}

static BOOL pnm_save_core(FILE* fp, HDIB hdib)
{
	mif image;
	if (!image.attachdib(hdib))
		return FALSE;

	if (image.m_bits == 2)
		pnmsave_p4(image, fp);
	if (image.m_bits == 24)
		pnmsave_p6(image, fp);
	else
	{
		BOOL bGray = TRUE;
		for (int i = 0; i < image.m_colors; i++)
		{
			BYTE val = image.m_map[i].red;
			if (val != image.m_map[i].green || val != image.m_map[i].blue)
			{
				bGray = FALSE;
				break;
			}
		}
		if (bGray)
			pnmsave_p5(image, fp);
		else
			pnmsave_p7(image, fp, FALSE);
	}
	return TRUE;
}

extern "C"
BOOL WINAPI imgPnmSave(LPCTSTR pszName, HDIB hdib)
{
	BOOL bOK = FALSE;
	if (hdib && pszName && *pszName) {
		FILE* fp = _tfopen(pszName, _T("wb"));
		if (fp) {
			bOK = pnm_save_core(fp, hdib);
			fclose(fp);
		}
	}
	return bOK;
}

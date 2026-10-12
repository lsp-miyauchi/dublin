/*
 * Copyright (c) 1999-2000 Image Power, Inc. and the University of
 *   British Columbia.
 * Copyright (c) 2001 Michael David Adams.
 * All rights reserved.
 */


#ifndef LSP_COD_H
#define LSP_COD_H

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jasper/jas_types.h"

/******************************************************************************\
* Constants and macros.
\******************************************************************************/

#define	BMP_MAGIC	0x4d42
/* The signature for a BMP file. */

#define	BMP_HDRLEN	14
/* The nominal header length. */

#define BMP_INFOLEN	40
/* The nominal info length. */

#define BMP_PALLEN(info)	((info)->numcolors * 4)
/* The length of the palette. */

#define	BMP_HASPAL(info)	((info)->numcolors > 0)
/* Is this a palettized image? */

/* Encoding types. */
#define	BMP_ENC_RGB		0 /* No special encoding. */
#define	BMP_ENC_RLE8	1 /* Run length encoding. */
#define	BMP_ENC_RLE4	2 /* Run length encoding. */

/******************************************************************************\
* Types.
\******************************************************************************/

/* BMP header. */
typedef struct {

	int_fast16_t magic;
	/* The signature (a.k.a. the magic number). */

	int_fast32_t siz;
	/* The size of the file in 32-bit words. */

	int_fast16_t reserved1;
	/* Ask Bill Gates what this is all about. */

	int_fast16_t reserved2;
	/* Ditto. */

	int_fast32_t off;
	/* The offset of the bitmap data from the bitmap file header in bytes. */

} lsp_hdr_t;

/* Palette entry. */
typedef struct {

	int_fast16_t red;
	/* The red component. */

	int_fast16_t grn;
	/* The green component. */

	int_fast16_t blu;
	/* The blue component. */

	int_fast16_t res;
	/* Reserved. */

} lsp_palent_t;

/* BMP info. */
typedef struct {

	int_fast32_t len;
	/* The length of the bitmap information header in bytes. */

	int_fast32_t width;
	/* The width of the bitmap in pixels. */

	int_fast32_t height;
	/* The height of the bitmap in pixels. */

	int_fast8_t topdown;
	/* The bitmap data is specified in top-down order. */

	int_fast16_t numplanes;
	/* The number of planes.  This must be set to a value of one. */

	int_fast16_t depth;
	/* The number of bits per pixel. */

	int_fast32_t enctype;
	/* The type of compression used. */

	int_fast32_t siz;
	/* The size of the image in bytes. */

	int_fast32_t hres;
	/* The horizontal resolution in pixels/metre. */

	int_fast32_t vres;
	/* The vertical resolution in pixels/metre. */

	int_fast32_t numcolors;
	/* The number of color indices used by the bitmap. */

	int_fast32_t mincolors;
	/* The number of color indices important for displaying the bitmap. */

	lsp_palent_t *palents;
	/* The colors should be listed in order of importance. */

} lsp_info_t;

/******************************************************************************\
* Functions and macros.
\******************************************************************************/

#define	lsp_issupported(info) \
	((info)->numplanes == 1 && \
	  ((info)->depth == 8 || (info)->depth == 24) && \
	  (info)->enctype == BMP_ENC_RGB)
/* Is this type of BMP file supported? */

#define	lsp_haspal(info) \
	((info)->depth == 8)
/* Is there a palette? */

int lsp_numcmpts(lsp_info_t *info);
/* Get the number of components. */

lsp_info_t *lsp_info_create();
/* Create BMP information. */

void lsp_info_destroy(lsp_info_t *info);
/* Destroy BMP information. */

int lsp_isgrayscalepal(lsp_palent_t *palents, int numpalents);
/* Does the specified palette correspond to a grayscale image? */

#endif

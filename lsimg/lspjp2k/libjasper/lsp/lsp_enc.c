/******************************************************************************\
* Includes.
\******************************************************************************/

#include <assert.h>

#include "jasper/jas_types.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"

#include "lsp_cod.h"

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

static int lsp_putinfo(jas_stream_t *out, lsp_info_t *info);
static int lsp_putdata(jas_stream_t *out, lsp_info_t *info, jas_image_t *image);
static int lsp_putint16(jas_stream_t *in, int_fast16_t val);
static int lsp_putint32(jas_stream_t *out, int_fast32_t val);

/******************************************************************************\
* Interface functions.
\******************************************************************************/

int lsp_encode(jas_image_t *image, jas_stream_t *out, char *optstr)
{
	uint_fast32_t width;
	uint_fast32_t height;
	uint_fast16_t depth;
	uint_fast16_t cmptno;
	lsp_info_t *info;
	int_fast32_t datalen;
	int numpad;
	uint_fast16_t numcmpts;

	numcmpts = jas_image_numcmpts(image);
	width = jas_image_cmptwidth(image, 0);
	height = jas_image_cmptheight(image, 0);
	depth = jas_image_cmptprec(image, 0);

	/* Check to ensure that the image to be saved can actually be represented
	  using the BMP format. */
	for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
		if (jas_image_cmptwidth(image, cmptno) != width ||
		  jas_image_cmptheight(image, cmptno) != height ||
		  jas_image_cmptprec(image, cmptno) != depth ||
		  jas_image_cmptsgnd(image, cmptno) != false ||
		  jas_image_cmpttlx(image, cmptno) != 0 ||
		  jas_image_cmpttly(image, cmptno) != 0) {
			fprintf(stderr, "The BMP format cannot be used to represent an image with this geometry.\n");
			return -1;
		}
	}

	/* The component depths must be 1, 4, or 8. */
	if (depth != 1 && depth != 4 && depth != 8) {
		return -1;
	}

	numpad = (width * numcmpts) % 4;
	if (numpad) {
		numpad = 4 - numpad;
	}
	datalen = (numcmpts * width + numpad) * height;

	if (!(info = lsp_info_create())) {
		return -1;
	}
	info->len = BMP_INFOLEN;
	info->width = width;
	info->height = height;
	info->numplanes = 1;
	info->depth = numcmpts * depth;
	info->enctype = BMP_ENC_RGB;
	info->siz = datalen;
	info->hres = 0;
	info->vres = 0;
	info->numcolors = (numcmpts == 1) ? 256 : 0;
	info->mincolors = 0;

	/* Write the bitmap information. */
	if (lsp_putinfo(out, info)) {
		return -1;
	}

	/* Write the bitmap data. */
	if (lsp_putdata(out, info, image)) {
		return -1;
	}

	return 0;
}

/******************************************************************************\
* Code for aggregate types.
\******************************************************************************/

static int lsp_putinfo(jas_stream_t *out, lsp_info_t *info)
{
	int i;

	info->len = 40;
	if (lsp_putint32(out, info->len) ||
		lsp_putint32(out, info->width) ||
		lsp_putint32(out, info->height) ||
		lsp_putint16(out, info->numplanes) ||
		lsp_putint16(out, info->depth) ||
		lsp_putint32(out, info->enctype) ||
		lsp_putint32(out, info->siz) ||
		lsp_putint32(out, info->hres) ||
		lsp_putint32(out, info->vres) ||
		lsp_putint32(out, info->numcolors) ||
		lsp_putint32(out, info->mincolors)) {
		return -1;
	}

	for (i = 0; i < info->numcolors; ++i)
	{
		if (jas_stream_putc(out, i) == EOF ||
			jas_stream_putc(out, i) == EOF ||
			jas_stream_putc(out, i) == EOF ||
			jas_stream_putc(out, 0) == EOF)
		{
			return -1;
		}
	}

	return 0;
}

static int lsp_putdata(jas_stream_t *out, lsp_info_t *info, jas_image_t *image)
{
	int i;
	int j;
	jas_matrix_t *cmpts[3];
	int numpad;
	unsigned char red;
	unsigned char grn;
	unsigned char blu;
	int ret;
	int numcmpts;
	int v;
	int cmptno;

	numcmpts = (info->depth == 24) ? 3:1;

	/* We do not support palettized images. */
	if (BMP_HASPAL(info) && numcmpts == 3) {
		fprintf(stderr, "no palettized image support for BMP format\n");
		return -1;
	}

	ret = 0;
	for (i = 0; i < numcmpts; ++i) {
		cmpts[i] = 0;
	}

	/* Create temporary matrices to hold component data. */
	for (i = 0; i < numcmpts; ++i) {
		if (!(cmpts[i] = jas_matrix_create(1, info->width))) {
			ret = -1;
			goto lsp_putdata_done;
		}
	}

	/* Calculate number of padding bytes per row of image data. */
	numpad = (numcmpts * info->width) % 4;
	if (numpad) {
		numpad = 4 - numpad;
	}

	/* Put the image data. */
	for (i = info->height - 1; i >= 0; --i) {
		for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
			if (jas_image_readcmpt(image, cmptno, 0, i, info->width, 1, cmpts[cmptno])) {
				ret = -1;
				goto lsp_putdata_done;
			}
		}
		for (j = 0; j < info->width; ++j) {
			if (numcmpts == 3) {
				red = (jas_matrix_getv(cmpts[0], j));
				grn = (jas_matrix_getv(cmpts[1], j));
				blu = (jas_matrix_getv(cmpts[2], j));
				if (jas_stream_putc(out, blu) == EOF ||
					jas_stream_putc(out, grn) == EOF ||
					jas_stream_putc(out, red) == EOF) {
					ret = -1;
					goto lsp_putdata_done;
				}
			} else if (numcmpts == 1) {
				v = (jas_matrix_getv(cmpts[0], j));
				if (jas_stream_putc(out, v) == EOF) {
					ret = -1;
					goto lsp_putdata_done;
				}
			} else {
				abort();
			}
		}
		for (j = numpad; j > 0; --j) {
			if (jas_stream_putc(out, 0) == EOF) {
				ret = -1;
				goto lsp_putdata_done;
			}
		}
	}

lsp_putdata_done:
	/* Destroy the temporary matrices. */
	for (i = 0; i < numcmpts; ++i) {
		if (cmpts[i]) {
			jas_matrix_destroy(cmpts[i]);
		}
	}

	return ret;
}

/******************************************************************************\
* Code for primitive types.
\******************************************************************************/

static int lsp_putint16(jas_stream_t *in, int_fast16_t val)
{
	if (jas_stream_putc(in, val & 0xff) == EOF || jas_stream_putc(in, (val >> 8) &
	  0xff) == EOF) {
		return -1;
	}
	return 0;
}

static int lsp_putint32(jas_stream_t *out, int_fast32_t val)
{
	int n;
	int_fast32_t v;

	/* This code needs to be changed if we want to handle negative values. */
	assert(val >= 0);
	v = val;
	for (n = 4;;) {
		if (jas_stream_putc(out, v & 0xff) == EOF) {
			return -1;
		}
		if (--n <= 0) {
			break;
		}
		v >>= 8;
	}
	return 0;
}

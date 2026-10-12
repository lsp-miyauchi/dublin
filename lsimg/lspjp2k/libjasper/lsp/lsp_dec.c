/******************************************************************************\
* Includes.
\******************************************************************************/

#include <assert.h>

#include "jasper/jas_types.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"
#include "jasper/jas_malloc.h"

#include "lsp_cod.h"

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

static lsp_info_t *lsp_getinfo(jas_stream_t *in);
static int lsp_getdata(jas_stream_t *in, lsp_info_t *info, jas_image_t *image);
static int lsp_getint16(jas_stream_t *in, int_fast16_t *val);
static int lsp_getint32(jas_stream_t *in, int_fast32_t *val);
static int lsp_gobble(jas_stream_t *in, long n);

/******************************************************************************\
* Interface functions.
\******************************************************************************/

jas_image_t *lsp_decode(jas_stream_t *in, char *optstr)
{
	jas_image_t* image;
	lsp_info_t* info;
	uint_fast16_t cmptno;
	jas_image_cmptparm_t cmptparms[3];
	jas_image_cmptparm_t *cmptparm;
	uint_fast16_t numcmpts;

	/* Read the bitmap information. */
	if (!(info = lsp_getinfo(in))) {
		fprintf(stderr, "cannot get info\n");
		return 0;
	}

	/* Ensure that we support this type of BMP file. */
	if (!lsp_issupported(info)) {
		fprintf(stderr, "error: unsupported BMP format\n");
		lsp_info_destroy(info);
		return 0;
	}

	/* Get the number of components. */
	numcmpts = lsp_numcmpts(info);

	for (cmptno = 0, cmptparm = cmptparms; cmptno < numcmpts; ++cmptno, ++cmptparm) {
		cmptparm->tlx = 0;
		cmptparm->tly = 0;
		cmptparm->hstep = 1;
		cmptparm->vstep = 1;
		cmptparm->width = info->width;
		cmptparm->height = info->height;
		cmptparm->prec = 8;
		cmptparm->sgnd = false;
	}

	/* Create image object. */
	if (!(image = jas_image_create(numcmpts, cmptparms, JAS_CLRSPC_UNKNOWN))) {
		lsp_info_destroy(info);
		return 0;
	}

	if (numcmpts == 3) {
		jas_image_setclrspc(image, JAS_CLRSPC_SRGB);
		jas_image_setcmpttype(image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
		jas_image_setcmpttype(image, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
		jas_image_setcmpttype(image, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
	} else {
		jas_image_setclrspc(image, JAS_CLRSPC_SGRAY);
		jas_image_setcmpttype(image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
	}

	/* Read the bitmap data. */
	if (lsp_getdata(in, info, image)) {
		lsp_info_destroy(info);
		jas_image_destroy(image);
		return 0;
	}

	lsp_info_destroy(info);
	return image;
}

int lsp_validate(jas_stream_t *in)
{
	int n;
	int i;
	uchar buf[4];

	assert(JAS_STREAM_MAXPUTBACK >= 2);

	/* Read the first two characters that constitute the signature. */
	if ((n = jas_stream_read(in, (char *) buf, 4)) < 0) {
		return -1;
	}
	/* Put the characters read back onto the stream. */
	for (i = n - 1; i >= 0; --i) {
		if (jas_stream_ungetc(in, buf[i]) == EOF) {
			return -1;
		}
	}
	/* Did we read enough characters? */
	if (n < 4) {
		return -1;
	}

	/* Is the BITMAPINFOHEADER size (= 0x00000040) correct for the BMP format? */
	if (buf[0] == 0x28 && buf[1] == 0 && buf[2] == 0 && buf[3] == 0) {
		return 0;
	}
	return -1;
}

/******************************************************************************\
* Code for aggregate types.
\******************************************************************************/

static lsp_info_t *lsp_getinfo(jas_stream_t *in)
{
	lsp_info_t *info;
	int i;
	lsp_palent_t *palent;

	if (!(info = lsp_info_create())) {
		return 0;
	}

	if (lsp_getint32(in, &info->len) || info->len != 40 ||
	  lsp_getint32(in, &info->width) || lsp_getint32(in, &info->height) ||
	  lsp_getint16(in, &info->numplanes) ||
	  lsp_getint16(in, &info->depth) || lsp_getint32(in, &info->enctype) ||
	  lsp_getint32(in, &info->siz) ||
	  lsp_getint32(in, &info->hres) || lsp_getint32(in, &info->vres) ||
	  lsp_getint32(in, &info->numcolors) ||
	  lsp_getint32(in, &info->mincolors)) {
		lsp_info_destroy(info);
		return 0;
	}

	if (info->height < 0) {
		info->topdown = 1;
		info->height = -info->height;
	} else {
		info->topdown = 0;
	}

	if (info->width <= 0 || info->height <= 0 || info->numplanes <= 0 ||
	  info->depth <= 0  || info->numcolors < 0 || info->mincolors < 0) {
		lsp_info_destroy(info);
		return 0;
	}

	if (info->enctype != BMP_ENC_RGB) {
		fprintf(stderr, "unsupported BMP format\n");
		lsp_info_destroy(info);
		return 0;
	}

	if (info->numcolors > 0) {
		if (!(info->palents = (lsp_palent_t*)jas_malloc(info->numcolors *
		  sizeof(lsp_palent_t)))) {
			lsp_info_destroy(info);
			return 0;
		}
	} else {
		info->palents = 0;
	}

	for (i = 0; i < info->numcolors; ++i) {
		palent = &info->palents[i];
		if ((palent->blu = jas_stream_getc(in)) == EOF ||
		  (palent->grn = jas_stream_getc(in)) == EOF ||
		  (palent->red = jas_stream_getc(in)) == EOF ||
		  (palent->res = jas_stream_getc(in)) == EOF) {
			lsp_info_destroy(info);
			return 0;
		}
	}

	return info;
}

static int lsp_getdata(jas_stream_t *in, lsp_info_t *info, jas_image_t *image)
{
	int i;
	int j;
	int y;
	jas_matrix_t *cmpts[3];
	int numpad;
	int red;
	int grn;
	int blu;
	int ret;
	int numcmpts;
	int cmptno;
	int ind;
	lsp_palent_t *palent;
	int mxind;
	int haspal;

	assert(info->depth == 8 || info->depth == 24);
	assert(info->enctype == BMP_ENC_RGB);

	numcmpts = lsp_numcmpts(info);
	haspal = lsp_haspal(info);

	ret = 0;
	for (i = 0; i < numcmpts; ++i) {
		cmpts[i] = 0;
	}

	/* Create temporary matrices to hold component data. */
	for (i = 0; i < numcmpts; ++i) {
		if (!(cmpts[i] = jas_matrix_create(1, info->width))) {
			ret = -1;
			goto lsp_getdata_done;
		}
	}

	/* Calculate number of padding bytes per row of image data. */
	numpad = (numcmpts * info->width) % 4;
	if (numpad) {
		numpad = 4 - numpad;
	}

	mxind = (1 << info->depth) - 1;
	for (i = 0; i < info->height; ++i) {
		for (j = 0; j < info->width; ++j) {
			if (haspal) {
				if ((ind = jas_stream_getc(in)) == EOF) {
					ret = -1;
					goto lsp_getdata_done;
				}
				if (ind > mxind) {
					ret = -1;
					goto lsp_getdata_done;
				}
				if (ind < info->numcolors) {
					palent = &info->palents[ind];
					red = palent->red;
					grn = palent->grn;
					blu = palent->blu;
				} else {
					red = ind;
					grn = ind;
					blu = ind;
				}
			} else {
				if ((blu = jas_stream_getc(in)) == EOF ||
					(grn = jas_stream_getc(in)) == EOF ||
					(red = jas_stream_getc(in)) == EOF) {
					ret = -1;
					goto lsp_getdata_done;
				}
			}
			if (numcmpts == 3) {
				jas_matrix_setv(cmpts[0], j, red);
				jas_matrix_setv(cmpts[1], j, grn);
				jas_matrix_setv(cmpts[2], j, blu);
			} else {
				jas_matrix_setv(cmpts[0], j, red);
			}
		}
		for (j = numpad; j > 0; --j) {
			if (jas_stream_getc(in) == EOF) {
				ret = -1;
				goto lsp_getdata_done;
			}
		}
		for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
			y = info->topdown ? i : (info->height - 1 - i);
			if (jas_image_writecmpt(image, cmptno, 0, y, info->width, 1, cmpts[cmptno])) {
				ret = -1;
				goto lsp_getdata_done;
			}
		}
	}

lsp_getdata_done:
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

static int lsp_getint16(jas_stream_t *in, int_fast16_t *val)
{
	int lo;
	int hi;
	if ((lo = jas_stream_getc(in)) == EOF || (hi = jas_stream_getc(in)) == EOF) {
		return -1;
	}
	if (val) {
		*val = (hi << 8) | lo;
	}
	return 0;
}

static int lsp_getint32(jas_stream_t *in, int_fast32_t *val)
{
	int n;
	uint_fast32_t v;
	int c;
	for (n = 4, v = 0;;) {
		if ((c = jas_stream_getc(in)) == EOF) {
			return -1;
		}
		v |= (c << 24);
		if (--n <= 0) {
			break;
		}
		v >>= 8;
	}
	if (val) {
		*val = v;
	}
	return 0;
}

static int lsp_gobble(jas_stream_t *in, long n)
{
	while (--n >= 0) {
		if (jas_stream_getc(in) == EOF) {
			return -1;
		}
	}
	return 0;
}

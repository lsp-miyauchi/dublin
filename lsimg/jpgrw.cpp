////////////////////////////////////////////////////////////////////////
// jpgrw read/write between jpeg file and mifimage

#include "stdafx.h"
#include "lsimg.h"
#include "dib_sup.h"

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

#include "jpgcode.h"		/* Common decls for cjpeg/djpeg applications */

#ifdef __cplusplus
}
#endif


// Private version of data source object

typedef struct _image_source_struct {		// JPEG COMPRESSION
	struct cjpeg_source_struct pub;			// 共通データフィールド
	UINT cur_row;							// 現在処理中の行
} image_source_struct, *image_source_ptr;

typedef struct {							// JPEG DECOMPRESSION
	struct djpeg_dest_struct pub;			// 共通データフィールド
	UINT cur_row;							// 現在処理中の行
} image_dest_struct, *image_dest_ptr;

typedef struct {
	struct jpeg_source_mgr pub;			/* public fields */
	MemStream* infile;						/* source stream */
	JOCTET * buffer;					/* start of buffer */
	boolean start_of_file;				/* have we gotten any data yet? */
	int orientation;					// 画像の向き
} my_source_mgr;
typedef my_source_mgr * my_src_ptr;

typedef struct {
	struct jpeg_destination_mgr pub;	/* public fields */
	MemStream* outfile;						/* target stream */
	JOCTET * buffer;					/* start of buffer */
} my_destination_mgr;
typedef my_destination_mgr * my_dest_ptr;

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)


#define OUTPUT_BUF_SIZE 	0x4000	/* choose an efficiently fwrite'able size */
#define INPUT_BUF_SIZE  	4096	/* choose an efficiently fread'able size */


//////////////////////////////////////////////////////////////////////
// jpeg encode here

METHODDEF(JDIMENSION)
get_mapcolor_row(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
	image_source_ptr src = (image_source_ptr)sinfo;
	register JSAMPROW outptr;
	register UINT col;

	// Fetch next row from virtual array
	mif* image = (mif*)((j_common_ptr)cinfo)->client_data;
	register RGBA* pb = image->pos(0, src->cur_row);
	src->cur_row++;

	// Expand the colormap indexes to real data
	outptr = src->pub.buffer[0];
	for (col = 0; col < cinfo->image_width; col++)
	{
		*outptr++ = image->m_map[pb->alpha].red;
		*outptr++ = image->m_map[pb->alpha].green;
		*outptr++ = image->m_map[pb->alpha].blue;
		pb++;
	}
	return 1;
}


METHODDEF(JDIMENSION)
get_rgbcolor_row(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
	image_source_ptr src = (image_source_ptr)sinfo;
	register JSAMPROW outptr;
	register UINT col;

	// 読み込む行位置
	mif* image = (mif*)((j_common_ptr)cinfo)->client_data;
	RGBA* pb = image->pos(0, src->cur_row);
	src->cur_row++;

	outptr = src->pub.buffer[0];
	for (col = 0; col < cinfo->image_width; col++) {
		*outptr++ = pb->red;
		*outptr++ = pb->green;
		*outptr++ = pb->blue;
		pb++;
	}
	return 1;
}

METHODDEF(JDIMENSION)
preload_image(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
	image_source_ptr src = (image_source_ptr) sinfo;
	mif* image = (mif*)((j_common_ptr)cinfo)->client_data;

	if (image->m_bits == 24)
		src->pub.get_pixel_rows = get_rgbcolor_row;
	else
		src->pub.get_pixel_rows = get_mapcolor_row;
	src->cur_row = 0;

	// 最初の行をまず読む
	return (*src->pub.get_pixel_rows)(cinfo, sinfo);
}


METHODDEF(void)
start_input_image(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
	image_source_ptr src = (image_source_ptr)sinfo;
	mif* image = (mif*)((j_common_ptr)cinfo)->client_data;
	src->pub.get_pixel_rows = preload_image;

	// １行分のデータバッファを確保
	src->pub.buffer = (*cinfo->mem->alloc_sarray)
							((j_common_ptr)cinfo, JPOOL_IMAGE, image->m_width * 3, 1);
	src->pub.buffer_height = 1;

	cinfo->in_color_space = JCS_RGB;
	cinfo->input_components = 3;
	cinfo->data_precision = 8;
	cinfo->image_width = image->m_width;
	cinfo->image_height = image->m_height;
}

METHODDEF(void)
finish_input_image(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
	;
}

GLOBAL(cjpeg_source_ptr)
jinit_read_image(j_compress_ptr cinfo)
{
	image_source_ptr src;

	// モジュールインタフェースオブジェクトの作成
	src = (image_source_ptr)
				(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				sizeof(image_source_struct));

	// メソッド関数ポインタの設定
	src->pub.start_input = start_input_image;
	src->pub.finish_input = finish_input_image;

	return (cjpeg_source_ptr)src;
}

//////////////////////////////////////////////////////////////////////
// jpeg decode here
// デコード時は高速化のためCDibHandleを利用する

METHODDEF(void)
put_pixel_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo, JDIMENSION rows_supplied)
{
	image_dest_ptr dst = (image_dest_ptr)dinfo;
	CDibHandle* dib = (CDibHandle*)cinfo->client_data;
	register BYTE* pb;
	register JSAMPROW inptr;
	register UINT col;

	inptr = dst->pub.buffer[0];

	my_src_ptr src = (my_src_ptr)cinfo->src;
	if (src->orientation == ORI_ROTATERIGHT) {
		// 右へ90度
		pb = (BYTE*)dib->RowPosition(0) + (dib->m_width - dst->cur_row - 1) * 3;
		for (col = 0; col < cinfo->output_width; col++) {
			pb[2] = *inptr++;
			pb[1] = *inptr++;
			pb[0] = *inptr++;
			pb = (BYTE*)dib->RowDown(pb);
		}
	}
	else if (src->orientation == ORI_ROTATERIGHTFLIPY) {
		// 右へ90度 + 上下反転
		pb = (BYTE*)dib->RowPosition(dib->m_height - 1) + (dib->m_width - dst->cur_row - 1) * 3;

		for (col = 0; col < cinfo->output_width; col++) {
			pb[2] = *inptr++;
			pb[1] = *inptr++;
			pb[0] = *inptr++;
			pb = (BYTE*)dib->RowUp(pb);
		}
	}
	else if (src->orientation == ORI_ROTATELEFT) {
		// 左へ90度
		pb = (BYTE*)dib->RowPosition(dib->m_height - 1) + dst->cur_row * 3;

		for (col = 0; col < cinfo->output_width; col++) {
			pb[2] = *inptr++;
			pb[1] = *inptr++;
			pb[0] = *inptr++;
			pb = (BYTE*)dib->RowUp(pb);
		}
	}
	else if (src->orientation == ORI_ROTATELEFT) {
		// 左へ90度 + 上下反転
		pb = (BYTE*)dib->RowPosition(0) + dst->cur_row * 3;
		for (col = 0; col < cinfo->output_width; col++) {
			pb[2] = *inptr++;
			pb[1] = *inptr++;
			pb[0] = *inptr++;
			pb = (BYTE*)dib->RowDown(pb);
		}
	}
	else if (src->orientation == ORI_FLIPX) {
		// 左右反転
		pb = (BYTE*)dib->RowPosition(dst->cur_row) + (dib->m_width - 1) * 3;
		for (col = 0; col < cinfo->output_width; col++) {
			pb[2] = *inptr++;
			pb[1] = *inptr++;
			pb[0] = *inptr++;
			pb -= 3;
		}
	}
	else if (src->orientation == ORI_FLIPY) {
		// 上下反転
		pb = (BYTE*)dib->RowPosition(dib->m_height - dst->cur_row - 1);
		for (col = 0; col < cinfo->output_width; col++) {
			pb[2] = *inptr++;
			pb[1] = *inptr++;
			pb[0] = *inptr++;
			pb += 3;
		}
	}
	else if (src->orientation == ORI_ROTATE180) {
		// 180°回転
		pb = (BYTE*)dib->RowPosition(dib->m_height - dst->cur_row - 1) + (dib->m_width - 1) * 3;
		for (col = 0; col < cinfo->output_width; col++) {
			pb[2] = *inptr++;
			pb[1] = *inptr++;
			pb[0] = *inptr++;
			pb -= 3;
		}
	}
	else {
		pb = (BYTE*)dib->RowPosition(dst->cur_row);
		for (col = 0; col < cinfo->output_width; col++) {
			pb[2] = *inptr++;
			pb[1] = *inptr++;
			pb[0] = *inptr++;
			pb += 3;
		}
	}
	dst->cur_row++;
}

METHODDEF(void)
put_gray_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo, JDIMENSION rows_supplied)
{
	image_dest_ptr dst = (image_dest_ptr)dinfo;
	CDibHandle* dib = (CDibHandle*)cinfo->client_data;
	register BYTE* pb;
	register JSAMPROW inptr;
	register UINT col;

	pb = (BYTE*)dib->RowPosition(dst->cur_row);
	dst->cur_row++;

	inptr = dst->pub.buffer[0];
	for (col = 0; col < cinfo->output_width; col++) {
		*pb++ = *inptr++;
	}
}

METHODDEF(void)
start_output_image(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
	// イメージの作成
	int bits;
	if (cinfo->out_color_space != JCS_RGB || cinfo->quantize_colors)
		bits = 8;
	else
		bits = 24;

	my_src_ptr src = (my_src_ptr)cinfo->src;
	int width = cinfo->output_width;
	int height = cinfo->output_height;
	switch (src->orientation) {
	case ORI_ROTATELEFTFLIPY:
	case ORI_ROTATERIGHT:
	case ORI_ROTATERIGHTFLIPY:
	case ORI_ROTATELEFT:
		width = cinfo->output_height;
		height = cinfo->output_width;
		break;
	}
	HDIB hdib = dib_create(width, height, bits);
	CDibHandle* dib = new CDibHandle(hdib);
	cinfo->client_data = dib;
}

LOCAL(void)
write_colormap(j_decompress_ptr cinfo)
{
	JSAMPARRAY colormap = cinfo->colormap;
	int num_colors = cinfo->actual_number_of_colors;
	CDibHandle* dib = (CDibHandle*)((j_common_ptr)cinfo)->client_data;
	register int i;

	register RGBQUAD* pc = dib->ColorMap();
	if (colormap != NULL) {
		if (cinfo->out_color_components == 3) {
			// Normal case with RGB colormap
			for (i = 0; i < num_colors; i++) {
				pc->rgbRed = GETJSAMPLE(colormap[0][i]);
				pc->rgbGreen = GETJSAMPLE(colormap[1][i]);
				pc->rgbBlue = GETJSAMPLE(colormap[2][i]);
				pc++;
			}
		}
		else {
			// Grayscale colormap (only happens with grayscale quantization)
			for (i = 0; i < num_colors; i++) {
				pc->rgbRed = GETJSAMPLE(colormap[0][i]);
				pc->rgbGreen = GETJSAMPLE(colormap[0][i]);
				pc->rgbBlue = GETJSAMPLE(colormap[0][i]);
				pc++;
			}
		}
	}
	else {
		// If no colormap, must be grayscale data.  Generate a linear "map".
		for (i = 0; i < 256; i++) {
			pc->rgbRed = pc->rgbGreen = pc->rgbBlue = i;
			pc++;
		}
	}
}

METHODDEF(void)
finish_output_image(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
	// カラーマップの計算
	if (cinfo->out_color_space != JCS_RGB || cinfo->quantize_colors)
		write_colormap(cinfo);
}

GLOBAL(djpeg_dest_ptr)
jinit_write_image(j_decompress_ptr cinfo)
{
	image_dest_ptr dst;
	int width;

	// モジュールインタフェースオブジェクトを作成し、
	// メソッド関数ポインタを設定する
	dst = (image_dest_ptr)(*cinfo->mem->alloc_small)
							((j_common_ptr) cinfo, JPOOL_IMAGE,
							sizeof(image_dest_struct));
	dst->pub.start_output = start_output_image;
	dst->pub.finish_output = finish_output_image;

	if (cinfo->out_color_space == JCS_GRAYSCALE)
		dst->pub.put_pixel_rows = put_gray_rows;
	else if (cinfo->out_color_space == JCS_RGB) {
		if (cinfo->quantize_colors)
			dst->pub.put_pixel_rows = put_gray_rows;
		else
			dst->pub.put_pixel_rows = put_pixel_rows;
	}
	else
		return NULL;

	// Calculate output image dimensions so we can allocate space
	jpeg_calc_output_dimensions(cinfo);

	dst->cur_row = 0;
	if (cinfo->progress != NULL) {
		cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
		progress->total_extra_passes++;		// count file input as separate pass
	}

	// Create decompressor output buffer.
	width = cinfo->output_width * cinfo->output_components;
	dst->pub.buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, width, 1);
	dst->pub.buffer_height = 1;

	return (djpeg_dest_ptr)dst;
}

//////////////////////////////////////////////////////////////////////
// data_source replace

METHODDEF(void)
mf_init_source (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr)cinfo->src;
	src->start_of_file = TRUE;
}

METHODDEF(boolean)
mf_fill_input_buffer (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;
	size_t nbytes;

	nbytes = _mfread(src->infile, src->buffer, INPUT_BUF_SIZE);
	if (nbytes <= 0) {
		if (src->start_of_file)	/* Treat empty input file as fatal error */
			ERREXIT(cinfo, JERR_INPUT_EMPTY);

		WARNMS(cinfo, JWRN_JPEG_EOF);

		/* Insert a fake EOI marker */
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;
	return TRUE;
}

METHODDEF(void)
mf_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void)mf_fill_input_buffer(cinfo);
		}
		src->pub.next_input_byte += (size_t)num_bytes;
		src->pub.bytes_in_buffer -= (size_t)num_bytes;
	}
}

METHODDEF(void)
mf_term_source (j_decompress_ptr cinfo)
{
	/* no work necessary here */
}

GLOBAL(void)
jpeg_mf_stdio_src (j_decompress_ptr cinfo, MemStream* infile, int orientation)
{
	my_src_ptr src;

	if (cinfo->src == NULL) {				/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
							(*cinfo->mem->alloc_small)((j_common_ptr) cinfo,
							JPOOL_PERMANENT, sizeof(my_source_mgr));
		src = (my_src_ptr) cinfo->src;
		src->buffer = (JOCTET *)
							(*cinfo->mem->alloc_small)((j_common_ptr) cinfo,
							JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
	}

	src = (my_src_ptr) cinfo->src;
	src->pub.init_source = mf_init_source;
	src->pub.fill_input_buffer = mf_fill_input_buffer;
	src->pub.skip_input_data = mf_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = mf_term_source;
	src->infile = infile;
	src->pub.bytes_in_buffer = 0;			/* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL;		/* until buffer loaded */
	src->orientation = orientation;			// 画像の向き
}


//////////////////////////////////////////////////////////////////////

METHODDEF(void)
mf_init_destination (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr)cinfo->dest;

	// Allocate the output buffer --- it will be released when done with image
	dest->buffer = (JOCTET*)
					(*cinfo->mem->alloc_small)((j_common_ptr) cinfo,
					JPOOL_IMAGE, OUTPUT_BUF_SIZE * sizeof(JOCTET));

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

METHODDEF(boolean)
mf_empty_output_buffer (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

	_mfwrite(dest->outfile, dest->buffer, OUTPUT_BUF_SIZE);
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
	return TRUE;
}

METHODDEF(void)
mf_term_destination(j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr)cinfo->dest;

	size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;
	if (datacount > 0)
		_mfwrite(dest->outfile, dest->buffer, datacount);
}

GLOBAL(void)
jpeg_mf_stdio_dest(j_compress_ptr cinfo, MemStream* outfile)
{
	my_dest_ptr dest;
	if (cinfo->dest == NULL) {					/* first time for this JPEG object? */
		cinfo->dest = (struct jpeg_destination_mgr *)
							(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo,
							JPOOL_PERMANENT, sizeof(my_destination_mgr));
	}

	dest = (my_dest_ptr)cinfo->dest;
	dest->pub.init_destination = mf_init_destination;
	dest->pub.empty_output_buffer = mf_empty_output_buffer;
	dest->pub.term_destination = mf_term_destination;
	dest->outfile = outfile;
}

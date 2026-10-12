#include "stdafx.h"
#include "ole2.h"
#include "lsimg.h"
#include "common.h"

#define HAVE_BOOLEAN


////////////////////////////////////////////////////////////////////////
//

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

#include "jpgcode.h"			// Common decls for cjpeg/djpeg applications
#include "jversion.h"			// for version message

#ifdef __cplusplus
}
#endif

EXTERN(void) strm_jpeg_stdio_src JPP((j_decompress_ptr cinfo, IStream* infile));

// Private version of data source object

typedef struct {							// JPEG DECOMPRESSION
	struct djpeg_dest_struct pub;			// 共通データフィールド
	UINT cur_row;							// 現在処理中の行
} image_dest_struct, *image_dest_ptr;

typedef struct {
	struct jpeg_source_mgr pub;			/* public fields */
	IStream* infile;					/* source stream */
	JOCTET* buffer;						/* start of buffer */
	boolean start_of_file;				/* have we gotten any data yet? */
} my_source_mgr;
typedef my_source_mgr * my_src_ptr;

#define INPUT_BUF_SIZE  	4096	/* choose an efficiently fread'able size */


//////////////////////////////////////////////////////////////////////
// jpeg decode here
// デコード時は高速化のためCDibHandleを利用する

METHODDEF(void)
strm_put_pixel_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo, JDIMENSION rows_supplied)
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
		pb[2] = *inptr++;
		pb[1] = *inptr++;
		pb[0] = *inptr++;
		pb += 3;
	}
}

METHODDEF(void)
strm_put_gray_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo, JDIMENSION rows_supplied)
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

	HDIB hdib = dib_create(cinfo->output_width, cinfo->output_height, bits);
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
strm_jinit_write_image(j_decompress_ptr cinfo)
{
	// モジュールインタフェースオブジェクトを作成し、
	// メソッド関数ポインタを設定する
	image_dest_ptr dst = (image_dest_ptr)(*cinfo->mem->alloc_small)
									((j_common_ptr) cinfo, JPOOL_IMAGE,
									sizeof(image_dest_struct));
	dst->pub.start_output = start_output_image;
	dst->pub.finish_output = finish_output_image;

	switch (cinfo->out_color_space) {
	case JCS_GRAYSCALE:
		dst->pub.put_pixel_rows = strm_put_gray_rows;
		break;
	case JCS_RGB:
		if (cinfo->quantize_colors)
			dst->pub.put_pixel_rows = strm_put_gray_rows;
		else
			dst->pub.put_pixel_rows = strm_put_pixel_rows;
		break;
	default:
		return NULL;
	}

	// Calculate output image dimensions so we can allocate space
	jpeg_calc_output_dimensions(cinfo);

	dst->cur_row = 0;
	if (cinfo->progress != NULL) {
		cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
		progress->total_extra_passes++;		// count file input as separate pass
	}

	// Create decompressor output buffer.
	int width = cinfo->output_width * cinfo->output_components;
	dst->pub.buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, width, 1);
	dst->pub.buffer_height = 1;

	return (djpeg_dest_ptr)dst;
}

METHODDEF(void)
strm_init_source (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr)cinfo->src;
	src->start_of_file = TRUE;
}

METHODDEF(boolean)
strm_fill_input_buffer (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;

	IStream* pstrm = (IStream*)src->infile;
	ULONG nbytes;
	if (pstrm->Read(src->buffer, INPUT_BUF_SIZE, &nbytes) != S_OK) {
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
strm_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void)strm_fill_input_buffer(cinfo);
		}
		src->pub.next_input_byte += (size_t)num_bytes;
		src->pub.bytes_in_buffer -= (size_t)num_bytes;
	}
}

METHODDEF(void)
strm_term_source (j_decompress_ptr cinfo)
{
	/* no work necessary here */
}

GLOBAL(void)
strm_jpeg_stdio_src (j_decompress_ptr cinfo, IStream* infile)
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

	src = (my_src_ptr)cinfo->src;
	src->pub.init_source = strm_init_source;
	src->pub.fill_input_buffer = strm_fill_input_buffer;
	src->pub.skip_input_data = strm_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = strm_term_source;
	src->infile = infile;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

////////////////////////////////////////////////////////////////////////
//

static const char * const cdjpeg_message_table[] = {
#include "jpgerr.h"
  NULL
};

// quality          = 0 - 100
// dct_method       = JC_ISLOW / JC_IFAST / JC_FLOAT
// smoothing_factor = 0 - 100

METHODDEF(boolean)
jd_com_handler(j_decompress_ptr cinfo)
{
	return TRUE;
}

static void jd_setswitches(j_decompress_ptr cinfo, int color, int scale, int dct, int dither, BOOL smoothing)
{
	cinfo->err->trace_level = 0;

	if (color) {
		cinfo->desired_number_of_colors = color;
		cinfo->quantize_colors = TRUE;
	}

	// IDCTアルゴリズムの設定
	//     JC_ISLOW / JC_IFAST / JC_FLOAT
	cinfo->dct_method = (J_DCT_METHOD)dct;

	// ディザリングの設定
	//     JD_FS / JD_NONE / JD_ORDERED
	cinfo->dither_mode = (J_DITHER_MODE)dither;

	// スムージング
	cinfo->do_fancy_upsampling = smoothing;

	// 出力スケールの設定
	cinfo->scale_num = 1;
	cinfo->scale_denom = scale;
	if (scale > 1)
		cinfo->two_pass_quantize = FALSE;
}

extern "C"
HDIB WINAPI imgJpgLoadStreamParams(IStream* pstrm, int colors, int scale, int dct, int dither, BOOL smoothing)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	djpeg_dest_ptr dest_mgr = NULL;
	JDIMENSION num_scanlines;
	HDIB hdib = NULL;

	// Initialize the JPEG decompression object with default error handling.
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// Add some application-specific error messages (from cderror.h)
	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;

	// Insert custom COM marker processor.
	jpeg_set_marker_processor(&cinfo, JPEG_COM, jd_com_handler);

	// 展開のための各種スイッチの設定
	jd_setswitches(&cinfo, colors, scale, dct, dither, smoothing);

	// 展開データソースを設定
	strm_jpeg_stdio_src(&cinfo, pstrm);

	// ファイルヘッダの読み込み
	jpeg_read_header(&cinfo, TRUE);

	// 展開のための各種スイッチの再設定
	jd_setswitches(&cinfo, colors, scale, dct, dither, smoothing);

	// 出力モジュールの初期化
	dest_mgr = strm_jinit_write_image(&cinfo);
	if (dest_mgr != NULL) {
		// 展開開始
		jpeg_start_decompress(&cinfo);

		// Write output file header
		(*dest_mgr->start_output)(&cinfo, dest_mgr);
		if (cinfo.client_data != NULL) {
			// データ処理
			while (cinfo.output_scanline < cinfo.output_height) {
				num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer, dest_mgr->buffer_height);
				if (num_scanlines < 1)
					break;
				(*dest_mgr->put_pixel_rows)(&cinfo, dest_mgr, num_scanlines);
			}
		}

		// 展開終了
		(*dest_mgr->finish_output)(&cinfo, dest_mgr);
		jpeg_finish_decompress(&cinfo);

		CDibHandle* dib = (CDibHandle*)cinfo.client_data;
		if (dib) {
			hdib = dib->Detach();
			delete dib;
		}
	}

	jpeg_destroy_decompress(&cinfo);
	return hdib;
}

extern "C"
HDIB WINAPI imgJpgLoadStream(IStream* pstrm, int colors, int scale)
{
	return imgJpgLoadStreamParams(pstrm, colors, scale, JC_ISLOW, JD_NONE, FALSE);
}

#include "stdafx.h"
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

EXTERN(void) jpeg_mf_stdio_src JPP((j_decompress_ptr cinfo, MemStream* infile, int orientation));
EXTERN(void) jpeg_mf_stdio_dest JPP((j_compress_ptr cinfo, MemStream* outfile));


////////////////////////////////////////////////////////////////////////
// Create the add-on message string table.

#define JMESSAGE(code,string)	string ,

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

void je_setswitches(j_compress_ptr cinfo, JPEG_CPARAMS* pjc)
{
	int q_scale_factor;					// scaling percentage for -qtables
	boolean force_baseline;
	jpeg_component_info * compptr;

	// Set up default JPEG parameters.
	// Note that default -quality level need not, and does not,
	// match the default scaling for an explicit -qtables argument.
	q_scale_factor = 100;				// default to no scaling for -qtables
	force_baseline = FALSE;				// by default, allow 16-bit quantizers
	cinfo->err->trace_level = 0;

	// Scan command line options, adjust parameters
	// 算術コーディング
//@	cinfo->arith_code = TRUE;

	// ベースライン出力
	force_baseline = (pjc->baseline != 0);

	// DCTアルゴリズム ... JC_ISLOW / JC_IFAST / JC_FLOAT
	cinfo->dct_method = (J_DCT_METHOD)pjc->dct;

	// グレースケール出力
	if (pjc->grayscale)
		jpeg_set_colorspace(cinfo, JCS_GRAYSCALE);

	// エントロピーオプティマイズ
//@	cinfo->optimize_coding = TRUE;

	// クオリティファクタ
	q_scale_factor = jpeg_quality_scaling(pjc->quality);

	// スムージングファクタ
	cinfo->smoothing_factor = pjc->smooth;

	// プログレッシブ
	BOOL simple_progressive = pjc->progressive;

	// 選択された品質のための減色テーブルを設定
	// Some or all may be overridden if -qtables is present.
	jpeg_set_quality(cinfo, pjc->quality, force_baseline);

	if (simple_progressive)
		jpeg_simple_progression(cinfo);

	// サンプリングファクタ ※ 重要 ※
	compptr = &cinfo->comp_info[0];
	compptr->h_samp_factor = 1;
	compptr->v_samp_factor = 1;
}

extern "C"
HGLOBAL WINAPI imgJpgSaveMemParams(HDIB hdib, JPEG_CPARAMS* pjc, DWORD* pdwSize)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cjpeg_source_ptr src_mgr;
	JDIMENSION num_scanlines;
	BOOL bOK = FALSE;
	HGLOBAL hmem = NULL;

	mif* image = new mif();
	if (!image->attachdib(hdib)) {
		delete image;
		return NULL;
	}

	// JPEG圧縮オブジェクトの初期化
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	// Add some application-specific error messages (from cderror.h)
	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;

	cinfo.in_color_space = JCS_RGB; 			// JCS_RGBと（勝手に）解釈
	jpeg_set_defaults(&cinfo);

	// パラメタ設定
	cinfo.client_data = image;
	je_setswitches(&cinfo, pjc);

	// 出力ファイルを開く
	int packets = image->m_packets * 3;
	MemStream* pms = _mfcreate(packets);
	if (!pms)
		goto exit_jpg;

	// Figure out the input file format, and set up to read it.
	src_mgr = jinit_read_image(&cinfo);

	// Read the input file header to obtain file size & colorspace.
	(*src_mgr->start_input)(&cinfo, src_mgr);

	// Now that we know input colorspace, fix colorspace-dependent defaults
	jpeg_default_colorspace(&cinfo);

	// パラメタ再設定
	je_setswitches(&cinfo, pjc);

	// Specify data destination for compression
	jpeg_mf_stdio_dest(&cinfo, pms);

	// 圧縮開始
	jpeg_start_compress(&cinfo, TRUE);

	// データ処理 
	while (cinfo.next_scanline < cinfo.image_height) {
		num_scanlines = (*src_mgr->get_pixel_rows)(&cinfo, src_mgr);
		jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
	}

	// 圧縮の終了
	(*src_mgr->finish_input)(&cinfo, src_mgr);
	jpeg_finish_compress(&cinfo);
	size_t size;
	hmem = _mfdetach(pms, &size);
	*pdwSize = (DWORD)size;
	bOK = TRUE;

exit_jpg:
	jpeg_destroy_compress(&cinfo);
	delete image;
	return hmem;
}

extern "C"
HGLOBAL WINAPI imgJpgSaveMemParamsSt(HDIB hdib, JPEG_CPARAMS * pjc, DWORD* pdwSize)
{
	/* 互換性保持の為 */
	return imgJpgSaveMemParams(hdib, pjc, pdwSize);
}

extern "C"
HGLOBAL WINAPI imgJpgSaveMem(HDIB hdib, int quality, int dct, int smoothing, DWORD* psize)
{
	JPEG_CPARAMS jc;
	jc.quality = quality;
	jc.dct = dct;
	jc.smooth = smoothing;
	jc.progressive = FALSE;
	jc.baseline = FALSE;
	jc.grayscale = FALSE;
	return imgJpgSaveMemParams(hdib, &jc, psize);
}

extern "C"
BOOL WINAPI imgJpgSaveParams(LPCTSTR fname, HDIB hdib, JPEG_CPARAMS* pjc)
{
	DWORD size;
	HGLOBAL hmem = imgJpgSaveMemParams(hdib, pjc, &size);
	return SaveMemToFileAndWep(fname, hmem, size);
}

extern "C"
BOOL WINAPI imgJpgSave(LPCTSTR fname, HDIB hdib, int quality, int dct, int smoothing)
{
	JPEG_CPARAMS jc;
	jc.quality = quality;
	jc.dct = dct;
	jc.smooth = smoothing;
	jc.progressive = FALSE;
	jc.baseline = FALSE;
	jc.grayscale = FALSE;
	return imgJpgSaveParams(fname, hdib, &jc);
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
	cinfo->do_fancy_upsampling = (smoothing != 0);

	// 出力スケールの設定
	cinfo->scale_num = 1;
	cinfo->scale_denom = scale;
	if (scale > 1)
		cinfo->two_pass_quantize = FALSE;
}

#define INTEL(a)	((a)==0x4949)

static int get_orientation(MemStream* pmf)
{
	long base = pmf->tellp();
	short order = _mfgetw(pmf);
	_mfgetw(pmf);			// Should be 42 for standard TIFF

	while (!pmf->eof()) {
		long doff = INTEL(order) ? _mfgetl(pmf) : _mfgetl_be(pmf);
		if (!doff) break;

		_mfseek(pmf, doff + base, SEEK_SET);

		int entries = INTEL(order) ? _mfgetw(pmf) : _mfgetw_be(pmf);
		if (entries > 512) break;

		while (entries--) {
			if (pmf->eof()) break;

			long save = pmf->tellp();
			int tag = INTEL(order) ? _mfgetw(pmf) : _mfgetw_be(pmf);
			if (tag == 0x112) {
				_mfgetw(pmf);			// type (is USHORT)
				_mfgetl(pmf);			// count (is 1)
				int orientation = INTEL(order) ? _mfgetw(pmf) : _mfgetw_be(pmf);
				return orientation;
			}
			_mfseek(pmf, save + 12, SEEK_SET);
		}
	}
	return ORI_NONE;
}

static int jpeg_get_exif_orientation(MemStream* pms)
{
	int orientation = ORI_NONE;
	_mfseek(pms, 2, SEEK_SET);
	BYTE hdr[32];
	while (_mfread(pms, hdr, 4) == 4) {
		if (hdr[0] != 0xff || hdr[1] < 0xd0 || hdr[1] == 0xd9)
			break;

		if (hdr[1] == 0xe1) {
			if (_mfread(pms, hdr, 6) != 6)
				break;

			if (!memcmp(hdr, "Exif\0\0", 6)) {
				orientation = get_orientation(pms);
				break;
			}
		}
		_mfseek(pms, (hdr[2] << 8) + hdr[3] - 2, SEEK_CUR);
	}
	return orientation;
}

static HDIB load_jpeg(MemStream* pms, int colors, int scale, int dct, int dither, BOOL smoothing)
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
	jd_setswitches(&cinfo, colors, scale, dct, dither, (smoothing != 0));

	// JPEGファイル検査
	if (_mfgetw_ns(pms) != 0xD8FF) {
		// マックバイナリ検査
		_mfseek(pms, 128, FILE_BEGIN);
		if (_mfgetw_ns(pms) != 0xD8FF) {
			_mfclose(pms);
			goto exit_jpg;
		}
	}

	// 画像の向きを取得(ver7)
	int orientation = ORI_NONE;
	if (isVer7()) {
		long pos = pms->tellp();
		orientation = jpeg_get_exif_orientation(pms);
		_mfseek(pms, pos, SEEK_SET);
	}

	// 展開データソースを設定
	jpeg_mf_stdio_src(&cinfo, pms, orientation);

	// ファイルヘッダの読み込み
	jpeg_read_header(&cinfo, TRUE);

	// 展開のための各種スイッチの再設定
	jd_setswitches(&cinfo, colors, scale, dct, dither, smoothing);

	// 出力モジュールの初期化
	dest_mgr = jinit_write_image(&cinfo);
	if (dest_mgr == NULL)
		goto exit_jpg;

	// 展開開始
	jpeg_start_decompress(&cinfo);

	// Write output file header
	(*dest_mgr->start_output)(&cinfo, dest_mgr);
	if (!cinfo.client_data)
		goto exit_jpg;

	// データ処理
	while (cinfo.output_scanline < cinfo.output_height) {
		num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer, dest_mgr->buffer_height);
		if (num_scanlines < 1)
			break;
		(*dest_mgr->put_pixel_rows)(&cinfo, dest_mgr, num_scanlines);
	}

	// 展開終了
	(*dest_mgr->finish_output)(&cinfo, dest_mgr);
	jpeg_finish_decompress(&cinfo);
	CDibHandle* dib = (CDibHandle*)cinfo.client_data;
	if (dib) {
		hdib = dib->Detach();
		delete dib;
	}

exit_jpg:
	jpeg_destroy_decompress(&cinfo);
	return hdib;
}

extern "C"
HDIB WINAPI imgJpgLoadMemParams(HGLOBAL hmem, int colors, int scale, int dct, int dither, BOOL smoothing)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(hmem);
	if (pms) {
		hdib = load_jpeg(pms, colors, scale, dct, dither, smoothing);
		_mfclose(pms);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgJpgLoadMem(HGLOBAL hmem, int colors, int scale)
{
	return imgJpgLoadMemParams(hmem, colors, scale, JC_ISLOW, JD_NONE, FALSE);
}

extern "C"
HDIB WINAPI imgJpgLoadParams(LPCTSTR fname, int colors, int scale, int dct, int dither, int smoothing)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(fname);
	if (pms) {
		hdib = load_jpeg(pms, colors, scale, dct, dither, smoothing);
		_mfclose(pms);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgJpgLoad(LPCTSTR pszFile, int colors, int scale)
{
	return imgJpgLoadParams(pszFile, colors, scale, JC_ISLOW, JD_FS, FALSE);
}

static BOOL jpg_getsize(MemStream* pms, int* width, int* height)
{
	if (!width || !height)
		return FALSE;

	BYTE buf[16];
	long pos = _mfseek(pms, 2, FILE_BEGIN);
	while (1) {
		// マーカーを取得
		if (_mfread(pms, buf, 4) < 4)
			break;

		// マーカーではない？
		if (buf[0] != 0xFF)
			break;

		// ハフマン式コードフレーム？
		if (0xC0 <= buf[1] && buf[1] <= 0xC3) {
			_mfread(pms, buf, 5);
			*height = (int)MAKEWORD(buf[2], buf[1]);
			*width = (int)MAKEWORD(buf[4], buf[3]);
			return TRUE;
		}

		long posw = _mfseek(pms, (long)MAKEWORD(buf[3], buf[2]) - 2, FILE_CURRENT);
		if (pos == posw || pms->eof())
			break;
		pos = posw;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI imgJpgGetImageSize(LPCTSTR fname, int* width, int* height, LPSTR pszCode)
{
	MemStream* pms = _mfcreate(fname);
	if (pms) {
		BOOL ok = jpg_getsize(pms, width, height);
		delete pms;
		return ok;
	}
	return FALSE;
}

extern "C"
HDIB WINAPI imgJpgLoadScaled(LPCTSTR fname, int xmax, int ymax, int colors, int dct, int dither, int smoothing)
{
	MemStream* pms = _mfcreate(fname);
	if (pms) {
		int x, y;
		if (jpg_getsize(pms, &x, &y)) {
			int scale;
			for (scale = 1; scale <= 8; scale <<= 1) {
				if (x <= xmax && y <= ymax)
					break;
				x >>= 1;
				y >>= 1;
			}
			HGLOBAL hmem = load_jpeg(pms, colors, scale, dct, dither, smoothing);
			_mfclose(pms);
			return hmem;
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//

static DWORD _xseek(HANDLE hf, long dist, int origin)
{
	DWORD pos = SetFilePointer(hf, 0, NULL, FILE_CURRENT);
	switch (origin) {
	case SEEK_SET: pos = SetFilePointer(hf, dist, NULL, FILE_BEGIN); break;
	case SEEK_CUR: pos = SetFilePointer(hf, dist, NULL, FILE_CURRENT); break;
	case SEEK_END: pos = SetFilePointer(hf, dist, NULL, FILE_END); break;
	}
	return pos;
}

static DWORD _xtell(HANDLE hf)
{
	return SetFilePointer(hf, 0, NULL, FILE_CURRENT);
}

static BOOL _xeof(HANDLE hf)
{
	DWORD cp = SetFilePointer(hf, 0, NULL, FILE_CURRENT);
	DWORD ep = SetFilePointer(hf, 0, NULL, FILE_END);
	SetFilePointer(hf, cp, NULL, FILE_BEGIN);
	return (ep <= cp);
}

int _xgetc(HANDLE hf)
{
	BYTE cb;
	DWORD len;
	if (ReadFile(hf, &cb, 1, &len, NULL))
		return (int)cb;
	return -1;
}

int _xgetw(HANDLE hf, bool le)
{
	BYTE cb[2];
	DWORD len;
	if (ReadFile(hf, &cb, 2, &len, NULL))
		return (le ? (int)(USHORT)((cb[1] << 8) | cb[0]) : (int)(USHORT)((cb[0] << 8) | cb[1]));
	return -1;
}

ULONG _xgetl(HANDLE hf, bool le)
{
	BYTE cb[4];
	DWORD len;
	if (ReadFile(hf, &cb, 4, &len, NULL))
		return (le ? (ULONG)((cb[3] << 24) | (cb[2] << 16) | (cb[1] << 8) | cb[0]) : (ULONG)((cb[0] << 24) | (cb[1] << 16) | (cb[2] << 8) | cb[3]));
	return (ULONG)-1;
}

static BOOL set_orientation(HANDLE hf, int orientation)
{
	long base = _xtell(hf);
	short order = _xgetw(hf, false);
	_xgetw(hf, false);			// Should be 42 for standard TIFF

	while (!_xeof(hf)) {
		long doff = _xgetl(hf, INTEL(order));
		if (!doff) break;

		_xseek(hf, doff + base, SEEK_SET);

		int entries = _xgetw(hf, INTEL(order));
		if (entries > 512) break;

		while (entries--) {
			if (_xeof(hf)) break;

			long save = _xtell(hf);
			int tag = _xgetw(hf, INTEL(order));
			if (tag == 0x112) {
				_xgetw(hf, true);			// type (is USHORT)
				_xgetl(hf, true);			// count (is 1)

				BYTE cb[2];
				if (INTEL(order)) {
					cb[0] = (BYTE)orientation;
					cb[1] = 0;
				}
				else {
					cb[0] = 0;
					cb[1] = (BYTE)orientation;
				}

				DWORD len;
				return WriteFile(hf, cb, 2, &len, NULL);

			}
			_xseek(hf, save + 12, SEEK_SET);
		}
	}
	return FALSE;
}

static BOOL jpeg_set_exif_orientation(HANDLE hf, int orientation)
{
	BOOL bOK = FALSE;
	_xseek(hf, 2, SEEK_SET);
	BYTE hdr[32];
	DWORD len;
	while (ReadFile(hf, hdr, 4, &len, NULL) && len == 4) {
		if (hdr[0] != 0xff || hdr[1] < 0xd0 || hdr[1] == 0xd9)
			break;

		if (hdr[1] == 0xe1) {
			if (!ReadFile(hf, hdr, 6, &len, NULL) || len != 6)
				break;

			if (!memcmp(hdr, "Exif\0\0", 6)) {
				bOK = set_orientation(hf, orientation);
				break;
			}
		}
		_xseek(hf, (hdr[2] << 8) + hdr[3] - 2, SEEK_CUR);
	}
	return bOK;
}

extern "C"
BOOL WINAPI imgJpgSetOrientation(LPCTSTR pszName, int orientation)
{
	BOOL bOK = FALSE;
	HANDLE hf = CreateFile(pszName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		// ファイル日付を取得
		FILETIME ftLastWrite;
		GetFileTime(hf, NULL, NULL, &ftLastWrite);

		BYTE cb[2];
		DWORD len;
		if (ReadFile(hf, cb, 2, &len, NULL) && len == 2 && cb[0] == 0xff && cb[1] == 0xd8)
			bOK = jpeg_set_exif_orientation(hf, orientation);

		// ファイル日付を元に戻す
		SetFileTime(hf, NULL, NULL, &ftLastWrite);
		CloseHandle(hf);
	}
	return bOK;
}

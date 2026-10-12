/*-
 * webpcode.cpp
 *
 */
#include "stdafx.h"
#include <time.h>
#include "lsimg.h"
#include "common.h"
#include "filemap.h"

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

#include "../../libwebp/src/webp/encode.h"
#include "../../libwebp/src/webp/decode.h"

#ifdef __cplusplus
}
#endif

static mif* decode_webp(uint8_t* encoded, size_t size, int use_upsampler, uint32_t xmax, uint32_t ymax)
{
	// Init a configuration object
	WebPDecoderConfig config;
	WebPInitDecoderConfig(&config);
	// optional: retrieve the bitstream's features.
	if (WebPGetFeatures(encoded, size, &config.input) == VP8_STATUS_OK) {
		config.options.use_threads = 1;


		uint32_t scaled_width = config.input.width,
				 scaled_height = config.input.height;

		if (xmax < scaled_width || ymax < scaled_height) {
			float ratio = CalcScaleRatio(scaled_width, scaled_height, xmax, ymax);
			if (ratio < 1.0f) {
				scaled_width = (int)floor(ratio * config.input.width);
				scaled_height = (int)floor(ratio * config.input.height);
				config.options.use_scaling = 1;
				config.options.scaled_width = scaled_width;
				config.options.scaled_height = scaled_height;
			}
		}
		config.options.no_fancy_upsampling = use_upsampler;

		// Decode
		if (WebPDecode(encoded, size, &config) == VP8_STATUS_OK) {
			bool hasAlpha = (config.output.colorspace == MODE_RGBA ||
				config.output.colorspace == MODE_ARGB ||
				config.output.colorspace == MODE_BGRA);
			// Decoded image is now in config.output (and config.output.u.RGBA)
			mif* pimage = new mif(scaled_width, scaled_height, hasAlpha ? 32 : 24);
			uint8_t* dp = (uint8_t*)config.output.u.RGBA.rgba;
			RGBA* p = pimage->m_pixels;
			for (int n = 0; n < pimage->m_packets; n++) {
				switch (config.output.colorspace) {
				case MODE_RGB:
					p->red = *dp++;		// red
					p->green = *dp++;	// green
					p->blue = *dp++;	// blue
					break;
				case MODE_RGBA:
					p->red = *dp++;		// red
					p->green = *dp++;	// green
					p->blue = *dp++;	// blue
					p->alpha = *dp++;
					break;
				case MODE_ARGB:
					p->alpha = *dp++;
					p->red = *dp++;		// red
					p->green = *dp++;	// green
					p->blue = *dp++;	// blue
					break;
				case MODE_BGR:
					p->blue = *dp++;	// blue
					p->green = *dp++;	// green
					p->red = *dp++;		// red
					break;
				case MODE_BGRA:
					p->blue = *dp++;	// blue
					p->green = *dp++;	// green
					p->red = *dp++;		// red
					p->alpha = *dp++;
					break;
				}
				++p;
			}
			WebPFreeDecBuffer(&config.output);
			return pimage;
		}
	}
	return NULL;
}


extern "C"
HDIB WINAPI imgWebpLoadMem(HANDLE hmem)
{
	HDIB hdib = NULL;
	size_t memsize = (size_t)GlobalSize(hmem);
	uint8_t* memptr = (uint8_t*)GlobalLock(hmem);
	if (memptr) {
		mif* pimage = decode_webp(memptr, memsize, 1, UINT32_MAX, UINT32_MAX);
		if (pimage) {
			hdib = pimage->createdib();
			delete pimage;
		}
		GlobalUnlock(hmem);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgWebpLoad(LPCTSTR pszName)
{
	HDIB hdib = NULL;
	filemap fm;
	if (fm.open(pszName)) {
		size_t memsize = (size_t)fm.filesize();
		uint8_t* memptr = (uint8_t*)fm.m_base;
		mif* pimage = decode_webp(memptr, memsize, 1, UINT32_MAX, UINT32_MAX);
		if (pimage) {
			hdib = pimage->createdib();
			delete pimage;
		}
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgWebpLoadScaled(LPCTSTR pszName, int xmax, int ymax)
{
	HDIB hdib = NULL;
	filemap fm;
	if (fm.open(pszName)) {
		size_t memsize = (size_t)fm.filesize();
		uint8_t* memptr = (uint8_t*)fm.m_base;
		mif* pimage = decode_webp(memptr, memsize, 1, xmax, ymax);
		if (pimage) {
			hdib = pimage->createdib();
			delete pimage;
		}
	}
	return hdib;
}

extern "C"
BOOL WINAPI imgWebpGetImageSize(LPCTSTR pszName, int* width, int* height)
{
	filemap fm;
	if (fm.open(pszName)) {
		WebPBitstreamFeatures info;
		memset(&info, sizeof(info), 0);
		if (WebPGetFeatures((uint8_t*)fm.baseptr(), (size_t)fm.size(), &info) == VP8_STATUS_OK) {
			*width = info.width;
			*height = info.height;
			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	int method;
	int quality;
	bool lossless;
	int preset;
} WEBP_ENCODE_OPTION;


static uint8_t* encode_webp_lossy(mif& image, int method, int quality, WebPPreset preset, size_t* psize)
{
	WebPPicture picture;
	WebPPictureInit(&picture);
	// libwebp がエンコード処理でメモリーへの書き出しに用いる WebPMemoryWriter 型を用意します。
	WebPMemoryWriter writerinfo;
	WebPMemoryWriterInit(&writerinfo);

	WebPConfig config;
	config.lossless = 0;	// lossy に圧縮するので lossless = 0。この値を 1 にするとロスレス圧縮
	WebPConfigPreset(&config, preset, (float)quality);
	// lossy 圧縮では 0 .. 6 までの複数の種類の method （WebP圧縮に用いられる内部アルゴリズム） 0 が高速、 6 が低速
	config.method = method;
	config.thread_level = 1;	// use if available
	// エンコーダーが扱う画像情報を設定します。
	picture.width = image.m_width;
	picture.height = image.m_height;
	// この値はコンフィグによって適切な設定値が変わります。クロッピングなどしている場合にも設定します。
	// これは libwebp に付属の cwebp.c ソースを読むと例が書いてあります。
	picture.use_argb = config.lossless || config.preprocessing > 0;
	// WebPPicture に入力画像を読み取らせます。
	// この API は API Reference よりも libwebp に付属する cwebp.c ソースが参考になります。
	if (image.m_bits == 32) {
		WebPPictureImportRGBA(&picture, (uint8_t*)image.m_pixels, image.m_width * sizeof(RGBA));
	} else {
		image.increasecolor(24);
		RGB_PIXEL* pixels = RGBAtoRGBPixels(image.m_pixels, image.m_packets);
		if (pixels) {
			WebPPictureImportRGB(&picture, (uint8_t*)pixels, image.m_width * sizeof(RGB_PIXEL));
			free(pixels);
		}
	}
	// WebPPicture に書き出し方法として WebPMemoryWrite を設定します。 WebPMemoryWriter ではないので地味にタイポ等に注意します。
	picture.writer = WebPMemoryWrite;
	// WebPPicture に書き出しを行う　WebPMemoryWriter オブジェクトを設定します。
	picture.custom_ptr = &writerinfo;

	// ここまでに用意した設定と画像情報を用いてデータをエンコードします。
	// 成功した場合には writerinfo.mem に WebPFree が必要となるので用意しておいた unique_writer_memory へ入れておきます。
	uint8_t* encoded = NULL;
	if (WebPEncode(&config, &picture)) {
		encoded = (uint8_t*)WebPMalloc(writerinfo.max_size);
		memcpy(encoded, writerinfo.mem, writerinfo.size);
		*psize = writerinfo.size;
	}
	WebPMemoryWriterClear(&writerinfo);
	WebPPictureFree(&picture);
	return encoded;
}

extern "C"
HANDLE WINAPI imgWebpSaveMemParams(HDIB hdib, int method, int quality, WEBP_ENCODE_PRESET preset, DWORD* pdwSize)
{
	HANDLE hmem = NULL;
	mif image;
	if (image.attachdib(hdib)) {
		size_t size;
		uint8_t* encoded = encode_webp_lossy(image, method, quality, (WebPPreset)preset, &size);
		if (encoded) {
			hmem = GlobalAlloc(GMEM_MOVEABLE, size);
			memcpy((void*)GlobalLock(hmem), encoded, size);
			GlobalUnlock(hmem);
			WebPFree(encoded);
			*pdwSize = (DWORD)size;
		}
	}
	return hmem;
}

extern "C"
HANDLE WINAPI imgWebpSaveMem(HDIB hdib, int quality, DWORD* pdwSize)
{
	return imgWebpSaveMemParams(hdib, 3, quality, WEC_PRESET_DEFAULT, pdwSize);
}

extern "C"
BOOL WINAPI imgWebpSaveParams(LPCTSTR pszName, HDIB hdib, int method, int quality, WEBP_ENCODE_PRESET preset)
{
	BOOL ok = FALSE;
	mif image;
	if (image.attachdib(hdib)) {
		size_t size;
		uint8_t* encoded = encode_webp_lossy(image, method, quality, (WebPPreset)preset, &size);
		if (encoded) {
			ok = SaveMemoryToFile(pszName, encoded, size);
			WebPFree(encoded);
		}
	}
	return ok;
}

extern "C"
BOOL WINAPI imgWebpSave(LPCTSTR pszName, HDIB hdib, int quality)
{
	return imgWebpSaveParams(pszName, hdib, 3, quality, WEC_PRESET_DEFAULT);
}


/*-
 * avifcode.cpp
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

#include "../../libavif/include/avif/avif.h"

#ifdef __cplusplus
}
#endif

#define STD_ENCODE_EFFORT_SPEED		4



static HDIB avif_load(avifDecoder* decoder, uint32_t xmax, uint32_t ymax)
{
	HDIB hdib = NULL;

	int jobs = 1;
	avifChromaUpsampling chromaUpsampling = AVIF_CHROMA_UPSAMPLING_AUTOMATIC;
	// AVIF_CHROMA_UPSAMPLING_AUTOMATIC,
	// AVIF_CHROMA_UPSAMPLING_FASTEST,
	// AVIF_CHROMA_UPSAMPLING_BEST_QUALITY,
	// AVIF_CHROMA_UPSAMPLING_NEAREST,
	// AVIF_CHROMA_UPSAMPLING_BILINEAR

	avifCodecChoice codecChoice = AVIF_CODEC_CHOICE_AUTO;
	avifStrictFlags strictFlags = AVIF_STRICT_ENABLED;
	avifBool allowProgressive = AVIF_FALSE;
	uint32_t imageSizeLimit = AVIF_DEFAULT_IMAGE_SIZE_LIMIT;
	uint32_t imageDimensionLimit = AVIF_DEFAULT_IMAGE_DIMENSION_LIMIT;
	uint32_t frameIndex = 0;

	decoder->maxThreads = jobs;
	decoder->codecChoice = codecChoice;
	decoder->imageSizeLimit = imageSizeLimit;
	decoder->imageDimensionLimit = imageDimensionLimit;
	decoder->strictFlags = strictFlags;
	decoder->allowProgressive = allowProgressive;

	if (avifDecoderParse(decoder) == AVIF_RESULT_OK &&
		avifDecoderNthImage(decoder, frameIndex) == AVIF_RESULT_OK) {

		avifImage* avif = decoder->image;
		// Scales the YUV/A planes in-place. dstWidth and dstHeight must both be <= AVIF_DEFAULT_IMAGE_DIMENSION_LIMIT and
		// dstWidth*dstHeight should be <= AVIF_DEFAULT_IMAGE_SIZE_LIMIT.
		if (xmax < avif->width || ymax < avif->height) {
			float scale = CalcScaleRatio(avif->width, avif->height, xmax, ymax);
			if (scale < 1.0f) {
				avifDiagnostics diag;
				avifImageScale(avif, (uint32_t)(avif->width * scale), (uint32_t)(avif->height * scale), &diag);
			}
		}
		bool hasAlpha = avif->alphaPlane != NULL;

		avifRGBImage rgb;
		memset(&rgb, 0, sizeof(rgb));

		avifRGBImageSetDefaults(&rgb, avif);
		rgb.chromaUpsampling = chromaUpsampling;
		rgb.depth = 8;
		rgb.format = hasAlpha ? AVIF_RGB_FORMAT_BGRA : AVIF_RGB_FORMAT_BGR;

		// Override YUV(A)->RGB(A) defaults here: depth, format, chromaUpsampling, ignoreAlpha, alphaPremultiplied, libYUVUsage, etc
		// Alternative: set rgb.pixels and rgb.rowBytes yourself, which should match your chosen rgb.format
		// Be sure to use uint16_t* instead of uint8_t* for rgb.pixels/rgb.rowBytes if (rgb.depth > 8)
		avifRGBImageAllocatePixels(&rgb);
		if (avifImageYUVToRGB(avif, &rgb) == AVIF_RESULT_OK) {
			mif* pimage = new mif(avif->width, avif->height, hasAlpha ? 32 : 24);
			RGBA* p = pimage->m_pixels;
			for (uint32_t y = 0; y < avif->height; ++y) {
				uint8_t* sp = rgb.pixels + rgb.rowBytes * y;
				uint32_t pixels = avif->width;
				while (pixels--) {
					p->blue = *sp++;
					p->green = *sp++;
					p->red = *sp++;
					if (hasAlpha)
						p->alpha = *sp++;
					p++;
				}
			}
			hdib = pimage->createdib();
			delete pimage;
		}
		avifRGBImageFreePixels(&rgb);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgAvifLoadMem(HANDLE hmem)
{
	avifDecoder* decoder = avifDecoderCreate();
	avifDecoderSetIOMemory(decoder, (uint8_t*)GlobalLock(hmem), GlobalSize(hmem));
	HDIB hdib = avif_load(decoder, UINT32_MAX, UINT32_MAX);
	GlobalUnlock(hmem);
	avifDecoderDestroy(decoder);
	return hdib;
}

static avifDecoder* decoder_from_file(LPCTSTR pszName)
{
	avifDecoder* decoder = avifDecoderCreate();
	char fname[MAX_PATH * 2];
	WideCharToMultiByte(CP_OEMCP, 0, pszName, -1, fname, ARRAYSIZE(fname), NULL, NULL);
	avifDecoderSetIOFile(decoder, fname);
	return decoder;
}

extern "C"
HDIB WINAPI imgAvifLoad(LPCTSTR pszName)
{
	avifDecoder* decoder = decoder_from_file(pszName);
	HDIB hdib = avif_load(decoder, UINT32_MAX, UINT32_MAX);
	avifDecoderDestroy(decoder);
	return hdib;
}

extern "C"
HDIB WINAPI imgAvifLoadScaled(LPCTSTR pszName, int xmax, int ymax)
{
	avifDecoder* decoder = decoder_from_file(pszName);
	HDIB hdib = avif_load(decoder, xmax, ymax);
	avifDecoderDestroy(decoder);
	return hdib;
}

extern "C"
BOOL WINAPI imgAvifGetImageSize(LPCTSTR pszName, int* width, int* height)
{
	BOOL ok = FALSE;
	avifDecoder* decoder = decoder_from_file(pszName);
	if (avifDecoderParse(decoder) == AVIF_RESULT_OK) {
		*width = decoder->image->width;
		*height = decoder->image->height;
		ok = TRUE;
	}
	avifDecoderDestroy(decoder);
	return ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	// AVIF_QUALITY_DEFAULT -1
	// AVIF_QUALITY_LOSSLESS 100
	// AVIF_QUALITY_WORST 0
	// AVIF_QUALITY_BEST 100
	int quality;
	// AVIF_SPEED_DEFAULT -1
	// AVIF_SPEED_SLOWEST 0
	// AVIF_SPEED_FASTEST 10
	int speed;
} AVIF_ENCODE_OPTION;


static uint8_t* avif_save(mif& image, const AVIF_ENCODE_OPTION& op, uint32_t* pmemsize)
{
	*pmemsize = 0;
	uint8_t* memptr = NULL;
	uint32_t width = image.width();
	uint32_t height = image.height();
	avifImage* ai = avifImageCreate(width, height, 8, AVIF_PIXEL_FORMAT_YUV444);
	avifRGBImage rgb;
	memset(&rgb, 0, sizeof(rgb));
	avifRGBImageSetDefaults(&rgb, ai);
	rgb.depth = 8;
	rgb.format = AVIF_RGB_FORMAT_RGB;
	if (avifRGBImageAllocatePixels(&rgb) == AVIF_RESULT_OK) {
		RGBA* p = image.m_pixels;
		for (uint32_t y = 0; y < height; y++) {
			uint8_t* dp = rgb.pixels + y * rgb.rowBytes;
			for (uint32_t x = 0; x < width; x++) {
				*dp++ = p->red;
				*dp++ = p->green;
				*dp++ = p->blue;
				p++;
			}
		}
		if (avifImageRGBToYUV(ai, &rgb) == AVIF_RESULT_OK) {
			avifEncoder* encoder = avifEncoderCreate();
			// Configure your encoder here (see avif/avif.h):
			// * maxThreads
			// * quality
			// * qualityAlpha
			// * tileRowsLog2
			// * tileColsLog2
			// * speed
			// * keyframeInterval
			// * timescale
			encoder->maxThreads = 2;
			encoder->quality = op.quality;
			encoder->qualityAlpha = op.quality;
			encoder->speed = op.speed;
			encoder->autoTiling = TRUE;

			// Call avifEncoderAddImage() for each image in your sequence
			// Only set AVIF_ADD_IMAGE_FLAG_SINGLE if you're not encoding a sequence
			// Use avifEncoderAddImageGrid() instead with an array of avifImage* to make a grid image
			if (avifEncoderAddImage(encoder, ai, 1, AVIF_ADD_IMAGE_FLAG_SINGLE) == AVIF_RESULT_OK) {
				avifRWData avifOutput = AVIF_DATA_EMPTY;
				if (avifEncoderFinish(encoder, &avifOutput) == AVIF_RESULT_OK) {
					*pmemsize = (uint32_t)avifOutput.size;
					memptr = (uint8_t*)malloc(avifOutput.size);
					memcpy(memptr, avifOutput.data, avifOutput.size);
				}
				avifRWDataFree(&avifOutput);
			}
			avifEncoderDestroy(encoder);
		}
	}
	avifImageDestroy(ai);
	return memptr;
}


extern "C"
HANDLE WINAPI imgAvifSaveMemParams(HDIB hdib, int quality, int speed, DWORD* pdwSize)
{
	HANDLE hmem = NULL;
	mif image;
	if (image.attachdib(hdib)) {
		AVIF_ENCODE_OPTION op;
		op.quality = quality;
		op.speed = speed;
		uint32_t memsize;
		uint8_t* memptr = avif_save(image, op, &memsize);
		if (memptr) {
			hmem = GlobalAlloc(GMEM_MOVEABLE, memsize);
			if (hmem) {
				void* ptr = GlobalLock(hmem);
				if (ptr) {
					memcpy(ptr, memptr, memsize);
					GlobalUnlock(hmem);
					if (pdwSize) {
						*pdwSize = memsize;
					}
				}
				free(memptr);
			}
		}
	}
	return hmem;
}

extern "C"
HANDLE WINAPI imgAvifSaveMem(HDIB hdib, int quality, DWORD* pdwSize)
{
	return imgAvifSaveMemParams(hdib, quality, STD_ENCODE_EFFORT_SPEED, pdwSize);
}

extern "C"
BOOL WINAPI imgAvifSaveParams(LPCTSTR pszName, HDIB hdib, int quality, int speed)
{
	BOOL ok = FALSE;
	mif image;
	if (image.attachdib(hdib)) {
		uint32_t memsize;
		AVIF_ENCODE_OPTION op;
		op.quality = quality;
		op.speed = speed;
		uint8_t* memptr = avif_save(image, op, &memsize);
		if (memptr) {
			ok = SaveMemoryToFile(pszName, memptr, memsize);
			free(memptr);
		}
	}
	return ok;
}

extern "C"
BOOL WINAPI imgAvifSave(LPCTSTR pszName, HDIB hdib, int quality)
{
	return imgAvifSaveParams(pszName, hdib, quality, STD_ENCODE_EFFORT_SPEED);
}


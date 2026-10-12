//////////////////////////////////////////////////////////////////////
// fpxcode.cpp

#include "stdafx.h"
#include "lsimg.h"

#include "..\gnu_libs\libfpx-1.2.0.13\fpx.h"

class waitcursor
{
public:
	waitcursor() { m_hcur = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	~waitcursor() { SetCursor(m_hcur); }
	HCURSOR m_hcur;
};


static HDIB GetThumbnail(FPXImageHandle* flashpix)
{
	FPXSummaryInformation si;
	if (FPX_GetSummaryInformation(flashpix, &si) != FPX_OK)
		return FALSE;

	HDIB hdib = NULL;
	if (si.thumbnail_valid && si.thumbnail.cbSize && si.thumbnail.pClipData) {
		ULONG size = si.thumbnail.cbSize - sizeof(long) * 2;
		BITMAPINFO* pbi = (BITMAPINFO*)(si.thumbnail.pClipData + sizeof(long) * 2);
		hdib = GlobalAlloc(GMEM_MOVEABLE, size);
		if (hdib) {
			BYTE* pd = (BYTE*)GlobalLock(hdib);
			memcpy(pd, pbi, size);
			GlobalUnlock(hdib);
		}
	}
	return hdib;
}

static BOOL GetSummaryInfo(FPXImageHandle* flashpix, char** label, char** comment)
{
	FPXSummaryInformation si;
	if (FPX_GetSummaryInformation(flashpix, &si) != FPX_OK)
		return FALSE;

	*label = NULL;
	*comment = NULL;
	if (si.title_valid && si.title.length && si.title.ptr) {
		// Note image label.
		*label = (char*)malloc(si.title.length + 1);
		if (*label) {
			strncpy(*label, (char*)si.title.ptr, si.title.length);
			(*label)[si.title.length] = '\0';
		}
	}

	if (si.comments_valid && si.comments.length && si.comments.ptr) {
		// Note image comment.
		*comment = (char*)malloc(si.comments.length + 1);
		if (*comment) {
			strncpy(*comment, (char*)si.comments.ptr, si.comments.length);
			(*comment)[si.comments.length] = '\0';
		}
	}
	return TRUE;
}

extern "C"
HDIB WINAPI imgFpxLoadThumbnail(LPCTSTR pszName, SIZE* frame)
{
	if (!pszName || *pszName == '\0')
		return NULL;

	char sbname[MAX_PATH * 2];
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, pszName, -1, sbname, _COUNTOF(sbname), NULL, NULL);
#else
	lstrcpy(sbname, pszName);
#endif

	// FPXツールキットの初期化
	ULONG memory_limit = 20000000;
	FPXStatus fstat = FPX_SetToolkitMemoryLimit(&memory_limit);
	if (fstat != FPX_OK)
		return NULL;

	ULONG tile_width = 64;
	ULONG tile_height = 64;
	FPXImageHandle *flashpix = NULL;
	FPXColorspace colorspace;
	fstat = FPX_OpenImageByFilename(sbname,
							NULL, (ULONG*)&frame->cx, (ULONG*)&frame->cy,
							&tile_width, &tile_height, &colorspace, &flashpix);
	HDIB hdib = NULL;
	if (fstat == FPX_OK)
		hdib = GetThumbnail(flashpix);
	FPX_CloseImage(flashpix);
	FPX_ClearSystem();
	return hdib;
}

extern "C"
HDIB WINAPI imgFpxLoadEx(LPCTSTR pszName, UINT target_subimage, SIZE* frame)
{
	register int i, x;
	register RGBA *q;
	register BYTE *a, *b, *g, *r;

	if (!pszName || *pszName == '\0')
		return NULL;

	char sbname[MAX_PATH * 2];
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, pszName, -1, sbname, _COUNTOF(sbname), NULL, NULL);
#else
	lstrcpy(sbname, pszName);
#endif

	// FPXツールキットの初期化
	ULONG memory_limit = 20000000;
	FPXStatus fstat = FPX_SetToolkitMemoryLimit(&memory_limit);
	if (fstat != FPX_OK)
		return NULL;

	ULONG tile_width = 64;
	ULONG tile_height = 64;
	ULONG height, width, org_height, org_width;
	FPXImageHandle *flashpix = NULL;
	FPXColorspace colorspace;
	fstat = FPX_OpenImageByFilename(sbname,
							NULL, &org_width, &org_height, &tile_width,
							&tile_height, &colorspace, &flashpix);
	if (fstat != FPX_OK)
	{
		FPX_ClearSystem();
		return NULL;
	}

	width = org_width;
	height = org_height;

	// Get the aspect ratio.
	float aspect_ratio = (float)width / height;
	if (FPX_GetImageResultAspectRatio(flashpix, &aspect_ratio) != FPX_OK) {
		FPX_ClearSystem();
		return NULL;
	}

	if (width != (ULONG)((aspect_ratio * height) + 0.5)) {
		// 縦横のスワップ
		ULONG t = width;
		width = height;
		height = t;
    }

	// サブイメージの仕様によって解像度を決定する
	for (i = 1; ; i++)
		if (((width >> i) < tile_width) || ((height >> i) < tile_height))
	 		break;

	UINT subimage = i;
	if (target_subimage != (UINT)-1)
		while (subimage > target_subimage) {
			width >>= 1;
			height >>= 1;
			subimage--;
		}

	if (frame != NULL) {
		while ((width > (ULONG)frame->cx) || (height > (ULONG)frame->cy))
		{
			width >>= 1;
			height >>= 1;
			subimage--;
		}

		// オリジナルサイズを呼び出し元に返す
		frame->cx = org_width;
		frame->cy = org_height;
	}


	BOOL matte = FALSE;
	if ((colorspace.numberOfComponents % 2) == 0)
		matte = TRUE;

	int bits = 24;
	if (colorspace.numberOfComponents == 1)
		bits = 8;

	mif image;
	if (!image.create(width, height, bits)) {
		FPX_CloseImage(flashpix);
		FPX_ClearSystem();
		return NULL;
	}
	if (colorspace.numberOfComponents == 1) {
		// グレースケールカラーマップ
		RGBA* pc = image.m_map;
		for (i = 0; i < 256; i++) {
			pc->red = pc->green = pc->blue = i;
			pc++;
		}
	}
	waitcursor wc;

	// Allocate memory for the image and pixel buffer.
	BYTE *scanline = (BYTE*)malloc(colorspace.numberOfComponents * image.m_width * (tile_height + 1));
	if (!scanline) {
		FPX_CloseImage(flashpix);
		FPX_ClearSystem();
		return NULL;
	}

	// Initialize FlashPix image description.
	FPXImageDesc desc;
	desc.numberOfComponents = colorspace.numberOfComponents;
	for (i = 0; i < 4; i++) {
		desc.components[i].myColorType.myDataType = DATA_TYPE_UNSIGNED_BYTE;
		desc.components[i].horzSubSampFactor = 1;
		desc.components[i].vertSubSampFactor = 1;
		desc.components[i].columnStride = desc.numberOfComponents;
		desc.components[i].lineStride = image.m_width * desc.components[i].columnStride;
		desc.components[i].theData = scanline + i;
	}
	desc.components[0].myColorType.myColor = desc.numberOfComponents > 2 ? NIFRGB_R : MONOCHROME;

	FPXImageComponentDesc* red_compo = &desc.components[0];
	desc.components[1].myColorType.myColor = desc.numberOfComponents > 2 ? NIFRGB_G : ALPHA;

	FPXImageComponentDesc* green_compo = &desc.components[1];
	desc.components[2].myColorType.myColor=NIFRGB_B;

	FPXImageComponentDesc* blue_compo = &desc.components[2];
	desc.components[3].myColorType.myColor = ALPHA;

	FPXImageComponentDesc* alpha_compo = &desc.components[desc.numberOfComponents - 1];
	FPX_SetResampleMethod(FPX_LINEAR_INTERPOLATION);

	// Initialize image pixels.
	for (int y = 0; y < image.m_height; y++) {
		q = image.pos(0, y);
		if ((y % tile_height) == 0) {
			// Read FPX image tile (with or without viewing affine)..
/* @@
			if (image_info->view != NULL)
				fstat = FPX_ReadImageRectangle(flashpix, 0,
										y, image->width,
										y + tile_height - 1, subimage, &desc);
			else
@@*/
				fstat = FPX_ReadImageTransformRectangle(flashpix, 0.0F,
									(float)y / image.m_height,
									(float)image.m_width / image.m_height,
									(float)(y + tile_height - 1) / image.m_height,
									(long)image.m_width, (long)tile_height,
									&desc);
			if (fstat == FPX_LOW_MEMORY_ERROR) {
				free(scanline);
				FPX_CloseImage(flashpix);
				FPX_ClearSystem();
				return NULL;
			}
		}

		// Transfer a FPX scanline.
		r = red_compo->theData+(y % tile_height) * red_compo->lineStride;
		g = green_compo->theData+(y % tile_height) * green_compo->lineStride;
		b = blue_compo->theData+(y % tile_height) * blue_compo->lineStride;
		a = alpha_compo->theData+(y % tile_height) * alpha_compo->lineStride;
		for (x = 0; x < image.m_width; x++) {
			if (desc.numberOfComponents > 2) {
				q->red = *r;
				q->green = *g;
				q->blue = *b;
			}
			else {
				q->red = *r;
				q->green = *r;
				q->blue = *r;
			}
			if (matte)
				q->alpha = 255 - *a;
			q++;
			r += red_compo->columnStride;
			g += green_compo->columnStride;
			b += blue_compo->columnStride;
			a += alpha_compo->columnStride;
		}
	}
	free(scanline);
	FPX_CloseImage(flashpix);
	FPX_ClearSystem();

	return image.createdib();
}

extern "C"
HDIB WINAPI imgFpxLoad(LPCTSTR pszName)
{
	return imgFpxLoadEx(pszName, (UINT)-1, NULL);
}

extern "C"
BOOL WINAPI imgFpxSaveEx(LPCTSTR pszName, HDIB hdib, int compress_mode, int quality)
{
	if (!pszName || *pszName == '\0' || !hdib)
		return FALSE;

	char sbname[MAX_PATH * 2];
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, pszName, -1, sbname, _COUNTOF(sbname), NULL, NULL);
#else
	lstrcpy(sbname, pszName);
#endif

	// Initialize FPX toolkit.
	ULONG memory_limit = 20000000;
	FPXStatus fstat = FPX_SetToolkitMemoryLimit(&memory_limit);
	if (fstat != FPX_OK)
		return NULL;

	mif image;
	if (!image.attachdib(hdib))
		return FALSE;

	// 24bitカラーへ変更
	image.m_bits = 24;

	ULONG tile_width = 64;
	ULONG tile_height = 64;

	// RGBカラーコンポーネント
	FPXColorspace colorspace = {
		TRUE, 4,
		NIFRGB_R, DATA_TYPE_UNSIGNED_BYTE,
		NIFRGB_G, DATA_TYPE_UNSIGNED_BYTE,
		NIFRGB_B, DATA_TYPE_UNSIGNED_BYTE,
		ALPHA, DATA_TYPE_UNSIGNED_BYTE
	};
	colorspace.numberOfComponents = 3;

	FPXBackground background_color;
	background_color.color1_value = 0;
	background_color.color2_value = 0;
	background_color.color3_value = 0;
	background_color.color4_value = 0;

	FPXCompressionOption compression = NONE;
	if (compress_mode == FPXCOMPRESS_JPEG_UNSPECIFIED)
		compression = JPEG_UNSPECIFIED;

	FPXImageHandle *flashpix;
	fstat = FPX_CreateImageByFilename(sbname,
					image.m_width, image.m_height, tile_width, tile_height,
					colorspace, background_color, compression, &flashpix);
	if (fstat != FPX_OK) {
		FPX_ClearSystem();
		return NULL;
	}

	if (compress_mode == FPXCOMPRESS_JPEG_UNSPECIFIED) {
		// Initialize the compression by quality for the entire image.
		if (FPX_SetJPEGCompression(flashpix, (USHORT)quality) != FPX_OK)
		{
			FPX_CloseImage(flashpix);
			FPX_ClearSystem();
			return NULL;
		}
	}

	// Allocate pixels.
	BYTE *pixels = (BYTE*)malloc(colorspace.numberOfComponents * image.m_width);
	if (!pixels) {
		FPX_CloseImage(flashpix);
		FPX_ClearSystem();
		return NULL;
	}

	// Initialize FlashPix image description.
	FPXImageDesc desc;
	desc.numberOfComponents = colorspace.numberOfComponents;
	for (int i = 0; i < desc.numberOfComponents; i++) {
		desc.components[i].myColorType.myDataType = DATA_TYPE_UNSIGNED_BYTE;
		desc.components[i].horzSubSampFactor = 1;
		desc.components[i].vertSubSampFactor = 1;
		desc.components[i].columnStride = desc.numberOfComponents;
		desc.components[i].lineStride =
						image.m_width * desc.components[i].columnStride;
		desc.components[i].theData = pixels + i;
	}
	desc.components[0].myColorType.myColor =
						(desc.numberOfComponents != 1) ? NIFRGB_R : MONOCHROME;
	desc.components[1].myColorType.myColor = NIFRGB_G;
	desc.components[2].myColorType.myColor = NIFRGB_B;
	desc.components[3].myColorType.myColor = ALPHA;

	// Write image scanlines.
	RGBA* ps = image.m_pixels;
	for (int y = 0; y < image.m_height; y++) {
		BYTE* pd = pixels;
		for (int x = 0; x < image.m_width; x++) {
			*pd++ = ps->red;
			*pd++ = ps->green;
			*pd++ = ps->blue;
			ps++;
		}
		if (FPX_WriteImageLine(flashpix, &desc) != FPX_OK)
			break;
	}
	free(pixels);
	FPX_CloseImage(flashpix);
	FPX_ClearSystem();
	return TRUE;
}

extern "C"
BOOL WINAPI imgFpxSave(LPCTSTR pszName, HDIB hdib)
{
	return imgFpxSaveEx(pszName, hdib, FPXCOMPRESS_JPEG_UNSPECIFIED, 90);
}


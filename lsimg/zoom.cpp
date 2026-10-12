///////////////////////////////////////////////////////////////////////
// zoom.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

extern "C"
HDIB WINAPI imgScaleFilter(HDIB hsrc, int cx, int cy, int method, BOOL ondisk)
{
	mif image;
	if (image.attachdib(hsrc) && image.zoom(cx, cy, method, ondisk))
		return image.createdib();
	return NULL;
}

extern "C"
BOOL WINAPI imgScalePrint(HDIB hdib, int cx, int cy, int method, HDC hdc, int sx, int sy)
{
	// サイズ変更した画像データをプリンタ(DC)へ送る
	// Windowsに限定した方法...スマートじゃないな

	BOOL bOK = FALSE;
	mif image;
	if (image.attachdib(hdib) && image.zoom(cx, cy, method, TRUE))
	{
		HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

		BITMAPINFO bi;
		memset(&bi, 0, sizeof(bi));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = image.m_width;
		bi.bmiHeader.biHeight = image.m_height;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 24;
		bi.bmiHeader.biCompression = BI_RGB;

		// Adjust image sizes for one scan line
		int widthbytes = WIDTHBYTES(image.m_width * 24);
		BYTE* line = (BYTE*)malloc(widthbytes);
		bi.bmiHeader.biSizeImage = widthbytes;
		bi.bmiHeader.biHeight = 1;

		RGBA* pixels = image.m_pixels;
		for (int y = 0; y < image.m_height; y++)
		{
			BYTE* ps = line;
			for (int x = 0; x < image.m_width; x++)
			{
				*ps++ = pixels->blue;
				*ps++ = pixels->green;
				*ps++ = pixels->red;
				pixels++;
			}

			SetDIBitsToDevice(hdc, sx, sy + y,
							image.m_width, 1, 0, 0, 0, 1,
							line, &bi, DIB_RGB_COLORS);
		}
		SetCursor(hcur);
		bOK = TRUE;
	}
	return bOK;
}

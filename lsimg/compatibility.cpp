///////////////////////////////////////////////////////////////////////
// compatibility.cpp
// FV4 / Pixefが動作するよう互換性を保つため
//

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"


extern "C"
HDIB WINAPI imgGamma(HDIB hsrc, double amount)
{
	mif image;
	if (amount != 1.0 && image.attachdib(hsrc)) {
		image.gamma_rgb(NULL, 1.0, 1.0, 1.0, amount);
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgBright(HDIB hsrc, int nAdd)
{
	mif image;
	if (nAdd != 0 && image.attachdib(hsrc)) {
		image.bright_contrast(NULL, nAdd, 0);
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgContrast(HDIB hsrc, int nLevel)
{
	mif image;
	if (nLevel != 0 && image.attachdib(hsrc)) {
		image.bright_contrast(NULL, 0, nLevel);
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgAdjustRGB(HDIB hsrc, int nRed, int nGreen, int nBlue)
{
	mif image;
	if (image.attachdib(hsrc)) {
		image.adjust_rgb(NULL, nRed, nGreen, nBlue);
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgReverse(HDIB hsrc, LPRECT prc, BOOL bHorz)
{
	mif image;
	CRect rc;
	if (image.attachdib(hsrc) && image.validaterect(prc, &rc)) {
		if (bHorz)
			image.mirror(rc.left, rc.top, rc.Width(), rc.Height());
		else
			image.flip(rc.left, rc.top, rc.Width(), rc.Height());
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgRotate(HDIB hsrc, BOOL bRotateLeft)
{
	mif image;
	if (image.attachdib(hsrc)) {
		image.rotate(bRotateLeft);
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgRotateLeft(HDIB hsrc)
{
	return imgRotate(hsrc, TRUE);
}

extern "C"
HDIB WINAPI imgRotateRight(HDIB hsrc)
{
	return imgRotate(hsrc, FALSE);
}

extern "C"
HDIB WINAPI imgGrayScale(HDIB hsrc)
{
	mif image;
	if (image.attachdib(hsrc)) {
		image.grayscale();
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgCutOut(HDIB hsrc, LPRECT prc)
{
	mif image;
	CRect rc;
	if (image.attachdib(hsrc) && image.validaterect(prc, &rc)) {
		image.crop(&rc);
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgQuantize(HDIB hsrc, int dither_type, int numcolors)
{
	mif image;
	if (image.attachdib(hsrc)) {
		image.quantize(dither_type, numcolors, NULL);
		return image.createdib();
	}
	return NULL;
}

extern "C"
HDIB WINAPI imgScalePixel(HDIB hsrc, int cx, int cy)
{
	mif image;
	if (image.attachdib(hsrc) && image.zoom_pixel(cx, cy))
		return image.createdib();
	return NULL;
}

extern "C"
HDIB WINAPI imgScaleNear(HDIB hsrc, int cx, int cy)
{
	mif image;
	if (image.attachdib(hsrc) && image.zoom_near(cx, cy))
		return image.createdib();
	return NULL;
}

extern "C"
HDIB WINAPI imgScaleLinear(HDIB hsrc, int cx, int cy)
{
	mif image;
	if (image.attachdib(hsrc) && image.zoom_linear(cx, cy))
		return image.createdib();
	return NULL;
}

extern "C"
HDIB WINAPI imgScaleInFrame(HDIB hsrc, int cx, int cy)
{
	mif image;
	if (image.attachdib(hsrc) && image.zoom_inframe(cx, cy))
		return image.createdib();
	return NULL;
}

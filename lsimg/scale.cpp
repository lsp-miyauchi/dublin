///////////////////////////////////////////////////////////////////////
// scale.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

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

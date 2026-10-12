//////////////////////////////////////////////////////////////////////
// grayscale.cpp

#include "stdafx.h"

HANDLE grayscale_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image(hdib);
	image.grayscale();
	return image.createdib();
}

HANDLE negative_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image(hdib);
	image.negative(prc);
	return image.createdib();
}

HANDLE equalize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image(hdib);
	image.equalize(NULL);
	return image.createdib();
}

HANDLE normalize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image(hdib);
	image.normalize(NULL);
	return image.createdib();
}

HANDLE autoenhance_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image(hdib);
	image.autocolorenhance(NULL);
	return image.createdib();
}

HANDLE autocontrast_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image(hdib);
	image.autocontrast(NULL);
	return image.createdib();
}


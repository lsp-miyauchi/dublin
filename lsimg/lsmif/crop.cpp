///////////////////////////////////////////////////////////////////////
// crop.cpp

#include "stdafx.h"
#include "lsmif.h"

HANDLE LSMIF_API mifCrop(HANDLE hsrc, LPRECT prc)
{
	mif src;
	if (src.attachdib(hsrc) && src.crop(prc))
		return src.createdib();
	return NULL;
}

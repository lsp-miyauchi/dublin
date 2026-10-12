///////////////////////////////////////////////////////////////////////
// filter3x3.cpp

#include "stdafx.h"

static HDIB filter(HDIB hsrc, LPRECT prc, UINT nType)
{
	mif src;
	if (src.attachdib(hsrc) && src.filter_3x3(prc, nType))
		return src.createdib();
	return NULL;
}

HANDLE soften_edge(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return filter(hdib, prc, filterSoftenEdge);
}

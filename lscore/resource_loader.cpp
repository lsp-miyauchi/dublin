/////////////////////////////////////////////////////////////////////////////
// resource_loader.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

static HIMAGELIST Loader(UINT idResource, int cxDelta)
{
	return ImageList_LoadImage(RESINST, _MKIR(idResource), 
					cxDelta, 0, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);
}

extern "C"
HIMAGELIST WINAPI lccLIMRoundMarker()
{
	return Loader(IsRGBA() ? IDB_XP_MARKER_ROUND : IDB_MARKER_ROUND, 48);
}

extern "C"
HIMAGELIST WINAPI lccLIMSmallRoundMarker()
{
	return Loader(IDB_MARKER_ROUND_SMALL, 16);
}

extern "C"
HIMAGELIST WINAPI lccLIMHorzMarker()
{
	return Loader(IDB_MARKER_HORZ, 45);
}

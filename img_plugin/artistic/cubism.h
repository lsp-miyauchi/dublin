///////////////////////////////////////////////////////////////////////
// cubism.h

#pragma once
#ifndef __CUBISM_H__
#define __CUBISM_H__

typedef struct tagCUBISMINFO {
	int tile_size;			// 0 - 100 Average diameter of tiles (in pixels)
	int tile_saturation;	// 0 - 100 Expand tiles by this amount
	COLORREF bg_color;
	int numpolygon;
} CUBISMINFO;

HDIB cubism(HDIB hsrc, LPRECT prc, CUBISMINFO* pcp);

#endif			// __CUBISM_H__


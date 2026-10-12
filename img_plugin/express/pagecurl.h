///////////////////////////////////////////////////////////////////////
// pagecurl.h

#pragma once
#ifndef __PAGECURL_H__
#define __PAGECURL_H__

enum {
	curlUpperLeft,
	curlUpperRight,
	curlLowerLeft,
	curlLowerRight
};


typedef struct tagPAGECURLINFO {
	int corner;
	BOOL vert;
	int opaque;
	int xper;
	int yper;
	COLORREF lighten_color;
	COLORREF darken_color;
	COLORREF back_color;
} PAGECURLINFO;

BOOL page_curl(mif& image, LPRECT prc, PAGECURLINFO* pci);


#endif				// __PAGECURL_H__

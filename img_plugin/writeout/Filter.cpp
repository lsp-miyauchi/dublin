//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

BOOL export_jp2k_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe);
BOOL export_jpeg_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe);
BOOL export_jpeg_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe);
BOOL export_png_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe);
BOOL export_gif_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe);
// BOOL export_webp_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("Jpegで書き出し(&J)"),		LPSTR_GID_WRITEOUT,0,{0,0},0,(FILTERCALLBACK)export_jpeg_ui },
	{ _T("Jpeg2000で書き出し(&K)"),	LPSTR_GID_WRITEOUT,0,{0,0},0,(FILTERCALLBACK)export_jp2k_ui },
	{ _T("PNG(8bit)で書き出し(&P)"),	LPSTR_GID_WRITEOUT,0,{0,0},0,(FILTERCALLBACK)export_png_ui },
	{ _T("GIF89aで書き出し(&G)"),	LPSTR_GID_WRITEOUT,0,{0,0},0,(FILTERCALLBACK)export_gif_ui },
};


//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

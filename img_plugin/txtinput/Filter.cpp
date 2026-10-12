//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE textinput_string_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE textinput_bumpmap_string_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("文字入力(&T)"),  LPSTR_GID_STRING,	IDB_TEXTINPUT,		{ 20,20 }, 1, textinput_string_ui },
	{ _T("透かし文字(&B)"),LPSTR_GID_STRING,	IDB_BUMPMAPSTRING,	{ 20,20 }, 1, textinput_bumpmap_string_ui },
};


//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

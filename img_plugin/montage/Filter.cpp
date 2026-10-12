//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE merge_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("ブレンド連結(&B)"), LPSTR_GID_MONTAGE,	IDB_BLENDMERGE,	{20,20},1, merge_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

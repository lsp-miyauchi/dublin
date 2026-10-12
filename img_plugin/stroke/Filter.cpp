//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE stroke_speed_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE stroke_deppcrunch_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE stroke_needlespin_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE stroke_pixelslime_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE stroke_xaggerate_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE stroke_zigzagerate_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE stroke_zigzack_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE stroke_tvdistortion_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("スピード(&S)"),	LPSTR_GID_STROKE,0,{0,0},0,stroke_speed_ui },
	{ _T("ワープ イン(&W)"),	LPSTR_GID_STROKE,0,{0,0},0,stroke_deppcrunch_ui },
	{ _T("烈風(&N)"),		LPSTR_GID_STROKE,0,{0,0},0,stroke_needlespin_ui },
	{ _T("スライム(&L)"),	LPSTR_GID_STROKE,0,{0,0},0,stroke_pixelslime_ui },
	{ _T("引っ掻き 1(&G)"),	LPSTR_GID_STROKE,0,{0,0},0,stroke_xaggerate_ui },
	{ _T("引っ掻き 2(&I)"),	LPSTR_GID_STROKE,0,{0,0},0,stroke_zigzagerate_ui },
	{ _T("引っ掻き 3(&T)"),	LPSTR_GID_STROKE,0,{0,0},0,stroke_tvdistortion_ui },
	{ _T("ジグザックリ(&Z)"),LPSTR_GID_STROKE,0,{0,0},0,stroke_zigzack_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

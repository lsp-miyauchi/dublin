//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE edge_detect_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE bumpmap_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE emboss_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE edge_chromium_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE edge_medical_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE edge_depth_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE edge_digitize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE edge_freeze_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE edge_shine_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE edge_innerbevel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE edge_accident_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("エッジ(&G)"),			LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_detect_ui },
	{ _T("バンプマップ(&B)"),	LPSTR_GID_EDGEDETECT,0,{0,0},0,bumpmap_ui },
	{ _T("エンボス(&E)"),		LPSTR_GID_EDGEDETECT,0,{0,0},0,emboss_ui },
	{ _T("クロム(&C)"),			LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_chromium_ui },
	{ _T("メディカル(&M)"),		LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_medical_ui },
	{ _T("深度(&P)"),			LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_depth_ui },
	{ _T("デジタイズ(&D)"),		LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_digitize_ui },
	{ _T("氷結(&F)"),			LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_freeze_ui },
	{ _T("シャイン(&S)"),		LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_shine_ui },
	{ _T("インナー ベベル(&B)"),	LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_innerbevel_ui },
	{ _T("ネオンアップ(&N)"),	LPSTR_GID_EDGEDETECT,0,{0,0},0,edge_accident_ui },
};


//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

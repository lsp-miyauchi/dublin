//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE glass_reflection_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_tiledglass_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_glasspane_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_glasstint_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_bitchescrystal_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_glassslices_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_mosaicripple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_legoland_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_diamondglass_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE glass_sectorize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("リフレクション(&R)"),	LPSTR_GID_GLASS,0,{0,0},0,glass_reflection_ui },
	{ _T("タイル ガラス 1(&G)"),	LPSTR_GID_GLASS,0,{0,0},0,glass_tiledglass_ui },
	{ _T("タイル ガラス 2(&L)"),	LPSTR_GID_GLASS,0,{0,0},0,glass_glasspane_ui },
	{ _T("ガラス ティント(&T)"),	LPSTR_GID_GLASS,0,{0,0},0,glass_glasstint_ui },
	{ _T("クリスタル(&C)"),		LPSTR_GID_GLASS,0,{0,0},0,glass_bitchescrystal_ui },
	{ _T("ガラス リプル(&P)"),	LPSTR_GID_GLASS,0,{0,0},0,glass_mosaicripple_ui },
	{ _T("ガラス スライス(&I)"),	LPSTR_GID_GLASS,0,{0,0},0,glass_glassslices_ui },
	{ _T("ガラス レゴランド(&O)"),LPSTR_GID_GLASS,0,{0,0},0,glass_legoland_ui },
	{ _T("ダイヤモンド ガラス(&D)"),LPSTR_GID_GLASS,0,{0,0},0,glass_diamondglass_ui },
	{ _T("セクタライズ(&S)"),	LPSTR_GID_GLASS,0,{0,0},0,glass_sectorize_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

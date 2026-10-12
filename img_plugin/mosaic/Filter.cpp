//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE mosaic_normal_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_sphere_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_weaver_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_plainmosaicblur_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_blendmosaic_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_plainmosaic2_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_ommadawn_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_dot_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_diagonal_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_honeycombo_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_aliasblur_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_mosapic_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_triangle_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_sectormosaic_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mosaic_polygon_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("モザイク(&M)"),		LPSTR_GID_MOSAIC,	IDB_MOSAIC, {20,20}, 1,	mosaic_normal_ui },
	{ _T("カラー ビーズ(&S)"),	LPSTR_GID_MOSAIC,	IDB_BEEDS, {20,20}, 1,	mosaic_sphere_ui },
	{ _T("織り込み(&W)"),		LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_weaver_ui },
	{ _T("スクエア シェード 1(&P)"),LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_plainmosaicblur_ui },
	{ _T("スクエア シェード 2(&B)"),LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_blendmosaic_ui },
	{ _T("プレーン(&L)"),		LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_plainmosaic2_ui },
	{ _T("フレア(&F)"),			LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_ommadawn_ui },
	{ _T("ドット(&D)"),			LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_dot_ui },
	{ _T("クロス チェッカー(&X)"),LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_diagonal_ui },
	{ _T("蜂の巣(&H)"),			LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_honeycombo_ui },
	{ _T("エリアス(&A)"),		LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_aliasblur_ui },
	{ _T("リバーブ(&V)"),		LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_mosapic_ui },
	{ _T("トライアングル(&T)"),	LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_triangle_ui },
	{ _T("セクター(&C)"),		LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_sectormosaic_ui },
	{ _T("ポリゴン(&G)"),		LPSTR_GID_MOSAIC,	0, {0,0}, 0, mosaic_polygon_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

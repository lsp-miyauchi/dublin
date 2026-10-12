//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE oilpaint_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE cubism_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_plasticsurgery_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_plasticsurface_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_metalwork_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_plasticwrap_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_blonzer_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_colorcube_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_icecreamspiral_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_squaresville_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE art_cubiclesrgb_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE ink_illust(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE ink_trace(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE ink_wax(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE ink_charcoal(HANDLE hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("オイルペイント(&O)"),			 	LPSTR_GID_ARTISTIC,	0, {20,20}, 0, oilpaint_ui },
	{ _T("キュービズム(&Q)"),			 	LPSTR_GID_ARTISTIC,	0, {20,20}, 0, cubism_ui },
	{ _T("プラスティック サーフェス(&F)"), 	LPSTR_GID_ARTISTIC,	0, {20,20}, 0, art_plasticsurface_ui },
	{ _T("プラスティック サージェリー(&S)"),	LPSTR_GID_ARTISTIC,	0, {20,20}, 0, art_plasticsurgery_ui },
	{ _T("プラスティック ラップ(&R)"),		LPSTR_GID_ARTISTIC, 0, {20,20}, 0, art_plasticwrap_ui },
	{ _T("メタル ラップ(&M)"),				LPSTR_GID_ARTISTIC,	0, {20,20}, 0, art_metalwork_ui },
	{ _T("ブロンズ ラップ(&B)"),				LPSTR_GID_ARTISTIC,	0, {20,20}, 0, art_blonzer_ui },
	{ _T("カラーキューブ(&C)"),				LPSTR_GID_ARTISTIC,	0, {20,20}, 0, art_colorcube_ui },
	{ _T("アイスクリーム スパイラル(&I)"),	LPSTR_GID_ARTISTIC,	0, {20,20}, 0, art_icecreamspiral_ui },
	{ _T("王朝の窓(&K)"),					LPSTR_GID_ARTISTIC,	0, {20,20}, 0, art_squaresville_ui },
	{ _T("キュービクルズ(&Z)"),				LPSTR_GID_ARTISTIC,	0, {20,20}, 0, art_cubiclesrgb_ui },
	{ _T("インク イラスト(&L)"),				LPSTR_GID_ARTISTIC,	0, {20,20}, 0, ink_illust },
	{ _T("インク トレース(&T)"),				LPSTR_GID_ARTISTIC,	0, {20,20}, 0, ink_trace },
	{ _T("ワックス加工(&W)"),				LPSTR_GID_ARTISTIC,	0, {20,20}, 0, ink_wax },
	{ _T("鉛筆画(&P)"),						LPSTR_GID_ARTISTIC,	0, {20,20}, 0, ink_charcoal },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

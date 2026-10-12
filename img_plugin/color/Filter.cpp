//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE fill_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE gradationmap_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_cut_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE colorify_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE colorify_hsb_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_exposure_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_booster_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_watercolor_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_histogramstretch_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_solarize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_contrast_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_converse_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_dual_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_inverse_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_reverse_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_mono_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_sepia_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_skylight_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_solar_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_removegray_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_oldphoto_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_graygray_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_temperature_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_pastel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("フィル(&F)"),				LPSTR_GID_COLOR,	IDB_FILL,		{20,20},	1,	fill_ui },
	{ _T("グラデーションマップ(&G)"),LPSTR_GID_COLOR,		IDB_GRADMAP,	{20,20},	1,	gradationmap_ui },
	{ _T("ソラライズ(&L)"),			LPSTR_GID_COLOR,	0,				{0,0},		0,	color_solarize_ui },
	{ _T("色空間減色(&C)"),			LPSTR_GID_COLOR,	0,				{0,0},		0,	color_cut_ui },
	{ _T("カラーライズ (RGB)(&Z)"),	LPSTR_GID_COLOR,	0,				{0,0},		0,	colorify_ui },
	{ _T("カラーライズ (HSB)(&H)"),	LPSTR_GID_COLOR,	0,				{0,0},		0,	colorify_hsb_ui },
	{ _T("露出(&E)"),				LPSTR_GID_COLOR,	0,				{0,0},		0,	color_exposure_ui },
	{ _T("色温度(&T)"),				LPSTR_GID_COLOR,	0,				{0,0},		0,	color_temperature_ui },
	{ _T("スカイライト(&K)"),		LPSTR_GID_COLOR,	0,				{0,0},		0,	color_skylight_ui },
	{ _T("彩度ブースト(&B)"),		LPSTR_GID_COLOR,	0,				{0,0},		0,	color_booster_ui },
	{ _T("水彩(&W)"),				LPSTR_GID_COLOR,	0,				{0,0},		0,	color_watercolor_ui },
	{ _T("ヒストグラム伸張(&G)"),	LPSTR_GID_COLOR,	0,				{0,0},		0,	color_histogramstretch_ui },
	{ _T("コントラスト(&O)"),		LPSTR_GID_COLOR,	0,				{0,0},		0,	color_contrast_ui },
	{ _T("コンバース(&V)"),			LPSTR_GID_COLOR,	0,				{0,0},		0,	color_converse_ui },
	{ _T("パステル(&P)"),			LPSTR_GID_COLOR,	0,				{0,0},		0,	color_pastel_ui },
	{ _T("デュアル(&D)"),			LPSTR_GID_COLOR,	0,				{0,0},		0,	color_dual_ui },
	{ _T("インバース(&I)"),			LPSTR_GID_COLOR,	0,				{0,0},		0,	color_inverse_ui },
	{ _T("リバース(&R)"),			LPSTR_GID_COLOR,	0,				{0,0},		0,	color_reverse_ui },
	{ _T("セピア(&S)"),				LPSTR_GID_COLOR,	0,				{0,0},		0,	color_sepia_ui },
	{ _T("ソーラー(&A)"),			LPSTR_GID_COLOR,	0,				{0,0},		0,	color_solar_ui },
	{ _T("グレーの除去(&E)"),		LPSTR_GID_COLOR,	0,				{0,0},		0,	color_removegray_ui },
	{ _T("オールドフォト(&T)"),		LPSTR_GID_COLOR,	0,				{0,0},		0,	color_oldphoto_ui },
	{ _T("グレー＆グレー(&Y)"),		LPSTR_GID_COLOR,	0,				{0,0},		0,	color_graygray_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

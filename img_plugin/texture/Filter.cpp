//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE tiling_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE tile_fill_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE texturizer_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE texture_weavedistortion_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE texture_crossweavedistortion_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE texture_weavetexture_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE pyramids_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE panel_stripes_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE square_bumps_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE gridtexture_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE stardust_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE sparkle_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE texture_poolshadow_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE texture_bedsheets_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("タイリング"),			LPSTR_GID_TEXTURE,	0,{0,0},0,tiling_ui },
	{ _T("タイルで埋める"),		LPSTR_GID_TEXTURE,	0,{0,0},0,tile_fill_ui },
	{ _T("テクスチャライザ"),		LPSTR_GID_TEXTURE,	0,{0,0},0,texturizer_ui },
	{ _T("デジタル織り込み"),		LPSTR_GID_TEXTURE,	0,{0,0},0,texture_weavedistortion_ui },
	{ _T("デジタル織り込み クロス"),	LPSTR_GID_TEXTURE,	0,{0,0},0,texture_crossweavedistortion_ui },
	{ _T("デジタル織り込み シェード"),LPSTR_GID_TEXTURE,	0,{0,0},0,texture_weavetexture_ui },
	{ _T("ピラミッド"),			LPSTR_GID_TEXTURE,	0,{0,0},0,pyramids_ui },
	{ _T("パネル ストライプ"),	LPSTR_GID_TEXTURE,	0,{0,0},0,panel_stripes_ui },
	{ _T("スクエア バンプ"),		LPSTR_GID_TEXTURE,	0,{0,0},0,square_bumps_ui },
	{ _T("グリッド テクスチャ"),	LPSTR_GID_TEXTURE,	0,{0,0},0,gridtexture_ui },
	{ _T("スターダスト"),		LPSTR_GID_TEXTURE,	0,{0,0},0,stardust_ui },
	{ _T("スパークル"),			LPSTR_GID_TEXTURE,	0,{0,0},0,sparkle_ui },
	{ _T("プール シャドウ"),		LPSTR_GID_TEXTURE,	0,{0,0},0,texture_poolshadow_ui },
	{ _T("シーツのうねり"),		LPSTR_GID_TEXTURE,	0,{0,0},0,texture_bedsheets_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

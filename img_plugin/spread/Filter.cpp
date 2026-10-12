//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"


HANDLE spread_shift_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE spread_blast_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE spread_blur_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE spread_rotation_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE spread_zoom_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE spread_cross_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE spread_speed_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE spread_twister_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const TCHAR sszGroup[] = _T("拡散");
static const FILTER_ENTRY_EX fen[] = {
	{ _T("拡散(&S)"),			sszGroup,0,{0,0},0,spread_shift_ui },
	{ _T("クロス(&X)"),			sszGroup,0,{0,0},0,spread_cross_ui },
	{ _T("ぼかし(&B)"),			sszGroup,0,{0,0},0,spread_blur_ui },
	{ _T("ブラスト(&L)"),		sszGroup,0,{0,0},0,spread_blast_ui },
	{ _T("ローテーション(&R)"),	sszGroup,0,{0,0},0,spread_rotation_ui },
	{ _T("ズーム(&Z)"),			sszGroup,0,{0,0},0,spread_zoom_ui },
	{ _T("スピード(&P)"),		sszGroup,0,{0,0},0,spread_speed_ui },
	{ _T("ツイスター(&T)"),		sszGroup,0,{0,0},0,spread_twister_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

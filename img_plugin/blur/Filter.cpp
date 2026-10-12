//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE sharpen_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE sharpen_edge(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE unsharp_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

HANDLE blur_shift_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE blur_shiver_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE blur_trans_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE blur_zoomin_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE blur_zoomout_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE blur_zoomsplash_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

HANDLE blur_simple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE blur_gauss_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE blur_gauss_sel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE blur_gauss_ld_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE soften_edge(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("アンシャープマスク(&U)"),	LPSTR_GID_SHARPEN,0,{0,0},0,unsharp_ui },
	{ _T("シャープに(&S)"),			LPSTR_GID_SHARPEN,IDB_SHARPEN, {20,20}, 1,	sharpen_ui },
	{ _T("輪郭をシャープに(&O)"),		LPSTR_GID_SHARPEN,0,{0,0},0,sharpen_edge },
	{ _T("ぼかし(&B)"),				LPSTR_GID_SHADING,IDB_BLUR, {20,20}, 1,	blur_simple_ui },
	{ _T("ぼかし（ガウス）(&G)"),		LPSTR_GID_SHADING,0,{0,0},0,blur_gauss_ui },
	{ _T("ぼかし（選択的）(&S)"),		LPSTR_GID_SHADING,0,{0,0},0,blur_gauss_sel_ui },
	{ _T("ぼかし（明暗）(&L)"),		LPSTR_GID_SHADING,0,{0,0},0,blur_gauss_ld_ui },
	{ _T("輪郭以外をぼかす（明暗）(&D)"),LPSTR_GID_SHADING,0,{0,0},0,soften_edge },
	{ _T("ぶれ（シフト）(&B)"),		LPSTR_GID_BLUR,0,{0,0},0,blur_shift_ui },
	{ _T("ぶれ（振動）(&S)"),		LPSTR_GID_BLUR,0,{0,0},0,blur_shiver_ui },
	{ _T("ぶれ（トランス）(&T)"),		LPSTR_GID_BLUR,0,{0,0},0,blur_trans_ui },
	{ _T("視点ズーム イン(&I)"),		LPSTR_GID_BLUR,0,{0,0},0,blur_zoomin_ui },
	{ _T("視点ズーム アウト(&O)"),	LPSTR_GID_BLUR,0,{0,0},0,blur_zoomout_ui },
	{ _T("視点ズーム スプラッシュ(&S)"),LPSTR_GID_BLUR,0,{0,0},0,blur_zoomsplash_ui },
};


//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

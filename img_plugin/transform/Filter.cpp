//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE wave_bubblecross_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_flute_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_smallpox_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_bubbledistort_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_crumple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_warphorz_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_warpvert_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_radialwave_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_watersurface_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_wonderland_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_sausagewave_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE wave_flutteringflag_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE water_ring_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE ripple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE shear_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE perspect_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE polar_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("波紋"),			LPSTR_GID_TRANSFORM,0,{0,0},0,water_ring_ui },
	{ _T("さざ波"),			LPSTR_GID_TRANSFORM,0,{0,0},0,ripple_ui },
	{ _T("傾斜"),			LPSTR_GID_TRANSFORM,0,{0,0},0,shear_ui },
	{ _T("波 バブルクロス"),	LPSTR_GID_TRANSFORM,0,{0,0},0,wave_bubblecross_ui },
	{ _T("波 フルート"),		LPSTR_GID_TRANSFORM,0,{0,0},0,wave_flute_ui },
	{ _T("うねり バブル 1"),	LPSTR_GID_TRANSFORM,0,{0,0},0,wave_smallpox_ui },
	{ _T("うねり バブル 2"),	LPSTR_GID_TRANSFORM,0,{0,0},0,wave_bubbledistort_ui },
	{ _T("うねり 放射"),		LPSTR_GID_TRANSFORM,0,{0,0},0,wave_crumple_ui },
	{ _T("うねり 水平"),		LPSTR_GID_TRANSFORM,0,{0,0},0,wave_warphorz_ui },
	{ _T("うねり 垂直"),		LPSTR_GID_TRANSFORM,0,{0,0},0,wave_warpvert_ui },
	{ _T("波紋 シンプル"),	LPSTR_GID_TRANSFORM,0,{0,0},0,wave_radialwave_ui },
	{ _T("波紋 遠近"),		LPSTR_GID_TRANSFORM,0,{0,0},0,wave_watersurface_ui },
	{ _T("波紋 合成 1"),		LPSTR_GID_TRANSFORM,0,{0,0},0,wave_wonderland_ui },
	{ _T("波紋 合成 2"),		LPSTR_GID_TRANSFORM,0,{0,0},0,wave_sausagewave_ui },
	{ _T("はためき"),		LPSTR_GID_TRANSFORM,0,{0,0},0,wave_flutteringflag_ui },
	{ _T("透視変換"),		LPSTR_GID_TRANSFORM,0,{0,0},0,perspect_ui },
	{ _T("極座標"),			LPSTR_GID_TRANSFORM,0,{0,0},0,polar_ui },
};


//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

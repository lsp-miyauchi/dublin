//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE mirror_seamlesswelder_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_quadflip_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_perverse_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_promille_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_radialshoved_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_kaleidoscopeI_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_kaleidoscopeII_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_kaleidoscopeIII_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_kaleidoscopeIV_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_ringmirror_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_distmirrorhorz_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE mirror_distmirrorvert_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const TCHAR sszGroup[] = _T("ミラー");

static const FILTER_ENTRY_EX fen[] = {
	{ _T("シームレス接着(&S)"),		sszGroup,0,{0,0},0,mirror_seamlesswelder_ui },
	{ _T("方形フリップ(&Q)"),		sszGroup,0,{0,0},0,mirror_quadflip_ui },
	{ _T("ミラースコープ 1(&M)"),		sszGroup,0,{0,0},0,mirror_perverse_ui },
	{ _T("ミラースコープ 2(&P)"),		sszGroup,0,{0,0},0,mirror_promille_ui },
	{ _T("放射(&A)"),				sszGroup,0,{0,0},0,mirror_radialshoved_ui },
	{ _T("カレイドスコープ 1(&K)"),	sszGroup,0,{0,0},0,mirror_kaleidoscopeI_ui },
	{ _T("カレイドスコープ 2(&E)"),	sszGroup,0,{0,0},0,mirror_kaleidoscopeII_ui },
	{ _T("カレイドスコープ 3(&I)"),	sszGroup,0,{0,0},0,mirror_kaleidoscopeIII_ui },
	{ _T("カレイドスコープ 4(&D)"),	sszGroup,0,{0,0},0,mirror_kaleidoscopeIV_ui },
	{ _T("リング ミラー(&R)"),		sszGroup,0,{0,0},0,mirror_ringmirror_ui },
	{ _T("ゆがみ ミラー（水平）(&H)"),sszGroup,0,{0,0},0,mirror_distmirrorhorz_ui },
	{ _T("ゆがみ ミラー（垂直）(&V)"),sszGroup,0,{0,0},0,mirror_distmirrorvert_ui },
};

////////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

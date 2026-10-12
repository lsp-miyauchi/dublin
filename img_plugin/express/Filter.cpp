//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE curtain_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_halftone_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE illusion_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE fractal_trace_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE seemless_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE halftone_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE pagecurl_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE grad_blinds_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_printscreen_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_acidrain_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_scrollshift_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_scanlineshifter_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_plasticstarfish_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_crystalball_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_marblemadnessone_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_circuitweaver_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_liftcover_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE expr_patchwork_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE dither_works_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("カーテン(&C)"),			LPSTR_GID_EXPRESSION,0,{0,0},0,curtain_ui },
	{ _T("カラー ハーフトーン(&H)"),	LPSTR_GID_EXPRESSION,0,{0,0},0,color_halftone_ui },
	{ _T("イリュージョン(&I)"),		LPSTR_GID_EXPRESSION,0,{0,0},0,illusion_ui },
	{ _T("フラクタル トレース(&F)"),	LPSTR_GID_EXPRESSION,0,{0,0},0,fractal_trace_ui },
	{ _T("シームレス(&S)"),			LPSTR_GID_EXPRESSION,0,{0,0},0,seemless_ui },
	{ _T("ハーフトーン(&T)"),		LPSTR_GID_EXPRESSION,0,{0,0},0,halftone_ui },
	{ _T("ページをめくる(&P)"),		LPSTR_GID_EXPRESSION,0,{0,0},0,pagecurl_ui },
	{ _T("グラデーション ブラインド(&G)"),
									LPSTR_GID_EXPRESSION,0,{0,0},0,grad_blinds_ui },
	{ _T("スクリーン プリント(&R)"),	LPSTR_GID_EXPRESSION,0,{0,0},0,expr_printscreen_ui },
	{ _T("レイン ドロップ(&D)"),		LPSTR_GID_EXPRESSION,0,{0,0},0,expr_acidrain_ui },
	{ _T("プラスティック スターフィッシュ(&A)"),
									LPSTR_GID_EXPRESSION,0,{0,0},0,expr_plasticstarfish_ui },
	{ _T("クリスタル ボール(&B)"),	LPSTR_GID_EXPRESSION,0,{0,0},0,expr_crystalball_ui },
	{ _T("遮蔽網(&M)"),				LPSTR_GID_EXPRESSION,0,{0,0},0,expr_marblemadnessone_ui },
	{ _T("スクロール(&L)"),			LPSTR_GID_EXPRESSION,0,{0,0},0,expr_scrollshift_ui },
	{ _T("ラインシフト(&N)"),		LPSTR_GID_EXPRESSION,0,{0,0},0,expr_scanlineshifter_ui },
	{ _T("巡回織り(&U)"),			LPSTR_GID_EXPRESSION,0,{0,0},0,expr_circuitweaver_ui },
	{ _T("ページを折る(&V)"),		LPSTR_GID_EXPRESSION,0,{0,0},0,expr_liftcover_ui },
	{ _T("パッチワーク(&W)"),		LPSTR_GID_EXPRESSION,0,{0,0},0,expr_patchwork_ui },
	{ _T("モノクロ ディザ(&M)"),		LPSTR_GID_EXPRESSION,0,{0,0},0,dither_works_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

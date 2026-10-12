//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE frame_simple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_fadeframe_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_bullseye_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_3d_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_3dglass_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_3dglassbevel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_metal_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_bevelIII_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_mirrored_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_mirroredbevel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE frame_shdowcaster_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE framemarble_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE framefuzzy_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("カラー フレーム(&C)"),	LPSTR_GID_FRAME,0,{0,0},0,frame_simple_ui },
	{ _T("フェード(&F)"),		LPSTR_GID_FRAME,0,{0,0},0,frame_fadeframe_ui },
//@	{ _T("円形ボタン"),			LPSTR_GID_FRAME,0,{0,0},0,frame_bullseye_ui },
	{ _T("3D ボタン(&B)"),		LPSTR_GID_FRAME,0,{0,0},0,frame_3d_ui },
	{ _T("3D ガラス(&G)"),		LPSTR_GID_FRAME,0,{0,0},0,frame_3dglass_ui },
	{ _T("3D ベベルガラス(&E)"),	LPSTR_GID_FRAME,0,{0,0},0,frame_3dglassbevel_ui },
//@	{ _T("3D 円形ボタン"),		LPSTR_GID_FRAME,0,{0,0},0,frame_bevelIII_ui },
	{ _T("メタルフレーム(&M)"),	LPSTR_GID_FRAME,0,{0,0},0,frame_metal_ui },
//@	{ _T("ミラー"),				LPSTR_GID_FRAME,0,{0,0},0,frame_mirrored_ui },
//@	{ _T("ミラー ベベル"),		LPSTR_GID_FRAME,0,{0,0},0,frame_mirroredbevel_ui },
	{ _T("シャドウ キャスト(&S)"),LPSTR_GID_FRAME,0,{0,0},0,frame_shdowcaster_ui },
	{ _T("テクスチャ枠(&T)"),	LPSTR_GID_FRAME,0,{0,0},0,framemarble_ui },
	{ _T("ファジー フレーム(&Z)"),LPSTR_GID_FRAME	,0,{0,0},0,framefuzzy_ui },
};


//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

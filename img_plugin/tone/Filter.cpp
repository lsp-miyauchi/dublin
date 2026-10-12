//////////////////////////////////////////////////////////////////
// filter.cpp

#include "stdafx.h"
#include "resource.h"

HANDLE modulate_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE tonecurve_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE contrast_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE color_balance_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE gamma_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE grayscale_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE negative_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE equalize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE normalize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE autocontrast_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE autoenhance_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);
HANDLE posterize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe);

static const FILTER_ENTRY_EX fen[] = {
	{ _T("色相・彩度(&M)"),		LPSTR_GID_TONE,	IDB_MODULATE, {20,20}, 1,	modulate_ui },
	{ _T("トーンカーブ(&T)"),	LPSTR_GID_TONE,	IDB_TONECURVE, {20,20}, 1,	tonecurve_ui },
	{ _T("明るさ・コントラスト(&B)"),	LPSTR_GID_TONE,0,{0,0},0,	contrast_ui },
	{ _T("カラーバランス(&C)"),	LPSTR_GID_TONE,	IDB_COLORBALANCE, {20,20}, 1, color_balance_ui },
	{ _T("ガンマ補正(&G)"),		LPSTR_GID_TONE,	IDB_GAMMA, {20,20}, 1,	gamma_ui },
	{ _T("コントラストの自動補正(&A)"),LPSTR_GID_TONE,0,{0,0},0,autocontrast_ui },
	{ _T("彩度の自動補正(&E)"),	LPSTR_GID_TONE,0,{0,0},0,autoenhance_ui },
	{ _T("グレースケール(&L)"),	LPSTR_GID_TONE,	IDB_GRAYSCALE, {20,20}, 1, grayscale_ui },
	{ _T("階調の反転(&R)"),		LPSTR_GID_TONE,	IDB_NEGATIVE, {20,20}, 1,	negative_ui },
	{ _T("平均化（イコライズ）(&E)"),	LPSTR_GID_TONE,	0,{0,0},0,equalize_ui },
	{ _T("ノーマライズ(&N)"),	LPSTR_GID_TONE,0,{0,0},0,normalize_ui },
	{ _T("ポスタライズ(&P)"),	LPSTR_GID_TONE,0,{0,0},0,posterize_ui },
};

//////////////////////////////////////////////////////////////////
//

#include "..\filter_core.cpp"

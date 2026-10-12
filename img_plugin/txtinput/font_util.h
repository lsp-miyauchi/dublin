// font_util.h : ヘッダー ファイル
//

#pragma once
#ifndef __FONT_UTIL_H__
#define __FONT_UTIL_H__

#include "filter.h"

#ifndef INVALID_COLOR
	#define INVALID_COLOR		((COLORREF)-1)
#endif

#define M_PI					(3.14159265358979323846)
#define INTENSITY(r,g,b)		((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)
#define	DEG2RAD(x)				((x) * M_PI / 180.0)

#define DSS_FMT_LEFT			0x00000000
#define DSS_FMT_CENTER			0x00000001
#define DSS_FMT_RIGHT			0x00000002
#define DSS_FMT_MASK			0x0000000F
#define DSS_AA_SMOOTH			0x00000000
#define DSS_AA_CLEAR			0x00000010
#define DSS_AA_STRONG			0x00000020
#define DSS_AA_MASK				0x000000F0
#define DSS_OUTLINE				0x00000100
#define DSS_OUTLINE_INNER		0x00000200
#define DSS_DROPSHADOW			0x00000400
#define DSS_EMBOSS_EDGE			0x00000800
#define DSS_FLIP_VERT			0x00001000
#define DSS_FLIP_HORZ			0x00002000
#define DSS_TATEGAKI			0x00004000
#define DSS_USE_TEXTURE			0x80000000

typedef struct tagDRAWSTRINGINFO {
	LOGFONT lf;
	POINT pt;
	UINT state;
	int direction;						// 方向
	int shear;							// 文字の傾き (-0.45 ～ 0.45) x 100倍
	int char_scale;						// 文字幅スケール 20 ～ 200 %
	int opaque;							// 全体の不透明度
	int anti_alias;						// アンチエリアス
	int numcolors;						// 塗り：色数
	GRADCOLORS grad[MAX_GRADCOLORS];	// 塗り：グラデーションデータ
	COLORREF shadow_cr;					// ドロップシャドウ：影色
	SHORT shadow_distance;				// ドロップシャドウ：距離
	WORD shadow_gauss;					// ドロップシャドウ：ぼかし（ガウス x 10倍）
	int shadow_opaque;					// ドロップシャドウ：不透明度
	COLORREF outline_cr;				// 袋文字：塗りつぶし色
	double ee_azimuth;					// 立体化：アジマス
	double ee_elevation;				// 立体化：高さ
	HDIB htxr;							// テクスチャ
	LPCTSTR text;
} DRAWSTRINGINFO;

typedef struct tagBUMPMAPSTRINGINFO {
	LOGFONT lf;						// フォント情報
	POINT pt;
	UINT state;
	int char_scale;					// 文字幅スケール 20 ～ 200 %
	int shear;						// フォントの傾き(-45度 ～ 45度）
	MIF_BUMPMAP bp;
	LPCTSTR text;					// 描画文字列
} BUMPMAPSTRINGINFO;

HFONT create_scalefont(LOGFONT* plf, int char_scale);
HDIB create_dragstring(LPCTSTR pszString, LPLOGFONT plf, int char_scale, UINT state, int shear);
HDIB draw_string(HDIB hdib, DRAWSTRINGINFO* pds);
HDIB draw_bumpmapstring(HDIB hdib, BUMPMAPSTRINGINFO* pbs);


#endif				// __FONT_UTIL_H__

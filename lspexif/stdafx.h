// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。

#pragma once

#ifndef STRICT
#define STRICT
#endif

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

#define WINVER			0x0600
#define _WIN32_WINNT	0x0600
#define _WIN32_IE		0x0700
#define _RICHEDIT_VER	0x0300

#define _WTL_NO_CSTRING
#define _WTL_NO_UNION_CLASSES

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
// #define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 一部の CString コンストラクタは明示的です。


#include <math.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atltypes.h>
#include <atlstr.h>

#include <atlapp.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlgdi.h>
#include <atlprint.h>
#include <atlmisc.h>
#include <atlframe.h>

using namespace ATL;

#include "..\lscore\atl_special_handler.h"
#include "..\lscore\lscore.h"
#include "..\lscore\lccctrls.h"
#include "..\lscore\osplatform.h"

#include "debug.h"

// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 一部の CString コンストラクタは明示的です。

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlstr.h>
#include <atltypes.h>

#include <shlobj.h>
#include <commctrl.h>
#include <shlwapi.h>

#include <atlwin.h>
#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>

#include "..\libarcspec\libarcspec.h"

using namespace ATL;


#include "..\lscore\atl_special_handler.h"
#include "..\lscore\lscore.h"
#include "..\lscore\osplatform.h"
#include "..\lscore\lccctrls.h"
#include "..\lscore\mrulist.h"
#include "..\lscore\cdll.h"
#include "..\fv7\oeminc.h"

#pragma comment(lib, "lscore.lib")
#pragma comment(lib, "libarcspec.lib")

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

// 一般的で無視しても安全な MFC の警告メッセージの一部の非表示を解除します。
#define _ATL_ALL_WARNINGS

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atltypes.h>
#include <atlstr.h>
#include <atlpath.h>

#include <comdef.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <commctrl.h>
#include <shlwapi.h>

#include <windowsx.h>
#include "..\fv7\oeminc.h"
#include "..\lscore\lscore.h"
#include "..\lscore\atl_special_handler.h"
#include "..\libarcspec\libarcspec.h"
#include <atlapp.h>
#include <atlwinx.h>
#include <atlctrls.h>
#include <atlctrlx.h>

using namespace ATL;

#include <gdiplus.h>
using namespace Gdiplus;

#include "..\lscore\lccctrls.h"
#include "foplib.h"
#include "debug.h"


// user libraries
#pragma comment(lib, "lscore.lib")
#pragma comment(lib, "libarcspec.lib")

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif

//#define SIZE_TOOLBUTTON	20
#define SIZE_TOOLBUTTON	16


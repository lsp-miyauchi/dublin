// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#define _CRT_SECURE_NO_DEPRECATE

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0600
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0700
#endif

// Windows ヘッダー ファイル:
#include <windows.h>
#include <versionhelpers.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 一部の CString コンストラクタは明示的です。

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlcoll.h>

using namespace ATL;

#define _WTL_NO_UNION_CLASSES
#define _WTL_NO_CSTRING

#include <atlapp.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <shlwapi.h>

//extern CAppModule _Module;

#include "cabmodule.h"

#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())

#include "atl_msg_handler.h"

#include "debug.h"

// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

// #define ISOLATION_AWARE_ENABLED 1

#include "targetver.h"

// #define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _WTL_NO_CSTRING

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>

#include <shlobj.h>
#include <atlcoll.h>

#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlmisc.h>
#include <atltheme.h>
#include <atlpath.h>

extern CAppModule _Module;

using namespace ATL;

#include "atl_special_handler.h"
#include "chartype.h"
#include "xwin.h"

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;


#include "debug.h"


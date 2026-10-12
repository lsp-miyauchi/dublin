// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

//#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
//#define _WTL_NO_CSTRING
//#define _WTL_NO_UNION_CLASSES

#pragma warning(disable: 4100)	// 引数が関数内で使われないという警告を無視

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlcoll.h>
#include <atlcom.h>
#include <atltypes.h>

extern CAppModule _Module;

using namespace ATL;

#define CString CAtlString

#include <shlobj.h>
#include <shlwapi.h>
#include <gdiplus.h>
using namespace Gdiplus;

#include "resource.h"

#include "gdiplus_sup.h"
#include "array.h"
#include "cdll.h"
#include "filehandle.h"

#include "common.h"

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif


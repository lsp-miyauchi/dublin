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

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 一部の CString コンストラクタは明示的です。
#define _ATL_ALL_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
#include <atltypes.h>

using namespace ATL;

#include <atlwin.h>
#include <atlstr.h>
#include <atltime.h>
#include <atlpath.h>
#include <atlcoll.h>

#include <shlobj.h>
#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>

#include <gdiplus.h>
using namespace Gdiplus;

#include <commoncontrols.h>			// for IID_IImageList
#include <htmlhelp.h>
#include <htmlguid.h>

#include <propkey.h>
#include <propsys.h>

#include "..\gnu_libs\tinyxml_2_5_3\tinyxml.h"

#include "..\lscore\atl_special_handler.h"
#include "..\lscore\lscore.h"
#include "..\lscore\lccctrls.h"
#include "..\lscore\chartype.h"
#include "..\lscore\xwin.h"
#include "..\lscore\mrulist.h"
#include "..\lscore\osplatform.h"

// user libraries
#ifdef _DEBUG
	#pragma comment(lib, "tinyxmld.lib")
#else
	#pragma comment(lib, "tinyxml.lib")
#endif

typedef DWORD DROPEFFECT;

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif


#include "cfile.h"
#include "debug.h"


// stdafx.h : 標準のシステム インクルード ファイル、
//            または参照回数が多く、かつあまり変更されない
//            プロジェクト専用のインクルード ファイルを記述します。
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "targetver.h"

#include <tchar.h>

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include <ppl.h>
#include "mfmem.h"
#include "memstream.h"
#include "memmgr.h"
#include "rects.h"
#include "lsmif\lsmif.h"

#include "debug.h"

#define KMIMG_LOCAL_USE

using namespace Concurrency;

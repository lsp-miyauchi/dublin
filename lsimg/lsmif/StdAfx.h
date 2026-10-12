// stdafx.h : 標準のシステム インクルード ファイル、
//            または参照回数が多く、かつあまり変更されない
//            プロジェクト専用のインクルード ファイルを記述します。
//

#if !defined(AFX_STDAFX_H__4150AB05_3183_4751_8862_B7F4071D3A45__INCLUDED_)
#define AFX_STDAFX_H__4150AB05_3183_4751_8862_B7F4071D3A45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS


// この位置にヘッダーを挿入してください
#define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから殆ど使用されないスタッフを除外します

#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <malloc.h>
#include <math.h>
#include <ppl.h>

using namespace Concurrency;

#include "rects.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#include "debug.h"

#endif // !defined(AFX_STDAFX_H__4150AB05_3183_4751_8862_B7F4071D3A45__INCLUDED_)

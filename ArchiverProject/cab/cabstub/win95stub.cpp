////////////////////////////////////////////////////////////////////////
// IsDebuggerPresent のリンクを無効にする

#include "stdafx.h"
#include "cabstub.h"

#ifndef _WIN64

// 装飾された名前のアドレスを作るための仮定義
// (これだけでインポートを横取りしている)
EXTERN_C int WINAPI _imp__IsDebuggerPresent()
{
	return PtrToInt((void*) &_imp__IsDebuggerPresent);
}

// 実際に横取り処理を行う関数
EXTERN_C BOOL WINAPI Cover_IsDebuggerPresent()
{
	return FALSE;
}

// 関数が実際に呼び出されたときに備えて
// 横取り処理関数を呼び出させるための下準備
EXTERN_C void __stdcall DoCover_IsDebuggerPresent()
{
	DWORD dw;
	DWORD_PTR FAR* lpdw;
	// 横取り関数を設定するアドレスを取得
	lpdw = (DWORD_PTR FAR*) &_imp__IsDebuggerPresent;
	// このアドレスを書き込めるように設定
	// (同じプログラム内なので障害なく行える)
	VirtualProtect(lpdw, sizeof(DWORD_PTR), PAGE_READWRITE, &dw);
	// 横取り関数を設定
	*lpdw = (DWORD_PTR)(FARPROC) Cover_IsDebuggerPresent;
	// 読み書きの状態を元に戻す
	VirtualProtect(lpdw, sizeof(DWORD_PTR), dw, NULL);
}

// アプリケーションが初期化される前に下準備を呼び出す
// ※ かなり早くに初期化したいときは、このコードを
//  ファイルの末尾に書いて「#pragma init_seg(lib)」を、
//  この変数宣言の手前に書きます。
//  初期化を急ぐ必要が無い場合は WinMain 内から
//  DoCover_IsDebuggerPresent を呼び出して構いません。
EXTERN_C int s_DoCover_IsDebuggerPresent = (int) (DoCover_IsDebuggerPresent(), 0);

#endif

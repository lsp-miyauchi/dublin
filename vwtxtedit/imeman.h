/////////////////////////////////////////////////////////////////////////
// imeman.h

#pragma once
#ifndef __IME_MANAGER_H__
#define __IME_MANAGER_H__



void InitializeImeManager();
void TerminateImeManager();


//=========================================================================
//	IME制御マネージャ
//
//	Global IME をサポートするには、ウインドウメッセージの処理を
//	根本的に入れ替える必要がある。そこで、処理をこのクラスにまとめ
//	Windowクラスと連携処理を行うことで、ライブラリの外からは一切
//	気にせず処理をできるようにしておく。なお、Global IMEに対応
//	するにはバージョンの新しいPlatform SDKが必要なため
//	マクロ USEGLOBALIME が定義されていなければその辺は処理しない。
//=========================================================================

class IMEManager
{
private:
	static IMEManager* m_pImmInstance;

public:
	IMEManager()
	{
	}

	~IMEManager()
	{
	}

	// フォント指定
	void SetFont(HWND hwnd, const LOGFONT& lf)
	{
		LOGFONT* plf = const_cast<LOGFONT*>(&lf);

		HIMC ime = ::ImmGetContext(hwnd);
		::ImmSetCompositionFont(ime, plf);
		::ImmReleaseContext(hwnd, ime);
	}

	// 位置指定
	void SetPos( HWND hwnd, int x, int y )
	{
		COMPOSITIONFORM cf;
		cf.dwStyle = CFS_POINT;
		cf.ptCurrentPos.x  = x;
		cf.ptCurrentPos.y  = y;

		HIMC ime = ::ImmGetContext(hwnd);
		::ImmSetCompositionWindow(ime, &cf);
		::ImmReleaseContext(hwnd, ime);
	}

	// 確定文字列ゲット
	int GetString(HWND hwnd, CString& str)
	{
		str.Empty();
		HIMC ime = ::ImmGetContext(hwnd);
		long s = ::ImmGetCompositionStringW(ime, GCS_RESULTSTR, NULL, 0);
		LPTSTR p = str.GetBuffer(s / sizeof(WCHAR) + 1);
		::ImmGetCompositionStringW(ime, GCS_RESULTSTR, p, s);
		p[s / sizeof(WCHAR)] = 0;
		str.ReleaseBuffer();
		::ImmReleaseContext(hwnd, ime);
		return str.GetLength();
	}

};

// 唯一のIME管理オブジェクトを返す
extern IMEManager* gpIme;
inline IMEManager& ime() { return *gpIme; }



#endif			// __IME_MANAGER_H__

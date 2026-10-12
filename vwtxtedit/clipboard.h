//=========================================================================
//	クリップボード管理
//
//	OpenClipboard や CloseClipboard 辺りの呼び出しを適当に自動化します。
//=========================================================================

#pragma once
#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__


class CClipboard
{
public:
	bool m_bOpened:1;

public:

	// 開く
	CClipboard(HWND owner, bool bKeepData = true)
		: m_bOpened(false)
	{
		if (::OpenClipboard(owner)) {
			if (bKeepData || ::EmptyClipboard())
				m_bOpened = true;
			else
				::CloseClipboard();
		}
	}

	// 閉じる
	~CClipboard()
	{
		if (m_bOpened)
			::CloseClipboard();
	}

	// データ読み込み
	inline HANDLE GetData( UINT uFormat ) const { return ::GetClipboardData(uFormat); }

	// 指定フォーマットのデータがクリップボード上にあるか？
	inline bool IsAvail(UINT uFormat) const { return false != ::IsClipboardFormatAvailable(uFormat); }

	// 指定フォーマットのデータがクリップボード上にあるか？(複数)
	inline bool IsAvail(UINT uFormats[], int num ) const { return -1 != ::GetPriorityClipboardFormat(uFormats, num); }

	// 空にする
	inline void Empty() { if (m_bOpened) ::EmptyClipboard(); }

	// テキスト読み込み
	int GetUnicodeText(CString& str)
	{
		str.Empty();
		HANDLE h = GetData(CF_UNICODETEXT);
		if (h != NULL) {
			WCHAR* p = (WCHAR*)::GlobalLock(h);
			if (p) {
				str = p;;
				::GlobalUnlock(h);
			}
		}
		return str.GetLength();
	}

	// データ書き込み
	inline bool SetData( UINT uFormat, HANDLE hData ) { return NULL != ::SetClipboardData( uFormat, hData ); }

	// 独自フォーマットの登録
	inline UINT RegisterFormat( const TCHAR* name ) { return ::RegisterClipboardFormat(name); }

	// 正常に開かれているかチェック
	inline bool isOpened() const { return m_bOpened; }
};



#endif		// __CLIPBOARD_H__

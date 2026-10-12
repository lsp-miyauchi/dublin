////////////////////////////////////////////////////////////////////////
// enumstring.h

#ifndef __ENUMSTRING_H__
#define __ENUMSTRING_H__

#include <windows.h>
#include <shlobj.h>
#include "resource.h"

class CEnumString : public IEnumString
{
private:
	ULONG           m_cRef;     // 参照カウント
	LPUNKNOWN       m_pUnkRef;  // 参照カウントを増やすIUnknownのポインタ
	ULONG           m_iCur;     // 現在のリスト位置
	ULONG           m_cstr;     // クラス内の文字列数
	LPOLESTR        *m_prgstr;  // 文字列のコピー
	IMalloc			*m_pmem;	// IMallocインターフェイスのポインタ

public:
	CEnumString(LPUNKNOWN pUnkRef, ULONG cstr, LPOLESTR* prgstr, IMalloc* pmem)
	{
		// pUnkRef	参照カウントを増やすLPUNKNOWN
		// cstr    prgstrに含まれる文字列の数
		// prgstr	オートコンプリートに使う文字列の配列
		// pmem    返す文字列のメモリを確保するために使うIMallocインターフェイスのポインタ

		m_cRef = 0;
		m_pUnkRef = pUnkRef;

		m_iCur = 0;
		m_cstr = cstr;
		m_prgstr = new LPOLESTR[cstr];
		m_pmem = pmem;

		if (m_prgstr) {
			for (UINT i=0; i < m_cstr; i++) {
				// メモリを確保してコピー
				m_prgstr[i] = new WCHAR[wcslen(prgstr[i]) + 1];
				wcscpy(m_prgstr[i], prgstr[i]);
			}
		}
		return;
	}

    ~CEnumString()
	{
		if (m_prgstr) {
			// コピーした文字列のバッファを解放する
			for (UINT i=0; i < m_cstr; i++) {
				delete [] m_prgstr[i];
			}
			delete [] m_prgstr;
		}
		return;
	}

	// IUnknownインターフェイスのメソッド
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv)
	{
		*ppv = NULL;
		if (IID_IUnknown == riid || IID_IEnumString == riid)
			*ppv = (LPVOID)this;
		if (*ppv) {
			((LPUNKNOWN)*ppv)->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}


	STDMETHODIMP_(ULONG) AddRef(void)
	{
		// このクラスの参照カウントをインクリメントする
		// 親クラスもあればそれもインクリメント
		++m_cRef;
		if(m_pUnkRef) 
			m_pUnkRef->AddRef();
		return m_cRef;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		// このオブジェクトを解放
		// 親があればそれも解放
		if (m_pUnkRef) 
			m_pUnkRef->Release();

		if (0L != --m_cRef)
			return m_cRef;

		delete this;
		return 0;
	}

	// IEnumStringインターフェイスのメソッド
	STDMETHODIMP Next(ULONG cstr, LPOLESTR *pstr, ULONG *pcstr)
	{
		/*
		 * 引数:
		 *  cstr    ULONG      pstrに返す文字列の個数
		 *  pstr    LPOLESTR*  戻り値を入れるポインタ
		 *  pulstr  ULONG*     実際に返した数を入れる変数へのポインタ
		 */
		ULONG cReturn = 0L;
		ULONG maxcount = cstr;

		*pcstr = 0L;	// ←戻り値を初期化
		*pstr = NULL;	// ←

		// このクラスが文字列を持ってなかったら失敗
		if (!m_prgstr)
			return S_FALSE;

		// pcstrがNULLだったら、１個のアイテムを取得時以外は失敗
		if (!pcstr) {
			if (cstr != 1L)
				return E_POINTER;
		}

		// リストの最後だったら失敗
		if (m_iCur >= m_cstr)
			return S_FALSE;

		// 要求した数まで返せるだけ返す
		while (m_iCur < m_cstr && cstr > 0) {
			int	size;

			// コピーする文字列の文字数を計算
			size = (wcslen(m_prgstr[m_iCur])+1);

			// メモリ確保
			pstr[cReturn] = (WCHAR*)m_pmem->Alloc(size * sizeof(WCHAR));
			if (pstr[cReturn])
				wcscpy(pstr[cReturn], m_prgstr[m_iCur]);

			// 次のアイテムに移る
			m_iCur++;
			cReturn++;
			cstr--;
		}

		if (pcstr)
			*pcstr = cReturn;

		if (cReturn == maxcount)
			return S_OK;
		return S_FALSE;
	}

	STDMETHODIMP Skip(ULONG cSkip)
	{
		/*
		 * 目的 -> アイテムをn個飛ばす
		 *
		 * 引数:
		 *  cSkip  ULONG  飛ばす個数
		 *
		 * 戻り値:
		 *  HRESULT	 成功時                        S_OK
		 *			 要求した数だけとばせないとき  S_FALSE
		 */
		if (((m_iCur + cSkip) >= m_cstr) || NULL == m_prgstr)
			return S_FALSE;

		m_iCur += cSkip;
		return S_OK;
	}

	STDMETHODIMP Reset(void)
	{
		// 現在位置を最初に戻す
		m_iCur = 0;
		return S_OK;
	}

	STDMETHODIMP Clone(LPENUMSTRING *ppEnum)
	{
		/*
		 * 目的 -> これと同じ状態のIEnumStringを作る
		 *
		 * 引数:
		 *  ppEnum LPENUMSTRING* 作った新しいオブジェクトのポインタを受け取る変数
		 */
		*ppEnum = NULL;

		// 同じのを作る
		CEnumString* pNew = new CEnumString(m_pUnkRef, m_cstr, m_prgstr, m_pmem);

		if (!pNew)
			return E_OUTOFMEMORY;

		// 参照カウントを増やす
		pNew->AddRef();

		// 位置をそろえる
		pNew->m_iCur = m_iCur;

		*ppEnum = pNew;
		return S_OK;
	}
};



#endif		// __ENUMSTRING_H__

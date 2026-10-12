////////////////////////////////////////////////////////////////////////
// storage.h

#pragma once
#ifndef __STORAGE_H__
#define __STORAGE_H__


//=========================================================================
//	基本型専用の可変長配列
//
//	非常に簡素な作りです。アクセスは好きなところへ自由に可能ですが、
//	要素の追加・削除は末尾に対してのみ。ビット毎のコピーだとか、
//	色々エキセントリックな作業をしてますので、そういうことをして良い
//	型以外には使わないでください。
//=========================================================================

template <typename T>
class CStorage
{
public:

	//	コンストラクタ
	//
	//	@param alloc_size
	//		最初に確保する"メモリの"サイズ。
	//		"配列の"サイズではないことに注意。
	explicit CStorage(ULONG allocSize = 20)
		: m_alen(max(allocSize, 1UL))
		, m_len ( 0 )
		, m_data (static_cast<T*>(malloc(m_alen * sizeof(T))) )
		{}

	~CStorage()
		{ free(m_data); }

	// 末尾に要素を追加
	void Add( const T& obj )
		{
			if( m_len >= m_alen )
				ReAllocate( m_alen<<2 );
			m_data[ m_len++ ] = obj;
		}

	//	配列サイズを強制変更
	//
	//	縮小/拡大のどちらも可能。コンストラクタと違い、
	//	指定した値に基づき最大indexが変化します。
	//	@param new_size 新しいサイズ。
	void ForceSize( ULONG newSize )
		{
			if( newSize > m_alen )
				ReAllocate( newSize );
			m_len = newSize;
		}

public:

	// 要素数
	ULONG size() const
		{ return m_len; }

	// 要素取得
	T& operator[]( size_t i )
		{ return m_data[i]; }

	// 要素取得(const)
	const T& operator[]( size_t i ) const
		{ return m_data[i]; }

	// 配列先頭のポインタを返す
	const T* head() const
		{ return m_data; }

private:
	void ReAllocate( ULONG siz )
		{
			ULONG p = m_alen * sizeof(T);
			m_alen = siz;
			T* newbuf = static_cast<T*>(malloc(m_alen * sizeof(T)));
			memmove(newbuf, m_data, m_len * sizeof(T));
			free(m_data);
			m_data = newbuf;
		}

private:
	ULONG m_alen;
	ULONG m_len;
	T*    m_data;
};




#endif		// __STORAGE_H__

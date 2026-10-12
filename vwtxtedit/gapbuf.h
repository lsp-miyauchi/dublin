////////////////////////////////////////////////////////////////////////
// gapbuf.h

#pragma once
#ifndef __GAPBUF_H__
#define __GAPBUF_H__


////////////////////////////////////////////////////////////////////////
//	基本型専用ギャップバッファ
//
//	小耳に挟んだギャップバッファというもの。
//	ものすごい勘違いをして別物ができているかもしれませんが、
//	細かいことはあまり気にしないでください。
//	配列のようにランダムアクセス可能で、
//	同一箇所への連続した挿入/削除が速いというデータ構造です。
//	( 下の図で言うと、gap_startへの挿入/削除にはデータの移動が必要ない )
//	<pre>
//@@  D  <--0
//@@  D
//@@  |  <--gap_start
//@@  |
//@@  D  <--gap_end
//@@  D
//@@     <--array_end
//	</pre>
//	メモリイメージをそのままコピーしてる実装なので、
//	プリミティブ型以外では絶対に使わないこと。
////////////////////////////////////////////////////////////////////////

template<class T>
class CGapBufT
{
public:

	//	コンストラクタ
	//	@param alloc_size
	//		最初に確保する"メモリの"サイズ。
	//		"配列の"サイズではないことに注意。
	explicit CGapBufT( ULONG alloc_size=40 )
		: m_alen( Max(alloc_size, 10UL) )
		, m_gs  ( 0 )
		, m_ge  ( m_alen )
		, m_data ( new T[m_alen] )
	{}

	~CGapBufT()
	{
		delete [] m_data;
	}

	// 要素挿入
	void InsertAt( ULONG i, const T& x )
	{
		MakeGapAt( i );
		m_data[m_gs++] = x;

		if (m_gs == m_ge)
			Reallocate(m_alen << 1);
		}

	// 要素挿入(複数)
	void InsertAt( ULONG i, const T* x, ULONG len )
	{
		MakeGapAt( size() );
		MakeGapAt( i );
		if (m_ge - m_gs <= len)
			Reallocate( Max(m_alen+len+1, m_alen<<1) );
		memmove( m_data+m_gs, x, len*sizeof(T) );
		m_gs += len;
	}

	// 末尾に要素を追加
	void Add( const T& x )
	{
		InsertAt(size(), x);
	}

	// 末尾に要素を追加(複数)
	void Add( const T* x, ULONG len )
	{
		InsertAt( size(), x, len );
	}

	// 要素削除
	void RemoveAt(ULONG i, ULONG len = 1)
	{
		if (i <= m_gs && m_gs <= i + len) {
				// この場合はメモリ移動の必要がない
				// まず前半を削除
				len -= (m_gs - i);
				m_gs = i;
			}
			else {
				MakeGapAt( i );
			}

			// 後半を削除
			m_ge += len;
		}

	// 要素削除(全部)
	void RemoveAll()
	{
		RemoveAt( 0, size() );
	}

	// 要素削除(指定index以降全部)
	void RemoveToTail( ULONG i )
	{
		RemoveAt( i, size() - i );
	}

	// 要素コピー(指定index以降全部)
	ULONG CopyToTail( ULONG i, T* x )
	{
		return CopyAt( i, size()-i, x );
	}

	// 要素コピー
	ULONG CopyAt( ULONG i, ULONG len, T* x )
	{
		ULONG copyed = 0;
		if (i < m_gs) {
			// 前半
			copyed += Min( len, m_gs-i );
			memmove( x, m_data+i, copyed*sizeof(T) );
			x   += copyed;
			len -= copyed;
			i   += copyed;
		}

		// 後半
		memmove(x, m_data+(i - m_gs) + m_ge, len * sizeof(T) );
		return copyed + len;
	}

public:

	// 要素数
	ULONG size() const
	{
		return m_alen - (m_ge - m_gs);
	}

	//@{ 要素取得 //@}
	T& operator[]( ULONG i )
	{
		return m_data[ ( i < m_gs ) ? i : i + (m_ge - m_gs) ];
	}

	//@{ 要素取得(const) //@}
	const T& operator[]( ULONG i ) const
	{
		return m_data[ ( i < m_gs ) ? i : i + (m_ge - m_gs) ];
	}

protected:

	ULONG m_alen;
	ULONG m_gs;
	ULONG m_ge;
	T*    m_data;

protected:

	void MakeGapAt( ULONG i )
	{
		if (i < m_gs) {
			m_ge -= (m_gs - i);
			memmove( m_data+m_ge, m_data+i, (m_gs-i)*sizeof(T) );
		}
		else if (i > m_gs) {
			int j = i+(m_ge-m_gs);
			memmove( m_data+m_gs, m_data+m_ge, (j-m_ge)*sizeof(T) );
			m_ge = j;
		}
		m_gs = i;
	}

	void Reallocate( ULONG newalen )
	{
		T *tmp = new T[newalen], *old=m_data;
		const ULONG tail = m_alen-m_ge;

		memmove( tmp, old, m_gs*sizeof(T) );
		memmove( tmp+newalen-tail, old+m_ge, tail*sizeof(T) );
		delete [] old;

		m_data  = tmp;
		m_ge   = newalen-tail;
		m_alen = newalen;
	}
};

//=========================================================================
//@{
//	CGapBufT + smartptr のふり
//
//	要素削除時にdeleteを実行しっちゃったりするバージョン。
//	任意オブジェクトをギャップバッファで使いたいときは
//	これでてきとーに代用すべし。
//@}
//=========================================================================

template<class T>
class CGapBufObj : public CGapBufT<T*>
{
public:

	explicit CGapBufObj( ULONG alloc_size = 40 )
		: CGapBufT<T*>( alloc_size )
	{
	}

	void RemoveAt( ULONG i, ULONG len=1 )
	{
		ULONG& m_gs = CGapBufT<T*>::m_gs;
		ULONG& m_ge = CGapBufT<T*>::m_ge;
		T**&   m_data= CGapBufT<T*>::m_data;

		if (i <= m_gs && m_gs <= i + len) {
			// 前半を削除
			for (ULONG j = i, ed = m_gs; j < ed; ++j)
				delete m_data[j];

			len -= (m_gs-i);
			m_gs  = i;
		}
		else {
			CGapBufT<T*>::MakeGapAt( i );
		}

		// 後半を削除
		for( ULONG j=m_ge, ed=m_ge+len; j<ed; ++j )
			delete m_data[j];
		m_ge = m_ge+len;
	}

	~CGapBufObj()
	{ RemoveAt( 0, CGapBufT<T*>::size() ); }

	void RemoveAll( ULONG i )
	{ RemoveAt( 0, CGapBufT<T*>::size() ); }

	void RemoveToTail( ULONG i )
	{ RemoveAt( i, CGapBufT<T*>::size()-i ); }

public:

	T& operator[]( ULONG i )
	{ return *CGapBufT<T*>::operator[](i); }

	const T& operator[]( ULONG i ) const
	{ return *CGapBufT<T*>::operator[](i); }
};



#endif		// __GAPBUF_H__

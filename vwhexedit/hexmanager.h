#ifndef __HEXMANAGER_H__
#define __HEXMANAGER_H__

#define WARNINGSIZE		0x800000
#define LOCAL_MEMORY

#ifndef LOCAL_MEMORY
	#define ALLOC(size)			(GlobalAllocPtr(GMEM_MOVEABLE,(size)))
	#define REALLOC(p, size)	(GlobalReAllocPtr((p),(size),GMEM_MOVEABLE))
	#define FREE(p)				(GlobalFreePtr((p)))
#else
	#define ALLOC(size)			(malloc((size)))
	#define REALLOC(p, size)	(realloc((p),(size)))
	#define FREE(p)				(free((p)))
#endif

typedef struct tagColorInfo {
	COLORREF rgbText;
	COLORREF rgbAddr;
	COLORREF rgbBack;
	COLORREF rgbLine;
	COLORREF rgbMarkText;
	COLORREF rgbMarkBack;
} VIEWCOLOR, *LPVIEWCOLOR;


#define ROWHEAD(a)			((a)&~0xf)
#define ROWTAIL(a)			((a)|0xf)
#define ISROWHEAD(a)		(((a)&0xf)==0)
#define ISROWTAIL(a)		(((a)&0xf)==0xf)
#define COL(a)				((a)&0xf)
#define ROW(a)				((a)>>4)
#define POS2ROW(a)			(((a)&~0xf)>>4)
#define NEXTROW(a)			((a)+16)

#define SLB_PREV			-1
#define SLB_NONE			0
#define SLB_NEXT			1

enum{
	SST_HIBYTE = 1,
	SST_LOBYTE = 2,
	SST_TEXT = 3,
};

enum{
	UNDO_INSERT,
	UNDO_OVERWRITE,
	UNDO_DELETE,
};

void* _memdup(const void* pv, size_t len);

////////////////////////////////////////////////////////////////////////
//


class CCriticalSection
{
private:
	CRITICAL_SECTION m_c;
public:
	CCriticalSection() { InitializeCriticalSection(&m_c); }
	~CCriticalSection() { DeleteCriticalSection(&m_c); }
	void Enter() { EnterCriticalSection(&m_c); }
	void Leave() { LeaveCriticalSection(&m_c); }
};

////////////////////////////////////////////////////////////////////////
//

class CUndo
{
private:
	int m_iType;
	UINT_PTR m_pos;
	BYTE* m_pData;
	size_t m_size;

public:
	CUndo(int t = 0, UINT_PTR p = 0, BYTE* d = NULL, size_t s = 0)
		: m_iType(t)
		, m_pos(p)
		, m_pData(d)
		, m_size(s)
	{
	}

	~CUndo()
	{
		if (m_pData) {
			FREE(m_pData);
			m_pData = NULL;
		}
	}

	inline int type() { return m_iType; }
	inline UINT_PTR position() { return m_pos; }
	inline BYTE* data() { return m_pData; }
	inline size_t size() { return m_size; }
};


enum {
	CHAR_ASCII,
	CHAR_SJIS,
	CHAR_JIS,
	CHAR_UNICODE,
	CHAR_EUC
};


////////////////////////////////////////////////////////////////////////
//


class CHexManager
{
public:
	CHexManager();
	~CHexManager();

	void ClearData();
	void Attach(BYTE* pData, size_t size);
	BOOL Load(LPCTSTR lpszPathName);
	BOOL Save(LPCTSTR lpszPathName);
	BOOL RenewMemory();
	void Select(UINT_PTR posSelFrom, UINT_PTR posSelTo);
	UINT_PTR MoveTo(UINT_PTR posMoveTo, UINT posType);
	UINT_PTR MoveToLeft(bool bPrevByte = false);
	UINT_PTR MoveToRight(bool bNextByte = false);
	UINT_PTR MoveToUp(bool bDoubleLine = false);
	UINT_PTR MoveToDown(bool bDoubleLine = false);
	void UpdateSelRange(UINT_PTR posNew);
	__inline CRange GetSelRange() const { return CRange(m_posSelFrom, m_posSelTo); }
	__inline UINT_PTR GetSelFrom() { return m_posSelFrom; }
	__inline UINT_PTR GetSelTo() { return m_posSelTo; }

	__inline bool IsReadOnly() { return m_bReadOnly; }
	__inline bool IsSelected() { return m_bSelected; }
	__inline void SetReadOnly(bool bReadOnly) { m_bReadOnly = bReadOnly; }

	// posInsertPointで指定された場所にpBufを挿入
	BOOL Insert(UINT_PTR posInsertPoint, const void* pBuf, size_t nBufSize);
	BOOL LoadAppend(const void* pData, size_t size);

	//dwOverPointで指定された場所からpBufを上書き
	BOOL Write(UINT_PTR dwOverPoint, const void* pBuf, size_t dwBufSize);
	BOOL CreateClone(UINT_PTR dwStartPoint, size_t dwVisibleSize);	// クローンの作成
	//データの取得（ここですべて管理する）
	BOOL Read(UINT_PTR dwOffset, size_t dwSize, LPBYTE pByte, size_t* pNumRead);
	LPBYTE AllocRead(UINT_PTR dwPos, size_t size, size_t* pNumRead);
	__inline size_t GetTotalSize() { return m_nTotalSize; }
	__inline size_t GetTotalLines() { return m_nTotalLine; }
	BOOL Delete(UINT_PTR posDelPoint, size_t nDelSize);
	BOOL DeleteSelectedRange();
	LPBYTE CutSelectedRange(size_t* pnCut);
	LPBYTE ReadSelectedRange(size_t* pnRead);

	BOOL FindPrev(UINT_PTR posStart, LPCTSTR pStr, UINT_PTR* pFoundPos, BOOL bM);
	BOOL FindNext(UINT_PTR posStart, LPCTSTR pStr, UINT_PTR* pFoundPos, BOOL bM);
	// バイトデータの検索
	BOOL FindPrev(UINT_PTR posStart, BYTE* pFind, DWORD dwLen, UINT_PTR* pFoundPos);
	BOOL FindNext(UINT_PTR posStart, BYTE* pFind, DWORD dwLen, UINT_PTR* pFoundPos);

	__inline bool CanUndo() { return (!m_bReadOnly && GetUndoCount()); }	// アンドゥの可否
	__inline bool CanCopy() { return m_bSelected; } 						// コピーの可否
	__inline bool CanCut() { return (!m_bReadOnly && m_bSelected); }		// カットの可否
	__inline bool IsModified() { return (m_nModified > 0); }				// 編集済みか否か
	__inline DWORD GetUndoCount() { return (DWORD)m_Undo.GetSize(); }

	BOOL DoUndo();				// 一つ前に戻すまたは直す
	void SweepUndo();			// アンドゥメモリを廃棄

	BOOL SaveUndo(int type, UINT_PTR pos, BYTE* pData, size_t nDataSize);

	__inline UINT_PTR GetCurAddr() { return m_posCur; }
	__inline UINT_PTR GetCurLineAddr() { return ROWHEAD(m_posCur); }
	__inline UINT_PTR GetLineAddr(UINT_PTR pos) { return ROWHEAD(pos); }
	__inline UINT_PTR GetEndAddr() { return (UINT_PTR)(m_nTotalSize - 1); }
	UINT_PTR GetLastEditPos();

public:
	UINT m_posType;				// HIバイト=SST_HIBYTE, LOバイト=SST_LOBYTE, テキスト=SST_TEXT
	int m_nModified;			// 修正変更回数

protected:
	UINT_PTR m_posSelFrom;		// 選択開始位置：開始位置は終了位置より後ろになることもある
	UINT_PTR m_posSelTo;		// 選択終了位置
	UINT_PTR m_posCur;			// 現在のカーソル位置
	size_t m_nTotalSize;		// データサイズ
	size_t m_nTotalLine;		// 総行数
	bool m_bReadOnly;			// 編集禁止モード
	bool m_bSelected;			// 範囲選択の有無

	BYTE* m_pEdit;
	BYTE* m_pClone;
	UINT_PTR m_posClone;
	size_t m_nOldCloneSize;
	size_t m_nNewCloneSize;
	CSimpleArray<CUndo*> m_Undo;

	void _UpdateTotalSize(size_t nTotalSize);
};


#endif

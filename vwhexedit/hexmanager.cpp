#include "stdafx.h"
#include "hexmanager.h"
#include "resource.h"

#define CLONESIZE 100 * 1024
#define MAX_UNDO  0x18000

int LastError(LPCTSTR, UINT);


void* _memdup(const void* pv, size_t len)
{
	void* pvn = NULL;
	if (pv && len) {
		pvn = ALLOC(len);
		memcpy(pvn, pv, len);
	}
	return pvn;
}

CHexManager::CHexManager()
{
	m_nModified = 0;
	m_pEdit = NULL;
	m_pClone = NULL;
	m_posClone = 0;
	m_nOldCloneSize = m_nNewCloneSize = 0;

	_UpdateTotalSize(0);
	m_posType = SST_HIBYTE;
	m_posSelFrom = 0;
	m_posSelTo = 0;
	m_bSelected = FALSE;
	m_posCur = 0;

	// 一つ、サイズを作っておく
	m_pEdit = (BYTE*)ALLOC(1);
	*m_pEdit = 0;
	_UpdateTotalSize(1);
	m_bReadOnly = FALSE;
}

CHexManager::~CHexManager()
{
	if (m_pEdit) {
		FREE(m_pEdit);
		m_pEdit = NULL;
	}

	if (m_pClone) {
		FREE(m_pClone);
		m_pClone = NULL;
	}

	// Undoデータ削除
	SweepUndo();
}

void CHexManager::ClearData()
{
	if (m_pEdit) {
		FREE(m_pEdit);
		m_pEdit = NULL;
	}

	if (m_pClone) {
		FREE(m_pClone);
		m_pClone = NULL;
	}

	SweepUndo();

	m_posClone = 0;
	m_nOldCloneSize = m_nNewCloneSize = 0;

	_UpdateTotalSize(0);
	m_posType = SST_HIBYTE;
	m_posSelFrom = 0;
	m_posSelTo = 0;
	m_posCur = 0;

	// 一つ、サイズを作っておく
	m_pEdit = (BYTE*)ALLOC(1);
	*m_pEdit = 0;
	_UpdateTotalSize(1);

	m_nModified = 0;
	m_bReadOnly = FALSE;
	m_bSelected = FALSE;
}

void CHexManager::SweepUndo()
{
	// Undoデータ削除
	CUndo* undo;
	int i, cnt = m_Undo.GetSize();
	for (i = 0; i < cnt; i++) {
		undo = m_Undo[i];
		if (undo) delete undo;
	}
	m_Undo.RemoveAll();
	m_nModified = 0;
}

void CHexManager::Attach(BYTE* dataptr, size_t size)
{
	ClearData();	// 今までのデータをすべて削除
	if (dataptr) {
		if (m_pEdit) FREE(m_pEdit);

		m_pEdit = dataptr;
		_UpdateTotalSize(size + 1);
	}
}

BOOL CHexManager::Load(LPCTSTR pszName)
{
	//ファイルをオープン
	HANDLE hf = CreateFile(pszName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return FALSE;

	// ファイルのサイズを取得
	DWORD dwFileSize = GetFileSize(hf, &dwFileSize);
	BYTE* dataptr = (BYTE*)ALLOC(dwFileSize + 1);
	if (!dataptr) {
		CloseHandle(hf);
		return FALSE;
	}

	DWORD dw;
	BOOL bOK = ReadFile(hf, dataptr, dwFileSize, &dw, NULL);
	CloseHandle(hf);

	if (!bOK || dw != dwFileSize) {
		FREE(dataptr);
		return FALSE;
	}

	ClearData();	// 今までのデータをすべて削除
	if (m_pEdit)
		FREE(m_pEdit);
	m_pEdit = dataptr;
	_UpdateTotalSize(dwFileSize + 1);
	return TRUE;
}

BOOL CHexManager::Save(LPCTSTR pszName)
{
	// ファイルをオープン
	HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf == INVALID_HANDLE_VALUE) {
		LastError(PathFindFileName(pszName), MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	// データを更新
	RenewMemory();		//クローンのデータをすべて書き込む

	BOOL bOK = FALSE;
	if (m_pEdit) {
		DWORD dw;
		DWORD writesize = (DWORD)(m_nTotalSize - 1);
		if (WriteFile(hf, m_pEdit, writesize, &dw, NULL) && dw == writesize) {
			SetEndOfFile(hf);
			SweepUndo();
			bOK = TRUE;
		}
	}
	CloseHandle(hf);
	return bOK;
}

BOOL CHexManager::RenewMemory()
{
	if (!m_pClone) return TRUE;

	// クローンデータが残っているならそれを書き込む
	UINT_PTR posOld = m_posClone + m_nOldCloneSize;
	UINT_PTR posNew = m_posClone + m_nNewCloneSize;

	// 総サイズもクローンのデータが増減されると一緒に変化している
	size_t nSize = m_nTotalSize;

	size_t nMove;
	BYTE* pData;
	if (m_nOldCloneSize < m_nNewCloneSize) {
		// データが増えた
		nMove = m_nTotalSize - posOld;
		nSize += m_nNewCloneSize - m_nOldCloneSize;

		// 容量を増やす
		pData = (BYTE*)REALLOC(m_pEdit, nSize);
		if (!pData)
			return FALSE;
		m_pEdit = pData;

		// メモリの移動
		memmove(m_pEdit + posNew, m_pEdit + posOld, nMove);

		// クローンのデータをメモリに書き込む
		memcpy(m_pEdit + m_posClone, m_pClone, m_nNewCloneSize);
	}
	else if (m_nOldCloneSize > m_nNewCloneSize) {
		// 減った
		nMove = m_nTotalSize - posNew;

		// 減った分メモリを移動
		memmove(m_pEdit + posNew, m_pEdit + posOld, nMove);

		pData = (BYTE*)REALLOC(m_pEdit, nSize);
		if (!pData)
			return FALSE;
		m_pEdit = pData;

		// クローンのデータを書き込む
		memcpy(m_pEdit + m_posClone, m_pClone, m_nNewCloneSize);
	}
	else {
		// 同じサイズ
		memcpy(m_pEdit + m_posClone, m_pClone, m_nOldCloneSize);
	}
	FREE(m_pClone);
	m_pClone = NULL;
	m_nNewCloneSize = m_nOldCloneSize = m_posClone = 0;
	return TRUE;
}

void CHexManager::Select(UINT_PTR posFrom, UINT_PTR posTo)
{
	if (posFrom > m_nTotalSize || posTo > m_nTotalSize)
		return;

	if (posFrom < posTo) {
		if (posTo >= m_nTotalSize - 1)
			posTo = m_nTotalSize - 2;
		if (m_posType != SST_TEXT)
			m_posType = SST_LOBYTE;
	}
	else if (posFrom > posTo) {
		if (posFrom >= m_nTotalSize - 1)
			posFrom = m_nTotalSize - 2;
		if (m_posType != SST_TEXT)
			m_posType = SST_HIBYTE;
	}

	m_posSelFrom = posFrom;
	m_posSelTo = posTo;
	m_bSelected = TRUE;

	// 現在行
	m_posCur = m_posSelTo;
}

UINT_PTR CHexManager::MoveTo(UINT_PTR posMoveTo, UINT posType)
{
	// 現在のポイント位置を指定位置に移動
	m_posCur = m_posSelFrom = m_posSelTo = posMoveTo;
	m_posType = posType;
	m_bSelected = FALSE;
	return m_posCur;
}

UINT_PTR CHexManager::MoveToLeft(bool bPrevByte)
{
	UINT_PTR posNew = m_posCur;
	DWORD posType = m_posType;
	switch (posType) {
	case SST_LOBYTE:
		posType = SST_HIBYTE;
		break;
	case SST_HIBYTE:
		if (posNew > 0) {
			posNew--;
			if (!bPrevByte)
				posType = SST_LOBYTE;
		}
		break;
	case SST_TEXT:
		if (posNew > 0)
			posNew--;
		break;
	}
	return MoveTo(posNew, posType);
}

UINT_PTR CHexManager::MoveToRight(bool bNextByte)
{
	UINT_PTR posNew = m_posCur;
	DWORD posType = m_posType;
	switch (posType) {
	case SST_HIBYTE:
		if (bNextByte) {
			if (posNew < m_nTotalSize - 1)
				posNew++;
		}
		else
			posType = SST_LOBYTE;
		break;
	case SST_LOBYTE:
		if (posNew < m_nTotalSize - 1) {
			posNew++;
			posType = SST_HIBYTE;
		}
		break;
	case SST_TEXT:
		if (posNew < m_nTotalSize - 1)
			posNew++;
		break;
	}
	return MoveTo(posNew, posType);
}

UINT_PTR CHexManager::MoveToUp(bool bDoubleLine)
{
	if (bDoubleLine) {
		if (m_posCur > 31)
			return MoveTo(m_posCur - 32, m_posType);
	}
	return (m_posCur > 15) ? MoveTo(m_posCur - 16, m_posType) : m_posCur;
}

UINT_PTR CHexManager::MoveToDown(bool bDoubleLine)
{
	if (bDoubleLine) {
		if (m_posCur < m_nTotalSize - 32)
			return MoveTo(m_posCur + 32, m_posType);
	}
	return (m_posCur < m_nTotalSize - 16) ? MoveTo(m_posCur + 16, m_posType) : m_posCur;
}

void CHexManager::UpdateSelRange(UINT_PTR posNewTo)
{
	if (m_nTotalSize > 1 && posNewTo >= m_nTotalSize - 1)
		posNewTo = m_nTotalSize - 2;
	Select(m_posSelFrom, posNewTo);
}

BOOL CHexManager::Insert(UINT_PTR posInsert, const void* pData, size_t size)
{
	if (m_bReadOnly) return FALSE;

	if (!pData || size < 1) return TRUE;

	// クローンがある場合はクローンの中身を変更
	// クローンが無い場合は元のデータを変更

	// クローンのポインタより前に挿入する場合
	// クローンを新たに作成
	if (!m_pClone ||
		posInsert < m_posClone ||
		posInsert >= m_posClone + m_nNewCloneSize)
	{
		if (!CreateClone(posInsert, size))
			return FALSE;
	}

	// クローンの容量を増やす
	LPBYTE pClone = (LPBYTE)REALLOC(m_pClone, m_nNewCloneSize + size);
	if (!pClone) return FALSE;

	m_pClone = pClone;

	// メモリ移動
	size_t nMoveSize = (m_posClone + m_nNewCloneSize) - posInsert;
	memmove(m_pClone + (posInsert - m_posClone) + size,  m_pClone + (posInsert - m_posClone), nMoveSize);

	// メモリコピー
	memcpy(m_pClone + (posInsert - m_posClone), pData, size);

	m_nNewCloneSize += size;
	_UpdateTotalSize(m_nTotalSize + size);
	return TRUE;
}

BOOL CHexManager::LoadAppend(const void* dataptr, size_t size)
{
	if (!dataptr || size < 1) return TRUE;

	BYTE* pb = NULL;
	if (m_nTotalSize <= 1) {
		pb = (BYTE*)ALLOC(size + 1);
		if (pb) {
			if (m_pEdit) FREE(m_pEdit);

			memcpy(pb, dataptr, size);
			m_pEdit = pb;
			_UpdateTotalSize(size + 1);
		}
	}
	else {
		pb = (BYTE*)REALLOC(m_pEdit, m_nTotalSize + size);
		if (pb) {
			memcpy(pb + m_nTotalSize - 1, dataptr, size);
			m_pEdit = pb;
			_UpdateTotalSize(m_nTotalSize + size);
		}
	}
	return (pb != NULL);
}

BOOL CHexManager::Write(UINT_PTR posOffset, const void* pBuffer, size_t nNumWrite)
{
	if (IsReadOnly()) return FALSE;

	if (!m_nTotalSize || !pBuffer || !nNumWrite || m_nTotalSize <= posOffset)
		return FALSE;

	if (!m_pClone)
		CreateClone(posOffset, nNumWrite);

	UINT_PTR pos = posOffset;
	const BYTE* pb = (const BYTE*)pBuffer;
	while (nNumWrite) {
		size_t size;

		// クローン内に更新する場所があるか？
		if (m_pClone &&
			pos >= m_posClone &&
			pos < m_posClone + m_nNewCloneSize)
		{
			// クローン内のデータを更新
			if (m_nNewCloneSize < (posOffset - m_posClone + nNumWrite))
				size = (posOffset - m_posClone + nNumWrite) - m_nNewCloneSize;
			else
				size = 0;
			memcpy(m_pClone + (pos - m_posClone), pb, nNumWrite - size);

			pos += (nNumWrite - size);
			pb += (nNumWrite - size);
			nNumWrite = size;
		}
		else {
			if (!CreateClone(pos, nNumWrite))
				return FALSE;

			// クローンが小さい場合
			if (pos + nNumWrite > m_posClone + m_nNewCloneSize) {
				size = (pos + nNumWrite) - (m_posClone + m_nNewCloneSize);
				LPBYTE pTemp = (LPBYTE)REALLOC(m_pClone, size + m_nNewCloneSize);
				if (pTemp) {
					m_pClone = pTemp;
					m_nNewCloneSize += size;
					_UpdateTotalSize(m_nTotalSize + size);
				}
			}
		}
	}
	return TRUE;
}

BOOL CHexManager::Delete(UINT_PTR posStart, size_t nDelSize)
{
	if (IsReadOnly()) return FALSE;
	if (posStart >= m_nTotalSize) return TRUE;

	// 削除するデータがクローン内に無い場合
	if (!m_pClone ||
		posStart < m_posClone ||
		(m_posClone + m_nNewCloneSize) <= posStart)
	{
		// クローンを作成
		if (!CreateClone(posStart, (nDelSize | 0xf) + 17))
			return FALSE;
	}

	size_t delta = posStart - m_posClone;
	while (nDelSize) {
		LPBYTE pTemp;

		// クローンをはみ出す場合
		// クローンのサイズを小さくする
		if (delta + nDelSize >= m_nNewCloneSize) {
			if (delta) {
				pTemp = (LPBYTE)REALLOC(m_pClone, delta);
				if (!pTemp) return FALSE;

				m_pClone = pTemp;
			}
			else {
				FREE(m_pClone);
				m_pClone = NULL;
			}
			_UpdateTotalSize(m_nTotalSize - (m_nNewCloneSize - delta));
			nDelSize -= m_nNewCloneSize - delta;
			m_nNewCloneSize = delta;
		}
		else {
			memmove(m_pClone + delta, m_pClone + delta + nDelSize, m_nNewCloneSize - (delta + nDelSize));

			pTemp = (LPBYTE)REALLOC(m_pClone, m_nNewCloneSize - nDelSize);
			if (!pTemp) return FALSE;

			m_pClone = pTemp;
			m_nNewCloneSize -= nDelSize;
			_UpdateTotalSize(m_nTotalSize - nDelSize);
			nDelSize = 0;
		}
	}
	return TRUE;
}

BOOL CHexManager::DeleteSelectedRange()
{
	CRange sel = GetSelRange();
	sel.Normalize();
	return Delete(sel.from, sel.Range() + 1);
}

LPBYTE CHexManager::CutSelectedRange(size_t* pnCut)
{
	CRange sel = GetSelRange();
	sel.Normalize();

	LPBYTE pc = AllocRead(sel.from, sel.Range() + 1, pnCut);
	if (pc) {
		if (Delete(sel.from, sel.Range() + 1))
			return pc;
		FREE(pc);
	}
	return NULL;
}

LPBYTE CHexManager::ReadSelectedRange(size_t* pnRead)
{
	CRange sel = GetSelRange();
	sel.Normalize();
	return AllocRead(sel.from, sel.Range() + 1, pnRead);
}

BOOL CHexManager::FindPrev(UINT_PTR posStart, LPCTSTR pszFind, UINT_PTR* pFoundPos, BOOL bCase)
{
	// データを更新
	if (m_pClone) RenewMemory();

	char sz[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pszFind, -1, sz, ARRAYSIZE(sz), NULL, NULL);
	int len = lstrlenA(sz);
	if (!m_pEdit || m_nTotalSize < 1 || len < 1 || (size_t)len >= m_nTotalSize) return FALSE;

	int (WINAPI* pfn)(LPCSTR, LPCSTR, int);
	pfn = bCase ? StrCmpNA : StrCmpNIA;

	INT_PTR pos = (INT_PTR)min(posStart, m_nTotalSize - len);
	while (pos >= 0) {
		if (pfn((LPCSTR)(m_pEdit + pos), sz, len) == 0) {
			*pFoundPos = pos;
			return TRUE;
		}
		pos--;
	}
	return FALSE;
}

BOOL CHexManager::FindPrev(UINT_PTR posStart, LPBYTE pcbFind, DWORD dwLen, UINT_PTR* pFoundPos)
{
	// データを更新
	if (m_pClone) RenewMemory();

	if (!m_pEdit || m_nTotalSize < 1 || !dwLen || dwLen >= m_nTotalSize) return FALSE;

	INT_PTR pos = (INT_PTR)min(posStart, m_nTotalSize - dwLen);
	while (pos >= 0) {
		if (memcmp(m_pEdit + pos, pcbFind, dwLen) == 0) {
			*pFoundPos = pos;
			return TRUE;
		}
		pos--;
	}
	return FALSE;
}

BOOL CHexManager::FindNext(UINT_PTR posStart, LPCTSTR pszFind, UINT_PTR* pFoundPos, BOOL bCase)
{
	if (!pszFind || *pszFind == 0) return FALSE;

	// データを更新
	if (m_pClone) RenewMemory();

	char sz[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pszFind, -1, sz, ARRAYSIZE(sz), NULL, NULL);
	int len = lstrlenA(sz);
	if (!m_pEdit || m_nTotalSize < 1 || len < 1 || posStart + len >= m_nTotalSize - 1) return FALSE;

	int (WINAPI* pfn)(LPCSTR, LPCSTR, int);
	pfn = bCase ? StrCmpNA : StrCmpNIA;
	while (posStart + len < m_nTotalSize) {

		int ret = pfn((LPCSTR)(m_pEdit + posStart), sz, len);

		if (ret == 0) {
			*pFoundPos = posStart;
			return TRUE;
		}
		posStart++;
	}
	return FALSE;
}

BOOL CHexManager::FindNext(UINT_PTR posStart, LPBYTE pcbFind, DWORD dwLen, UINT_PTR* pFoundPos)
{
	// データを更新
	if (m_pClone) RenewMemory();

	if (!m_pEdit || m_nTotalSize < 1 || !dwLen || posStart + dwLen >= m_nTotalSize) return FALSE;

	while (posStart + dwLen < m_nTotalSize) {

		int ret = memcmp(m_pEdit + posStart, pcbFind, dwLen);

		if (ret == 0) {
			*pFoundPos = posStart;
			return TRUE;
		}
		posStart++;
	}
	return FALSE;
}

void CHexManager::_UpdateTotalSize(size_t nTotalSize)
{
	m_nTotalSize = nTotalSize;
	m_nTotalLine = (m_nTotalSize & ~0xf) >> 4;
	if (m_nTotalSize & 0xf)
		m_nTotalLine++;
}

BOOL CHexManager::Read(UINT_PTR posStart, size_t nRead, BYTE* pBuffer, size_t* pNumRead)
{
	*pNumRead = 0;
	if (posStart >= m_nTotalSize) return FALSE;

	// クローンデータが無い場合
	if (!m_pClone) {
		nRead = min(nRead, m_nTotalSize - posStart);
		memcpy(pBuffer, m_pEdit + posStart, nRead);
		*pNumRead = nRead;
		return TRUE;
	}

	size_t n = 0;
	size_t size;
	while (nRead) {
		if (m_posClone <= posStart &&
			(m_posClone + m_nNewCloneSize) > posStart)
		{
			if (m_nNewCloneSize < ((posStart - m_posClone) + nRead))
				size = ((posStart - m_posClone) + nRead) - m_nNewCloneSize;
			else
				size = 0;

			memcpy(pBuffer + n, m_pClone + (posStart - m_posClone), nRead - size); 

			posStart += (nRead - size);
			n += nRead - size;
			*pNumRead += (nRead - size);
			nRead = size;
		}
		else {
			if (posStart < m_posClone) {	// クローンより前にポイントがある場合
				// クローンに取得するデータが掛かってしまう場合
				if ((posStart + nRead) > m_posClone)
					size = nRead - (m_posClone - posStart);
				else
					size = 0;
			}
			else {							// クローンより後ろにポイントがある場合
				size_t delta;

				if ((posStart + nRead) > m_nTotalSize)
					size = (posStart + nRead) - m_nTotalSize;
				else
					size = 0;

				// クローンのデータが増えている場合
				if (m_nNewCloneSize > m_nOldCloneSize) {
					delta = m_nNewCloneSize - m_nOldCloneSize;
					posStart -= delta;		// 増えた分だけポイントを減らす

					// サイズの調整
					if ((posStart + (nRead - size)) >= (m_nTotalSize - delta)) {
						nRead = (m_nTotalSize - delta) - posStart;
						size = 0;
					}
				}
				else if (m_nNewCloneSize < m_nOldCloneSize) {
					// サイズが減った場合
					// サイズの調整
					delta = m_nOldCloneSize - m_nNewCloneSize;
					posStart += delta;
					if ((posStart + (nRead - size)) >= (m_nTotalSize + delta)) {
						nRead = (m_nTotalSize + delta) - posStart;
						size = 0;
					}
				}
				else {
					// 同じサイズ
					if ((posStart + (nRead - size)) >= m_nTotalSize) {
						nRead = m_nTotalSize - posStart;
						size = 0;
					}
				}
			}

			memcpy(pBuffer + n, m_pEdit + posStart, nRead - size);
			posStart += (nRead - size);
			*pNumRead += (nRead - size);
			n += nRead - size;
			nRead = size;
			break;
		}
	}
	return TRUE;
}

LPBYTE CHexManager::AllocRead(UINT_PTR posRead, size_t size, size_t* pNumRead)
{
	*pNumRead = 0;
	LPBYTE pb = (LPBYTE)ALLOC(size);
	if (pb) {
		size_t n;
		if (Read(posRead, size, pb, &n) && n) {
			*pNumRead = n;
			return pb;
		}
		FREE(pb);
	}
	return NULL;
}


BOOL CHexManager::CreateClone(UINT_PTR posStart, size_t nVisibleSize)
{
	// クローンデータ内にポイントがあるなら何もしない
	if (m_pClone) {
		if (m_posClone <= posStart &&  (posStart + nVisibleSize) <= (m_posClone + m_nNewCloneSize))
			return TRUE;

		// クローンデータが残っているならそれを書き込む
		RenewMemory();
	}

	// クローンの作成
	UINT_PTR pc;
	if (posStart <= (CLONESIZE / 2))
		pc = 0;
	else
		pc = ROWHEAD(posStart - (CLONESIZE / 2));
	nVisibleSize = max(nVisibleSize + (posStart - pc), CLONESIZE);

	size_t nNewSize;
	if ((pc + nVisibleSize) > m_nTotalSize)
		nNewSize = m_nTotalSize - pc;
	else
		nNewSize = nVisibleSize;

	// メモリで取得
	m_pClone = (LPBYTE)ALLOC(nNewSize);
	if (!m_pClone) return FALSE;

	if (!m_pEdit) {
		FREE(m_pClone);
		m_pClone = NULL;
		return FALSE;
	}

	// データのコピー
	memcpy(m_pClone, m_pEdit + pc, nNewSize);
	m_posClone = pc;
	m_nOldCloneSize = m_nNewCloneSize = nNewSize;

	return TRUE;
}

BOOL CHexManager::SaveUndo(int type, UINT_PTR pos, LPBYTE pData, size_t nDataSize)
{
	if (m_bReadOnly || !pData || !nDataSize) return TRUE;

	m_nModified++;

	CUndo* undo = new CUndo(type, pos, pData, nDataSize);
	if (undo) {
		m_Undo.Add(undo);
		return TRUE;
	}
	return FALSE;
}

BOOL CHexManager::DoUndo()
{
	if (!CanUndo()) return FALSE;

	m_nModified--;
	int last = m_Undo.GetSize() - 1;
	CUndo* undo = m_Undo[last];
	m_Undo.RemoveAt(last);

	UINT_PTR posNew;
	switch (undo->type()) {
		case UNDO_INSERT:
			// 挿入の場合は挿入したデータ（場所からのサイズ分）を削除
			Delete(undo->position(), undo->size());
			posNew = undo->position();
			break;

		case UNDO_OVERWRITE:
			if (!undo->data()) {
				delete undo;
				return FALSE;
			}
			Write(undo->position(), undo->data(), undo->size());
			posNew = undo->position() + undo->size() - 1;
			break;

		case UNDO_DELETE:
			if (!undo->data()) {
				delete undo;
				return FALSE;
			}

			Insert(undo->position(), undo->data(), undo->size());
			posNew = undo->position() + undo->size();
			break;

		default:
			delete undo;
			return FALSE;
	}
	delete undo;

	// 選択場所を変更
	UINT type;
	if (m_posType != SST_TEXT)
		type = SST_HIBYTE;
	else
		type = SST_TEXT;

	MoveTo(posNew, type);
	return TRUE;
}

UINT_PTR CHexManager::GetLastEditPos()
{
	if (!CanUndo()) return 0;

	int last = m_Undo.GetSize() - 1;
	return m_Undo[last]->position();
}

////////////////////////////////////////////////////////////////////////////////
// smartheap.h

#pragma once
#ifndef __SMART_HEAP_H__
#define __SMART_HEAP_H__

#include <atlcore.h>
#include <atlfile.h>

#include "Allocator.h"


////////////////////////////////////////////////////////////////////////////////
// CSmartHeapPtrT

template 
class CSmartHeapPtrT
{
protected:
	typedef CSmartHeapPtrT thisClass;

protected:
	CHeapPtr m_heap;
	size_t m_dwElementSize;

public:
	CSmartHeapPtrT()
	{
		m_dwElementSize = 0;
	}

	CSmartHeapPtrT(size_t dwElements)
	{
		HRESULT hr = S_OK;
		m_dwElementSize = 0;
		hr = this->Allocate(dwElements);
		if (FAILED(hr)) {
			// 無視する。。
		}
	}

	CSmartHeapPtrT(const T* p, size_t dwElements)
	{
		HRESULT hr = S_OK;
		m_dwElementSize = 0;
		hr = this->Load(p, dwElements);
		if (FAILED(hr)) {
			// 無視する。。
		}
	}

	CSmartHeapPtrT(IStreamPtr spStream)
	{
		HRESULT hr = S_OK;
		m_dwElementSize = 0;
		hr = this->Load(spStream);
		if (FAILED(hr)) {
			// 無視する。。
		}
	}

	virtual ~CSmartHeapPtrT()
	{
	}

public:
	// メモリを割り当てる
	HRESULT Allocate(size_t dwElements)
	{
		bool br = m_heap.Allocate(dwElements);
		if (br == false) {
			return E_OUTOFMEMORY;
		}
		m_dwElementSize = dwElements;
		return S_OK;
	}

	// メモリを再割り当てする
	HRESULT Reallocate(size_t dwElements)
	{
		if (m_heap.m_pData == NULL) {
			return this->Allocate(dwElements);
		}

		if (m_dwElementSize == dwElements) {
			return S_FALSE;
		}

		bool br = m_heap.Reallocate(dwElements);
		if (br == false) {
			return E_OUTOFMEMORY;
		}

		m_dwElementSize = dwElements;
		return S_OK;
	}
	// メモリを開放する
	HRESULT Free()
	{
		m_heap.Free();
		m_dwElementSize = 0;
		return S_OK;
	}

public:
	operator T*() const throw()
	{
//		ATLASSERT(m_heap.m_pData != NULL);
		return m_heap.m_pData;
	}

	T* operator ->() const throw()
	{
//		ATLASSERT(m_heap.m_pData != NULL);
		return m_heap.m_pData;
	}

public:
	T* GetData()
	{
//		ATLASSERT(m_heap.m_pData != NULL);
		return m_heap.m_pData;
	}

	size_t GetElementSize() const
	{
		return m_dwElementSize;
	}

	size_t GetByteSize() const
	{
		return thisClass::ClacByteSize(m_dwElementSize);
	}

public:
	static size_t ClacByteSize(size_t dwElements)
	{
		return sizeof(T) * dwElements;
	}

	static size_t ClacElementSize(size_t dwByteSize)
	{
		ATLASSERT(sizeof(T));
		return dwByteSize / sizeof(T);
	}

public:
	HRESULT Append(const T* p, size_t dwElements)
	{
		if (p == NULL) {
			return E_POINTER;
		}

		if (dwElements == 0) {
			// NOTE: サイズが0の場合も成功とする
			return S_FALSE;
		}

		HRESULT hr = S_OK;
		size_t dwOldByteSize = this->GetByteSize();
		hr = this->Reallocate(dwElements);
		if (FAILED(hr)) {
			return hr;
		}

		size_t dwByteSize = thisClass::ClacByteSize(dwElements);
		::MoveMemory(m_heap.m_pData + dwOldByteSize, p, dwByteSize);
		return S_OK;
	}

public:
	HRESULT Load(const T* p, size_t dwElements)
	{
		if (p == NULL) {
			ATLASSERT(0);
			return E_POINTER;
		}

		HRESULT hr = S_OK;
		try {
			hr = this->Free();
			if (FAILED(hr)) {
				// 無視する。。
			}

			size_t dwByteSize = thisClass::ClacByteSize(dwElements);
			if (dwByteSize == 0) {
				// NOTE: サイズが0の場合も成功とする
				return S_FALSE;
			}

			hr = this->Allocate(dwElements);
			if (FAILED(hr)) {
				return hr;
			}

			::MoveMemory(m_heap.m_pData, p, dwByteSize);
		}
		catch (...) {
			return E_FAIL;
		}

		return hr;
	}

public:
	HRESULT Load(IStreamPtr spStream, bool bReset = true)
	{
		if (spStream == NULL) {
			ATLASSERT(0);
			return E_INVALIDARG;
		}

		HRESULT hr = S_OK;
		try {
			hr = this->Free();
			if (FAILED(hr)) {
				// 無視する。。
			}

			ULARGE_INTEGER llByteSize = { 0, 0 };
			hr = thisClass::Emulete_IStream_Size(spStream, &llByteSize);
			if (FAILED(hr)) {
				return hr;
			}

			size_t dwByteSize = llByteSize.LowPart;
			if (dwByteSize == 0) {
				// NOTE: サイズが0の場合も成功とする
				return S_FALSE;
			}

			size_t dwElementSize = thisClass::ClacElementSize(dwByteSize);
			hr = this->Allocate(dwElementSize);
			if (FAILED(hr)) {
				return hr;
			}

			// シーク位置を先頭に戻す
			hr = thisClass::Emulete_IStream_Reset(spStream);
			if (FAILED(hr)) {
				// NOTE: 実装によっては無視する必要があるか？
				return hr;
			}

			hr = spStream->Read(m_heap.m_pData, (ULONG)dwByteSize, NULL);
			if (FAILED(hr)) {
				return hr;
			}

			if (bReset) {
				// シーク位置を先頭に戻す
				hr = thisClass::Emulete_IStream_Reset(spStream);
				if (FAILED(hr)) {
					// NOTE: 実装によっては無視する必要があるか？
					return hr;
				}
			}
		}
		catch (...) {
			return E_FAIL;
		}

		return S_OK;
	}

	HRESULT Save(IStreamPtr spStream, bool bReset = true)
	{
		if (spStream == NULL) {
			ATLASSERT(0);
			return E_INVALIDARG;
		}

		HRESULT hr = S_OK;
		try {
			// ストリームを空にする
			ULARGE_INTEGER ulnSize = {0, 0};
			hr = spStream->SetSize(ulnSize);
			if (FAILED(hr)) {
				// 実装によるので無視する。
			}

			// シーク位置を先頭に戻す
			hr = thisClass::Emulete_IStream_Reset(spStream);
			if (FAILED(hr)) {
				// NOTE: 実装によっては無視する必要があるか？
				return hr;
			}

			if (m_dwElementSize == 0) {
				return S_OK;
			}

			size_t dwByteSize = this->GetByteSize();
			hr = spStream->Write(m_heap.m_pData, (ULONG)dwByteSize, NULL);
			if (FAILED(hr)) {
				return hr;
			}

			if (bReset) {
				// シーク位置を先頭に戻す
				hr = thisClass::Emulete_IStream_Reset(spStream);
				if (FAILED(hr)) {
					// NOTE: 実装によっては無視する必要があるか？
					return hr;
				}
			}
		}
		catch (...) {
			return E_FAIL;
		}

		return S_OK;
	}

public:
	HRESULT Load(LPCTSTR szFileName)
	{
		if (szFileName == NULL) {
			return E_INVALIDARG;
		}

		HRESULT hr = S_OK;
		try {
			ULONGLONG llFileSize = 0;
			hr = thisClass::GetFileSize(szFileName, llFileSize);
			if (FAILED(hr)) {
				return hr;
			}

			// メモリの確保
			hr = this->Allocate((size_t)llFileSize);
			if (FAILED(hr)) {
				return hr;
			}

			CAtlFile file;
			hr = file.Create(szFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
			if (FAILED(hr)) {
				return E_FAIL;
			}

			hr = file.Read(m_heap.m_pData, (DWORD)llFileSize);
			if (FAILED(hr)) {
				return E_FAIL;
			}

			file.Close();
		}
		catch (...) {
			ATLASSERT(0);
			return E_FAIL;
		}

		return S_OK;
	}

	HRESULT Save(LPCTSTR szFileName)
	{
		if (szFileName == NULL) {
			return E_INVALIDARG;
		}

		HRESULT hr = S_OK;

		try {
			CAtlFile file;
			hr = file.Create(szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS);
			if (FAILED(hr)) {
				return E_FAIL;
			}

			size_t dwByteSize = this->GetByteSize();
#pragma warning(disable: 4267)
			hr = file.Write(m_heap.m_pData, dwByteSize);
#pragma warning(default: 4267)
			if (FAILED(hr)) {
				return hr;
			}

			file.Close();
		}
		catch (...) {
			ATLASSERT(0);
			return E_FAIL;
		}

		return hr;
	}

protected:
	// StreamHelper.hよりコピー
	// shlwapi.dll version 5.0 or later 互換
	static HRESULT Emulete_IStream_Reset(IStream *pstm)
	{
		if (pstm == NULL) {
			return E_FAIL;
		}

		HRESULT hr = S_OK;
		try {
			LARGE_INTEGER zeroPos = {0, 0};
			ULARGE_INTEGER streamSize = {0, 0};
			hr = pstm->Seek(zeroPos, STREAM_SEEK_SET, &streamSize);
			if (FAILED(hr)) {
				return hr;
			}
		}
		catch (...) {
			return E_FAIL;
		}

		return hr;
	}

	// StreamHelper.hよりコピー
	// shlwapi.dll version 5.0 or later 互換
	static HRESULT Emulete_IStream_Size(IStream *pstm, ULARGE_INTEGER *pui)
	{
		if (pstm == NULL) {
			return E_FAIL;
		}

		if (pui == NULL) {
			return E_POINTER;
		}

		HRESULT hr = S_OK;
		try {
			ULARGE_INTEGER zero = {0, 0};
			*pui = zero;

			STATSTG stat;
			memset(&stat, 0, sizeof(stat));

			hr = pstm->Stat(&stat, STATFLAG_NONAME);
			if (FAILED(hr)) {
				return hr;
			}

			*pui = stat.cbSize;
		}
		catch (...) {
			return E_FAIL;
		}

		return hr;
	}

	// FileHelper.h よりコピー
	static HRESULT GetFileSize(LPCTSTR szFileName, ULONGLONG& nLen)
	{
		WIN32_FIND_DATA wfd;
		memset(&wfd, 0, sizeof(wfd));
		HANDLE hFind = ::FindFirstFile(szFileName, &wfd);
		if (hFind == INVALID_HANDLE_VALUE) {
			return E_FAIL;
		}
		::FindClose(hFind);

		ULARGE_INTEGER liFileSize;
		liFileSize.HighPart = wfd.nFileSizeHigh;
		liFileSize.LowPart = wfd.nFileSizeLow;
		nLen = liFileSize.QuadPart;
		return S_OK;
	}
};

typedef CSmartHeapPtrT<BYTE>  CSmartHeapPtr;


#endif		// __SMART_HEAP_PTR_H__

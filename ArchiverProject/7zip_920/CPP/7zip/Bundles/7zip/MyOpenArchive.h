// MyOpenArchive.h: OpenArchive クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENARCHIVE_H__A28A0BBF_060E_4378_9854_0FF8C2A59CFC__INCLUDED_)
#define AFX_OPENARCHIVE_H__A28A0BBF_060E_4378_9854_0FF8C2A59CFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Windows/FileFind.h"
#include "Common/Wildcard.h"
#include "Common/StringConvert.h"
#include "../../UI/Common/OpenArchive.h"
#include "7-zip32.h"

class COpenArchive
{
public:
	int GetArchiveType() { return m_nArchiveType; }
	int GetFileCount() { return m_aNumItems; }
	LPCWSTR GetDefaultPassword() { return m_lpPassword; }
	void SetDefaultPassword(LPCSTR lpPassword);
	BOOL GetCompressedSizeEx(ULHA_INT64* lpllSize);
	BOOL GetOriginalSizeEx(ULHA_INT64* lpllSize);
	BOOL GetArcCompressedSizeEx(ULHA_INT64* lpllSize);
	BOOL GetArcOriginalSizeEx(ULHA_INT64* lpllSize);
	BOOL GetArcFileSizeEx(ULHA_INT64* lpllSize);
	BOOL GetArcWriteTimeEx(FILETIME* lpftLastWriteTime);
	BOOL GetArcAccessTimeEx(FILETIME* lpftLastAccessTime);
	BOOL GetArcCreateTimeEx(FILETIME* lpftCreationTime);
	BOOL GetAccessTimeEx(FILETIME *lpftLastAccessTime);
	BOOL GetCreateTimeEx(FILETIME *lpftLastCreateTime);
	BOOL GetWriteTimeEx(FILETIME* lpftLastWriteTime);
	DWORD GetAccessTime();
	DWORD GetCreateTime();
	DWORD GetWriteTime();
	LPCWSTR GetMethod() { return m_bSearchMode ? (LPCWSTR)m_aMode : NULL; }
	UINT GetOSType() { return m_bSearchMode ? 0 : -1; }
	int GetAttribute() { return m_bSearchMode ? m_dwAttribute & 0xff : -1; }
	DWORD GetCRC() { return m_bSearchMode ? m_dwCRC : -1; }
	WORD GetTime();
	WORD GetDate();
	WORD GetRatio();
	DWORD GetCompressedSize() { return m_bSearchMode && m_ui64CompressedSize <= 0xFFFFFFFF ? (DWORD)m_ui64CompressedSize : -1; }
	DWORD GetOriginalSize() { return m_bSearchMode && m_ui64OriginalSize <= 0xFFFFFFFF ? (DWORD)m_ui64OriginalSize : -1; }
	LPCWSTR GetFileName() { return m_bSearchMode ? (LPCWSTR)m_aFileName : NULL; }
	int IsSFXFile() { return 0; }
	UINT GetArcOSType() { return 0; }
	WORD GetArcTime();
	WORD GetArcDate();
	WORD GetArcRatio() { return m_ui64ArcOriginalSize ? (WORD)(m_ui64ArcCompressedSize * 1000 / m_ui64ArcOriginalSize) : 0; }
	DWORD GetArcCompressedSize() { return m_bSolid || m_ui64ArcCompressedSize > 0xFFFFFFFF ? -1 : (DWORD)m_ui64ArcCompressedSize; }
	DWORD GetArcOriginalSize() { return m_ui64ArcOriginalSize > 0xFFFFFFFF ? -1 : (DWORD)m_ui64ArcOriginalSize; }
	DWORD GetArcFileSize() { return  m_fiArchive.Size > 0xFFFFFFFF ? -1 : (DWORD)m_fiArchive.Size; }
	LPCWSTR GetArcFileName() { return m_fiArchive.Name; }
	int FindNext(INDIVIDUALINFO *lpSubInfo);
	int FindFirst(LPCWSTR lpszWildName, INDIVIDUALINFO *lpSubInfo);
	BOOL Open(LPCWSTR lpFileName, DWORD dwMode);
	HRESULT OpenCheck(LPCWSTR lpFileName, DWORD dwMode);

	static void RemoveAll();
	static COpenArchive* FindObject(HARC hArc);

	COpenArchive();
	virtual ~COpenArchive();
	COpenArchive* m_pOAPrev;
	COpenArchive* m_pOANext;

protected:
	CArchiveLink m_archiveLink;
	NWildcard::CCensor* m_pWildcardCensor;
	UINT32 m_aNumItems;
	UINT32 m_aItemPos;
	UINT64 m_ui64OriginalSize;
	UINT64 m_ui64CompressedSize;
	UINT64 m_ui64ArcCompressedSize;
	UINT64 m_ui64ArcOriginalSize;
	UString m_aFileName;
	UString m_aMode;
	FILETIME m_ftLastCreateTime;
	FILETIME m_ftLastAccessTime;
	FILETIME m_ftLastWriteTime;
	DWORD m_dwCRC;
	DWORD m_dwAttribute;
	DWORD m_dwMode;
	NWindows::NFile::NFind::CFileInfoW m_fiArchive;
	bool m_bSearchMode;
	bool m_bSolid;
	int m_nArchiveType;
	UString m_strDefaultItemName;
	LPWSTR m_lpPassword;
	bool m_bEncrypt;
};

#endif // !defined(AFX_OPENARCHIVE_H__A28A0BBF_060E_4378_9854_0FF8C2A59CFC__INCLUDED_)

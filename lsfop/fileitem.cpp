///////////////////////////////////////////////////////////////////////
//

#include "stdafx.h"
#include "fopstd.h"
#include "match.h"

#define GMSHARE		(GMEM_SHARE | GMEM_ZEROINIT)


///////////////////////////////////////////////////////////////////////
// CWin32FindDataクラス

long __fastcall _comparefiletime(const FILETIME* pftSrc, const FILETIME* pftDst, BOOL bRaw)
{
	// そのまま比較の場合
	if (bRaw) return CompareFileTime(pftSrc, pftDst);

	DWORDLONG s, t;
	SETDWORDLONG(s, pftSrc->dwLowDateTime, pftSrc->dwHighDateTime);
	SETDWORDLONG(t, pftDst->dwLowDateTime, pftDst->dwHighDateTime);

	// 秒単位で比較の場合
	s /= 10000000;
	t /= 10000000;
	return ((s < t) ? -1 : ((s > t) ? 1 : 0));
}

///////////////////////////////////////////////////////////////////////
// CFileInfoクラス
//

int CFileInfo::FormatSize(LPTSTR pszSize, int cch)
{
	if (m_dwAttr & FA_SUBDIR) {
		*pszSize = 0;
		return 0;
	}
	FormatByteSize(m_dlSize, pszSize, cch);
	return lstrlen(pszSize);
}

///////////////////////////////////////////////////////////////////////
// CFileArray class

void CFileArray::DeleteAll()
{
	CFileInfo* pf;
	int cnt = (int)GetCount();
	for (int i = 0; i < cnt; i++) {
		pf = GetAt(i);
		if (pf) delete pf;
	}
	RemoveAll();
}

BOOL CFileArray::AddFile(LPCTSTR pszFile)
{
	CFileInfo fi;
	return (GetFileInfo(pszFile, fi) ? AddFile(fi) : FALSE);
}

BOOL CFileArray::AddFile(CFileInfo& fi)
{
	CFileInfo* pfNew = new CFileInfo(fi);
	Add(pfNew);
	return TRUE;
}

HDROP CFileArray::MakeDropFiles()
{
	if (GetCount() < 1) return NULL;

	CFileInfo* pf;
	POINT pt;
	pt.x = pt.y = 0;
	HDROP hDrop = fopDragInitBlock(pt, FALSE);
	HDROP hw;
	int cnt = (int)GetCount();
	for (int i = 0; i < cnt; i++) {
		pf = GetAt(i);
		hw = fopDragAppendFile(hDrop, pf->m_strFullPathName);
		if (!hw) {
			DragFinish(hDrop);
			return NULL;
		}
		hDrop = hw;
	}
	return hDrop;
}

int CFileArray::Filter(LPCTSTR pszFilter, BOOL bExclude, BOOL bRangeCase)
{
	if (!pszFilter || *pszFilter == 0 ||
		!StrCmp(pszFilter, gszAst) || !StrCmp(pszFilter, _T("*")))
		return 0;

	CMatchSpec msp(pszFilter, bRangeCase);

	CAtlArray<CFileInfo*> arrPickup;
	int cnt = (int)GetCount();
	for (int i = 0; i < cnt; i++) {
		CFileInfo* pf = GetAt(i);
		if (!pf || pf->IsFolder()) continue;

		BOOL bMatch = msp.Match(pf->m_pszName);
		if (bExclude != bMatch) {
			arrPickup.Add(pf);
			SetAt(i, NULL);
		}
	}

	cnt = (int)arrPickup.GetCount();
	if (cnt > 0) {
		DeleteAll();
		Append(arrPickup);
		FreeExtra();
	}
	return cnt;
}

// ファイル名で比較
int CFileArray::GetItemNameIndex(LPCTSTR lpName)
{
	register CFileInfo *pfSrc;
	int cnt = (int)GetCount();
	for (int i = 0; i < cnt; i++) {
		pfSrc = GetAt(i);
		if (!_pathcompare(pfSrc->m_pszName, lpName))
			return i;
	}
	return -1;
}

int CFileArray::BrowseFiles(LPCTSTR pszPath, LPCTSTR pszFilter, BOOL bUnmatchFilter, BOOL bUseRegExp, BOOL bCaseSense)
{
	// ファイルのみ検索
	CEnterFolder ef(pszPath);
	if (!ef.IsFolderChanged()) return -1;

	CMatchSpec msp(pszFilter, bUseRegExp, bCaseSense);

	int cnt = 0;
	CFileInfo fi;
	CFileFinder ff;
	CString str;
	if (ff.BeginFind(gszAst)) {
		do {
			if (!ff.IsFolder()) {
				BOOL bMatch = msp.Match(ff.cFileName);
				if (bMatch != bUnmatchFilter) {
					fi.SetFile(ff, pszPath);
					AddFile(fi);
					cnt++;
				}
			}
		} while (ff.NextFind());
		ff.EndFind();
	}
	return cnt;
}

int CFileArray::EnumSubFolder(LPCTSTR pszPath)
{
	// フォルダのみ検索
	CEnterFolder ef(pszPath);
	if (!ef.IsFolderChanged()) return -1;

	CFileInfo fi;

	// ディレクトリ検索
	int cnt = 0;
	CFileFinder ff;
	if (ff.BeginFind(gszAst)) {
		do {
			if (ff.IsFolder() && !ff.IsDot()) {
				fi.SetFile(ff, pszPath);
				AddFile(fi);
				cnt++;
			}
		} while (ff.NextFind());
		ff.EndFind();
	}
	return cnt;
}

BOOL CFileArray::IsFolderInclude()
{
	CFileInfo* pf;
	int cnt = (int)GetCount();
	for (int i = 0; i < cnt; i++) {
		pf = GetAt(i);
		if (pf->IsFolder())
			return TRUE;
	}
	return FALSE;
}

static int comparray(LPCVOID arg1, LPCVOID arg2)
{
	CFileInfo *p1 = *(CFileInfo**)arg1;
	CFileInfo *p2 = *(CFileInfo**)arg2;
	return StrCmpI(p1->m_pszName, p2->m_pszName);
}

void CFileArray::Sort()
{
	int cnt = (int)GetCount();
	if (cnt > 1)
		qsort(GetData(), cnt, sizeof(CFileInfo*), comparray);
}


//////////////////////////////////////////////////////////////////////
// CDropFiles

CDropFiles::CDropFiles(HDROP hDrop, DWORD eff)
{
	m_dropEffect = eff;
	SIZE_T size = GlobalSize(hDrop);
	m_hDrop = (HDROP)GlobalAlloc(GMSHARE, size);
	if (m_hDrop) {
		LPTSTR lpSrc = (LPTSTR)GlobalLock(hDrop);
		LPTSTR lpDst = (LPTSTR)GlobalLock(m_hDrop);
		memcpy(lpDst, lpSrc, size);
		GlobalUnlock(hDrop);
		GlobalUnlock(m_hDrop);

		m_nEntry = ::DragQueryFile(m_hDrop, 0xFFFFFFFF, NULL, 0);
	}
}

CDropFiles::CDropFiles(CFileInfo* pf, DWORD eff)
{
	m_dropEffect = eff;
	POINT pt;
	pt.x = pt.y = 0;
	if (DragInitHeader(pt, FALSE))
		DragAppendFile(pf);
}

CDropFiles::CDropFiles(LPCTSTR pszFile, DWORD eff)
{
	m_dropEffect = eff;
	POINT pt;
	pt.x = pt.y = 0;
	if (DragInitHeader(pt, FALSE))
		DragAppendFile(pszFile);
}

CDropFiles::CDropFiles(CFileArray* pArray, DWORD eff)
{
	m_dropEffect = eff;
	POINT pt;
	pt.x = pt.y = 0;
	if (DragInitHeader(pt, FALSE)) {
		CFileInfo* pf;
		int cnt = (int)pArray->GetCount();
		for (int i = 0; i < cnt; i++) {
			pf = (*pArray)[i];
			if (pf)
				DragAppendFile(pf);
		}
	}
}

HDROP CDropFiles::Attach(HDROP hDrop, DWORD eff)
{
	m_dropEffect = eff;
	HDROP hDropTemp = m_hDrop;
	m_hDrop = hDrop;
	if (m_hDrop)
		m_nEntry = ::DragQueryFile(m_hDrop, 0xFFFFFFFF, NULL, 0);
	else
		m_nEntry = 0;
	return hDropTemp;
}

HDROP CDropFiles::Detach()
{
	HDROP hDropTemp = m_hDrop;
	m_nEntry = 0;
	m_hDrop = NULL;
	return hDropTemp;
}

void CDropFiles::DragFinish()
{
	if (m_hDrop)
		::DragFinish(m_hDrop);
	m_hDrop = NULL;
	m_nEntry = 0;
}

BOOL CDropFiles::DragQueryFile(int nIndex, LPTSTR lpFile, int cchMax)
{
	if (m_hDrop && nIndex >= 0 && nIndex < m_nEntry) {
		::DragQueryFile(m_hDrop, nIndex, lpFile, cchMax);
		return TRUE;
	}
	return FALSE;
}

UINT CDropFiles::DragQueryFile(int nIndex, CString& string)
{
	UINT len = 0;
	string.Empty();
	if (m_hDrop && nIndex >= 0 && nIndex < m_nEntry) {
		len = ::DragQueryFile(m_hDrop, nIndex, NULL, 0);
		LPTSTR p = string.GetBuffer(len + 1);
		::DragQueryFile(m_hDrop, nIndex, p, len + 1);
		string.ReleaseBuffer();
	}
	return len;
}

BOOL CDropFiles::DragInitHeader(POINT pt, BOOL fInNonClientArea)
{
	DragFinish();
	m_hDrop = (HDROP)GlobalAlloc(GMSHARE, sizeof(DROPFILES) + sizeof(TCHAR));
	if (m_hDrop) {
		LPDROPFILES lpdf = (LPDROPFILES)GlobalLock(m_hDrop);
		lpdf->pFiles = sizeof(DROPFILES);
		lpdf->pt = pt;
		lpdf->fNC = (WORD)fInNonClientArea;
		lpdf->fWide = TRUE;						// UNICODE
		GlobalUnlock(m_hDrop);
		return TRUE;
	}
	return FALSE;
}

BOOL CDropFiles::DragAppendFile(LPCTSTR pszPath)
{
	if (m_hDrop) {
		int len = lstrlen(pszPath) + 2;
		HDROP hDropT = (HDROP)GlobalReAlloc(m_hDrop,
								GlobalSize(m_hDrop) + len * sizeof(TCHAR), GHND | GMEM_SHARE);
		if (hDropT) {
			LPDROPFILES pdf = (LPDROPFILES)GlobalLock(hDropT);
			LPTSTR p = (LPTSTR)((BYTE*)pdf + pdf->pFiles);
			while (*p) {
				while (*p)
					p++;
				p++;
			}
			lstrcpy(p, pszPath);
			GlobalUnlock(hDropT);
			m_hDrop = hDropT;
			m_nEntry++;
			return TRUE;
		}
	}
	return FALSE;
}

int CDropFiles::EnumItemToStr(CString& csFiles)
{
	// 注意：このメンバは選択されているファイルの全てを返すわけではない！
	csFiles.Empty();
	int numfiles = 0;
	if (m_hDrop) {
		int cnt = min(4, m_nEntry);
		TCHAR buf[MAX_PATH * 2], name[MAX_PATH * 2];
		for (int i = 0; i < cnt; i++) {
			::DragQueryFile(m_hDrop, i, buf, ARRAYSIZE(buf));
			if (!_pathfileexists(buf))
				continue;

			lstrcpy(name, PathFindFileName(buf));
			PathQuoteSpaces(name);

			if (!csFiles.IsEmpty())
				csFiles += _T(' ');
			csFiles += name;
			numfiles++;
		}
		if (cnt < m_nEntry)
			csFiles += _T(" ...");
	}
	return numfiles;
}

LPTSTR CDropFiles::MakeFileListBuffer(int* numFiles)
{
	// このルーチンを呼び出して取得したリストバッファはLocalFreeで解放すること

	LPTSTR str = NULL;
	int cnt = 0;
	if (m_hDrop && m_nEntry > 0) {
		str = (LPTSTR)LocalAlloc(LPTR, (MAX_PATH + 2) * m_nEntry * sizeof(TCHAR));
		LPTSTR p = str;
		for (int i = 0; i < m_nEntry; i++) {
			::DragQueryFile(m_hDrop, i, p, MAX_PATH);
			if (!_pathfileexists(p)) continue;

			cnt++;
			p += lstrlen(p) + 1;
		}

		if (cnt > 0)
			*p = 0;
		else {
			LocalFree(str);
			str = NULL;
		}
	}

	if (numFiles) *numFiles = cnt;
	return str;
}

BOOL CDropFiles::IsFolderInclude()
{
	if (m_hDrop) {
		TCHAR buf[MAX_PATH * 2];
		for (int i = 0; i < m_nEntry; i++) {
			::DragQueryFile(m_hDrop, i, buf, ARRAYSIZE(buf));
			if (_pathisdirectory(buf))
				return TRUE;
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////
//

#include "stdafx.h"
#include "common.h"
#include "fileitem.h"
#include "matchspec.h"

static const TCHAR sszPF[] = _T("%pf%");
static const TCHAR sszDate[] = _T("yy/MM/dd");
static const TCHAR sszTime[] = _T("HH:mm:ss");

TCHAR gchNil = 0;

#define GMSHARE		(GMEM_SHARE | GMEM_ZEROINIT)



static const TCHAR szClipHeader[] =
_T("  date       time     dos name        size      attr   filename\x0d\x0a")
_T("---------- -------- ------------ ------------- ------ ----------------------\x0d\x0a");

#define STRCOMP(a,b)		(rStringCompare((a),(b)))



///////////////////////////////////////////////////////////////////////
//

BOOL __fastcall GetFileInfo(LPCTSTR pszName, CFileInfo& fi)
{
	CFileFinder finder;
	BOOL bOK = FALSE;
	if (finder.BeginFind(pszName)) {
		finder.EndFind();

		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, pszName);
		PathRemoveFileSpec(buf);
		fi.SetFile(buf, finder);
		bOK = TRUE;
	}
	return bOK;
}


///////////////////////////////////////////////////////////////////////
//

int __cdecl CompareFileInfo(const void* pa1, const void* pa2, const void* typeseq)
{
	CFileInfo* p1 = *(CFileInfo**)pa1;
	CFileInfo* p2 = *(CFileInfo**)pa2;

	int r;
	switch (LOWORD(typeseq)) {
		case SORT_NAME:
			if (p1->IsFolder()) {
				if (!p2->IsFolder())
					return -1;
			}
			else if (p2->IsFolder())
				return 1;
			r = STRCOMP(p1->m_strName, p2->m_strName);
			break;

		case SORT_TYPE:
		{
			if (p1->IsFolder()) {
				if (!p2->IsFolder())
					return -1;
				else {
					r = STRCOMP(p1->m_strName, p2->m_strName);
					break;
				}
			}
			else if (p2->IsFolder())
				return 1;

			if (p1->m_strType.IsEmpty() || p2->m_strType.IsEmpty()) {
				if (!p1->m_strType.IsEmpty())
					r = 1;
				else if (!p2->m_strType.IsEmpty())
					r = -1;
				else
					r = 0;
			}
			else
				r = STRCOMP(p1->m_strType, p2->m_strType);

			// 等しくなければ終了
			if (r)
				break;

			// そうでなければ拡張子で比較のために流下
		}

		case SORT_EXT:
		{
			if (p1->IsFolder()) {
				if (!p2->IsFolder())
					return -1;
				else {
					r = STRCOMP(p1->m_strName, p2->m_strName);
					break;
				}
			}
			else if (p2->IsFolder())
				return 1;

			LPCTSTR ps1 = p1->m_strName;
			LPCTSTR ps2 = StrRChr(ps1, NULL, '.');
			LPCTSTR pt1 = p2->m_strName;
			LPCTSTR pt2 = StrRChr(pt1, NULL, '.');
			if (!ps2 || !pt2) {
				if (ps2)
					r = 1;
				else if (pt2)
					r = -1;
				else
					r = 0;
			}
			else
				r = STRCOMP(ps2, pt2);

			if (!r)
				r = STRCOMP(ps1, pt1);
			break;
		}

		case SORT_SIZE:
			if (p1->IsFolder()) {
				if (p2->IsFolder())
					r = STRCOMP(p1->m_strName, p2->m_strName);
				else
					return -1;
			}
			else if (p2->IsFolder())
				return 1;

			if (p1->m_dlSize < p2->m_dlSize)
				r = -1;
			else if (p1->m_dlSize == p2->m_dlSize)
				r = STRCOMP(p1->m_strName, p2->m_strName);
			else
				r = 1;
			break;

		case SORT_TIME:
		{
			if (p1->IsFolder()) {
				if (!p2->IsFolder())
					return -1;
			}
			else if (p2->IsFolder())
				return 1;

			LONG l;
			l = CompareFileTime(&p1->m_ftLastWriteTime, &p2->m_ftLastWriteTime);
			r = (l == 0) ? STRCOMP(p1->m_strName, p2->m_strName) : (int)l;
			break;
		}

		case SORT_ATTR:
			if (p1->IsFolder()) {
				if (!p2->IsFolder())
					return -1;
			}
			else if (p2->IsFolder())
				return 1;

			if (p1->m_dwAttr < p2->m_dwAttr)
				r = -1;
			else if (p1->m_dwAttr == p2->m_dwAttr)
				r = STRCOMP(p1->m_strName, p2->m_strName);
			else
				r = 1;
			break;

		case SORT_PATH:
			r = lccComparePath(p1->m_strPath, p2->m_strPath);
			if (!r)
				r = STRCOMP(p1->m_strName, p2->m_strName);
			break;

		default:
			r = p1->m_nSerial - p2->m_nSerial;
			if (!r)
				r = STRCOMP(p1->m_strName, p2->m_strName);
			break;
	}

    //
	// 昇順・降順の決定
	//
	if (HIWORD(typeseq) == SORT_DOWN)
		r *= -1;
    return r;
}


///////////////////////////////////////////////////////////////////////
// CFileInfoクラス
//

void CFileInfo::SetTypeName()
{
	m_strType.Empty();

	UINT flags = SHGFI_TYPENAME;
	SHFILEINFO sfi;
	if (IsFolder()) {
		SHGetFileInfo(m_strName, 0, &sfi, sizeof(sfi), flags);
		m_strType = sfi.szTypeName;
		return;
	}

	LPCTSTR pszExt = PathFindExtension(m_strName);
	if (*pszExt != '.')
		return;

	LPCTSTR pszType = GetMapType(++pszExt);
	if (pszType) {
		m_strType = pszType;
		return;
	}

	flags |= SHGFI_USEFILEATTRIBUTES;
	SHGetFileInfo(m_strName, 0, &sfi, sizeof(sfi), flags);
	m_strType = AddMapType(pszExt, sfi.szTypeName);
}

//////////////////////////////////////////////////////////////////////
// CDropFiles


void CDropFiles::CopyToClipboard(HWND hwnd)
{
	if (!m_hDrop)
		return;

	CString cs = szClipHeader;
	LPTSTR lp = tchar_malloc(cs.GetLength());
	lstrcpy(lp, cs);
	long lPos = cs.GetLength();

	LPTSTR lpT;
	TCHAR path[MAX_PATH * 2], wk[MAX_PATH];
	CFileInfo fi;
	for (int i = 0; i < m_nEntry; i++) {
		::DragQueryFile(m_hDrop, i, path, _COUNTOF(path));
		if (!GetFileInfo(path, fi))
			continue;

		// タイムスタンプ
		FormatFileTime(&fi.m_ftLastWriteTime, wk, TRUE, FALSE);
		// ファイルサイズ
		num_form(fi.m_dlSize, &wk[40]);
		lccFileAttrToStr(fi.m_dwAttr, &wk[60]);

		char dos[16], dosfmt[16];
		WideCharToMultiByte(CP_ACP, 0, fi.m_strDos, -1, dos, _COUNTOF(dos), NULL, NULL);
		wsprintfA(dosfmt, "%-12s", dos);
		MultiByteToWideChar(CP_ACP, 0, dosfmt, -1, &wk[128], 16);

		wsprintf(path, _T("%s %s %13s %6s "), wk, &wk[128], &wk[40], &wk[60]);
		lstrcat(path, fi.m_strName);
		lstrcat(path, gszCrLf);

		lpT = tchar_realloc(lp, lPos + lstrlen(path) + 1);
		if (!lpT)
			break;

		lp = lpT;
		lstrcpy(&lp[lPos], path);
		lPos += lstrlen(path);
	}

	if (lp) {
		lccSetStringToClipboard(hwnd, lp);
		free(lp);
	}
}

//////////////////////////////////////////////////////////////////////
//

static CExtMap mapExt;

LPCTSTR __fastcall GetMapType(LPCTSTR key)
{
	if (*key == '.') key++;

	LPCTSTR p = NULL;
	return mapExt.Lookup(key, &p) ? p : NULL;
}

LPCTSTR __fastcall AddMapType(LPCTSTR key, LPCTSTR name)
{
	if (*key == '.') key++;

	return mapExt.AddType(key, name);
}

void __fastcall InitMapType()
{
	mapExt.DeleteAll();
}

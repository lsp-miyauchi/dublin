// arcitem.cpp : implementation file
//

#include "stdafx.h"
#include "arcitem.h"
#include "..\lsarcsvr\comarcdef.h"
#include "common.h"
#include "resource.h"

int match(LPCTSTR string, LPCTSTR pattern);
void __cdecl qsort_ex(void *base, size_t n, size_t size,
	int (__cdecl *comp)(const void*, const void*, const void*), const void* param);

extern TCHAR gszAst[];

typedef struct tagCMPINF {
	int type;
	int seq;
} CMPINF;


#define STRCOMP(a,b)		(StrCmpLogicalW((a),(b)))


/////////////////////////////////////////////////////////////////////////////
//

static int sort_fullname(const void* arg1, const void* arg2, const void* plm)
{
	CArcItem* p1 = *(CArcItem**)arg1;
	CArcItem* p2 = *(CArcItem**)arg2;
	CMPINF* pci = (CMPINF*)plm;

	TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
	lstrcpy(src, p1->m_strRealName);
	lstrcpy(dst, p2->m_strRealName);
	if (p1->IsFolder())
		add_slash(src);
	if (p2->IsFolder())
		add_slash(dst);
	int r = compare_fullname(src, dst);

	if (pci->seq == SORT_DOWN)
		r *= -1;
    return r;
}

static int sort_path(const void* arg1, const void* arg2, const void* plm)
{
	CArcItem* p1 = *(CArcItem**)arg1;
	CArcItem* p2 = *(CArcItem**)arg2;
	CMPINF* pci = (CMPINF*)plm;

	int r = compare_path(p1->m_strPath, p2->m_strPath);
	if (!r) {
		if (p1->IsFolder()) {
			if (!p2->IsFolder())
				return -1;
		}
		else if (p2->IsFolder())
			return 1;
		else
			r = STRCOMP(p1->m_strName, p2->m_strName);
	}
	if (pci->seq == SORT_DOWN)
		r *= -1;
    return r;
}

static int sort_name(const void* arg1, const void* arg2, const void* plm)
{
	CArcItem* p1 = *(CArcItem**)arg1;
	CArcItem* p2 = *(CArcItem**)arg2;
	CMPINF* pci = (CMPINF*)plm;

	if (p1->IsFolder()) {
		if (!p2->IsFolder())
			return -1;
	}
	else if (p2->IsFolder())
		return 1;

	int r = STRCOMP(p1->m_strName, p2->m_strName);
	if (pci->seq == SORT_DOWN)
		r *= -1;
    return r;
}

static int sort_ext(const void* arg1, const void* arg2, const void* plm)
{
	CArcItem* p1 = *(CArcItem**)arg1;
	CArcItem* p2 = *(CArcItem**)arg2;
	CMPINF* pci = (CMPINF*)plm;

	if (p1->IsFolder()) {
		if (!p2->IsFolder())
			return -1;
	}
	else if (p2->IsFolder())
		return 1;

	int r;
	LPTSTR e1 = PathFindExtension(p1->m_strName);
	LPTSTR e2 = PathFindExtension(p2->m_strName);
	if (*e1 != '.') {
		if (*e2 != '.')
			r = STRCOMP(p1->m_strName, p2->m_strName);
		else
			r = -1;
	}
	else if (*e2 != '.')
		r = 1;
	else {
		r = STRCOMP(e1, e2);
		if (!r)
			r = STRCOMP(p1->m_strName, p2->m_strName);
	}

	if (pci->seq == SORT_DOWN)
		r *= -1;
    return r;
}

static int sort_size(const void* arg1, const void* arg2, const void* plm)
{
	CArcItem* p1 = *(CArcItem**)arg1;
	CArcItem* p2 = *(CArcItem**)arg2;
	CMPINF* pci = (CMPINF*)plm;

	if (p1->IsFolder()) {
		if (!p2->IsFolder())
			return -1;
	}
	else if (p2->IsFolder())
		return 1;

	int r;
	if (p1->m_dwSize < p2->m_dwSize)
		r = -1;
	else if (p1->m_dwSize == p2->m_dwSize)
		r = STRCOMP(p1->m_strName, p2->m_strName);
	else
		r = 1;

	if (pci->seq == SORT_DOWN)
		r *= -1;
    return r;
}

static int sort_time(const void* arg1, const void* arg2, const void* plm)
{
	CArcItem* p1 = *(CArcItem**)arg1;
	CArcItem* p2 = *(CArcItem**)arg2;
	CMPINF* pci = (CMPINF*)plm;

	if (p1->IsFolder()) {
		if (!p2->IsFolder())
			return -1;
	}
	else if (p2->IsFolder())
		return 1;

	int r = p1->m_wDate - p2->m_wDate;
	if (!r) {
		r = p1->m_wTime - p2->m_wTime;
		if (!r)
			r = STRCOMP(p1->m_strName, p2->m_strName);
	}

	if (pci->seq == SORT_DOWN)
		r *= -1;
    return r;
}

static int sort_attr(const void* arg1, const void* arg2, const void* plm)
{
	CArcItem* p1 = *(CArcItem**)arg1;
	CArcItem* p2 = *(CArcItem**)arg2;
	CMPINF* pci = (CMPINF*)plm;

	if (p1->IsFolder()) {
		if (!p2->IsFolder())
			return -1;
	}
	else if (p2->IsFolder())
		return 1;

	int r = STRCOMP(p1->m_strAttr, p2->m_strAttr);
	if (!r)
		r = STRCOMP(p1->m_strName, p2->m_strName);
	if (pci->seq == SORT_DOWN)
		r *= -1;
    return r;
}

static int sort_none(const void* arg1, const void* arg2, const void* plm)
{
	CArcItem* p1 = *(CArcItem**)arg1;
	CArcItem* p2 = *(CArcItem**)arg2;
	CMPINF* pci = (CMPINF*)plm;

	int r = (p1->m_serial - p2->m_serial);
	if (pci->seq == SORT_DOWN)
		r *= -1;
    return r;
}

/////////////////////////////////////////////////////////////////////////////
//

void SortArcItemArray(CSimpleArray<CArcItem*>& arr)
{
	int cnt = arr.GetSize();
	if (cnt > 1) {
		CMPINF ci;
		ci.seq = SORT_UP;
		qsort_ex(arr.GetData(), cnt, sizeof(CArcItem*), sort_fullname, &ci);
	}
}


//////////////////////////////////////////////////////////////////////
//


BOOL CArcParse::EnumArchive(HWND hwnd, LPCTSTR pszName, ENUMARCPROC callback, LPARAM param)
{
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(_uuidof(ArcManager));
	if (FAILED(hr))
		return FALSE;

	LONG type;
	BOOL unicode, utf8;
	hr = pam->GetArchiveType(pszName, &type, &unicode, &utf8);
	if (FAILED(hr) || type == ARC_ERROR || type == ARC_ISH)
		return FALSE;

	INDIVIDUALINFOW ii;
	ICommArchiver* pca;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, gszAst);
	if (type == ARC_7ZIP || type == ARC_ISO || type == ARC_RPM) {
		// 7zip は "*"
		buf[1] = 0;
	}
	hr = pam->BeginFind(hwnd, type, pszName, buf, (BYTE*)&ii, (IUnknown**)&pca);
	if (FAILED(hr))
		return FALSE;

	Wep();

	m_nType = type;
	m_bUnicodeSupported = unicode;
	m_bUtf8Supported = utf8;

	DWORD_PTR folderid = 1;
	m_strFile = pszName;
	CArcItem* pai;
	SHFILEINFO sfi;
	int serial = 0;
	TCHAR* p;
	do {
		lstrcpy(buf, ii.szFileName);

		pai = new CArcItem;
		pai->SetRealName(buf);
		replace_char(buf, _T('\\'), _T('/'));

		// キャンセルチェック
		if (callback != NULL) {
			if (callback(buf, param) != 0)
				return FALSE;
		}

		pca->GetAttribute(&pai->m_attr);
		if (pai->m_attr == (DWORD)-1)
			pai->m_attr = 0;

		// ちょっと特殊かな
		if (m_nType == ARC_LZH && !lstrcmpi(ii.szMode, _T("-lhd-")))
			pai->m_attr |= FILE_ATTRIBUTE_DIRECTORY;
		if (m_nType == ARC_TAR && buf[lstrlen(buf) - 1] == _T('/'))
			pai->m_attr |= FILE_ATTRIBUTE_DIRECTORY;

		// 暗号化
		if (pai->m_attr & FILE_ATTRIBUTE_ENCRYPTED)
			m_bHasEncryptedItems = true;

		p = StrRChr(buf, NULL, _T('/'));
		if (p) {
			if (*(p + 1) != 0) {			// '/'の位置が終端ではない
				pai->SetName(++p);
				*p = 0;
			}
			else {							// '/'が終端
				TCHAR* q = StrRChr(buf, p - 1, _T('/'));
				if (q)
					q++;
				else
					q = buf;

				TCHAR w[MAX_PATH * 2];
				lstrcpy(w, q);
				del_slash(w);
				pai->SetName(w);
				*q = 0;
			}
			pai->SetPath(buf);
		}
		else
			pai->SetName(buf);

		// フォルダは無条件に登録
		if (pai->IsFolder())
			MapFolder(pai->m_strRealName, folderid);
		// 親パスがあれば登録
		if (!pai->m_strPath.IsEmpty()) {
			MapFolder(pai->m_strPath, folderid);
			pai->m_key = m_folder.Lookup(pai->m_strPath);
		}

		pai->m_dwSize = ii.dwOriginalSize;
		pai->m_dwCompressedSize = ii.dwCompressedSize;
		pai->m_wDate = ii.wDate;
		pai->m_wTime = ii.wTime;
		pai->m_wRatio = ii.wRatio;
		pai->SetModeName(ii.szMode);
		pai->SetAttribString(ii.szAttribute);

		// メソッドモード詳細取得
		if (m_nType == ARC_7ZIP && pca->GetMethod(buf, ARRAYSIZE(buf)) == S_OK)
			pai->SetModeName(buf);

		// ファイルアイコンと種別名を取得
		SHGetFileInfo(pai->m_strName, pai->m_attr, &sfi, sizeof(sfi),
						SHGFI_SMALLICON | SHGFI_SYSICONINDEX |
						SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
		pai->m_iImage = sfi.iIcon;
		pai->SetTypeName(sfi.szTypeName);

		// 総合サイズへの加算
		m_dlTotal += pai->m_dwSize;

		pai->m_serial = serial++;
		m_array.Add(pai);

		hr = pam->NextFind(pca, (BYTE*)&ii);
	} while (SUCCEEDED(hr));
	pam->EndFind(pca);

	// 仮想フォルダをリストに加える
	int cnt = m_folder.GetSize();
	if (cnt > 0) {
		int i;
		for (i = 0; i < cnt; i++) {
			CString t = m_folder.GetKeyAt(i);
			int j;
			bool found = false;
			for (j = 0; j < m_array.GetSize(); j++) {
				pai = m_array[j];
				if (pai->IsFolder()) {
					CString s = pai->m_strRealName.GetString();
					s.Replace(_T('\\'), _T('/'));
					s.TrimRight(_T('/'));
					s += _T('/');
					if (!s.CompareNoCase(t)) {
						found = true;
						break;
					}
				}
			}
			if (!found) {
				pai = new CArcItem;
				pai->SetRealName(t);
				pai->m_attr = FILE_ATTRIBUTE_DIRECTORY;

				t.TrimRight(_T('/'));
				int pos = t.ReverseFind(_T('/'));
				if (pos < 0)
					pai->SetName(t);
				else {
					pai->SetName(t.Mid(pos + 1));
					t = t.Left(pos);
					t += _T('/');
					pai->SetPath(t);

					MapFolder(pai->m_strPath, folderid);
					pai->m_key = m_folder.Lookup(pai->m_strPath);
				}
				// ファイルアイコンと種別名を取得
				SHGetFileInfo(pai->m_strName, pai->m_attr, &sfi, sizeof(sfi),
						SHGFI_SMALLICON | SHGFI_SYSICONINDEX |
						SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
				pai->m_iImage = sfi.iIcon;
				pai->SetTypeName(sfi.szTypeName);

				pai->m_serial = serial++;
				m_array.Add(pai);
			}
		}
	}

	if (callback != NULL)
		callback(_T(""), param);
	return TRUE;
}

void CArcParse::SortItems(int type, int seq)
{
	CWaitCursor wc;
	int cnt = m_array.GetSize();
	if (cnt > 1) {
		int (*cmpfnc)(const void*, const void*, const void*);
		switch (type) {
			case SORT_FULLNAME: cmpfnc = sort_fullname; break;
			case SORT_PATH: cmpfnc = sort_path; break;
			case SORT_NAME: cmpfnc = sort_name; break;
			case SORT_EXT: cmpfnc = sort_ext; break;
			case SORT_SIZE: cmpfnc = sort_size; break;
			case SORT_TIME: cmpfnc = sort_time; break;
			case SORT_ATTR: cmpfnc = sort_attr; break;
			default: cmpfnc = sort_none; break;
		}

		CMPINF ci;
		ci.seq = seq;
		qsort_ex(m_array.GetData(), cnt, sizeof(CArcItem*), cmpfnc, &ci);
	}
}

void CArcParse::MakeSfx(HWND hwnd)
{
	if (!CanMakeSfx())
		return;

	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(_uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		TCHAR dir[MAX_PATH];
		PathGetParent(m_strFile, dir);
		pam->MakeSFX(hwnd, m_strFile, dir);
	}
}

BOOL CArcParse::DeleteItems(HWND hwnd, LPCTSTR pszItemList)
{
	if (!CanDelete())
		return FALSE;

	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(_uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		// コマンドライン引数のセットアップ
		TCHAR name[MAX_PATH * 2];
		lstrcpy(name, m_strFile);
		PathQuoteSpaces(name);

		CString cmd;
		int typeArc = m_nType;
		if (typeArc == ARC_LZH)
			cmd.Format(_T("d -n -r2 -p %s "), name);
		else if (typeArc == ARC_ZIP) {
			typeArc = ARC_ZIPPACK;
			cmd.Format(_T("-d --i %s "), name);
		}
		else if (typeArc == ARC_BGA)
			cmd.Format(_T("d -i %s "), name);
		else if (typeArc == ARC_7ZIP)
			cmd.Format(_T("d %s -ms=off "), name);

		cmd += pszItemList;
		hr = pam->ArchiveW(hwnd, typeArc, cmd, NULL, 0, NULL);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL CArcParse::RenameItems(HWND hwnd, LPCTSTR pszItemList)
{
	if (!CanRename())
		return FALSE;

	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(_uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		// コマンドライン引数のセットアップ
		CString cmd;
		if (m_nType == ARC_LZH)
			cmd.Format(_T("n -n \"%s\" "), (LPCTSTR)m_strFile);
		else if (m_nType == ARC_BGA)
			cmd.Format(_T("n \"%s\" "), (LPCTSTR)m_strFile);
		cmd += pszItemList;
		hr = pam->ArchiveW(hwnd, m_nType, cmd, NULL, 0, NULL);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL CArcParse::ListToClipboard(HWND hwnd)
{
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(_uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		ULONG size = 80L * (m_array.GetSize() + 10);
		HGLOBAL hmem = GlobalAlloc(GHND, size);
		if (hmem) {
			LPSTR pszResult = (LPSTR)GlobalLock(hmem);
			hr = pam->ListArchive(hwnd, m_nType, m_strFile, pszResult, size);
			GlobalUnlock(hmem);

			if (SUCCEEDED(hr)) 	{
				OpenClipboard(hwnd);
				EmptyClipboard();
				SetClipboardData(CF_TEXT, hmem);
				CloseClipboard();
			}
			else {
				GlobalFree(hmem);
				return FALSE;
			}
		}
	}
	return SUCCEEDED(hr) ? TRUE : FALSE;
}

int CArcParse::PackByFilter(CSimpleArray<CArcItem*>& arr, LPCTSTR pszFilter)
{
	arr.RemoveAll();
	int cnt = m_array.GetSize();
	for (int i = 0; i < cnt; i++) {
		CArcItem* pai = (CArcItem*)m_array[i];
		if (match(pai->m_strRealName, pszFilter) == 1)
			arr.Add(pai);
	}

	cnt = arr.GetSize();
	if (cnt > 1) {
		CMPINF ci;
		ci.seq = SORT_UP;
		qsort_ex(arr.GetData(), cnt, sizeof(CArcItem*), sort_fullname, &ci);
	}
	return (int)cnt;
}

void CArcParse::MapFolder(LPCTSTR pszDir, DWORD_PTR& folderid)
{
	CString name = pszDir;
	name.Replace(_T('\\'), _T('/'));

	CSimpleArray<CString> arr;
	int argc = split_pathcomponent(name, arr);
	if (argc > 0) {
		CString tmp;
		for (int i = 0; i < argc; i++) {
			tmp = arr[i];
			if (tmp != _T("/")) {
				tmp += _T('/');
				if (!m_folder.Lookup(tmp))
					m_folder.Add(tmp, folderid++);
			}
		}
	}
}

BOOL CArcParse::IsTypeDragSource()
{
	switch (m_nType) {
	case ARC_ZIP:
	case ARC_LZH:
	case ARC_CAB:
	case ARC_TAR:
	case ARC_7ZIP:
	case ARC_ISO:
	case ARC_RPM:
	case ARC_RAR:
	case ARC_ARJ:
	case ARC_GCA:
	case ARC_BGA:
	case ARC_YZ1:
		return TRUE;
	}
	return FALSE;
}

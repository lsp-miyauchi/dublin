// finfo.cpp : implementation file
//

#include "stdafx.h"

DEFINE_PROPERTYKEY(PKEY_Software_ProductVersion, 0x0CEF7D53, 0xFA64, 0x11D1, 0xA2, 0x03, 0x00, 0x00, 0xF8, 0x1F, 0xED, 0xEE, 8);

#include <atldlgs.h>
#include <dbghelp.h>
#include "fopstd.h"
#include "dlgtab.h"
#include "iconres.h"
#include "dirgrep.h"
#include "reparse.h"
#include "gdiplus_sup.h"
#include "..\lspexif\lspexifapi.h"
#include "drgdrp.h"
#include "finfo.h"


/////////////////////////////////////////////////////////////////////
// バージョン情報ピックアップ

typedef struct tagVERSION_SCID {
	LPCTSTR label;
	SHCOLUMNID scid;
} VERSION_SCID;

static const VERSION_SCID vsPickup[] = {
	{ MAKEINTRESOURCE(IDS_VER_PRODUCTNAME),		PKEY_Software_ProductName },	// 製品名
	{ MAKEINTRESOURCE(IDS_VER_DESCRIPTION),		PKEY_FileDescription },			// 説明
	{ MAKEINTRESOURCE(IDS_VER_FILEVERSION),		PKEY_FileVersion },				// ファイルバージョン
	{ MAKEINTRESOURCE(IDS_VER_PRODUCTVERSION),	PKEY_Software_ProductVersion },	// 製品バージョン
	{ MAKEINTRESOURCE(IDS_VER_COPYRIGHT),		PKEY_Copyright },				// 著作権
	{ MAKEINTRESOURCE(IDS_VER_COMPANY),			PKEY_Company },					// 会社名
};


/////////////////////////////////////////////////////////////////////
//

const static TCHAR sszLinkTo[] = _T("リンク先");
const static TCHAR sszTypes[] = _T("種類");
const static TCHAR sszOtherProps[] = _T("その他のプロパティ");

void ShowIconDetail(HWND hwnd, const ICONRESOURCE* pir);
void CopyIconsToClipboard(HICON* phics, int numIcons, COLORREF clrBack);


/////////////////////////////////////////////////////////////////////
//

class CFileInfoList	: public CSimpleArray<LPITEMIDLIST>
{
public:
	int m_pageIndex;
	CPageBar* m_pbar;

	CFileInfoList() : m_pageIndex(0), m_pbar(NULL) { }
	int SetItems(int pageindex, LPITEMIDLIST* pidls, int numItems)
	{
		m_pageIndex = pageindex;
		for (int i = 0; i < numItems; i++)
			Add(pidls[i]);
		return GetSize();
	}

	bool canForward() const { return (m_pageIndex < GetSize() - 1); }
	int Forward() {
		if (canForward()) m_pageIndex++;
		return m_pageIndex;
	}
	bool canBack() const { return m_pageIndex > 0 && m_pageIndex < GetSize(); }
	int Back() {
		if (canBack()) m_pageIndex--;
		return m_pageIndex;
	}
};


/////////////////////////////////////////////////////////////////////
// 一般情報ページ

static SIZE calcscale(int width, int height, int cx, int cy)
{
	SIZE s;
	s.cx = cx;
	s.cy = cy;
	if (cx > width || cy > height) {
		for (float f = 1.25F; ; f += 0.25F) {
			s.cx = (int)((float)cx / f);
			s.cy = (int)((float)cy / f);
			if (s.cx <= width && s.cy <= height)
				break;
		}
	}
	return s;
}

////////////////////////////////////////////////////////////////////////
//

static int CALLBACK ExifProc(int type, LPCTSTR pszTitle, LPCTSTR pszText, LPARAM param)
{
	CInfoListViewCtrl* plist = (CInfoListViewCtrl*)param;

	switch (type) {
	case EXIFCB_GROUP:	plist->AddItemOrGroup(true, pszTitle, NULL); break;
	case EXIFCB_ITEM:	plist->AddItemOrGroup(false, pszTitle, pszText); break;
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////

#pragma pack(1)
typedef struct tagPROPERTYKEY_STRUCT {
	DWORD p1;
	WORD p2;
	WORD p3;
	BYTE p4[9];
} PROPERTYKEY_STRUCT;
#pragma pack()

static void dump_scid(const SHCOLUMNID& scid)
{
	LPWSTR pwz;
	if (PSGetNameFromPropertyKey(scid, &pwz) == S_OK) {
		WCHAR wsz[256];
		PSStringFromPropertyKey(scid, wsz, ARRAYSIZE(wsz));
		dbg.printf(L"%s\n   %s\n", pwz, wsz);
		CoTaskMemFree(pwz);
	}
}

static const SHCOLUMNID sscScids[] = {
	PKEY_FileName,							// ファイル名
	PKEY_ItemFolderPathDisplayNarrow,		// フォルダ
	PKEY_ItemFolderNameDisplay,				// フォルダ名
	PKEY_ItemPathDisplay,					// 場所
	PKEY_ItemPathDisplayNarrow,				// パス
	PKEY_FileExtension,						// 拡張子

};

static bool GetPropertyNameAndValue(IPropertyStore* pps, REFPROPERTYKEY scid, CString& name, CString& value)
{
	bool ok = false;
	PROPVARIANT prop = { 0 };
	if (pps->GetValue(scid, &prop) == S_OK) {
		IPropertyDescription* ppd;
		if (PSGetPropertyDescription(scid, IID_IPropertyDescription, (void**)&ppd) == S_OK) {
			LPWSTR pwz;
			if (ppd->GetDisplayName(&pwz) == S_OK) {
				name = pwz;
				CoTaskMemFree(pwz);

/* --- こういうやり方は邪道？ ---
if (scid == PKEY_Rating) {
	if (prop.ulVal < 13)
		value = _T("★☆☆☆☆");
	else if (prop.ulVal < 38)
		value = _T("★★☆☆☆");
	else if (prop.ulVal < 63)
		value = _T("★★★☆☆");
	else if (prop.ulVal < 88)
		value = _T("★★★★☆");
	else
		value = _T("★★★★☆");
	ok = true;
}
else
------------------------------- */
				if (ppd->FormatForDisplay(prop, PDFF_DEFAULT, &pwz) == S_OK) {
					value = pwz;
					CoTaskMemFree(pwz);
					ok = true;
				}
			}
			ppd->Release();
		}
		PropVariantClear(&prop);
	}
	return ok;
}

static int SetShellColumnInformation(CInfoListViewCtrl& list, LPCITEMIDLIST pidlFq, CSimpleArray<SHCOLUMNID>* pscids)
{
	// カラム情報
	int numAdd = 0;
	if (pidlFq) {
		IShellFolder2* psf2;
		LPCITEMIDLIST pidl;
		if (SHBindToParent(pidlFq, IID_IShellFolder2, (void**)&psf2, &pidl) == S_OK) {
			CSimpleMap<CString, CString> arr;

			// カラム詳細を取得
			SHELLDETAILS sdt;
			SHCOLUMNID scid;
			int i = 0;
			while (psf2->MapColumnToSCID(i, &scid) == S_OK) {
				// 対象外かチェック
				if (!pscids || pscids->Find(scid) < 0) {
					LPTSTR tit = NULL;
					LPTSTR str = NULL;
					if (psf2->GetDetailsOf(NULL, i, &sdt) == S_OK && StrRetToStr(&sdt.str, NULL, &tit) == S_OK && tit) {
						if (psf2->GetDetailsOf(pidl, i, &sdt) == S_OK && StrRetToStr(&sdt.str, pidl, &str) == S_OK && str) {
							if (*str) {
								// 項目を記録
								if (numAdd == 0)
									list.AddItemOrGroup(true, sszOtherProps, NULL);
								list.AddItemOrGroup(false, tit, str);
								// 出力済みリストに追加
								if (pscids) pscids->Add(scid);
								numAdd++;
							}
							CoTaskMemFree(str);
						}
						CoTaskMemFree(tit);
					}
				}
				i++;
			}
			psf2->Release();
		}
	}
	return numAdd;
}

static int SetShellPropertyInformation(CInfoListViewCtrl& list, LPCITEMIDLIST pidlFq, CSimpleArray<SHCOLUMNID>* pscids)
{
	// 表示しなくてもいい項目を予めセットしておく
	if (pscids) {
		for (int i = 0; i < ARRAYSIZE(sscScids); i++) {
			if (pscids->Find(sscScids[i]) < 0)
				pscids->Add(sscScids[i]);
		}
	}
	// まずシェルカラム情報を列挙
	int numAdd = SetShellColumnInformation(list, pidlFq, pscids);
	// プロパティ情報
	if (pidlFq) {
		IPropertyStore* pps;
		if (SHGetPropertyStoreFromIDList(pidlFq, GPS_DEFAULT, IID_IPropertyStore, (void**)&pps) == S_OK) {
			DWORD numProp;
			if (pps->GetCount(&numProp) == S_OK && numProp > 0) {
				PROPERTYKEY scid;
				for (DWORD i = 0; i < numProp; i++) {
					// scid取得
					if (pps->GetAt(i++, &scid) != S_OK) continue;
					// 対象外かチェック
					if (pscids && pscids->Find(scid) >= 0) continue;

					CString name, value;
					if (GetPropertyNameAndValue(pps, scid, name, value)) {
						// 項目を記録
						if (numAdd == 0)
							list.AddItemOrGroup(true, sszOtherProps, NULL);
						list.AddItemOrGroup(false, name, value);
						numAdd++;
						// 出力済みリストに追加
						if (pscids) pscids->Add(scid);
					}
				}
			}
			pps->Release();
		}
	}
	return numAdd;
}

static void SetOverviewInformation(CInfoListViewCtrl& list, LPCITEMIDLIST pidlFq, CSimpleArray<SHCOLUMNID>* pscids)
{
	TCHAR name[MAX_PATH * 2];
	if (!SHGetPathFromIDList(pidlFq, name))
		name[0] = 0;

	bool bVersionInfoAdded = false;
	CString str, txt;
	CFileVersion ver;
	if (ver.Open(name)) {
		// バージョン情報
		str.LoadString(IDS_VER_OVERVIEW);
		list.AddItemOrGroup(true, str, NULL);
		// ファイルバージョン
		if (ver.GetFileVersion(txt)) {
			str.LoadString(IDS_VER_FILEVERSION);
			list.AddItemOrGroup(false, str, txt);
			if (pscids) pscids->Add(PKEY_FileVersion);
		}
		// 説明
		if (ver.GetFileDescription(txt)) {
			str.LoadString(IDS_VER_DESCRIPTION);
			list.AddItemOrGroup(false, str, txt);
			if (pscids) pscids->Add(PKEY_FileDescription);
		}
		// 著作権
		if (ver.GetLegalCopyright(txt)) {
			str.LoadString(IDS_VER_COPYRIGHT);
			list.AddItemOrGroup(false, str, txt);
			if (pscids) pscids->Add(PKEY_Copyright);
		}
		// 会社名
		if (ver.GetCompanyName(txt)) {
			str.LoadString(IDS_VER_COMPANY);
			list.AddItemOrGroup(false, str, txt);
			if (pscids) pscids->Add(PKEY_Company);
		}
		// 商標
		if (ver.GetLegalTradeMarks(txt)) {
			str.LoadString(IDS_VER_TRADEMARK);
			list.AddItemOrGroup(false, str, txt);
			if (pscids) pscids->Add(PKEY_Trademarks);
		}
		// 製品バージョン
		if (ver.GetProductVersion(txt)) {
			str.LoadString(IDS_VER_PRODUCTVERSION);
			list.AddItemOrGroup(false, str, txt);
			if (pscids) pscids->Add(PKEY_Software_ProductVersion);
		}
		// 製品名
		if (ver.GetProductName(txt)) {
			str.LoadString(IDS_VER_PRODUCTNAME);
			list.AddItemOrGroup(false, str, txt);
			if (pscids) pscids->Add(PKEY_Software_ProductName);
		}
		// 正式ファイル名
		if (ver.GetOriginalFileName(txt)) {
			str.LoadString(IDS_VER_REGALNAME);
			list.AddItemOrGroup(false, str, txt);
		}
		// 内部名
		if (ver.GetInternalName(txt)) {
			str.LoadString(IDS_VER_INTERNALNAME);
			list.AddItemOrGroup(false, str, txt);
		}
		// コメント
		if (ver.GetComments(txt)) {
			str.LoadString(IDS_VER_COMMENT);
			list.AddItemOrGroup(false, str, txt);
			if (pscids) pscids->Add(PKEY_Comment);
		}
		// ファイルＯＳ
		if (ver.GetFileOS(txt)) {
			str.LoadString(IDS_VER_FILEOS);
			list.AddItemOrGroup(false, str, txt);
		}
		ver.Close();
		bVersionInfoAdded = true;
	}

	// エクスポート関数
#ifndef _WIN64
	// dbghelp.dll をロード
	CDll dll;
	if (name[0] && dll.LoadLibrary(_T("dbghelp.dll")) != NULL) {
		// 関数アドレス取得
		IMAGE_EXPORT_DIRECTORY* (WINAPI* pfn)(PVOID, BOOLEAN, USHORT, PULONG);
		(FARPROC&)pfn = dll.FindFunction("ImageDirectoryEntryToData");
		if (pfn) {
			// 列挙開始
			HMODULE hmod = ::LoadLibraryEx(name, NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (hmod) {
				ULONG nSize;
				IMAGE_EXPORT_DIRECTORY*	pImageExportDir = pfn((PVOID)hmod,TRUE,IMAGE_DIRECTORY_ENTRY_EXPORT, &nSize);
				if (pImageExportDir) {
					// pImageExportDir内のアドレスはhModuleからの相対的なアドレス
					char** ppszFunctionName = (char**)(pImageExportDir->AddressOfNames + (ULONGLONG)hmod);
					WORD* pwFunctionOrdinal = (WORD*)(pImageExportDir->AddressOfNameOrdinals + (ULONGLONG)hmod);
					bool bInitExports = false;
					DWORD i, j;
					for (i = 0; i < pImageExportDir->NumberOfFunctions; i++) {
						for (j = 0; j < pImageExportDir->NumberOfNames; j++) {
							if (pwFunctionOrdinal[j] != i)		// pwFunctionOrdinal[j]は序数ではなくインデックス
								continue;
							// hModuleからの相対アドレスで関数名が格納されている
							if (!bInitExports) {
								str.LoadString(IDS_EXPORT_FUNCTION);
								list.AddItemOrGroup(true, str, NULL);
								bInitExports = true;
							}

							txt.Format(IDS_EXPORT_NUMBER, pImageExportDir->Base + i);
							str = (char*)(ppszFunctionName[j] + (ULONGLONG)hmod);
							list.AddItemOrGroup(false, str, txt);
							break;
						}
					}
				}
				::FreeLibrary(hmod);
			}
		}
	}
#endif

	// Exifチェック
	if (name[0] && !bVersionInfoAdded) {
		CComPtr<IExifParser> pep;
		HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
		if (SUCCEEDED(hr)) {
			if (pep->IsExifFile(name) == S_OK) {
				pep->EnumExifInfo(name, (UINT_PTR)ExifProc, (LPARAM)&list);
				list.AutoSizeColumns();
				return;
			}
		}
	}

	// その他のカラム情報
	SetShellPropertyInformation(list, pidlFq, pscids);

	// VISTAではファイルに存在するストリームを列挙できる
	if (name[0]) {
		HANDLE (WINAPI* _fFindFirstStreamW)(LPCWSTR, STREAM_INFO_LEVELS, LPVOID, DWORD);
		BOOL (WINAPI* _fFindNextStreamW)(HANDLE, LPVOID);

		CDllHandle dll = GetModuleHandle(_T("kernel32.dll"));
		(FARPROC&)_fFindFirstStreamW = dll.FindFunction("FindFirstStreamW");
		(FARPROC&)_fFindNextStreamW = dll.FindFunction("FindNextStreamW");
		if (_fFindFirstStreamW && _fFindNextStreamW) {
			WIN32_FIND_STREAM_DATA sd;
			HANDLE hFind = _fFindFirstStreamW(name, FindStreamInfoStandard, (void*)&sd, 0);
			if (hFind != INVALID_HANDLE_VALUE) {
				list.AddItemOrGroup(true, _T("ストリーム情報"), NULL);
				do {
					// ストリームサイズ
					FileSizeInformation(sd.StreamSize.QuadPart, str);

					// streamname is ":<name>:<type>"
					LPTSTR sname = sd.cStreamName;
					LPTSTR p = StrChrW(sname, _T(':'));
					if (p) {
						sname = ++p;
						p = StrChrW(p, _T(':'));
						if (p) {
							*p++ = 0;
							str.Insert(0, _T(' '));
							str.Insert(0, p);
						}
					}

					// ストリームに名前がない場合
					if (*sname == 0)
						sname = _T("(noname)");
					list.AddItemOrGroup(false, sname, str);
				} while (_fFindNextStreamW(hFind, &sd));
				FindClose(hFind);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
//


class CGenericPage : public CDialogImpl<CGenericPage>
{
public:
	enum { IDD = IDD_FINFO_GENERIC };

	CString m_name;
	CString m_type;
	LPCITEMIDLIST m_pidl;
	CInfoListViewCtrl m_list;
	CDirGrepThread* m_grep;
	UINT_PTR m_timerid;
	CSimpleArray<SHCOLUMNID> m_scids;

	int m_indexType;
	int m_indexSize;
	int m_indexDiskSize;

	CGenericPage()
	{
		m_grep = NULL;
		m_timerid = 0;
		m_pidl = NULL;
	}

BEGIN_MSG_MAP(CGenericPage)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_MESSAGE_VOID(WM_ON_INIT, OnInit)
	REFLECT_NOTIFY_ID(IDC_F1_LIST)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnTimer(UINT_PTR nIDEvent);
	void OnInit(WPARAM wParam, LPARAM lParam);
};

BOOL CGenericPage::OnInitDialog(LPARAM lParam)
{
	m_list.Initialize(GetDlgItem(IDC_F1_LIST));
	return TRUE;
}

void CGenericPage::OnDestroy()
{
	if (m_timerid) KillTimer(m_timerid);
	if (m_grep) delete m_grep;
}

void CGenericPage::OnTimer(UINT_PTR idEvent)
{
	if (idEvent == m_timerid && m_grep) {
		DWORDLONG tot, occ;
		int fil, fol;
		m_grep->GetSizes(&fil, &fol, &tot, &occ);

		// サイズ
		CString str;
		FileSizeInformation(tot, str);
		m_list.SetItemText(m_indexSize, 1, str);

		// 占有サイズ
		FileSizeInformation(occ, str);
		m_list.SetItemText(m_indexDiskSize, 1, str);

		// ファイル・フォルダ数
		str.Format(_T("%s ( ファイル数: %d, フォルダ数: %d )"), (LPCTSTR)m_type, fil, fol);
		m_list.SetItemText(m_indexType, 1, str);

		if (!m_grep->IsRunning()) {
			KillTimer(m_timerid);
			m_timerid = 0;
			delete m_grep;
			m_grep = NULL;
		}

		m_list.AutoSizeColumns();
	}
}

void CGenericPage::OnInit(WPARAM wParam, LPARAM lParam)
{
	if (::ILIsEqual(m_pidl, (LPCITEMIDLIST)lParam)) return;

	m_pidl = (LPCITEMIDLIST)lParam;
	m_name.Empty();
	SHGetPathFromIDList(m_pidl, m_name.GetBuffer(MAX_PATH * 2));
	m_name.ReleaseBuffer();

	// フォルダ計算スレッドが動作中であれば止める
	if (m_grep) {
		delete m_grep;
		m_grep = NULL;
	}

	m_scids.RemoveAll();
	m_list.ResetItems();

	// 全般情報
	TCHAR buf[MAX_PATH * 2];
	CString str; str.LoadString(IDS_F1_GENERIC);
	m_list.AddItemOrGroup(true, str, NULL);

	CWin32FindData wfd;
	DWORD code = _getfileinfo(m_name, &wfd);
	if (code != NOERROR) {
		SetShellPropertyInformation(m_list, m_pidl, &m_scids);
		m_list.AutoSizeColumns();
		return;
	}

	// ファイルパス
	PathGetParent(m_name, buf);
	CFileInfo fi;
	fi.SetFile(wfd, buf);

	// 場所
	str.LoadString(IDS_F1_PATH);
	PathGetParent(m_name, buf);
	m_list.AddItemOrGroup(false, str, buf);
	m_scids.Add(PKEY_ItemFolderPathDisplay);

	// 種別
	str.LoadString(IDS_F1_TYPENAME);
	SHFILEINFO sfi;
	SHGetFileInfo(m_name, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);
	m_type = sfi.szTypeName;
	m_indexType = m_list.AddItemOrGroup(false, str, sfi.szTypeName);
	m_scids.Add(PKEY_ItemTypeText);

	// DOS名
	str.LoadString(IDS_F1_DOSNAME);
	m_list.AddItemOrGroup(false, str, wfd.cAlternateFileName);

	// 作成日付
	str.LoadString(IDS_F1_CREATIONTIME);
	FormatFileDateTimeEx(&fi.m_ftCreationTime, buf, ARRAYSIZE(buf));
	m_list.AddItemOrGroup(false, str, buf);
	m_scids.Add(PKEY_DateCreated);

	// 更新日付
	str.LoadString(IDS_F1_UPDATETIME);
	FormatFileDateTimeEx(&fi.m_ftLastWriteTime, buf, ARRAYSIZE(buf));
	m_list.AddItemOrGroup(false, str, buf);
	m_scids.Add(PKEY_DateModified);

	// サイズ
	str.LoadString(IDS_F1_SIZE);
	CString fsize;
	FileSizeInformation(fi.m_dlSize, fsize);
	m_indexSize = m_list.AddItemOrGroup(false, str, fsize);
	m_scids.Add(PKEY_Size);

	// 圧縮ファイルのチェック
	DWORDLONG dl = fi.m_dlSize;
	if (fi.IsAttrib(FA_CMPRSD)) {
		DWORD hi, lo;
		lo = GetCompressedFileSize(m_name, &hi);
		if (lo != 0xffffffff || GetLastError() == NO_ERROR)
			SETDWORDLONG(dl, lo, hi);
	}

	// ディスク情報
	// 占有サイズ
	str.LoadString(IDS_F1_DISKSIZE);
	CDiskSpace ds(m_name, true);
	FileSizeInformation(ds.ToClustersBytes(dl), fsize);
	if (!fi.IsFolder() && fi.m_dlSize != dl)
		fsize.AppendFormat(_T(" 圧縮率 %d%%"), (int)(dl * 100 / fi.m_dlSize));
	m_indexDiskSize = m_list.AddItemOrGroup(false, str, fsize);

	// 属性
	CString satt;
/*
	if (fi.IsAttrib(FILE_ATTRIBUTE_SYSTEM))					satt += _T("SYSTEM,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_HIDDEN))					satt += _T("HIDDEN,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_READONLY))				satt += _T("READ ONLY,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_ARCHIVE))				satt += _T("ARCHIVE,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_COMPRESSED))				satt += _T("COMPRESSED,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_ENCRYPTED))				satt += _T("ENCRYPTED,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_REPARSE_POINT))			satt += _T("REPARS POINT,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_OFFLINE))				satt += _T("OFFLINE,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_SPARSE_FILE))			satt += _T("SPARSE,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_NOT_CONTENT_INDEXED))	satt += _T("NC INDEXED,");
*/
	if (fi.IsAttrib(FILE_ATTRIBUTE_SYSTEM))					satt += _T("システム,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_HIDDEN))					satt += _T("隠しファイル,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_READONLY))				satt += _T("読み取り専用,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_ARCHIVE))				satt += _T("アーカイブ,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_COMPRESSED))				satt += _T("圧縮,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_ENCRYPTED))				satt += _T("暗号化,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_REPARSE_POINT))			satt += _T("再解析ポイント,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_OFFLINE))				satt += _T("オフライン,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_SPARSE_FILE))			satt += _T("疎ファイル,");
	if (fi.IsAttrib(FILE_ATTRIBUTE_NOT_CONTENT_INDEXED))	satt += _T("インデクス対象外,");
	satt.TrimRight(_T(','));
	str.LoadString(IDS_F1_ATTRIBUTES);
	m_list.AddItemOrGroup(false, str.GetString(), satt.GetString());
	m_scids.Add(PKEY_FileAttributes);

	// 所有者
	str.LoadString(IDS_F1_OWNER);
	if (!ds.isNTFS() || !lccGetFileCreateOwner(m_name, buf))
		buf[0] = 0;
	m_list.AddItemOrGroup(false, str, buf);
	m_scids.Add(PKEY_FileOwner);

	// その他の概要情報
	SetOverviewInformation(m_list, m_pidl, &m_scids);

	m_list.AutoSizeColumns();
	// フォルダであれば配下のサイズを取得
	if (fi.IsFolder()) {
		m_grep = new CDirGrepThread;
		m_grep->Start(m_hWnd, m_name);
		m_timerid = SetTimer(999, 1000, NULL);
	}
}

/////////////////////////////////////////////////////////////////////
//

class CIconPage : public CDialogImpl<CIconPage>
{
public:
	enum { IDD = IDD_FINFO_ICON };
	CString m_name;
	CLccSliderCtrl m_slider;
	CListViewCtrl m_list;
	CIconListManager m_iconman;

	CIconPage() {}
	~CIconPage() {}

BEGIN_MSG_MAP(CIconPage)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_MESSAGE_VOID(WM_ON_INIT, OnInit)
	ON_BN_CLICKED(IDC_F3_WRITE, OnExportIcons)
	ON_NOTIFY(NM_DBLCLK, IDC_F3_LIST, OnNotifyDblClk)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_F3_LIST, OnNotifyBeginDrag)
	ON_BN_CLICKED(IDC_F3_DETAIL, OnShowIconDetail)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	inline void OnNotifyDblClk(NMHDR* phdr, LRESULT* pResult) { OnShowIconDetail(); }
	void OnNotifyBeginDrag(NMHDR* pNmhdr, LRESULT* pResult);
	void OnInit(WPARAM wParam, LPARAM lParam);
	void OnExportIcons();
	void OnShowIconDetail();
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hctl);
	void ChangeIconView(int dx);
};

BOOL CIconPage::OnInitDialog(LPARAM lParam)
{
	m_slider = GetDlgItem(IDC_F3_SIZE);
	m_slider.SetRange(0, 3);
	m_slider.SetPos(2);
	m_list = GetDlgItem(IDC_F3_LIST);
	m_list.SetViewType(LVS_ICON);
	m_list.SetExtendedListViewStyle(m_list.GetExtendedListViewStyle() | LVS_EX_DOUBLEBUFFER);

	return TRUE;
}

void CIconPage::OnInit(WPARAM wParam, LPARAM lParam)
{
	TCHAR name[MAX_PATH * 2];
	if (!SHGetPathFromIDList((LPCITEMIDLIST)lParam, name) || !m_name.CompareNoCase(name)) return;

	m_name = name;
	GetDlgItem(IDC_F3_COPY).EnableWindow(FALSE);
	GetDlgItem(IDC_F3_WRITE).EnableWindow(FALSE);
	GetDlgItem(IDC_F3_DETAIL).EnableWindow(FALSE);

	CWaitCursor wc;
	int numIcons = m_iconman.Initialize(m_name);
	if (numIcons > 0) {
		GetDlgItem(IDC_F3_COPY).EnableWindow(TRUE);
		GetDlgItem(IDC_F3_WRITE).EnableWindow(TRUE);
		GetDlgItem(IDC_F3_DETAIL).EnableWindow(TRUE);
		for (int i = 0; i < numIcons; i++) {
			m_list.InsertItem(LVIF_IMAGE, i, NULL, 0, 0, i, 0);
		}
		m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ChangeIconView(32);
	}
}

void CIconPage::OnHScroll(UINT nSBCode, UINT nPos, HWND hctl)
{
	if (hctl != m_slider) {
		DefWindowProc();
		return;
	}

	int val;
	switch (nSBCode) {
		case TB_THUMBPOSITION: val = (int)nPos; break;
		case TB_TOP:
		case TB_BOTTOM:
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_PAGEUP:
		case TB_PAGEDOWN: val = m_slider.GetPos(); break;
		default: return;
	}
	int dx;
	switch (val) {
		case 0:  dx = 16; break;
		case 1:  dx = 24; break;
		case 2:  dx = 32; break;
		default: dx = 48; break;
	}

	ChangeIconView(dx);
}

void CIconPage::ChangeIconView(int dx)
{
	HIMAGELIST him = m_iconman.GetImageList(dx);
	m_list.SetIconSpacing(dx + 8, dx + 8);
	m_list.SetImageList(him, LVSIL_NORMAL);

	int index = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (index >= 0) {
		m_list.EnsureVisible(index, TRUE);
	}
}

void CIconPage::OnNotifyBeginDrag(NMHDR* /*pNmhdr*/, LRESULT* /*pResult*/)
{
	CPoint pt;
	GetCursorPos(&pt);

	// ドラッグ＆ドロップ処理
	int cnt = m_list.GetSelectedCount();
	if (cnt < 1) return;

	CSimpleArray<CIconItem*> arr;
	int index = -1;
	while (1) {
		index = m_list.GetNextItem(index, LVNI_ALL | LVNI_SELECTED);
		if (index < 0)
			break;
		arr.Add( m_iconman.m_arrIcon[index]);
	}

	if (lccBeginTrack(m_hWnd, pt)) {
		CDataObject* pdo = new CDataObject(m_name, &arr);
		DWORD eff = DROPEFFECT_COPY;
		SHDoDragDrop(m_hWnd, pdo, NULL, eff, &eff);
		pdo->Release();
	}
}

void CIconPage::OnExportIcons()
{
	LPCTSTR pszIconPath = m_iconman.GetIconPath();
	if (pszIconPath == NULL)
		return;

	int cnt = m_list.GetSelectedCount();
	if (cnt < 1) {
		AppMessage(IDR_FOPSERV, IDS_NOSELICONS, NULL, MB_STOP);
		return;
	}

	CSimpleArray<int> arrSel;
	int n = -1;
	for (int i = 0; i < cnt; i++) {
		arrSel.Add( m_list.GetNextItem(n, LVNI_SELECTED) );
	}
	TCHAR path[MAX_PATH * 2];
	PathGetParent(m_name, path);

	if (cnt == 1) {
		TCHAR name[MAX_PATH * 2];
		lstrcpy(name, m_name);
		wsprintf(&name[lstrlen(name)], _T(".(#%d).ico"), arrSel[0]);
		if (FileBrowse(FALSE, m_hWnd, IDS_TITLE_ICONPATH, IDS_FILTER_ICON, path, name))
			ExportSingleIcon(pszIconPath, name, arrSel[0]);
	} else {
		if (fopBrowseForFolder(m_hWnd, path)) {
			ExportMultipleIcons(pszIconPath, path, arrSel.GetData(), cnt);
		}
	}
}

void CIconPage::OnShowIconDetail()
{
	LPCTSTR pszIconPath = m_iconman.GetIconPath();
	if (pszIconPath == NULL)
		return;

	int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (sel < 0) {
		AppMessage(IDR_FOPSERV, IDS_NOSELICONS, NULL, MB_STOP);
		return;
	}
	fopShowIconDetail(m_hWnd, pszIconPath, sel);
}

/////////////////////////////////////////////////////////////////////
// 関連付け・起動情報


class CAssocPage : public CDialogImpl<CAssocPage>
{
public:
	enum { IDD = IDD_FINFO_ASSOC };
	LPCITEMIDLIST m_pidl;
	CInfoListViewCtrl m_list;

	CAssocPage() : m_pidl(NULL) { }

BEGIN_MSG_MAP(CAssocPage)
	ON_WM_INITDIALOG()
	ON_MESSAGE_VOID(WM_ON_INIT, OnInit)
	REFLECT_NOTIFY_ID(IDC_F4_LIST)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnInit(WPARAM wParam, LPARAM lParam);
};

BOOL CAssocPage::OnInitDialog(LPARAM lParam)
{
	m_list.Initialize(GetDlgItem(IDC_F4_LIST));
	return TRUE;
}

void CAssocPage::OnInit(WPARAM wParam, LPARAM lParam)
{
	if (::ILIsEqual(m_pidl, (LPCITEMIDLIST)lParam)) return;

	m_pidl = (LPCITEMIDLIST)lParam;
	m_list.ResetItems();

	// ターゲットファイルを起動するアプリのパス名を取得してバージョン情報を列挙
	CString str, txt;
	IShellFolder2* psf2;
	LPCITEMIDLIST pidl;
	TCHAR name[MAX_PATH * 2], exe[MAX_PATH * 2];
	if (SHGetPathFromIDList(m_pidl, name) && fopFindExecutable(name, exe)) {
		LPITEMIDLIST pidlFq = ILCreateFromPath(exe);
		if (pidlFq) {
/* --- vista以降ならこちらのコード -----------
			IPropertyStore* pps = NULL;
			if (grSHGetPropertyStoreFromIDList && grSHGetPropertyStoreFromIDList(pidlFq, GPS_DEFAULT, IID_IPropertyStore, (void**)&pps) == S_OK) {
				PROPVARIANT prop = { 0 };
				int cnt = 0;
				for (int i = 0; i < ARRAYSIZE(vsPickup); i++) {
					if (pps->GetValue(vsPickup[i].scid, &prop) == S_OK) {
//						HRESULT hr = PropVariantToString(prop, txt.GetBuffer(256), 256);
//						txt.ReleaseBuffer();
						txt = prop.bstrVal;
						if (!txt.IsEmpty()) {
							if (cnt == 0) {
								str.LoadString(IDS_F4_EXECUTABLE);
								m_list.AddItemOrGroup(true, str, NULL);
							}
							m_list.AddItemOrGroup(false, vsPickup[i].label, txt);
							cnt++;
						}
						PropVariantClear(&prop);
					}
				}
				if (cnt > 0 && pps->GetValue(PKEY_ItemPathDisplay, &prop) == S_OK) {
					// 場所情報
					txt = prop.bstrVal;
					m_list.AddItemOrGroup(false, MAKEINTRESOURCE(IDS_F4_PATH), txt);
					PropVariantClear(&prop);
				}
				pps->Release();
			}
			else
------------------------------------------ */
			if (SHBindToParent(pidlFq, IID_IShellFolder2, (void**)&psf2, &pidl) == S_OK) {
				VARIANT vr;
				int cnt = 0;
				for (int i = 0; i < ARRAYSIZE(vsPickup); i++) {
					if (psf2->GetDetailsEx(pidl, &vsPickup[i].scid, &vr) == S_OK) {
						txt = vr.bstrVal;
						if (!txt.IsEmpty()) {
							if (cnt == 0) {
								str.LoadString(IDS_F4_EXECUTABLE);
								m_list.AddItemOrGroup(true, str, NULL);
							}
							m_list.AddItemOrGroup(false, vsPickup[i].label, txt);
							cnt++;
						}
						VariantClear(&vr);
					}
				}
				if (cnt > 0 && psf2->GetDetailsEx(pidl, &PKEY_ItemPathDisplay, &vr) == S_OK) {
					// 場所情報
					txt = vr.bstrVal;
					m_list.AddItemOrGroup(false, MAKEINTRESOURCE(IDS_F4_PATH), txt);
					VariantClear(&vr);
				}
				psf2->Release();
			}
			CoTaskMemFree(pidlFq);
		}
	}

	if (SHBindToParent(m_pidl, IID_IShellFolder2, (void**)&psf2, &pidl) == S_OK) {
		IQueryAssociations* pqa;
		if (NOERROR == psf2->GetUIObjectOf(NULL, 1, &pidl, IID_IQueryAssociations, 0, (void**)&pqa)) {

			typedef struct _ENUMASSOCSTR {
				ASSOCSTR idType;
				LPCTSTR pszResource;
			} ENUMASSOCSTR;

			static ENUMASSOCSTR assocs[] = {
				ASSOCSTR_COMMAND,			MAKEINTRESOURCE(IDS_ASSOCSTR_COMMAND),
				ASSOCSTR_EXECUTABLE,		MAKEINTRESOURCE(IDS_ASSOCSTR_EXECUTABLE),
				ASSOCSTR_FRIENDLYDOCNAME,	MAKEINTRESOURCE(IDS_ASSOCSTR_FRIENDLYDOCNAME),
				ASSOCSTR_FRIENDLYAPPNAME,	MAKEINTRESOURCE(IDS_ASSOCSTR_FRIENDLYAPPNAME),
				ASSOCSTR_CONTENTTYPE,		MAKEINTRESOURCE(IDS_ASSOCSTR_CONTENTTYPE),
				ASSOCSTR_DEFAULTICON,		MAKEINTRESOURCE(IDS_ASSOCSTR_DEFAULTICON),
				ASSOCSTR_SHELLNEWVALUE,		MAKEINTRESOURCE(IDS_ASSOCSTR_SHELLNEWVALUE),
				ASSOCSTR_INFOTIP,			MAKEINTRESOURCE(IDS_ASSOCSTR_INFOTIP),
				ASSOCSTR_QUICKTIP,			MAKEINTRESOURCE(IDS_ASSOCSTR_QUICKTIP),
				ASSOCSTR_TILEINFO,			MAKEINTRESOURCE(IDS_ASSOCSTR_FILEINFO),
				ASSOCSTR_DDECOMMAND,		MAKEINTRESOURCE(IDS_ASSOCSTR_DDECOMMAND),
				ASSOCSTR_DDEIFEXEC,			MAKEINTRESOURCE(IDS_ASSOCSTR_DDEIFEXEC),
				ASSOCSTR_DDEAPPLICATION,	MAKEINTRESOURCE(IDS_ASSOCSTR_DDEAPPLICATION),
				ASSOCSTR_DDETOPIC,			MAKEINTRESOURCE(IDS_ASSOCSTR_DDETOPIC),
				ASSOCSTR_NOOPEN,			_T("No open"),
//@				ASSOCSTR_SHELLEXTENSION,	MAKEINTRESOURCE(IDS_ASSOCSTR_SHELLEXTENSION),
//@				ASSOCSTR_DROPTARGET,		_T("DROPTARGET"),
//@				ASSOCSTR_DELEGATEEXECUTE,	_T("ASSOCSTR_DELEGATEEXECUTE"),
				(ASSOCSTR)-1,				0
			};

			m_list.AddItemOrGroup(true, MAKEINTRESOURCE(IDS_F4_ASSOCIATE), NULL);

			TCHAR buf[MAX_PATH];
			int i = 0;
			while ((int)assocs[i].idType > 0) {
				DWORD len = ARRAYSIZE(buf);
				buf[0] = 0;
				if (pqa->GetString(0, assocs[i].idType, NULL, buf, &len) == S_OK) {
					m_list.AddItemOrGroup(false, assocs[i].pszResource, buf);
				}
				i++;
			}
			pqa->Release();
		}
		psf2->Release();
	}

	if (m_list.GetItemCount() > 0)
		m_list.AutoSizeColumns();
}

/////////////////////////////////////////////////////////////////////
//

static TCHAR* keyname[] = {
/*00-03*/	NULL,		NULL,		NULL,		NULL,
/*04-07*/	NULL,		NULL,		NULL,		NULL,
/*08-0B*/	_T("Bs"),	_T("Tab"),	NULL,		NULL,
/*0C-0F*/	_T("Clear"),_T("Enter"),NULL,		NULL,
/*10-13*/	NULL,		NULL,		NULL,		NULL,
/*14-17*/	NULL,		NULL,		NULL,		NULL,
/*18-1B*/	NULL,		NULL,		NULL,		_T("Esc"),
/*1C-1F*/	NULL,		NULL,		NULL,		NULL,
/*20-23*/	_T("Space"),_T("PgUp"),	_T("PgDn"),	_T("End"),
/*24-27*/	_T("Home"),	_T("←"),	_T("↑"),	_T("→"),
/*28-2B*/	_T("↓"),	NULL,		NULL,		NULL,
/*2C-2F*/	NULL,		_T("Ins"),	_T("Del"),	NULL,
/*30-33*/	_T("0"),	_T("1"),	_T("2"),	_T("3"),
/*34-37*/	_T("4"),	_T("5"),	_T("6"),	_T("7"),
/*38-3B*/	_T("8"),	_T("9"),	NULL,		NULL,
/*3C-3F*/	NULL,		NULL,		NULL,		NULL,
/*40-43*/	NULL,		_T("A"),	_T("B"),	_T("C"),
/*44-47*/	_T("D"),	_T("E"),	_T("F"),	_T("G"),
/*48-4B*/	_T("H"),	_T("I"),	_T("J"),	_T("K"),
/*4C-4F*/	_T("L"),	_T("M"),	_T("N"),	_T("O"),
/*50-53*/	_T("P"),	_T("Q"),	_T("R"),	_T("S"),
/*54-57*/	_T("T"),	_T("U"),	_T("V"),	_T("W"),
/*58-5B*/	_T("X"),	_T("Y"),	_T("Z"),	NULL,
/*5C-5F*/	NULL, 		NULL,		NULL,		NULL,
/*60-63*/	_T("0"),	_T("1"),	_T("2"),	_T("3"),
/*64-67*/	_T("4"),	_T("5"),	_T("6"),	_T("7"),
/*68-6B*/	_T("8"),	_T("9"),	_T("*"),	_T("+"),
/*6C-6F*/	NULL,		_T("-"),	NULL,		_T("/"),
/*70-73*/	_T("F1"),	_T("F2"),	_T("F3"),	_T("F4"),
/*74-77*/	_T("F5"),	_T("F6"),	_T("F7"),	_T("F8"),
/*78-7B*/	_T("F9"),	_T("F10"),	_T("F11"),	_T("F12"),
/*7C-7F*/	_T("F13"),	_T("F14"),	_T("F15"),	_T("F16"),
};

////////////////////////////////////////////////////////////////////////
//

class CLinkPage : public CDialogImpl<CLinkPage>
{
public:
	enum { IDD = IDD_FINFO_LINK };
	CString m_name;
	CInfoListViewCtrl m_list;

	CLinkPage()
	{
	}

BEGIN_MSG_MAP(CLinkPage)
	ON_WM_INITDIALOG()
	ON_MESSAGE_VOID(WM_ON_INIT, OnInit)
	REFLECT_NOTIFY_ID(IDC_F5_LIST)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnInit(WPARAM wParam, LPARAM lParam);
};

BOOL CLinkPage::OnInitDialog(LPARAM lParam)
{
	m_list.Initialize(GetDlgItem(IDC_F5_LIST));
	return TRUE;
}

void CLinkPage::OnInit(WPARAM wParam, LPARAM lParam)
{
	TCHAR name[MAX_PATH * 2];
	if (!SHGetPathFromIDList((LPCITEMIDLIST)lParam, name) || !m_name.CompareNoCase(name)) return;

	m_name = name;
	m_list.ResetItems();

	// --------------------------
	// リンク情報
	// --------------------------
	CString str;
	bool bGetInfo = false;
	IShellLink* psl;
	IPersistFile* ppf;
	TCHAR buf[MAX_PATH * 2];
	if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl) == S_OK) {
		if (psl->QueryInterface(IID_IPersistFile, (void**)&ppf) == S_OK) {
			if (ppf->Load(m_name, STGM_READ) == S_OK) {

				m_list.AddItemOrGroup(true, _T("リンク情報"), NULL);

				WIN32_FIND_DATA wfd;
				SHFILEINFO sfi;
				// リンク先
				LPITEMIDLIST pidl;
				if (psl->GetIDList(&pidl) == NOERROR) {
					if (SHGetPathFromIDList(pidl, buf)) {
						SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_TYPENAME);
						m_list.AddItemOrGroup(false, sszLinkTo, buf);
						m_list.AddItemOrGroup(false, sszTypes, sfi.szTypeName);
					}
					else {
						idl_GetName(pidl, SHGDN_FORPARSING | SHGDN_NORMAL, buf, ARRAYSIZE(buf));
						SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_TYPENAME | SHGFI_DISPLAYNAME);
						if (_pathfileexists(buf) || PathIsUNC(buf))
							m_list.AddItemOrGroup(false, sszLinkTo, buf);
						else
							m_list.AddItemOrGroup(false, sszLinkTo, sfi.szDisplayName);
						m_list.AddItemOrGroup(false, sszTypes, sfi.szTypeName);
					}
					CoTaskMemFree(pidl);
				}
				else {
					if (psl->GetPath(buf, ARRAYSIZE(buf), &wfd, 0) == NOERROR)
						m_list.AddItemOrGroup(false, sszLinkTo, buf);
				}

				// 引数
				if (psl->GetArguments(buf, ARRAYSIZE(buf)) != NOERROR)
					buf[0] = 0;
				m_list.AddItemOrGroup(false, _T("引数"), buf);

				// 作業フォルダ
				if (psl->GetWorkingDirectory(buf, ARRAYSIZE(buf)) != NOERROR)
					buf[0] = 0;
				m_list.AddItemOrGroup(false, _T("作業フォルダ"), buf);

				// コメント
				if (psl->GetDescription(buf, ARRAYSIZE(buf)) != NOERROR)
					buf[0] = 0;
				m_list.AddItemOrGroup(false, _T("コメント"), buf);

				// ウィンドウ
				CSimpleArray<CString> arr;
				SeparateString(MAKEINTRESOURCE(IDS_LIST_CMDSHOW), _T(';'), arr);
				str = arr[0];
				int n;
				if (psl->GetShowCmd(&n) == NOERROR) {
					switch (n) {
					case SW_SHOWNORMAL:		str = arr[1]; break;
					case SW_SHOWMINIMIZED:	str = arr[2]; break;
					case SW_SHOWMAXIMIZED:	str = arr[3]; break;
					default: break;
					}
				}
				m_list.AddItemOrGroup(false, _T("実行時の大きさ"), str);

				// ホットキー
				str.Empty();
				WORD hot = 0;
				if (psl->GetHotkey(&hot) == NOERROR) {
					BYTE cb = (hot >> 8) & 0xff;
					if (cb & HOTKEYF_CONTROL)	str += _T("Ctrl+");
					if (cb & HOTKEYF_SHIFT)		str += _T("Shift+");
					if (cb & HOTKEYF_ALT)		str += _T("Alt+");
					if (cb & HOTKEYF_EXT)		str += _T("Ext+");
					str += keyname[hot & 0xff];
				}
				m_list.AddItemOrGroup(false, _T("ホットキー"), str);

				// アイコン位置
				str.Empty();
				if (psl->GetIconLocation(buf, ARRAYSIZE(buf), &n) == NOERROR && buf[0])
					wsprintf(&buf[lstrlen(buf)], _T(",%d"), n);
				m_list.AddItemOrGroup(false, _T("アイコン"), str);

				bGetInfo = true;
			}
			ppf->Release();
		}
		psl->Release();
	}

	// --------------------------
	// ジャンクション情報
	// --------------------------
	if (!bGetInfo) {
		CFileInfo fi;
		if (GetFileInfo(m_name, fi) && fi.IsAttrib(FA_REPARSE)) {
			LPWSTR pszOriginalName;
			DWORD type = 0;
			if (fopGetReparsePointInfo(m_name, &pszOriginalName, &type) == NOERROR) {
				str.LoadString((type == IO_REPARSE_TAG_SYMLINK) ? IDS_TITLE_SYMLINK : IDS_TITLE_REPARSE);
				m_list.AddItemOrGroup(true, _T("ジャンクション情報"), NULL);
				m_list.AddItemOrGroup(false, sszTypes, str.GetString());
				m_list.AddItemOrGroup(false, sszLinkTo, pszOriginalName);
				CoTaskMemFree(pszOriginalName);
			}
		}
	}
	m_list.AutoSizeColumns();
}



/////////////////////////////////////////////////////////////////////////////
// CFileInfoDlg dialog

class CFileInfoDlg : public CDialogImpl<CFileInfoDlg>
{
public:
	CFileInfoList m_fi;
	CIcon m_ic;
	CString m_strFileTitle;
	CString m_strFileInfo;

	CGenericPage m_pageGeneric;
	CIconPage m_pageIcon;
	CAssocPage m_pageAssoc;
	CLinkPage m_pageLink;

public:
	CFileInfoDlg() { }
	~CFileInfoDlg() { }

	enum { IDD = IDD_FINFO_MAIN };

BEGIN_MSG_MAP(CFileInfoDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_DRAWITEM_ID(IDC_TITLE, OnDrawTitleArea)
	ON_DRAWITEM_ID(IDC_BARAREA, OnDrawBarArea)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_PREVPAGE, OnPrevPage)
	ON_BN_CLICKED(IDC_NEXTPAGE, OnNextPage)
	ON_COMMAND_RANGE(IDD_FINFO_START, IDD_FINFO_END, OnCommandRange)
	ON_MESSAGE(WM_CTLCOLORDLG, OnCtlColorDlg)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnDrawTitleArea(LPDRAWITEMSTRUCT pdi);
	void OnDrawBarArea(LPDRAWITEMSTRUCT pdi);
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnPrevPage();
	void OnNextPage();
	void OnCommandRange(UINT id, UINT code, HWND lParam);
	LRESULT OnCtlColorDlg(WPARAM wParam, LPARAM lParam);

	void SetFilePage(int page);
	void SetCurrentFile();
	void ChangePage(UINT idCommand);
	inline LPITEMIDLIST GetCurrentObject() { return m_fi[m_fi.m_pageIndex]; }
	void _DrawArea(HDC hdc, RECT* prc);
};

BOOL CFileInfoDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	if (m_fi.GetSize() < 2)
		GetDlgItem(IDC_NEXTPAGE).EnableWindow(FALSE);
	GetDlgItem(IDC_PREVPAGE).EnableWindow(FALSE);

	// フォルダのロードと設定
	m_fi.m_pbar = new CPageBar();

	CRect rc;
	GetDlgItem(IDC_AREA).GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_fi.m_pbar->Initialize(m_hWnd, GetDlgItem(IDC_TAB), &rc);
	m_fi.m_pbar->AddPage(m_pageGeneric.IDD,	IDD_FINFO_GENERIC,	m_pageGeneric.Create(m_hWnd));
	m_fi.m_pbar->AddPage(m_pageIcon.IDD,	IDD_FINFO_ICON,		m_pageIcon.Create(m_hWnd));
	m_fi.m_pbar->AddPage(m_pageAssoc.IDD,	IDD_FINFO_ASSOC,	m_pageAssoc.Create(m_hWnd));
	m_fi.m_pbar->AddPage(m_pageLink.IDD,	IDD_FINFO_LINK,		m_pageLink.Create(m_hWnd));
	m_fi.m_pbar->RecalcLayout();

	ChangePage(m_fi.m_pbar->IndexToCommand(m_fi.m_pageIndex));
	SetCurrentFile();
	return TRUE;
}

void CFileInfoDlg::OnDestroy()
{
	if (m_fi.m_pbar) {
		delete m_fi.m_pbar;
		m_fi.m_pbar = NULL;
	}
}

void CFileInfoDlg::OnPrevPage()
{
	SetFilePage(m_fi.Back());
}

void CFileInfoDlg::OnNextPage()
{
	SetFilePage(m_fi.Forward());
}

void CFileInfoDlg::SetFilePage(int page)
{
	GetDlgItem(IDC_PREVPAGE).EnableWindow(m_fi.canBack());
	GetDlgItem(IDC_NEXTPAGE).EnableWindow(m_fi.canForward());
	SetCurrentFile();
}

void CFileInfoDlg::OnCommandRange(UINT id, UINT /*code*/, HWND /*hwnd*/)
{
	ChangePage(id);
	GetDlgItem(IDC_TAB).SetFocus();
}

void CFileInfoDlg::SetCurrentFile()
{
	m_strFileTitle.Empty();
	m_strFileInfo.Empty();

	if (!m_ic.IsNull()) {
		m_ic.DestroyIcon();
	}

	LPITEMIDLIST pidlTarget = GetCurrentObject();
	IShellItem2* psi2;
	if (SHCreateItemFromIDList(pidlTarget, IID_IShellItem2, (void**)&psi2) == S_OK) {
		CComHeapPtr<WCHAR> hpw;		// 
		// 名前
		if (psi2->GetString(PKEY_ItemNameDisplay, &hpw) == S_OK) {
			m_strFileTitle = hpw;
			hpw.Free();
		}
		// 種別
		if (psi2->GetString(PKEY_ItemTypeText, &hpw) == S_OK) {
			m_strFileInfo = hpw;
			m_strFileInfo += _T("\n");
			hpw.Free();
		}
		// 更新日付
		TCHAR buf[128];
		FILETIME ft;
		if (psi2->GetFileTime(PKEY_DateModified, &ft) == S_OK) {
			FILETIME lt;
			FileTimeToLocalFileTime(&ft, &lt);
			FormatFileDateTimeWin(&lt, buf, ARRAYSIZE(buf));
			m_strFileInfo += buf;
		}
		// サイズ
		ULONGLONG fsz;
		if (psi2->GetUInt64(PKEY_Size, &fsz) == S_OK) {
			StrFormatByteSize64(fsz, buf, ARRAYSIZE(buf));
			m_strFileInfo.AppendFormat(_T(" ( %s )"), buf);
		}
		psi2->Release();
	}

	// --------------------------
	// イメージ（ビットマップ）からアイコンを作成
	// --------------------------
	IShellItemImageFactory* psif;
	if (SHCreateItemFromIDList(pidlTarget, IID_IShellItemImageFactory, (void**)&psif) == S_OK) {
		CRect rc;
		GetDlgItem(IDC_TITLE).GetWindowRect(&rc);
		UINT dxdy = min(rc.Width() - 2, rc.Height() - 2);			// make it square
		CSize size(dxdy, dxdy);
		HBITMAP hbm = NULL;
		if (psif->GetImage(size, SIIGBF_ICONONLY | SIIGBF_RESIZETOFIT, &hbm) == S_OK) {
			BITMAP bm;
			GetObject(hbm, sizeof(bm), &bm);

			ICONINFO ii = { TRUE, 0, 0 };
			ii.hbmColor = hbm;
			int msize = WIDTHBYTES(bm.bmWidth) * bm.bmHeight;
			BYTE* maskdata = (BYTE*)calloc(msize, 1);
			ii.hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, maskdata);
			free(maskdata);

			m_ic.CreateIconIndirect(&ii);
			::DeleteObject(ii.hbmMask);
			::DeleteObject(hbm);
		}
		psif->Release();
	}

	GetDlgItem(IDC_TITLE).Invalidate();

	UINT id = m_fi.m_pbar->GetActivePage();
	m_fi.m_pbar->OnInit(id, (LPARAM)pidlTarget);
}

void CFileInfoDlg::ChangePage(UINT idCommand)
{
	m_fi.m_pbar->SetActivePage(idCommand);

	LPITEMIDLIST pidlTarget = GetCurrentObject();
	m_fi.m_pbar->OnInit(idCommand, (LPARAM)pidlTarget);
}

void CFileInfoDlg::_DrawArea(HDC hdc, RECT* prc)
{
	ATLASSERT(prc != NULL);

	CDCHandle dc = hdc;
	COLORREF clrFrom = GetSysColor(COLOR_WINDOW);
	COLORREF clrTo   = GetSysColor(COLOR_3DLIGHT);

	TRIVERTEX vert[2];
	GRADIENT_RECT grc;
	vert[0].x     = prc->left;
	vert[0].y     = prc->top;
	vert[0].Red   = GetRValue(clrFrom) << 8;
	vert[0].Green = GetGValue(clrFrom) << 8;
	vert[0].Blue  = GetBValue(clrFrom) << 8;
	vert[0].Alpha = 0x0000;

	vert[1].x     = prc->right;
	vert[1].y     = prc->bottom;
	vert[1].Red   = GetRValue(clrTo) << 8;
	vert[1].Green = GetGValue(clrTo) << 8;
	vert[1].Blue  = GetBValue(clrTo) << 8;
	vert[1].Alpha = 0x0000;

	grc.UpperLeft  = 0;
	grc.LowerRight = 1;
	dc.GradientFill(vert, 2, &grc, 1, GRADIENT_FILL_RECT_V);
}

void CFileInfoDlg::OnDrawTitleArea(LPDRAWITEMSTRUCT pdi)
{
	CWindow ctl = GetDlgItem(IDC_TITLE);

	CRect rc = pdi->rcItem;
	CDCHandle dc = pdi->hDC;

	_DrawArea(dc, &rc);
	rc.DeflateRect(4, 4);

	// イメージまたはアイコンの表示
	CRect rcw = rc;
	rcw.left += 8;
	rcw.right = rcw.left + rc.Height();

	if (!m_ic.IsNull()) {
		ICONINFO ii;
		if (m_ic.GetIconInfo(&ii)) {
			BITMAP bm;
			::GetObject(ii.hbmColor, sizeof(bm), &bm);
			if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
			if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
			dc.DrawIconEx(rcw.left, rcw.top, m_ic, min(rcw.Width(), bm.bmWidth), min(rcw.Height(), bm.bmHeight), 0, NULL, DI_NORMAL);
		}
	}

	// ファイル情報の表示
	int bkMode = dc.SetBkMode(TRANSPARENT);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	rc.left = rcw.right + 4;

	int cy = tm.tmHeight * 3 + 2;				// 3行 + 1行目と2行目のマージン
	rc.DeflateRect(0, (rc.Height() - cy) / 2);

	LOGFONT lf;
	GetObject(ctl.GetFont(), sizeof(lf), &lf);
	lf.lfWeight = FW_BOLD;
	HFONT hfont = CreateFontIndirect(&lf);
	hfont = dc.SelectFont(hfont);
	dc.DrawText(m_strFileTitle, -1, &rc, DT_NOPREFIX | DT_SINGLELINE);
	::DeleteObject(dc.SelectFont(hfont));

	rc.top += tm.tmHeight + 2;
	dc.DrawText(m_strFileInfo, -1, &rc, DT_NOPREFIX | DT_WORDBREAK);
	dc.SetBkMode(bkMode);
}

void CFileInfoDlg::OnDrawBarArea(LPDRAWITEMSTRUCT pdi)
{
	_DrawArea(pdi->hDC, &pdi->rcItem);
}

LRESULT CFileInfoDlg::OnCtlColorDlg(WPARAM wParam, LPARAM lParam)
{
	CDCHandle dc = (HDC)wParam;
	CRect rc;
	GetDlgItem(IDC_BARAREA).GetWindowRect(&rc);
	ScreenToClient(&rc);
	_DrawArea(dc, &rc);

	int save = dc.SaveDC();
	dc.ExcludeClipRect(&rc);
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));
	dc.RestoreDC(save);

	return (LRESULT)::GetStockObject(NULL_BRUSH);
}

//////////////////////////////////////////////////////////////////////
//

static void show_fileinfo(HWND hwnd, int page, HDROP hdrop)
{
	CSimpleArray<LPITEMIDLIST> arr;
	int i, cnt = ::DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	for (i = 0; i < cnt; i++) {
		TCHAR buf[MAX_PATH * 2];
		DragQueryFile(hdrop, i, buf, ARRAYSIZE(buf));
		LPITEMIDLIST pidl = ILCreateFromPath(buf);
		if (pidl)
			arr.Add(pidl);
	}

	cnt = arr.GetSize();
	if (cnt > 0) {
		CFileInfoDlg dlg;
		dlg.m_fi.SetItems(page, arr.GetData(), cnt);
		dlg.DoModal(hwnd);
		for (i = 0; i < cnt; i++)
			CoTaskMemFree((void*)arr[i]);
	}
}

extern "C"
void WINAPI fopShowFileInfo(HWND hwnd, HDROP hdrop)
{
	show_fileinfo(hwnd, 0, hdrop);
}

extern "C"
void WINAPI fopShowFileInfoEx(HWND hwnd, HDROP hdrop, int nPageType)
{
	if (nPageType < 0 || nPageType > 4) nPageType = 0;
	show_fileinfo(hwnd, nPageType, hdrop);
}

extern "C"
void WINAPI fopShowFileInformation(HWND hwnd, LPITEMIDLIST* pidls, int numItems, int nPageType)
{
	if (nPageType < 0 || nPageType > 4) nPageType = 0;
	CFileInfoDlg dlg;
	if (dlg.m_fi.SetItems(nPageType, pidls, numItems) > 0)
		dlg.DoModal(hwnd);
}


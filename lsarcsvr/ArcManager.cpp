// ArcManager.cpp : CArcManager の実装

#include "stdafx.h"
#include "ArcManager.h"
#include "common.h"
#include "pack.h"
#include "packadd.h"
#include "unpack.h"
#include "arcres.h"
#include "checkarc.h"
#include "comarcdef.h"
#include "commarchiver.h"


////////////////////////////////////////////////////////////////////////
// functions

int QueryMakeDir(HWND hwnd, LPCTSTR pszText, BOOL bSingle);

////////////////////////////////////////////////////////////////////////
//

static const WCHAR szUNZIP[]  = L"lspuzp.dll";
static const WCHAR szZIP[]    = L"lspzip.dll";
static const WCHAR szTAR[]    = L"tar.dll";
static const WCHAR szCAB[]    = L"lspcab.dll";
static const WCHAR szUNRAR[]  = L"lspunr.dll";
static const WCHAR sz7Zip[]   = L"7-zip.dll";
#ifdef _WIN64
static const TCHAR szUNLHA[]	= L"unlha64.dll";
static const WCHAR szUNARJ[]	= L"unarj32j.dll";
static const WCHAR szISH[]		= L"ish32.dll";
static const WCHAR szDosARJ[]	= L"arj32.exe";
static const WCHAR szBGA[]		= L"bga32.dll";
static const WCHAR szYZ1[]		= L"yz1.dll";
static const WCHAR szGCA[]		= L"lspgca.dll";
#else
static const WCHAR szUNLHA[]	= L"unlha32.dll";
static const WCHAR szUNARJ[]	= L"unarj32j.dll";
static const WCHAR szISH[]		= L"ish32.dll";
static const WCHAR szDosARJ[]	= L"arj32.exe";
static const WCHAR szBGA[]		= L"bga32.dll";
static const WCHAR szYZ1[]		= L"yz1.dll";
static const WCHAR szGCA[]		= L"lspgca.dll";
#endif

////////////////////////////////////////////////////////////////////////
// CArcManager

static const WCHAR sszRegApp[]		= L"Software\\LightShip\\";
static const WCHAR sszRegSec[]		= L"GuidOperation";
static const WCHAR sszHeader[]		= L"ArcManager";


#define EFFECT_COMPRESS			0x00000000
#define EFFECT_EXTRACT			0x00000001

#pragma pack(1)
typedef struct _ACCEPTFILESINFO {
	DWORD effect;
	TCHAR path[MAX_PATH * 2];
	size_t hdrop_size;
	WORD viena;									// for FV7
	BYTE hdrop_data[1];
} ACCEPTFILESINFO;
#pragma pack()

////////////////////////////////////////////////////////////////////////
//

class CExtractFile
{
public:
	CString m_name;
	int m_type;

	CExtractFile(LPCTSTR pszFile, int type)
	{
		m_name = pszFile;
		m_type = type;
	}
};


/////////////////////////////////////////////////////////////////////
//

static HRESULT __RunSetupGuid(HWND hwnd, LPCTSTR path, HDROP hdrop, DWORD effect)
{
	UINT numFiles = hdrop ? DragQueryFile(hdrop, 0xffffffff, NULL, 0) : 0;
	if (numFiles < 1) return E_INVALIDARG;

	size_t size = GlobalSize((HGLOBAL)hdrop);
	size_t totalsize = sizeof(ACCEPTFILESINFO) + size;
	ACCEPTFILESINFO* info = (ACCEPTFILESINFO*)malloc(totalsize);
	if (!info) return E_OUTOFMEMORY;

	info->effect = effect;
	lstrcpy(info->path, path);
	info->hdrop_size = size;

	void* p = GlobalLock((HGLOBAL)hdrop);
	memcpy(info->hdrop_data, p, size);
	GlobalUnlock((HGLOBAL)hdrop);

	OLECHAR oz[128];
	BOOL bOK = FALSE;
	CString str = sszRegApp;
	str += sszHeader;
	CRegKey reg;
	if (reg.Create(HKEY_CURRENT_USER, str) == ERROR_SUCCESS) {
		CRegKey sec;
		if (sec.Create(reg, sszRegSec, REG_NONE, REG_OPTION_VOLATILE, KEY_WRITE) == ERROR_SUCCESS) {
			GUID g;
			if (CoCreateGuid(&g) == S_OK) {
				StringFromGUID2(g, oz, ARRAYSIZE(oz));
				sec.SetBinaryValue(oz, info, (ULONG)totalsize);
				bOK = TRUE;
			}
			sec.Close();
		}
		reg.Close();
	}

	free(info);

	// 別プロセスで実行
	if (bOK) {
		TCHAR buf[MAX_PATH * 2];
		GetModuleFileName(ATL::_AtlBaseModule.GetModuleInstance(), buf, ARRAYSIZE(buf));
		bOK = lccDllExecute(hwnd, PathFindFileName(buf), _T("RunArchive"), OLE2T(oz), _T("/archive"), FALSE);
	}
	return bOK ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////
//

BOOL __GetArchiverTypeInfo(ULONG type, LPWSTR pszPrefix, LPWSTR pszDll, BOOL& bUnicodeSupported, BOOL& bUtf8Supported)
{
	bUnicodeSupported = FALSE;
	bUtf8Supported = FALSE;

	*pszDll = 0;
	LPCTSTR psz = NULL;
	switch (type) {
		case ARC_LZH:		psz = szUNLHA;	lstrcpy(pszPrefix, _T("Unlha"));	break;
		case ARC_ZIP:		psz = szUNZIP;	lstrcpy(pszPrefix, _T("UnZip"));	break;
		case ARC_ZIPPACK:	psz = szZIP;	lstrcpy(pszPrefix, _T("Zip"));		break;
		case ARC_CAB:		psz = szCAB;	lstrcpy(pszPrefix, _T("Cab"));		break;
		case ARC_ARJ:		psz = szUNARJ;	lstrcpy(pszPrefix, _T("Unarj"));	break;
		case ARC_ISH:		psz = szISH;	lstrcpy(pszPrefix, _T("Ish"));		break;
		case ARC_TAR:		psz = szTAR;	lstrcpy(pszPrefix, _T("Tar"));		break;
		case ARC_RAR:		psz = szUNRAR;	lstrcpy(pszPrefix, _T("Unrar"));	break;
		case ARC_BGA:		psz = szBGA;	lstrcpy(pszPrefix, _T("Bga"));		break;
		case ARC_YZ1:		psz = szYZ1;	lstrcpy(pszPrefix, _T("Yz1"));		break;
		case ARC_GCA:		psz = szGCA;	lstrcpy(pszPrefix, _T("Gca"));		break;
		case ARC_ISO:
		case ARC_RPM:
		case ARC_7ZIP:		psz = sz7Zip;	lstrcpy(pszPrefix, _T("SevenZip"));	break;
		default: SetLastError(ERROR_INVALID_PARAMETER); return FALSE;
	}

	// ベースDLL名をコピー
	lstrcpy(pszDll, psz);

	// DLLの存在確認
	TCHAR path[MAX_PATH * 2];
	HMODULE hmod = LoadLibraryEx(psz, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (hmod) {
		// 読み込めたらフルパス名をコピー
		if (GetModuleFileName(hmod, path, ARRAYSIZE(path)) > 0)
			lstrcpy(pszDll, path);
		else
			lstrcpy(pszDll, psz);
	}
	else {
		GetModuleFileName(ATL::_AtlBaseModule.GetModuleInstance(), path, ARRAYSIZE(path));
		PathRemoveFileSpec(path);
		PathAppend(path, psz);
		if (PathFileExists(path)) {
			// 読み込めたらフルパス名をコピー
			hmod = LoadLibraryEx(pszDll, NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (hmod)
				lstrcpy(pszDll, path);
		}
	}

	if (hmod) {
		char func[MAX_PATH];
		WideCharToMultiByte(CP_ACP, NULL, pszPrefix, -1, func, ARRAYSIZE(func), NULL, NULL);
		lstrcatA(func, "W");
		bUnicodeSupported = (GetProcAddress(hmod, func) != NULL);

		WideCharToMultiByte(CP_ACP, NULL, pszPrefix, -1, func, ARRAYSIZE(func), NULL, NULL);
		lstrcatA(func, "SetUnicodeMode");
		bUtf8Supported = (GetProcAddress(hmod, func) != NULL);

		FreeLibrary(hmod);
	}
	return (hmod != NULL);
}

BOOL CArcManager::GetArchiverTypeInfo(ULONG type, LPWSTR pszPrefix, LPWSTR pszDll)
{
	BOOL bUnicodeSupported, bUtf8Supported;
	return __GetArchiverTypeInfo(type, pszPrefix, pszDll, bUnicodeSupported, bUtf8Supported);
}

HRESULT CArcManager::CompressProc(HWND hwndParent, void* pv)
{
	CPackInfo* pd = (CPackInfo*)pv;
	long arctype = (pd->m_arctype == ARC_ZIP) ? ARC_ZIPPACK : pd->m_arctype;

	CEnterFolder ef;

	// 基準ディレクトリの設定
	TCHAR dir[MAX_PATH * 2];
	if (GetPathCommonPrefix(pd->m_hdrop, dir)) {
		ef.Enter(dir);
	}

	// 書庫作成先フォルダの確認
	lstrcpy(dir, pd->m_arcname);
	if (arctype != ARC_ISH && arctype != ARC_UPX)
		PathRemoveFileSpec(dir);
	if (!PathIsDirectory(dir)) {
		if (AppMessage(IDS_MAKEDIR, dir, MB_QYESNO) != IDYES ||
			SHCreateDirectoryEx(NULL, dir, NULL) != ERROR_SUCCESS)
			return E_FAIL;
	}

	BOOL bTempDelete = TRUE;
	HRESULT hr = ArchiveW(hwndParent, arctype, pd->m_cmd, NULL, 0, NULL);
	if (hr == S_OK) {
#ifndef _WIN64
		if (pd->m_sfx) {
			if (arctype == ARC_LZH || arctype == ARC_CAB || arctype == ARC_BGA) {
				PathAddBackslash(dir);
				PathQuoteSpaces(dir);

				CString str;
				switch (arctype) {
				case ARC_LZH:						// LZH形式
					str.Format(_T("s -gw3 -x1 \"%s\" %s"), (LPCTSTR)pd->m_arcname, dir);
					break;
				case ARC_BGA:						// BGA形式
					str.Format(_T("s \"%s\" %s"), (LPCTSTR)pd->m_arcname, dir);
					break;
				}
#ifdef _CABSFX_SUPPORT
				case ARC_CAB:						// CAB形式
					str.Format(_T("f \"%s\" %s"), (LPCTSTR)pd->m_arcname, dir);
					break;
#endif	// _CABSFX_SUPPORT
				}
				hr = ArchiveW(hwndParent, arctype, str, NULL, 0, NULL);
			}
		}
#endif	// _WIN64
	}

	ef.Leave();

	if (bTempDelete)
		DeleteFile(pd->m_temp);
	return hr;
}

STDMETHODIMP CArcManager::CompressFiles(HWND hwnd, LPCWSTR pszBase, LONG_PTR hdrop_ptr)
{
	HDROP hdrop = (HDROP)hdrop_ptr;

	CPackDlg dlg;
	dlg.m_pd.m_arctype = ARC_LZH;
	dlg.m_pd.m_hdrop = hdrop;
	dlg.m_pd.m_path = pszBase;
	if (dlg.DoModal(hwnd) != IDOK)
		return E_FAIL;

	// 書庫タイプチェック
	if (dlg.m_pd.m_arctype != ARC_LZH &&
		dlg.m_pd.m_arctype != ARC_ZIP &&
		dlg.m_pd.m_arctype != ARC_CAB &&
		dlg.m_pd.m_arctype != ARC_TAR &&
		dlg.m_pd.m_arctype != ARC_BGA &&
		dlg.m_pd.m_arctype != ARC_7ZIP &&
		dlg.m_pd.m_arctype != ARC_ISH &&
		dlg.m_pd.m_arctype != ARC_UPX)
	{
		AppMessage(IDS_UNSUPPORTED_TYPE);
		return E_FAIL;
	}

	if (dlg.m_pd.m_arctype == ARC_UPX) {
		return (exec_console_app(dlg.m_pd.m_cmd, pszBase, _T("UPX")) ? S_OK : E_FAIL);
	}
	return CompressProc(hwnd, &dlg.m_pd);
}

STDMETHODIMP CArcManager::CompressFilesExec(HWND hwnd, LPCWSTR pszBase, LONG_PTR hdrop_ptr)
{
	return __RunSetupGuid(hwnd, pszBase, (HDROP)hdrop_ptr, EFFECT_COMPRESS);
}

STDMETHODIMP CArcManager::ExtractFiles(HWND hwnd, LPCWSTR pszPath, LONG_PTR hdrop_ptr)
{
	HDROP hdrop = (HDROP)hdrop_ptr;
	UINT numFiles = DragQueryFile(hdrop, 0xffffffff, NULL, 0);
	if (numFiles < 1) {
		AppMessage(IDS_NOFILES, NULL, MB_STOP);
		return E_FAIL;
	}

	UNPACKDATA ud;
	memset(&ud, 0, sizeof(ud));

	CSimpleArray<CExtractFile*> array;
	CExtractFile* pe;

	// 解凍対象のファイルをリストアップ
	LONG nType;
	TCHAR name[MAX_PATH * 2];
	UINT index;
	for (index = 0; index < numFiles; index++) {
		DragQueryFile(hdrop, index, name, ARRAYSIZE(name));
		GetArchiveType(name, &nType, NULL, NULL);
		if (nType != ARC_ERROR) {
			ud.arcbits |= ARC2BIT(nType);
			pe = new CExtractFile(name, nType);
			array.Add(pe);
		}
	}

	numFiles = (UINT)array.GetSize();
	HRESULT hr = S_OK;
	if (numFiles > 0) {
		// 出力先パス名の取得
		lstrcpy(ud.szPath, pszPath);
		if (GetUnpackInfo(hwnd, &ud)) {
			CString path = ud.szPath;
			if (path.IsEmpty())
				path = pszPath;

			// コマンドライン引数のセットアップ
			int action = CR_YES;
			BOOL bOK = FALSE;
			TCHAR buf[MAX_PATH * 2], fmt[MAX_PATH];
			for (index = 0; index < numFiles; index++) {
				// ファイル名と書庫タイプを取得する
				pe = array[index];
				lstrcpy(name, pe->m_name);
				// アーカイバ・コマンドラインをビルド
				MakeUnpackCommand(pe->m_type, &ud, fmt, ARRAYSIZE(fmt));
				// 解凍先
				lstrcpy(buf, path);
				if (!PathIsDirectory(buf)) {
					// パスが存在しなければ問い合わせて作成
					if (action != CR_YESALL) {
						action = QueryMakeDir(hwnd, buf, (numFiles < 2));
						if (action == CR_NO)
							break;
					}
					if (SHCreateDirectoryEx(NULL, buf, NULL) != ERROR_SUCCESS)
						break;
				}
				if (ud.bNamedir) {
					// 書庫名と同名のフォルダを作成する場合
					PathAppend(buf, PathFindFileName(name));
					PathRemoveExtension(buf);
					// パスが存在しなければ作成
					if (!PathIsDirectory(buf) && SHCreateDirectoryEx(NULL, buf, NULL) != ERROR_SUCCESS)
						break;
				}
				PathAddBackslash(buf);

				if (pe->m_type != ARC_ISH) {
					PathQuoteSpaces(buf);
					PathQuoteSpaces(name);
				}
				else {
					GetShortPathName(buf, buf, ARRAYSIZE(buf));
					GetShortPathName(name, name, ARRAYSIZE(name));
				}
				CString cmd;
				cmd.Format(fmt, name, buf);

				LONG lasterror = 0;
				if (ArchiveW(hwnd, pe->m_type, cmd, NULL, 0, &lasterror) != S_OK && lasterror == ERROR_USER_CANCEL) {
					if (index < numFiles - 1) {
						if (AppMessage(IDS_CANCEL_LEFTFILES, NULL, MB_QYESNO) == IDYES)
							break;
					}
				}
			}
		}
		else
			hr = E_FAIL;

		for (index = 0; index < numFiles; index++)
			delete array[index];
		array.RemoveAll();
	}
	else {
		AppMessage(IDS_NOARCHIVES, NULL, MB_STOP);
		hr = E_INVALIDARG;
	}
	return hr;
}

STDMETHODIMP CArcManager::ExtractFilesExec(HWND hwnd, LPCWSTR pszPath, LONG_PTR hdrop_ptr)
{
	return __RunSetupGuid(hwnd, pszPath, (HDROP)hdrop_ptr, EFFECT_EXTRACT);
}

STDMETHODIMP CArcManager::IsLibraryAvailable(ULONG type)
{
	TCHAR prx[80], dll[MAX_PATH * 2];
	BOOL bOK = GetArchiverTypeInfo(type, prx, dll);
	return (bOK ? S_OK : E_FAIL);
}

STDMETHODIMP CArcManager::Archive(HWND hwnd, ULONG type, LPCWSTR pszCmd, CHAR* pszResult, ULONG sizeResult, LONG* plArcError)
{
	HRESULT hr = E_FAIL;
	TCHAR prx[80], dll[MAX_PATH];

	BOOL bUnicodeSupported, bUtf8Supported;
	if (__GetArchiverTypeInfo(type, prx, dll, bUnicodeSupported, bUtf8Supported)) {
		CComPtr<ICommArchiver> pca;
		hr = pca.CoCreateInstance(__uuidof(CommArchiver));
		if (SUCCEEDED(hr)) {
			hr = pca->Initialize(hwnd, prx, dll);
			if (SUCCEEDED(hr)) {
				CWaitCursor wc;

				pca->SetBkgndMode(FALSE);		// バックグラウンドモード設定
				pca->SetCursorMode(TRUE);		// カーソルモード設定

				char* resptr;
				if (pszResult)
					resptr = pszResult;
				else {
					resptr = new char[0x8000];
					sizeResult = 0x8000;
				}

				hr = pca->Command(pszCmd, resptr, sizeResult);
				LONG error = 0;
				pca->GetLastError(&error);
				if (plArcError) *plArcError = error;
				if (FAILED(hr) && (error != ERROR_USER_CANCEL) && !pszResult) {
					MessageBeep(0);
					CArcResDlg dlg;
					dlg.SetResult(resptr);
					dlg.DoModal();
				}

				if (!pszResult)
					delete [] resptr;
			}
		}
	}
	return hr;
}

STDMETHODIMP CArcManager::ArchiveW(HWND hwnd, ULONG type, LPCWSTR pszCmd, WCHAR* pszResult, ULONG sizeResult, LONG* plArcError)
{
	HRESULT hr = E_FAIL;
	TCHAR prx[80], dll[MAX_PATH];

	BOOL bUnicodeSupported, bUtf8Supported;
	if (__GetArchiverTypeInfo(type, prx, dll, bUnicodeSupported, bUtf8Supported)) {
		if (!bUnicodeSupported) {
			char* resptr = NULL;
			ULONG size = 0;
			if (pszResult && sizeResult > 0) {
				size = sizeResult * 2;
				resptr = new char[size];
				*resptr = 0;
			}
			hr = Archive(hwnd, type, pszCmd, resptr, size, plArcError);
			if (resptr) {
				MultiByteToWideChar(CP_ACP, 0, resptr, -1, pszResult, (int)sizeResult);
				delete [] resptr;
			}
			return hr;
		}

		CComPtr<ICommArchiver> pca;
		hr = pca.CoCreateInstance(__uuidof(CommArchiver));
		if (SUCCEEDED(hr)) {
			hr = pca->Initialize(hwnd, prx, dll);
			if (SUCCEEDED(hr)) {
				CWaitCursor ec;
				pca->SetBkgndMode(FALSE);		// バックグラウンドモード設定
				pca->SetCursorMode(TRUE);		// カーソルモード設定

				WCHAR* wresptr = NULL;
				if (pszResult)
					wresptr = pszResult;
				else {
					sizeResult = 0x10000;
					wresptr = new WCHAR[sizeResult];
				}

				*wresptr = 0;
				hr = pca->CommandW(pszCmd, wresptr, sizeResult);

				LONG error = 0;
				pca->GetLastError(&error);
				if (plArcError) *plArcError = error;
				if (FAILED(hr) && (error != ERROR_USER_CANCEL) && !pszResult) {
					MessageBeep(0);
					CArcResDlg dlg;
					dlg.SetResult(wresptr);
					dlg.DoModal();
				}

				if (!pszResult)
					delete [] wresptr;
			}
		}
	}
	return hr;
}

STDMETHODIMP CArcManager::AddFiles(HWND hwnd, LPCWSTR pszArc, LPCWSTR pszBase, LONG_PTR hdrop_ptr)
{
	HDROP hdrop = (HDROP)hdrop_ptr;

	// append files is supported for ARC_LZH / ARC_ZIP / ARC_7ZIP / ARC_BGA
	int type = SpecifyArchiveType(pszArc);
	if (type != ARC_LZH && type != ARC_ZIP && type != ARC_7ZIP && type != ARC_BGA)
		return E_FAIL;

	TCHAR tmp[MAX_PATH * 2], tmpw[MAX_PATH * 2 + 4];
	BOOL bUnicodeSupported, bUtf8Supported;
	if (!__GetArchiverTypeInfo(type, tmp, tmpw, bUnicodeSupported, bUtf8Supported))
		return E_FAIL;

	BOOL bIncDir = FALSE;
	int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < cnt; i++) {
		DragQueryFile(hdrop, i, tmp, ARRAYSIZE(tmp));
		if (PathIsDirectory(tmp)) {
			bIncDir = TRUE;
			break;
		}
	}

	CPackAddDlg dlg;
	dlg.m_strArchive = pszArc;
	dlg.m_strBasePath = pszBase;
	dlg.m_pathMode = bIncDir ? BASEDIR_RELATIVE : BASEDIR_NOPATH;
	dlg.m_hdrop = hdrop;
	if (dlg.DoModal() != IDOK)
		return E_FAIL;

	CEnterFolder ef;
	if (!dlg.m_strBasePath.IsEmpty())
		ef.Enter(dlg.m_strBasePath);

	// 引数リストファイル作成
	MakeTempFile(hdrop, tmp, dlg.m_pathMode, FALSE, type, (bUnicodeSupported != FALSE), (bUtf8Supported != FALSE), dlg.m_strBasePath);
	lstrcpy(&tmpw[1], tmp);
	tmpw[0] = '@';
	PathQuoteSpaces(tmpw);

	TCHAR arc[MAX_PATH * 2], dir[MAX_PATH * 2];
	lstrcpy(dir, dlg.m_strBasePath);
	PathAddBackslash(dir);
	PathQuoteSpaces(dir);
	lstrcpy(arc, pszArc);
	PathQuoteSpaces(arc);

	CString cmd;
	if (type == ARC_LZH) {
		// LZH
		cmd = _T("u -a2 -jp1 -+ ");
		if (bIncDir)
			cmd += _T("-r2 ");
		cmd.AppendFormat(_T("%s %s %s"), arc, dir, tmpw);
		if (dlg.m_pathMode != BASEDIR_NOPATH)
			cmd += _T(" -x1");
	}
	else if (type == ARC_ZIP) {
		// ZIP
		cmd = _T("-u -g -S -X ");
		if (bIncDir) {
			cmd += _T("-r ");
			if (dlg.m_pathMode == BASEDIR_NOPATH)
				cmd += _T("-j ");
		}
		cmd.AppendFormat(_T("%s %s %s"), arc, dir, tmpw);

		// ZIP圧縮!
		type = ARC_ZIPPACK;
	}
	else if (type == ARC_7ZIP) {
		// 7ZIP
		cmd = _T("a -t7z -ms=off ");
		if (bIncDir) {
			cmd += _T("-r0 ");
		}
		cmd.AppendFormat(_T("%s %s %s"), arc, dir, tmpw);
	}
	else if (type == ARC_BGA) {
		// BGA
		if (!StrCmpI(PathFindExtension(pszArc), _T(".bza")))
			cmd = _T("a -m2 ");
		else
			cmd = _T("a -m1 ");

		if (bIncDir)
			cmd += _T("-r ");
		cmd.AppendFormat(_T("%s %s %s"), arc, dir, tmpw);
	}

	BOOL bOK = ArchiveW(hwnd, type, cmd, NULL, 0, NULL);
	DeleteFile(tmp);

	return (bOK ? S_OK : E_FAIL);
}

STDMETHODIMP CArcManager::CheckFiles(HWND hwnd, LONG_PTR hdrop_ptr)
{
	CCheckArcDlg dlg;
	dlg.m_hdrop = (HDROP)hdrop_ptr;
	dlg.m_pArcManager = this;
	dlg.DoModal();
	return S_OK;
}

STDMETHODIMP CArcManager::IsArchive(HWND hwnd, ULONG type, LPCWSTR pszArc)
{
	CString cmd;
	if (type == ARC_LZH)
		cmd.Format(_T("t -n -gm \"%s\""), pszArc);
	else if (type == ARC_CAB)
		cmd.Format(_T("c \"%s\""), pszArc);
	else if (type == ARC_ZIP)
		return (__iszipfile(pszArc) ? S_OK : E_FAIL);
	else if (type == ARC_7ZIP)
		cmd.Format(_T("t \"%s\""), pszArc);
	else
		return E_FAIL;

	// ArchiveWでエラーメッセージ表示をスキップさせるためにresptrを渡す
	LONG err;
	WCHAR* resptr = new WCHAR[512];
	HRESULT hr = ArchiveW(hwnd, type, cmd, resptr, 512, &err);
	delete [] resptr;
	return hr;
}

STDMETHODIMP CArcManager::GetDefaultFolder(LPWSTR pszFolder)
{
	TCHAR buf[MAX_PATH * 2];
	if (GetRegText(IDS_SEC_OPERATION, IDS_KEY_PLACEFOLDER, buf, ARRAYSIZE(buf)) > 0) {
		lstrcpy(pszFolder, buf);
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CArcManager::SetDefaultFolder(LPCWSTR pszFolder)
{
	SetRegText(IDS_SEC_OPERATION, IDS_KEY_PLACEFOLDER, pszFolder);
	return S_OK;
}

STDMETHODIMP CArcManager::GetArchiveType(LPCWSTR pszFile, LONG* type, BOOL* pbUnicodeSupported, BOOL* pbUtf8Supported)
{
	*type = ARC_ERROR;
	if (pszFile && *pszFile) {
		*type = SpecifyArchiveTypeByFileHeader(pszFile);
		if (*type == ARC_ERROR) {
			// 自己解凍形式をチェック
			LPCTSTR pe = PathFindExtension(pszFile);
			if (*pe == '.' && !StrCmpI(++pe, _T("exe"))) {
				LONG last = GetRegValue(IDS_SEC_OPERATION, IDS_KEY_LASTSFXTYPE, ARC_LZH);
				LONG types[4];
				if (last == ARC_CAB) {
					types[0] = ARC_CAB;
					types[1] = ARC_LZH;
					types[2] = ARC_ZIP;
					types[3] = ARC_7ZIP;
				}
				else if (last == ARC_LZH) {
					types[0] = ARC_LZH;
					types[1] = ARC_CAB;
					types[2] = ARC_ZIP;
					types[3] = ARC_7ZIP;
				}
				else if (last == ARC_ZIP) {
					types[0] = ARC_ZIP;
					types[1] = ARC_LZH;
					types[2] = ARC_CAB;
					types[3] = ARC_7ZIP;
				}
				else {
					types[0] = ARC_7ZIP;
					types[1] = ARC_LZH;
					types[2] = ARC_CAB;
					types[3] = ARC_ZIP;
				}

				HWND hwnd = GetActiveWindow();
				for (int i = 0; i < 4; i++) {
					if (IsArchive(hwnd, (ULONG)types[i], pszFile) == S_OK) {
						*type = types[i];
						if (*type != last)
							SetRegValue(IDS_SEC_OPERATION, IDS_KEY_LASTSFXTYPE, *type);
						break;
					}
				}
			}
		}
	}

	if (*type != ARC_ERROR && (pbUnicodeSupported != NULL || pbUtf8Supported != NULL)) {
		if (pbUnicodeSupported) *pbUnicodeSupported = FALSE;
		if (pbUtf8Supported) *pbUtf8Supported = FALSE;

		WCHAR prx[64], dll[MAX_PATH * 2];
		BOOL bUnicodeSupported, bUtf8Supported;
		if (__GetArchiverTypeInfo(*type, prx, dll, bUnicodeSupported, bUtf8Supported)) {
			if (pbUnicodeSupported) *pbUnicodeSupported = bUnicodeSupported;
			if (pbUtf8Supported) *pbUtf8Supported = bUtf8Supported;
		}
	}

	return S_OK;
}

STDMETHODIMP CArcManager::ExtractFromArchive(HWND hwnd, ULONG type, LPCWSTR pszFile, LPCWSTR pszList)
{
	// 出力先パス名の取得
	TCHAR path[MAX_PATH * 2], cmd[MAX_PATH * 2];
	PathGetParent(pszFile, path);

	CEnterFolder ef;
	ef.Enter(path);
	UNPACKDATA ud;
	memset(&ud, 0, sizeof(ud));
	ud.arcbits = ARC2BIT(type);
	lstrcpy(ud.szPath, path);
	BOOL bOK = GetUnpackInfo(hwnd, &ud);
	ef.Leave();

	if (!bOK)
		return E_FAIL;

	MakeUnpackCommand(type, &ud, cmd, ARRAYSIZE(cmd));
	lstrcpy(path, ud.szPath);
	if (lstrlen(path) < 1) {
		lstrcpy(path, pszFile);
		PathRemoveFileSpec(path);
	}

	if (ud.bNamedir) {
		// 書庫名と同名のフォルダを作成する場合
		PathAppend(path, PathFindFileName(pszFile));
		PathRemoveExtension(path);
	}

	if (!PathIsDirectory(path)) {
		if (AppMessage(IDS_MAKEDIR, path, MB_QYESNO) != IDYES ||
			SHCreateDirectoryEx(NULL, path, NULL) != ERROR_SUCCESS)
			return FALSE;
	}
	PathAddBackslash(path);

	// コマンドライン引数のセットアップ
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, pszFile);
	PathQuoteSpaces(path);
	PathQuoteSpaces(buf);

	CString cmdline;
	cmdline.Format(cmd, buf, path);
	cmdline.AppendFormat(_T(" %s"), pszList);

	return ArchiveW(hwnd, type, cmdline, NULL, 0, NULL);
}

//////////////////////////////////
// BeginFindに渡すフラグ
static DWORD sdwFindFlag[] = {
	M_CHECK_FILENAME_ONLY,						// ARC_LZH
	M_CHECK_FILENAME_ONLY,						// ARC_ZIP
	M_CHECK_FILENAME_ONLY,						// ARC_ARJ
	0,											// ARC_ISH
	M_CHECK_FILENAME_ONLY,						// ARC_CAB
	0,											// ARC_TAR
	0,											// ARC_RAR
	M_CHECK_FILENAME_ONLY,						// ARC_BGA
	M_CHECK_FILENAME_ONLY | M_ERROR_MESSAGE_ON,	// ARC_YZ1
	M_CHECK_FILENAME_ONLY | M_ERROR_MESSAGE_ON,	// ARC_GCA
	0,											// ARC_7ZIP
	0,											// ARC_ISO
	0,											// ARC_RPM
};

STDMETHODIMP CArcManager::BeginFind(HWND hwnd, ULONG type, LPCWSTR pszFile, LPCWSTR pszFilter, BYTE* piiw, IUnknown** ppInterface)
{
	*ppInterface = NULL;
	HRESULT hr = E_FAIL;
	TCHAR prx[80], dll[MAX_PATH];
	BOOL bUnicodeSupported, bUtf8Supported;
	if (__GetArchiverTypeInfo(type, prx, dll, bUnicodeSupported, bUtf8Supported)) {
		CComPtr<ICommArchiver> pca;
		hr = pca.CoCreateInstance(__uuidof(CommArchiver));
		if (SUCCEEDED(hr)) {
			hr = pca->Initialize(hwnd, prx, dll);
			if (SUCCEEDED(hr)) {
				hr = pca->OpenArchive(pszFile, sdwFindFlag[type]);
				if (SUCCEEDED(hr)) {
					hr = pca->FindFirstW(pszFilter, piiw);
					if (SUCCEEDED(hr)) {
						*ppInterface = pca.Detach();
					}
				}
			}
		}
	}
	return hr;
}

STDMETHODIMP CArcManager::NextFind(IUnknown* pInterface, BYTE* piiw)
{
	ICommArchiver* pca = (ICommArchiver*)pInterface;
	if (!pca)
		return E_INVALIDARG;
	return pca->FindNextW(piiw);
}

STDMETHODIMP CArcManager::EndFind(IUnknown* pInterface)
{
	ICommArchiver* pca = (ICommArchiver*)pInterface;
	if (!pca)
		return E_INVALIDARG;
	pca->CloseArchive();
	return pca->Release();
}

STDMETHODIMP CArcManager::MakeSFX(HWND hwnd, LPCWSTR pszFile, LPCWSTR pszTargetPath)
{
	int type = SpecifyArchiveType(pszFile);
	if (type != ARC_LZH && type != ARC_CAB && type != ARC_BGA && type != ARC_7ZIP)
		return E_NOINTERFACE;

	TCHAR dir[MAX_PATH * 2];
	if (pszTargetPath)
		lstrcpy(dir, pszTargetPath);	// 作成先が指定されている場合
	else
		PathGetParent(pszFile, dir);	// 指定されていない場合は書庫と同じフォルダ

	// 作成先フォルダの有無確認
	if (!PathIsDirectory(dir)) {
		if (AppMessage(IDS_MAKEDIR, dir, MB_QYESNO) != IDYES ||
			SHCreateDirectoryEx(NULL, dir, NULL) != ERROR_SUCCESS)
			return E_ABORT;
	}

	PathAddBackslash(dir);
	PathQuoteSpaces(dir);

	CString cmd;
	switch (type) {
	case ARC_LZH:  cmd.Format(_T("s -gw3 -x1 \"%s\" %s"), pszFile, dir); break;
	case ARC_CAB:  cmd.Format(_T("f \"%s\" %s"), pszFile, dir); break;
	case ARC_BGA:  cmd.Format(_T("s \"%s\" %s"), pszFile, dir); break;
	case ARC_7ZIP: cmd.Format(_T("u -sfx7zC.sfx \"%s\" %s"), pszFile, dir); break;
	}
	return ArchiveW(hwnd, type, cmd, NULL, 0, NULL);
}

STDMETHODIMP CArcManager::ListArchive(HWND hwnd, ULONG type, LPCWSTR pszFile, LPSTR pszResult, ULONG size)
{
	CString name;
	LPTSTR p = name.GetBuffer(lstrlen(pszFile) + 8);
	lstrcpy(p, pszFile);
	PathQuoteSpaces(p);
	name.ReleaseBuffer();

	CString cmd;
	switch (type) {
	case ARC_LZH:  cmd.Format(_T("v %s"), name); break;
	case ARC_ZIP:  cmd.Format(_T("-lv %s"), name); break;
	case ARC_ARJ:  cmd.Format(_T("l %s"), name); break;
	case ARC_CAB:  cmd.Format(_T("l %s"), name); break;
	case ARC_RAR:  cmd.Format(_T("-l %s"), name); break;
	case ARC_TAR:  cmd.Format(_T("-tf %s"), name); break;
	case ARC_GCA:  cmd.Format(_T("-l %s"), name); break;
	case ARC_ISO:
	case ARC_RPM:
	case ARC_7ZIP: cmd.Format(_T("l %s"), name); break;
	default: return E_NOINTERFACE;
	}

	WCHAR* wresptr = NULL;
	ULONG len = 0;
	if (pszResult && size > 0) {
		len = size * 2;
		wresptr = new WCHAR[len];
		*wresptr = 0;
	}

	HRESULT hr = ArchiveW(hwnd, type, cmd, wresptr, len, NULL);

	if (wresptr) {
		WideCharToMultiByte(CP_ACP, 0, wresptr, -1, pszResult, (int)size, NULL, NULL);
		delete [] wresptr;
	}

	return hr;

}
////////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI RunArchiveW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, sszRegApp);
	PathAppend(buf, sszHeader);
	PathAppend(buf, sszRegSec);

	CRegKey reg;
	if (reg.Open(HKEY_CURRENT_USER, buf) == ERROR_SUCCESS) {
		ULONG len = 0;
		if (reg.QueryBinaryValue(lpCmdLine, NULL, &len) == ERROR_SUCCESS && len > sizeof(ACCEPTFILESINFO)) {
			ACCEPTFILESINFO* info = (ACCEPTFILESINFO*)malloc(len);
			if (info) {
				if (reg.QueryBinaryValue(lpCmdLine, info, &len) == ERROR_SUCCESS) {
					HGLOBAL hmem = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, info->hdrop_size);
					if (hmem) {
						void* p = GlobalLock(hmem);
						memcpy(p, info->hdrop_data, info->hdrop_size);
						GlobalUnlock(hmem);

						// 書庫サーバ
						CComPtr<IArcManager> pas;
						HRESULT hr = pas.CoCreateInstance(__uuidof(ArcManager));
						if (SUCCEEDED(hr)) {
							switch (info->effect) {
							case EFFECT_COMPRESS:
								hr = pas->CompressFiles(hwnd, info->path, (LONG_PTR)hmem);
								break;
							case EFFECT_EXTRACT:
								hr = pas->ExtractFiles(hwnd, info->path, (LONG_PTR)hmem);
								break;
							default:
								hr = E_INVALIDARG;
								break;
							}
						}

						GlobalFree(hmem);
					}
				}
				free(info);
			}
		}
		reg.DeleteValue(lpCmdLine);
		reg.Close();
	}
}

extern "C"
void WINAPI RunArchive(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	CStringW wstr;
	StrToWStr(lpCmdLine, wstr);
	RunArchiveW(hwnd, hInst, (LPWSTR)(LPCWSTR)wstr, nCmdShow);
}

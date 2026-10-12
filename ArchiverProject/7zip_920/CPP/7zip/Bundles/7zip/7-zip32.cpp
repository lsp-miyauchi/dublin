// 7-zip32.cpp : DLL アプリケーション用のエントリ ポイントを定義します。
//
#include "stdafx.h"
#include <versionhelpers.h>
#include "7-zip32.h"
#include "Dialog.h"
#include "resource.h"
#include "MyOpenArchive.h"
#include "SplitCmdLine.h"
#include "Common/StdOutStream.h"

#include "Common/UTFConvert.h"
#include "Windows/FileDir.h"

extern int g_CodePage;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = (HINSTANCE)hModule;
		g_IsNT = IsWindows8Point1OrGreater();
		break;
	case DLL_PROCESS_DETACH:
		COpenArchive::RemoveAll();
		break;
	}
  return TRUE;
}

int WINAPI SevenZip(const HWND _hwnd, LPCSTR _szCmdLine, LPSTR _szOutput, const DWORD _dwSize)
{
	if (g_StdOut.GetThread())
		return ERROR_ALREADY_RUNNING;

	CSplitCmdLine scl;
	if (!scl.Split(_szCmdLine))
		return ERROR_COMMAND_NAME;

	if (scl.m_bSfx && scl.IsUpdateCommands())
	{
		CSfxDialog dlgSfx(_hwnd);
		if (dlgSfx.DoModal() == IDCANCEL)
			return ERROR_USER_CANCEL;
	}

	g_StdOut.SetCommandLine(scl);
	UString strCurrentDirectory;
	if (scl.m_lpBaseDirectory)
	{
		::NWindows::NFile::NDirectory::MyGetCurrentDirectory(strCurrentDirectory);
		if (::NWindows::NFile::NDirectory::MySetCurrentDirectory(::MultiByteToUnicodeString(scl.m_lpBaseDirectory, scl.m_codePage)) == false)
		{
			g_StdOut << "Base directory isn't found.";
			g_StdOut.CopyBuf(_szOutput, _dwSize);
			g_StdOut.ReSet();
			return ERROR_FATAL;
		}
	}

	g_StdOut.CreateMainThread();

	CProgressDialog dlgProgress(_hwnd);
	g_StdOut.SetProgressDialog(&dlgProgress);
	dlgProgress.SetShowDialog(!scl.m_bHide);
	dlgProgress.SetProgressMode(scl.m_argv[0][0]);
	dlgProgress.CreateModal();

	DWORD dwThreadId;
	::WaitForSingleObject(g_StdOut.GetThread(), INFINITE);
	::GetExitCodeThread(g_StdOut.GetThread(), &dwThreadId);
	::CloseHandle(g_StdOut.GetThread());
	g_StdOut.CopyBuf(_szOutput, _dwSize);
	g_StdOut.ReSet();
	g_CodePage = -1;
	if (scl.m_bSfx)
		::NWindows::NFile::NDirectory::DeleteFileAlways(g_StdOut.GetTempSFXName());
	if (scl.m_lpBaseDirectory)
		::NWindows::NFile::NDirectory::MySetCurrentDirectory(strCurrentDirectory);
    return dwThreadId;
}

WORD WINAPI SevenZipGetVersion()
{
	return (SEVENZIP32_VERSION / 10000);
}

// 基本的に意味なし
static BOOL bCursorMode = FALSE;
static int  nCursorInterval = 80;
static BOOL bBackGroundMode = FALSE;
BOOL WINAPI SevenZipGetCursorMode() { return bCursorMode; }
BOOL WINAPI SevenZipSetCursorMode(const BOOL _CursorMode) { bCursorMode = _CursorMode; return TRUE; }
WORD WINAPI SevenZipGetCursorInterval() { return nCursorInterval; }
BOOL WINAPI SevenZipSetCursorInterval(const WORD _Interval) { nCursorInterval = _Interval; return TRUE; }
BOOL WINAPI SevenZipGetBackGroundMode() { return bBackGroundMode; }
BOOL WINAPI SevenZipSetBackGroundMode(const BOOL _BackGroundMode) { bBackGroundMode = _BackGroundMode; return TRUE; }

BOOL WINAPI SevenZipGetRunning()
{
	return (g_StdOut.GetThread() != NULL);
}

// 呼び出すだけ
BOOL WINAPI SevenZipConfigDialog(const HWND _hwnd, LPSTR _szOptionBuffer, const int _iMode)
{
	if (g_StdOut.GetThread())
		return ERROR_ALREADY_RUNNING;
	CConfigDialog dlg(_hwnd);
	return (BOOL)dlg.DoModal();
}

BOOL WINAPI SevenZipCheckArchive(LPCSTR _szFileName, const int _iMode)
{
	COpenArchive arc;
	return (arc.OpenCheck(g_StdOut.ConvertUnicodeString(_szFileName), _iMode) != E_FAIL);
}

int WINAPI SevenZipGetFileCount(LPCSTR _szArcFile)
{
	HARC hArc = SevenZipOpenArchive(NULL, _szArcFile, 0);
	COpenArchive* pOpenArchive = COpenArchive::FindObject(hArc);
	if (pOpenArchive == NULL)
		return -1;
	int nCount = pOpenArchive->GetFileCount();
	SevenZipCloseArchive(hArc);
	return nCount;
}

BOOL WINAPI SevenZipQueryFunctionList(const int _iFunction)
{
	switch (_iFunction)
	{
	case ISARC:
	case ISARC_GET_VERSION:
	case ISARC_GET_CURSOR_INTERVAL:
	case ISARC_SET_CURSOR_INTERVAL:
	case ISARC_GET_BACK_GROUND_MODE:
	case ISARC_SET_BACK_GROUND_MODE:
	case ISARC_GET_CURSOR_MODE:
	case ISARC_SET_CURSOR_MODE:
	case ISARC_GET_RUNNING:
		
	case ISARC_CHECK_ARCHIVE:
	case ISARC_CONFIG_DIALOG:
	case ISARC_GET_FILE_COUNT:
	case ISARC_QUERY_FUNCTION_LIST:
//	case ISARC_HOUT:
//	case ISARC_STRUCTOUT:
//	case ISARC_GET_ARC_FILE_INFO:
		
	case ISARC_OPEN_ARCHIVE:
	case ISARC_CLOSE_ARCHIVE:
	case ISARC_FIND_FIRST:
	case ISARC_FIND_NEXT:
//	case ISARC_EXTRACT:
//	case ISARC_ADD:
//	case ISARC_MOVE:
//	case ISARC_DELETE:
	case ISARC_SETOWNERWINDOW:
	case ISARC_CLEAROWNERWINDOW:
	case ISARC_SETOWNERWINDOWEX:
	case ISARC_KILLOWNERWINDOWEX:
		
	case ISARC_GET_ARC_FILE_NAME:
	case ISARC_GET_ARC_FILE_SIZE:
	case ISARC_GET_ARC_ORIGINAL_SIZE:
	case ISARC_GET_ARC_COMPRESSED_SIZE:
	case ISARC_GET_ARC_RATIO:
	case ISARC_GET_ARC_DATE:
	case ISARC_GET_ARC_TIME:
//	case ISARC_GET_ARC_OS_TYPE:
//	case ISARC_GET_ARC_IS_SFX_FILE:
	case ISARC_GET_ARC_WRITE_TIME_EX:
	case ISARC_GET_ARC_CREATE_TIME_EX:
	case ISARC_GET_ARC_ACCESS_TIME_EX:
//	case ISARC_GET_ARC_CREATE_TIME_EX2:
//	case ISARC_GET_ARC_WRITE_TIME_EX2:
	case ISARC_GET_FILE_NAME:
	case ISARC_GET_ORIGINAL_SIZE:
	case ISARC_GET_COMPRESSED_SIZE:
	case ISARC_GET_RATIO:
	case ISARC_GET_DATE:
	case ISARC_GET_TIME:
	case ISARC_GET_CRC:
	case ISARC_GET_ATTRIBUTE:
//	case ISARC_GET_OS_TYPE:
	case ISARC_GET_METHOD:
	case ISARC_GET_WRITE_TIME:
	case ISARC_GET_CREATE_TIME:
	case ISARC_GET_ACCESS_TIME:
	case ISARC_GET_WRITE_TIME_EX:
	case ISARC_GET_CREATE_TIME_EX:
	case ISARC_GET_ACCESS_TIME_EX:
//	case ISARC_SET_ENUM_MEMBERS_PROC:
//	case ISARC_CLEAR_ENUM_MEMBERS_PROC:
	case ISARC_GET_ARC_FILE_SIZE_EX:
	case ISARC_GET_ARC_ORIGINAL_SIZE_EX:
	case ISARC_GET_ARC_COMPRESSED_SIZE_EX:
	case ISARC_GET_ORIGINAL_SIZE_EX:
	case ISARC_GET_COMPRESSED_SIZE_EX:
	case ISARC_SETOWNERWINDOWEX64:
	case ISARC_KILLOWNERWINDOWEX64:
//	case ISARC_SET_ENUM_MEMBERS_PROC64:
//	case ISARC_CLEAR_ENUM_MEMBERS_PROC64:
//	case ISARC_OPEN_ARCHIVE2:
//	case ISARC_GET_ARC_READ_SIZE:
//	case ISARC_GET_ARC_READ_SIZE_EX:
	case ISARC_SET_PRIORITY:
	case ISARC_SET_UNICODE_MODE:
	case ISARC_SET_DEFAULT_PASSWORD:
	case ISARC_GET_DEFAULT_PASSWORD:
	case ISARC_PASSWORD_DIALOG:
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

// とりあえず_dwModeは保存してるけど意味無し
HARC WINAPI SevenZipOpenArchive(const HWND _hwnd, LPCSTR _szFileName, const DWORD _dwMode)
{
	COpenArchive* pOpenArchive = new COpenArchive;
	if (pOpenArchive)
	{
		if (pOpenArchive->Open(g_StdOut.ConvertUnicodeString(_szFileName), _dwMode) == FALSE)
		{
			delete pOpenArchive;
			return NULL;
		}
	}
	return (HARC)pOpenArchive;
}

int WINAPI SevenZipCloseArchive(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	delete pOpenArchive;
	return 0;
}

int WINAPI SevenZipFindFirst(HARC _harc, LPCSTR _szWildName, INDIVIDUALINFO *_lpSubInfo)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return ERROR_HARC_ISNOT_OPENED;
	return pOpenArchive->FindFirst(g_StdOut.ConvertUnicodeString(_szWildName), _lpSubInfo);
}

int WINAPI SevenZipFindNext(HARC _harc, INDIVIDUALINFO *_lpSubInfo)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return ERROR_NOT_SEARCH_MODE;
	return pOpenArchive->FindNext(_lpSubInfo);
}

int WINAPI SevenZipGetArcFileName(HARC _harc, LPSTR _lpBuffer, const int _nSize)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return ERROR_HARC_ISNOT_OPENED;
	g_StdOut.WideCharToMultiByte(pOpenArchive->GetArcFileName(), _lpBuffer, _nSize);
	return 0;
}

DWORD WINAPI SevenZipGetArcFileSize(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetArcFileSize();
}

DWORD WINAPI SevenZipGetArcOriginalSize(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetArcOriginalSize();
}

DWORD WINAPI SevenZipGetArcCompressedSize(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetArcCompressedSize();
}

WORD WINAPI SevenZipGetArcRatio(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetArcRatio();
}

WORD WINAPI SevenZipGetArcDate(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetArcDate();
}

WORD WINAPI SevenZipGetArcTime(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetArcTime();
}

// エントリのみ。エラー時以外、必ず0を返す
UINT WINAPI SevenZipGetArcOSType(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetArcOSType();
}

// エントリのみ。エラー時以外、必ず0を返す
int WINAPI SevenZipIsSFXFile(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->IsSFXFile();
}

int WINAPI SevenZipGetFileName(HARC _harc, LPSTR _lpBuffer, const int _nSize)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return ERROR_HARC_ISNOT_OPENED;
	LPCWSTR lpFileName = pOpenArchive->GetFileName();
	if (lpFileName == NULL)
		return ERROR_NOT_SEARCH_MODE;
	g_StdOut.WideCharToMultiByte(lpFileName, _lpBuffer, _nSize);
	return 0;
}

DWORD WINAPI SevenZipGetOriginalSize(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetOriginalSize();
}

DWORD WINAPI SevenZipGetCompressedSize(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetCompressedSize();
}

WORD WINAPI SevenZipGetRatio(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetRatio();
}

WORD WINAPI SevenZipGetDate(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetDate();
}

WORD WINAPI SevenZipGetTime(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetTime();
}

DWORD WINAPI SevenZipGetCRC(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetCRC();
}

int WINAPI SevenZipGetAttribute(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetAttribute();
}

// エントリのみ。エラー時以外、必ず0を返す
UINT WINAPI SevenZipGetOSType(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetOSType();
}

int WINAPI SevenZipGetMethod(HARC _harc, LPSTR _lpBuffer, const int _nSize)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return ERROR_HARC_ISNOT_OPENED;
	LPCWSTR lpMethod = pOpenArchive->GetMethod();
	if (lpMethod == NULL)
		return ERROR_NOT_SEARCH_MODE;
	g_StdOut.WideCharToMultiByte(lpMethod, _lpBuffer, _nSize);
	if (_nSize == 8 && pOpenArchive->GetArchiveType() == ARCHIVETYPE_7Z)
		_lpBuffer[strcspn(_lpBuffer, " :")] = '\0';
	return 0;
}

DWORD WINAPI SevenZipGetWriteTime(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetWriteTime();
}

DWORD WINAPI SevenZipGetCreateTime(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetCreateTime();
}

DWORD WINAPI SevenZipGetAccessTime(HARC _harc)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return -1;
	return pOpenArchive->GetAccessTime();
}

BOOL WINAPI SevenZipGetWriteTimeEx(HARC _harc, FILETIME *_lpftLastWriteTime)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetWriteTimeEx(_lpftLastWriteTime);
}

BOOL WINAPI SevenZipGetCreateTimeEx(HARC _harc, FILETIME *_lpftLastWriteTime)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetCreateTimeEx(_lpftLastWriteTime);
}

BOOL WINAPI SevenZipGetAccessTimeEx(HARC _harc, FILETIME *_lpftLastWriteTime)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetAccessTimeEx(_lpftLastWriteTime);
}

BOOL WINAPI SevenZipGetArcCreateTimeEx(HARC _harc, FILETIME *_lpftCreationTime)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetArcCreateTimeEx(_lpftCreationTime);
}

BOOL WINAPI SevenZipGetArcAccessTimeEx(HARC _harc, FILETIME *_lpftLastAccessTime)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetArcAccessTimeEx(_lpftLastAccessTime);
}

BOOL WINAPI SevenZipGetArcWriteTimeEx(HARC _harc, FILETIME *_lpftLastWriteTime)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetArcWriteTimeEx(_lpftLastWriteTime);
}

BOOL WINAPI SevenZipGetArcFileSizeEx(HARC _harc, ULHA_INT64 *_lpllSize)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetArcFileSizeEx(_lpllSize);
}

BOOL WINAPI SevenZipGetArcOriginalSizeEx(HARC _harc, ULHA_INT64 *_lpllSize)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetArcOriginalSizeEx(_lpllSize);
}

BOOL WINAPI SevenZipGetArcCompressedSizeEx(HARC _harc, ULHA_INT64 *_lpllSize)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetArcCompressedSizeEx(_lpllSize);
}

BOOL WINAPI SevenZipGetOriginalSizeEx(HARC _harc, ULHA_INT64 *_lpllSize)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetOriginalSizeEx(_lpllSize);
}

BOOL  WINAPI SevenZipGetCompressedSizeEx(HARC _harc, ULHA_INT64 *_lpllSize)
{
	COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
	if (pOpenArchive == NULL)
		return FALSE;
	return pOpenArchive->GetCompressedSizeEx(_lpllSize);
}

BOOL WINAPI SevenZipSetOwnerWindow(HWND _hwnd)
{
	return CProgressDialog::SetOwnerWindow(_hwnd);
}

BOOL WINAPI SevenZipClearOwnerWindow()
{
	return CProgressDialog::ClearOwnerWindow();
}

BOOL WINAPI SevenZipSetOwnerWindowEx(HWND _hwnd, LPARCHIVERPROC _lpArcProc)
{
	return CProgressDialog::SetOwnerWindowEx(_hwnd, _lpArcProc);
}

BOOL WINAPI SevenZipKillOwnerWindowEx(HWND _hwnd)
{
	return CProgressDialog::KillOwnerWindowEx(_hwnd);
}

BOOL WINAPI SevenZipSetOwnerWindowEx64(HWND _hwnd, LPARCHIVERPROC _lpArcProc, DWORD _dwStructSize)
{
	return CProgressDialog::SetOwnerWindowEx64(_hwnd, _lpArcProc, _dwStructSize);
}

BOOL WINAPI SevenZipKillOwnerWindowEx64(HWND _hwnd)
{
	return CProgressDialog::KillOwnerWindowEx64(_hwnd);
}

WORD WINAPI SevenZipGetSubVersion()
{
	return (SEVENZIP32_VERSION % 10000);
}

int WINAPI SevenZipGetArchiveType(LPCSTR _szFileName)
{
	COpenArchive arc;
	if (arc.OpenCheck(g_StdOut.ConvertUnicodeString(_szFileName), CHECKARCHIVE_RAPID) == E_FAIL)
		return -1;
	return arc.GetArchiveType();
}

BOOL WINAPI SevenZipSetUnicodeMode(BOOL _bUnicode)
{
	return g_StdOut.SetUnicodeMode(_bUnicode);
}

int WINAPI SevenZipSetDefaultPassword(HARC _harc, LPCSTR _szPassword)
{
	if (_harc == NULL)
		g_StdOut.SetDefaultPassword(_szPassword);
	else
	{
		COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
		if (pOpenArchive == NULL)
			return ERROR_HARC_ISNOT_OPENED;
		pOpenArchive->SetDefaultPassword(_szPassword);
	}
	return 0;
}

DWORD WINAPI SevenZipGetDefaultPassword(HARC _harc, LPSTR _szPassword, DWORD _dwSize)
{
	LPCWSTR lpPassword;
	if (_harc == NULL)
		lpPassword = g_StdOut.GetDefaultPassword();
	else
	{
		COpenArchive* pOpenArchive = COpenArchive::FindObject(_harc);
		if (pOpenArchive == NULL)
			return -1;
		lpPassword = pOpenArchive->GetDefaultPassword();
	}
	if (lpPassword == NULL)
		return -1;
	AString strPassword;
	if (g_StdOut.GetUnicodeMode())
		::ConvertUnicodeToUTF8(lpPassword, strPassword);
	else
		strPassword = ::GetOemString(lpPassword);
	if ((DWORD)strPassword.Length() >= _dwSize)
		return strPassword.Length() + 1;
	::lstrcpyn(_szPassword, strPassword, _dwSize);
	return 0;
}

int WINAPI SevenZipPasswordDialog(HWND _hwnd, LPSTR _szBuffer, DWORD _dwSize)
{
	CPasswordDialog dlg(_hwnd, IDS_ENTER_PASSWORD);
	if (dlg.DoModal() == IDCANCEL)
		return -1;
	AString strPassword;
	if (g_StdOut.GetUnicodeMode())
		::ConvertUnicodeToUTF8(dlg.GetPassword(), strPassword);
	else
		strPassword = ::GetOemString(dlg.GetPassword());
	::lstrcpynA(_szBuffer, strPassword, _dwSize);
	return (DWORD)strPassword.Length() < _dwSize ? 0 : 1;
}

BOOL WINAPI SevenZipSetPriority(const int _nPriority)
{
	return g_StdOut.SetPriority(_nPriority);
}

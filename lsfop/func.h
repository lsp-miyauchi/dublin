//{{NO_DEPENDENCIES}}
//
// FUNCTION PROTOTYPE DEFINITION
//

#pragma once

#ifndef __LOCAL_FUNCTION_PROTOTYPE_H__
#define __LOCAL_FUNCTION_PROTOTYPE_H__


///////////////////////////////////////////////////////////////////////
// prptotype function written by C
//

typedef BOOL (CALLBACK* DLGFN)(HWND, UINT, WPARAM, LPARAM);

class CFileInfo;
class CFileArray;

///////////////////////////////////////////////////////////////////////
// common.cpp
//

int __fastcall FormatFileDate(FILETIME* pft, LPTSTR pszDate, int cch);
int __fastcall FormatFileTime(FILETIME* pft, LPTSTR pszTime, int cch);
int __fastcall FormatFileDateTime(FILETIME* pft, LPTSTR pszDate, int cch);
int __fastcall FormatFileDateTimeEx(FILETIME* pft, LPTSTR pszDate, int cch);
int __fastcall FormatFileDateTimeWin(FILETIME* pft, LPTSTR pszDate, int cchMax);

LPTSTR __fastcall SkipTabSpace(LPTSTR p);
int SeparateString(LPCTSTR pszText, int delim, CSimpleArray<CString>& array);
BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* lpwfd);
BOOL __fastcall GetFileInfo(LPCTSTR pszName, CFileInfo& fi);
int AppMessage(UINT nTitleID, UINT nTextID, LPCTSTR pszInsert = NULL, UINT nType = MB_STOP);
int AppMessage(LPCTSTR pszTitle, LPCTSTR pszMsg, LPCTSTR pszInsert = NULL, UINT nType = MB_STOP);
void GetErrorString(DWORD code, CString& str);
int ErrorBox(HWND hwnd, DWORD code, LPCTSTR pszCaption, LPCTSTR pszAddtional, int typeAction);
int LastError(UINT idCaption, LPCTSTR pszTitle, UINT nType = MB_STOP);
BOOL MakeDir(LPCTSTR pszPath);
BOOL MakeTree(LPCTSTR pszPath);
BOOL FileBrowse(BOOL bRead, HWND hwnd, _RES_STRING caption, _RES_STRING filter, LPTSTR pszInitialDir, LPTSTR pszName);
BOOL FileBrowseMultiSel(HWND hwnd, _RES_STRING caption, _RES_STRING filter, LPTSTR pszInitialDir, LPTSTR pszName, int cchMax);
int DoMessages();
BOOL HasFiles(LPCTSTR lpPath);
HICON __fastcall GetFileIcon(LPCTSTR lpName, int typeIcon = SHIL_LARGE);
void ReplaceString(LPTSTR lpText, LPCTSTR lpSrc, LPCTSTR lpDst);
void __cdecl FormatStringV(CString& str, UINT nIDS, ...);
void StringSerialize(CString& str, CFile& file, BOOL bRead);
BOOL GetAppDataFolder(LPTSTR pszFolder);
void __fastcall initcrc16tbl(WORD* pcrctbl);
WORD __fastcall crc16(WORD* pcrctbl, int n, BYTE* pcb, WORD crc);
int RegLoadHistory(CSimpleArray<CString>& array, LPCTSTR pszSec, LPCTSTR pszKey, int maxEntry);
int RegLoadHistoryEx(HWND hCombo, LPCTSTR pszSec, LPCTSTR pszKey, int maxEntry);
BOOL RegSaveHistory(LPCTSTR pszSec, LPCTSTR pszKey, int maxEntry, LPCTSTR pszString);
void RegClearHistory(LPCTSTR pszSec, LPCTSTR pszKey);
void RegClearHistoryEx(HWND hCombo, LPCTSTR pszSec, LPCTSTR pszKey);
BOOL WINAPI fopFindExecutable(LPCTSTR pszFile, LPTSTR pszExec);
//@ void WINAPI fopAdjustStaticCtrlString(HWND hctl, LPTSTR pszText);
void __fastcall ReplaceInvalidToken(LPTSTR pszFile, TCHAR chReplaceWith = _T('\0'));
void __fastcall ConvertInvalidToken(CString& str);
BOOL GetFopLocalDataFolder(LPTSTR pszFolder);

DWORD __fastcall _getfileinfo(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
BOOL __fastcall _pathfileexists(LPCTSTR pszName);
BOOL __fastcall _pathisfile(LPCTSTR pszName);
BOOL __fastcall _pathisdirectory(LPCTSTR pszName);
int __fastcall _pathcompare(LPCTSTR src, LPCTSTR dst);
int __fastcall _pathisparent(LPCTSTR parent, LPCTSTR child);
BOOL __fastcall _pathdecorate(LPCTSTR pszOriginal, LPTSTR pszDecolate, int cchMax);
void __fastcall _pathaddsuffix(LPCTSTR pszOriginal, LPCTSTR pszSuffix, LPTSTR pszDecolate, int cchMax);
LPTSTR __fastcall FormatByteSize(DWORDLONG dlNum, LPTSTR pszFormat, int cch);
void FileSizeInformation(DWORDLONG dlNum, CString& str);
BOOL __fastcall PathGetUNCServerShare(LPCTSTR pPath, LPTSTR pShare);
LPTSTR __fastcall PathGetRootFromPath(LPCTSTR pszPath, LPTSTR pszRoot);
BOOL __fastcall PathSetUnique(LPTSTR pszName, int renametype = 0);
void __fastcall SetDialogIcon(HWND hdlg, UINT idIcon);
void __fastcall RemoveDialogIcon(HWND hdlg);
BOOL __fastcall _movefile(LPCTSTR pszExist, LPCTSTR pszNew);
HANDLE __fastcall _createfile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, LPSECURITY_ATTRIBUTES psa, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile);
BOOL __fastcall validate_makefile(LPTSTR pszName);
BOOL __fastcall _createdirectory(LPCTSTR pName, LPSECURITY_ATTRIBUTES psa);
BOOL __fastcall _removedirectory(LPCTSTR pszPath);
void __fastcall _time_NTFS2FAT(FILETIME& ft);
int __fastcall GetAnotherDataStream(LPCTSTR pszName);
void __fastcall ShowHtmlHelp(HWND hwnd, UINT idDlg);
void __fastcall ShowCtrlHelp(HWND hwnd, UINT idCtrl);
BOOL __fastcall WriteAnsiString(HANDLE hf, LPCTSTR psz, int cchMax);
bool __fastcall PathIsCommonPrefixFolder(LPCTSTR pSrc, LPCTSTR pDst);
BOOL __fastcall RegGetAppPath(LPCTSTR pszAppName, LPTSTR pszPath);
void __fastcall InitDlgCtrl(HWND hctl, LPCTSTR pszResource);
void EnumFileDir(LPCTSTR pszPath, LPCTSTR pszFilter, CSimpleArray<CString>& arrFile, CSimpleArray<CString>& arrFolder);
void FormatFromToString(LPCTSTR pszFrom, LPCTSTR pszTo, CString& str);
COLORREF calc_mixed_color(COLORREF clrA, COLORREF clrB, int perA);
void ReadStringFromStream(IStream* pstrm, CString& str);
void WriteStringToStream(IStream* pstrm, CString& str);
void FillShadeRect(HDC hdc, RECT* prc, COLORREF crFrom, COLORREF crTo, BOOL bVert);
BOOL ValidateFolderConnection(HWND hwnd, LPCTSTR pszFolder);
HRESULT CoCreateInstanceAsAdmin(HWND hwnd, bool bAdmin, REFCLSID rclsid, REFIID riid, __out void ** ppv);

int wsz2utf(const WCHAR* wsz, char* utf, int cchMax);
int utf2wsz(const char* utf, WCHAR* wsz, int cchMax);
int utf2str(const char* utf, CString& str);

int CreateBitmapMapArray(UINT idResource, CSimpleMap<UINT, HBITMAP>& arr);
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

///////////////////////////////////////////////////////////////////////
// sync.cpp
//

void GetReportTempFile(LPTSTR lpName);


///////////////////////////////////////////////////////////////////////
// syncres.cpp
//

void OpenSyncReport(HWND hParent, TiXmlDocument* pXmlDoc);
TiXmlDocument* LoadSyncReport(LPCTSTR pszFile);
void SaveSyncReport(TiXmlDocument* pXmlDoc);

///////////////////////////////////////////////////////////////////////
// name.cpp
//

BOOL GetUnpackData(HWND hwnd, int nType);

///////////////////////////////////////////////////////////////////////
// rename.cpp
//

BOOL RenameFileOrDirectory(LPCTSTR oldname, LPCTSTR newname);

///////////////////////////////////////////////////////////////////////
// copy.cpp
//

DWORD CopyFileSilent(LPCTSTR pSrc, LPCTSTR pDst);

///////////////////////////////////////////////////////////////////////
// delete.cpp
//

int TrashOneFile(HWND hwnd, LPTSTR pszFileName, FILEOP_FLAGS fFlags);

///////////////////////////////////////////////////////////////////////
// chdir.cpp
//

bool validate_dir(HWND hwndOwner, LPCTSTR path, bool bConfirm);

///////////////////////////////////////////////////////////////////////
// attrib.cpp
//

BOOL SetFileDateTimes(LPCTSTR pszName, LPFILETIME pftCreate, LPFILETIME pftWrite, LPFILETIME pftAccess);
BOOL SetFileProperties(LPCTSTR pszFile, DWORD attr, FILETIME* pftLast, FILETIME* pftCreate);


///////////////////////////////////////////////////////////////////////
// copy.cpp
//

BOOL ExecDropFileOperation(HWND hOwner, HDROP hdrop, DWORD eff, LPCTSTR pszDropPath, bool bSaveFolderHierarchy);
BOOL ExecDropFileOperationEx(DROPFILEOPSTRUCT* info);
BOOL CopyOrMoveFile(HWND hOwner, LPCTSTR lpSrc, LPCTSTR lpDst, BOOL bCopy);
BOOL CopyOrMoveFileEx(COPYORMOVEINFOSTRUCT* info);
HRESULT CreateLinkObject(LPCTSTR pszSrc, LPCTSTR pszDst, LPCTSTR pszDesc, LPCTSTR pszWorkdir, BOOL bIDList);

///////////////////////////////////////////////////////////////////////
// copysid.cpp
//

BOOL CopyAccessRights(LPCTSTR account, LPCTSTR src, LPCTSTR dst);

///////////////////////////////////////////////////////////////////////
// poppkeys.cpp
//

bool GetItemProperty(LPCTSTR name, const SHCOLUMNID* pscid, VARIANT* pvr);
void VariantTimeToFileTime(VARIANT* pvr, FILETIME* pft, bool bLocalTime = false);
void PropVariantTimeToFileTime(PROPVARIANT* ppvr, FILETIME* pft, bool bLocalTime = false);


#endif		// __LOCAL_FUNCTION_PROTOTYPE_H__

//////////////////////////////////////////////////////////////////////
// lspop.h

#ifndef UNICODE
	#error lspop.h requires UNICODE base compile
#endif

#ifndef __LSFOP_H__
#define __LSFOP_H__


#define LODWORD(a)				(*((DWORD*)&(a)))
#define HIDWORD(a)				(*(((DWORD*)&(a)) + 1))
#define SETDWORDLONG(a,b,c)		(*((DWORD*)&(a))=(DWORD)(b),*(((DWORD*)&(a))+1)=(DWORD)(c))

////////////////////////////////////////////////
// ファイルシステムタイプ
//

enum {
	FSTYPE_UNKNOWN,
	FSTYPE_FAT,
	FSTYPE_FAT32,
	FSTYPE_NTFS,
	FSTYPE_CDFS,
	FSTYPE_UDF,
	FSTYPE_HPFS,
	FSTYPE_EXFAT,
};

////////////////////////////////////////////////
// ファイル情報
//

enum {
	FINFO_GENERIC = 0,
	FINFO_ICON,
	FINFO_EXECUTE,
	FINFO_LINK,
};

////////////////////////////////////////////////
// 完了フラグ

#define CR_ERROR		-1
#define CR_DONE			0
#define CR_IGNORED		1
#define CR_ABORT		2


////////////////////////////////////////////////
// ファイル操作種別
//

typedef enum {
	FAC_QUERY,
	FAC_NEWONLY,
	FAC_OVERWRITE,
	FAC_RENAMEDST,
	FAC_IGNORE,
	FAC_MERGE,
	FAC_RENAMESRC
} FOPACTION;

////////////////////////////////////////////////
//

typedef struct _COPYORMOVEINFOSTRUCT {
	HWND hwnd;
	LPCTSTR pszSource;
	LPCTSTR pszDestination;
	BOOL bCopy;
	BOOL bSaveFolderHierarchy;
	BOOL bYesToAll;
	FOPACTION fAction;
} COPYORMOVEINFOSTRUCT;

typedef struct _DROPFILEOPSTRUCT {
	HWND hwnd;
	HDROP hdrop;
	LPCTSTR pszDestination;
	DWORD dropEffect;
	BOOL bSaveFolderHierarchy;
	BOOL bYesToAll;
	FOPACTION fAction;
} DROPFILEOPSTRUCT;

////////////////////////////////////////////////
// CPE暗号・復号エラー値

#define CPEE_GENERAL			-1
#define CPEE_FILENOTFOUND		-2
#define CPEE_OPEN				-3
#define CPEE_READ				-4
#define CPEE_WRITE				-5
#define CPEE_NOMEMORY			-6
#define CPEE_PASSINCORRECT		-7
#define CPEE_CRC				-8
#define CPEE_NOTCPE				-9
#define CPEE_NOPASS				-10
#define CPEE_PASSCHARSET		-11
#define CPEE_USERCANCEL			-12
#define CPEE_PATHNOTFOUND		-13


////////////////////////////////////////////////
// 削除フラグ

#define DELETE_ABORT	-2
#define DELETE_NO		-1
#define DELETE_NORMAL	0
#define DELETE_OK		1
#define DELETE_ALL		2

typedef struct tagDELSTAT {
	int nEntry;
	BOOL bConfirm;
	int nAction;
	BOOL bAnyFile;
	LPCTSTR lpName;
	DWORD dwAttr;
	FILETIME ftLastWriteTime;
	LPARAM param;							// オプショナリ
	BOOL bCanceled;
} DELSTAT;
typedef DELSTAT* LPDELSTAT;

////////////////////////////////////////////////
//

enum {
	DIVIDETYPE_NORMAL = 0,
	DIVIDETYPE_CAB,
	DIVIDETYPE_7ZIP,
};

typedef struct _tagDIVIDEDATA {
	TCHAR szTarget[MAX_PATH * 2];
	TCHAR szDivideTo[MAX_PATH * 2];
	TCHAR szPrefix[MAX_PATH * 2];
	DWORDLONG nFileSize;
	int nDivideSize;
	int nDivideType;
	BOOL bMakeBatch;
} DIVIDEDATA;
typedef DIVIDEDATA* LPDIVIDEDATA;


////////////////////////////////////////////////////////////////

#pragma pack(1)

typedef struct tagFAT_DEVICEPARAMS
{
	BYTE bSepcFunc;
	BYTE bDevType;
	WORD wDevAttr;
	WORD wCylinders;
	BYTE bMediaType;

	// 以下 BPB
	WORD wBytesPerSector;
	BYTE bSectorsPerCluster;
	WORD wReservedSectors;
	BYTE bNumberOfFATs;
	WORD wRootEntries;
	WORD wTotalSectors;
	BYTE bMediaDescriptor;
	WORD wSectorsPerFAT;
	WORD wSectorsPerTrack;
	WORD wHeads;
	WORD wHiddenSectors;
	WORD wHiddenSectorsHigh;
	WORD wBigTotalSectors;
	WORD wBigTotalSectorsHigh;
	WORD wBigSectorsPerFat;
	WORD wBigSectorsPerFatHi;
	WORD wExtFlags;
	WORD wFSVersion;
	WORD wRootDirStrtClus;
	WORD wRootDirStrtClusHi;
	WORD wFSInfoSec;
	WORD wBkUpBootSec;
	WORD wReserved[6];

	WORD wTrackTableEntries;
	BYTE bSectorTable[64 * 256];
} FAT_DEVICEPARAMS, *LPFAT_DEVICEPARAMS;


////////////////////////////////////////////////////////////////
// ショートカットファイルの情報収集

typedef struct tagFSLINKINFO {
	WIN32_FIND_DATA wfd;
	TCHAR szName[MAX_PATH * 2];
	TCHAR szTargetName[MAX_PATH * 2];
	TCHAR szWorkDir[MAX_PATH * 2];
	TCHAR szArguments[MAX_PATH * 2];
	TCHAR szDescription[MAX_PATH * 2];
	TCHAR szIconPath[MAX_PATH * 2];
	int nIconIndex;
} FSLINKINFO, *LPFSLINKINFO;


////////////////////////////////////////////////////////////////
// 日付時刻範囲

#define DTR_START		0x00000001
#define DTR_END			0x00000002
#define DTR_BOTH		(DTR_START | DTR_END)

typedef struct tagDTRANGE {
	DWORD flags;
	SYSTEMTIME tm_start;
	SYSTEMTIME tm_end;
} DTRANGE;

////////////////////////////////////////////////////////////////
// DROPFILEOPERATION 情報

typedef struct tagDROPFILEOPERATION {
	DWORD cbSize;
	HWND hwnd;
	HDROP hDrop;
	DWORD dwEffect;
	LPCTSTR pszDropPath;
	BOOL bAsAdmin;
	BOOL bAsyncOperation;
	BOOL bCopyAccessRights;
	BOOL bSaveFolderHierarchy;
} DROPFILEOPERATION;

////////////////////////////////////////////////////////////////
// ディスクスペース情報

typedef struct tagDISKSPACEINFO {
	DWORDLONG dlFreeCaller;
	DWORDLONG dlTotal;
	DWORDLONG dlFreeTotal;
	DWORD dwSectorsPerCluster;
	DWORD dwBytesPerSector;
	DWORD dwBytesPerCluster;
	DWORD dwFreeClusters;
	DWORD dwTotalClusters;
	int nFileSystemType;
	TCHAR szVolume[64];
	TCHAR szFileSystem[64];
	DWORD dwVolser;
	DWORD dwFileSystemFlags;
} DISKSPACEINFO;

#pragma pack()

#define VERIFY_FIXED		0x00000001
#define VERIFY_REMOVABLE	0x00000002
#define VERIFY_REMOTE		0x00000004


////////////////////////////////////////////////
//

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

int WINAPI fopBrowseForFolder(HWND hParent, LPTSTR lpPath);
int WINAPI fopExpandSyncPath(LPTSTR psz, int cchMax);
int WINAPI fopSubstSyncPath(LPTSTR psz, int cchMax);
void WINAPI fopEnumFileSize(HWND hParent, HDROP hDrop);
void WINAPI fopEnumPathSize(HWND hParent, LPCTSTR lpDir);
BOOL WINAPI fopFileAttributes(HWND hwnd, HDROP hDrop);
void WINAPI fopFilterDelete(HWND hParent, LPCTSTR lpPath);
void WINAPI fopFilterDeleteEx(HWND hParent, HDROP hdrop);
void WINAPI fopDeleteFile(HWND hParent, HDROP hDrop);
int WINAPI fopRemoveFile(LPCTSTR pszName, LPDELSTAT pds);
int WINAPI fopTrashFile(HWND hwnd, LPTSTR lpFileNames, FILEOP_FLAGS fFlags);
BOOL WINAPI fopRemoveTreeNoConfirm(LPCTSTR lpPath);
int WINAPI fopRemoveFileNoConfirm(LPCTSTR pszName);
BOOL WINAPI fopFileShredder(HWND hwnd, HDROP hDrop, BOOL bNeedConfirm);
BOOL WINAPI fopEraseFile(HDROP hDrop);
void WINAPI fopFormatDrive(HWND hwndOwner);
BOOL WINAPI fopMultiSyncDir(HWND hwnd);
BOOL WINAPI fopScriptSyncNew(HWND hwnd);
int WINAPI fopQueryDivide(HWND hwnd, LPDIVIDEDATA lpdd);
BOOL WINAPI fopDivideFile(HWND hwnd, LPDIVIDEDATA lpdd);
BOOL WINAPI fopMergeFile(HWND hwnd, HDROP hDrop);
void WINAPI fopDriveInfo(HWND hwnd, int nDrive);
BOOL WINAPI fopRename(HWND hwnd, LPTSTR lpName);
BOOL WINAPI fopRenameFiles(HWND hwnd, HDROP hdrop);
BOOL WINAPI fopShellRename(HWND hwnd, LPCITEMIDLIST pidlTarget);
BOOL WINAPI fopShellRenameItems(HWND hwnd, IDataObject* pdo);
BOOL WINAPI fopGetNewName(HWND hwnd, LPTSTR pszPathName);
BOOL WINAPI fopCreateNewFolder(HWND hwnd, LPTSTR lpName);
BOOL WINAPI fopMakeTree(LPCTSTR pszPath);
BOOL WINAPI fopDropFileOperation(const DROPFILEOPERATION* pdfo);
BOOL WINAPI fopValidateDir(HWND hwndOwner, LPCTSTR path);
BOOL WINAPI fopMakeBriefcase(HWND hwnd, LPTSTR pszPath);
void WINAPI fopEmptyRecentFiles();
BOOL WINAPI fopFlushDrive(LPCTSTR pszRoot);
BOOL WINAPI fopGetFatDeviceParams(LPCTSTR pszRoot, LPFAT_DEVICEPARAMS pdp);
BOOL WINAPI fopGetIconPath(HWND hwnd, LPTSTR pszPath, int* pnIndex);
BOOL WINAPI fopExportIconFromFile(HWND hwnd, LPCTSTR pszName, LPCTSTR pszTo);
HDROP WINAPI fopDragInitBlock(POINT pt, BOOL fInNonClientArea);
HDROP WINAPI fopDragAppendFile(HDROP hDrop, LPCTSTR pszPath);
BOOL WINAPI fopRenameExtension(HWND hwnd, LPCTSTR pszFileName);
BOOL WINAPI fopSeriesName(HWND hwnd, HDROP hDrop);
BOOL WINAPI fopRepStrNames(HWND hwnd, HDROP hDrop);
BOOL WINAPI fopGetCryptFileInfo(LPCTSTR pszFile, WIN32_FIND_DATA* pwfd, LPTSTR pszText, int cchText);
int WINAPI fopEncryptFile(LPCTSTR pszSrc, LPCTSTR pszDst, LPCTSTR pszPass, LPCTSTR pszComment);
int WINAPI fopDecryptFile(LPCTSTR pszSrc, LPCTSTR pszPass);
BOOL WINAPI fopCryptFiles(HWND hwnd, HDROP hDrop, BOOL bCrypt);
BOOL WINAPI fopEncryptFilesTo(HWND hwnd, HDROP hDrop, LPCTSTR pszTo);
BOOL WINAPI fopDecryptFilesTo(HWND hwnd, HDROP hDrop, LPCTSTR pszTo);
int WINAPI fopGetCryptErrorString(int nErrorCode, LPTSTR pszText, int cchText);
BOOL WINAPI fopParseLinkInfo(LPCTSTR pszName, FSLINKINFO* pli);
int WINAPI shnEnumExtension(LPTSTR pszExt);
BOOL WINAPI shnCreateShellNew(LPCTSTR pszExt, LPCTSTR pszName, LPCTSTR pszDir);
BOOL WINAPI shnCreateNewFile(LPCTSTR pszExt, LPCTSTR pszDir);
BOOL WINAPI fopGetDirRemind(int nDrive, LPTSTR pszDir);
void WINAPI fopSetDirRemind(int nDrive, LPCTSTR pszDir);
BOOL WINAPI fopGetSyncDataFolder(LPTSTR pszFolder);
BOOL WINAPI fopChangeFileCaps(HWND hwnd, HDROP hDrop);
void WINAPI fopShowFileInfo(HWND hwnd, HDROP hdrop);
void WINAPI fopShowFileInfoEx(HWND hwnd, HDROP hdrop, int nPageType);
void WINAPI fopShowFileInformation(HWND hwnd, LPITEMIDLIST* pidls, int numItems, int nPageType);
void WINAPI fopAssociateExtension(HWND hwnd, LPCTSTR pszFileName);
HICON WINAPI fopGetIconFromInstance(HMODULE hmod, LPTSTR index, int width, int height);
BOOL WINAPI fopGetTaskFolder(LPTSTR pszPath, int cchMax);
BOOL WINAPI fopCreateJunctionUI(LPCTSTR pszLinkBase, LPCTSTR pszSrc);
BOOL WINAPI fopCreateJunction(LPCTSTR pszLink, LPCTSTR pszSrc);
DWORD WINAPI fopGetJunctionPointInfo(LPCTSTR pszMountDir, LPTSTR pszDestBuff, int cchMax);
DWORD WINAPI fopGetReparsePointInfo(LPCTSTR pszReparsePointFile, LPTSTR* ppszOriginalFile, DWORD* pdwTagType);
BOOL WINAPI fopRemoveJunction(LPCTSTR pszPath);
BOOL WINAPI fopCreateSymbolicLinkUI(HWND hwnd, LPCTSTR pszLinkBase, LPCTSTR pszSrc);
void WINAPI fopCalcFileHash(HWND hParent, HDROP hDrop);
void WINAPI fopShowIconDetail(HWND hwnd, LPCTSTR pszFile, int index);


BOOL WINAPI fopExportSingleIcon(LPCTSTR pszName, LPCTSTR pszExportIconName, int index);
BOOL WINAPI fopExportMultipleIcons(LPCTSTR pszName, LPCTSTR pszTo, int* pnIndex, int entries);
HANDLE WINAPI fopIcmOpen(LPCTSTR pszFileName);
void WINAPI fopIcmClose(HANDLE h);
HIMAGELIST WINAPI fopIcmGetImageList(HANDLE h, int cx);
int WINAPI fopIcmGetIconCount(HANDLE h);
LPCTSTR WINAPI fopIcmGetIconPath(HANDLE h);
HRESULT WINAPI fopIcmCreateDataObject(HANDLE h, const int* pIndex, int entries, IDataObject** ppdo);

HRESULT WINAPI fopGpsAccess(HWND hwnd, LPCTSTR pszFileName);
BOOL WINAPI fopHasGpsData(LPCTSTR pszFileName);
typedef BOOL(CALLBACK* ENUMMAPSERVICEPROC)(UINT id, LPCTSTR lpTitle, LONG_PTR lParam);
int WINAPI fopEnumMapServices(ENUMMAPSERVICEPROC cbfn, LONG_PTR lParam);
HRESULT WINAPI fopGetMapServiceUrl(UINT id, double lat, double lon, LPTSTR pServiceUrl, int maxLength);

#ifdef __cplusplus
}
#endif

#endif				// __LSFOP_H__

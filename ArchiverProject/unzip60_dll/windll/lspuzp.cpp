#ifndef WIN32   /* this code is currently only tested for 32-bit console */
  #define WIN32
#endif

#if defined(__WIN32__) && !defined(WIN32)
  #define WIN32
#endif

#include <intrin.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <mbctype.h>
#include <mbstring.h>
#include "windll.h"
#include "resource.h"
#include "unzpriv.h"
#include "lspuzp.h"
#include "openarchive.h"

#pragma comment(lib, "shlwapi.lib")


extern ParsedCommandLine parse_command_line(const std::string& cmdline);


void __cdecl dbgPrintf(const char* pszFormat, ...);

extern BOOL gbTryLastPassword;

/*
 */
static UzpArchive* OaFromHarc(HANDLE harc)
{
  auto poa = (UzpArchive*)harc;
  return (poa && poa->magic == OAMAGIC) ? poa : nullptr;
}

/*
 */
static std::shared_ptr<UzpFileItem> CurrentFileItem(HANDLE harc)
{
  auto poa = OaFromHarc(harc);
  return poa ? poa->pCurFileItem : nullptr;
}

////////////////////////////////////////////////////////////
// Forward References


int WINAPI LuzSingleEntryUnzip(int, char**, int, char**, LPDCL, LPUSERFUNCTIONS);

int WINAPI LuzDisplay(void*, LPCSTR, ULONG);
int WINAPI LuzReplace(void* pInfo, const char* filename);
int WINAPI LuzPassword(void* pInfo, char *, int, const char* );
void WINAPI LuzMessage64(void*, const PACKED_FILEITEM*);
int WINAPI LuzReport(void*, UINT, LPCSTR, LPCSTR, void*);

BOOL WINAPI PasswordDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI ProgressDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI ReplaceDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
int DlgModal(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam);
HWND DlgModeless(HWND hwnd, DLGPROC lpFn, int nID, void* pvParam);


////////////////////////////////////////////////////////////
//

LONG64 muldiv64(LONG64 a, LONG64 b, LONG64 c) {
  if (c == 0) return -1;

  LONG64 high;
  LONG64 low = _mul128(a, b, &high);
  LONGLONG remainder;
  return _div128(high, low, c, &remainder);
}

void ReplaceChar(LPSTR pszText, int nSrcChar, int nDstChar)
{
  char *p = pszText;
  while (p) {
    if (p = (char*)_mbschr((BYTE*)p, nSrcChar))
      *p++ = nDstChar;
  }
}

char* SkipTabSpace(char* p)
{
  if (p) {
    while (*p == ' ' || *p == '\t')
      p++;
  }
  return p;
}

int SeparateString(char* pszText, std::vector<char*>& argv)
{
  argv.clear();

  char* p = SkipTabSpace(pszText);
  while (p && *p) {
    char* q;
    if (*p == '"') {
      q = (char*)_mbschr((BYTE*)(++p), '"');
      if (q)
        *q++ = '\0';
      else
        break;
    }
    else {
      q = (char*)_mbschr((BYTE*)(p + 1), ' ');
      if (q)
        *q++ = '\0';
    }
    argv.push_back(p);
    p = SkipTabSpace(q);
  }
  return (int)argv.size();
}


/*
 *
 */
BOOL setswitchs(LPDCL pDcl, LPCSTR psw)
{
  while (psw && *psw) {
    switch (*psw) {
      case 'a':
        pDcl->naflag = TRUE;		// Convert CR to CRLF
        break;
      case '^':
        pDcl->fInsideVerb = TRUE;
      case 'v':
        pDcl->nvflag = TRUE;
        break;
      case 't':
        pDcl->ntflag = TRUE;
        break;
      case 'z':
        pDcl->nzflag = TRUE;
        break;
      case 'j':						// junk paths
        pDcl->ndflag = 0;
        break;
      case 'o':						// Over-write all files
        pDcl->noflag = TRUE;
        break;
      case 'q':
        pDcl->fQuiet = 1;
        psw++;
        if (*psw == 'q')
          pDcl->fQuiet = 2;
        break;
      case 'u':
        pDcl->nfflag = TRUE;
        break;
      case 'f':
        pDcl->ExtractOnlyNewer = TRUE;
        break;
      case 'x':
        break;
      case '-':
        psw++;
        if (*psw == 'i')
          pDcl->fProgress = FALSE;
        break;
      case 'P':						// command line password
        psw++;
        if (*psw) {
          lstrcpyn(pDcl->password, psw, MAX_PATH);
          return TRUE;
        }
        return FALSE;
      default:
        break;
    }
    psw++;
  }
  return TRUE;
}

static BOOL PerseResponseFile(LPDCL pDcl, LPCSTR pszResp)
{
  FILE* fp = fopen(pszResp, "r");
  if (fp == nullptr)
    return FALSE;

  char str[MAX_PATH * 4];
  while (fgets(str, sizeof(str), fp)) {
    int len = lstrlen(str);
    if (len > 0 && str[len - 1] == '\n')
      str[len - 1] = '\0';

    std::vector<char*> argv;
    if (SeparateString(str, argv) > 0) {
      for (auto pa : argv) {
        if (pDcl->iMaxFilters < pDcl->iNumFilters + 1) {
          char** ppw = (char**)realloc(pDcl->ppFilters, sizeof(char*) * (pDcl->iMaxFilters + 256));
          if (!ppw)
            break;

          pDcl->ppFilters = ppw;
          pDcl->iMaxFilters += 256;
        }
        pDcl->ppFilters[pDcl->iNumFilters++] = StrDup(pa);
      }
    }
  }
  fclose(fp);
  return TRUE;
}

BOOL WINAPI DllGetFunctionPrefix(LPSTR pszPrefix, LONG cchMax)
{
  if (pszPrefix) {
    (void)lstrcpyn(pszPrefix, "Unzip", cchMax);
    return TRUE;
  }
  return FALSE;
}

/*
 *
 */
int WINAPI Unzip(HWND hwnd, LPCSTR pszCmd, LPSTR pszResult, DWORD dwSize)
{
  LPDCL pDcl = (LPDCL)malloc(sizeof(DCL));
  if (!pDcl)
    return -1;

  LPUSERFUNCTIONS pUserFn = (LPUSERFUNCTIONS)malloc(sizeof(USERFUNCTIONS));
  if (!pUserFn) {
    free(pDcl);
    return -1;
  }

  // コールバック関数の設定
  pUserFn->password = LuzPassword;
  pUserFn->print = LuzDisplay;
  pUserFn->replace = LuzReplace;
  pUserFn->SendApplicationMessage64 = LuzMessage64;
  pUserFn->report = LuzReport;
  pUserFn->pDcl = pDcl;

  if (pszResult && dwSize > 0) {
    *pszResult = '\0';
    pDcl->pResult = pszResult;
    pDcl->nResultLength = dwSize;
  }
  else
    pDcl->nResultLength = 0;

  pDcl->hwndParent = hwnd;
  pDcl->hwndProgress = NULL;
  pDcl->pResult = pszResult;
  pDcl->nReplace = 0;
  // デフォルト処理フラグの設定
  pDcl->StructVersID = UZ_DCL_STRUCTVER;
  pDcl->ExtractOnlyNewer = FALSE;
  pDcl->SpaceToUnderscore = FALSE;      // true if convert space to underscore
  pDcl->PromptToOverwrite = TRUE;       // true if prompt to overwrite is wanted
  pDcl->fQuiet = 0;                     // 0 = all messages, 1 = fewer messages, 2 = no messages
  pDcl->ncflag = FALSE;                 // Write to stdout if true
  pDcl->ntflag = FALSE;                 // test zip file if true
  pDcl->nvflag = FALSE;                 // give a verbose listing if true
  pDcl->nfflag = FALSE;                 // Do not extract only newer
  pDcl->nzflag = FALSE;								  // display a zip file comment if true
  pDcl->ndflag = TRUE;                  // Recreate directories if true:
                                        //    0 = junk paths from filenames
                                        //    1 = "safe" usage of paths in filenames (skip ../)
                                        //    2 = allow unsafe path components (dir traversal)
  pDcl->noflag = FALSE;                 // Over-write all files if true
  pDcl->naflag = FALSE;                 // Do not convert CR to CRLF

  pDcl->nZIflag = FALSE;                // get ZipInfo output if TRUE
  pDcl->B_flag = FALSE;                 // backup existing files if TRUE
  pDcl->C_flag = FALSE;                 // ignore case if TRUE
  pDcl->D_flag = 0;                     // controls restoration of timestamps
                                        //    0 = restore all timestamps (default)
                                        //    1 = skip restoration of timestamps for folders
                                        //        created on behalf of directory entries in the Zip archive
                                        //    2 = no restoration of timestamps; extracted files
                                        //        and dirs get stamped with current time */
  pDcl->U_flag = 0;                     // controls UTF-8 filename coding support
                                        //    0 = automatic UTF-8 translation enabled (default)
                                        //    1 = recognize UTF-8 coded names, but all non-ASCII  characters are "escaped" into "#Uxxxx"
                                        //    2 = UTF-8 support is disabled, filename handling works exactly as in previous UnZip versions */
  pDcl->fPrivilege = 0;                 // ACLs, 1 => restore ACLs in user mode,


  pDcl->zipfile[0] = '\0';              // The archive name
  pDcl->extractDir[0] = '\0';           // The directory to extract to. This is set
                                        // to NULL if you are extracting to the
                                        // current directory.
  pDcl->password[0] = '\0';            // command line password
  pDcl->fProgress = TRUE;

  // swithes
  pDcl->iNumFilters = 0;
  pDcl->ppFilters = (char**)malloc(sizeof(char*) * 256);
  pDcl->iMaxFilters = 256;


  ParsedCommandLine parsed = parse_command_line(pszCmd);
  for (auto p : parsed.argv) {
    if (*p == '-')					// indicator
      setswitchs(pDcl, p + 1);
    else if (*p == '@')				// file-list
      PerseResponseFile(pDcl, ++p);
    else {
      int len;
      len = lstrlen(p) - 1;
      if (p[len] == '\\' && _mbsbtype((BYTE*)p, len) == _MBC_SINGLE) {
        // 解凍先フォルダ
        StrCpyN(pDcl->extractDir, p, std::size(pDcl->extractDir));
      }
      else if (pDcl->zipfile[0] == '\0') {
        // ZIP書庫名
        StrCpyN(pDcl->zipfile, p, std::size(pDcl->zipfile));
      }
      else {
        // 解凍ファイル
        if (pDcl->iMaxFilters < pDcl->iNumFilters + 1) {
          char** ppw = (char**)realloc(pDcl->ppFilters, sizeof(char*) * (pDcl->iMaxFilters + 256));
          if (!ppw)
            break;

          pDcl->ppFilters = ppw;
          pDcl->iMaxFilters += 256;
        }
        pDcl->ppFilters[pDcl->iNumFilters++] = _strdup(p);
      }
    }
  }

  // 解凍除外ファイルはここでは無視！
  int exfc = 0;
  char** exfv = (char**)0;

  if (!pDcl->ntflag && !pDcl->nvflag) {
    if (pDcl->extractDir[0] == '\0') {
      GetCurrentDirectory(MAX_PATH * 2, pDcl->extractDir);
    }

    if (pDcl->fProgress) {
      pDcl->hwndProgress = DlgModeless(hwnd, (DLGPROC)ProgressDlgFn, IDD_RELAY, 0);
      if (pDcl->hwndProgress)
        PathSetDlgItemPath(pDcl->hwndProgress, IDC_FROM, pDcl->zipfile);
      ShowWindow(pDcl->hwndProgress, SW_SHOW);
      UpdateWindow(pDcl->hwndProgress);
    }
  }

  int result = LuzpSingleEntryUnzip(pDcl->iNumFilters, pDcl->ppFilters, exfc, exfv, pUserFn);
  if (pDcl->hwndProgress)
    DestroyWindow(pDcl->hwndProgress);

  if (pDcl->iNumFilters > 0) {
    for (int i = 0; i < pDcl->iNumFilters; i++)
      free(pDcl->ppFilters[i]);
  }
  free(pDcl->ppFilters);
  free(pDcl);
  free(pUserFn);

  if (result != 0) {
    switch (result) {
      case IZ_CTRLC: result = ERROR_USER_CANCEL; break;
      case PK_WARN: result = 0; break;		// 警告はno-error
      case PK_MEM:
      case PK_MEM2:
      case PK_MEM3:
      case PK_MEM4:
      case PK_MEM5: result = ERROR_ENOUGH_MEMORY; break;
      case PK_NOZIP: result = ERROR_NOT_ARC_FILE; break;
      case PK_PARAM: result = ERROR_COMMAND_NAME; break;
      case PK_FIND: result = ERROR_NOT_FILENAME; break;
      case PK_DISK: result = ERROR_CANNOT_WRITE; break;
      default: result = -1; break;
    }
  }
  return result;
}

/*
 *
 */
WORD WINAPI UnzipGetVersion()
{
  return 90;
}

/*
 *
 */
BOOL WINAPI UnzipGetRunning()
{
  return TRUE;
}

/*
 *
 */
BOOL WINAPI UnzipGetBackGroundMode()
{
  return FALSE;
}

/*
 *
 */
BOOL WINAPI UnzipSetBackGroundMode(BOOL bBkgndMode)
{
  return TRUE;
}

/*
 *
 */
BOOL WINAPI UnzipGetCursorMode()
{
  return TRUE;
}

/*
 *
 */
BOOL WINAPI UnzipSetCursorMode(BOOL bCursorMode)
{
  return TRUE;
}

/*
 *
 */
WORD WINAPI UnzipGetCursorInterval()
{
  return 0;
}

/*
 *
 */
BOOL WINAPI UnzipSetCursorInterval(WORD wInterval)
{
  return TRUE;
}

/*
 *
 */
BOOL WINAPI UnzipCheckArchive(LPCSTR pszName, int iMode)
{
  char cmd[MAX_PATH * 3];
  snprintf(cmd, sizeof(cmd), "-t \"%s\"", pszName);
  return (Unzip(NULL, cmd, NULL, 0) == 0) ? TRUE : FALSE;
}

/*
 *
 */
BOOL WINAPI UnzipQueryFunctionList(int iFunction)
{
  switch (iFunction) {
    case ISARC_GET_VERSION:
    case ISARC_CHECK_ARCHIVE:
    case ISARC_GET_FILE_COUNT:
    case ISARC_QUERY_FUNCTION_LIST:
    case ISARC_OPEN_ARCHIVE:
    case ISARC_CLOSE_ARCHIVE:
    case ISARC_FIND_FIRST:
    case ISARC_FIND_NEXT:
    case ISARC_GET_CURSOR_INTERVAL:
    case ISARC_SET_CURSOR_INTERVAL:
    case ISARC_GET_BACK_GROUND_MODE:
    case ISARC_SET_BACK_GROUND_MODE:
    case ISARC_GET_CURSOR_MODE:
    case ISARC_SET_CURSOR_MODE:
      return TRUE;

    case ISARC:
    case ISARC_GET_RUNNING:
    case ISARC_GET_ARC_FILE_NAME:
    case ISARC_GET_ARC_FILE_SIZE:
    case ISARC_GET_ARC_ORIGINAL_SIZE:
    case ISARC_GET_ARC_DATE:
    case ISARC_GET_ARC_TIME:
    case ISARC_GET_FILE_NAME:
    case ISARC_GET_ORIGINAL_SIZE:
    case ISARC_GET_DATE:
    case ISARC_GET_TIME:
    case ISARC_CONFIG_DIALOG:
    case ISARC_HOUT:
    case ISARC_STRUCTOUT:
    case ISARC_GET_ARC_FILE_INFO:
    case ISARC_EXTRACT:
    case ISARC_ADD:
    case ISARC_MOVE:
    case ISARC_DELETE:
    case ISARC_GET_ARC_COMPRESSED_SIZE:
    case ISARC_GET_ARC_RATIO:
    case ISARC_GET_ARC_OS_TYPE:
    case ISARC_GET_ARC_IS_SFX_FILE:
    case ISARC_GET_COMPRESSED_SIZE:
    case ISARC_GET_RATIO:
    case ISARC_GET_CRC:							// CabGetCRC
    case ISARC_GET_ATTRIBUTE:					// CabGetAttribute
    case ISARC_GET_OS_TYPE:						// CabGetOSType
    case ISARC_GET_METHOD:						// CabGetMethod
    case ISARC_GET_WRITE_TIME:					// CabGetWriteTime
    case ISARC_GET_CREATE_TIME:					// CabGetCreateTime
    case ISARC_GET_ACCESS_TIME:					// CabGetAccessTime
    default:
      break;
  }
  return FALSE;
}

/*
 *
 */
HANDLE WINAPI UnzipOpenArchive(HWND hwnd, LPCSTR pszZip, DWORD /*dwMode*/)
{
  auto pufi = new UzpArchive(hwnd, pszZip);
  return (HANDLE)pufi;
}

/**/
int WINAPI UnzipCloseArchive(HANDLE harc)
{
  auto poa = OaFromHarc(harc);
  if (!poa)
    return -1;

  delete poa;
  return 0;
}

/*
 *
 */
int WINAPI UnzipFindNext(HANDLE harc, void* pidi)
{
  auto poa = OaFromHarc(harc);
  if (!pidi || !poa)
    return -1;

  if (poa->indexFineFile >= poa->filestack.size()) {
    poa->pCurFileItem = nullptr;
    return -1;
  }

  auto pii = poa->filestack[poa->indexFineFile];
  if (!pii)
    return -1;

  poa->pCurFileItem = pii;
  poa->indexFineFile++;

  auto pdv = (INDIVIDUALINFO*)pidi;
  memset(pdv, 0, sizeof(INDIVIDUALINFO));
  // ファイル名
  (void)lstrcpyn(pdv->szFileName, pii->fname.c_str(), std::size(pdv->szFileName));
  // オリジナルサイズ
  pdv->dwOriginalSize = (DWORD)pii->originalSize;
  // 圧縮サイズ
  pdv->dwCompressedSize = (DWORD)pii->compressedSize;
  // 圧縮比率
  if (pii->originalSize)
    pdv->wRatio = (WORD)muldiv64(pii->compressedSize, 1000, pii->originalSize);
  else
    pdv->wRatio = 1000;
  // 日付時刻
  FileTimeToDosDateTime(&pii->filetime, &pdv->wDate, &pdv->wTime);
  // CRC
  pdv->dwCRC = pii->crc;
  // 暗号化
  pdv->fCrypt = pii->fCrypt;
  // メソッド
  lstrcpy(pdv->szMode, pii->method.c_str());
  // 属性文字列変換
  pii->attrib2string(pdv->szAttribute, std::size(pdv->szAttribute));
  return 0;
}

/*
 *
 */
int WINAPI UnzipFindNext64(HANDLE harc, void* pidi)
{
  auto poa = OaFromHarc(harc);
  if (!pidi || !poa)
    return -1;

  if (poa->indexFineFile >= poa->filestack.size()) {
    poa->pCurFileItem = nullptr;
    return -1;
  }

  auto pii = poa->filestack[poa->indexFineFile];
  if (!pii)
    return -1;

  poa->pCurFileItem = pii;
  poa->indexFineFile++;

  auto pzfi = (PACKED_FILEITEM*)pidi;
  memset(pzfi, 0, sizeof(PACKED_FILEITEM));
  // ファイル名
  (void)lstrcpyn(pzfi->filename, pii->fname.c_str(), std::size(pzfi->filename));
  // オリジナルサイズ
  pzfi->originalSize = pii->originalSize;
  // 圧縮サイズ
  pzfi->compressedSize = pii->compressedSize;
  // 圧縮比率
  if (pii->originalSize)
    pzfi->ratio = (unsigned)muldiv64(pii->compressedSize, 1000, pii->originalSize);
  else
    pzfi->ratio = 1000;
  // CRC
  pzfi->crc = pii->crc;
  // 暗号化
  pzfi->crypt = pii->fCrypt;
  // メソッド
  lstrcpyn(pzfi->method, pii->method.c_str(), std::size(pzfi->method));
  return 0;
}

/*
 *
 */
static int _FindFirst_core(HANDLE harc, LPCSTR pszFilter, void* pidi)
{
  auto poa = OaFromHarc(harc);
  if (!poa) {
    std::cout << "UnzipFindFirst: Invalid handle magic\n";
    return -1;
  }

  // 既存のアイテムリストを削除
  poa->clear();
  if (pszFilter && lstrcmp(pszFilter, "*.*"))
    poa->filter = pszFilter;
  else
    poa->filter = "*";

  std::string cmd = std::format("-^ \"{}\" {}", poa->zipfile.c_str(), poa->filter.c_str());
  return Unzip(poa->hwndOwner, cmd.c_str(), (LPSTR)poa, 0);
}

/*
 *
 */
int WINAPI UnzipFindFirst(HANDLE harc, LPCSTR pszFilter, void* pidi)
{
  int res = _FindFirst_core(harc, pszFilter, pidi);
  if (res == 0)
    return UnzipFindNext(harc, pidi);
  std::cout << std::format("UnzipFindFirst: error({})\n", res);
  return -1;
}

/*
 *
 */
int WINAPI UnzipFindFirst64(HANDLE harc, LPCSTR pszFilter, void* pidi)
{
  int res = _FindFirst_core(harc, pszFilter, pidi);
  if (res == 0)
    return UnzipFindNext64(harc, pidi);
  std::cout << std::format("UnzipFindFirst64: error({})\n", res);
  return -1;
}

/*
 *
 */
BOOL WINAPI UnzipGetPackedFileData(HANDLE harc, PACKED_FILEITEM* pzfi)
{
  auto pi = CurrentFileItem(harc);
  if (!pi || !pzfi)
    return FALSE;

  memset(pzfi, 0, sizeof(PACKED_FILEITEM));
  pzfi->originalSize = pi->originalSize;
  pzfi->compressedSize = pi->compressedSize;
  // 圧縮比率
  if (pi->originalSize)
    pzfi->ratio = (unsigned)muldiv64(pi->compressedSize, 1000, pi->originalSize);
  else
    pzfi->ratio = 1000;
  FileTimeToSystemTime(&pi->filetime, &pzfi->fstime);
  pzfi->crc = pi->crc;
  pzfi->crypt = pi->fCrypt;
  pzfi->attributs = pi->attrib;
  (void)lstrcpyn(pzfi->filename, pi->fname.c_str(), std::size(pzfi->filename));
  (void)lstrcpyn(pzfi->method, pi->method.c_str(), std::size(pzfi->method));
  return TRUE;
}

/*
 * 
 */
WORD WINAPI UnzipGetDate(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  if (pfi) {
    WORD dt, tm;
    FileTimeToDosDateTime(&pfi->filetime, &dt, &tm);
    return dt;
  }
  return (WORD)-1;
}

/*
 *
 */
WORD WINAPI UnzipGetTime(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  if (pfi) {
    WORD dt, tm;
    FileTimeToDosDateTime(&pfi->filetime, &dt, &tm);
    return tm;
  }
  return (WORD)-1;
}

/*
 *
 */
DWORD WINAPI UnzipGetCRC(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  return pfi ? pfi->crc : (DWORD)-1;
}

/*
 *
 */
int WINAPI UnzipGetAttribute(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  return pfi ? (int)(pfi->attrib & 0xffff) : -1;
}

/**
 *
 */
DWORD WINAPI UnzipGetOriginalSize(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  return pfi ? (DWORD)pfi->originalSize : (DWORD)-1;
}

/*
 *
 */
BOOL WINAPI UnzipGetOriginalSize64(HANDLE harc, ULONGLONG* pull)
{
  auto pfi = CurrentFileItem(harc);
  if (pull && pfi) {
    *pull = pfi->originalSize;
    return TRUE;
  }
  return FALSE;
}

/*
 *
 */
DWORD WINAPI UnzipGetCompressedSize(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  return pfi ? (DWORD)pfi->compressedSize : (DWORD)-1;
}

/*
 *
 */
BOOL WINAPI UnzipGetCompressedSize64(HANDLE harc, ULONGLONG* pull)
{
  auto pfi = CurrentFileItem(harc);
  if (pull && pfi) {
    *pull = pfi->compressedSize;
    return TRUE;
  }
  return FALSE;
}

/*
 *
 */
WORD WINAPI UnzipGetRatio(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  if (pfi) {
    return (pfi->originalSize != 0 ) ? (WORD)muldiv64(pfi->compressedSize, 100, pfi->originalSize) : 0;
  }
  return (WORD)-1;
}

/*
 *
 */
static time_t FileTimeToUnixTime(FILETIME* pft)
{
  SYSTEMTIME st;
  struct tm t;

  FileTimeToSystemTime(pft, &st);
  t.tm_sec = (int)st.wSecond;
  t.tm_min = (int)st.wMinute;
  t.tm_hour = (int)st.wHour;
  t.tm_mday = (int)st.wDay;
  t.tm_mon = (int)st.wMonth - 1;			// tm_mon is 0 based
  t.tm_year = (int)st.wYear - 1900;		// tm_year is 1900 based
  t.tm_isdst = 0;
  return mktime(&t);
}

/*
 *
 */
DWORD WINAPI UnzipGetCreateTime(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  return pfi ? (DWORD)FileTimeToUnixTime(&pfi->filetime) : (DWORD)-1;
}

/*
 *
 */
DWORD WINAPI UnzipGetAccessTime(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  return pfi ? (DWORD)FileTimeToUnixTime(&pfi->filetime) : (DWORD)-1;
}

/*
 *
 */
DWORD WINAPI UnzipGetWriteTime(HANDLE harc)
{
  auto pfi = CurrentFileItem(harc);
  return pfi ? (DWORD)FileTimeToUnixTime(&pfi->filetime) : (DWORD)-1;
}

/*
 *
 */
BOOL WINAPI UnzipGetCreateTimeEx(HANDLE harc, FILETIME* pft)
{
  auto pfi = CurrentFileItem(harc);
  if (pfi) {
    *pft = pfi->filetime;
    return TRUE;
  }
  return FALSE;
}

/*
 *
 */
BOOL WINAPI UnzipGetAccessTimeEx(HANDLE harc, FILETIME* pft)
{
  auto pfi = CurrentFileItem(harc);
  if (pfi) {
    *pft = pfi->filetime;
    return TRUE;
  }
  return FALSE;
}

/*
 *
 */
BOOL WINAPI UnzipGetWriteTimeEx(HANDLE harc, FILETIME* pft)
{
  auto pfi = CurrentFileItem(harc);
  if (pfi) {
    *pft = pfi->filetime;
    return TRUE;
  }
  return FALSE;
}

/*
 *
 */
int WINAPI LuzReplace(void* pInfo, const char* filename)
{
  LPUSERFUNCTIONS pUserFn = (LPUSERFUNCTIONS)pInfo;
  LPDCL pDcl = pUserFn->pDcl;
  REPLACEINFO ri;
  ri.pszName = filename;
  ri.dwReplace = pDcl->nReplace;
  if (DlgModal(GetFocus(), (DLGPROC)ReplaceDlgFn, IDD_REPLACE, &ri) == IDOK) {
    pDcl->nReplace = ri.dwReplace;
    return pDcl->nReplace;
  }
  return 1;
}

// This is a very stripped down version of what is done in Wiz. Essentially
// what this function is for is to do a listing of an archive contents. It
// is actually never called in this example, but a dummy procedure had to
// be put in, so this was used.


/*
 *
 */
void WINAPI LuzMessage64(void* pInfo, const PACKED_FILEITEM* pItem)
{
  auto pUserFn = (LPUSERFUNCTIONS)pInfo;
  auto pDcl = pUserFn->pDcl;
  if (pDcl->fInsideVerb) {
    auto poa = (UzpArchive*)pDcl->pResult;
    auto pii = std::make_shared<UzpFileItem>(pItem->filename, pItem->originalSize, pItem->compressedSize, pItem->attributs, pItem->method);
    if (!pii) return;

    SystemTimeToFileTime(&pItem->fstime, &pii->filetime);
    FileTimeToDosDateTime(&pii->filetime, &pii->fdate, &pii->ftime);

    pii->crc = pItem->crc;
    pii->fCrypt = pItem->crypt;
    pii->fLowerCase = pDcl->C_flag ? 1 : 0;

    poa->filestack.push_back(pii);
    poa->totalSize += pii->originalSize;
    poa->compressedTotalSize += pii->compressedSize;
  }
  else {
    int per = pItem->originalSize ? (int)muldiv64(pItem->compressedSize, 100, pItem->originalSize) : 100;
    std::string info = std::format(
      "{:7} {:7} {:<8} {:3}% {:04}/{:02}/{:02} {:2}:{:02}:{:02} {:08x} {}{} {}\n",
      pItem->originalSize, pItem->compressedSize, pItem->method, per,
      pItem->fstime.wYear, pItem->fstime.wMonth, pItem->fstime.wDay, pItem->fstime.wHour, pItem->fstime.wMinute, pItem->fstime.wSecond,
      pItem->crc,
      pItem->crypt ? 'e' : '-',
      pDcl->C_flag ? 'c' : '-',
      pItem->filename);

    size_t len = info.size();
    if (pDcl->nResultLength > len) {
      lstrcpy(pDcl->pResult, info.c_str());
      pDcl->pResult += len;
      pDcl->nResultLength -= (int)len;
    }
  }
}
/*
 * Password entry routine - see password.c in the wiz directory for how
 * this is actually implemented in WiZ. If you have an encrypted file,
 * this will probably give you great pain.
 */
int WINAPI LuzPassword(void* pInfo, char *ppass, int n, const char *name)
{
  LPUSERFUNCTIONS pUserFn = (LPUSERFUNCTIONS)pInfo;
  LPDCL pDcl = pUserFn->pDcl;
  PASSENTER pe;
  pe.pszFile = name;
  pe.pszCode = ppass;

  if (DlgModal(GetFocus(), (DLGPROC)PasswordDlgFn, IDD_PASSWORD, &pe) != IDOK)
    return IZ_PW_CANCELALL;
  return IZ_PW_ENTERED;
}

static void __ZapMessage(HWND hdlg)
{
  MSG	msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (IsWindow(hdlg) && !IsDialogMessage(hdlg, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

/*
 * 
 */
int WINAPI LuzReport(void* pInfo, UINT type, LPCSTR pZip, LPCSTR pFile, void* data)
{
  LPUSERFUNCTIONS pUserFunc = (LPUSERFUNCTIONS)pInfo;
  LPDCL pDcl = pUserFunc->pDcl;
  if (IsWindow(pDcl->hwndProgress)) {
    HWND hctl;
    PROGRESSINFO* pmi = (PROGRESSINFO*)GetWindowLongPtr(pDcl->hwndProgress, DWLP_USER);
    if (pmi->bCanceled)
      return 1;

    hctl = GetDlgItem(pDcl->hwndProgress, IDC_METER);
    if (type == UZ_ST_START_EXTRACT) {
      char buf[MAX_PATH * 3];

      cdir_file_hdr* pcrec = (cdir_file_hdr*)data;
      lstrcpy(buf, pFile);
      ReplaceChar(buf, '/', '\\');
      PathSetDlgItemPath(pDcl->hwndProgress, IDC_FILE, buf);

      pDcl->ullMeterSize = pcrec->ucsize;
      if (!pDcl->ullMeterSize)
        pDcl->ullMeterSize = 1;
      pDcl->ullMeterDone = 0;
      SendMessage(hctl, PBM_SETPOS, 0, 0);
      UpdateWindow(hctl);
    } else if (type == UZ_ST_IN_PROGRESS) {
      int per;
      pDcl->ullMeterDone += (ULONG_PTR)data;
      per = (int)muldiv64(pDcl->ullMeterDone, 100, pDcl->ullMeterSize);
      SendMessage(hctl, PBM_SETPOS, per, 0);
      UpdateWindow(hctl);
    }

    __ZapMessage(pDcl->hwndProgress);
  }
  return 0;
}

/*
 * Dummy "print" routine that simply outputs what is sent from the dll
 */
int WINAPI LuzDisplay(void* pInfo, LPCSTR buf, ULONG size)
{
  LPUSERFUNCTIONS pUserFunc = (LPUSERFUNCTIONS)pInfo;
  LPDCL pDcl = pUserFunc->pDcl;
  if (pDcl->nResultLength > (int)size) {
    lstrcpy(pDcl->pResult, buf);
    pDcl->pResult += size;
    pDcl->nResultLength -= size;
  }
  return (int)size;
}


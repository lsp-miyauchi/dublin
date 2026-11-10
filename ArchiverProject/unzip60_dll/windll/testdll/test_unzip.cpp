#include "uzexampl.h"
#include "../../unzvers.h"

extern LONG64 muldiv64(LONG64 a, LONG64 b, LONG64 c);
extern void zipfileInfo(ULONGLONG ucsize, ULONGLONG csiz, const SYSTEMTIME* pst, const char* attr, LPCSTR filename, LPCSTR methbuf);
extern HINSTANCE hUnzipDll;

HANDLE (WINAPI* arc_open)(HWND hwnd, LPCSTR pszZip, DWORD dwMode);
int (WINAPI* arc_close)(HANDLE harc);
int (WINAPI* arc_findfirst)(HANDLE harc, LPCSTR pszFilter, void* pidi);
int (WINAPI* arc_findnext)(HANDLE harc, void* pidi);

static int errorHandler(int errCode, LPCSTR pszError) {
  std::cout << pszError << '\n';
  return errCode;
}

int unzipSession(int argc, char** argv)
{
  (FARPROC&)arc_open = GetProcAddress(hUnzipDll, "UnzipOpenArchive");
  if (arc_open == nullptr) {
    return errorHandler(-1, "Could not get UnzipOpenArchive function address.");
  }
  (FARPROC&)arc_close = GetProcAddress(hUnzipDll, "UnzipCloseArchive");
  if (arc_close == nullptr) {
    return errorHandler(-1, "Could not get UnzipCloseArchive function address.");
  }
  (FARPROC&)arc_findfirst = GetProcAddress(hUnzipDll, "UnzipFindFirst");
  if (arc_findfirst == nullptr) {
    return errorHandler(-1, "Could not get UnzipFindFirst function address.");
  }
  (FARPROC&)arc_findnext = GetProcAddress(hUnzipDll, "UnzipFindNext");
  if (arc_findnext == nullptr) {
    return errorHandler(-1, "Could not get UnzipFindNext function address.");
  }

  HANDLE harc = arc_open(nullptr, argv[1], 0);
  if (harc == nullptr) {
    return errorHandler(-1, "Could not open archive.");
  }

  FILETIME ft;
  SYSTEMTIME st;
  INDIVIDUALINFO idi = { 0 };
  int res = arc_findfirst(harc, nullptr, &idi);
  do {
    if (res != NOERROR) {
      arc_close(harc);
      std::string msg = std::format("FindFirst/Next failed in archive {} ({}}.", argv[1], res);
      return errorHandler(res, msg.c_str());
    }
    DosDateTimeToFileTime(idi.wDate, idi.wTime, &ft);
    FileTimeToSystemTime(&ft, &st);
    zipfileInfo(idi.dwOriginalSize, idi.dwCompressedSize, &st, idi.szAttribute, idi.szFileName, idi.szMode);
    res = arc_findnext(harc, &idi);
  } while (res == NOERROR);

  arc_close(harc);
  return 0;
}


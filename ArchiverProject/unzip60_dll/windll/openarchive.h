#pragma once

/////////////////////////////////////////////////////////////////////
// for OpenArchive

class UzpFileItem {
public:
  std::string fname;
  std::string method;
  ULONGLONG compressedSize;
  ULONGLONG originalSize;
  DWORD attrib;
  DWORD crc;
  FILETIME filetime;
  WORD fdate;
  WORD ftime;
  BOOL fCrypt;
  BOOL fLowerCase;

  explicit UzpFileItem(const char* filename, ULONGLONG compressed, ULONGLONG original, DWORD attr, LPCSTR meth)
    : fname(filename)
    , compressedSize(compressed)
    , originalSize(original)
    , attrib(attr)
    , method(meth)
    , crc(0)
    , filetime(0)
    , fdate(0)
    , ftime(0)
    , fCrypt(FALSE)
    , fLowerCase(FALSE)
  {}

  void attrib2string(char* pattr, size_t len) const
  {
    std::vector<char> att(8);
    att[0] = (attrib & FILE_ATTRIBUTE_DIRECTORY) ? 'd' : '-';
    att[1] = (attrib & FILE_ATTRIBUTE_READONLY) ? 'a' : '-';
    att[2] = (attrib & FILE_ATTRIBUTE_READONLY) ? 'r' : '-';
    att[3] = (attrib & FILE_ATTRIBUTE_HIDDEN) ? 'h' : '-';
    att[4] = (attrib & FILE_ATTRIBUTE_SYSTEM) ? 's' : '-';
    att[5] = (attrib & FILE_ATTRIBUTE_ENCRYPTED) ? 'E' : '-';
    att[6] = 0;
    (void)lstrcpyn(pattr, att.data(), (int)len - 1);
  }
};

#define OAMAGIC			0x9999

class UzpArchive {
public:
  unsigned magic;
  HWND hwndOwner;
  std::string zipfile;
  std::string filter;
  ULONGLONG totalSize;
  ULONGLONG compressedTotalSize;
  std::vector<std::shared_ptr<UzpFileItem>> filestack;
  int indexFineFile;
  std::shared_ptr<UzpFileItem> pCurFileItem;

  UzpArchive(HWND hwnd, const char* pZipFileName)
    : magic(OAMAGIC)
    , hwndOwner(hwnd)
    , zipfile(pZipFileName)
    , totalSize(0)
    , compressedTotalSize(0)
    , indexFineFile(0)
    , pCurFileItem(nullptr)
  {}

  ~UzpArchive() {
    clear();
  }

  void clear() {
    filestack.clear();
    totalSize = 0;
    compressedTotalSize = 0;
    indexFineFile = 0;
    pCurFileItem = nullptr;
  }
};



// for parse command line paramater
struct ParsedCommandLine {
  std::vector<std::string> storage;
  std::vector<char*> argv;
};

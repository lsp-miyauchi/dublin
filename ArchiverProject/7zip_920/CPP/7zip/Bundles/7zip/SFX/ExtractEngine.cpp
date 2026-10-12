// ExtractEngine.cpp

#include "StdAfx.h"

#include "Windows/FileDir.h"
#include "Windows/Thread.h"

#include "../../../UI/Common/OpenArchive.h"		// パス変更

#include "../../../UI/FileManager/FormatUtils.h"	// パス変更

#include "ExtractCallback.h"
#include "../../SFXSetup/ExtractEngine.h"

using namespace NWindows;
/* 削除
static LPCWSTR kCantFindArchive = L"Can not find archive file";
static LPCWSTR kCantOpenArchive = L"Can not open the file as archive";
  削除ここまで */
struct CThreadExtracting
{
  CCodecs *Codecs;
  UString FileName;
  UString DestFolder;

  CExtractCallbackImp *ExtractCallbackSpec;
  CMyComPtr<IArchiveExtractCallback> ExtractCallback;

  CArchiveLink ArchiveLink;
  HRESULT Result;
  UString ErrorMessage;

  void Process2()
  {
    NFile::NFind::CFileInfoW fi;
    if (!fi.Find(FileName))
    {
      ErrorMessage = NWindows::MyLoadStringW(IDS_CANT_FIND_ARCHIVE);	// 変更
      Result = E_FAIL;
      return;
    }
    
    Result = ArchiveLink.Open2(Codecs, CIntVector(), false, NULL, FileName, ExtractCallbackSpec);
    if (Result != S_OK)
    {
      if (Result != S_OK)
      ErrorMessage = NWindows::MyLoadStringW(IDS_CANT_OPNE_ARCHIVE);	// 変更
      return;
    }

    UString dirPath = DestFolder;
    NFile::NName::NormalizeDirPathPrefix(dirPath);
    
    if (!NFile::NDirectory::CreateComplexDirectory(dirPath))
    {
      ErrorMessage = MyFormatNew(IDS_CANNOT_CREATE_FOLDER,
        #ifdef LANG
        0x02000603,
        #endif
        dirPath);
      Result = E_FAIL;
      return;
    }

    ExtractCallbackSpec->Init(ArchiveLink.GetArchive(), dirPath, L"Default", fi.MTime, 0);

    Result = ArchiveLink.GetArchive()->Extract(0, (UInt32)-1 , BoolToInt(false), ExtractCallback);
  }

  void Process()
  {
    try
    {
      #ifndef _NO_PROGRESS
      CProgressCloser closer(ExtractCallbackSpec->ProgressDialog);
      #endif
      Process2();
    }
    catch(...) { Result = E_FAIL; }
  }
  
  static THREAD_FUNC_DECL MyThreadFunction(void *param)
  {
    ((CThreadExtracting *)param)->Process();
    return 0;
  }
};

HRESULT ExtractArchive(CCodecs *codecs,const UString &fileName, const UString &destFolder,
    bool showProgress, bool &isCorrupt, UString &errorMessage)
{
  isCorrupt = false;
  CThreadExtracting t;

  t.Codecs = codecs;
  t.FileName = fileName;
  t.DestFolder = destFolder;

  t.ExtractCallbackSpec = new CExtractCallbackImp;
  t.ExtractCallback = t.ExtractCallbackSpec;
  
  #ifndef _NO_PROGRESS

  if (showProgress)
  {
    t.ExtractCallbackSpec->ProgressDialog.IconID = IDI_STUB;
    NWindows::CThread thread;
    RINOK(thread.Create(CThreadExtracting::MyThreadFunction, &t));
    
    UString title;
    #ifdef LANG
    title = LangLoadString(IDS_PROGRESS_EXTRACTING, 0x02000890);
    #else
    title = NWindows::MyLoadStringW(IDS_PROGRESS_EXTRACTING);
    #endif
    t.ExtractCallbackSpec->StartProgressDialog(title, thread);
  }
  else

  #endif
  {
    t.Process2();
  }

  errorMessage = t.ErrorMessage;
  if (errorMessage.IsEmpty())
    errorMessage = t.ExtractCallbackSpec->_message;
  isCorrupt = t.ExtractCallbackSpec->_isCorrupt;
  return t.Result;
}

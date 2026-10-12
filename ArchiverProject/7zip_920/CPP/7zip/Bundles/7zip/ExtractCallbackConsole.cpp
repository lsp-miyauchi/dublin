// ExtractCallbackConsole.h

#include "StdAfx.h"

#include "Dialog.h"											// 追加
#include "resource.h"										// 追加

#include "../../UI/Console/ExtractCallbackConsole.h"		//	パス変更
//#include "UserInputUtils.h"								//	削除
#include "../../UI/Console/ConsoleClose.h"					//	パス変更

#include "Common/Wildcard.h"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/Time.h"
#include "Windows/Defs.h"
#include "Windows/PropVariant.h"
#include "Windows/Error.h"
#include "Windows/PropVariantConversions.h"

#include "../../Common/FilePathAutoRename.h"

#include "../../UI/Common/ExtractingFilePath.h"				//	パス変更

using namespace NWindows;
using namespace NFile;
using namespace NDirectory;

static const char *kTestString    =  "Testing     ";
static const char *kExtractString =  "Extracting  ";
static const char *kSkipString   =  "Skipping    ";

//static const char *kCantAutoRename = "can not create file with auto name\n";
//static const char *kCantRenameFile = "can not rename existing file\n";
//static const char *kCantDeleteOutputFile = "can not delete output file ";
static const char *kError = "ERROR: ";
static const char *kMemoryExceptionMessage = "Can't allocate required memory!";

static const char *kProcessing = "Processing archive: ";
static const char *kEverythingIsOk = "Everything is Ok";
static const char *kNoFiles = "No files to process";

static const char *kUnsupportedMethod = "Unsupported Method";
static const char *kCrcFailed = "CRC Failed";
static const char *kCrcFailedEncrypted = "CRC Failed in encrypted file. Wrong password?";
static const char *kDataError = "Data Error";
static const char *kDataErrorEncrypted = "Data Error in encrypted file. Wrong password?";
static const char *kUnknownError = "Unknown Error";

STDMETHODIMP CExtractCallbackConsole::SetTotal(UInt64 size)		// 変更
{
  if (g_StdOut.GetProgressDialog()) g_StdOut.GetProgressDialog()->SetTotal(size);					// 追加
  if (NConsoleClose::TestBreakSignal())
    return E_ABORT;
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::SetCompleted(const UInt64 *completeValue)		// 変更
{
  if (NConsoleClose::TestBreakSignal())
    return E_ABORT;

  if (completeValue && g_StdOut.GetProgressDialog())				// 追加
    g_StdOut.GetProgressDialog()->SetCompleted(*completeValue);		// 追加

  return S_OK;
}

/* 削除ここから
STDMETHODIMP CExtractCallbackConsole::AskOverwrite(
    const wchar_t *existName, const FILETIME *, const UInt64 *,
    const wchar_t *newName, const FILETIME *, const UInt64 *,
    Int32 *answer)
{
  (*OutStream) << "file " << existName <<
    "\nalready exists. Overwrite with " << endl;
  (*OutStream) << newName;
  
  NUserAnswerMode::EEnum overwriteAnswer = ScanUserYesNoAllQuit(OutStream);
  
  switch(overwriteAnswer)
  {
    case NUserAnswerMode::kQuit:  return E_ABORT;
    case NUserAnswerMode::kNo:     *answer = NOverwriteAnswer::kNo; break;
    case NUserAnswerMode::kNoAll:  *answer = NOverwriteAnswer::kNoToAll; break;
    case NUserAnswerMode::kYesAll: *answer = NOverwriteAnswer::kYesToAll; break;
    case NUserAnswerMode::kYes:    *answer = NOverwriteAnswer::kYes; break;
    case NUserAnswerMode::kAutoRenameAll: *answer = NOverwriteAnswer::kAutoRename; break;
    default: return E_FAIL;
  }
  return S_OK;
}
   削除ここまで */

/* 追加ここから */
STDMETHODIMP CExtractCallbackConsole::AskOverwrite(
    const wchar_t *existName, const FILETIME *existTime, const UInt64 *existSize,	// 変更
    const wchar_t *newName, const FILETIME *newTime, const UInt64 *newSize,			// 変更
    Int32 *answer)
{
  if (g_StdOut.GetProgressDialog() == NULL)
    return S_OK;
  CConfirmationDialog dlg;
  dlg.SetFileInfo(newName, *newSize, *newTime, existName, *existSize, *existTime);
  switch(dlg.DoModal())
  {
    case IDCANCEL:        return E_ABORT;
    case IDB_NO:          *answer = NOverwriteAnswer::kNo; break;
    case IDB_NO_ALL:      *answer = NOverwriteAnswer::kNoToAll; break;
    case IDB_YES_ALL:     *answer = NOverwriteAnswer::kYesToAll; break;
    case IDB_YES:         *answer = NOverwriteAnswer::kYes; break;
    case IDB_AUTO_RENAME: *answer = NOverwriteAnswer::kAutoRename; break;
    default: return E_FAIL;
  }
  return S_OK;
}
/* 追加ここまで */

STDMETHODIMP CExtractCallbackConsole::PrepareOperation(const wchar_t *name, bool /* isFolder */, Int32 askExtractMode, const UInt64 *position)
{
  switch (askExtractMode)
  {
    case NArchive::NExtract::NAskMode::kExtract: (*OutStream) << kExtractString; break;
    case NArchive::NExtract::NAskMode::kTest:    (*OutStream) << kTestString; break;
    case NArchive::NExtract::NAskMode::kSkip:    (*OutStream) << kSkipString; break;
  };
  (*OutStream) << name;
  if (position != 0)
    (*OutStream) << " <" << *position << ">";
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::MessageError(const wchar_t *message)
{
  (*OutStream) << message << endl;
  NumFileErrorsInCurrentArchive++;
  NumFileErrors++;
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::SetOperationResult(Int32 operationResult, bool encrypted)
{
  switch(operationResult)
  {
    case NArchive::NExtract::NOperationResult::kOK:
      break;
    default:
    {
      NumFileErrorsInCurrentArchive++;
      NumFileErrors++;
      (*OutStream) << "     ";
      switch(operationResult)
      {
        case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
          (*OutStream) << kUnsupportedMethod;
          break;
        case NArchive::NExtract::NOperationResult::kCRCError:
          (*OutStream) << (encrypted ? kCrcFailedEncrypted: kCrcFailed);
          break;
        case NArchive::NExtract::NOperationResult::kDataError:
          (*OutStream) << (encrypted ? kDataErrorEncrypted : kDataError);
          break;
        default:
          (*OutStream) << kUnknownError;
      }      
      /* 追加ここから */
      if (encrypted)
      {
        (*OutStream) << endl;
        return ERROR_PASSWORD_FILE;
      }
      /* 追加ここまで */
    }
  }
  (*OutStream) << endl;
  return S_OK;
}

#ifndef _NO_CRYPTO

HRESULT CExtractCallbackConsole::SetPassword(const UString &password)
{
  PasswordIsDefined = true;
  Password = password;
  return S_OK;
}

STDMETHODIMP CExtractCallbackConsole::CryptoGetTextPassword(BSTR *password)
{
  if (!PasswordIsDefined)
  {
    RINOK(GetPassword(Password, IDS_DECRYPT));		// 変更
    PasswordIsDefined = true;
  }
  return StringToBstr(Password, password);
}

#endif

HRESULT CExtractCallbackConsole::BeforeOpen(const wchar_t *name)
{
  NumArchives++;
  NumFileErrorsInCurrentArchive = 0;
  (*OutStream) << endl << kProcessing << name << endl;
  if (g_StdOut.GetProgressDialog()) g_StdOut.GetProgressDialog()->SetArchiveFile(name);	// 追加
  return S_OK;
}

HRESULT CExtractCallbackConsole::OpenResult(const wchar_t * /* name */, HRESULT result, bool encrypted)
{
  (*OutStream) << endl;
  if (result != S_OK)
  {
    if (result == S_FALSE)
    {
      (*OutStream) << (encrypted ?
        "Can not open encrypted archive. Wrong password?" :
        "Can not open file as archive");
      if (encrypted)						// 追加
        return ERROR_PASSWORD_FILE;			// 追加
    }
    else
    {
      if (result == E_OUTOFMEMORY)
        (*OutStream) << "Can't allocate required memory";
      else
        (*OutStream) << NError::MyFormatMessage(result);
    }
    (*OutStream) << endl;
    NumArchiveErrors++;
  }
  return S_OK;
}
  
HRESULT CExtractCallbackConsole::ThereAreNoFiles()
{
  (*OutStream) << endl << kNoFiles << endl;
  return S_OK;
}

HRESULT CExtractCallbackConsole::ExtractResult(HRESULT result)
{
  /* 追加ここから */
  CProgressDialog* pDlg = g_StdOut.GetProgressDialog();
  if (pDlg)
  {
    pDlg->SetCompleted(pDlg->GetTotalSize());
    pDlg->SendExtractingInfo(ARCEXTRACT_END);
  }
  /* 追加ここまで */
  if (result == S_OK)
  {
    (*OutStream) << endl;
    if (NumFileErrorsInCurrentArchive == 0)
      (*OutStream) << kEverythingIsOk << endl;
    else
    {
      NumArchiveErrors++;
      (*OutStream) << "Sub items Errors: " << NumFileErrorsInCurrentArchive << endl;
    }
  }
  if (result == S_OK)
    return result;
//  NumArchiveErrors++;										// 削除
//  if (result == E_ABORT || result == ERROR_DISK_FULL)		// 削除
//    return result;										// 削除
  /* 追加ここから */
  if (result == E_ABORT || result == ERROR_PASSWORD_FILE)
  {
    NumArchiveErrors = 0;
	NumFileErrors = 0;
    return result;
  }
  NumArchiveErrors++;
  if (result == ERROR_DISK_FULL)
	  return ERROR_DISK_FULL;
  /* 追加ここまで */
  (*OutStream) << endl << kError;
  if (result == E_OUTOFMEMORY)
    (*OutStream) << kMemoryExceptionMessage;
  else
  {
    UString message;
    NError::MyFormatMessage(result, message);
    (*OutStream) << message;
  }
  (*OutStream) << endl;

  return S_OK;
}

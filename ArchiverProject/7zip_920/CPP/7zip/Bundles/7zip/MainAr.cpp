// MainAr.cpp

#include "StdAfx.h"
#include "MainAr.h"									// 追加

#include "Windows/COM.h"
#include "Windows/FileDir.h"

#include "Common/MyException.h"
#include "Common/StdOutStream.h"

#include "Windows/Error.h"
#include "Windows/NtCheck.h"

#include "../../UI//Common/ArchiveCommandLine.h"	// パス変更
#include "../../UI//Common/ExitCode.h"				// パス変更

#include "../../UI/Console/ConsoleClose.h"			// パス変更

using namespace NWindows;

CStdOutStream *g_StdStream = 0;

extern int Main2(
  #ifndef _WIN32
  int numArgs, const char *args[]
  #endif
);

static const char *kExceptionErrorMessage = "\n\nError:\n";
static const char *kUserBreak  = "\nBreak signaled\n";
static const char *kMemoryExceptionMessage = "\n\nERROR: Can't allocate required memory!\n";
static const char *kUnknownExceptionMessage = "\n\nUnknown Error\n";
static const char *kInternalExceptionMessage = "\n\nInternal Error #";

#define NT_CHECK_FAIL_ACTION (*g_StdStream) << "Unsupported Windows version"; return NExitCode::kFatalError;

/* 追加ここから */
static const int kErrorCode[23][2] = { NExitCode::kSuccess, 0,
									   NExitCode::kWarning, ERROR_WARNING,
									   NExitCode::kFatalError, ERROR_FATAL,
//									   NExitCode::kCRCError, ERROR_FILE_CRC,
//									   NExitCode::kLockedArchive, ERROR_READ_ONLY,
//									   NExitCode::kWriteError, ERROR_CANNOT_WRITE,
//									   NExitCode::kOpenError, ERROR_FILE_OPEN,
									   NExitCode::kUserError, ERROR_COMMAND_NAME,
									   NExitCode::kMemoryError, ERROR_ENOUGH_MEMORY,
									   
//									   NExitCode::kNotSupported, ERROR_NOT_ARC_FILE,
//									   NExitCode::kFileError, ERROR_NOT_ARC_FILE,
									   
//									   NExitCode::kVerError, ERROR_VERSION,
//									   NExitCode::kMethodError, ERROR_METHOD,
									   
//									   NExitCode::kUserQuit, ERROR_METHOD, 
									   
//									   NExitCode::kFileIsNotArchive, ERROR_NOT_ARC_FILE,
									   
//									   NExitCode::kCommonError, ERROR_FATAL,
									   
//									   NExitCode::kInputArchiveException, ERROR_NOT_FIND_ARC_FILE,
									   
//									   NExitCode::kErrorsDuringDecompression, ERROR_DURING_DECOMPRESSION,
									   
//									   NExitCode::kDirFileWith64BitSize, ERROR_DIR_FILE_WITH_64BIT_SIZE,
//									   NExitCode::kFileTimeWinToDosConvertError, ERROR_CONVERT_TIME,
									   
//									   NExitCode::kFileChangedDuringOperation, ERROR_FILE_CHANGED_DURING_OPERATION,
									   
									   NExitCode::kUserBreak, ERROR_USER_CANCEL,
									   -1, -1 };


static int ExitCodeToErrorCode(int nExitCode)
{
	if (nExitCode >= 0x8000)
		return nExitCode;
	int nErrorCode = ERROR_FATAL;
	for (int i = 0; kErrorCode[i][0] != -1; ++i)
	{
		if (kErrorCode[i][0] == nExitCode)
		{
			nErrorCode = kErrorCode[i][1];
			break;
		}
	}
	return nErrorCode;
}

UINT _stdcall Main(LPVOID lpParameter)
{
	// コマンドラインを分割してメイン処理へ渡す
	BOOL bCodePage = ::AreFileApisANSI();
	int result = SendCommand7zip();
	g_StdOut.CloseProgressDialog();
	if (bCodePage)
		SetFileApisToANSI();

	return ExitCodeToErrorCode(result);
}
/* 追加ここまで */

/* 削除ここから
int MY_CDECL main
(
  #ifndef _WIN32
  int numArgs, const char *args[]
  #endif
)
   削除ここまで */
int SendCommand7zip()	// 追加
{
  g_StdStream = &g_StdOut;

  NT_CHECK

  NConsoleClose::CCtrlHandlerSetter ctrlHandlerSetter;
  int res = 0;
  try
  {
    res = Main2(
    #ifndef _WIN32
    numArgs, args
    #endif
    );
  }
  catch(const CNewException &)
  {
    (*g_StdStream) << kMemoryExceptionMessage;
    return (NExitCode::kMemoryError);
  }
  catch(const NConsoleClose::CCtrlBreakException &)
  {
    (*g_StdStream) << endl << kUserBreak;
    return (NExitCode::kUserBreak);
  }
  catch(const CArchiveCommandLineException &e)
  {
    (*g_StdStream) << kExceptionErrorMessage << e << endl;
    return (NExitCode::kUserError);
  }
  catch(const CSystemException &systemError)
  {
    if (systemError.ErrorCode == E_OUTOFMEMORY)
    {
      (*g_StdStream) << kMemoryExceptionMessage;
      return (NExitCode::kMemoryError);
    }
    if (systemError.ErrorCode == E_ABORT)
    {
      (*g_StdStream) << endl << kUserBreak;
      return (NExitCode::kUserBreak);
    }
	/* 追加ここから */
    if (systemError.ErrorCode == ERROR_PASSWORD_FILE)
    {
      (*g_StdStream) << endl << "Password Error\n";
      return (ERROR_PASSWORD_FILE);
    }
	/* 追加ここまで */
    UString message;
    NError::MyFormatMessage(systemError.ErrorCode, message);
    (*g_StdStream) << endl << endl << "System error:" << endl << message << endl;
    return (NExitCode::kFatalError);
  }
  catch(NExitCode::EEnum &exitCode)
  {
    (*g_StdStream) << kInternalExceptionMessage << exitCode << endl;
    return (exitCode);
  }
  /*
  catch(const NExitCode::CMultipleErrors &multipleErrors)
  {
    (*g_StdStream) << endl << multipleErrors.NumErrors << " errors" << endl;
    return (NExitCode::kFatalError);
  }
  */
  catch(const UString &s)
  {
    (*g_StdStream) << kExceptionErrorMessage << s << endl;
    return (NExitCode::kFatalError);
  }
  catch(const AString &s)
  {
    (*g_StdStream) << kExceptionErrorMessage << s << endl;
    return (NExitCode::kFatalError);
  }
  catch(const char *s)
  {
    (*g_StdStream) << kExceptionErrorMessage << s << endl;
    return (NExitCode::kFatalError);
  }
  catch(int t)
  {
    (*g_StdStream) << kInternalExceptionMessage << t << endl;
    return (NExitCode::kFatalError);
  }
  catch(...)
  {
    (*g_StdStream) << kUnknownExceptionMessage;
    return (NExitCode::kFatalError);
  }
  return  res;
}

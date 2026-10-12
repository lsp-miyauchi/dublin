// UpdateCallback7z.h

#ifndef __UPDATE_CALLBACK_CONSOLE_H
#define __UPDATE_CALLBACK_CONSOLE_H

#include "Common/StdOutStream.h"

#include "../Common/Update.h"

#include "PercentPrinter.h"

class CUpdateCallback7z: public IUpdateCallbackUI2
{
  CPercentPrinter m_PercentPrinter;
  bool m_NeedBeClosed;
  bool m_NeedNewLine;

  bool m_WarningsMode;

  CStdOutStream *OutStream;
public:
  bool EnablePercents;
  bool StdOutMode;

  bool PasswordIsDefined;
  UString Password;
  bool AskPassword;

  CUpdateCallback7z():
      m_PercentPrinter(1 << 16),
      PasswordIsDefined(false),
      AskPassword(false),
      StdOutMode(false),
      EnablePercents(true),
      m_WarningsMode(false)
      {}
  
  ~CUpdateCallback7z() { Finilize(); }
  void Init(CStdOutStream *outStream)
  {
    m_NeedBeClosed = false;
    m_NeedNewLine = false;
    FailedFiles.Clear();
    FailedCodes.Clear();
    OutStream = outStream;
    m_PercentPrinter.OutStream = outStream;
  }

  INTERFACE_IUpdateCallbackUI2(;)

  UStringVector FailedFiles;
  CRecordVector<HRESULT> FailedCodes;

  UStringVector CantFindFiles;
  CRecordVector<HRESULT> CantFindCodes;
};

#endif

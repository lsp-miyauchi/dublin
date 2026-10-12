// OpenCallback7z.h

#ifndef __OPENCALLBACKCONSOLE_H
#define __OPENCALLBACKCONSOLE_H

#include "Common/StdOutStream.h"
#include "../Common/ArchiveOpenCallback.h"

class COpenCallback7z: public IOpenCallbackUI
{
public:
  INTERFACE_IOpenCallbackUI(;)
  
  CStdOutStream *OutStream;

  bool PasswordIsDefined;
  bool PasswordWasAsked;
  UString Password;
  COpenCallback7z(): PasswordIsDefined(false), PasswordWasAsked(false) {}
};

#endif

// SevenZCloseUtils.h

#ifndef __CONSOLECLOSEUTILS_H
#define __CONSOLECLOSEUTILS_H

namespace N7zClose {

bool TestBreakSignal();

class CCtrlHandlerSetter
{
public:
  CCtrlHandlerSetter();
  virtual ~CCtrlHandlerSetter();
};

class CCtrlBreakException
{};

void CheckCtrlBreak();

}

#endif

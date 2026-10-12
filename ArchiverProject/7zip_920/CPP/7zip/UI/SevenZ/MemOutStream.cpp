// MemOutStream.cpp

#include "StdAfx.h"

#include <tchar.h>

#include "IntToString.h"
#include "StdOutStream.h"
#include "StringConvert.h"
#include "UTFConvert.h"

#ifdef _MSC_VER
// "was declared deprecated" disabling
#pragma warning(disable : 4996 )
#endif

static const char kNewLineChar =  '\n';

extern int g_CodePage;

CMemOutStream g_MemOut;

CMemOutStream& CMemOutStream::operator<<(CMemOutStream& (*aFunction)(CMemOutStream&))
{
	(*aFunction)(*this);
	return *this;
}

CMemOutStream& CMemOutStream::endl(CMemOutStream& outStream)
{
	return outStream << kNewLineChar;
}

CMemOutStream& CMemOutStream::operator<<(const char *s)
{
	write((void*)s, strlen(s));
	return *this;
}

CMemOutStream& CMemOutStream::operator<<(const wchar_t* s)
{
	int codePage = g_CodePage;
	if (codePage == -1)
		codePage = CP_OEMCP;
	AString dest;
	if (codePage == CP_UTF8)
		ConvertUnicodeToUTF8(s, dest);
	else
		dest = UnicodeStringToMultiByte(s, (UINT)codePage);
	*this << (const char *)dest;
	return *this;
}

CMemOutStream& CMemOutStream::operator<<(char c)
{
	putc((BYTE)c);
	return *this;
}

CMemOutStream& CMemOutStream::operator<<(int number)
{
	char textString[32];
	ConvertInt64ToString(number, textString);
	return operator<<(textString);
}

CMemOutStream& CMemOutStream::operator<<(UInt64 number)
{
	char textString[32];
	ConvertUInt64ToString(number, textString);
	return operator<<(textString);
}

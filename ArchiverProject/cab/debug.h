//////////////////////////////////////////////////////////////////////
// debug.h

#pragma once
#ifndef __DEBUG_CONSOLE_H__
#define __DEBUG_CONSOLE_H__

#ifdef _DEBUG

class CConsole
{
public:
	CConsole();
	~CConsole();

	void Init();

	void putchar(TCHAR ch);
	void putstr(LPCTSTR pstr, int cch = -1);
	void puts(LPCTSTR pstr);
	void __cdecl printf(LPCTSTR lpszFormat, ...);
	void dump(BYTE* data, int len);

	BOOL m_bInit;
	HANDLE m_stdin;
	HANDLE m_stdout;
	HANDLE m_stderr;
};

extern CConsole dbg;

#endif		// _DEBUG

#endif		// __DEBUG_CONSOLE_H__

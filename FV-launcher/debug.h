//////////////////////////////////////////////////////////////////////
// debug.h

#pragma once
#ifndef __DEBUG_CONSOLE_H__
#define __DEBUG_CONSOLE_H__

#ifndef _COUNTOF
#define _COUNTOF(a)		(sizeof(a)/sizeof(a[0]))
#endif

class CConsole
{
public:
	CConsole() :
		m_bInit(FALSE),
		m_stdin(NULL),
		m_stdout(NULL),
		m_stderr(NULL)
	{
	}

	~CConsole()
	{
		if (m_bInit)
			FreeConsole();
	}

	void Init()
	{
		if (!m_bInit) {
			m_bInit = TRUE;
			AllocConsole();
			m_stdin = GetStdHandle(STD_INPUT_HANDLE);
			m_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
			m_stderr = GetStdHandle(STD_ERROR_HANDLE);
		}
	}

	void putchar(TCHAR ch)
	{
		DWORD dw;
		WriteConsole(m_stdout, &ch, 1, &dw, NULL);
	}

	void putcharA(char ch)
	{
		DWORD dw;
		WriteConsoleA(m_stdout, &ch, 1, &dw, NULL);
	}

	void putstr(LPCTSTR pstr, int cch = -1)
	{
		if (!m_bInit) Init();

		if (cch < 0)
			cch = lstrlen(pstr);
		DWORD dw;
		WriteConsole(m_stdout, pstr, cch, &dw, NULL);
	}

	void putstrA(LPCSTR pstr, int cch = -1)
	{
		if (!m_bInit) Init();

		if (cch < 0)
			cch = lstrlenA(pstr);
		DWORD dw;
		WriteConsoleA(m_stdout, pstr, cch, &dw, NULL);
	}

	void puts(LPCTSTR pstr)
	{
		putstr(pstr, -1);
		putchar(_T('\n'));
	}

	void putsA(LPCSTR pstr)
	{
		putstrA(pstr, -1);
		putcharA('\n');
	}

	void __cdecl printf(LPCTSTR lpszFormat, ...)
	{
		TCHAR* pOutput = new TCHAR[1024];
		va_list argList;
		va_start(argList, lpszFormat);
		int len = wvsprintf(pOutput, lpszFormat, argList);
		va_end(argList);

		putstr(pOutput, len);
		delete [] pOutput;
	}

	void __cdecl printfA(LPCSTR lpszFormat, ...)
	{
		char* pOutput = new char[1024];
		va_list argList;
		va_start(argList, lpszFormat);
		int len = wvsprintfA(pOutput, lpszFormat, argList);
		va_end(argList);

		putstrA(pOutput, len);
		delete [] pOutput;
	}

	void dump(BYTE* data, int len)
	{
		TCHAR s[64], t[64];

		s[0] = 0;
		t[0] = 0;
		int slen = 0;
		int addr = 0;
		for (int i = 0; i < len; i++) {
			if (addr == 0) {
				slen = 0;
				printf(_T("%04X: "), i);
			}

			slen += wsprintf(&s[slen], _T("%02X "), *data);
			t[addr++] = isprint((int)(*data)) ? *data : _T('.');
			t[addr] = 0;

			if (addr > 15) {
				putstr(s);
				putchar(_T(':'));
				puts(t);
				s[0] = 0;
				t[0] = 0;
				addr = 0;
			}
			data++;
		}

		if (s[0]) {
			putstr(s);
			while (addr < 16) {
				putstr(_T("   "));
				addr++;
			}
			putchar(_T(':'));
			puts(t);
		}
		putchar(_T('\n'));
	}

	BOOL m_bInit;
	HANDLE m_stdin;
	HANDLE m_stdout;
	HANDLE m_stderr;
};

extern CConsole dbg;

#endif			// __DEBUG_CONSOLE_H__

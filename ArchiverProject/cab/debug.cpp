#include "stdafx.h"

#ifdef _DEBUG

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))
#endif

CConsole::CConsole()
{
	m_bInit = FALSE;
}

CConsole::~CConsole()
{
	if (m_bInit)
		FreeConsole();
}

void CConsole::Init()
{
	if (!m_bInit) {
		m_bInit = TRUE;
		AllocConsole();
		m_stdin = GetStdHandle(STD_INPUT_HANDLE);
		m_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
		m_stderr = GetStdHandle(STD_ERROR_HANDLE);
	}
}

void CConsole::putchar(TCHAR ch)
{
	DWORD dw;
	WriteConsole(m_stdout, &ch, 1, &dw, NULL);
}

void CConsole::putstr(LPCTSTR pszOutput, int cch)
{
	if (!m_bInit) Init();

	if (cch < 0)
		cch = lstrlen(pszOutput);
	DWORD dw;
	WriteConsole(m_stdout, pszOutput, cch, &dw, NULL);
}

void CConsole::puts(LPCTSTR pszOutput)
{
	putstr(pszOutput, -1);
	putchar(_T('\n'));
}

void __cdecl CConsole::printf(LPCTSTR lpszFormat, ...)
{
	TCHAR* pOutput = new TCHAR[1024];
	va_list argList;
	va_start(argList, lpszFormat);
	int len = wvsprintf(pOutput, lpszFormat, argList);
	va_end(argList);

	putstr(pOutput, len);
	delete [] pOutput;
}

void CConsole::dump(BYTE* data, int len)
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

CConsole dbg;

#endif		// _DEBUG

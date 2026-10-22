#include "stdafx.h"
#include <vector>

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

void CConsole::putcharA(char ch)
{
	DWORD dw;
	WriteConsoleA(m_stdout, &ch, 1, &dw, NULL);
}

void CConsole::putstr(LPCTSTR pszOutput, int cch)
{
	if (!m_bInit) Init();

	if (cch < 0)
		cch = static_cast<int>(_tcslen(pszOutput));
	DWORD dw;
	WriteConsole(m_stdout, pszOutput, cch, &dw, NULL);
}

void CConsole::putstrA(LPCSTR pszOutput, int cch)
{
	if (!m_bInit) Init();

	if (cch < 0)
		cch = static_cast<int>(_mbstrlen(pszOutput));
	DWORD dw;
	WriteConsoleA(m_stdout, pszOutput, cch, &dw, NULL);
}

void CConsole::puts(LPCTSTR pszOutput)
{
	putstr(pszOutput, -1);
	putchar(_T('\n'));
}

void CConsole::putsA(LPCSTR pszOutput)
{
	putstrA(pszOutput, -1);
	putcharA('\n');
}

void __cdecl CConsole::printf(LPCTSTR lpszFormat, ...)
{
	std::array<TCHAR, 1024> buf;
	va_list argList;
	va_start(argList, lpszFormat);
	int len = std::vswprintf(buf.data(), buf.size(), lpszFormat, argList);
	va_end(argList);
	putstr(buf.data(), len);
}

void __cdecl CConsole::msgboxf(HWND hwnd, LPCTSTR lpszFormat, ...)
{
	std::array<TCHAR, 1024> buf;
	va_list argList;
	va_start(argList, lpszFormat);
	std::vswprintf(buf.data(), buf.size(), lpszFormat, argList);
	va_end(argList);
	::MessageBox(hwnd, buf.data(), _T("debug"), MB_OK);
}

void __cdecl CConsole::printfA(LPCSTR lpszFormat, ...)
{
	std::array<char, 1024> buf;
	va_list argList;
	va_start(argList, lpszFormat);
	int len = std::vsnprintf(buf.data(), buf.size(), lpszFormat, argList);
	va_end(argList);

	putstrA(buf.data(), len);
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

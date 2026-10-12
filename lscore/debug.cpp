#include "stdafx.h"
#include <stdio.h>

CConsole dbg;

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
	if (!m_bInit)
	{
		m_bInit = TRUE;
		AllocConsole();
		m_stdin = GetStdHandle(STD_INPUT_HANDLE);
		m_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
		m_stderr = GetStdHandle(STD_ERROR_HANDLE);
	}
}

void CConsole::puts(LPCTSTR pszOutput)
{
	if (!m_bInit)
		Init();

	DWORD dw;
	WriteConsole(m_stdout, pszOutput, lstrlen(pszOutput), &dw, NULL);
	WriteConsole(m_stdout, _T("\n"), 1, &dw, NULL);
}

void __cdecl CConsole::printf(LPCTSTR lpszFormat, ...)
{
	if (!m_bInit)
		Init();

	TCHAR* pOutput = new TCHAR[1024];
	va_list argList;
	va_start(argList, lpszFormat);
	int len = _vstprintf_s(pOutput, 1024, lpszFormat, argList);
	va_end(argList);

	DWORD dw;
	WriteConsole(m_stdout, pOutput, len, &dw, NULL);
	delete [] pOutput;
}

#define ISPRINTABLE(a)		(((a)>=0x20&&(a)<=0x7e)||((a)>=0xa1&&(a)<=0xdf))

void CConsole::dump(BYTE* pDump, int len)
{
	TCHAR s[64], t[64], w[64];

	s[0] = '\0';
	t[0] = '\0';

	for (int i = 0; i < len; i++) {
		if (!(i & 0xf)) {
			int j;
			for (j = 0; j < sizeof(s) / sizeof(TCHAR); j++) s[j] = ' ';
			for (j = 0; j < sizeof(t) / sizeof(TCHAR); j++) t[j] = ' ';
			s[48] = '\0';
			t[16] = '\0';
			printf(_T("%04X: "), i);
		}

		wsprintf(w, _T("%02X "), *pDump);
		memcpy(&s[(i & 0xf) * 3], w, 3);
		t[i & 0xf] = ISPRINTABLE((int)(*pDump)) ? *pDump : '.';
		pDump++;

		if ((i & 0xf) == 0xf) {
			printf(s);
			printf(t);
			printf(_T("\n"));
			s[0] = '\0';
			t[0] = '\0';
		}
	}

	printf(s);
	printf(t);
	printf(_T("\n"));
}

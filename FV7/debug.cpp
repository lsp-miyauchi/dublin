#include "stdafx.h"

// #define LOG_FILEREC

#define MAX_STRING_BUFFER	32768

#ifdef _DEBUG
	#ifdef LOG_FILEREC
		#undef LOG_FILEREC
	#endif
#endif

#ifdef LOG_FILEREC
	static HANDLE shfTrace = INVALID_HANDLE_VALUE;
	static TCHAR sszTraceFile[] = _T("trace.log");
#endif

CConsole::CConsole()
{
	m_bInit = FALSE;
}

CConsole::~CConsole()
{
	if (m_bInit) {
#ifdef LOG_FILEREC
		if (shfTrace != INVALID_HANDLE_VALUE) {
			FlushFileBuffers(shfTrace);
			CloseHandle(shfTrace);
		}
#else
		FreeConsole();
#endif
	}
}

void CConsole::Init()
{
	if (!m_bInit) {
		m_bInit = TRUE;
#ifdef LOG_FILEREC
		TCHAR buf[MAX_PATH * 2];
		GetModuleFileName(NULL, buf, ARRAYSIZE(buf));
		PathRemoveFileSpec(buf);
		PathAppend(buf, sszTraceFile);
		shfTrace = CreateFile(buf, GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
								FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (shfTrace != INVALID_HANDLE_VALUE) {
			DWORD pos = SetFilePointer(shfTrace, 0, NULL, FILE_END);
			if (pos == 0) {
				BYTE cb[2];
				cb[0] = 0xff;
				cb[1] = 0xfe;
				WriteFile(shfTrace, &cb, 2, &pos, NULL);
			}
			SYSTEMTIME st;
			GetLocalTime(&st);
			printf(_T("[start trace log at %02d:%02d:%02d]\n"), st.wHour, st.wMinute, st.wSecond);
		}
#else
		AllocConsole();
		m_stdin = GetStdHandle(STD_INPUT_HANDLE);
		m_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
		m_stderr = GetStdHandle(STD_ERROR_HANDLE);
#endif
	}
}

LPCTSTR CConsole::errortext(DWORD code, LPTSTR pszText, int cch)
{
	*pszText = 0;
	CMemLocal lm;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lm, 0, NULL);
	StrCpyN(pszText, lm, cch);
	return pszText;
}

void CConsole::puts(LPCTSTR pszOutput)
{
	if (!m_bInit) Init();

	DWORD dw;
#ifdef LOG_FILEREC
	if (shfTrace != INVALID_HANDLE_VALUE) {
		WriteFile(shfTrace, pszOutput, lstrlen(pszOutput) * sizeof(TCHAR), &dw, NULL);
		WriteFile(shfTrace, _T("\n"), 1 * sizeof(TCHAR), &dw, NULL);
		FlushFileBuffers(shfTrace);
	}
#else
	WriteConsole(m_stdout, pszOutput, lstrlen(pszOutput), &dw, NULL);
	WriteConsole(m_stdout, _T("\n"), 1, &dw, NULL);
#endif
}

void __cdecl CConsole::printf(LPCTSTR lpszFormat, ...)
{
	if (!m_bInit) Init();

	TCHAR* pOutput = new TCHAR[MAX_STRING_BUFFER];
	va_list argList;
	va_start(argList, lpszFormat);
	int len;
	len = _vstprintf_s(pOutput, MAX_STRING_BUFFER, lpszFormat, argList);
	va_end(argList);

	DWORD dw;
#ifdef LOG_FILEREC
	if (shfTrace != INVALID_HANDLE_VALUE) {
		WriteFile(shfTrace, pOutput, len * sizeof(TCHAR), &dw, NULL);
		FlushFileBuffers(shfTrace);
	}
#else
	WriteConsole(m_stdout, pOutput, len, &dw, NULL);
#endif
	delete [] pOutput;
}

void __cdecl CConsole::printfA(LPCSTR lpszFormat, ...)
{
	if (!m_bInit) Init();

	char* pOutput = new char[MAX_STRING_BUFFER];
	va_list argList;
	va_start(argList, lpszFormat);
	int len = wvsprintfA(pOutput, lpszFormat, argList);
	va_end(argList);

	DWORD dw;
	WriteConsoleA(m_stdout, pOutput, len, &dw, NULL);
	delete [] pOutput;
}

void __cdecl CConsole::trace2(DWORD_PTR dwCategory, UINT nLevel, LPCSTR pszFormat, ...)
{
	if (!m_bInit) Init();

	char* psz = new char[MAX_STRING_BUFFER];
	TCHAR* pOutput = new TCHAR[MAX_STRING_BUFFER];
	va_list argList;
	va_start(argList, pszFormat);
	int len = vsprintf_s(psz, MAX_STRING_BUFFER, pszFormat, argList);
	va_end(argList);

	len = MultiByteToWideChar(CP_ACP, 0, psz, -1, pOutput, MAX_STRING_BUFFER - 1);

	DWORD dw;
#ifdef LOG_FILEREC
	if (shfTrace != INVALID_HANDLE_VALUE) {
		WriteFile(shfTrace, pOutput, len * sizeof(TCHAR), &dw, NULL);
		FlushFileBuffers(shfTrace);
	}
#else
	WriteConsole(m_stdout, pOutput, len, &dw, NULL);
#endif
	delete [] psz;
	delete [] pOutput;
}

void __cdecl CConsole::trace2(DWORD_PTR dwCategory, UINT nLevel, LPCTSTR pszFormat, ...)
{
	if (!m_bInit) Init();

	TCHAR* pOutput = new TCHAR[MAX_STRING_BUFFER];
	va_list argList;
	va_start(argList, pszFormat);
	int len = _vstprintf_s(pOutput, MAX_STRING_BUFFER, pszFormat, argList);
	va_end(argList);

	DWORD dw;
#ifdef LOG_FILEREC
	if (shfTrace != INVALID_HANDLE_VALUE) {
		WriteFile(shfTrace, pOutput, len * sizeof(TCHAR), &dw, NULL);
		FlushFileBuffers(shfTrace);
	}
#else
	WriteConsole(m_stdout, pOutput, len, &dw, NULL);
#endif
	delete [] pOutput;
}

#define ISPRINTABLE(a)		(((a)>=0x20&&(a)<=0x7e)||((a)>=0xa1&&(a)<=0xdf))

static void tchset(LPTSTR psz, TCHAR ch, size_t size)
{
	for (size_t i = 0; i < size; i++)
		*psz++ = ch;
}

void CConsole::dump(BYTE* pDump, int len)
{
	TCHAR s[64], t[64];

	s[0] = '\0';
	t[0] = '\0';
	int slen = 0;
	for (int i = 0; i < len; i++) {
		if (!(i & 0xf)) {
			slen = 0;
			printf(_T("%04X: "), i);
		}

		slen += wsprintf(&s[slen], _T("%02X "), *pDump);
		t[(i & 0xf)] = isprint((int)(*pDump)) ? *pDump : _T('.');
		t[(i & 0xf) + 1] = 0;
		pDump++;

		if ((i & 0xf) == 0xf) {
			printf(s);
			printf(t);
			printf(_T("\n"));
			s[0] = 0;
			t[0] = 0;
		}
	}

	if (s[0]) {
		printf(s);
		printf(t);
	}
	printf(_T("\n"));
}

CConsole dbg;

//////////////////////////////////////////////////////////////////////
// debug.h

class CConsole
{
public:
	CConsole();
	~CConsole();

	void Init();

	void putchar(TCHAR ch);
	void putcharA(char ch);
	void putstr(LPCTSTR pstr, int cch = -1);
	void putstrA(LPCSTR pstr, int cch = -1);
	void puts(LPCTSTR pstr);
	void putsA(LPCSTR pstr);
	void __cdecl printf(LPCTSTR lpszFormat, ...);
	void __cdecl printfA(LPCSTR lpszFormat, ...);
	void dump(BYTE* data, int len);
	void msgboxf(HWND hwnd, LPCTSTR lpszFormat, ...);

	BOOL m_bInit;
	HANDLE m_stdin;
	HANDLE m_stdout;
	HANDLE m_stderr;
};

extern CConsole dbg;

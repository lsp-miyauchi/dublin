//////////////////////////////////////////////////////////////////////
// debug.h

class CConsole
{
public:
	CConsole();
	~CConsole();

	void Init();

	void puts(LPCSTR);
	void __cdecl printf(LPCSTR lpszFormat, ...);
	void dump(BYTE* pDump, int len);

	BOOL m_bInit;
	HANDLE m_stdin;
	HANDLE m_stdout;
	HANDLE m_stderr;
};

extern CConsole dbg;

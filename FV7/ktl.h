//////////////////////////////////////////////////////////////////////
// debug.h

#ifndef __DEBUGCONSOLE_LSP
#define __DEBUGCONSOLE_LSP

class CConsole
{
public:
	CConsole();
	~CConsole();

	void Init();

	void puts(LPCTSTR);
	void __cdecl printf(LPCTSTR lpszFormat, ...);
	void __cdecl printfA(LPCSTR lpszFormat, ...);
	void __cdecl trace2(DWORD_PTR dwCategory, UINT nLevel, LPCSTR pszFormat, ...);
	void __cdecl trace2(DWORD_PTR dwCategory, UINT nLevel, LPCTSTR pszFormat, ...);
	void dump(BYTE* pDump, int len);
	LPCTSTR errortext(DWORD code, LPTSTR pszText, int cch);

	BOOL m_bInit;
	HANDLE m_stdin;
	HANDLE m_stdout;
	HANDLE m_stderr;
};

extern CConsole dbg;


class CTimeWaste
{
public:
	CTimeWaste() { m_dwTime = 0; }
	~CTimeWaste() {}
	void Start() { m_dwTime = GetTickCount(); }
	void Stop() { m_dwTime = GetTickCount() - m_dwTime; }
	DWORD m_dwTime;
};

class CDebugEnterLeave
{
private:
	CString m_strTitle;
public:
	CDebugEnterLeave(LPCTSTR pszTitle)
	{
		m_strTitle = pszTitle;
		dbg.printf(L"*** Enter %s ***\n", (LPCTSTR)m_strTitle);
	}
	~CDebugEnterLeave() 
	{
		dbg.printf(L"*** Leave %s ***\n", (LPCTSTR)m_strTitle);
	}
};

#endif			// __DEBUGCONSOLE_LSP

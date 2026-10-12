//////////////////////////////////////////////////////////////////////
// codepage_output.h ファイル出力用

#pragma once
#ifndef __CODEPAGE_OUTPUT_H__
#define __CODEPAGE_OUTPUT_H__

class CCodePageOutput
{
private:
	HANDLE m_hf;
	UINT m_codepage;

public:
	CCodePageOutput(HANDLE hf, UINT cp = CP_UTF8)
		: m_hf(hf)
		, m_codepage(cp)
	{}
	~CCodePageOutput()
	{}

	void puts(LPCTSTR psz)
	{
		// コード変換
		int len = lstrlen(psz) * 3;
		char* pAnsi = new char[len];
		WideCharToMultiByte(m_codepage, 0, psz, -1, pAnsi, len, NULL, NULL);
		DWORD dw;
		WriteFile(m_hf, pAnsi, lstrlenA(pAnsi), &dw, NULL);
		delete [] pAnsi;
	}

	void __cdecl printf(LPCTSTR pszFormat, ...)
	{
		TCHAR* pOutput = new TCHAR[2048];
		va_list argList;
		va_start(argList, pszFormat);
		wvsprintf(pOutput, pszFormat, argList);
		va_end(argList);

		puts(pOutput);
		delete [] pOutput;
	}
};


#endif		// __CODEPAGE_OUTPUT_H__

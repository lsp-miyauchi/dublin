// SplitCmdLine.h: CSplitCmdLine クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLITCMDLINE_H__33078DD1_4D0A_4096_A86F_04F5C9972590__INCLUDED_)
#define AFX_SPLITCMDLINE_H__33078DD1_4D0A_4096_A86F_04F5C9972590__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSplitCmdLine  
{
public:
	BOOL IsUpdateCommands();
	BOOL m_bHide;
	BOOL m_bSfx;
	BOOL m_bScs;
	BOOL m_bScc;
	UINT m_codePage;
	char* m_lpBaseDirectory;
	char** m_argv;
	int m_argc;
	BOOL Split(const char* lpCmdLine);
	CSplitCmdLine();
	virtual ~CSplitCmdLine();

private:
	char* m_lpCmdLine;
	BOOL IsBaseDir(const char* lpOption);
};

#endif // !defined(AFX_SPLITCMDLINE_H__33078DD1_4D0A_4096_A86F_04F5C9972590__INCLUDED_)

/*
 * type definition
 */

#pragma once
extern LPCTSTR EXTENSION_EXE;
extern LPCTSTR EXTENSION_ZIP;
extern LPCTSTR EXTENSION_CAB;
extern LPCTSTR EXTENSION_TAR;
extern LPCTSTR EXTENSION_7ZP;
extern LPCTSTR EXTENSION_LZH;
extern LPCTSTR EXTENSION_BZA;
extern LPCTSTR EXTENSION_GZA;

//////////////////////////////////////////////////////////////////////
//
typedef struct tagCMDNAME {
	LPCTSTR pszName;
	LPCTSTR pszCmd;
} CMDNAME;

//////////////////////////////////////////////////////////////////////
//
class CPackInfo
{
public:
	CPageBar* m_pbar;
	HDROP m_hdrop;
	BOOL m_incdir;              // フォルダが含まれるか
	BOOL m_sfx;
	BOOL m_unicode;
	BOOL m_utf8;
	int m_pathmode;
	int m_arctype;
	CString m_cmd;
	CString m_path;
	CString m_arcname;
	CString m_temp;
	CString m_base;

public:
	CPackInfo()
		: m_pbar(NULL)
		, m_hdrop(NULL)
		, m_incdir(FALSE)
		, m_sfx(FALSE)
		, m_unicode(FALSE)
		, m_utf8(FALSE)
		, m_pathmode(BASEDIR_NOPATH)
		, m_arctype(ARC_ERROR)
	{
	}

	~CPackInfo()
	{
	}
};

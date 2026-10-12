/////////////////////////////////////////////////////////////////////////////
// LCCCtrls - client side for a LCC controls

#ifndef _LCCCTRLS_H_
#define _LCCCTRLS_H_

#pragma once

#ifndef _LSCORE_H_
	#error lccctrls.h requires lscore.h to be included first
#endif

namespace WTL
{

#include "lcc_include_rectctrl.h"				// CRectCtrl
#include "lcc_include_trackctrl.h"				// CTrackCtrl
#include "lcc_include_degreectrl.h"				// CDegreeCtrl
#include "lcc_include_sliderctrl.h"				// CLccSliderCtrl
#include "lcc_include_foldertreectrl.h"			// CFolderTreeCtrl
#include "lcc_include_fontpickerctrl.h"			// CFontPickerCtrl
#include "lcc_include_dlview.h"					// CDrawListView
#include "lcc_include_zoomviewctrl.h"			// CZoomViewCtrl
#include "lcc_include_splitterctrl.h"			// CSplitterCtrl
#include "lcc_include_coloredprogressctrl.h"	// CColoredProgressCtrl
#include "lcc_include_checktreeviewctrl.h"		// CCheckTreeViewCtrl
#include "lcc_include_animationimagectrl.h"		// CAnimationImageCtrl
#include "lcc_include_tabctrl.h"				// CLccTabCtrl
#include "lcc_include_searchpathctrl.h"			// CLccSearchPathCtrl


/////////////////////////////////////////////////////////////////////////////
// Version Information

class CFileVersion
{
private:
	HGLOBAL m_hVersion;

public:
	CFileVersion() : m_hVersion(NULL)
	{
	}

	~CFileVersion()
	{
		Close();
	}

	void Close()
	{
		if (m_hVersion) {
			lccVerClose(m_hVersion);
			m_hVersion = NULL;
		}
	}

	BOOL Open(LPCTSTR pszFile)
	{
		ATLASSERT(pszFile != NULL);
		m_hVersion = lccVerOpen(pszFile);
		return (m_hVersion != NULL);
	}

	BOOL QueryValue(LPCTSTR pszValue, LPTSTR pszText, int cchMax)
	{
		ATLASSERT(pszValue != NULL);
		ATLASSERT(pszText != NULL);
		ATLASSERT(m_hVersion != NULL);
		return (m_hVersion ? lccVerQueryValueEx(m_hVersion, pszValue, pszText, cchMax) : FALSE);
	}

	BOOL QueryValue(LPCTSTR pszValue, CString& str)
	{
		ATLASSERT(pszValue != NULL);
		LPTSTR p = str.GetBuffer(MAX_PATH * 4);
		*p = 0;
		BOOL bOK = QueryValue(pszValue, p, str.GetAllocLength());
		str.ReleaseBuffer();
		return bOK;
	}

	inline BOOL QueryValue(int idValue, CString& str) { return QueryValue(MAKEINTRESOURCE(idValue), str); }
	inline BOOL QueryValue(int idValue, LPTSTR psz, int cchMax) { ATLASSERT(psz != NULL); return QueryValue(MAKEINTRESOURCE(idValue), psz, cchMax); }

	inline BOOL GetComments(LPTSTR psz, int cchMax) { return QueryValue(KV_COMMENTS, psz, cchMax); }
	inline BOOL GetComments(CString& str) { return QueryValue(KV_COMMENTS, str); }

	inline BOOL GetCompanyName(LPTSTR psz, int cchMax) { return QueryValue(KV_COMPANYNAME, psz, cchMax); }
	inline BOOL GetCompanyName(CString& str) { return QueryValue(KV_COMPANYNAME, str); }

	inline BOOL GetFileDescription(LPTSTR psz, int cchMax) { return QueryValue(KV_FILEDESCRIPTION, psz, cchMax); }
	inline BOOL GetFileDescription(CString& str) { return QueryValue(KV_FILEDESCRIPTION, str); }

	inline BOOL GetInternalName(LPTSTR psz, int cchMax) { return QueryValue(KV_INTERNALNAME, psz, cchMax); }
	inline BOOL GetInternalName(CString& str) { return QueryValue(KV_INTERNALNAME, str); }

	inline BOOL GetLegalCopyright(LPTSTR psz, int cchMax) { return QueryValue(KV_LEGALCOPYRIGHT, psz, cchMax); }
	inline BOOL GetLegalCopyright(CString& str) { return QueryValue(KV_LEGALCOPYRIGHT, str); }

	inline BOOL GetLegalTradeMarks(LPTSTR psz, int cchMax) { return QueryValue(KV_LEGALTRADEMARKS, psz, cchMax); }
	inline BOOL GetLegalTradeMarks(CString& str) { return QueryValue(KV_LEGALTRADEMARKS, str); }

	inline BOOL GetOriginalFileName(LPTSTR psz, int cchMax) { return QueryValue(KV_ORIGINALFILENAME, psz, cchMax); }
	inline BOOL GetOriginalFileName(CString& str) { return QueryValue(KV_ORIGINALFILENAME, str); }

	inline BOOL GetPrivateBuild(LPTSTR psz, int cchMax) { return QueryValue(KV_PRIVATEBUILD, psz, cchMax); }
	inline BOOL GetPrivateBuild(CString& str) { return QueryValue(KV_PRIVATEBUILD, str); }

	inline BOOL GetProductName(LPTSTR psz, int cchMax) { return QueryValue(KV_PRODUCTNAME, psz, cchMax); }
	inline BOOL GetProductName(CString& str) { return QueryValue(KV_PRODUCTNAME, str); }

	inline BOOL GetSpecialBuild(LPTSTR psz, int cchMax) { return QueryValue(KV_SPECIALBUILD, psz, cchMax); }
	inline BOOL GetSpecialBuild(CString& str) { return QueryValue(KV_SPECIALBUILD, str); }

	BOOL GetFileOS(LPTSTR psz)
	{
		ATLASSERT(psz != NULL);
		ATLASSERT(m_hVersion != NULL);
		return (m_hVersion ? lccVerGetFileOS(m_hVersion, psz) : FALSE);
	}

	BOOL GetFileOS(CString& str)
	{
		LPTSTR p = str.GetBuffer(MAX_PATH);
		*p = 0;
		BOOL bOK = GetFileOS(p);
		str.ReleaseBuffer();
		return bOK;
	}

	DWORD GetFileOSFlags()
	{
		ATLASSERT(m_hVersion != NULL);
		return (m_hVersion ? lccVerGetFileOSFlags(m_hVersion) : FALSE);
	}

	BOOL GetFileType(LPTSTR psz)
	{
		ATLASSERT(psz != NULL);
		return (m_hVersion ? lccVerGetFileType(m_hVersion, psz) : FALSE);
	}

	BOOL GetFileType(CString& str)
	{
		LPTSTR p = str.GetBuffer(MAX_PATH);
		*p = 0;
		BOOL bOK = GetFileType(p);
		str.ReleaseBuffer();
		return bOK;
	}

	BOOL GetProductVersion(LPTSTR psz)
	{
		ATLASSERT(psz != NULL);
		ATLASSERT(m_hVersion != NULL);
		return (m_hVersion ? lccVerGetVersion(m_hVersion, KV_PRODUCTVERSION, psz) : FALSE);
	}

	BOOL GetProductVersion(CString& str)
	{
		LPTSTR p = str.GetBuffer(MAX_PATH);
		*p = 0;
		BOOL bOK = GetProductVersion(p);
		str.ReleaseBuffer();
		return bOK;
	}

	BOOL GetFileVersion(LPTSTR psz)
	{
		ATLASSERT(psz != NULL);
		ATLASSERT(m_hVersion != NULL);
		return (m_hVersion ? lccVerGetVersion(m_hVersion, KV_FILEVERSION, psz) : FALSE);
	}

	BOOL GetFileVersion(CString& str)
	{
		LPTSTR p = str.GetBuffer(MAX_PATH);
		*p = 0;
		BOOL bOK = GetFileVersion(p);
		str.ReleaseBuffer();
		return bOK;
	}
};


}; //namespace WTL

#endif			// _LCCCTRLS_H_

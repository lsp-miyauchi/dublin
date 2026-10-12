////////////////////////////////////////////////////////////////////////
// pidl.h

#pragma once
#ifndef __PIDL_H__
#define __PIDL_H__

BOOL idl_GetName(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD flags, CString& strName);
BOOL idl_GetName(LPCITEMIDLIST pidlFq, DWORD flag, CString& str);
BOOL getlinkinfo(LPCTSTR pszName, CString& target);
BOOL idl_GetPathFromIDList(LPCITEMIDLIST pidlFq, CString& strPath);


#endif		// __PIDL_H__

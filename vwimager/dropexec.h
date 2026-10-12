////////////////////////////////////////////////////////////////////////
// dropexec.h

#pragma once
#ifndef __DROPEXEC_H__
#define __DROPEXEC_H__

class CRegExec
{
public:
	CString m_path;
	HGLOBAL m_hmem;
	DWORD m_eff;

	CRegExec()
		: m_eff(0)
		, m_hmem(NULL)
	{
	}
	~CRegExec()
	{
		if (m_hmem) GlobalFree(m_hmem);
	}
	bool Initialize(LPCTSTR pszRegName)
	{
		bool ok = false;
		CRegKey reg;
		if (reg.Open(HKEY_CURRENT_USER, pszRegName) == ERROR_SUCCESS) {
			// effect
			if (reg.QueryDWORDValue(_T("effect"), m_eff) != ERROR_SUCCESS)
				m_eff = 0;

			// path
			TCHAR buf[MAX_PATH * 2];
			ULONG len = _COUNTOF(buf);
			if (reg.QueryStringValue(_T("path"), buf, &len) == ERROR_SUCCESS)
				m_path = buf;

			// HDROP
			len = 0;
			if (reg.QueryBinaryValue(_T("drop"), NULL, &len) == ERROR_SUCCESS && len > 0) {
				m_hmem = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, len);
				if (m_hmem) {
					void* p = GlobalLock(m_hmem);
					bool ok = (reg.QueryBinaryValue(_T("drop"), p, &len) == ERROR_SUCCESS);
					GlobalUnlock(m_hmem);
					if (!ok) {
						GlobalFree(m_hmem);
						m_hmem = NULL;
					}
				}
			}

			reg.Close();
			SHDeleteKey(HKEY_CURRENT_USER, pszRegName);
			ok = true;
		}
		return ok;
	}
};



#endif		// __DROPEXEC_H__

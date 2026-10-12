////////////////////////////////////////////////////////////////////////
// regist.h

#pragma once
#ifndef __LSPSHELL_REGIST_H__
#define __LSPSHELL_REGIST_H__


////////////////////////////////////////////////////////////////////////
//

class CUserClassKey : public CRegKey
{
private:
	CString m_strStartRoot;
public:
	LSTATUS m_r;
	CUserClassKey(bool bWrite = true)
	{
		m_strStartRoot = _T("Software\\Classes");
		if (bWrite)
			m_r = Create(HKEY_CURRENT_USER, m_strStartRoot);
		else
			m_r = Open(HKEY_CURRENT_USER, m_strStartRoot, KEY_READ);
	}
	inline LSTATUS Status() { return m_r; }
	inline LPCTSTR GetRoot() const { return (LPCTSTR)m_strStartRoot; }
};

class CSettingKey : public CRegKey
{
private:
	CString m_strStartRoot;
public:
	LSTATUS m_r;
	CSettingKey(bool bWrite = true)
	{
		m_strStartRoot.Format(_T("Software\\%s\\ShellSetting"), APP_COMPANY);
		if (bWrite)
			m_r = Create(HKEY_CURRENT_USER, m_strStartRoot);
		else
			m_r = Open(HKEY_CURRENT_USER, m_strStartRoot, KEY_READ);
	}
	inline LSTATUS Status() { return m_r; }
	inline LPCTSTR GetRoot() const { return (LPCTSTR)m_strStartRoot; }
};


////////////////////////////////////////////////////////////////////////
//


void GetShellSetting(DWORD& arc, DWORD& etc, DWORD& img);
void SetShellSetting(DWORD arc, DWORD etc, DWORD img);
void ClearShellSetting();
void RegisterInit(DWORD arc, DWORD etc, DWORD img);
void RegisterTerm();

#endif		// __LSPSHELL_REGIST_H__

//////////////////////////////////////////////////////////////////////
// regkey.h

#ifndef __REGKEY_INCLUDE
#define __REGKEY_INCLUDE

namespace WTL
{

class regkey
{
public:
	regkey() : m_sec(NULL)
	{
	}

	regkey(HKEY hRoot, _U_STRINGorID sec)
	{
		open(hRoot, sec);
	}

	~regkey()
	{
		close();
	}

	LPTSTR str_alloc(_U_STRINGorID& sid)
	{
		LPTSTR pstr = NULL;
		if (IS_INTRESOURCE(sid.m_lpstr)) {
			pstr = new TCHAR[256];
			::LoadString(_Module.GetResourceInstance(), LOWORD(sid.m_lpstr), pstr, 256);
			sid.m_lpstr = pstr;
		}
		return pstr;
	}

	bool open(HKEY hRoot, _U_STRINGorID sec)
	{
		LPTSTR pstr = str_alloc(sec);

		DWORD dw;
		long res = RegCreateKeyEx(hRoot, sec.m_lpstr, 0,
							REG_NONE, REG_OPTION_NON_VOLATILE,
							(KEY_WRITE|KEY_READ) & ~KEY_NOTIFY, NULL, &m_sec, &dw);
		if (pstr) delete [] pstr;

		if (res != ERROR_SUCCESS) {
			m_sec = NULL;
			return false;
		}
		return true;
	}

	void close()
	{
		if (m_sec) {
			RegCloseKey(m_sec);
			m_sec = NULL;
		}
	}

	int gettext(_U_STRINGorID key, LPTSTR pText, int cchMax)
	{
		if (!m_sec) return 0;

		LPTSTR pstr = str_alloc(key);

		DWORD type = REG_SZ;
		DWORD len = cchMax * sizeof(TCHAR);
		long res = RegQueryValueEx(m_sec, key.m_lpstr, NULL, &type, (BYTE*)pText, &len);
		if (res != ERROR_SUCCESS)
			*pText = '\0';
		if (pstr) delete [] pstr;

		return lstrlen(pText);
	}

	BOOL settext(_U_STRINGorID key, LPCTSTR pText)
	{
		if (!m_sec) return FALSE;

		LPTSTR pstr = str_alloc(key);
		long res;
		if (!pText)	res = RegDeleteValue(m_sec, key.m_lpstr);
		else		res = RegSetValueEx(m_sec, key.m_lpstr, 0, REG_SZ, (BYTE*)pText, sizeof(TCHAR) * (lstrlen(pText) + 1));

		if (pstr) delete [] pstr;
		return (res == ERROR_SUCCESS);
	}

	int getvalue(_U_STRINGorID key, int nDefValue)
	{
		if (!m_sec) return nDefValue;

		LPTSTR pstr = str_alloc(key);
		DWORD val;
		DWORD type = REG_DWORD;
		DWORD len = sizeof(DWORD);
		long res = RegQueryValueEx(m_sec, key.m_lpstr, NULL, &type, (BYTE*)&val, &len);

		if (pstr) delete [] pstr;
		return (res != ERROR_SUCCESS) ? nDefValue : (int)val;
	}

	BOOL setvalue(_U_STRINGorID key, int val)
	{
		if (!m_sec) return FALSE;

		LPTSTR pstr = str_alloc(key);
		DWORD dw = (DWORD)val;
		long res = RegSetValueEx(m_sec, key.m_lpstr, 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));

		if (pstr) delete [] pstr;
		return (res != ERROR_SUCCESS);
	}

	int getbinary(_U_STRINGorID key, void* pData, int cbSize)
	{
		if (!m_sec) return 0;

		LPTSTR pstr = str_alloc(key);
		DWORD type = REG_BINARY;
		DWORD len = cbSize;
		long res = RegQueryValueEx(m_sec, key.m_lpstr, NULL, &type, (BYTE*)pData, &len);

		if (pstr) delete [] pstr;
		return (res == ERROR_SUCCESS) ? len : 0;
	}

	BOOL setbinary(_U_STRINGorID key, void* pData, int cbSize)
	{
		if (!m_sec) return FALSE;

		LPTSTR pstr = str_alloc(key);
		long res;
		if (!pData)	res = RegDeleteValue(m_sec, key.m_lpstr);
		else		res = RegSetValueEx(m_sec, key.m_lpstr, 0, REG_BINARY, (BYTE*)pData, cbSize);

		if (pstr) delete [] pstr;
		return (res == ERROR_SUCCESS);
	}


protected:
	HKEY m_sec;
};

};				// namespace WTL

#endif			// __REGKEY_INCLUDE

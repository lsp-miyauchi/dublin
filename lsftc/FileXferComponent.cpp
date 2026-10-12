// FileXferComponent.cpp : CFileXferComponent の実装

#include "stdafx.h"
#include "FileXferComponent.h"

extern BOOL (WINAPI* __FileTimeToDosDateTime)(CONST FILETIME*, LPWORD, LPWORD);
extern BOOL (WINAPI* __DosDateTimeToFileTime)(WORD, WORD, FILETIME*);
extern LONG (WINAPI* __CompareFileTime)(CONST FILETIME*, CONST FILETIME*);
extern HRESULT (WINAPI* __CoCreateGuid)(GUID*);

extern long GetDeltaDate(const FILETIME* ps, const FILETIME* pt);

bool MakeRegSecString(REFCLSID rc, CString& sec);
bool GetCurrentFileTime(FILETIME* pft);
DWORD PickRegSecData(LPCTSTR psz);
void BuildRegSecData(GUID g, DWORD dt, CString& str);


// {E23FB438-BDDF-4a1a-AF11-E059D36421F2}
static const GUID GUID_FileVisor7 = 
{ 0xe23fb438, 0xbddf, 0x4a1a, { 0xaf, 0x11, 0xe0, 0x59, 0xd3, 0x64, 0x21, 0xf2 } };

// {3FC90CD5-53C9-431b-8B26-DDF5707237DD}
static const GUID GUID_Limitter7 = 
{ 0x3fc90cd5, 0x53c9, 0x431b, { 0x8b, 0x26, 0xdd, 0xf5, 0x70, 0x72, 0x37, 0xdd } };


static bool getRealKeyValue(REFCLSID rc, CStringW& key)
{
	bool ok = false;
	CString sec;
	MakeRegSecString(rc, sec);
	CRegKey reg;
	if (reg.Open(HKEY_CURRENT_USER, sec, KEY_READ & ~KEY_NOTIFY) == ERROR_SUCCESS) {
		TCHAR buf[MAX_PATH * 2];
		ULONG len = ARRAYSIZE(buf);
		if (reg.QueryStringValue(NULL, buf, &len) == ERROR_SUCCESS && len > 1) {
			key = buf;
			ok = true;
		}
		reg.Close();
	}
	return ok;
}

// CFileXferComponent

HRESULT CFileXferComponent::FinalConstruct()
{
	GetCurrentFileTime(&m_ftCurrent);
	return S_OK;
}

STDMETHODIMP CFileXferComponent::GetInstance(LONG* val)
{
	LONG limits = *val;
	*val = 0;
	HRESULT hr = E_FAIL;
	CString str;
	if (getRealKeyValue(GUID_FileVisor7, str)) {
		DWORD datetime = PickRegSecData(str);
		FILETIME ft;
		if (__DosDateTimeToFileTime(HIWORD(datetime), 0, &ft)) {
			if (__CompareFileTime(&m_ftCurrent, &ft) > 0) {
				*val = limits - GetDeltaDate(&ft, &m_ftCurrent);
				hr = S_OK;
			}
		}
	}
	return hr;
}

STDMETHODIMP CFileXferComponent::ResetInstance(void)
{
	CString sec;
	MakeRegSecString(GUID_FileVisor7, sec);
	CRegKey reg;
	if (reg.Open(HKEY_CURRENT_USER, sec, KEY_READ & ~KEY_NOTIFY) == ERROR_SUCCESS) {
		reg.Close();
		SHDeleteKey(HKEY_CURRENT_USER, sec);
	}
	return S_OK;
}

STDMETHODIMP CFileXferComponent::InitInstance(void)
{
	//
	// FileVisor7をアンインスト－ルした後に再インストールした際に呼び出される
	// fv7.cpp:CheckAndConvertReg()
	//

    // オブジェクト、タイプ ライブラリおよびタイプ ライブラリ内の全てのインターフェイスを登録します
	CString str;
	bool bKeyExists = getRealKeyValue(GUID_FileVisor7, str);
	if (bKeyExists) {
		// インストールしてから180日以上経過していれば、試用期間をリセット
		DWORD datetime = PickRegSecData(str);
		FILETIME ftInstall;
		__DosDateTimeToFileTime(HIWORD(datetime), LOWORD(datetime), &ftInstall);
		if (GetDeltaDate(&ftInstall, &m_ftCurrent) > 180)
			bKeyExists = false;
	}

	if (!bKeyExists) {
		CString sec;
		MakeRegSecString(GUID_FileVisor7, sec);
		CRegKey reg;
		if (reg.Create(HKEY_CURRENT_USER, sec, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE & ~KEY_NOTIFY) == ERROR_SUCCESS) {
			// 今日を試用期間開始日にセット
			WORD d, t;
			__FileTimeToDosDateTime(&m_ftCurrent, &d, &t);
			BuildRegSecData(GUID_Limitter7, MAKELONG(t, d), sec);
			reg.SetStringValue(NULL, sec);
			reg.Close();
		}
	}
	return S_OK;
}

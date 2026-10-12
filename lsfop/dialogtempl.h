/////////////////////////////////////////////////////////////////////////////
// CDialogTemplate

#pragma once

#pragma pack(1)

typedef struct {
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
} zDLGTEMPLATEEX;

#pragma pack()

/////////////////////////////////////////////////////////////////////////////
//

class CDialogTemplate
{
public:
	DLGTEMPLATE* m_pTemplate;

	CDialogTemplate(LPCTSTR pszResource = NULL)
	{
		m_pTemplate = NULL;
		if (pszResource) Load(pszResource);
	}

	~CDialogTemplate()
	{
		if (m_pTemplate) {
			free(m_pTemplate);
		}
	}

	bool Load(LPCTSTR pszResource = NULL)
	{
		if (m_pTemplate) {
			free(m_pTemplate);
			m_pTemplate = NULL;
		}

		bool ok = false;
		if (pszResource) {
			HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
			HRSRC hRsrc = FindResource(hInst, pszResource, RT_DIALOG);
			if (hRsrc) {
				HGLOBAL hTemplate = LoadResource(hInst, hRsrc);
				DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)LockResource(hTemplate);
				size_t size = SizeofResource(hInst, hRsrc);
				m_pTemplate = (DLGTEMPLATE*)malloc(size);
				if (m_pTemplate) {
					memcpy(m_pTemplate, pTemplate, size);
					ok = true;
				}
				UnlockResource(hTemplate);
				FreeResource(hTemplate);
			}
		}
		return ok;
	}

	__inline bool IsNull() const
	{
		return (m_pTemplate == NULL);
	}

	__inline bool IsDialogEx() const
	{
		ATLASSERT(m_pTemplate != NULL);
		return (((zDLGTEMPLATEEX*)m_pTemplate)->signature == 0xFFFF);
	}

	void ModifyStyle(DWORD dwRemove, DWORD dwAdd)
	{
		ATLASSERT(m_pTemplate != NULL);
		DWORD* pstyle = IsDialogEx() ? &((zDLGTEMPLATEEX*)m_pTemplate)->style : &m_pTemplate->style;
		*pstyle = (*pstyle & ~dwRemove) | dwAdd;
	}

	void ModifyExStyle(DWORD dwRemove, DWORD dwAdd)
	{
		ATLASSERT(m_pTemplate != NULL);
		DWORD* pexStyle = IsDialogEx() ? &((zDLGTEMPLATEEX*)m_pTemplate)->exStyle : &m_pTemplate->dwExtendedStyle;
		*pexStyle = (*pexStyle & ~dwRemove) | dwAdd;
	}

	operator LPCDLGTEMPLATE() const
	{
		return m_pTemplate;
	}
};

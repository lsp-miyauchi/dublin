/////////////////////////////////////////////////////////////////////////////
// CDialogTemplate

#pragma once

#pragma pack(1)

typedef struct
{
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
} DLGTEMPLATEEX;

typedef struct
{
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	DWORD id;
} DLGITEMTEMPLATEEX;

#pragma pack()

extern inline BOOL IsDialogEx(const DLGTEMPLATE* pTemplate)
{
	return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}

extern inline BOOL HasFont(const DLGTEMPLATE* pTemplate)
{
	return (DS_SETFONT & (IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->style : pTemplate->style));
}

extern inline int FontAttrSize(BOOL bDialogEx)
{
	return (int)sizeof(WORD) * (bDialogEx ? 3 : 1);
}

inline WCHAR* _SkipString(WCHAR* p)
{
	while (*p++);
	return p;
}

extern inline void ConvertDialogUnitsToPixels(LPCTSTR pszFontFace, WORD wFontSize, int cxDlg, int cyDlg, SIZE* pSizePixel)
{
	// Attempt to create the font to be used in the dialog box
	UINT cxSysChar, cySysChar;
	LOGFONT lf;
	HDC hDC = ::GetDC(NULL);
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = -MulDiv(wFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lstrcpyn(lf.lfFaceName, pszFontFace, ARRAYSIZE(lf.lfFaceName));

	HFONT hNewFont = CreateFontIndirect(&lf);
	if (hNewFont != NULL) {
		HFONT hFontOld = (HFONT)SelectObject(hDC, hNewFont);
		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);
		cySysChar = tm.tmHeight + tm.tmExternalLeading;
		SIZE size;
		::GetTextExtentPoint32(hDC, _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"), 52, &size);
		cxSysChar = (size.cx + 26) / 52;
		SelectObject(hDC, hFontOld);
		DeleteObject(hNewFont);
	}
	else {
		// Could not create the font so just use the system's values
		cxSysChar = LOWORD(GetDialogBaseUnits());
		cySysChar = HIWORD(GetDialogBaseUnits());
	}
	::ReleaseDC(NULL, hDC);

	// Translate dialog units to pixels
	pSizePixel->cx = MulDiv(cxDlg, cxSysChar, 4);
	pSizePixel->cy = MulDiv(cyDlg, cySysChar, 8);
}

////////////////////////////////////////////////////////////////////////////
//

class CDialogTemplate
{
// Constructors
public:
	/* explicit */ CDialogTemplate(const DLGTEMPLATE* pTemplate = NULL)
	{
		if (pTemplate == NULL) {
			m_hTemplate = NULL;
			m_dwTemplateSize = 0;
			m_bSystemFont = FALSE;
		}
		else {
			SetTemplate(pTemplate, GetTemplateSize(pTemplate));
		}
	}

	explicit CDialogTemplate(HGLOBAL hTemplate)
	{
		if (hTemplate == NULL) {
			m_hTemplate = NULL;
			m_dwTemplateSize = 0;
			m_bSystemFont = FALSE;
		}
		else {
			DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);
			SetTemplate(pTemplate, GetTemplateSize(pTemplate));
			GlobalUnlock(hTemplate);
		}
	}


// Attributes
	BOOL HasFont() const
	{
		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
		BOOL bHasFont = ::HasFont(pTemplate);
		GlobalUnlock(m_hTemplate);
		return bHasFont;
	}

	BOOL SetFont(LPCTSTR lpFaceName, WORD nFontSize)
	{
		ATLASSERT(m_hTemplate != NULL);

		if (m_dwTemplateSize == 0) return FALSE;

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);

		BOOL bDialogEx = IsDialogEx(pTemplate);
		BOOL bHasFont = ::HasFont(pTemplate);
		int cbFontAttr = FontAttrSize(bDialogEx);

		if (bDialogEx)
			((DLGTEMPLATEEX*)pTemplate)->style |= DS_SETFONT;
		else
			pTemplate->style |= DS_SETFONT;

		int nFaceNameLen = lstrlen(lpFaceName);
		if( nFaceNameLen >= LF_FACESIZE ) {
			// Name too long
			return FALSE;
		}

#ifdef _UNICODE
		int cbNew = cbFontAttr + ((nFaceNameLen + 1) * sizeof(TCHAR));
		BYTE* pbNew = (BYTE*)lpFaceName;
#else
		WCHAR wszFaceName [LF_FACESIZE];
		int cbNew = cbFontAttr + 2 * MultiByteToWideChar(CP_ACP, 0, lpFaceName, -1, wszFaceName, LF_FACESIZE);
		BYTE* pbNew = (BYTE*)wszFaceName;
#endif

		BYTE* pb = GetFontSizeField(pTemplate);
		int cbOld = (int)(bHasFont ? cbFontAttr + 2 * (wcslen((WCHAR*)(pb + cbFontAttr)) + 1) : 0);

		BYTE* pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~DWORD_PTR(3));
		BYTE* pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~DWORD_PTR(3));

		WORD nCtrl = bDialogEx ? (WORD)((DLGTEMPLATEEX*)pTemplate)->cDlgItems : (WORD)pTemplate->cdit;

		if (cbNew != cbOld && nCtrl > 0)
			memmove(pNewControls, pOldControls, (size_t)(m_dwTemplateSize - (pOldControls - (BYTE*)pTemplate)));

		*(WORD*)pb = nFontSize;
		memmove(pb + cbFontAttr, pbNew, cbNew - cbFontAttr);

		//IA64: Max dialog template size of 4GB should be fine
		m_dwTemplateSize += ULONG(pNewControls - pOldControls);

		GlobalUnlock(m_hTemplate);
		m_bSystemFont = FALSE;
		return TRUE;
	}

	BOOL SetSystemFont(WORD nFontSize = 0)
	{
		LPCTSTR pszFace = _T("System");
		WORD wDefSize = 10;
		HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
		if (hFont == NULL)
			hFont = (HFONT)::GetStockObject(SYSTEM_FONT);
		if (hFont != NULL) {
			LOGFONT lf;
			if (::GetObject(hFont, sizeof(LOGFONT), &lf) != 0) {
				pszFace = lf.lfFaceName;
				HDC hDC = ::GetDC(NULL);
				if (lf.lfHeight < 0)
					lf.lfHeight = -lf.lfHeight;
				wDefSize = (WORD)MulDiv(lf.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY));
				::ReleaseDC(NULL, hDC);
			}
		}

		if (nFontSize == 0)
			nFontSize = wDefSize;

		return SetFont(pszFace, nFontSize);
	}

	BOOL GetFont(CString& strFaceName, WORD& nFontSize) const
	{
		ATLASSERT(m_hTemplate != NULL);

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
		BOOL bResult = GetFont(pTemplate, strFaceName, nFontSize);
		GlobalUnlock(m_hTemplate);
		return bResult;
	}

	void GetSizeInDialogUnits(SIZE* pSize) const
	{
		ATLASSERT(m_hTemplate != NULL);

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
		if (IsDialogEx(pTemplate)) {
			pSize->cx = ((DLGTEMPLATEEX*)pTemplate)->cx;
			pSize->cy = ((DLGTEMPLATEEX*)pTemplate)->cy;
		}
		else {
			pSize->cx = pTemplate->cx;
			pSize->cy = pTemplate->cy;
		}
		GlobalUnlock(m_hTemplate);
	}

	void GetSizeInPixels(SIZE* pSize) const
	{
		ATLASSERT(m_hTemplate != NULL);

		if (m_bSystemFont) {
			GetSizeInDialogUnits(pSize);
			DWORD dwDLU = GetDialogBaseUnits();
			pSize->cx = (pSize->cx * LOWORD(dwDLU)) / 4;
			pSize->cy = (pSize->cy * HIWORD(dwDLU)) / 8;
		}
		else {
			CString strFace;
			WORD wSize = 10;
			GetFont(strFace, wSize);

			SIZE size;
			GetSizeInDialogUnits(&size);
			ConvertDialogUnitsToPixels(strFace, wSize, size.cx, size.cy, pSize);
		}
	}

	static BOOL __cdecl GetFont(const DLGTEMPLATE* pTemplate, CString& strFaceName, WORD& nFontSize)
	{
		ATLASSERT(pTemplate != NULL);
		if (!::HasFont(pTemplate))
			return FALSE;

		BYTE* pb = GetFontSizeField(pTemplate);
		nFontSize = *(WORD*)pb;
		pb += FontAttrSize(IsDialogEx(pTemplate));

#if defined(_UNICODE)
		// Copy font name
		strFaceName = (LPCTSTR)pb;
#else
		// Convert Unicode font name to MBCS
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)pb, -1, strFace.GetBufferSetLength(LF_FACESIZE), LF_FACESIZE, NULL, NULL);
		strFaceName.ReleaseBuffer();
#endif
		return TRUE;
	}

	DWORD GetStyle()
	{
		ATLASSERT(m_hTemplate != NULL);

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
		DWORD style = (IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->style : pTemplate->style);
		GlobalUnlock(m_hTemplate);
		return style;
	}

	DWORD GetExStyle()
	{
		ATLASSERT(m_hTemplate != NULL);

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
		DWORD exStyle = (IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->exStyle : pTemplate->dwExtendedStyle);
		GlobalUnlock(m_hTemplate);
		return exStyle;
	}

/* 
	void ModifyStyle(DWORD dwRemove, DWORD dwAdd)
	{
		ATLASSERT(m_hTemplate != NULL);

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
		DWORD* pstyle = IsDialogEx(pTemplate) ? &((DLGTEMPLATEEX*)pTemplate)->style : &pTemplate->style;
		*pstyle = (*pstyle & ~dwRemove) | dwAdd;
		GlobalUnlock(m_hTemplate);
	}

	void ModifyExStyle(DWORD dwRemove, DWORD dwAdd)
	{
		ATLASSERT(m_hTemplate != NULL);

		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
		DWORD* pexStyle = IsDialogEx(pTemplate) ? &((DLGTEMPLATEEX*)pTemplate)->exStyle : &pTemplate->dwExtendedStyle;
		*pexStyle = (*pexStyle & ~dwRemove) | dwAdd;
		GlobalUnlock(m_hTemplate);
	}
*/

// Operations
	BOOL Load(LPCTSTR lpDialogTemplateID)
	{
		HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
		HRSRC hRsrc = FindResource(hInst, lpDialogTemplateID, RT_DIALOG);
		if (hRsrc == NULL)
			return FALSE;
		HGLOBAL hTemplate = LoadResource(hInst, hRsrc);
		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)LockResource(hTemplate);
		SetTemplate(pTemplate, (UINT)SizeofResource(hInst, hRsrc));
		UnlockResource(hTemplate);
		FreeResource(hTemplate);
		return TRUE;
	}

	HGLOBAL Detach()
	{
		HGLOBAL hTmp = m_hTemplate;
		m_hTemplate = NULL;
		return hTmp;
	}

// Implementation
public:
	~CDialogTemplate()
	{
		if (m_hTemplate != NULL)
			GlobalFree(m_hTemplate);
	}

	HGLOBAL m_hTemplate;
	DWORD m_dwTemplateSize;
	BOOL m_bSystemFont;

protected:
	static BYTE* __cdecl GetFontSizeField(const DLGTEMPLATE* pTemplate)
	{
		BOOL bDialogEx = IsDialogEx(pTemplate);
		WORD* pw;

		if (bDialogEx)
			pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
		else
			pw = (WORD*)(pTemplate + 1);

		if (*pw == (WORD)-1)        // Skip menu name string or ordinal
			pw += 2; // WORDs
		else
			while(*pw++);

		if (*pw == (WORD)-1)        // Skip class name string or ordinal
			pw += 2; // WORDs
		else
			while(*pw++);

		while (*pw++);          // Skip caption string

		return (BYTE*)pw;
	}

	static UINT __cdecl GetTemplateSize(const DLGTEMPLATE* pTemplate)
	{
		BOOL bDialogEx = IsDialogEx(pTemplate);
		BYTE* pb = GetFontSizeField(pTemplate);

		if (::HasFont(pTemplate)) {
			// Skip font size and name
			pb += FontAttrSize(bDialogEx);  // Skip font size, weight, (italic, charset)
			pb += 2 * (wcslen((WCHAR*)pb) + 1);
		}

		WORD nCtrl = bDialogEx ? (WORD)((DLGTEMPLATEEX*)pTemplate)->cDlgItems : (WORD)pTemplate->cdit;

		while (nCtrl > 0) {
			pb = (BYTE*)(((DWORD_PTR)pb + 3) & ~DWORD_PTR(3)); // DWORD align
			pb += (bDialogEx ? sizeof(DLGITEMTEMPLATEEX) : sizeof(DLGITEMTEMPLATE));
			if (*(WORD*)pb == (WORD)-1)     // Skip class name string or ordinal
				pb += 2 * sizeof(WORD);
			else
				pb = (BYTE*)_SkipString((WCHAR*)pb);

			if (*(WORD*)pb == (WORD)-1)     // Skip text string or ordinal
				pb += 2 * sizeof(WORD);
			else
				pb = (BYTE*)_SkipString((WCHAR*)pb);

			WORD cbExtra = *(WORD*)pb;      // Skip extra data
			if (cbExtra != 0 && !bDialogEx)
				cbExtra -= 2;
			pb += sizeof(WORD) + cbExtra;
			--nCtrl;
		}

		//IA64: Max dialog template size of 4GB should be fine
		return UINT(pb - (BYTE*)pTemplate);
	}

	BOOL SetTemplate(const DLGTEMPLATE* pTemplate, UINT cb)
	{
		m_dwTemplateSize = cb;

		if ((m_hTemplate = GlobalAlloc(GPTR, m_dwTemplateSize + LF_FACESIZE * 2)) == NULL)
			return FALSE;
		DLGTEMPLATE* pNew = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
		memcpy((BYTE*)pNew, pTemplate, (size_t)m_dwTemplateSize);

		m_bSystemFont = (::HasFont(pNew) == 0);

		GlobalUnlock(m_hTemplate);
		return TRUE;
	}
};


#pragma once

/////////////////////////////////////////////////////////////////////////////
// shell_new_item.h

class CShellNewItem
{
public:
	CShellNewItem()
	{
		m_ext.Empty();
		m_name.Empty();
		m_iImage = -1;
	}
	~CShellNewItem()
	{
	}

	void Initialize(LPCTSTR pszExtText)
	{
		// \tで区切られた後半は（もしあればそれは）メニューテキスト
		m_ext = pszExtText;
		int pos = m_ext.Find(_T('\t'));
		if (pos > 0) {
			m_name = m_ext.Mid(pos + 1);
			m_ext = m_ext.Left(pos);
		}
		else {
			TCHAR buf[MAX_PATH];
			DWORD cch = ARRAYSIZE(buf);
			if (AssocQueryString(0, ASSOCSTR_FRIENDLYDOCNAME, pszExtText, NULL, buf, &cch) == S_OK && cch > 0)
				m_name = buf;
			else {
				SHFILEINFO sfi;
				SHGetFileInfo(m_ext, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES);
				m_name = sfi.szDisplayName;
			}
		}
	}
	int GetImageIndex(bool bNeedBitmap = false)
	{
		if (m_iImage == -1) {
			TCHAR buf[MAX_PATH];
			DWORD cch = ARRAYSIZE(buf);
			if (AssocQueryString(0, ASSOCSTR_DEFAULTICON, m_ext, NULL, buf, &cch) == S_OK) {
				int index = PathParseIconLocation(buf);
				int (WINAPI * pfn)(LPCWSTR, int, UINT);
#ifdef Shell_GetCachedImageIndex
#undef Shell_GetCachedImageIndex
#endif
				pfn = Shell_GetCachedImageIndex;
				m_iImage = (*pfn)(buf, index, FALSE);
			}
			/*
						SHFILEINFO sfi;
						HIMAGELIST him = (HIMAGELIST)SHGetFileInfo(m_ext, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON);
						m_iImage = sfi.iIcon;
						him;		// suppress warning C4189
			*/

			if (bNeedBitmap && m_iImage >= 0) {
				HIMAGELIST hs, hl;
				Shell_GetImageLists(&hl, &hs);
				m_bmIcon.Attach(CreateBitmapFromImageList(hs, m_iImage));
			}
		}
		return m_iImage;
	}

	CString m_ext;
	CString m_name;
	CBitmap m_bmIcon;

	LPCTSTR FormatNameExtension()
	{
		CString s;
		s.Format(_T("%s(%s)"), m_name.GetString(), m_ext.GetString());
		return s.GetString();
	}

protected:
	int m_iImage;
};


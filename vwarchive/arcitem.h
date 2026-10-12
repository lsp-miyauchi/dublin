// arcitem.h : header file
//

#pragma once
#ifndef __ARCITEM_H__
#define __ARCITEM_H__

typedef INT_PTR (CALLBACK* ENUMARCPROC)(LPCTSTR, LPARAM);


////////////////////////////////////////////////////////////////////
// TIME ACCESS DTE_(EXPAND)
//

#define DTE_SEC(d)		(((d) & 0x001f) << 1)
#define DTE_MIN(d)		(((d) & 0x07e0) >> 5)
#define DTE_HOUR(d)		(((d) & 0xf800) >> 11)
#define DTE_DAY(d)		(((d) & 0x001f))
#define DTE_MON(d)		(((d) & 0x01e0) >> 5)
#define DTE_YEAR(d)		(((d) & 0xfe00) >> 9)


////////////////////////////////////////////////////////////////////
// TIME ACCESS DTC_(CONSTRUCT)
//
#define DTC_SEC(d)		(((d) >> 1) & 0x001f)
#define DTC_MIN(d)		(((d) << 5) & 0x07e0)
#define DTC_HOUR(d)		(((d) << 11) & 0xf800)
#define DTC_DAY(d)		(((d) & 0x001f))
#define DTC_MON(d)		(((d) << 5) & 0x01e0)
#define DTC_YEAR(d)		(((d) << 9) & 0xfe00)


/////////////////////////////////////////////////////////////////////////////
// SORT MODE DEFINITION
//

enum {
	SORT_NONE,
	SORT_NAME,
	SORT_EXT,
	SORT_SIZE,
	SORT_TIME,
	SORT_PATH,
	SORT_FULLNAME,
	SORT_ATTR,
};

enum {
	SORT_UP,
	SORT_DOWN
};


#define AIF_CRYPT			0x0001


/////////////////////////////////////////////////////////////////////////////
//

class CArcItem
{
public:
	int m_serial;
	DWORD m_dwSize;
	DWORD m_dwCompressedSize;
	WORD m_wRatio;
	WORD m_wDate;
	WORD m_wTime;
	WORD m_wOsType;
	WORD m_wFlags;
	CString m_strRealName;
	CString m_strName;
	CString m_strType;
	CString m_strMode;
	CString m_strPath;
	CString m_strTips;
	CString m_strAttr;
	int m_iImage;
	DWORD m_attr;
	DWORD_PTR m_key;
public:
	CArcItem()
		: m_dwSize(0)
		, m_dwCompressedSize(0)
		, m_wRatio(0)
		, m_wDate(0)
		, m_wTime(0)
		, m_wOsType(0)
		, m_wFlags(0)
		, m_iImage(-1)
		, m_attr(0)
		, m_key(0)
	{ }
	~CArcItem()
	{ }

public:
	inline BOOL IsFolder() { return (m_attr & FILE_ATTRIBUTE_DIRECTORY); }
	inline void SetRealName(LPCTSTR psz) { m_strRealName = psz; }
	inline void SetName(LPCTSTR psz) { m_strName = psz; }
	inline void SetTypeName(LPCTSTR psz) { m_strType = psz; }
	inline void SetModeName(LPCTSTR psz) { m_strMode = psz; }
	inline void SetAttribString(LPCTSTR psz) { m_strAttr = psz; }
	inline void SetPath(LPCTSTR psz) { m_strPath = psz; }

	void SplitName(LPTSTR pszName, LPTSTR pszExt)
	{
		*pszName = 0;
		*pszExt = 0;
		if (m_strName[0] == '.')
			return;

		CString str = m_strName.GetString();
		TCHAR* p = PathFindExtension(str);
		if (*p == '.') {
			lstrcpy(pszExt, p);
			*p = 0;
			str.ReleaseBuffer();
		}
		lstrcpy(pszName, str);
	}

	void FormatSize(CString& fmt, BOOL bKB)
	{
		fmt.Empty();
		if (!IsFolder()) {
			LPTSTR p = fmt.GetBuffer(128);
			if (bKB)
				lccFormatFileSize(m_dwSize, p);
			else
				lccNumForm(m_dwSize, p);
			fmt.ReleaseBuffer();
		}
	}

	void FormatDate(CString& fmt)
	{
		fmt.Empty();
		if (m_wDate)
			fmt.Format(_T("%4u/%02u/%02u  %02u:%02u:%02u"),
				DTE_YEAR(m_wDate) + 1980, DTE_MON(m_wDate), DTE_DAY(m_wDate),
				DTE_HOUR(m_wTime), DTE_MIN(m_wTime), DTE_SEC(m_wTime));
		else
			fmt = _T("N/A");
	}

	void FormatRatio(CString& fmt)
	{
		if (IsFolder())
			fmt.Empty();
		else if (m_dwSize && m_wRatio)
			fmt.Format(_T("%u%%"), m_wRatio / 10);
		else if (!m_dwSize)
			fmt = _T("100%");
		else
			fmt = _T("N/A");
	}
};


//////////////////////////////////////////////////////////////////////
//

class CArcParse
{
public:
	DWORDLONG m_dlTotal;
	LONG m_nType;
	CString m_strFile;
	CSimpleMap<CString, DWORD_PTR> m_folder;
	CSimpleArray<CArcItem*> m_array;
	BOOL m_bUnicodeSupported;
	BOOL m_bUtf8Supported;
	bool m_bHasEncryptedItems;
public:
	CArcParse()
		: m_dlTotal(0)
		, m_nType(ARC_ERROR)
		, m_bUnicodeSupported(FALSE)
		, m_bUtf8Supported(FALSE)
		, m_bHasEncryptedItems(false)
	{
	}

	~CArcParse()
	{
		Wep();
	}

	void Wep()
	{
		int cnt = m_array.GetSize();
		for (int i = 0; i < cnt; i++) {
			CArcItem* pai = m_array[i];
			delete pai;
		}
		m_array.RemoveAll();
		m_folder.RemoveAll();
		m_dlTotal = 0;
		m_nType = ARC_ERROR;
		m_bUnicodeSupported = FALSE;
		m_bHasEncryptedItems = false;
	}

	BOOL EnumArchive(HWND hwnd, LPCTSTR pszName, ENUMARCPROC callback, LPARAM param);
	void SortItems(int type, int seq);
	void MakeSfx(HWND hwnd);
	BOOL DeleteItems(HWND hwnd, LPCTSTR pszItemList);
	BOOL RenameItems(HWND hwnd, LPCTSTR pszItemList);
	BOOL ListToClipboard(HWND hwnd);

	BOOL AppendFiles(HWND hwnd, HDROP hDrop)
	{
		if (!hDrop || DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0) < 1)
			return FALSE;

		TCHAR buf[MAX_PATH * 2], base[MAX_PATH * 2];
		DragQueryFile(hDrop, 0, buf, _COUNTOF(buf));
		PathGetParent(buf, base);

		CComPtr<IArcManager> pam;
		HRESULT hr = pam.CoCreateInstance(_uuidof(ArcManager));
		if (SUCCEEDED(hr))
			hr = pam->AddFiles(hwnd, m_strFile, base, (LONG_PTR)hDrop);
		return (SUCCEEDED(hr) ? TRUE : FALSE);
	}

	int CopyFolderGroup(CSimpleArray<CArcItem*>& arr, DWORD_PTR folderid)
	{
		arr.RemoveAll();
		int cnt = m_array.GetSize();
		for (int i = 0; i < cnt; i++) {
			CArcItem* pai = m_array[i];
			if (folderid == 0xFFFFFFFF || pai->m_key == folderid)
				arr.Add(pai);
		}
		return arr.GetSize();
	}

	int PackByFilter(CSimpleArray<CArcItem*>& arr, LPCTSTR pszFilter);

	inline BOOL CanRename() { return (m_nType == ARC_LZH || m_nType == ARC_BGA); }
	inline BOOL CanDelete() { return (m_nType == ARC_LZH || m_nType == ARC_ZIP || m_nType == ARC_BGA || m_nType == ARC_7ZIP); }
#ifdef _WIN64
	inline BOOL CanMakeSfx() { return m_nType == ARC_CAB; }
#else
	inline BOOL CanMakeSfx() { return (m_nType == ARC_LZH || m_nType == ARC_CAB || m_nType == ARC_BGA); }
#endif
	inline int GetItemCount() { return m_array.GetSize(); }
	inline BOOL IsUnicodeSupported() { return m_bUnicodeSupported; }
	BOOL IsTypeDragSource();

protected:
	void MapFolder(LPCTSTR pszDir, DWORD_PTR& folderid);
};



////////////////////////////////////////////////////////////
//

void SortArcItemArray(CSimpleArray<CArcItem*>& arr);



#endif			// __ARCITEM_H__

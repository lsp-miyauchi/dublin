//////////////////////////////////////////////////////////////////////
// filtersetting.h フィルタ設定

#pragma once
#ifndef __FILTERSETTING_H__
#define __FILTERSETTING_H__

BOOL GetAppDataFolder(CString& strFolder, LPCTSTR pszAppend);
bool CheckFileWritable(const char* pszFile);

////////////////////////////////////////////////////////////////////////
//

#define FSMAGIC				('F'|('S'<<8)|('M'<<16)|('G'<<24))

class CFilterSetting
{
public:
	CString m_strName;
	CString m_strFilter;
	BOOL m_bUnmatchFilter;
	BOOL m_bUseRegExp;
	BOOL m_bCaseSense;
	BOOL m_bIncludeFolder;

	CFilterSetting()
		: m_bUnmatchFilter(FALSE)
		, m_bUseRegExp(FALSE)
		, m_bCaseSense(FALSE)
		, m_bIncludeFolder(FALSE)
	{
		m_strFilter = gszAst;
	}
	CFilterSetting(BOOL bUseRegExp)
		: m_bUnmatchFilter(FALSE)
		, m_bCaseSense(FALSE)
		, m_bIncludeFolder(FALSE)
	{
		m_strFilter = gszAst;
		m_bUseRegExp = bUseRegExp;
	}

	void Clear()
	{
		m_strName.Empty();
		m_strFilter = gszAst;
		m_bUnmatchFilter = FALSE;
		m_bUseRegExp = FALSE;
		m_bCaseSense = FALSE;
		m_bIncludeFolder = FALSE;
	}

	_inline BOOL isDefault() const
	{
		return m_strFilter.Compare(gszAst) == 0;
	}

	void Set(LPCTSTR pszName, LPCTSTR pszFilter, BOOL bUnmatchFilter = FALSE, BOOL bUseRegExp = FALSE, BOOL bCaseSense = FALSE, BOOL bIncludeFolder = FALSE)
	{
		m_strName = pszName;
		m_strFilter = pszFilter;
		m_bUnmatchFilter = bUnmatchFilter;
		m_bUseRegExp = bUseRegExp;
		m_bCaseSense = bCaseSense;
		m_bIncludeFolder = bIncludeFolder;
	}

	void CopyFrom(const CFilterSetting& src)
	{
		if (&src != this) {
			m_strName        = src.m_strName;
			m_strFilter      = src.m_strFilter;
			m_bUnmatchFilter = src.m_bUnmatchFilter;
			m_bUseRegExp     = src.m_bUseRegExp;
			m_bCaseSense     = src.m_bCaseSense;
			m_bIncludeFolder = src.m_bIncludeFolder;
		}
	}
	_inline CFilterSetting& operator=(const CFilterSetting& src) { CopyFrom(src); return *this; }
	_inline BOOL operator ==(const CFilterSetting& fs) const
			{ return ((m_strName        == fs.m_strName) &&
					  (m_strFilter      == fs.m_strFilter) &&
					  (m_bUnmatchFilter == fs.m_bUnmatchFilter) &&
					  (m_bUseRegExp     == fs.m_bUseRegExp) &&
					  (m_bCaseSense     == fs.m_bCaseSense) &&
					  (m_bIncludeFolder == fs.m_bIncludeFolder)); }
	_inline BOOL operator !=(const CFilterSetting& fs) const
			{ return ((m_strName        != fs.m_strName) ||
					  (m_strFilter      != fs.m_strFilter) ||
					  (m_bUnmatchFilter != fs.m_bUnmatchFilter) ||
					  (m_bUseRegExp     != fs.m_bUseRegExp) ||
					  (m_bCaseSense     != fs.m_bCaseSense) ||
					  (m_bIncludeFolder != fs.m_bIncludeFolder)); }

	_inline BOOL isFilterMatched(const CFilterSetting& fs) const
	{
		return ((m_strFilter == fs.m_strFilter) &&
			(m_bUnmatchFilter == fs.m_bUnmatchFilter) &&
			(m_bUseRegExp == fs.m_bUseRegExp) &&
			(m_bCaseSense == fs.m_bCaseSense) &&
			(m_bIncludeFolder == fs.m_bIncludeFolder));
	}

	int Save(IStream* pStrm) const
	{
		if (!pStrm) return -1;

		// magic
		ULONG len;
		DWORD dw = FSMAGIC;
		pStrm->Write(&dw, sizeof(dw), &len);

		// flags
		dw = 0;
		if (m_bUnmatchFilter)		dw |= 0x00000001;
		if (m_bUseRegExp)			dw |= 0x00000002;
		if (m_bCaseSense)			dw |= 0x00000004;
		if (m_bIncludeFolder)		dw |= 0x00000008;
		pStrm->Write(&dw, sizeof(dw), &len);

		// name
		dw = (DWORD)m_strName.GetLength() + 1;
		pStrm->Write(&dw, sizeof(dw), &len);
		if (dw == 1) {
			TCHAR ch = 0;
			pStrm->Write(&ch, sizeof(ch), &len);
		}
		else
			pStrm->Write((LPCTSTR)m_strName, dw * sizeof(TCHAR), &len);

		// filter
		dw = (DWORD)m_strFilter.GetLength() + 1;
		pStrm->Write(&dw, sizeof(dw), &len);
		if (dw == 1) {
			TCHAR ch = 0;
			pStrm->Write(&ch, sizeof(ch), &len);
		}
		else
			pStrm->Write((LPCTSTR)m_strFilter, dw * sizeof(TCHAR), &len);

		// サイズ切り捨て
		LARGE_INTEGER mov;
		ULARGE_INTEGER pos;
		mov.QuadPart = 0;
		pStrm->Seek(mov, STREAM_SEEK_CUR, &pos);
		pStrm->SetSize(pos);

		return (int)pos.QuadPart;
	}

	BOOL Load(IStream* pStrm)
	{
		if (!pStrm) return FALSE;

		// magic
		ULONG len = 0;
		DWORD dw = 0;
		pStrm->Read(&dw, sizeof(dw), &len);
		if (len != sizeof(DWORD) || dw != FSMAGIC) return FALSE;

		// flags
		pStrm->Read(&dw, sizeof(dw), &len);
		m_bUnmatchFilter		= ((dw & 0x00000001) != 0);
		m_bUseRegExp			= ((dw & 0x00000002) != 0);
		m_bCaseSense			= ((dw & 0x00000004) != 0);
		m_bIncludeFolder		= ((dw & 0x00000008) != 0);

		// name
		m_strName.Empty();
		pStrm->Read(&dw, sizeof(dw), &len);
		if (dw == 1) {
			TCHAR ch;
			pStrm->Read(&ch, sizeof(ch), &len);
		}
		else {
			LPTSTR p = m_strName.GetBuffer(dw);
			pStrm->Read(p, dw * sizeof(TCHAR), &len);
			m_strName.ReleaseBuffer();
		}

		// filter
		m_strFilter.Empty();
		pStrm->Read(&dw, sizeof(dw), &len);
		if (dw == 1) {
			TCHAR ch;
			pStrm->Read(&ch, sizeof(ch), &len);
		}
		else {
			LPTSTR p = m_strFilter.GetBuffer(dw);
			pStrm->Read(p, dw * sizeof(TCHAR), &len);
			m_strFilter.ReleaseBuffer();
		}

		return TRUE;
	}

	_inline LPCTSTR GetName() const { return (LPCTSTR)m_strName; }
	_inline LPCTSTR GetFilter() const { return (LPCTSTR)m_strFilter; }

	LPCTSTR label() const
	{
		CString str;
		JSArray jsa;
		if (m_bUnmatchFilter) { jsa.push(L"!"); }
		if (m_bUseRegExp) { jsa.push(L"//"); }
		if (m_bCaseSense) { jsa.push(L"aA"); }
		if (jsa.GetSize() > 0) {
			str.Format(L"[%s] ", (jsa.join(L"+")).GetString());
		}
		str += m_strFilter;
		return str;
	}
	LPCTSTR menuLabel() const
	{
		CString str = m_strFilter;
		JSArray jsa;
		if (m_bUnmatchFilter) { jsa.push(_T("!")); }
		if (m_bUseRegExp) { jsa.push(_T("//")); }
		if (m_bCaseSense) { jsa.push(_T("aA")); }
		if (jsa.GetSize() > 0) {
			str.AppendFormat(L"\t[%s]", (jsa.join(_T("+"))).GetString());
		}
		return str;
	}
};

////////////////////////////////////////////////////////////////////////
//

class CFilterSettingList : public CSimpleArray<CFilterSetting>
{
private:
	enum {
		fUnmatchFilter = 0x0001,
		fUseRegExp = 0x0002,
		fCaseSense = 0x0004,
		fIncludeFolder = 0x0008
	};

public:

/* ---
	int Load(LPCTSTR pszSection)
	{
		bool GetRegApp(CRegKey& app);

		int num = 0;
		CRegKey reg;
		if (GetRegApp(reg)) {
			CComPtr<IStream> pstrm;
			CRegKey sec;
			if (sec.Open(reg, pszSection, KEY_READ) == ERROR_SUCCESS) {
				CString name;
				int index = 0;
				while (1) {
					name.Format(_T("%d"), index++);
					ULONG len = 0;
					if (sec.QueryValue(name, NULL, NULL, &len) != ERROR_SUCCESS)
						break;

					pstrm = SHOpenRegStream2(reg, pszSection, name, STGM_READ);
					if (pstrm) {
						CFilterSetting fs;
						if (fs.Load(pstrm)) {
							Add(fs);
							num++;
						}
					}
				}
			}
		}
		return num;
	}

	void Save(LPCTSTR pszSection)
	{
		CRegKey reg;
		if (GetRegApp(reg)) {
			// 存在する値をすべて削除
			SHDeleteKey(reg, pszSection);

			CRegKey sec;
			if (sec.Create(reg, pszSection) == ERROR_SUCCESS) {
				sec.Close();

				CString name;
				int cnt = GetSize();
				CComPtr<IStream> pstrm;
				for (int i = 0; i < cnt; i++) {
					name.Format(_T("%d"), i);
					pstrm = SHOpenRegStream2(reg, pszSection, name, STGM_WRITE);
					if (pstrm.p != NULL) {
						CFilterSetting fs = (*this)[i];
						fs.Save(pstrm);
					}
				}
			}
		}
	}
--- */

	void GetDefaultXmlFile(CString& xmlfile)
	{
		GetAppDataFolder(xmlfile, _T("filtersettings.xml"));
	}

	inline int wsz2utf(const WCHAR* wsz, char* utf, int cchMax) { return WideCharToMultiByte(CP_UTF8, 0, wsz, -1, utf, cchMax, NULL, NULL); }
	inline int utf2wsz(const char* utf, WCHAR* wsz, int cchMax) { return MultiByteToWideChar(CP_UTF8, 0, utf, -1, wsz, cchMax); }
	int utf2str(const char* utf, CString& str)
	{
		str.Empty();
		int len = lstrlenA(utf) + 1;
		len = utf2wsz(utf, str.GetBuffer(len), len);
		str.ReleaseBuffer();
		return len;
	}

	bool LoadXML()
	{
		CString xmlfile;
		GetDefaultXmlFile(xmlfile);

		char xml[MAX_PATH * 2];
		WideCharToMultiByte(CP_ACP, 0, xmlfile, -1, xml, ARRAYSIZE(xml), NULL, NULL);

		TiXmlDocument xmlDoc;
		if (!xmlDoc.LoadFile( xml )) return false;

		TiXmlNode* pDoc = xmlDoc.ToDocument();
		if (pDoc) {
			TiXmlElement* pRoot = pDoc->FirstChildElement("FilterSettings");
			if (pRoot) {
				TiXmlElement* pItem = pRoot->FirstChildElement("item");
				CFilterSetting fs;
				CString str;
				while (pItem) {
					fs.Clear();
					if (utf2str(pItem->Attribute("name"), fs.m_strName) > 0 &&
						utf2str(pItem->Attribute("filter"), fs.m_strFilter) > 0) {
						int value = 0;
						pItem->Attribute("option", &value);
						if (value & fUnmatchFilter)	fs.m_bUnmatchFilter = TRUE;
						if (value & fUseRegExp)		fs.m_bUseRegExp = TRUE;
						if (value & fCaseSense)		fs.m_bCaseSense = TRUE;
						if (value & fIncludeFolder)	fs.m_bIncludeFolder = TRUE;
						Add(fs);
					}
					pItem = pItem->NextSiblingElement("item");
				}
			}
		}
		return true;
	}

	bool SaveXML()
	{
		char utf[MAX_PATH * 2];
		CString str;

		// Save XML
		TiXmlDocument xmlDoc;
		xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

		// --------------------------
		// XML directory header
		// --------------------------
		TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "FilterSettings" ) );
		TiXmlElement* pRoot = pDoc->ToElement();

		// XML item
		int cnt = GetSize();
		TiXmlElement* pElement;
		for (int i = 0; i < cnt; i++) {
			CFilterSetting& fs = (*this)[i];

			TiXmlNode* pItem = pRoot->InsertEndChild( TiXmlElement( "item" ) );
			pElement = pItem->ToElement();

			// フィルタ名
			wsz2utf(fs.m_strName, utf, ARRAYSIZE(utf));
			pElement->SetAttribute( "name", utf );

			// フィルタ
			wsz2utf(fs.m_strFilter, utf, ARRAYSIZE(utf));
			pElement->SetAttribute( "filter", utf );

			// オプション
			int value = 0;
			if (fs.m_bUnmatchFilter)	value |= fUnmatchFilter;
			if (fs.m_bUseRegExp)		value |= fUseRegExp;
			if (fs.m_bCaseSense)		value |= fCaseSense;
			if (fs.m_bIncludeFolder)	value |= fIncludeFolder;
			pElement->SetAttribute( "option", value );
		}

		// Save XML
		CString xmlfile;
		GetDefaultXmlFile(xmlfile);
		WideCharToMultiByte(CP_ACP, 0, xmlfile, -1, utf, ARRAYSIZE(utf), NULL, NULL);
		// 書き出し可能か？
		if (!CheckFileWritable(utf)) {
			;	// とりあえずreadonly属性がついていれば解除される
		}
		return xmlDoc.SaveFile(utf);
	}

	int FindName(LPCTSTR pszName)
	{
		CFilterSetting fs;
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			fs = (*this)[i];
			if (!lstrcmpi(fs.m_strName, pszName))
				return i;
		}
		return -1;
	}
};

#endif		// __FILTERSETTING_H__

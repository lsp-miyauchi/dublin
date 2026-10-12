///////////////////////////////////////////////////////////////////////////
// fvmapico.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "fv7.h"
#include "mapicon.h"

//////////////////////////////////////////////////////////////////////////
//
class CIconEntry
{
public:
	CIconEntry(HICON hicl, HICON hics) : m_hicl(hicl), m_hics(hics) { }
	~CIconEntry() { if (m_hicl) DestroyIcon(m_hicl);
					if (m_hics) DestroyIcon(m_hics); }
	HICON m_hicl;
	HICON m_hics;
};

//////////////////////////////////////////////////////////////////////////
//

void BuildMapIconFileName(CString& str)
{
	GetAppDataFolder(str, _T("filevisor7.mapicon"));
}

bool SaveMapIcon(const CSimpleArray<CIconMap*>& array, LPCTSTR pszFileName)
{
	char ssz[MAX_PATH * 2];
	// Save XML
	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

	// --------------------------
	// XML directory header
	// --------------------------
	TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "mapicon" ) );
	TiXmlElement* pRoot = pDoc->ToElement();

	// XML item
	TiXmlElement* pElement;
	int cnt = array.GetSize();
	for (int i = 0; i < cnt; i++) {
		CIconMap* pim = array[i];
		TiXmlNode* pItem = pRoot->InsertEndChild( TiXmlElement( "item" ) );
		pElement = pItem->ToElement();
		// extension
		WideCharToMultiByte(CP_UTF8, 0, pim->m_ext, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
		pElement->SetAttribute( "extension", ssz );
		// file
		WideCharToMultiByte(CP_UTF8, 0, pim->m_file, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
		pElement->SetAttribute( "file", ssz );
		// index
		pElement->SetAttribute( "index", pim->m_index );
	}
	// Save XML
	WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
	// 書き出し可能か？
	if (!CheckFileWritable(ssz)) {
		;	// とりあえずreadonly属性がついていれば解除される
	}
	return xmlDoc.SaveFile(ssz);
}

int LoadMapIcon(CSimpleArray<CIconMap*>& array, LPCTSTR pszFileName)
{
	char xml[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, xml, ARRAYSIZE(xml), NULL, NULL);

	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile( xml )) return 0;

	int entries = 0;
	WCHAR wsz[MAX_PATH * 2];
	int val;
	const char* psz;
	TiXmlNode* pDoc = xmlDoc.ToDocument();
	if (pDoc) {
		TiXmlElement* pRoot = pDoc->FirstChildElement("mapicon");
		if (pRoot) {
			TiXmlElement* pItem = pRoot->FirstChildElement("item");
			while (pItem) {
				// ext
				psz = pItem->Attribute("extension");
				if (psz != NULL) {
					CIconMap* pim = new CIconMap;
					array.Add(pim);
					MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
					pim->m_ext = wsz;
					pim->m_ext.MakeUpper();
					// file
					psz = pItem->Attribute("file");
					if (psz) {
						MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
						pim->m_file = wsz;
					}
					// index
					if (pItem->Attribute("index", &val) == NULL)
						val = 0;
					pim->m_index = val;
					entries++;
				}
				pItem = pItem->NextSiblingElement("item");
			}
		}
	}
	return entries;
}

int RestoreMapIcon(CSimpleArray<CIconMap*>& array)
{
	// 外部xmlファイルから読み込み
	CString str;
	BuildMapIconFileName(str);
	if (_pathfileexists(str))
		return LoadMapIcon(array, str);

	// ファイルが無ければレジストリ
	int entries = 0;
	CSimpleArray<CString> list;
	int cnt = GetRegEnumValue(IDS_SEC_MAPICON, list);
	for (int i = 0; i < cnt; i++) {
		list[i].MakeUpper();
		if (GetRegText(IDS_SEC_MAPICON, list[i].GetString(), str) > 0) {
			int index = 0;
			LPTSTR p = StrRChr(str, NULL, ',');
			if (p) {
				*p++ = 0;
				index = StrToInt(p);
			}
			CIconMap* pim = new CIconMap(list[i], str, index);
			array.Add(pim);
			entries++;
		}
	}
	return entries;
}


/////////////////////////////////////////////////////////////////////////////
// CSelectIconDlg メッセージ ハンドラ

BOOL CSelectIconDlg::OnInitDialog(LPARAM lParam) 
{
	DlgResize_Init();

	CenterWindow();
	m_list = GetDlgItem(IDC_LIST);

	CWindow ctrl = GetDlgItem(IDC_EXT);
	LPCTSTR ext = m_im.GetExtension();
	if (ext) {
		// 書き換え禁止
		ctrl.SetWindowText(ext);
		ctrl.ModifyStyle(0, ES_READONLY);
	}
	if (m_im.GetIconIndex() < 0) {
		TCHAR buf[MAX_PATH];
		GetModuleFileName(GetModuleHandle(gszShell32), buf, ARRAYSIZE(buf));
		m_im.SetIconPath(buf, 0);
	}

	SetDlgItemText(IDC_EDIT, m_im.GetIconPath());
	IconStamp();
	OnCheck();

	if (ext)
		m_list.SetFocus();
	else
		ctrl.SetFocus();
	return FALSE;
}

void CSelectIconDlg::OnOK()
{
	CString ext;
	if (GetDlgItemText(IDC_EXT, ext) > 0) {
		while (ext[0] == '.')
			ext.Delete(0);
	}
	if (ext.GetLength() <  1) {
		MessageBeep(0);
		GetDlgItem(IDC_EXT).SetFocus();
		return;
	}

	CString path;
	if (GetDlgItemText(IDC_EDIT, path) < 1 || !_pathfileexists(path)) {
		MessageBeep(0);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}

	int index = (int)m_list.SendMessage(DLM_GETCURSEL);
	if (index < 0) {
		MessageBeep(0);
		m_list.SetFocus();
		return;
	}

	m_im.Initialize(ext, path, index);

	EndDialog(IDOK);
}

void CSelectIconDlg::OnCheck()
{
	int dx = IsDlgButtonChecked(IDC_CHECK) ? 16 + 8 : 32 + 8;
	m_list.SendMessage(DLM_SETITEMSIZE, 0, MAKELPARAM(dx, dx));
}

void CSelectIconDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (buf[0])
		PathRemoveFileSpec(buf);
	CString str;
	if (FileBrowse(TRUE, m_hWnd, IDS_FILTER_ICON, str, buf)) {
		lstrcpyn(psgt->pszText, str, psgt->cchTextMax);
		m_im.SetIconPath(str, 0);
		IconStamp();
		*pResult = TRUE;
	}
}

void CSelectIconDlg::OnDrawItem(int /*id*/, DRAWITEMSTRUCT* pdi)
{
	bool active = (::GetFocus() == pdi->hwndItem);

	CRect rc = pdi->rcItem;
	int index = (int)pdi->itemData;
	if (index >= 0 && pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		// 選択状態の場合
		if (pdi->itemState & ODS_SELECTED)
			SetBkColor(pdi->hDC, GetSysColor(active ? COLOR_HIGHLIGHT : COLOR_3DFACE));
		else
			SetBkColor(pdi->hDC, GetSysColor(COLOR_WINDOW));
		ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

		HICON hic;
		int dx;
		if (IsDlgButtonChecked(IDC_CHECK)) {
			dx = 16;
			hic = m_iconSmall[index];
		}
		else {
			dx = 32;
			hic = m_iconLarge[index];
		}
		DrawIconEx(pdi->hDC,
					rc.left + (rc.Width() - dx) / 2,
					rc.top + (rc.Height() - dx) / 2,
					hic, dx, dx, 0, NULL, DI_NORMAL);
	}
	if (active && pdi->itemState & ODS_FOCUS)
		DrawFocusRect(pdi->hDC, &pdi->rcItem);
}

void CSelectIconDlg::IconStamp()
{
	m_list.SendMessage(DLM_RESETCONTENT);
	m_iconSmall.DeleteAll();
	m_iconLarge.DeleteAll();

	LPCTSTR path = m_im.GetIconPath();
	if (path) {
		m_list.SetRedraw(FALSE);
		int cnt = (int)(ULONG_PTR)ExtractIcon(RESINST, path, (UINT)-1);
		if (cnt > 0) {
			m_iconSmall.SetSize(cnt);
			m_iconLarge.SetSize(cnt);
			cnt = ExtractIconEx(path, 0, m_iconLarge.GetData(), m_iconSmall.GetData(), cnt);
			for (int i = 0; i < cnt; i++)
				m_list.SendMessage(DLM_ADDITEM, 0, (LPARAM)i);
			m_list.SendMessage(DLM_SETCURSEL, m_im.GetIconIndex());
		}
		m_list.SetRedraw(TRUE);
		m_list.Invalidate(TRUE);
	}
}

void CSelectIconDlg::OnUpdatePath()
{
	CString str;
	if (GetDlgItemText(IDC_EDIT, str) > 0 && _pathfileexists(str) && str.CompareNoCase(m_im.GetIconPath())) {
		m_im.SetIconPath(str, -1);
		IconStamp();
	}
}

void CSelectIconDlg::OnHelpCall()
{
	fvApp.HelpTopic(NULL, HB_DLG, IDD);
}

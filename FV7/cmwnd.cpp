// CmWnd.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "cdll.h"
#include "data.h"
#include "idlist.h"
#include "fileitem.h"
#include "dragdrop.h"
#include "shellexec.h"
#include "cmwnd.h"
#include "func.h"

/////////////////////////////////////////////////////////////////////////////
//

void __cdecl qsort_ex(void *base, size_t n, size_t size,
	int (__cdecl *comp)(const void*, const void*, const void*), const void* param);

/////////////////////////////////////////////////////////////////////////////
// serialize CCmItemArray

void XmlWriteIDListCore(TiXmlElement* pElement, LPCITEMIDLIST pidl, LPCTSTR pszName)
{
	// ITEMIDLIST
	int idlsize = ::ILGetSize(pidl);
	pElement->SetAttribute( "size", idlsize );
	// crc16
	int crc = lccCalcCrc16((BYTE*)pidl, idlsize);
	pElement->SetAttribute( "crc", crc );
	// idlist
	char* data = b64Encode((const BYTE*)pidl, idlsize);
	pElement->SetAttribute( "data", data );
	free(data);
	// name
	if (pszName && *pszName) {
		char ssz[MAX_PATH * 2];
		WideCharToMultiByte(CP_UTF8, 0, pszName, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
		pElement->SetAttribute( "name", ssz );
	}
}

void WriteCmItemCore(TiXmlElement* pElement, CCmItem* pci)
{
	XmlWriteIDListCore(pElement, pci->m_idl, pci->m_name);
	// guid
	CString str;
	pci->GetGuid(str);
	CStringA astr = str;
	pElement->SetAttribute( "guid", astr.GetString() );
}

LPITEMIDLIST XmlLoadIDListCore(TiXmlElement* pElement, CString& str)
{
	LPITEMIDLIST pidl = NULL;
	// datasize
	int idlsize; if (pElement->QueryIntAttribute("size", &idlsize) != TIXML_SUCCESS) idlsize = 0;
	// crc
	int crc; if (pElement->QueryIntAttribute("crc", &crc) != TIXML_SUCCESS) crc = 0;
	// idlist
	int size;
	BYTE* data = b64Decode(pElement->Attribute("data"), &size);
	if (data) {
		// サイズとcrcのチェック
		if (size >= idlsize && lccCalcCrc16(data, idlsize) == crc) {
			pidl = (LPITEMIDLIST)::CoTaskMemAlloc(size);
			if (pidl) {
				memcpy(pidl, data, size);
				// name
				const char* psz = pElement->Attribute("name");
				if (psz) {
					MultiByteToWideChar(CP_UTF8, 0, psz, -1, str.GetBuffer(MAX_PATH), MAX_PATH);
					str.ReleaseBuffer();
				}
				else
					str.Empty();
			}
		}
		free(data);
	}
	return pidl;
}

CCmItem* LoadCmItemCore(TiXmlElement* pElement)
{
	CString str;
	CCmItem* pci = NULL;
	LPITEMIDLIST pidl = XmlLoadIDListCore(pElement, str);
	if (pidl) {
		pci = new CCmItem(pidl, str);
		// guid
		str = pElement->Attribute("guid");
		if (!str.IsEmpty())
			pci->SetGuid(str);
		::CoTaskMemFree(pidl);
	}
	return pci;
}

bool SaveCmItemArray(LPCTSTR pszFileName, CCmItemArray& array, int maxEntry)
{
	char ssz[MAX_PATH * 2];

	// Save XML
	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

	// --------------------------
	// XML directory header
	// --------------------------
	TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "cmitems" ) );
	TiXmlElement* pRoot = pDoc->ToElement();

	// XML item
	int cnt = array.GetSize();
	if (maxEntry > 0)
		cnt = min(cnt, maxEntry);
	for (int i = 0; i < cnt; i++) {
		TiXmlNode* pItem = pRoot->InsertEndChild( TiXmlElement( "item" ) );
		WriteCmItemCore(pItem->ToElement(), array[i]);
	}
	// Save XML
	WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
	// 書き出し可能か？
	if (!CheckFileWritable(ssz)) {
		;	// とりあえずreadonly属性がついていれば解除される
	}
	return xmlDoc.SaveFile(ssz);
}

int LoadCmItemArray(LPCTSTR pszFileName, CCmItemArray& array)
{
	char xml[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, xml, ARRAYSIZE(xml), NULL, NULL);

	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile( xml )) return -1;

	int index = 0;
	TiXmlNode* pDoc = xmlDoc.ToDocument();
	if (pDoc) {
		TiXmlElement* pRoot = pDoc->FirstChildElement("cmitems");
		if (pRoot) {
			TiXmlElement* pItem = pRoot->FirstChildElement("item");
			while (pItem) {
				CCmItem* pci = LoadCmItemCore(pItem);
				if (pci) {
					pci->m_id = index++;
					array.Add(pci);
				}
				pItem = pItem->NextSiblingElement("item");
			}
		}
	}
	return index;
}


/////////////////////////////////////////////////////////////////////////////
// CCmWnd

CCmWnd::CCmWnd(HMENU hMenu, UINT nFolderID, BOOL bRetrieveTree)
{
	PIDLIST_ABSOLUTE pidl;
	if (SHGetFolderLocation(m_hWnd, nFolderID, NULL, 0, &pidl) == S_OK) {
		Initialize(hMenu, pidl, bRetrieveTree);
		CoTaskMemFree(pidl);
	}
}

CCmWnd::CCmWnd(HMENU hMenu, LPITEMIDLIST pidl, BOOL bRetrieveTree)
{
	Initialize(hMenu, pidl, bRetrieveTree);
}

CCmWnd::~CCmWnd()
{
	int cnt = m_arrVistaBitmaps.GetSize();
	for (int i = 0; i < cnt; i++) {
		HBITMAP hbm = m_arrVistaBitmaps[i];
		if (hbm)
			::DeleteObject(hbm);
	}
	m_arrVistaBitmaps.RemoveAll();
	m_array.DeleteAll();
}

void CCmWnd::Initialize(HMENU hMenu, LPITEMIDLIST pidl, BOOL bRetrieveTree)
{
	m_idl.Clone(pidl);
	m_nNextMenuID = 1;
	m_hMenu = hMenu;
	m_bRetrieveTree = bRetrieveTree;
	CComPtr<IShellFolder> psf = m_idl.GetShellFolder();
	if (psf.p != NULL) {
		AddFolderMenu(psf, m_idl, m_hMenu);
	}
}



/////////////////////////////////////////////////////////////////////////////
// CCmWnd message handlers

void CCmWnd::OnInitMenuPopup(HMENU hMenu, UINT nIndex, BOOL bSysMenu) 
{
	if (bSysMenu) return;

	CCmMenuItem* pci = FindMenu(hMenu);
	if (pci && !pci->m_bInitMenu) {
		CComPtr<IShellFolder> psf = (IShellFolder*)pci->BindToObject(IID_IShellFolder);
		if (psf.p != NULL) {
			AddFolderMenu(psf, pci->m_idl, pci->m_menu);
			pci->m_bInitMenu = true;
		}
	}

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_BACKGROUND;
	mi.hbrBack = _GCDATA.GetMenuBackgroundBrush();
	::SetMenuInfo(hMenu, &mi);
}


static int __cdecl cmp_folder(const void* arg1, const void* arg2, const void* plm)
{
	CCmMenuItem* p1 = *(CCmMenuItem**)arg1;
	CCmMenuItem* p2 = *(CCmMenuItem**)arg2;
	IShellFolder* psf = (IShellFolder*)plm;
	HRESULT hr = psf->CompareIDs(0, p1->SkipToLastItem(), p2->SkipToLastItem());
	return (short)SCODE_CODE(GetScode(hr));
}

void CCmWnd::AddFolderMenu(IShellFolder* psf, LPITEMIDLIST pidlFq, HMENU hmenu)
{
	CWaitCursor wc;

	UINT flg;
	LPCTSTR pszMenuText;

	CCmMenuItem* pci;
	int cyLine = (GetSystemMetrics(SM_CYSCREEN) / GetSystemMetrics(SM_CYMENU)) - 10;
	int cnt = 0;
	if (psf) {
		CComPtr<IEnumIDList> pe;
		if (psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_FASTITEMS, &pe) == S_OK) {
			LPITEMIDLIST pidl;
			CSimpleArray<CCmMenuItem*> array;
			while (pe->Next(1, &pidl, NULL) == NOERROR) {
				CIDListPtr idlTarget;
				if (idlTarget.Combine(pidlFq, pidl)) {
					pci = new CCmMenuItem(idlTarget);
					array.Add(pci);

					if (pci->isFolder())
						pci->m_menu.CreatePopupMenu();

					idlTarget.Free();
				}
				CoTaskMemFree(pidl);
			}

			if (array.GetSize() > 1)
				qsort_ex(array.GetData(), array.GetSize(), sizeof(CCmItem*), cmp_folder, psf);

			// メニュー列挙
			UINT id;
			for (int i = 0; i < array.GetSize(); i++) {
				pci = array[i];
				flg = MF_STRING;
				pszMenuText = pci->m_name;
				if (m_bRetrieveTree && pci->m_menu.IsMenu()) {
					flg |= MF_POPUP;
					id = HandleToULong(pci->m_menu);
				}
				else {
					pci->m_id = m_nNextMenuID++;
					id = pci->m_id;
				}

				m_array.Add(pci);
				if (cnt > 0 && (cnt % cyLine) == 0)
					flg |= MF_MENUBARBREAK;
				AppendMenu(hmenu, flg, id, pszMenuText);

				HBITMAP hbm = CreateBitmapFromImageList(m_hImage, pci->GetImageIndex());
				if (hbm) {
					SetBitmapToMenuItem(hmenu, id, hbm);
					m_arrVistaBitmaps.Add(hbm);
				}
				cnt++;
			}
			array.RemoveAll();
		}
	}

	if (cnt == 0) {
		pci = new CCmMenuItem();
		pci->m_name.LoadString(IDS_UNDEFINED);
		pszMenuText = pci->m_name;
		flg = MF_STRING;
		AppendMenu(hmenu, flg | MF_GRAYED, 0, pszMenuText);
		m_array.Add(pci);
	}
}

BOOL CCmWnd::AddMenuItem(HMENU hmenu, CCmMenuItem* pci)
{
	if (!pci || !IsMenu(hmenu)) return FALSE;

	pci->m_id = m_nNextMenuID++;
	m_array.Add(pci);

	LPCTSTR pszMenuText;
	pszMenuText = pci->m_name;
	UINT flg = MF_STRING;

	int cyLine = (GetSystemMetrics(SM_CYSCREEN) / GetSystemMetrics(SM_CYMENU)) - 10;
	int cnt = m_array.GetSize();
	if (cnt > 0 && (cnt % cyLine) == 0)
		flg |= MF_MENUBARBREAK;
	AppendMenu(hmenu, flg, pci->m_id, pszMenuText);
	HBITMAP hbm = CreateBitmapFromImageList(m_hImage, pci->GetImageIndex());
	if (hbm) {
		SetBitmapToMenuItem(hmenu, pci->m_id, hbm);
		m_arrVistaBitmaps.Add(hbm);
	}
	return TRUE;
}

CCmMenuItem* CCmWnd::FindItem(int id)
{
	int cnt = m_array.GetSize();
	for (int i = 0; i < cnt; i++) {
		if (m_array[i]->m_id == id)
			return m_array[i];
	}
	return NULL;
}

CCmMenuItem* CCmWnd::FindMenu(HMENU hmenu)
{
	if (hmenu) {
		int cnt = m_array.GetSize();
		for (int i = 0; i < cnt; i++) {
			if (m_array[i]->m_menu == hmenu)
				return m_array[i];
		}
	}
	return NULL;
}


// CmWnd.h : header file
//

#pragma once
#ifndef __CMWND_H__
#define __CMWND_H__

#ifndef MAX_URL_LENGTH
	#define MAX_URL_LENGTH			(2048 + 32 + 8)
#endif

#include "shellexec.h"

#ifndef ISAPLHA
	#define ISALPHA(a)		(((a)>='a'&&(a)<='z')||((a)>='A'&&(a)<='Z'))
#endif

BOOL getlinkinfo(LPCTSTR pszName, CString& target);
HBITMAP CreateBitmapFromImageList(HIMAGELIST him, int iImage);

/////////////////////////////////////////////////////////////////////////////
//

class CCmItem
{
public:
	IShellItem* m_psi;
	CIDListPtr m_idl;
	CString m_name;
	int m_id;
	CBitmap m_bmIcon;
	GUID m_guid;
	__time64_t m_time;

protected:
	CString m_path;
	SFGAOF m_attr;
	int m_iImage;

public:
	CCmItem() : m_psi(NULL)
	{
		_init_member();
	}

	CCmItem(PCIDLIST_ABSOLUTE pidlFq, LPCTSTR pszName = NULL) : m_psi(NULL)
	{
		_init_member();
		if (pszName && *pszName) m_name = pszName;
		if (pidlFq) {
			m_idl.Clone(pidlFq);
			SHCreateItemFromIDList(pidlFq, IID_PPV_ARGS(&m_psi));
			if (m_name.IsEmpty())
				idl_GetName(m_idl, SHGDN_NORMAL, m_name);
		}
	}

	CCmItem(IShellItem* psi, LPCTSTR pszName = NULL) : m_psi(NULL)
	{
		_init_member();
		if (pszName && *pszName) m_name = pszName;
		if (psi) {
			m_psi = psi;
			m_psi->AddRef();
			m_idl.GetFromShellItem(m_psi);
			if (m_name.IsEmpty()) {
				CMemTask name;
				if (m_psi->GetDisplayName(SIGDN_NORMALDISPLAY, (PWSTR*)&name) == S_OK) {
					m_name = name;
				}
			}
		}
	}

	CCmItem(const CCmItem& src) : m_psi(NULL)
	{
		copyfrom(src);
	}

	~CCmItem()
	{
		if (m_psi) m_psi->Release();
	}

	void _init_member()
	{
		m_id = 0;
		m_iImage = -1;
		if (m_psi) { m_psi->Release(); m_psi = NULL; }
		m_idl.Free();
		m_name.Empty();
		m_path.Empty();
		m_attr = 0xffffffff;
		m_guid = GUID_NULL;
		m_time = 0;
	}

	const CCmItem& copyfrom(const CCmItem& src)
	{
		_init_member();
		m_idl.Clone(src.m_idl);
		m_psi = src.m_psi; if (m_psi) m_psi->AddRef();
		m_name = src.m_name.GetString();
		m_path = src.m_path.GetString();
		m_iImage = src.m_iImage;
		m_attr = src.m_attr;
		m_guid = src.m_guid;
		m_time = src.m_time;
		return *this;
	}

	const CCmItem& operator =(const CCmItem& src)
	{
		return copyfrom(src);
	}

	__inline bool operator==(const CCmItem& item) const { return (m_idl == item.m_idl); }

	IShellFolder* BindToParent(LPCITEMIDLIST* ppidl = NULL)
	{
		LPCITEMIDLIST pidl = NULL;
		IShellFolder* psf = m_idl.BindToParent(&pidl);
		if (!psf) {
			SHGetDesktopFolder(&psf);
			pidl = m_idl;
		}
		if (ppidl) *ppidl = pidl;
		return psf;
	}

	__inline LPCITEMIDLIST SkipToLastItem() { return m_idl.FindLastID(); }
	__inline LPVOID GetUIObjectOf(HWND hwnd, REFIID riid) { return m_idl.GetUIObjectOf(hwnd, riid); }

	void* BindToObject(const IID& riid)
	{
		return m_idl.BindToObject(riid);
	}

	SFGAOF GetAttributesOf()
	{
		if (m_attr == 0xffffffff) {
			if (!m_idl.isNull()) {
				SHFILEINFO sfi;
				// ※※ SFGAO_HASSUBFOLDER はチェックしない ※※
				sfi.dwAttributes = 0x70000000 | SFGAO_LINK | SFGAO_CAPABILITYMASK;
				m_idl.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
				m_attr = sfi.dwAttributes;
			}
			else
				m_attr = 0;
		}
		return m_attr;
	}

	__inline BOOL Invoke(HWND hwnd, LPCTSTR lpVerb) { return InvokeIDList(hwnd, m_idl, lpVerb); }
	__inline bool isFolder() { return ((GetAttributesOf() & SFGAO_FOLDER) != 0); }
	__inline bool isFileSystem() { return ((GetAttributesOf() & SFGAO_FILESYSTEM) != 0); }

	int GetImageIndex(bool bNeedBitmap = false)
	{
		if (m_iImage == -1 && !m_idl.isNull()) {
			m_iImage = idl_GetSysIconIndex(m_idl, TRUE);		// 簡易取得
			if (bNeedBitmap && m_iImage >= 0) {
				HIMAGELIST himL, himS;
				Shell_GetImageLists(&himL, &himS);
				m_bmIcon.Attach(CreateBitmapFromImageList(himS, m_iImage));
			}
		}
		return m_iImage;
	}

	void SetImageIndex(int iImage, bool bNeedBitmap = false)
	{
		m_iImage = iImage;
		if (bNeedBitmap && m_iImage >= 0) {
			HIMAGELIST himL, himS;
			Shell_GetImageLists(&himL, &himS);
			m_bmIcon.Attach(CreateBitmapFromImageList(himS, m_iImage));
		}
		else if (!m_bmIcon.IsNull())
			m_bmIcon.DeleteObject();
	}

	BOOL GetParseName(CString& name)
	{
		name.Empty();
		if (!m_idl.isNull()) {
			if (m_path.IsEmpty()) {
				CString str;
				BOOL ok = idl_GetName(m_idl, SHGDN_FORPARSING, str);
				if (ok) {
					SFGAOF attr = GetAttributesOf();
					if ((attr & SFGAO_LINK) && (attr & SFGAO_FOLDER)) {
						CString tmp;
						if (getlinkinfo(str, tmp)) {
							if (!PathIsURL(tmp))
								str = tmp;
						}
					}
					m_path = str;
				}
			}
			name = m_path;
		}
		return !name.IsEmpty();
	}

	void SetPathDisplayName()
	{
		if (m_idl.isNull()) return;

		CString str;
		if (m_idl.isDesktop() || idl_GetName(m_idl, SHGDN_INFOLDER | SHGDN_FORPARSING, str) && str[0] == _T(':'))
			;
		else {
			GetParseName(str);
			// とりあえず高速に処理するために簡易チェックをおこなう
			if (!PathIsUNC(str) && (!ISALPHA(str[0]) || str[1] != _T(':'))) {
				;
			}
			else if (!PathIsRoot(str) || PathIsUNCServer(str))
				m_name = m_path.GetString();
		}
	}

	bool GetGuid(CString& str)
	{
		if (m_guid == GUID_NULL)
			CoCreateGuid(&m_guid);

		str.Empty();
		OLECHAR* clsid;
		if (StringFromCLSID(m_guid, &clsid) == S_OK)
			str = clsid;
		return (!str.IsEmpty());
	}
	bool SetGuid(LPCTSTR pszGuidString)
	{
		if (CLSIDFromString((OLECHAR*)pszGuidString, &m_guid) != NOERROR) {
			m_guid = GUID_NULL;
			return false;
		}
		return true;
	}

	void SetTime(__time64_t t = 0)
	{
		if (t == 0) {
			CTime tm = CTime::GetCurrentTime();
			t = tm.GetTime();
		}
		m_time = t;
	}
	inline __time64_t GetTime() { return m_time; }
	bool GetAsSystemTime(SYSTEMTIME& st)
	{
		CTime tm(m_time);
		return tm.GetAsSystemTime(st);
	}
};

//////////////////////////////////////////////////////////////////////////////
//

class CCmItemArray : public CSimpleArray<CCmItem*>
{
public:
	void DeleteAll()
	{
		CCmItem* pci;
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			pci = (*this)[i];
			if (pci) delete pci;
		}
		RemoveAll();
	}
	CCmItem* LookupGuid(GUID& guid)
	{
		CCmItem* pci;
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			pci = (*this)[i];
			if (pci->m_guid == guid)
				return pci;
		}
		return NULL;
	}
};

bool SaveCmItemArray(LPCTSTR pszFileName, CCmItemArray& array, int maxEntry = 0);
int LoadCmItemArray(LPCTSTR pszFileName, CCmItemArray& array);

/////////////////////////////////////////////////////////////////////////////
// CCmMenuItem

class CCmMenuItem : public CCmItem
{
public:
	bool m_bInitMenu;
	CMenu m_menu;

	CCmMenuItem() : m_bInitMenu(false) { }
	CCmMenuItem(PCIDLIST_ABSOLUTE pidlFq) : m_bInitMenu(false)
	{
//		CCmItem::CCmItem(pidlFq, NULL);
		if (pidlFq) {
			m_idl.Clone(pidlFq);
			SHCreateItemFromIDList(pidlFq, IID_PPV_ARGS(&m_psi));
			if (m_name.IsEmpty())
				idl_GetName(m_idl, SHGDN_NORMAL, m_name);
		}
	}

};

class CCmMenuItemArray : public CSimpleArray<CCmMenuItem*>
{
public:
	void DeleteAll()
	{
		CCmMenuItem* pmi;
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			pmi = (*this)[i];
			if (pmi) delete pmi;
		}
		RemoveAll();
	}
};

/////////////////////////////////////////////////////////////////////////////
//

class CTabReopenArray
{
private:
	CAtlArray<CCmItem*> m_arrItem;
	CAtlArray<CIconListStyle> m_arrStyle;
	int m_maxItems;

public:
	CTabReopenArray() : m_maxItems(10) { }
	~CTabReopenArray() { DeleteAll(); }
	void DeleteAll()
	{
		int cnt = (int)m_arrItem.GetCount();
		for (int i = 0; i < cnt; i++) {
			CCmItem* pci = m_arrItem.GetAt(i);
			if (pci) delete pci;
			m_arrItem.SetAt(i, NULL);
		}
		m_arrItem.RemoveAll();
		m_arrStyle.RemoveAll();
	}
	inline int GetItemCount() { return (int)m_arrItem.GetCount(); }
	void Append(CCmItem* pci, const CIconListStyle& ils)
	{
		int cnt = (int)m_arrItem.GetCount();
		if (cnt < m_maxItems) {
			m_arrItem.Add(pci);
			m_arrStyle.Add(ils);
		}
	}
	void Push(CCmItem* pci, const CIconListStyle& ils)
	{
		int cnt = (int)m_arrItem.GetCount();
		if (cnt == m_maxItems)
			RemoveAt(cnt - 1);
		m_arrItem.InsertAt(0, pci);
		m_arrStyle.InsertAt(0, ils);
	}
	CCmItem* Pop(size_t iElement, CIconListStyle& ils)
	{
		size_t cnt = m_arrItem.GetCount();
		if (!cnt || iElement >= cnt) return NULL;

		ils = m_arrStyle.GetAt(iElement);
		CCmItem* pci = m_arrItem.GetAt(iElement);
		RemoveAt(iElement);
		return pci;
	}
	CCmItem* GetItemAt(size_t iElement)
	{
		size_t cnt = m_arrItem.GetCount();
		if (!cnt || iElement >= cnt) return NULL;
		return m_arrItem.GetAt(iElement);
	}
	CCmItem* LookupGuid(GUID& guid)
	{
		CCmItem* pci;
		size_t cnt = (int)m_arrItem.GetCount();
		for (size_t i = 0; i < cnt; i++) {
			pci = m_arrItem.GetAt(i);
			if (pci->m_guid == guid)
				return pci;
		}
		return NULL;
	}
	bool GetStyleAt(size_t iElement, CIconListStyle& ils)
	{
		size_t cnt = m_arrStyle.GetCount();
		if (!cnt || iElement >= cnt) return false;
		ils = m_arrStyle.GetAt(iElement);
		return true;
	}
	void RemoveAt(size_t iElement)
	{
		m_arrItem.RemoveAt(iElement);
		m_arrStyle.RemoveAt(iElement);
	}
};

/////////////////////////////////////////////////////////////////////////////
// CIDListWnd window

#ifndef TOUPPER
	#define TOUPPER(ch)		(((ch)>='a' && (ch)<='z') ? ((ch)-('a'-'A')) : (ch))
#endif

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CIDListWndImpl : public CWindowImpl< T, TBase, TWinTraits>
{
public:
	CIDListWndImpl()
	{
		HIMAGELIST dumm;
		Shell_GetImageLists(&dumm, &m_hImage);
	}

	~CIDListWndImpl() {}

	DECLARE_WND_CLASS(NULL)

	HIMAGELIST m_hImage;

BEGIN_MSG_MAP(this)
	ON_WM_INITMENUPOPUP()
	ON_WM_MENUCHAR()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MSG_MAP()

	void OnInitMenuPopup(HMENU hPopupMenu, UINT nIndex, BOOL bSysMenu)
	{
		if (!bSysMenu) {
			MENUINFO mi;
			mi.cbSize = sizeof(mi);
			mi.fMask = MIM_BACKGROUND;
			mi.hbrBack = _GCDATA.GetMenuBackgroundBrush();
			::SetMenuInfo(hPopupMenu, &mi);
		}
	}

	LRESULT OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->MenuChar(nChar, nFlags, hMenu);
	}

	void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT lpdi)
	{
		T* pT = static_cast<T*>(this);
		pT->DrawItem(idCtrl, lpdi);
	}

	void OnMeasureItem(int idCtrl, LPMEASUREITEMSTRUCT lpmi)
	{
		T* pT = static_cast<T*>(this);
		pT->MeasureItem(idCtrl, lpmi);
	}

	int DoTrack(HWND hParent, HMENU hMenu, CPoint pt)
	{
		CRect rc(0, 0, 10, 10);
		Create(hParent, rc, NULL, WS_CHILD);
		int idCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x,  pt.y, 0, m_hWnd, NULL);
		DestroyWindow();
		return idCmd;
	}

	virtual LRESULT MenuChar(UINT nChar, UINT nFlags, HMENU hMenu)
	{
		int cnt = GetMenuItemCount(hMenu);
		CMenuItemInfo mii;
		mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
		for (int i = 0; i < cnt; i++) {
			if (GetMenuItemInfo(hMenu, i, TRUE, &mii)) {
				if (mii.fType & MFT_OWNERDRAW) {
					CCmItem* pci = (CCmItem*)mii.dwItemData;
					if (pci) {
						TCHAR* p = StrChr(pci->m_name, '&');
						if (p && TOUPPER((TCHAR)nChar) == TOUPPER(*(p + 1)))
							return MAKELONG(i, MNC_EXECUTE);
					}
				}
			}
		}
		return 0;
	}

	virtual void DrawItem(int idCtl, LPDRAWITEMSTRUCT pdi)
	{
		if (pdi->CtlType != ODT_MENU) return;

		if (pdi->itemData) {
			CCmItem* pci = (CCmItem*)pdi->itemData;
			lccDrawCustomMenu(pdi, pci->m_name, m_hImage, pci->GetImageIndex(), DT_PATH_ELLIPSIS);
		}
		else
			lccDrawCustomMenu(pdi, NULL, NULL, -1, DT_NOCLIP);
	}

	virtual void MeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpmi)
	{
		if (lpmi->CtlType != ODT_MENU) return;

		if (lpmi->itemData) {
			lpmi->itemWidth = _GCDATA.m_NcMetrics.iMenuWidth;
			lpmi->itemHeight = _GCDATA.m_NcMetrics.iMenuHeight;

			CClientDC dc(NULL);
			HFONT hfont = dc.SelectFont(_GCDATA.GetMenuFont());
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);

			CCmItem* pci = (CCmItem*)lpmi->itemData;
			CSize csz;
			dc.GetTextExtent(pci->m_name, pci->m_name.GetLength(), &csz);

			lpmi->itemWidth = min(csz.cx, tm.tmAveCharWidth * 100) + 34;
			lpmi->itemHeight = max(lpmi->itemHeight, MENU_OWNERDRAW_HEIGHT);
			dc.SelectFont(hfont);
		}
		else {
			lpmi->itemHeight = ::GetSystemMetrics(SM_CYMENU) / 2;
			lpmi->itemWidth  = 0;
		}
	}
};

class CIDListWnd : public CIDListWndImpl<CIDListWnd>
{
public:
};


/////////////////////////////////////////////////////////////////////////////
// CCmWnd window

class CCmWnd : public CIDListWndImpl<CCmWnd>
{
public:
	CCmWnd(HMENU hMenu, UINT nFolderID, BOOL bRetreiveTree);
	CCmWnd(HMENU hMenu, LPITEMIDLIST pidl, BOOL bRetreiveTree);
	~CCmWnd();
	void Initialize(HMENU hMenu, LPITEMIDLIST pidl, BOOL bRetreiveTree);

	DECLARE_WND_CLASS(NULL)

	CCmMenuItemArray m_array;
	CSimpleArray<HBITMAP> m_arrVistaBitmaps;
	int m_nNextMenuID;
	HMENU m_hMenu;
	CIDListPtr m_idl;
	BOOL m_bRetrieveTree;

BEGIN_MSG_MAP(CCmWnd)
	ON_WM_INITMENUPOPUP()
	CHAIN_MSG_MAP(CIDListWndImpl<CCmWnd>)
END_MSG_MAP()

	void OnInitMenuPopup(HMENU hMenu, UINT nIndex, BOOL bSysMenu);

	BOOL AddMenuItem(HMENU hmenu, CCmMenuItem* pci);
	CCmMenuItem* FindItem(int id);
	CCmMenuItem* FindMenu(HMENU hmenu);
	void AddFolderMenu(IShellFolder* psf, LPITEMIDLIST pidlFq, HMENU hmenu);
};



#endif			//__CMWND_H__

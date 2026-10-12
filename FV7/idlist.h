/////////////////////////////////////////////////////////////////////////
// idlist.h

#pragma once
#ifndef __IDLIST_CLASS_H__
#define __IDLIST_CLASS_H__

#include "idl_func.h"

/* ------------------------------------
 * FOLDERID_xx data
 * ------------------------------------

 <Home>  ........ X:/Users/AccountName/
 <MyAppData> .... X:/Users/AccountName/AppData/
 <UserPublic> ... X:/Users/Public/
 <ProgData> ..... X:/ProgramData/
 <CommonFiles> .. X:/Program Files/Common Files/


 FOLDER                       BROWSE  PATH
------------------------------------------------------------------------------------
FOLDERID_NetworkFolder:        OK    N/A
FOLDERID_ComputerFolder:       OK    N/A
FOLDERID_InternetFolder:       OK    N/A
FOLDERID_ControlPanelFolder:   OK    N/A
FOLDERID_PrintersFolder:       OK    N/A
FOLDERID_SyncManagerFolder:    OK    N/A
FOLDERID_SyncSetupFolder:      OK    N/A
FOLDERID_ConflictFolder:       OK    N/A
FOLDERID_SyncResultsFolder:    OK    N/A
FOLDERID_RecycleBinFolder:     OK    N/A
FOLDERID_ConnectionsFolder:    OK    N/A
FOLDERID_Fonts:                OK    <Windir>/Fonts
FOLDERID_Desktop:              OK    <Home>/Desktop
FOLDERID_Startup:              OK    <MyAppData>/Roaming/Microsoft/Windows/Start Menu/Programs/Startup
FOLDERID_Programs:             OK    <MyAppData>/Roaming/Microsoft/Windows/Start Menu/Programs
FOLDERID_StartMenu:            OK    <MyAppData>/Roaming/Microsoft/Windows/Start Menu
FOLDERID_Recent:               OK    <MyAppData>/Roaming/Microsoft/Windows/Recent
FOLDERID_SendTo:               OK    <MyAppData>/Roaming/Microsoft/Windows/SendTo
FOLDERID_Documents:            OK    <Home>/Documents
FOLDERID_Favorites:            OK    <Home>/Favorites
FOLDERID_NetHood:              OK    <MyAppData>/Roaming/Microsoft/Windows/Network Shortcuts
FOLDERID_PrintHood:            OK    <MyAppData>/Roaming/Microsoft/Windows/Printer Shortcuts
FOLDERID_Templates:            OK    <MyAppData>/Roaming/Microsoft/Windows/Templates
FOLDERID_CommonStartup:        OK    <ProgData>/Microsoft/Windows/Start Menu/Programs/Startup
FOLDERID_CommonPrograms:       OK    <ProgData>/Microsoft/Windows/Start Menu/Programs
FOLDERID_CommonStartMenu:      OK    <ProgData>/Microsoft/Windows/Start Menu
FOLDERID_PublicDesktop:        OK    <UserPublic>/Desktop
FOLDERID_ProgramData:          OK    <ProgData>
FOLDERID_CommonTemplates:      OK    <ProgData>/Microsoft/Windows/Templates
FOLDERID_PublicDocuments:      OK    <UserPublic>/Documents
FOLDERID_RoamingAppData:       OK    <MyAppData>/Roaming
FOLDERID_LocalAppData:         OK    <MyAppData>/Local
FOLDERID_LocalAppDataLow:      OK    <MyAppData>/LocalLow
FOLDERID_InternetCache:        OK    <MyAppData>/Local/Microsoft/Windows/INetCache
FOLDERID_Cookies:              OK    <MyAppData>/Local/Microsoft/Windows/INetCookies
FOLDERID_History:              OK    <MyAppData>/Local/Microsoft/Windows/History
FOLDERID_System:               OK    <Windir>/System32
FOLDERID_SystemX86:            OK    <Windir>/SysWOW64
FOLDERID_Windows:              OK    <Windir>
FOLDERID_Profile:              OK    <UserProfile>
FOLDERID_Pictures:             OK    <Home>/Pictures
FOLDERID_ProgramFilesX86:      OK    <ProgramFilesX86>
FOLDERID_ProgramFilesCommonX86:OK    <CommonFIlesX86>
FOLDERID_ProgramFilesX64:      OK    <ProgramFiles>
FOLDERID_ProgramFilesCommonX64:OK    <CommonFiles>
FOLDERID_ProgramFiles:         OK    <ProgramFiles>
FOLDERID_ProgramFilesCommon:   OK    <CommonFiles>
FOLDERID_UserProgramFiles:     OK    <MyAppData>/Local/Programs
FOLDERID_UserProgramFilesCommon: OK  <MyAppData>/Local/Programs/Common
FOLDERID_AdminTools:           OK    <MyAppData>/Roaming/Microsoft/Windows/Start Menu/Programs/Administrative Tools
FOLDERID_CommonAdminTools:     OK    <ProgData>/Microsoft/Windows/Start Menu/Programs/Administrative Tools
FOLDERID_Music:                OK    <Home>/Music
FOLDERID_Videos:               OK    <Home>/Videos
FOLDERID_Ringtones:            OK    <MyAppData>/Local/Microsoft/Windows/Ringtones
FOLDERID_PublicPictures:       OK    <UserPublic>/Pictures
FOLDERID_PublicMusic:          OK    <UserPublic>/Music
FOLDERID_PublicVideos:         OK    <UserPublic>/Videos
FOLDERID_PublicRingtones:      OK    <ProgData>/Microsoft/Windows/Ringtones
FOLDERID_ResourceDir:          OK    <Windir>/Resources
FOLDERID_LocalizedResourcesDir:***
FOLDERID_CommonOEMLinks:       ***
FOLDERID_CDBurning:            OK    <MyAppData>/Local/Microsoft/Windows/Burn/Burn
FOLDERID_UserProfiles:         OK    X:/Users
FOLDERID_Playlists:            ***
FOLDERID_SamplePlaylists:      ***
FOLDERID_SampleMusic:          ***
FOLDERID_SamplePictures:       ***
FOLDERID_SampleVideos:         ***
FOLDERID_PhotoAlbums:          ***
FOLDERID_Public:               OK    <UserPublic>
FOLDERID_ChangeRemovePrograms: NG    N/A
FOLDERID_AppUpdates:           NG    N/A
FOLDERID_AddNewPrograms:       NG    N/A
FOLDERID_Downloads:            OK    <Home>/Downloads
FOLDERID_PublicDownloads:      OK    <UserPublic>/Downloads
FOLDERID_SavedSearches:        OK    <Home>/Searches
FOLDERID_QuickLaunch:          OK    <MyAppData>/Roaming/Microsoft/Internet Explorer/Quick Launch
FOLDERID_Contacts:             OK    <Home>/Contacts
FOLDERID_SidebarParts:         ***
FOLDERID_SidebarDefaultParts:  ***
FOLDERID_PublicGameTasks:      OK    <ProgData>/Microsoft/Windows/GameExplorer
FOLDERID_GameTasks:            OK    <MyAppData>/Local/Microsoft/Windows/GameExplorer
FOLDERID_SavedGames:           OK    <Home>/Saved Games
FOLDERID_Games:                ***
FOLDERID_SEARCH_MAPI:          NG    N/A
FOLDERID_SEARCH_CSC:           NG    N/A
FOLDERID_Links:                OK    <Home>/Links
FOLDERID_UsersFiles:           OK    <Home>
FOLDERID_UsersLibraries:       OK    N/A
FOLDERID_SearchHome:           NG    N/A
FOLDERID_OriginalImages:       ***
FOLDERID_DocumentsLibrary:     OK    N/A
FOLDERID_MusicLibrary:         OK    N/A
FOLDERID_PicturesLibrary:      OK    N/A
FOLDERID_VideosLibrary:        OK    N/A
FOLDERID_RecordedTVLibrary:    NG    <UserPublic>/Libraries/RecordedTV.library-ms
FOLDERID_HomeGroup:            ***
FOLDERID_HomeGroupCurrentUser: ***
FOLDERID_DeviceMetadataStore:  OK    <ProgData>/Microsoft/Windows/DeviceMetadataStore
FOLDERID_Libraries:            OK    N/A
FOLDERID_PublicLibraries:      OK    <UserPublic>/Libraries
FOLDERID_UserPinned:           OK    <MyAppData>/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned
FOLDERID_ImplicitAppShortcuts: OK    <MyAppData>/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/ImplicitAppShortcutsFOLDERID_
FOLDERID_AccountPictures:      OK    <MyAppData>/Roaming/Microsoft/Windows/AccountPictures
FOLDERID_PublicUserTiles:      OK    <UserPublic>/AccountPictures
FOLDERID_AppsFolder:           OK    N/A
FOLDERID_StartMenuAllPrograms: ***
FOLDERID_CommonStartMenuPlaces:***
FOLDERID_ApplicationShortcuts: OK    <MyAppData>/Local/Microsoft/Windows/Application Shortcuts
FOLDERID_RoamingTiles:         OK    <MyAppData>/Local/Microsoft/Windows/RoamingTiles
FOLDERID_RoamedTileImages:     ***
FOLDERID_Screenshots:          OK    <Home>/Pictures/Screenshots
FOLDERID_CameraRoll:           OK    <Home>/Pictures/Camera Roll
FOLDERID_SkyDrive:             OK    <Home>/OneDrive
FOLDERID_OneDrive:             OK    <Home>/OneDrive
FOLDERID_SkyDriveDocuments:    OK    <Home>/OneDrive/ドキュメント
FOLDERID_SkyDrivePictures:     OK    <Home>/OneDrive/画像
FOLDERID_SkyDriveMusic:        ***
FOLDERID_SkyDriveCameraRoll:   OK    <Home>/OneDrive/画像/カメラ ロール
FOLDERID_SearchHistory:        ***
FOLDERID_SearchTemplates:      ***
FOLDERID_CameraRollLibrary:    OK    N/A
FOLDERID_SavedPictures:        OK    <Home>/Pictures/Saved Pictures
FOLDERID_SavedPicturesLibrary: OK    N/A
FOLDERID_RetailDemo:           OK    <ProgData>/Microsoft/Windows/RetailDemo
FOLDERID_Device:               OK    ::{F8278C54-A712-415B-B593-B77A2BE0DDA9}
FOLDERID_DevelopmentFiles:     ***
FOLDERID_Objects3D:            ***
FOLDERID_AppCaptures:          OK    <Home>/Videos/Captures
FOLDERID_LocalDocuments:       OK    <Home>/Documents
FOLDERID_LocalPictures:        OK    <Home>/Pictures
FOLDERID_LocalVideos:          OK    <Home>/Videos
FOLDERID_LocalMusic:           OK    <Home>/Music
FOLDERID_LocalDownloads:       OK    <Home>/Downloads
FOLDERID_RecordedCalls:        ***
FOLDERID_AllAppMods:           ***
FOLDERID_CurrentAppMods:       ***
FOLDERID_AppDataDesktop:       ***
FOLDERID_AppDataDocuments:     ***
FOLDERID_AppDataFavorites:     ***
FOLDERID_AppDataProgramData:   ***
FOLDERID_LocalStorage:         ***

*/

#define FOLDERID_AppData		FOLDERID_RoamigAppData

////////////////////////////////////////////////////////////////////////
//

template <bool t_bManaged>
class CIDListPtrT
{
public:
	LPCITEMIDLIST m_pidl;

	CIDListPtrT(LPCITEMIDLIST pidl = NULL) : m_pidl(pidl)
	{
	}

	CIDListPtrT(IShellFolder* psf, PCUIDLIST_RELATIVE pidl) : m_pidl(NULL)
	{
		if (t_bManaged)
			GetFullyQual(psf, pidl);
	}

	CIDListPtrT(IShellItem* psi) : m_pidl(NULL)
	{
		if (t_bManaged)
			GetFromShellItem(psi);
	}

	CIDListPtrT(LPCTSTR pszPath) : m_pidl(NULL)
	{
		if (t_bManaged)
			CreateFromPath(pszPath);
	}

	CIDListPtrT(HWND hwndOwner, int nFolder, HANDLE hToken = NULL) : m_pidl(NULL)
	{
		if (t_bManaged)
			GetFolderLocation(hwndOwner, nFolder, hToken);
	}

	CIDListPtrT(REFKNOWNFOLDERID guid, DWORD flags = 0, HANDLE hToken = NULL) : m_pidl(NULL)
	{
		if (t_bManaged)
			GetKnownFolder(guid, flags, hToken);
	}

	~CIDListPtrT()
	{
		if (t_bManaged && m_pidl != NULL)
			::CoTaskMemFree((void*)Detach());
	}

	CIDListPtrT<t_bManaged>& operator =(LPCITEMIDLIST pidl)
	{
		Attach(pidl);
		return *this;
	}

	void Attach(LPCITEMIDLIST pidl)
	{
		if (t_bManaged && m_pidl != NULL && m_pidl != pidl)
			::CoTaskMemFree((void*)m_pidl);
		m_pidl = pidl;
	}

	LPCITEMIDLIST Detach()
	{
		LPCITEMIDLIST pidl = m_pidl;
		m_pidl = NULL;
		return pidl;
	}

	operator LPITEMIDLIST() const { return (LPITEMIDLIST)m_pidl; }
	operator LPCITEMIDLIST() const { return m_pidl; }

	inline bool isNull() const { return (m_pidl == NULL); }
	inline bool isEmpty() const { return (m_pidl == NULL || m_pidl->mkid.cb == 0); }

	void Free()
	{
		LPCITEMIDLIST pidl = Detach();
		if (pidl) ::CoTaskMemFree((void*)pidl);
	}

	void Alloc(DWORD size)
	{
		Attach((LPCITEMIDLIST)memset(::CoTaskMemAlloc(size), 0, size));
	}

	inline void* GetData() const { return (void*)m_pidl; }
	inline bool isDesktop() const { return (m_pidl && (::ILGetSize(m_pidl) == 2)); }

	LPCITEMIDLIST Clone(LPCITEMIDLIST pidlSrc)
	{
		Attach(pidlSrc != NULL ? ILClone(pidlSrc) : NULL);
		return m_pidl;
	}

	LPCITEMIDLIST Combine(PCIDLIST_ABSOLUTE pidl1, PCUIDLIST_RELATIVE pidl2)
	{
		Attach(ILCombine(pidl1, pidl2));
		return m_pidl;
	}

	HRESULT GetFolderLocation(HWND hwndOwner, int nFolder, HANDLE hToken = NULL)
	{
		LPITEMIDLIST pidl = NULL;
		HRESULT hr = ::SHGetFolderLocation(hwndOwner, nFolder, hToken, 0, &pidl);
		if (hr == S_OK)
			Attach(pidl);
		return hr;
	}

	HRESULT GetKnownFolder(REFKNOWNFOLDERID guid, DWORD flags = 0, HANDLE hToken = NULL)
	{
		HRESULT hr = E_FAIL;
		LPITEMIDLIST pidl = NULL;
		hr = SHGetKnownFolderIDList(guid, flags, hToken, &pidl);
		if (hr == S_OK)
			Attach(pidl);
		return hr;
	}

	HRESULT GetMyDocument()
	{
		if (GetKnownFolder(FOLDERID_DocumentsLibrary) == S_OK)
			return S_OK;
		LPITEMIDLIST pidl = NULL;
		HRESULT hr = E_FAIL;
		CComPtr<IShellFolder> psf;
		if (::SHGetDesktopFolder(&psf) == NOERROR) {
			ULONG eat;
			hr = psf->ParseDisplayName(0, 0, STR_MYDOCS_CLSID_W, &eat, &pidl, 0);
			if (hr == S_OK)
				Attach(pidl);
		}
		return hr;
	}

	HRESULT GetFromShellItem(IShellItem* psi)
	{
		HRESULT hr = E_FAIL;
		LPITEMIDLIST pidl;
		hr = SHGetIDListFromObject(psi, &pidl);
		if (hr == S_OK)
			Attach(pidl);
		return hr;
	}

	LPCITEMIDLIST CreateFromPath(LPCTSTR pszPath)
	{
		Attach(::ILCreateFromPath(pszPath));
		return m_pidl;
	}

	LPCITEMIDLIST GetFullyQual(IShellFolder* psf, PCUIDLIST_RELATIVE pidl)
	{
		Attach(idl_GetFullyQualPidl(psf, pidl));
		return m_pidl;
	}

	UINT GetNumIds() const
	{
		CIDListPtrT<false> idl = m_pidl;
		UINT cIds = 0;
		while (!idl.isEmpty()) {
			cIds++;
			idl = idl.GetNext();
		}
		return cIds;
	}

	inline BOOL RemoveLastID() { return ::ILRemoveLastID((LPITEMIDLIST)m_pidl); }
	inline LPCITEMIDLIST FindLastID() const { return ::ILFindLastID(m_pidl); }
	inline LPCITEMIDLIST GetNext() const { return ::ILGetNext(m_pidl); }
	inline BOOL isEqual(LPCITEMIDLIST pidlTarget) const { return ::ILIsEqual(m_pidl, pidlTarget); }
	inline BOOL isParent(LPCITEMIDLIST pidlTarget, BOOL fImmediate = FALSE) const { return ::ILIsParent(pidlTarget, m_pidl, fImmediate); }
	inline BOOL isChild(LPCITEMIDLIST pidlTarget, BOOL fImmediate = FALSE) const { return ::ILIsParent(m_pidl, pidlTarget, fImmediate); }
	inline UINT GetSize() const { return ::ILGetSize(m_pidl); }
	inline bool operator==(const CIDListPtrT& item) const { return (::ILIsEqual(m_pidl, item.m_pidl) != FALSE); }
	inline bool operator==(const LPCITEMIDLIST item) const { return (::ILIsEqual(m_pidl, item) != FALSE); }
	inline bool operator!=(const CIDListPtrT& item) const { return (::ILIsEqual(m_pidl, item.m_pidl) == FALSE); }

	BOOL GetPath(CString& str) const
	{
		LPTSTR p = str.GetBuffer(MAX_PATH * 2);
		*p = 0;
		BOOL bOK = ::SHGetPathFromIDList(m_pidl, p);
		str.ReleaseBuffer();
		return bOK;
	}

	inline BOOL GetPath(LPTSTR path) const
	{
		return ::SHGetPathFromIDList(m_pidl, path);
	}

	inline DWORD_PTR GetFileInfo(DWORD dwFileAttr, SHFILEINFO* psfi, UINT cbFileInfo, UINT uFlags) const { return ::SHGetFileInfo((LPCTSTR)m_pidl, dwFileAttr, psfi, cbFileInfo, uFlags | SHGFI_PIDL); }

	SFGAOF GetAttributes(SFGAOF mask) const
	{
		SHFILEINFO sfi = { 0 };
		sfi.dwAttributes = mask;
		GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
		return (SFGAOF)sfi.dwAttributes;
	}

	void* GetUIObjectOf(HWND hwnd, REFIID riid) const
	{
		void* pv = NULL;
		CComPtr<IShellFolder> psf;
		LPCITEMIDLIST pidl;
		if (::SHBindToParent(m_pidl, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
			if (NOERROR != psf->GetUIObjectOf(hwnd, 1, &pidl, riid, 0, &pv))
				pv = NULL;
		}
		return pv;
	}

	IShellFolder* GetShellFolder() const
	{
		if (!m_pidl) return NULL;

		IShellFolder* psf = NULL;
		IShellFolder* psfDesk;
		if (::SHGetDesktopFolder(&psfDesk) == S_OK) {
			if (isDesktop())
				return psfDesk;

			if (psfDesk->BindToObject(m_pidl, NULL, IID_PPV_ARGS(&psf)) != S_OK)
				psf = NULL;
			psfDesk->Release();
		}
		if (!psf) {
			CComPtr<IShellFolder> psfParent;
			LPCITEMIDLIST pidlSingle;
			if (::SHBindToParent(m_pidl, IID_PPV_ARGS(&psfParent), &pidlSingle) == S_OK) {
				if (psfParent->BindToObject(pidlSingle, 0, IID_PPV_ARGS(&psf)) != S_OK)
					psf = NULL;
			}
		}
		return psf;
	}

	IShellFolder* BindToParent(LPCITEMIDLIST* ppidl) const
	{
		IShellFolder* psf;
		if (m_pidl && SHBindToParent(m_pidl, IID_PPV_ARGS(&psf), ppidl) == S_OK)
			return psf;
		return NULL;
	}

	void* BindToObject(const IID& riid) const
	{
		void* pInterface = NULL;
		LPCITEMIDLIST pidl;
		CComPtr<IShellFolder> psf = BindToParent(&pidl);
		if (psf.p != NULL) {
			psf->BindToObject(pidl, NULL, riid, &pInterface);
		}
		return pInterface;
	}

	BOOL GetDisplayName(DWORD flags, CString& name) const
	{
		// flags: SHGDN_FORPARSING, SHGDN_INFOLDER... etc
		LPCITEMIDLIST pidlSimple;
		CComPtr<IShellFolder> psf = BindToParent(&pidlSimple);
		return !psf ? FALSE : idl_GetName(psf, pidlSimple, flags, name);
	}

	int GetSysImageListIndex(int* piOpenIcon) const
	{
		LPCITEMIDLIST pidlSimple;
		CComPtr<IShellFolder> psf = BindToParent(&pidlSimple);
		return !psf ? -1 : SHMapPIDLToSystemImageListIndex(psf, pidlSimple, piOpenIcon);
	}

	LPCITEMIDLIST MakeRoot()
	{
		if (!m_pidl) return NULL;

		// パス名を取得
		CString dir;
		if (GetPath(dir)) {
			CIDListPtr idlTemp;
			idlTemp.CreateFromPath(dir);
			if (idlTemp.isEqual(m_pidl)) {
				// 一致すればパスからルートパスを取得
				PathStripToRoot((LPTSTR)dir.GetString());
				idlTemp.CreateFromPath(dir);
				Attach(idlTemp.Detach());
			}
			else {
				// まずmydocumentを確認
				if (idlTemp.GetMyDocument() == S_OK && ::ILIsParent(idlTemp, m_pidl, false))
					Attach(idlTemp.Detach());
				// 次にユーザー\<ユーザー名>を確認
				else if (idlTemp.GetKnownFolder(FOLDERID_UsersFiles) == S_OK && ::ILIsParent(idlTemp, m_pidl, false))
					Attach(idlTemp.Detach());
				// ライブラリを確認(vista以降)
				else if (idlTemp.GetKnownFolder(FOLDERID_UsersLibraries) == S_OK && ::ILIsParent(idlTemp, m_pidl, false))
					Attach(idlTemp.Detach());
			}
		}
		return m_pidl;
	}

	/*
	 * 指定のフォルダでEnumObjectsできるかチェック
	 */
	bool isBrowsable(HWND hwnd) const
	{
		bool ok = false;
		CComPtr<IShellFolder> psf = GetShellFolder();
		if (psf.p != NULL) {
			CComPtr<IEnumIDList> pe;
//			ok = (psf->EnumObjects(hwnd, SHCONTF_FASTITEMS, &pe) == S_OK);
			ok = (psf->EnumObjects(hwnd, SHCONTF_CHECKING_FOR_CHILDREN | SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pe) == S_OK);
		}
		return ok;
	}
};

typedef CIDListPtrT<false>   CIDListPtrHandle;
typedef CIDListPtrT<true>    CIDListPtr;

#endif		// __IDLIST_CLASS_H__

////////////////////////////////////////////////////////////////
// pidl.cpp

#include "stdafx.h"
#include "pidl.h"


////////////////////////////////////////////////////////////////
//


LPITEMIDLIST __fastcall idl_Next(LPCITEMIDLIST pidl)
{
	int cb = pidl->mkid.cb;
	if (cb == 0)
		return NULL;

	pidl = (LPITEMIDLIST)(((LPBYTE)pidl) + cb);
	return (pidl->mkid.cb == 0) ? NULL : (LPITEMIDLIST)pidl;
}

UINT __fastcall idl_GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	if (pidl) {
		cbTotal += sizeof(pidl->mkid.cb);	// Null terminator
		while (pidl && pidl->mkid.cb) {
			cbTotal += pidl->mkid.cb;
			pidl = idl_Next(pidl);
		}
	}
	return cbTotal;
}

LPITEMIDLIST __fastcall idl_Create(UINT cbSize)
{
	LPITEMIDLIST pidl = NULL;
	pidl = (LPITEMIDLIST)idl_Alloc(cbSize);
	if (pidl)
		ZeroMemory(pidl, cbSize);			// zero-init for external task alloc
	return pidl;
}

UINT __fastcall idl_GetNumIds(LPCITEMIDLIST pidl)
{
	UINT cIds = 0;
	while (pidl) {
		cIds++;
		pidl = idl_Next(pidl);
	}
	return cIds;
}

LPITEMIDLIST __fastcall idl_ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	UINT cb1 = (pidl1 != NULL) ? (idl_GetSize(pidl1) - sizeof(pidl1->mkid.cb)) : 0;
	UINT cb2 = idl_GetSize(pidl2);

	LPITEMIDLIST pidlNew = idl_Create(cb1 + cb2);
	if (pidlNew) {
		if (pidl1)
			memcpy(pidlNew, pidl1, cb1);
		memcpy(((BYTE*)pidlNew) + cb1, pidl2, cb2);
	}
	return pidlNew;
}

////////////////////////////////////////////////////////////////
// アイテムリストの先頭アイテムをコピー
//
LPITEMIDLIST __fastcall idl_CopyITEMID(LPITEMIDLIST pidl)
{
	if (!pidl)
		return NULL;

	int nSize = pidl->mkid.cb;
	LPITEMIDLIST pidlT;
	pidlT = (LPITEMIDLIST)idl_Alloc(nSize + sizeof(USHORT));
	if (pidlT) {
		memcpy(pidlT, pidl, nSize);
		*((USHORT*)(((LPBYTE)pidlT) + nSize)) = 0;
	}
	return pidlT;
}

BOOL __fastcall idl_GetInfoTip(HWND hwnd, LPITEMIDLIST pidlFq, LPTSTR pTip, int cchMax)
{
	BOOL bOK = FALSE;
	LPSHELLFOLDER psf;
	LPITEMIDLIST pidl;
	if (pidlFq &&
		SUCCEEDED(SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, (LPCITEMIDLIST*)&pidl))) {
		bOK = idl_GetInfoTip(hwnd, psf, pidl, pTip, cchMax);
		psf->Release();
	}
	return bOK;
}

BOOL __fastcall idl_GetInfoTip(HWND hwnd, LPSHELLFOLDER psf, LPITEMIDLIST pidl, LPTSTR pTip, int cchMax)
{
	BOOL bOK = FALSE;
	IQueryInfo* pqi;
	if (psf &&
		pidl &&
		psf->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidl, IID_IQueryInfo, NULL, (void**)&pqi) == NOERROR) {
		LPWSTR pw;
		if (pqi->GetInfoTip(0, &pw) == NOERROR) {
			lstrcpyn(pTip, pw, cchMax);
			idl_Free(pw);
			bOK = TRUE;
		}
		pqi->Release();
	}
	return bOK;
}

BOOL __fastcall idl_GetName(LPSHELLFOLDER psf, LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pName, UINT cchName)
{
	STRRET str;
	if (psf && pidl && NOERROR == psf->GetDisplayNameOf(pidl, dwFlags, &str)) {
		LPTSTR psz;
		if (StrRetToStr(&str, pidl, &psz) == S_OK) {
			lstrcpyn(pName, psz, cchName);
			CoTaskMemFree(psz);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL __fastcall idl_GetName(LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pName, UINT cchName)
{
	BOOL bOK = FALSE;
	IShellFolder* psf;
	if (pidl && SHGetDesktopFolder(&psf) == NOERROR) {
		bOK = idl_GetName(psf, pidl, dwFlags, pName, cchName);
		psf->Release();
	}
	return bOK;
}

LPITEMIDLIST __fastcall idl_GetFullyQualPidl(LPSHELLFOLDER psf, LPITEMIDLIST pidl)
{
	if (!psf || !pidl)
		return NULL;

	LPITEMIDLIST pidlFq = NULL;
	IShellFolder* psfDesk;
	SHGetDesktopFolder(&psfDesk);

	STRRET str;
	if (NOERROR == psf->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str)) {
		LPTSTR psz;
		if (StrRetToStr(&str, pidl, &psz) == S_OK) {
			ULONG eat;
			if (psfDesk->ParseDisplayName(0, 0, psz, &eat, &pidlFq, NULL) != NOERROR)
				pidlFq = NULL;
			CoTaskMemFree(psz);
		}
	}
	psfDesk->Release();
	return pidlFq;
}

BOOL __fastcall idl_GetIcon(LPSHELLFOLDER lpsf, LPITEMIDLIST pidl, HICON* phLarge, HICON* phSmall)
{
	BOOL bOK = FALSE;
	*phLarge = NULL;
	*phSmall = NULL;
	LPEXTRACTICON lpxi;
	HRESULT hr = lpsf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl, IID_IExtractIcon, 0, (LPVOID*)&lpxi);
	if (SUCCEEDED(hr)) {
		TCHAR buf[MAX_PATH * 2];
		int nIndex;
		UINT nFlags;
		hr = lpxi->GetIconLocation(GIL_FORSHELL, buf, _COUNTOF(buf), &nIndex, &nFlags);
		if (SUCCEEDED(hr)) {
			hr = lpxi->Extract(buf, nIndex,	phLarge, phSmall, MAKELONG(32, 16));
			if (!SUCCEEDED(hr))
				ExtractIconEx(buf, nIndex, phLarge, phSmall, 1);
			bOK = TRUE;
			lpxi->Release();
		}
	}
	return bOK;
}

////////////////////////////////////////////////////////////////
// アイテムリストすべてをコピー
//
LPITEMIDLIST __fastcall idl_Copy(LPCITEMIDLIST pidlSrc)
{
	if (pidlSrc) {
		UINT cbSource = idl_GetSize(pidlSrc);
		if (cbSource > 0) {
			LPITEMIDLIST pidlDst = (LPITEMIDLIST)idl_Alloc(cbSource);
			if (pidlDst) {
				// Copy the source to the target
				memcpy(pidlDst, pidlSrc, cbSource);
				return pidlDst;
			}
		}
	}
	return NULL;
}

LPCITEMIDLIST __fastcall idl_SkipToLastItem(LPITEMIDLIST pidlFq)
{
	// pidlFQ が NULL かどうかをチェック
	if (!pidlFq || pidlFq->mkid.cb == 0)
		return NULL;

	register LPITEMIDLIST pidlNext, pidlTemp = pidlFq;
	while (pidlTemp->mkid.cb) {
		// アイテムリストから次のアイテムを取得
		pidlNext = (LPITEMIDLIST)((LPBYTE)pidlTemp + pidlTemp->mkid.cb);
		if (!pidlNext->mkid.cb)
			return (LPCITEMIDLIST)pidlTemp;
		pidlTemp = pidlNext;
	}
	return NULL;
}


////////////////////////////////////////////////////////////////
//  Takes a fully qualified pidl and returns the the relative pidl
//
//	IN: pidlFQ   - Pointer to the fully qualified ITEMIDLIST that needs to be
//				   parsed.
//	RET:         - Points to the pidl that will contain the root after parsing.
//
LPITEMIDLIST __fastcall idl_GetParent(LPITEMIDLIST pidlFQ)
{
	if (!pidlFQ)
		return NULL;

	LPITEMIDLIST pidlParent = idl_Copy(pidlFQ);
	register LPITEMIDLIST pidlTemp = pidlParent;
	register LPITEMIDLIST pidlNext;

	// Now walk through the list looking for the last item
	while (pidlTemp->mkid.cb) {
		// Get the next item and see if it's the NULL terminator at the end
		// of the list.
		pidlNext = (LPITEMIDLIST)((LPBYTE)pidlTemp + pidlTemp->mkid.cb);
		if (!pidlNext->mkid.cb) {
			// Add the NULL item to terminate the list
			pidlTemp->mkid.cb = 0;
			break;
		}
		pidlTemp = pidlNext;
	}
	return pidlParent;
}

////////////////////////////////////////////////////////////////
//  This routine takes a fully qualified pidl for a folder and returns 
//	the IShellFolder pointer for that pidl
//
//	IN: pidl - Pointer to a fully qualified ITEMIDLIST for the folder
//
LPSHELLFOLDER __fastcall idl_GetShellFolder(LPITEMIDLIST pidl)
{
	IShellFolder* psf = NULL;
	IShellFolder* psfDesk;
	if (SHGetDesktopFolder(&psfDesk) == NOERROR) {
		if (psfDesk->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID*)&psf) != NOERROR)
			psf = NULL;
		psfDesk->Release();
	}
	return psf;
}

////////////////////////////////////////////////////////////////
//  This routine takes a full path to a file and converts that
//	to a fully qualified ITEMIDLIST.
//
//  IN: pszFile  - Full path to the file.
//
LPITEMIDLIST __fastcall idl_GetFromPath(LPCTSTR pszFile)
{
	LPITEMIDLIST pidl = NULL;
	IShellFolder* psf;
	if (SHGetDesktopFolder(&psf) == NOERROR) {
		// Convert the path to an ITEMIDLIST
		ULONG eat;
		if (psf->ParseDisplayName(NULL, NULL, (LPTSTR)pszFile, &eat, &pidl, NULL) != NOERROR)
			pidl = NULL;
		psf->Release();
	}
	return pidl;
}

LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, LPITEMIDLIST pidlFq, REFIID riid)
{
	void* pv = NULL;
	LPSHELLFOLDER psf;
	LPCITEMIDLIST pidl;
	if (SUCCEEDED(SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, &pidl))) {
		if (NOERROR != psf->GetUIObjectOf(hwnd, 1, &pidl, riid, 0, &pv))
			pv = NULL;
		psf->Release();
	}
	return pv;
}

LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, IShellFolder* psf, LPITEMIDLIST pidl, REFIID riid)
{
	if (psf)
		psf->AddRef();
	else
		SHGetDesktopFolder(&psf);

	LPVOID pv = NULL;
	if (NOERROR != psf->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidl, riid, 0, &pv))
		pv = NULL;

	psf->Release();
	return pv;
}

DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, LPITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DWORD effSupp)
{
	// 指定のITEMIDLISTに、DATAOBJECTがドロップされた場合の動作を得る

	DROPEFFECT eff = DROPEFFECT_NONE;
	LPDROPTARGET pdt = (LPDROPTARGET)idl_GetUIObjectOf(hwnd, pidl, IID_IDropTarget);
	if (pdt) {
		POINTL ptl;
		ptl.x = ptl.y = 0;
		eff = effSupp;
		if (pdt->DragEnter(pdo, keyState, ptl, &eff) == S_OK)
			pdt->DragLeave();
		else
			eff = DROPEFFECT_NONE;

		pdt->Release();
	}
	return eff;
}

DROPEFFECT __fastcall idl_GetDropEffect(HWND hwnd, IShellFolder* psf, LPITEMIDLIST pidl, LPDATAOBJECT pdo, DWORD keyState, DWORD effSupp)
{
	// 指定のITEMIDLISTに、DATAOBJECTがドロップされた場合の動作を得る

	DROPEFFECT eff = DROPEFFECT_NONE;
	LPDROPTARGET pdt = (LPDROPTARGET)idl_GetUIObjectOf(hwnd, psf, pidl, IID_IDropTarget);
	if (pdt) {
		POINTL ptl;
		ptl.x = ptl.y = 0;
		eff = effSupp;
		if (pdt->DragEnter(pdo, keyState, ptl, &eff) == S_OK)
			pdt->DragLeave();
		else
			eff = DROPEFFECT_NONE;

		pdt->Release();
	}
	return eff;
}

DROPEFFECT __fastcall idl_Execute(HWND hwnd, LPITEMIDLIST pidl, LPDATAOBJECT pdo, DWORD keyState, DROPEFFECT dropEffect, POINT pt)
{
	DROPEFFECT eff = DROPEFFECT_NONE;
	LPDROPTARGET pdt = (LPDROPTARGET)idl_GetUIObjectOf(hwnd, pidl, IID_IDropTarget);
	if (pdt) {
		POINTL ptl;
		ptl.x = pt.x;
		ptl.y = pt.y;
		eff = dropEffect;
		if (pdt->DragEnter(pdo, keyState, ptl, &eff) == S_OK) {
			eff = dropEffect;
			pdt->Drop(pdo, keyState, ptl, &eff);
		}
		else
			eff = DROPEFFECT_NONE;
		pdt->Release();
	}
	return eff;
}

DROPEFFECT __fastcall idl_ExecuteDirect(HWND hwnd, LPITEMIDLIST pidl, LPDATAOBJECT pdo, DWORD keyState, DROPEFFECT dropEffect, POINT pt)
{
	DROPEFFECT eff = DROPEFFECT_NONE;
	LPDROPTARGET pdt = (LPDROPTARGET)idl_GetUIObjectOf(hwnd, pidl, IID_IDropTarget);
	if (pdt) {
		POINTL ptl;
		ptl.x = pt.x;
		ptl.y = pt.y;
		eff = dropEffect;
		pdt->Drop(pdo, keyState, ptl, &eff);
		pdt->Release();
	}
	return eff;
}

IShellFolder* __fastcall idl_GetSpecialFolder(UINT id, LPITEMIDLIST* ppidl)
{
	if (ppidl)
		*ppidl = NULL;

	IShellFolder* psf = NULL;
	IShellFolder* psfDesktop;
	SHGetDesktopFolder(&psfDesktop);
	if (psfDesktop) {
		LPITEMIDLIST pidl;
		SHGetFolderLocation(NULL, id, NULL, 0, &pidl);
		if (id == CSIDL_DESKTOP) {
			if (ppidl)
				*ppidl = pidl;
			else
				idl_Free(pidl);
			return psfDesktop;
		}

		if (pidl) {
			psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID*)&psf);
			if (ppidl)
				*ppidl = pidl;
			else
				idl_Free(pidl);
		}
		psfDesktop->Release();
	}
	return psf;
}

void __fastcall idl_ValidateCacheFolderInformation(LPSHELLFOLDER psf)
{
	if (psf) {
		SFGAOF attr = SFGAO_VALIDATE;
		psf->GetAttributesOf(0, NULL, &attr);
	}
}

void __fastcall idl_ValidateDriveInformation()
{
	LPITEMIDLIST pidlDrives;
	SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidlDrives);
	IShellFolder* psf = idl_GetShellFolder(pidlDrives);
	if (psf) {
		idl_ValidateCacheFolderInformation(psf);
		psf->Release();
	}
	idl_Free(pidlDrives);
}

int __fastcall idl_GetSysIconIndex(IShellFolder* psf, LPITEMIDLIST pidl, UINT flg)
{
	int index = -1;
	LPITEMIDLIST pidlFq = idl_GetFullyQualPidl(psf, pidl);
	if (pidlFq) {
		SHFILEINFO sfi;
		HIMAGELIST him = (HIMAGELIST)SHGetFileInfo((LPCTSTR)pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | flg);
		if (him)
			index = sfi.iIcon;
		idl_Free(pidlFq);
	}
	return index;
}

int __fastcall idl_GetSysIconIndex(LPITEMIDLIST pidlPar, LPITEMIDLIST pidl, UINT flg)
{
	int index = -1;
	LPITEMIDLIST pidlFq = idl_ConcatPidls(pidlPar, pidl);
	if (pidlFq) {
		SHFILEINFO sfi;
		HIMAGELIST him = (HIMAGELIST)SHGetFileInfo((LPCTSTR)pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | flg);
		if (him)
			index = sfi.iIcon;
		idl_Free(pidlFq);
	}
	return index;
}

int __fastcall idl_GetSysIconIndex(LPITEMIDLIST pidlFq, UINT flg)
{
	int index = -1;
	if (pidlFq) {
		SHFILEINFO sfi;
		HIMAGELIST him = (HIMAGELIST)SHGetFileInfo((LPCTSTR)pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | flg);
		if (him)
			index = sfi.iIcon;
	}
	return index;
}

#pragma pack(1)
typedef struct tagVIDLIST {
	WORD size;
	BYTE mediatype;			// MTYPE_???
	BYTE cb2;
	DWORD filesize;
	WORD dosdate;
	WORD dostime;
	WORD fileattr;
	BYTE filename[1];
} VIDLIST;
#pragma pack()

/*************************************
  シングルIDLISTの構成解析

  mediatype:
  0x20～0x2F ドライブ関連
  0x30～0x3F フォルダ・ファイル関連
  0x40～0x4F ネットワーク関連

#define MTYPE_ROOT_REGITEM		0x1F		// マイドキュメントやマイコンピュータ
#define MTYPE_DRIVE_REMOVABLE	0x22
#define MTYPE_DRIVE_FIXED		0x23
#define MTYPE_DRIVE_OTHER		0x24
#define MTYPE_DRIVE_CDROM		0x25
#define MTYPE_DRIVE_RAMDISK		0x26
#define MTYPE_DRIVE_OTHER		0x27
#define MTYPE_DRIVE_FDD525		0x28
#define MTYPE_DRIVE_FDD35		0x29
#define MTYPE_DRIVE_NETDRIVE	0x2A
#define MTYPE_FSYS_CONTROLS		0x2E		// コントロールパネル
#define MTYPE_FS_FOLDER			0x31
#define MTYPE_FS_FILE			0x32
#define MTYPE_FS_UNICODEFOLDER	0x35		// UNICODE フォルダ
#define MTYPE_FS_UNICODEFILE	0x36		// UNICODE ファイル
#define MTYPE_FSYS_A002			0x3E		// 
#define MTYPE_NET_SERVER		0x42		// サーバー
#define MTYPE_FSYS_A003			0x45		// コントロールパネルのアプレット等
#define MTYPE_NET_DOMAIN		0x46		// ネットワークドメイン
#define MTYPE_NET_RESTOFNET		0x47		// ネットワーク全体
#define MTYPE_FSYS_A005			0x4D		// ネットワークプレースの追加
#define MTYPE_FSYS_A006			0x71		// フォルダ?
#define MTYPE_FSYS_A007			0xB1		// お気に入りや履歴フォルダ等
#define MTYPE_FSYS_A008			0xB5		// ブリーフケース等
#define MTYPE_NET_SHARE			0xC3		// サーバシェア
*************************************/

int __fastcall idl_GetDescriptionId(IShellFolder* psf, LPITEMIDLIST pidl)
{
	int descid = 0;
	if (psf && pidl) {
		SHDESCRIPTIONID sdi;
		if (SHGetDataFromIDList(psf, pidl, SHGDFIL_DESCRIPTIONID, &sdi, sizeof(sdi)) == NOERROR)
			descid = sdi.dwDescriptionId;
		else {
			VIDLIST* pvd = (VIDLIST*)pidl;
			switch (pvd->mediatype) {
				case 0x22: descid = SHDID_COMPUTER_REMOVABLE; break;
				case 0x23: descid = SHDID_COMPUTER_FIXED; break;
				case 0x24: descid = SHDID_COMPUTER_OTHER; break;
				case 0x25: descid = SHDID_COMPUTER_CDROM; break;
				case 0x26: descid = SHDID_COMPUTER_RAMDISK; break;
				case 0x28: descid = SHDID_COMPUTER_DRIVE525; break;
				case 0x27: descid = SHDID_COMPUTER_OTHER; break;
				case 0x29: descid = SHDID_COMPUTER_DRIVE35; break;
				case 0x2A: descid = SHDID_COMPUTER_NETDRIVE; break;
				case 0x2E: descid = SHDID_ROOT_REGITEM; break;
				case 0x31: descid = SHDID_FS_DIRECTORY; break;
				case 0x32: descid = SHDID_FS_FILE; break;
				case 0x3A: descid = SHDID_FS_FILE; break;
						// ↑Win98デスクトップ上のOutLookExpressリンクファイル
				case 0xC3: descid = SHDID_NET_SHARE; break;
				case 0x42: descid = SHDID_NET_SERVER; break;
				case 0x46: descid = SHDID_NET_DOMAIN; break;
				case 0x47: descid = SHDID_NET_RESTOFNET; break;
				default: break;
			}
		}
	}
	return descid;
}

int __fastcall idl_GetDescriptionId(LPITEMIDLIST pidlFq)
{
	int descid = 0;
	IShellFolder* psf;
	LPITEMIDLIST pidl;
	if (SUCCEEDED(SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, (LPCITEMIDLIST*)&pidl))) {
		descid = idl_GetDescriptionId(psf, pidl);
		psf->Release();
	}
	return descid;
}

BOOL __fastcall idl_GetAttributesOf(LPITEMIDLIST pidlFq, SFGAOF* pattr)
{
/* @@@ あまり信用できない
	IShellFolder* psf;
	SHGetDesktopFolder(&psf);
	HRESULT hr = psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlFq, pattr);
	psf->Release();
	return (hr == NOERROR) ? TRUE : FALSE;
@@@ */
	SHFILEINFO sfi;
	sfi.dwAttributes = *pattr;
	BOOL bOK = (SHGetFileInfo((LPCTSTR)pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES) != 0);
	if (bOK) *pattr = sfi.dwAttributes;
	return bOK;
}

HBITMAP __fastcall idl_ExtractImage(IShellFolder* psf, LPITEMIDLIST pidl, SIZE* prgSize, DWORD depth)
{
	HBITMAP hbm = NULL;
	IExtractImage* pie = NULL;
	psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl, IID_IExtractImage, NULL, (void**)&pie);
	if (pie) {
		OLECHAR wsz[MAX_PATH * 2];
		DWORD prior = 0;			// IEI_PRIORITY_NORMAL is defined nowhere!
		DWORD flags = IEIFLAG_SCREEN;
		HRESULT hr = pie->GetLocation(wsz, _COUNTOF(wsz), &prior, prgSize, depth, &flags);
		// even if we've got shell v4.70+, not all files support thumbnails 
		if (NOERROR == hr)
			hr = pie->Extract(&hbm);
		pie->Release();
	}
	return hbm;
}


BOOL __fastcall idl_IDListIsValid(LPITEMIDLIST pidlFq)
{
	BOOL bOK = FALSE;
	IShellFolder* psf;
	LPCITEMIDLIST pidl;
	if (SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, &pidl) == S_OK) {
		SFGAOF ula = SFGAO_VALIDATE;
		if (psf->GetAttributesOf(1, &pidl, &ula) == NOERROR)
			bOK = TRUE;
		psf->Release();
	}
	return bOK;
}

///////////////////////////////
// 正常に動作するか不明
//
BOOL __fastcall idl_IsValidPidl(LPITEMIDLIST pidlFq)
{
	if (!pidlFq)
		return FALSE;

	IShellFolder* psf;
	SHGetDesktopFolder(&psf);

	BOOL bOK = FALSE;
	STRRET str;
	if (NOERROR == psf->GetDisplayNameOf(pidlFq, SHGDN_FORPARSING, &str)) {
		WCHAR oc[MAX_PATH * 4];
		if (StrRetToBufW(&str, pidlFq, oc, _COUNTOF(oc)) == S_OK) {
			ULONG eat, attr = SFGAO_VALIDATE;
			LPITEMIDLIST pidl;
			if (psf->ParseDisplayName(0, 0, oc, &eat, &pidl, &attr) == NOERROR) {
				idl_Free(pidl);
				bOK = TRUE;
			}
		}
	}
	psf->Release();
	return bOK;
}

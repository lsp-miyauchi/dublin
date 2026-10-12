////////////////////////////////////////////////////////////////
// pidl.cpp

#include "stdafx.h"
#include "idlist.h"

#ifndef MAX_URL_LENGTH
	#define MAX_URL_LENGTH			(2048 + 32 + 8)
#endif

////////////////////////////////////////////////////////////////
//

UINT ILGetItemCount(LPCITEMIDLIST pidl)
{
	UINT cIds = 0;
	while (!idl_IsEmpty(pidl)) {
		cIds++;
		pidl = ILGetNext(pidl);
	}
	return cIds;
}

BOOL idl_GetName(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD flags, CString& strName)
{
	BOOL bOK = FALSE;
	STRRET str;
	if (psf && pidl && NOERROR == psf->GetDisplayNameOf(pidl, flags, &str)) {
		LPTSTR psz;
		if (StrRetToStr(&str, pidl, &psz) == S_OK) {
			strName = psz;
			CoTaskMemFree(psz);
			bOK = TRUE;
		}
	}
	return bOK;
}

BOOL idl_GetName(LPCITEMIDLIST pidlFq, DWORD flag, CString& str)
{
	BOOL bOK = FALSE;
	if (pidlFq) {
		IShellFolder* psf;
		LPCITEMIDLIST pidl;
		if (SUCCEEDED(SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, &pidl))) {
			bOK = idl_GetName(psf, pidl, flag, str);
			psf->Release();
		}
	}
	return bOK;
}

LPITEMIDLIST __fastcall idl_GetFullyQualPidl(LPSHELLFOLDER psf, LPITEMIDLIST pidl)
{
	if (!psf || !pidl) return NULL;

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
/* @@ うまく働かない???
	LPITEMIDLIST pidlFq;
	return (SHGetRealIDL(psf, pidl, &pidlFq) == S_OK) ? pidlFq : NULL;
@@ */
}



////////////////////////////////////////////////////////////////
//  Takes a fully qualified pidl and returns the the relative pidl
//
//	IN: pidlFQ   - Pointer to the fully qualified ITEMIDLIST that needs to be
//				   parsed.
//	RET:         - Points to the pidl that will contain the root after parsing.
//
LPITEMIDLIST __fastcall idl_GetParent(LPCITEMIDLIST pidlFullyQual)
{
	if (!pidlFullyQual) return NULL;

	LPITEMIDLIST pidlParent = ILClone(pidlFullyQual);
	if (pidlParent) {
		if (ILRemoveLastID(pidlParent))
			return pidlParent;
		CoTaskMemFree(pidlParent);
	}
	return NULL;
}

////////////////////////////////////////////////////////////////
//  This routine takes a fully qualified pidl for a folder and returns 
//	the IShellFolder pointer for that pidl
//
//	IN: pidl - Pointer to a fully qualified ITEMIDLIST for the folder
//

IShellFolder* __fastcall idl_GetShellFolder(LPITEMIDLIST pidl)
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

IShellFolder* __fastcall idl_BindToShellFolder(IShellFolder* psf, LPITEMIDLIST pidl)
{
	IShellFolder* psfNew;
	HRESULT hr;
	if (psf)
		hr = psf->BindToObject(pidl, 0, IID_IShellFolder, (void**)&psfNew);
	else
		hr = SHGetDesktopFolder(&psfNew);
	if (SUCCEEDED(hr))
		return psfNew;
	return NULL;
}

LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, LPITEMIDLIST pidlFq, REFIID riid)
{
	void* pv = NULL;
	LPSHELLFOLDER psf;
	LPCITEMIDLIST pidl;
	if (SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, &pidl) == S_OK) {
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
				CoTaskMemFree(pidl);
			return psfDesktop;
		}

		if (pidl) {
			psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID*)&psf);
			if (ppidl)
				*ppidl = pidl;
			else
				CoTaskMemFree(pidl);
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
	CoTaskMemFree(pidlDrives);
}

int __fastcall idl_GetSysIconIndex(LPCTSTR pszFile, UINT flg, DWORD attr)
{
	int index = -1;
	if (pszFile) {
		flg |= SHGFI_SYSICONINDEX;
		if (attr != (DWORD)-1)
			flg |= SHGFI_USEFILEATTRIBUTES;
		else
			attr = 0;

		SHFILEINFO sfi;
		HIMAGELIST him = (HIMAGELIST)SHGetFileInfo(pszFile, attr, &sfi, sizeof(sfi), flg);
		if (him)
			index = sfi.iIcon;
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

int __fastcall idl_GetSysIconIndex(LPITEMIDLIST pidlPar, LPITEMIDLIST pidl, UINT flg)
{
	int index = -1;
	LPITEMIDLIST pidlFq = ILCombine(pidlPar, pidl);
	if (pidlFq) {
		index = idl_GetSysIconIndex(pidlFq, flg);
		CoTaskMemFree(pidlFq);
	}
	return index;
}

int __fastcall idl_GetSysIconIndex(IShellFolder* psf, LPITEMIDLIST pidl, UINT flg)
{
	int index = -1;
/* @@ こういう方法もあるけど、IShellIconをサポートしないフォルダもあるからねぇ
	CComQIPtr<IShellIcon, &IID_IShellIcon> psi(psf);
	if (psi.p != NULL) {
		if (psi->GetIconOf(pidl, flg, &index) != NOERROR)
			index = -1;
	}
@@ */
	LPITEMIDLIST pidlFq = idl_GetFullyQualPidl(psf, pidl);
	if (pidlFq) {
		index = idl_GetSysIconIndex(pidlFq, flg);
		CoTaskMemFree(pidlFq);
	}
	return index;
}

int __fastcall idl_GetShellColumnInfo(IShellFolder* psf, SDTCOLINFO* pcol, int maxarray)
{
	int index = 0;
	if (psf) {
		SHELLDETAILS sdt;
		// IShellFolder2インタフェースで試行
		CComQIPtr<IShellFolder2, &IID_IShellFolder2> psf2(psf);
		if (psf2.p != NULL) {
			while (index < maxarray) {
				if (psf2->GetDetailsOf(NULL, index, &sdt) == S_OK) {
					pcol->iCol = index;
					pcol->fmt = sdt.fmt;
					pcol->cxChar = sdt.cxChar;

					LPTSTR psz;
					if (StrRetToStr(&sdt.str, NULL, &psz) == S_OK) {
						lstrcpyn(pcol->title, psz, 80);
						CoTaskMemFree(psz);
					}
					else
						*pcol->title = 0;

					if (psf2->GetDefaultColumnState(index, &pcol->state) != NOERROR)
						pcol->state = SHCOLSTATE_SECONDARYUI | SHCOLSTATE_TYPE_STR;
					pcol++;
					index++;
				}
				else
					break;
			}
		}
	}
	return index;
}

BOOL __fastcall idl_GetShellColumnText(IShellFolder2* psf2, LPITEMIDLIST pidl, int col, LPTSTR pszText, UINT cchText)
{
	*pszText = NULL;
	SHELLDETAILS sdt;
	if (psf2 && psf2->GetDetailsOf(pidl, col, &sdt) == S_OK) {
		LPTSTR psz;
		if (StrRetToStr(&sdt.str, pidl, &psz) == S_OK) {
			lstrcpyn(pszText, psz, cchText);
			CoTaskMemFree(psz);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL __fastcall idl_GetShellColumnText(IShellDetails* psdt, LPITEMIDLIST pidl, int col, LPTSTR pszText, UINT cchText)
{
	*pszText = NULL;
	SHELLDETAILS sdt;
	if (psdt && psdt->GetDetailsOf(pidl, col, &sdt) == S_OK) {
		LPTSTR psz;
		if (StrRetToStr(&sdt.str, pidl, &psz) == S_OK) {
			lstrcpyn(pszText, psz, cchText);
			CoTaskMemFree(psz);
			return TRUE;
		}
	}
	return FALSE;
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


void dump_descid(int descid)
{
#ifndef SHDID_MOBILE_DEVICE
	#define SHDID_MOBILE_DEVICE         21  // PDA/PalmPC
	#define SHDID_COMPUTER_LIBRARIES    22
	#define SHDID_SHAREPOINT_SITES      23
#endif

	switch (descid) {
	case SHDID_ROOT_REGITEM:		dbg.printf(L"id=SHDID_ROOT_REGITEM\n"); break;
	case SHDID_FS_FILE:				dbg.printf(L"id=SHDID_FS_FILE\n"); break;
	case SHDID_FS_DIRECTORY:		dbg.printf(L"id=SHDID_FS_DIRECTORY\n"); break;
	case SHDID_FS_OTHER:			dbg.printf(L"id=SHDID_FS_OTHER\n"); break;
	case SHDID_COMPUTER_DRIVE35:	dbg.printf(L"id=SHDID_COMPUTER_DRIVE35\n"); break;
	case SHDID_COMPUTER_DRIVE525:	dbg.printf(L"id=SHDID_COMPUTER_DRIVE525\n"); break;
	case SHDID_COMPUTER_REMOVABLE:	dbg.printf(L"id=SHDID_COMPUTER_REMOVABLE\n"); break;
	case SHDID_COMPUTER_FIXED:		dbg.printf(L"id=SHDID_COMPUTER_FIXED\n"); break;
	case SHDID_COMPUTER_NETDRIVE:	dbg.printf(L"id=SHDID_COMPUTER_NETDRIVE\n"); break;
	case SHDID_COMPUTER_CDROM:		dbg.printf(L"id=SHDID_COMPUTER_CDROM\n"); break;
	case SHDID_COMPUTER_RAMDISK:	dbg.printf(L"id=SHDID_COMPUTER_RAMDISK\n"); break;
	case SHDID_COMPUTER_OTHER:		dbg.printf(L"id=SHDID_COMPUTER_OTHER\n"); break;
	case SHDID_NET_DOMAIN:			dbg.printf(L"id=SHDID_NET_DOMAIN\n"); break;
	case SHDID_NET_SERVER:			dbg.printf(L"id=SHDID_NET_SERVER\n"); break;
	case SHDID_NET_SHARE:			dbg.printf(L"id=SHDID_NET_SHARE\n"); break;
	case SHDID_NET_RESTOFNET:		dbg.printf(L"id=SHDID_NET_RESTOFNET\n"); break;
	case SHDID_NET_OTHER:			dbg.printf(L"id=SHDID_NET_OTHER\n"); break;
	case SHDID_COMPUTER_IMAGING:	dbg.printf(L"id=SHDID_COMPUTER_IMAGING\n"); break;
	case SHDID_COMPUTER_AUDIO:		dbg.printf(L"id=SHDID_COMPUTER_AUDIO\n"); break;
	case SHDID_COMPUTER_SHAREDDOCS:	dbg.printf(L"id=SHDID_COMPUTER_SHAREDDOCS\n"); break;
	case SHDID_MOBILE_DEVICE:		dbg.printf(L"id=SHDID_MOBILE_DEVICE\n"); break;
//@	case SHDID_COMPUTER_LIBRARIES:	dbg.printf(L"id=SHDID_COMPUTER_LIBRARIES\n"); break;
//@	case SHDID_SHAREPOINT_SITES:	dbg.printf(L"id=SHDID_SHAREPOINT_SITES\n"); break;
	default:						dbg.printf(L"id=** unknown ** (%d)\n", descid); break;
	}
}

int __fastcall idl_GetDescriptionId(IShellFolder* psf, LPCITEMIDLIST pidl)
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

int __fastcall idl_GetDescriptionId(LPCITEMIDLIST pidlFq)
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

BOOL ILGetAttributesOf(LPITEMIDLIST pidlFq, SFGAOF* pattr)
{
	SHFILEINFO sfi;
	sfi.dwAttributes = *pattr;
	BOOL bOK = (BOOL)SHGetFileInfo((LPCTSTR)pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
	if (bOK) *pattr = sfi.dwAttributes;
	return bOK;
}

HBITMAP __fastcall idl_ExtractImage(IShellFolder* psf, LPITEMIDLIST pidl, SIZE* prgSize, DWORD depth)
{
	if (!psf || !pidl || !prgSize) return NULL;

	HBITMAP hbm = NULL;
	IExtractImage* pie;
	if (psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl, IID_IExtractImage, NULL, (void**)&pie) == S_OK) {
		OLECHAR* wsz = new OLECHAR[MAX_URL_LENGTH];
		DWORD prior = 0;			// IEI_PRIORITY_NORMAL is defined nowhere!
		DWORD flags = IEIFLAG_SCREEN | IEIFLAG_OFFLINE | IEIFLAG_CACHE;
		if (pie->GetLocation(wsz, MAX_URL_LENGTH, &prior, prgSize, depth, &flags) == NOERROR) {
			// even if we've got shell v4.70+, not all files support thumbnails 
			if (pie->Extract(&hbm) != NOERROR)
				hbm = NULL;
		}
		delete [] wsz;
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
	else
		bOK = ILIsDesktop(pidlFq);
	return bOK;
}

BOOL __fastcall idl_SendTo(HWND hwnd, LPITEMIDLIST pidlFq, LPDATAOBJECT pdo)
{
	BOOL bOK = FALSE;
	LPDROPTARGET pdt = (LPDROPTARGET)idl_GetUIObjectOf(hwnd, pidlFq, IID_IDropTarget);
	if (pdt) {
		POINTL pt;
		pt.x = pt.y = 0;
		DWORD eff = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;

		DWORD keyState = MK_LBUTTON;
		if (GetKeyState(VK_CONTROL) < 0)	keyState |= MK_CONTROL;
		if (GetKeyState(VK_SHIFT) < 0)		keyState |= MK_SHIFT;
		pdt->DragEnter(pdo, keyState, pt, &eff);
		pdt->Drop(pdo, keyState, pt, &eff);
		bOK = TRUE;

		pdt->Release();
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
		LPTSTR psz;
		if (StrRetToStr(&str, pidlFq, &psz) == S_OK) {
			ULONG eat, attr = SFGAO_VALIDATE;
			LPITEMIDLIST pidl;
			if (psf->ParseDisplayName(0, 0, psz, &eat, &pidl, &attr) == NOERROR) {
				CoTaskMemFree(pidl);
				bOK = TRUE;
			}
			CoTaskMemFree(psz);
		}
	}
	psf->Release();
	return bOK;
}


////////////////////////////////////////////////////////////////////////
//


////////////////////////////////////////////////////////////////////////
//


#define PT_CPLAPPLET	0x00
#define PT_GUID			0x1F
#define PT_DRIVE		0x23
#define PT_DRIVE2		0x25
#define PT_DRIVE3		0x29
#define PT_SHELLEXT		0x2E
#define PT_DRIVE1		0x2F
#define PT_FOLDER1		0x30
#define PT_FOLDER		0x31
#define PT_VALUE		0x32
#define PT_VALUEW       0x34
#define PT_WORKGRP		0x41
#define PT_COMP			0x42
#define PT_NETPROVIDER  0x46
#define PT_NETWORK		0x47
#define PT_IESPECIAL1	0x61
#define PT_YAGUID		0x70 /* yet another guid.. */
#define PT_IESPECIAL2	0xb1
#define PT_SHARE		0xc3

typedef BYTE PIDLTYPE;

#pragma pack(1)

typedef struct tagPIDLCPanelStruct { 
	BYTE dummy;					/*01 is 0x00 */
	DWORD iconIdx;				/*02 negative icon ID */
	WORD offsDispName;			/*06*/
	WORD offsComment;			/*08*/
	CHAR szName[1];				/*10*/ /* terminated by 0x00, followed by display name and comment string */
} PIDLCPanelStruct;

typedef struct tagGUIDStruct {
	BYTE dummy;					/* offset 01 is unknown */
	GUID guid;					/* offset 02 */
} GUIDStruct;

typedef struct tagDriveStruct {
	CHAR szDriveName[20];		/*01*/
	WORD unknown;				/*21*/
} DriveStruct;

typedef struct tagFileStruct {
	BYTE dummy;					/*01 is 0x00 for files or dirs */
	DWORD dwFileSize;			/*02*/
	WORD uFileDate;				/*06*/
	WORD uFileTime;				/*08*/
	WORD uFileAttribs;			/*10*/
	CHAR szNames[1];			/*12*/
	/* Here are coming two strings. The first is the long name.
	   The second the dos name when needed or just 0x00 */
} FileStruct;

typedef struct tagValueW {
	WCHAR name[1];
} ValueWStruct;

typedef struct tagPIDLDATA {
	PIDLTYPE type;				/*00*/
	union {
		GUIDStruct guid;
		DriveStruct drive;
		FileStruct file;
		struct {
			WORD dummy;			/*01*/
			CHAR szNames[1];	/*03*/
		} network;
		struct {
			WORD dummy;			/*01*/
			DWORD dummy1;		/*02*/
			CHAR szName[1];		/*06*/ /* terminated by 0x00 0x00 */
		} htmlhelp;
		PIDLCPanelStruct cpanel;
		ValueWStruct valueW;
	} u;
} PIDLDATA, *LPPIDLDATA;

#pragma pack()


LPPIDLDATA idl_GetDataPointer(LPCITEMIDLIST pidl)
{
	if (pidl && pidl->mkid.cb != 0)
		return (LPPIDLDATA) &(pidl->mkid.abID);
	return NULL;
}

IID* idl_GetGUIDPointer(LPCITEMIDLIST pidl)
{
	LPPIDLDATA pdata = idl_GetDataPointer(pidl);
	if (!pdata) return NULL;

//	TRACE("pdata->type 0x%04x\n", pdata->type);
	switch (pdata->type) {
	case PT_SHELLEXT:
	case PT_GUID:
	case PT_YAGUID:
		return &(pdata->u.guid.guid);
	}
//	TRACE("Unknown pidl type 0x%04x\n", pdata->type);
	return NULL;
}


BOOL idl_IsMyComputer(LPCITEMIDLIST pidl)
{
//@	REFIID iid = idl_GetGUIDPointer(pidl);
	IID* piid = idl_GetGUIDPointer(pidl);
	if (piid)
		return IsEqualIID(*piid, CLSID_MyComputer);
	return FALSE;
}

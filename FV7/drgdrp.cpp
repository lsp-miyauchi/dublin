////////////////////////////////////////////////////////////////////////
// drgdrp.cpp

#include "stdafx.h"
#include "resource.h"
#include "fv7.h"
#include "cdll.h"
#include "data.h"
#include "idlist.h"
#include "fileitem.h"
#include "dragdrop.h"
#include "shellexec.h"
#include "common.h"
#include "func.h"

#include "atten.cxx"

////////////////////////////////////////////////////////////////////////
// クリップボードフォーマット

#define CFSTR_DRAGIMAGEBITS			_T("DragImageBits")
#define CFSTR_INFILEVISORDRAGLOOP	_T("InFileVisorDragLoop")

CLIPFORMAT nRCF_SHELLIDLIST			= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
CLIPFORMAT nRCF_SHELLIDLISTOFFSET	= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLISTOFFSET);
CLIPFORMAT nRCF_FILENAMEA			= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILENAMEA);
CLIPFORMAT nRCF_FILENAMEW			= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILENAMEW);
CLIPFORMAT nRCF_FILENAMEMAPA		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILENAMEMAPA);
CLIPFORMAT nRCF_FILENAMEMAPW		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILENAMEMAPW);
CLIPFORMAT nRCF_FILECONTENTS		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
CLIPFORMAT nRCF_FILEDESCRIPTOR		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
CLIPFORMAT nRCF_PREFERREDDROPEFFECT	= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
//@ CLIPFORMAT nRCF_HYPERLINK			= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_HYPERLINK);
CLIPFORMAT nRCF_SHELLURL			= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLURL);
CLIPFORMAT nRCF_MOUNTEDVOLUME		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_MOUNTEDVOLUME);
CLIPFORMAT nRCF_NETRESOURCES		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_NETRESOURCES);
CLIPFORMAT nRCF_INDRAGLOOP			= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_INDRAGLOOP);
CLIPFORMAT nRCF_LOGICALPERFORMEDDROPEFFECT
									= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_LOGICALPERFORMEDDROPEFFECT);
CLIPFORMAT nRCF_PASTESUCCEEDED		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PASTESUCCEEDED);
CLIPFORMAT nRCF_PERFORMEDDROPEFFECT	= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);
CLIPFORMAT nRCF_TARGETCLSID			= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_TARGETCLSID);
CLIPFORMAT nRCF_DRAGIMAGEBITS		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DRAGIMAGEBITS);
CLIPFORMAT nRCF_DROPDESCRIPTION		= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DROPDESCRIPTION);
CLIPFORMAT nRCF_INFILEVISORDRAGLOOP	= (CLIPFORMAT)RegisterClipboardFormat(CFSTR_INFILEVISORDRAGLOOP);

CLIPFORMAT nRCF_MOZ_MESSAGE			= (CLIPFORMAT)RegisterClipboardFormat(_T("text/x-moz-message"));		// thumderbird
// CLIPFORMAT nRCF_MOZ_URL				= (CLIPFORMAT)RegisterClipboardFormat(_T("text/x-moz-url"));			// firefox



////////////////////////////////////////////////////////////////////////
//
#define GMSHARE		(GMEM_SHARE | GMEM_ZEROINIT)
#define SDO_CANDROP	(SDO_HDROP | SDO_SHELLIDLIST | SDO_FILEDESCRIPTOR | SDO_FILECONTENTS | SDO_SHELLURL)


////////////////////////////////////////////////////////////////////////
//

DROPEFFECT GetDropEffect(DWORD eff, DWORD keyState, BOOL bEnableForceOverWrite)
{
	BOOL ctrl  = (keyState & MK_CONTROL);
	BOOL shift = (keyState & MK_SHIFT);
	BOOL alt   = (keyState & MK_ALT);

	eff &= (DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK);
	if (eff == (DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK)) {
		if (bEnableForceOverWrite && alt && (ctrl || shift))
			return ctrl ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
		return shift ? (ctrl ? DROPEFFECT_LINK : DROPEFFECT_MOVE) : DROPEFFECT_COPY;
	} else if (eff == (DROPEFFECT_COPY | DROPEFFECT_LINK)) {
		if (bEnableForceOverWrite && alt && ctrl)
			return DROPEFFECT_COPY;
		return ctrl ? DROPEFFECT_LINK : DROPEFFECT_COPY;
	} else if (eff == (DROPEFFECT_MOVE | DROPEFFECT_LINK)) {
		if (bEnableForceOverWrite && alt && shift)
			return DROPEFFECT_MOVE;
		return ctrl ? DROPEFFECT_MOVE : DROPEFFECT_MOVE;
	} else if (eff == (DROPEFFECT_COPY | DROPEFFECT_MOVE)) {
		if (bEnableForceOverWrite && alt && (ctrl || shift))
			return ctrl ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
		return shift ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
	}
	return (eff == 0) ? DROPEFFECT_NONE : eff;
}

////////////////////////////////////////////////////////////////////////
//

BOOL AcceptFiles(ACCEPTFILESINFO* pafi)
{
	return AcceptFiles(pafi->hdrop, pafi->dropEffect, pafi->pszDestination, pafi->bThreaded, pafi->bAsAdmin, pafi->bForceOverwrite);
}


BOOL AcceptFiles(HDROP hDrop, DROPEFFECT eff, LPCTSTR pszPath, BOOL bThreaded, BOOL bAsAdmin, BOOL bForceOverwrite)
{
	if (!hDrop || !pszPath) return FALSE;

	__attension_app();

	if (GetForegroundWindow() != fvApp.wndMain)
		SetForegroundWindow(fvApp.wndMain);

	RegSaveHistory(IDS_SEC_HISTORY, IDS_KEY_COPYTARGET, MAXACCESS, pszPath);

	BOOL bOK = FALSE;
	if (!bThreaded) {
		CComPtr<ILSFileOperation> pfo;
		HRESULT hr = CoCreateInstanceAsAdmin(fvApp.wndMain, bAsAdmin, __uuidof(LSFileOperation), __uuidof(ILSFileOperation), (void**)&pfo);
		if (hr == S_OK) {
			if (bForceOverwrite) {
				DROPFILEOPSTRUCT dfos = {0};
				dfos.hwnd = fvApp.wndMain;
				dfos.hdrop = hDrop;
				dfos.pszDestination = pszPath;
				dfos.dropEffect = eff;
				dfos.bSaveFolderHierarchy = TRUE;
				dfos.bYesToAll = TRUE;
				dfos.fAction = FAC_OVERWRITE;
				hr = pfo->DropFileOperationEx((LPARAM)&dfos);
			} else {
				hr = pfo->DropFileOperation(fvApp.wndMain, hDrop, eff, pszPath, TRUE);
			}
		} else {
			lccErrorBox(fvApp.wndMain, hr, APP_PRODUCT, NULL, MB_STOP);
		}
		bOK = (hr == S_OK);
	} else {
		// 別プロセスで処理する
		DROPFILEOPERATION dfo;
		dfo.cbSize = sizeof(dfo);
		dfo.hwnd = NULL;
		dfo.hDrop = hDrop;
		dfo.dwEffect = eff;
		dfo.pszDropPath = pszPath;
		dfo.bAsAdmin = bAsAdmin;
		dfo.bAsyncOperation = bThreaded;
		dfo.bCopyAccessRights = FALSE;
		dfo.bSaveFolderHierarchy = TRUE;
		bOK = fopDropFileOperation(&dfo);
	}
	return bOK;
}

////////////////////////////////////////////////////////////////////////
//

typedef struct tagACCEPTDATAOBJECTINFO {
	HWND hwnd;
	LPITEMIDLIST pidl;
	IDataObject* pdo;
	DWORD keyState;
	DROPEFFECT eff;
	POINT pt;
} ACCEPTDATAOBJECTINFO;

DROPEFFECT AcceptDataObject(HWND hwnd, LPCITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DROPEFFECT eff, POINT pt)
{
	DROPEFFECT ret = DROPEFFECT_NONE;
	if (pidl && pdo) {
		ret = idl_Execute(hwnd, pidl, pdo, keyState, eff, pt);
	}
	return ret;
}


////////////////////////////////////////////////////////////////////////
//

static CLIPFORMAT nRCF_HDROP = CF_HDROP;
static CLIPFORMAT nRCF_UNICODETEXT = CF_UNICODETEXT;

typedef struct tagCHECKCLIPFORMAT {
	CLIPFORMAT cf;
	DWORD flag;
} CHECKCLIPFORMAT;

static const CHECKCLIPFORMAT sstCCF[] = {
	nRCF_HDROP,						SDO_HDROP,
	nRCF_SHELLIDLIST,				SDO_SHELLIDLIST,
	nRCF_SHELLIDLISTOFFSET,			SDO_SHELLIDLISTOFFSET,
	nRCF_FILENAMEA,					SDO_FILENAMEA,
	nRCF_FILENAMEW,					SDO_FILENAMEW,
	nRCF_FILENAMEMAPA,				SDO_FILENAMEMAPA,
	nRCF_FILENAMEMAPW,				SDO_FILENAMEMAPW,
	nRCF_FILEDESCRIPTOR,			SDO_FILEDESCRIPTOR,
	nRCF_FILECONTENTS,				SDO_FILECONTENTS,
	nRCF_PREFERREDDROPEFFECT,		SDO_PREFERREDDROPEFFECT,
	nRCF_LOGICALPERFORMEDDROPEFFECT,SDO_LOGICALPERFORMEDDROPEFFECT,
	nRCF_SHELLURL,					SDO_SHELLURL,
	nRCF_MOUNTEDVOLUME,				SDO_MOUNTEDVOLUME,
	nRCF_NETRESOURCES,				SDO_NETRESOURCES,
//@	nRCF_HYPERLINK,					SDO_HYPERLINK,
	nRCF_INDRAGLOOP,				SDO_INDRAGLOOP,
	nRCF_PASTESUCCEEDED,			SDO_PASTESUCCEEDED,
	nRCF_PERFORMEDDROPEFFECT,		SDO_PERFORMEDDROPEFFECT,
	nRCF_TARGETCLSID,				SDO_TARGETCLSID,
	nRCF_DRAGIMAGEBITS,				SDO_DRAGIMAGEBITS,
	nRCF_INFILEVISORDRAGLOOP,		SDO_INFILEVISORDRAGLOOP,
	nRCF_UNICODETEXT,				SDO_UNICODETEXT,
//@	nRCF_MOZ_MESSAGE,				SDO_MOZ_MESSAGE,
	0,								0,
};


DWORD GetDataObjectFormats(IDataObject* pdo)
{
	DWORD type = 0;
	CComPtr<IEnumFORMATETC> pef;
	FORMATETC fe;
	if (pdo->EnumFormatEtc(DATADIR_GET, &pef) == S_OK) {
		while (pef->Next(1, &fe, NULL) == S_OK) {
			if (fe.ptd) CoTaskMemFree(fe.ptd);
			if ((fe.tymed != TYMED_HGLOBAL) || !(fe.dwAspect & DVASPECT_CONTENT)) continue;

			if      (fe.cfFormat == CF_HDROP)						type |= SDO_HDROP;
			else if (fe.cfFormat == nRCF_SHELLIDLIST)				type |= SDO_SHELLIDLIST;
			else if (fe.cfFormat == nRCF_SHELLIDLISTOFFSET)			type |= SDO_SHELLIDLISTOFFSET;
			else if (fe.cfFormat == nRCF_FILENAMEA)					type |= SDO_FILENAMEA;
			else if (fe.cfFormat == nRCF_FILENAMEW)					type |= SDO_FILENAMEW;
			else if (fe.cfFormat == nRCF_FILENAMEMAPA)				type |= SDO_FILENAMEMAPA;
			else if (fe.cfFormat == nRCF_FILENAMEMAPW)				type |= SDO_FILENAMEMAPW;
			else if (fe.cfFormat == nRCF_FILEDESCRIPTOR)			type |= SDO_FILEDESCRIPTOR;
			else if (fe.cfFormat == nRCF_FILECONTENTS)				type |= SDO_FILECONTENTS;
			else if (fe.cfFormat == nRCF_PREFERREDDROPEFFECT)		type |= SDO_PREFERREDDROPEFFECT;
			else if (fe.cfFormat == nRCF_LOGICALPERFORMEDDROPEFFECT)type |= SDO_LOGICALPERFORMEDDROPEFFECT;
			else if (fe.cfFormat == nRCF_SHELLURL)					type |= SDO_SHELLURL;
			else if (fe.cfFormat == nRCF_MOUNTEDVOLUME)				type |= SDO_MOUNTEDVOLUME;
			else if (fe.cfFormat == nRCF_NETRESOURCES)				type |= SDO_NETRESOURCES;
//@			else if (fe.cfFormat == nRCF_HYPERLINK)					type |= SDO_HYPERLINK;
			else if (fe.cfFormat == nRCF_INDRAGLOOP)				type |= SDO_INDRAGLOOP;
			else if (fe.cfFormat == nRCF_PASTESUCCEEDED)			type |= SDO_PASTESUCCEEDED;
			else if (fe.cfFormat == nRCF_PERFORMEDDROPEFFECT)		type |= SDO_PERFORMEDDROPEFFECT;
			else if (fe.cfFormat == nRCF_TARGETCLSID)				type |= SDO_TARGETCLSID;
			else if (fe.cfFormat == nRCF_UNICODETEXT)				type |= SDO_UNICODETEXT;
			else if (fe.cfFormat == nRCF_DRAGIMAGEBITS)				type |= SDO_DRAGIMAGEBITS;
			else if (fe.cfFormat == nRCF_INFILEVISORDRAGLOOP)		type |= SDO_INFILEVISORDRAGLOOP;
//@			else if (fe.cfFormat == nRCF_MOZ_MESSAGE)				type |= SDO_MOZ_MESSAGE;
		}
	}
	else {
		// IEnumFORMATETC インタフェースを取得できないときは直接チェックする
		int i = 0;
		while (sstCCF[i].cf) {
			SETFormatEtc(fe, sstCCF[i].cf, TYMED_HGLOBAL, -1);
			if (pdo->QueryGetData(&fe) == S_OK) type |= sstCCF[i].flag;
			i++;
		}
	}
// dbg.printf(L"GetDataObjectFormats: %08X\n", type);
	return type;
}

bool QueryDataObjectFormat(IDataObject* pdo, CLIPFORMAT cf)
{
	FORMATETC fe;
	SETFormatEtc(fe, cf, TYMED_HGLOBAL, -1);
	return (pdo->QueryGetData(&fe) == S_OK);
}

bool QueryDataObjectFormat(IDataObject* pdo, LPCTSTR pszClipFormatString)
{
	UINT cf = RegisterClipboardFormat(pszClipFormatString);
	return QueryDataObjectFormat(pdo, (CLIPFORMAT)cf);
}




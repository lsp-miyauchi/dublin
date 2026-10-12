////////////////////////////////////////////////////////////////
// drgdrp.h

#pragma once
#ifndef __DRGDRP_H__
#define __DRGDRP_H__

#include "resource.h"
#include "tiplabel.h"

////////////////////////////////////////////////////////////////////////
// クリップボードフォーマット

extern CLIPFORMAT nRCF_SHELLIDLIST;
extern CLIPFORMAT nRCF_SHELLIDLISTOFFSET;
extern CLIPFORMAT nRCF_FILENAMEA;
extern CLIPFORMAT nRCF_FILENAMEW;
extern CLIPFORMAT nRCF_FILENAMEMAPA;
extern CLIPFORMAT nRCF_FILENAMEMAPW;
extern CLIPFORMAT nRCF_FILECONTENTS;
extern CLIPFORMAT nRCF_FILEDESCRIPTOR;
extern CLIPFORMAT nRCF_PREFERREDDROPEFFECT;
//@ extern CLIPFORMAT nRCF_HYPERLINK;
extern CLIPFORMAT nRCF_SHELLURL;
extern CLIPFORMAT nRCF_MOUNTEDVOLUME;
extern CLIPFORMAT nRCF_NETRESOURCES;
extern CLIPFORMAT nRCF_INDRAGLOOP;
extern CLIPFORMAT nRCF_LOGICALPERFORMEDDROPEFFECT;
extern CLIPFORMAT nRCF_PASTESUCCEEDED;
extern CLIPFORMAT nRCF_PERFORMEDDROPEFFECT;
extern CLIPFORMAT nRCF_TARGETCLSID;
extern CLIPFORMAT nRCF_DRAGIMAGEBITS;
extern CLIPFORMAT nRCF_DROPDESCRIPTION;
extern CLIPFORMAT nRCF_INFILEVISORDRAGLOOP;


////////////////////////////////////////////////////////////////////////
//

#define MK_ALTMENU							0x0020

#define SDO_HDROP							0x00000001
#define SDO_SHELLIDLIST						0x00000002
#define SDO_SHELLIDLISTOFFSET				0x00000004
#define SDO_FILENAMEA						0x00000008
#define SDO_FILENAMEW						0x00000010
#define SDO_FILENAMEMAPA					0x00000020
#define SDO_FILENAMEMAPW					0x00000040
#define SDO_FILECONTENTS					0x00000080
#define	SDO_FILEDESCRIPTOR					0x00000100
#define	SDO_PREFERREDDROPEFFECT				0x00000200
#define	SDO_SHELLURL						0x00000400
#define SDO_MOUNTEDVOLUME					0x00000800
#define SDO_NETRESOURCES					0x00001000
#define SDO_INDRAGLOOP						0x00002000
#define SDO_LOGICALPERFORMEDDROPEFFECT		0x00004000
#define SDO_PASTESUCCEEDED					0x00008000
#define SDO_PERFORMEDDROPEFFECT				0x00010000
#define SDO_TARGETCLSID						0x00020000
#define SDO_UNICODETEXT						0x00040000
#define SDO_MOZ_MESSAGE						0x01000000		// thunderbird
#define SDO_DRAGIMAGEBITS					0x40000000		// undoc
#define SDO_INFILEVISORDRAGLOOP				0x80000000		// FileVisorでドラッグ中

#define SDO_DVASPECT_MASK					0x0F000000
#define SDO_DVASPECT_CONTENT				0x01000000
#define SDO_DVASPECT_THUMBNAIL				0x02000000
#define SDO_DVASPECT_ICON					0x04000000
#define SDO_DVASPECT_DOCPRINT				0x08000000
#define GetDVASPECTValue(a)					(((a)&SDO_DVASPECT_MASK)>>24)

#define SDO_TYMED_MASK						0xF0000000
#define SDO_TYMED_HGLOBAL					0x10000000
#define SDO_TYMED_FILE						0x20000000
#define SDO_TYMED_ISTREAM					0x40000000
#define SDO_TYMED_ISTORAGE					0x80000000
#define GetTYMEDValue(a)					(((a)&SDO_TYMED_MASK)>>28)


#define PREFERREDEFFECT_COPY				0x00000005
#define PREFERREDEFFECT_MOVE				0x00000002

#define _GetPIDLFolder(pida)				(LPITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define _GetPIDLItem(pida,i)				(LPITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

#define SETFormatEtc(fe, cf, med, li)\
	{\
		(fe).cfFormat = cf;\
		(fe).dwAspect = DVASPECT_CONTENT;\
		(fe).ptd = NULL;\
		(fe).tymed = med;\
		(fe).lindex = li;\
	};

////////////////////////////////////////////////////////////////
//

typedef struct __ACCEPTFILESINFO {
	HDROP hdrop;
	DROPEFFECT dropEffect;
	LPCTSTR pszDestination;
	BOOL bThreaded;
	BOOL bAsAdmin;
	BOOL bForceOverwrite;
} ACCEPTFILESINFO;

BOOL AcceptFiles(ACCEPTFILESINFO* pafi);
BOOL AcceptFiles(HDROP hDrop, DROPEFFECT dwEffect, LPCTSTR pszDropPath, BOOL bThreaded = FALSE, BOOL bAsAdmin = FALSE, BOOL bForceOverwrite = FALSE);
DROPEFFECT AcceptDataObject(HWND hwnd, LPCITEMIDLIST pidl, IDataObject* pdo, DWORD keyState, DROPEFFECT eff, POINT pt);
DWORD GetDataObjectFormats(IDataObject* pdo);


////////////////////////////////////////////////////////////////////////
//

#pragma pack(1)

typedef struct tagINFILEVISORDRAGLOOP {
	DWORD magic;
	UINT itemcount;
	UINT flag;
	LPARAM param;
} INFILEVISORDRAGLOOP;

#pragma pack()


////////////////////////////////////////////////////////////////////////
//

DROPEFFECT GetDropEffect(DWORD eff, DWORD keyState, BOOL bForceOverWrite = FALSE);

inline HGLOBAL GetDataFromDataObject(IDataObject* pdo, CLIPFORMAT cf)
{
	HGLOBAL hmem = NULL;
	FORMATETC fe;
	SETFormatEtc(fe, cf, TYMED_HGLOBAL, -1);
	if (pdo && pdo->QueryGetData(&fe) == S_OK) {
		STGMEDIUM sm;
		if (pdo->GetData(&fe, &sm) == S_OK) {
			hmem = lccDuplicateMemoryHandle(sm.hGlobal, GMEM_SHARE | GMEM_MOVEABLE);
			ReleaseStgMedium(&sm);
		}
	}
	return hmem;
}


////////////////////////////////////////////////////////////////////////
// 

class TDropTargetHandler
{
public:
	virtual DWORD OnTargetDragEnter(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
	virtual DWORD OnTargetDragOver(IDataObject* pDataObject, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
	virtual void OnTargetDragLeave()
	{
	}
	virtual DWORD OnTargetDrop(IDataObject* pDataObject, DWORD keyState, DROPEFFECT dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}
};


////////////////////////////////////////////////////////////////
// TDropTarget

class TDropTarget : public IDropTarget
{
private:
	IDataObject* m_pDataObject;
	ULONG m_refs;
	TDropTargetHandler* m_pTargetHandler;
	DWORD m_keyState;
	DWORD m_dropEffect;

public:
	static void CreateInstance(IDropTarget** pp, TDropTargetHandler* pHandler)
	{
		if (pp != NULL) {
			TDropTarget* p = new TDropTarget(pHandler);
			p->QueryInterface(IID_IDropTarget, (void**)pp);
		}
	}

	TDropTarget(TDropTargetHandler* pHandler)
		: m_refs(1)
		, m_pTargetHandler(pHandler)
		, m_keyState(0)
		, m_dropEffect(0)
	{
	}

	// -------------------------------
	// IUnknown methods
	// -------------------------------
	STDMETHODIMP QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
	{
		if (riid == IID_IUnknown || riid == IID_IDropTarget) {
			*ppvObj = this;
			AddRef();
			return NOERROR;
		}
		*ppvObj = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return ++m_refs;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		if (--m_refs == 0) {
			delete this;
			return 0;
		}
		return m_refs;
	}

	// -------------------------------
	// IDropTarget methods
	// -------------------------------
	STDMETHODIMP DragEnter(IDataObject* pDataObj, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		m_pDataObject = pDataObj;

		// デフォルト動作をコピーにするため、コピー可であれば
		// ctrlキー押下を模倣
		if (!(keyState & (MK_CONTROL | MK_SHIFT | MK_ALT)) && *pdwEffect & DROPEFFECT_COPY)
			keyState |= MK_CONTROL;
		m_keyState = keyState;

		POINT point;
		point.x = pt.x;
		point.y = pt.y;
		m_dropEffect = m_pTargetHandler->OnTargetDragEnter(m_pDataObject, keyState, *pdwEffect, point);
		*pdwEffect = m_dropEffect;
		return S_OK;
	}

	STDMETHODIMP DragOver(DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		// デフォルト動作をコピーにするため、コピー可であれば
		// ctrlキー押下を模倣
		if (!(keyState & (MK_CONTROL | MK_SHIFT | MK_ALT)) && *pdwEffect & DROPEFFECT_COPY)
			keyState |= MK_CONTROL;
		m_keyState = keyState;

		POINT point;
		point.x = pt.x;
		point.y = pt.y;
		m_dropEffect = m_pTargetHandler->OnTargetDragOver(m_pDataObject, keyState, *pdwEffect, point);
		*pdwEffect = m_dropEffect;
		return S_OK;
	}

	STDMETHODIMP DragLeave()
	{
		m_pTargetHandler->OnTargetDragLeave();
		return S_OK;
	}

	STDMETHODIMP Drop(IDataObject* pdo, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
	{
		if (*pdwEffect != DROPEFFECT_NONE) {
			// 左ボタンが押されていれば、DragOver時に取得した効果を設定
			if (m_keyState & MK_LBUTTON)
				*pdwEffect = m_dropEffect;

			POINT point;
			point.x = pt.x;
			point.y = pt.y;
			*pdwEffect = m_pTargetHandler->OnTargetDrop(pdo, m_keyState, *pdwEffect, point);
		}
		else
			*pdwEffect = DROPEFFECT_NONE;

		DragLeave();
		return S_OK;
	}
};


////////////////////////////////////////////////////////////////
// TDropSource

HCURSOR ImageList_GetCursor(HIMAGELIST, int, UINT, const POINT*);

typedef struct tagDRAGIMAGEBITSSTRUCT {
	DWORD width;									// イメージの幅
	DWORD height;									// イメージの高さ
	DWORD xoffset;									// X カーソルオフセット
	DWORD yoffset;									// Y カーソルオフセット
	DWORD handle;									//
	DWORD clrkey;									// カラーキー
	BYTE data[1];
} DRAGIMAGEBITSSTRUCT;

class TDropSource : public IDropSource
{
private:
	ULONG m_refs;
	int m_numObjects;
	CComPtr<IDragSourceHelper> m_pHelper;
	CComPtr<IDataObject> m_pdo;
	CSimpleArray<CCursor> m_cursors;
	CTipLabelWnd m_tip;

public:
	static void CreateInstance(IDropSource** pp, HWND hwnd, IDataObject* pdo, int numObjects)
	{
		if (pp != NULL) {
			TDropSource* p = new TDropSource(hwnd, pdo, numObjects);
			p->QueryInterface(IID_IDropSource, (void**)pp);
		}
	}

	TDropSource(HWND hwnd, IDataObject* pdo, int numObjects)
		: m_numObjects(numObjects)
	{
		// データオブジェクトの参照カウント増
		m_pdo = pdo;
		// フィードバックカーソル作成
		HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_DRAGDROP_CURSOR), 32, RGB(255, 0, 255));
		if (him) {
			for (int i = 0; i < 4; i++) {
				CCursor cc(ImageList_GetCursor(him, i, ILD_TRANSPARENT, NULL));
				m_cursors.Add(cc);
			}
			ImageList_Destroy(him);
		}

		m_refs = 1;
		HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pHelper));
		if (hr == S_OK) {
			if (IsWindow(hwnd)) {
				CPoint pt(0, 0);
				m_pHelper->InitializeFromWindow(hwnd, &pt, pdo);
			}
		}

		// ツールラベル
		m_tip.Create();
	}
		
	~TDropSource()
	{
		if (m_tip.IsWindow())
			m_tip.DestroyWindow();
	}

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
	{
		if (riid == IID_IUnknown ||
			riid == IID_IDropSource) {
			*ppvObj = this;
			AddRef();
			return NOERROR;
		}
		*ppvObj = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return ++m_refs;
	}

	STDMETHODIMP_(ULONG) Release(void)
	{
		if (--m_refs == 0) {
			delete this;
			return 0;
		}
		return m_refs;
	}

	// IDropSource methods
	STDMETHODIMP QueryContinueDrag(BOOL fEsc, DWORD keyState)
	{
		HRESULT hr = S_OK;
		if (fEsc)
			hr = ResultFromScode(DRAGDROP_S_CANCEL);
		else if (!(keyState & (MK_LBUTTON | MK_RBUTTON)))
			hr = ResultFromScode(DRAGDROP_S_DROP);
		if ((hr != S_OK) && m_tip.IsWindow())
			m_tip.ShowWindow(SW_HIDE);
		return hr;
	}

	STDMETHODIMP GiveFeedback(DWORD eff)
	{
		int index = -1;						// カーソルインデクス

		CPoint point;
		GetCursorPos(&point);
		if (m_tip.IsWindow()) {
			m_tip.SetWindowPos(NULL, point.x + 10, point.y + 13, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

			// DROPDESCRIPTIONへの対応
			FORMATETC fe = { nRCF_DROPDESCRIPTION, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM stm;
			if (m_pdo->GetData(&fe, &stm) == S_OK) {
				DROPDESCRIPTION desc;
				memset(&desc, 0, sizeof(desc));
				desc.type = DROPIMAGE_INVALID;
				DROPDESCRIPTION* pdd = (DROPDESCRIPTION*)::GlobalLock(stm.hGlobal);
				if (pdd) {
					switch (pdd->type) {
					case DROPIMAGE_NONE: index = 0; break;
					case DROPIMAGE_COPY: index = 1; break;
					case DROPIMAGE_MOVE: index = 2; break;
					case DROPIMAGE_LINK: index = 3; break;
					}
					memcpy(&desc, pdd, sizeof(desc));
					GlobalUnlock(stm.hGlobal);
				}
				ReleaseStgMedium(&stm);
				m_tip.SetDropDescription(&desc);
			}
		}

		if (index < 0) {
			if (eff & DROPEFFECT_COPY)	index = 1;
			else if (eff & DROPEFFECT_MOVE)	index = 2;
			else if (eff & DROPEFFECT_LINK)	index = 3;
			else                           	index = 0;
		}

		::SetCursor(m_cursors[index]);
		return ResultFromScode(S_OK);
	}
};


#endif			// __DRGDRP_H__

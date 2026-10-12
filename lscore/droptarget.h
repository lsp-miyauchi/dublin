////////////////////////////////////////////////////////////////
// droptarget.h

#ifndef DROPEFFECT
	typedef DWORD DROPEFFECT;
#endif

extern UINT nRM_DRAGENTER;
extern UINT nRM_DRAGOVER;
extern UINT nRM_DRAGLEAVE;
extern UINT nRM_DROP;


#define SETFormatEtc(fe, cf, med, li)\
		{\
			(fe).cfFormat = cf;\
			(fe).dwAspect = DVASPECT_CONTENT;\
			(fe).ptd = NULL;\
			(fe).tymed = med;\
			(fe).lindex = li;\
		};

// ローカルＤ＆Ｄメッセージへの応答
#define LDD_ERROR				-1
#define LDD_OK					4440

typedef struct tagLLDROPOBJCT
{
	POINT point;
	DWORD keyState;
	DROPEFFECT dropEffect;
	IDataObject* pDataObject;
	IDropTargetHelper* pHelper;
} LLDROPOBJECT, *LPLLDROPOBJECT;


////////////////////////////////////////////////////////////////
//

LPDROPTARGET __fastcall BeginDropTarget(HWND hWndOwner);
void __fastcall EndDropTarget(HWND hWndOwner, LPDROPTARGET pDropTarget);
DROPEFFECT __fastcall GetDropEffect(DWORD dwSupportEffect, DWORD keyState);


////////////////////////////////////////////////////////////////
// CDropTarget

class CDropTarget : public IDropTarget
{
public:
	CDropTarget(HWND hwnd);

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// IDropTarget methods
	STDMETHODIMP DragEnter(LPDATAOBJECT pDataObj, DWORD keyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHODIMP DragOver(DWORD keyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHODIMP DragLeave();
	STDMETHODIMP Drop(LPDATAOBJECT pDataObj, DWORD keyState, POINTL pt, LPDWORD pdwEffect);

private:
	LPDATAOBJECT m_pDataObject;
	ULONG m_refs;
	HWND m_hWnd;
	IDropTargetHelper* m_pHelper;
	DWORD m_dropEffect;
	DWORD m_keyState;
};

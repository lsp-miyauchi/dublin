/////////////////////////////////////////////////////////////////////////////
// ローカルＤ＆Ｄ
//
#define __LOCALDD_H__

/////////////////////////////////////////////////////////////////////////////
// ローカルＤ＆Ｄメッセージ
//
extern const UINT nRM_DRAGENTER;
extern const UINT nRM_DRAGOVER;
extern const UINT nRM_DRAGLEAVE;
extern const UINT nRM_DROP;
extern const UINT nRM_PACKITEM;
extern const UINT nRM_QUERYPATH;

// ローカルＤ＆Ｄメッセージへの応答
#define LDD_ERROR			-1
#define LDD_OK				4440


/////////////////////////////////////////////////////////////////////////////
// DRAG & DROP IMPLEMENTATION
//

#ifndef DROPEFFECT_NONE
	#define DROPEFFECT_NONE	0
	#define DROPEFFECT_COPY	1
	#define DROPEFFECT_MOVE	2
	#define DROPEFFECT_LINK	4
#endif

#pragma pack(4)

typedef struct _FVDRAGINFO {
	DROPFILES stDropFiles;
    DWORD dwEffect;
    DWORD grfKeyState;
    HWND hwndSource;
} FVDRAGINFO, FAR* LPFVDRAGINFO;

#pragma pack()


/////////////////////////////////////////////////////////////////////////////
// DRAG & DROP IMPLEMENTATION
//

int EnumDropItem(HDROP hDrop, CStringing& csFiles);


/////////////////////////////////////////////////////////////////////////////
// class DRAG & DROP
//

class CLocalDD
{
public:
	CLocalDD() {}
	~CLocalDD() {}

	BOOL BeginDrag(CWnd* pWnd, CPoint pt);
	void DoDragDrop(CWnd* pWnd, CFileArray& array);
};

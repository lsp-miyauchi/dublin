// quick.h : header file
//

#include "cmwnd.h"

#pragma pack(1)

#define QUICKMAGIC			0xFF81
typedef struct tagQISTRUCT {
	WORD magic;
	WORD lenName;
	WORD lenPidl;
	LPTSTR GetNamePtr()			{ return (LPTSTR)((BYTE*)this + sizeof(QISTRUCT)); }
	LPITEMIDLIST GetIDListPtr()	{ return (LPITEMIDLIST)((BYTE*)GetNamePtr() + lenName * sizeof(TCHAR)); }
} QISTRUCT;

#pragma pack()

/////////////////////////////////////////////////////////////////////////////
// CFavoriteArray

class CFavoriteItemArray : public CCmItemArray
{
public:
	int Load();
	void Save();
};

////////////////////////////////////////////////////////////////
// class CFavoWnd

enum {
	FavoOpenDefault = 0,
	FavoOpenByNewTab,
	FavoOpenByNewWindow,
};

class CFavoWnd : public CIDListWndImpl<CFavoWnd>
{
public:
	bool m_bShift;
	bool m_bCtrl;
private:
	int m_cxChar;
	int m_cyChar;

public:
	CFavoWnd();

BEGIN_MSG_MAP(CFavoWnd)
	CHAIN_MSG_MAP(CIDListWndImpl<CFavoWnd>)
END_MSG_MAP()

	virtual void DrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpdi);
	virtual void MeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpmi);
	virtual LRESULT MenuChar(UINT nChar, UINT nFlags, HMENU hMenu);
};

LPITEMIDLIST TrackFavoriteMenu(HWND hOwner, bool bAddTabFolder, bool bAddCopyHistory, CPoint pt, int& openMode);


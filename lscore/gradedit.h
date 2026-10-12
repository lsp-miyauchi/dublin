//////////////////////////////////////////////////////////////////////
// gradedit.h CGradEdit control
//

#ifndef __GRADEDIT_INCLUDE

#define WM_RECALCPARENT     0x0368  // force RecalcLayout on frame window

#define GRADITEM_WIDTH		11
#define GRADITEM_HEIGHT		17


class CGradItem
{
public:
	CGradItem();
	~CGradItem() {}

	CGradItem* m_next;
	BOOL m_fixed;
	COLORREF m_cr;
	int m_pos;
	BOOL m_hide;
};

enum {
	htGradBar,
	htGradItem,
	htGradSpace,
	htClient,
	htError
};

class CGradEdit : public CWindow
{
public:
	CGradEdit();
	virtual ~CGradEdit();

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
// CWnd インプリメンテーション
	LRESULT Default();

// Attributes
public:

// Operations
public:

// Implementation
protected:
	BOOL m_bCaptured;
	MSG m_msg;
	CGradItem* m_list;
	int m_numlist;
	double m_scale;
	HIMAGELIST m_him;
	CGradItem* m_pDrag;
	CPoint m_ptBase;
	int m_numlimits;
	COLORREF m_crRef;

public:
	CToolTipCtrl m_tip;
	CWindow m_parent;

protected:
	virtual void OnDraw(HDC hdc);      // pass on pure virtual

private:
	int HitTest(CPoint point);
	int GetItemFromPoint(CPoint point, CGradItem** ppItem);
	void CreateToolTip();

protected:
	// メッセージマップ
	int OnCreate(LPCREATESTRUCT lpcs);
	BOOL OnSetCursor(HWND hwnd, UINT nHitTest, UINT message);
	void OnDestroy();
	void OnSize(UINT nType, int cx, int cy);
	void OnPaint();
	void OnLButtonDown(UINT flags, CPoint point);
	void OnLButtonUp(UINT flags, CPoint point);
	void OnRButtonUp(UINT flags, CPoint point);
	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	BOOL OnSetColors(int numcolors, GRAD_COLORS* pColors);
	int OnGetColors(GRAD_COLORS* pColors);
	int OnGetNumColors();
	void OnReverseMap();
};


#endif		// __GRADEDIT_INCLUDEE

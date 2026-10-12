//////////////////////////////////////////////////////////////////////
// rangebar.h degree control
//

#ifndef __RANGEBAR_INCLUDE

//#define THUMBWIDTH				15
//#define THUMBHEIGHT				8
#define THUMBWIDTH				11
#define THUMBHEIGHT				12

class CRangeBar : public CWindow
{
public:
	CRangeBar();
	virtual ~CRangeBar();

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	LRESULT Default();

// Attributes
public:

// Operations
public:

// Implementation
protected:
	int m_min;
	int m_max;
	int m_pos_min;
	int m_pos_max;
	BOOL m_bCaptured;
	BOOL m_bAlphaImage;
	MSG m_msg;
	HIMAGELIST m_him;
	CSize m_sThumb;
	int m_range;

public:
	CWindow m_parent;

protected:
	virtual void OnDraw(CDC& dc);      // pass on pure virtual
	void DrawThumb(CDC& dc);

protected:
	// メッセージマップ
	int OnCreate(LPCREATESTRUCT pcs);
	void OnSize(UINT flags, int cx, int cy);
	void OnPrintClient(CDC& dc, LPARAM flags);
	void OnPaint();
	void OnLButtonDown(UINT flags, CPoint point);
	void OnLButtonUp(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	void OnCancelMode();
	void OnChangeFocus(BOOL bHaveFocus);
	void OnSetRange(int nMin, int nMax, BOOL bRedraw);
	void OnSetPos(int pos, BOOL bRedraw);
};


#endif		// __RANGEBAR_INCLUDEE

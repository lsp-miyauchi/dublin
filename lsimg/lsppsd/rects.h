#ifndef _RECTS_H
	#define _RECTS_H

/////////////////////////////////////////////////////////////////////////////
// Classes declared in this file

class CSize;
class CPoint;
class CRect;

/////////////////////////////////////////////////////////////////////////////
// CSize - An extent, similar to Windows SIZE structure.

class CSize : public tagSIZE
{
public:

// Constructors
	CSize();
	CSize(int initCX, int initCY);
	CSize(SIZE initSize);
	CSize(POINT initPt);
	CSize(DWORD dwSize);

// Operations
	BOOL operator==(SIZE size) const;
	BOOL operator!=(SIZE size) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);

// Operators returning CSize values
	CSize operator+(SIZE size) const;
	CSize operator-(SIZE size) const;
	CSize operator-() const;

// Operators returning CPoint values
	CPoint operator+(POINT point) const;
	CPoint operator-(POINT point) const;

// Operators returning CRect values
	CRect operator+(const RECT* lpRect) const;
	CRect operator-(const RECT* lpRect) const;
};


/////////////////////////////////////////////////////////////////////////////
// CPoint - A 2-D point, similar to Windows POINT structure.

class CPoint : public tagPOINT
{
public:

// Constructors
	CPoint();
	CPoint(int initX, int initY);
	CPoint(POINT initPt);
	CPoint(SIZE initSize);
	CPoint(DWORD dwPoint);

// Operations
	void Offset(int xOffset, int yOffset);
	void Offset(POINT point);
	void Offset(SIZE size);
	BOOL operator==(POINT point) const;
	BOOL operator!=(POINT point) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);
	void operator+=(POINT point);
	void operator-=(POINT point);

// Operators returning CPoint values
	CPoint operator+(SIZE size) const;
	CPoint operator-(SIZE size) const;
	CPoint operator-() const;
	CPoint operator+(POINT point) const;

// Operators returning CSize values
	CSize operator-(POINT point) const;

// Operators returning CRect values
	CRect operator+(const RECT* lpRect) const;
	CRect operator-(const RECT* lpRect) const;
};


/////////////////////////////////////////////////////////////////////////////
// CRect - A 2-D rectangle, similar to Windows RECT structure.

typedef const RECT* LPCRECT;    // pointer to read/only RECT

class CRect : public tagRECT
{
public:

// Constructors
	CRect();
	CRect(int l, int t, int r, int b);
	CRect(const RECT& srcRect);
	CRect(LPCRECT lpSrcRect);
	CRect(POINT point, SIZE size);
	CRect(POINT topLeft, POINT bottomRight);

// Attributes (in addition to RECT members)
	int Width() const;
	int Height() const;
	CSize Size() const;
	CPoint& TopLeft();
	CPoint& BottomRight();
	const CPoint& TopLeft() const;
	const CPoint& BottomRight() const;
	CPoint CenterPoint() const;

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT();
	operator LPCRECT() const;

	BOOL IsRectEmpty() const;
	BOOL IsRectNull() const;
	BOOL PtInRect(POINT point) const;

// Operations
	void SetRect(int x1, int y1, int x2, int y2);
	void SetRect(POINT topLeft, POINT bottomRight);
	void SetRectEmpty();
	void CopyRect(LPCRECT lpSrcRect);
	BOOL EqualRect(LPCRECT lpRect) const;

	void InflateRect(int x, int y);
	void InflateRect(SIZE size);
	void InflateRect(LPCRECT lpRect);
	void InflateRect(int l, int t, int r, int b);
	void DeflateRect(int x, int y);
	void DeflateRect(SIZE size);
	void DeflateRect(LPCRECT lpRect);
	void DeflateRect(int l, int t, int r, int b);

	void OffsetRect(int x, int y);
	void OffsetRect(SIZE size);
	void OffsetRect(POINT point);
	void NormalizeRect();

	// operations that fill '*this' with result
	BOOL IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2);
	BOOL UnionRect(LPCRECT lpRect1, LPCRECT lpRect2);
	BOOL SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2);

// Additional Operations
	void operator=(const RECT& srcRect);
	BOOL operator==(const RECT& rect) const;
	BOOL operator!=(const RECT& rect) const;
	void operator+=(POINT point);
	void operator+=(SIZE size);
	void operator+=(LPCRECT lpRect);
	void operator-=(POINT point);
	void operator-=(SIZE size);
	void operator-=(LPCRECT lpRect);
	void operator&=(const RECT& rect);
	void operator|=(const RECT& rect);

// Operators returning CRect values
	CRect operator+(POINT point) const;
	CRect operator-(POINT point) const;
	CRect operator+(LPCRECT lpRect) const;
	CRect operator+(SIZE size) const;
	CRect operator-(SIZE size) const;
	CRect operator-(LPCRECT lpRect) const;
	CRect operator&(const RECT& rect2) const;
	CRect operator|(const RECT& rect2) const;
	CRect MulDiv(int nMultiplier, int nDivisor) const;
};

// CSize
_inline CSize::CSize()
	{ /* random filled */ }
_inline CSize::CSize(int initCX, int initCY)
	{ cx = initCX; cy = initCY; }
_inline CSize::CSize(SIZE initSize)
	{ *(SIZE*)this = initSize; }
_inline CSize::CSize(POINT initPt)
	{ *(POINT*)this = initPt; }
_inline CSize::CSize(DWORD dwSize)
	{
		cx = (short)LOWORD(dwSize);
		cy = (short)HIWORD(dwSize);
	}
_inline BOOL CSize::operator==(SIZE size) const
	{ return (cx == size.cx && cy == size.cy); }
_inline BOOL CSize::operator!=(SIZE size) const
	{ return (cx != size.cx || cy != size.cy); }
_inline void CSize::operator+=(SIZE size)
	{ cx += size.cx; cy += size.cy; }
_inline void CSize::operator-=(SIZE size)
	{ cx -= size.cx; cy -= size.cy; }
_inline CSize CSize::operator+(SIZE size) const
	{ return CSize(cx + size.cx, cy + size.cy); }
_inline CSize CSize::operator-(SIZE size) const
	{ return CSize(cx - size.cx, cy - size.cy); }
_inline CSize CSize::operator-() const
	{ return CSize(-cx, -cy); }
_inline CPoint CSize::operator+(POINT point) const
	{ return CPoint(cx + point.x, cy + point.y); }
_inline CPoint CSize::operator-(POINT point) const
	{ return CPoint(cx - point.x, cy - point.y); }
_inline CRect CSize::operator+(const RECT* lpRect) const
	{ return CRect(lpRect) + *this; }
_inline CRect CSize::operator-(const RECT* lpRect) const
	{ return CRect(lpRect) - *this; }

// CPoint
_inline CPoint::CPoint()
	{ /* random filled */ }
_inline CPoint::CPoint(int initX, int initY)
	{ x = initX; y = initY; }
_inline CPoint::CPoint(POINT initPt)
	{ *(POINT*)this = initPt; }
_inline CPoint::CPoint(SIZE initSize)
	{ *(SIZE*)this = initSize; }
_inline CPoint::CPoint(DWORD dwPoint)
	{
		x = (short)LOWORD(dwPoint);
		y = (short)HIWORD(dwPoint);
	}
_inline void CPoint::Offset(int xOffset, int yOffset)
	{ x += xOffset; y += yOffset; }
_inline void CPoint::Offset(POINT point)
	{ x += point.x; y += point.y; }
_inline void CPoint::Offset(SIZE size)
	{ x += size.cx; y += size.cy; }
_inline BOOL CPoint::operator==(POINT point) const
	{ return (x == point.x && y == point.y); }
_inline BOOL CPoint::operator!=(POINT point) const
	{ return (x != point.x || y != point.y); }
_inline void CPoint::operator+=(SIZE size)
	{ x += size.cx; y += size.cy; }
_inline void CPoint::operator-=(SIZE size)
	{ x -= size.cx; y -= size.cy; }
_inline void CPoint::operator+=(POINT point)
	{ x += point.x; y += point.y; }
_inline void CPoint::operator-=(POINT point)
	{ x -= point.x; y -= point.y; }
_inline CPoint CPoint::operator+(SIZE size) const
	{ return CPoint(x + size.cx, y + size.cy); }
_inline CPoint CPoint::operator-(SIZE size) const
	{ return CPoint(x - size.cx, y - size.cy); }
_inline CPoint CPoint::operator-() const
	{ return CPoint(-x, -y); }
_inline CPoint CPoint::operator+(POINT point) const
	{ return CPoint(x + point.x, y + point.y); }
_inline CSize CPoint::operator-(POINT point) const
	{ return CSize(x - point.x, y - point.y); }
_inline CRect CPoint::operator+(const RECT* lpRect) const
	{ return CRect(lpRect) + *this; }
_inline CRect CPoint::operator-(const RECT* lpRect) const
	{ return CRect(lpRect) - *this; }

// CRect
_inline CRect::CRect()
	{ /* random filled */ }
_inline CRect::CRect(int l, int t, int r, int b)
	{ left = l; top = t; right = r; bottom = b; }
_inline CRect::CRect(const RECT& srcRect)
	{ ::CopyRect(this, &srcRect); }
_inline CRect::CRect(LPCRECT lpSrcRect)
	{ ::CopyRect(this, lpSrcRect); }
_inline CRect::CRect(POINT point, SIZE size)
	{ right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy; }
_inline CRect::CRect(POINT topLeft, POINT bottomRight)
	{ left = topLeft.x; top = topLeft.y;
		right = bottomRight.x; bottom = bottomRight.y; }
_inline int CRect::Width() const
	{ return right - left; }
_inline int CRect::Height() const
	{ return bottom - top; }
_inline CSize CRect::Size() const
	{ return CSize(right - left, bottom - top); }
_inline CPoint& CRect::TopLeft()
	{ return *((CPoint*)this); }
_inline CPoint& CRect::BottomRight()
	{ return *((CPoint*)this+1); }
_inline const CPoint& CRect::TopLeft() const
	{ return *((CPoint*)this); }
_inline const CPoint& CRect::BottomRight() const
	{ return *((CPoint*)this+1); }
_inline CPoint CRect::CenterPoint() const
	{ return CPoint((left+right)/2, (top+bottom)/2); }
_inline CRect::operator LPRECT()
	{ return this; }
_inline CRect::operator LPCRECT() const
	{ return this; }
_inline BOOL CRect::IsRectEmpty() const
	{ return ::IsRectEmpty(this); }
_inline BOOL CRect::IsRectNull() const
	{ return (left == 0 && right == 0 && top == 0 && bottom == 0); }
_inline BOOL CRect::PtInRect(POINT point) const
	{ return ::PtInRect(this, point); }
_inline void CRect::SetRect(int x1, int y1, int x2, int y2)
	{ ::SetRect(this, x1, y1, x2, y2); }
_inline void CRect::SetRect(POINT topLeft, POINT bottomRight)
	{ ::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y); }
_inline void CRect::SetRectEmpty()
	{ ::SetRectEmpty(this); }
_inline void CRect::CopyRect(LPCRECT lpSrcRect)
	{ ::CopyRect(this, lpSrcRect); }
_inline BOOL CRect::EqualRect(LPCRECT lpRect) const
	{ return ::EqualRect(this, lpRect); }
_inline void CRect::InflateRect(int x, int y)
	{ ::InflateRect(this, x, y); }
_inline void CRect::InflateRect(SIZE size)
	{ ::InflateRect(this, size.cx, size.cy); }
_inline void CRect::DeflateRect(int x, int y)
	{ ::InflateRect(this, -x, -y); }
_inline void CRect::DeflateRect(SIZE size)
	{ ::InflateRect(this, -size.cx, -size.cy); }
_inline void CRect::OffsetRect(int x, int y)
	{ ::OffsetRect(this, x, y); }
_inline void CRect::OffsetRect(POINT point)
	{ ::OffsetRect(this, point.x, point.y); }
_inline void CRect::OffsetRect(SIZE size)
	{ ::OffsetRect(this, size.cx, size.cy); }
_inline BOOL CRect::IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2)
	{ return ::IntersectRect(this, lpRect1, lpRect2);}
_inline BOOL CRect::UnionRect(LPCRECT lpRect1, LPCRECT lpRect2)
	{ return ::UnionRect(this, lpRect1, lpRect2); }
_inline void CRect::operator=(const RECT& srcRect)
	{ ::CopyRect(this, &srcRect); }
_inline BOOL CRect::operator==(const RECT& rect) const
	{ return ::EqualRect(this, &rect); }
_inline BOOL CRect::operator!=(const RECT& rect) const
	{ return !::EqualRect(this, &rect); }
_inline void CRect::operator+=(POINT point)
	{ ::OffsetRect(this, point.x, point.y); }
_inline void CRect::operator+=(SIZE size)
	{ ::OffsetRect(this, size.cx, size.cy); }
_inline void CRect::operator+=(LPCRECT lpRect)
	{ InflateRect(lpRect); }
_inline void CRect::operator-=(POINT point)
	{ ::OffsetRect(this, -point.x, -point.y); }
_inline void CRect::operator-=(SIZE size)
	{ ::OffsetRect(this, -size.cx, -size.cy); }
_inline void CRect::operator-=(LPCRECT lpRect)
	{ DeflateRect(lpRect); }
_inline void CRect::operator&=(const RECT& rect)
	{ ::IntersectRect(this, this, &rect); }
_inline void CRect::operator|=(const RECT& rect)
	{ ::UnionRect(this, this, &rect); }
_inline CRect CRect::operator+(POINT pt) const
	{ CRect rect(*this); ::OffsetRect(&rect, pt.x, pt.y); return rect; }
_inline CRect CRect::operator-(POINT pt) const
	{ CRect rect(*this); ::OffsetRect(&rect, -pt.x, -pt.y); return rect; }
_inline CRect CRect::operator+(SIZE size) const
	{ CRect rect(*this); ::OffsetRect(&rect, size.cx, size.cy); return rect; }
_inline CRect CRect::operator-(SIZE size) const
	{ CRect rect(*this); ::OffsetRect(&rect, -size.cx, -size.cy); return rect; }
_inline CRect CRect::operator+(LPCRECT lpRect) const
	{ CRect rect(this); rect.InflateRect(lpRect); return rect; }
_inline CRect CRect::operator-(LPCRECT lpRect) const
	{ CRect rect(this); rect.DeflateRect(lpRect); return rect; }
_inline CRect CRect::operator&(const RECT& rect2) const
	{ CRect rect; ::IntersectRect(&rect, this, &rect2);
		return rect; }
_inline CRect CRect::operator|(const RECT& rect2) const
	{ CRect rect; ::UnionRect(&rect, this, &rect2);
		return rect; }
_inline BOOL CRect::SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2)
	{ return ::SubtractRect(this, lpRectSrc1, lpRectSrc2); }


#endif			// _RECTS_H

/////////////////////////////////////////////////////////////////////////////
// CRange - An extent, similar to RANGE structure.

typedef struct tagRNG {
	int from;
	int to;
} RNG;

class CRange : public tagRNG
{
public:
// Constructors
	CRange();
	CRange(int initFrom, int initTo);
	CRange(RNG initRange);

// Attributes
	int Range() const;
	BOOL InRange(int value) const;
	BOOL InnerRange(int value) const;
	int Min() const;
	int Max() const;

// Operations
	void Inflate(int delta);
	void Offset(int delta);
	void Normalize();

	BOOL operator==(RNG range) const;
	BOOL operator!=(RNG range) const;
	BOOL operator<(int value) const;
	BOOL operator<=(int value) const;
	BOOL operator>(int value) const;
	BOOL operator>=(int value) const;
	void operator+=(RNG range);
	void operator-=(RNG range);

// Operators returning CRange values
	CRange operator+(RNG range) const;
	CRange operator-(RNG range) const;
	CRange operator-() const;
};

// CRange
_inline CRange::CRange()
	{ /* random filled */ }
_inline CRange::CRange(int initFrom, int initTo)
	{ from = initFrom; to = initTo; }
_inline CRange::CRange(RNG initRange)
	{ *(RNG*)this = initRange; }
_inline int CRange::Range() const
	{ return (Max() - Min()); }
_inline BOOL CRange::InRange(int value) const
	{ return ((min(from, to) <= value) && (value <= max(from, to))); }
_inline BOOL CRange::InnerRange(int value) const
	{ return ((min(from, to) < value) && (value < max(from, to))); }
_inline int CRange::Min() const
	{ return min(from, to); }
_inline int CRange::Max() const
	{ return max(from, to); }
_inline void CRange::Inflate(int delta)
	{ from += delta; to -= delta; }
_inline void CRange::Offset(int delta)
	{ from += delta; to += delta; }
_inline void CRange::Normalize()
	{ if (from > to) { from^=to; to^=from; from^=to; } }
_inline BOOL CRange::operator==(RNG range) const
	{ return (from == range.from && to == range.to); }
_inline BOOL CRange::operator!=(RNG range) const
	{ return (from != range.from || to != range.to); }
_inline BOOL CRange::operator<(int value) const
	{ return (Max() < value); }
_inline BOOL CRange::operator<=(int value) const
	{ return (Max() <= value); }
_inline BOOL CRange::operator>(int value) const
	{ return (Min() > value); }
_inline BOOL CRange::operator>=(int value) const
	{ return (Min() >= value); }
_inline void CRange::operator+=(RNG range)
	{ from += range.from; to += range.to; }
_inline void CRange::operator-=(RNG range)
	{ from -= range.from; to -= range.to; }
_inline CRange CRange::operator+(RNG range) const
	{ return CRange(from + range.from, to + range.to); }
_inline CRange CRange::operator-(RNG range) const
	{ return CRange(from - range.from, to - range.to); }
_inline CRange CRange::operator-() const
	{ return CRange(-from, -to); }

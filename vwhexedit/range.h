/////////////////////////////////////////////////////////////////////////////
// CRange - An extent, similar to RANGE structure.

#pragma once
#ifndef __LSP_RANGE_H__
#define __LSP_RANGE_H__

typedef struct tagRNG {
	INT_PTR from;
	INT_PTR to;
} RNG;

class CRange : public tagRNG
{
public:
// Constructors
	inline CRange() { from = to = 0; }
	inline CRange(INT_PTR initFrom, INT_PTR initTo) { from = initFrom; to = initTo; }
	inline CRange(RNG initRange) { *(RNG*)this = initRange; }

// Attributes
	inline BOOL IsEmpty() const { return (!from && !to); }
	inline INT_PTR Range() const { return (to - from); }
	inline INT_PTR RangeAbs() const { return ((from < to) ? (to - from) : (from - to)); }
	inline BOOL InRange(INT_PTR value) const { return ((min(from, to) <= value) && (value <= max(from, to))); }
	inline BOOL InnerRange(INT_PTR value) const { return ((min(from, to) < value) && (value < max(from, to))); }
	inline INT_PTR Min() const { return min(from, to); }
	inline INT_PTR Max() const { return max(from, to); }

// Operations
	inline void SetRange(INT_PTR f, INT_PTR t) { from = f; to = t; }
	inline void Inflate(size_t delta) { from += delta; to -= delta; }
	inline void Offset(size_t delta) { from += delta; to += delta; }
	inline void Normalize() { if (from > to) { from^=to; to^=from; from^=to; } }
	inline BOOL IsIntersectRange(RNG range) const { return ((to < range.from) || (range.to < from)) ? FALSE : TRUE; }

	BOOL IntersectRange(RNG range)
	{
		if ((to < range.from) || (range.to < from)) {
			from = to = 0;
			return FALSE;
		}

		from = min(from, range.from);
		to = max(to, range.to);
		return TRUE;
	}

	void UnionRange(RNG range)
	{
		INT_PTR smin = Min();
		INT_PTR smax = Max();
		INT_PTR dmin = min(range.from, range.to);
		INT_PTR dmax = max(range.from, range.to);
		from = min(smin, dmin);
		to = max(smax, dmax);
	}

	inline BOOL operator==(RNG range) const { return (from == range.from && to == range.to); }
	inline BOOL operator!=(RNG range) const { return (from != range.from || to != range.to); }
	inline void operator&=(RNG range) { IntersectRange(range); }
	inline void operator|=(RNG range) { UnionRange(range); }
};


#endif		// __LSP_RANGE_H__

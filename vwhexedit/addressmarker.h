////////////////////////////////////////////////////////////////////////
// addressmarker.h

#pragma once

#ifndef __ADDRESSMARKER_H__
#define __ADDRESSMARKER_H__

class CAddressMarker : public CAtlArray<RNG>
{
public:
	RNG Find(INT_PTR addr, size_t* pindex = NULL)
	{
		RNG r;
		size_t cnt = GetCount();
		for (size_t i = 0; i < cnt; i++) {
			r = GetAt(i);
			if (r.from <= addr && addr <= r.to) {
				if (pindex)
					*pindex = i;
				return r;
			}
		}
		r.from = r.to = 0;
		return r;
	}

	static int _cdecl compare(const void* arg1, const void* arg2)
	{
		RNG r1 = *(RNG*)arg1;
		RNG r2 = *(RNG*)arg2;
		int n = (int)(r1.from - r2.from);
		if (!n)
			n = (int)(r1.to - r2.to);
		return n;
	}

	void Sort()
	{
		size_t cnt = GetCount();
		if (cnt > 1) {
			qsort(GetData(), cnt, sizeof(RNG), compare);
		}
	}

	RNG FindPrev(INT_PTR addr)
	{
		Sort();

		CRange f;
		size_t cnt = GetCount();
		for (size_t i = 0; i < cnt; i++) {
			RNG r = GetAt(i);
			if (r.from < addr)
				f = r;
			else
				break;
		}
		return f;
	}

	RNG FindNext(INT_PTR addr)
	{
		Sort();

		CRange f;
		size_t cnt = GetCount();
		for (size_t i = 0; i < cnt; i++) {
			RNG r = GetAt(i);
			if (addr < r.from)
				return r;
		}
		return f;
	}
};

#endif		// __ADDRESSMARKER_H__

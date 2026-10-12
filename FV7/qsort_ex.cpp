//////////////////////////////////////////////////////////////////////
// qsort_ex.cpp

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif

typedef int __cdecl _Cmpfun(const void*, const void*, const void*);

#define BUF_SIZE		256		// > 0, auto buffer size
#define SORT_MAX		32		// > 1, cutover for insertion sort

static void __fastcall swap(char *qb, char *qe, size_t size)
{
	// swap elements *qb and *qe
	char buf[BUF_SIZE];		// chunk to copy on swap
    size_t ms;

	for (ms = size; 0 < ms; )
	{
		// swap as many as possible
		size_t m = ms < sizeof (buf) ? ms : sizeof (buf);
		memcpy(buf, qb, m);
		memcpy(qb, qe, m);
		memcpy(qe, buf, m);
		ms -= m, qb += m, qe += m;
	}
}

static void __fastcall rotate(char *qb, char *qe, size_t size)
{
	// right rotate [*qb, *qe] one place
	char buf[BUF_SIZE];		// chunk to copy on rotate
	size_t ms, mtot = qe - qb + size;

	for (ms = size; 0 < ms; )
	{
		// rotate as many as possible
		size_t m = ms < sizeof (buf) ? ms : sizeof (buf);
		memcpy(buf, qe + (size - m), m);
		memmove(qb + m, qb, mtot - m);
		memcpy(qb, buf, m);
		ms -= m;
	}
}

static void __fastcall adjust_heap(char *qb, size_t h, size_t n, size_t size, _Cmpfun *cmp, const void* param)
{
	// reheap item at h in heap (char qb[size])[n]
	size_t h0 = h;
	size_t k = 2 * h + 2;
	char *qh = qb + h * size;
	char *qk = qb + k * size;

	for (; k <= n; k = 2 * k + 2, qk = qb + k * size)
	{
		// percolate hole out to larger/only child
		if (k == n || (*cmp)(qk, qk - size, param) < 0)
			--k, qk -= size;
		swap(qh, qk, size);
		h = k, qh = qk;
	}

	for (; h0 < h; )
	{
		// percolate hole back in as far as h0
		size_t i = (h - 1) / 2;
		char *qi = qb + i * size;
		if ((*cmp)(qh, qi, param) <= 0)
			break;
		swap(qi, qh, size);
		h = i, qh = qi;
	}
}

static void __fastcall intro_sort(char *qb, size_t n, size_t ideal, size_t size, _Cmpfun *cmp, const void* param)
{
	// sort recursively
	for (; 0 < ideal && SORT_MAX < n; )
	{
		// quick sort with fat pivot
		size_t m = n / 2;
		char *qm = qb + m * size;
		char *qf = qb, *ql = qb + n * size;

		if ((*cmp)(qm, qf, param) < 0)
			swap(qf, qm, size);
		if ((*cmp)(ql - size, qm, param) < 0)
			swap(qm, ql - size, size);
		if ((*cmp)(qm, qf, param) < 0)
			swap(qf, qm, size);

		for (; ; qf += size)
		{
			// partition about pivot value
			for (; qf < ql && (*cmp)(qf, qm, param) <= 0; qf += size)
				;
			for (; qb < ql && (*cmp)(qm, ql -= size, param) <= 0; )
				;
			if (ql <= qf)
				break;

			swap(qf, ql, size);
			if (qm == qf)
				qm = ql;
			else if (qm == ql)
				qm = qf;
		}
		ideal /= 2, ideal += ideal / 2;
		m = (ql - qb) / size + 1;
		n -= (qf - qb) / size;
		if (m <= n)
			intro_sort(qb, m, ideal, size, cmp, param), qb = qf;
		else
			intro_sort(qf, n, ideal, size, cmp, param), n = m;
	}

	if (SORT_MAX < n)
	{
		// heap sort
		size_t h;
		char *qe;
		for (h = n / 2; 0 < h; )
			adjust_heap(qb, --h, n, size, cmp, param);
		for (qe = qb + n * size; 1 < n; )
		{
			// pop largest item to (shrinking) end
			swap(qb, qe -= size, size);
			adjust_heap(qb, 0, --n, size, cmp, param);
		}
	}
	else if (1 < n)
	{
		// insertion sort
		char *qm;
		for (qm = qb; 0 < --n; )
		{
			// percolate back elements [2, n)
			qm += size;
			if ((*cmp)(qm, qb, param) < 0)
				rotate(qb, qm, size);
			else
			{
				// scan backwards for insertion point
				char *qx = qm, *qx0 = qm;
				for (; (*cmp)(qm, qx0 -= size, param) < 0; qx = qx0)
					;
				if (qx != qm)
					rotate(qx, qm, size);
			}
		}
	}
}

void __cdecl qsort_ex(void *base, size_t n, size_t size, _Cmpfun *cmp, const void* param)
{
	// sort (char base[size])[n] using introsort
	intro_sort((char*)base, n, n, size, cmp, param);
}


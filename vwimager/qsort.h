//////////////////////////////////////////////////////////////////////
// qsort.h

#pragma once

#define QSRT_BUFSIZE		256		// > 0, auto buffer size
#define QSRT_SORTMAX		32		// > 1, cutover for insertion sort

typedef int __cdecl _QSortCompareFunction(const void*, const void*, const void*);


class CQSort
{
private:
	_QSortCompareFunction* m_compare;
	const void* m_param;

public:
	CQSort()
		: m_compare(NULL)
		, m_param(NULL)
	{
	}
	~CQSort()
	{
	}

private:
	void swap(char *qb, char *qe, size_t size)
	{
		// swap elements *qb and *qe
		char buf[QSRT_BUFSIZE];		// chunk to copy on swap
	    size_t ms;

		for (ms = size; 0 < ms; ) {
			// swap as many as possible
			size_t m = ms < sizeof (buf) ? ms : sizeof (buf);
			memcpy(buf, qb, m);
			memcpy(qb, qe, m);
			memcpy(qe, buf, m);
			ms -= m, qb += m, qe += m;
		}
	}

	void rotate(char *qb, char *qe, size_t size)
	{
		// right rotate [*qb, *qe] one place
		char buf[QSRT_BUFSIZE];		// chunk to copy on rotate
		size_t ms, mtot = qe - qb + size;

		for (ms = size; 0 < ms; ) {
			// rotate as many as possible
			size_t m = ms < sizeof (buf) ? ms : sizeof (buf);
			memcpy(buf, qe + (size - m), m);
			memmove(qb + m, qb, mtot - m);
			memcpy(qb, buf, m);
			ms -= m;
		}
	}

	void adjust_heap(char *qb, size_t h, size_t n, size_t size)
	{
		// reheap item at h in heap (char qb[size])[n]
		size_t h0 = h;
		size_t k = 2 * h + 2;
		char *qh = qb + h * size;
		char *qk = qb + k * size;

		for (; k <= n; k = 2 * k + 2, qk = qb + k * size) {
			// percolate hole out to larger/only child
			if (k == n || (*m_compare)(qk, qk - size, m_param) < 0)
				--k, qk -= size;
			swap(qh, qk, size);
			h = k, qh = qk;
		}

		for (; h0 < h; ) {
			// percolate hole back in as far as h0
			size_t i = (h - 1) / 2;
			char *qi = qb + i * size;
			if ((*m_compare)(qh, qi, m_param) <= 0)
				break;
			swap(qi, qh, size);
			h = i, qh = qi;
		}
	}

	void intro_sort(char *qb, size_t n, size_t ideal, size_t size)
	{
		// sort recursively
		for (; 0 < ideal && QSRT_SORTMAX < n; ) {
			// quick sort with fat pivot
			size_t m = n / 2;
			char *qm = qb + m * size;
			char *qf = qb, *ql = qb + n * size;

			if ((*m_compare)(qm, qf, m_param) < 0)
				swap(qf, qm, size);
			if ((*m_compare)(ql - size, qm, m_param) < 0)
				swap(qm, ql - size, size);
			if ((*m_compare)(qm, qf, m_param) < 0)
				swap(qf, qm, size);

			for (; ; qf += size) {
				// partition about pivot value
				for (; qf < ql && (*m_compare)(qf, qm, m_param) <= 0; qf += size)
					;
				for (; qb < ql && (*m_compare)(qm, ql -= size, m_param) <= 0; )
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
				intro_sort(qb, m, ideal, size), qb = qf;
			else
				intro_sort(qf, n, ideal, size), n = m;
		}

		if (QSRT_SORTMAX < n) {
			// heap sort
			size_t h;
			char *qe;
			for (h = n / 2; 0 < h; )
				adjust_heap(qb, --h, n, size);
			for (qe = qb + n * size; 1 < n; ) {
				// pop largest item to (shrinking) end
				swap(qb, qe -= size, size);
				adjust_heap(qb, 0, --n, size);
			}
		}
		else if (1 < n) {
			// insertion sort
			char *qm;
			for (qm = qb; 0 < --n; ) {
				// percolate back elements [2, n)
				qm += size;
				if ((*m_compare)(qm, qb, m_param) < 0)
					rotate(qb, qm, size);
				else {
					// scan backwards for insertion point
					char *qx = qm, *qx0 = qm;
					for (; (*m_compare)(qm, qx0 -= size, m_param) < 0; qx = qx0)
						;
					if (qx != qm)
						rotate(qx, qm, size);
				}
			}
		}
	}

	static int __cdecl StringCompare(const void* arg1, const void* arg2, const void* param)
	{
		LPCTSTR p1 = *(LPCTSTR*)arg1;
		LPCTSTR p2 = *(LPCTSTR*)arg2;
		if (!p1) return -1;
		if (!p2) return 1;
		return StrCmpLogicalW(p1, p2);
	}

public:
	void sort(void *base, size_t n, size_t size, _QSortCompareFunction *cmp, const void* param)
	{
		// sort (char base[size])[n] using introsort
		m_compare = (cmp != NULL) ? cmp : StringCompare;
		m_param = param;
		intro_sort((char*)base, n, n, size);
	}
};

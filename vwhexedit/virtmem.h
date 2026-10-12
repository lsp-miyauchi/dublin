//////////////////////////////////////////////////////////////////////
// virtmem.h


_inline LPVOID valloc(size_t size)
	{ return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE); }
_inline BOOL vfree(LPVOID pv)
	{ return VirtualFree(pv, 0, MEM_RELEASE); }


class CVirtMem
{
public:
	CVirtMem();
	~CVirtMem();

	BOOL Alloc(size_t size);
	void Free();
	BOOL Realloc(size_t size);
	inline operator LPBYTE() const { return (LPBYTE)m_pPage; }
	inline size_t GetSize() { return m_nDataSize; }

	size_t Read(UINT_PTR posOffset, LPVOID pBuffer, size_t size);
	size_t Write(UINT_PTR posOffset, LPVOID pBuffer, size_t size);

	inline BOOL IsEmpty() { return m_nDataSize == 0; }

	LPBYTE m_pPage;
	size_t m_nDataSize;
	size_t m_nPageSize;
	size_t m_nSysPageSize;
};


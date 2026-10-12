/////////////////////////////////////////////////////////////////////
// filemap.h

class filemap
{
public:
	filemap();
	~filemap();

	BOOL open(const char* name);
	BOOL openW(LPCWSTR name);
	void close();
	int getc();
	short getshort();
	short getmsbshort();
	unsigned long getlong();
	unsigned long getmsblong();
	int read(void* pbuf, int len);
	long seek(long pos, int mode);

	HANDLE m_hf;
	HANDLE m_map;
	BYTE* m_base;
	BYTE* m_ptr;
	long m_size;
};

_inline filemap::filemap()
	{ m_hf = INVALID_HANDLE_VALUE, m_map = NULL, m_base = NULL; }
_inline filemap::~filemap()
	{ close(); }
_inline int filemap::getc()
	{ if (m_base + m_size > m_ptr) return (int)*m_ptr++; return EOF; }


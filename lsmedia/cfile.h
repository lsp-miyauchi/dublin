//////////////////////////////////////////////////////////////////////
// CFile

class CFile
{
public:
	CFile();
	~CFile();

	BOOL Create(LPCTSTR lpFile);
	BOOL Open(LPCTSTR lpFile);
	BOOL Read(LPVOID lpBuffer, UINT cbRead);
	BOOL Write(LPVOID lpWrite, UINT cbWrite);
	long Seek(long lOff, int nFrom);
	void Close();
	long GetLength();
	long GetReminder();

	HANDLE m_hf;
};

_inline CFile::CFile()
	{ m_hf = NULL; }
_inline CFile::~CFile()
	{ Close(); }

////////////////////////////////////////////////////////////////////
// findfile.h


///////////////////////////////////////////////////////////////////////
// CWin32FileDataクラス
//

typedef const WIN32_FIND_DATA* LPCWIN32_FIND_DATA;

class CWin32FindData : public WIN32_FIND_DATA
{
public:
	CWin32FindData();
	CWin32FindData(const WIN32_FIND_DATA& srcData);

	BOOL IsFolder();
	BOOL IsDot();
	BOOL IsSysHidden();
	BOOL IsSystem();
	BOOL IsHidden();
	BOOL IsAttrib(DWORD dwAttr);
	BOOL LocalCreationTime(LPFILETIME lpft);
	BOOL LocalLastWriteTime(LPFILETIME lpft);
	BOOL LocalLastAccessTime(LPFILETIME lpft);

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPWIN32_FIND_DATA();
	operator LPCWIN32_FIND_DATA() const;

	void operator=(const WIN32_FIND_DATA& srcData);
};

_inline CWin32FindData::CWin32FindData()
	{ }
_inline CWin32FindData::CWin32FindData(const WIN32_FIND_DATA& srcData)
	{ *this = srcData; }
_inline BOOL CWin32FindData::IsFolder()
	{ return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
_inline BOOL CWin32FindData::IsDot()
	{ return (cFileName[0]=='.' && (cFileName[1]==0 || (cFileName[1]=='.' && cFileName[2]==0))); }
_inline BOOL CWin32FindData::IsSysHidden()
	{ return (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)); }
_inline BOOL CWin32FindData::IsHidden()
	{ return (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN); }
_inline BOOL CWin32FindData::IsSystem()
	{ return (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM); }
_inline BOOL CWin32FindData::IsAttrib(DWORD dwAttr)
	{ return (dwFileAttributes & dwAttr); }
_inline BOOL CWin32FindData::LocalCreationTime(LPFILETIME lpft)
	{ return FileTimeToLocalFileTime(&ftCreationTime, lpft); }
_inline BOOL CWin32FindData::LocalLastWriteTime(LPFILETIME lpft)
	{ return FileTimeToLocalFileTime(&ftLastWriteTime, lpft); }
_inline BOOL CWin32FindData::LocalLastAccessTime(LPFILETIME lpft)
	{ return FileTimeToLocalFileTime(&ftLastAccessTime, lpft); }
_inline CWin32FindData::operator LPWIN32_FIND_DATA()
	{ return this; }
_inline CWin32FindData::operator LPCWIN32_FIND_DATA() const
	{ return this; }
_inline void CWin32FindData::operator=(const WIN32_FIND_DATA& srcData)
	{ CopyMemory(this, &srcData, sizeof(WIN32_FIND_DATA)); }


////////////////////////////////////////////////////////////////////
// CFileFinder クラス

class CFileFinder : public CWin32FindData
{
public:
	CFileFinder();
	~CFileFinder();
	BOOL BeginFind(LPCTSTR pFilter);
	BOOL NextFind();
	void EndFind();

	HANDLE m_hFind;
};

_inline CFileFinder::CFileFinder()
	{ m_hFind = NULL; }
_inline CFileFinder::~CFileFinder()
	{ if (m_hFind) FindClose(m_hFind); }
_inline BOOL CFileFinder::BeginFind(LPCTSTR lpFilter)
	{
		m_hFind = FindFirstFile(lpFilter, this);
		if (m_hFind == INVALID_HANDLE_VALUE)
			m_hFind = NULL;
		return (m_hFind != NULL);
	}
_inline BOOL CFileFinder::NextFind()
	{ return (FindNextFile(m_hFind, this) || GetLastError() == ERROR_MORE_DATA); }
_inline void CFileFinder::EndFind()
	{ FindClose(m_hFind); m_hFind = NULL; }

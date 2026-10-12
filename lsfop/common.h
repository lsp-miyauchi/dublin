////////////////////////////////////////////////////////////////////
// common.h


extern COSPlatform gosp;
__inline bool Os7() { return gosp.Os7(); }
__inline bool IsWindowThemed() { return gosp.IsThemed(); }

////////////////////////////////////////////////////////////////////
// Help ID bases

#define HID_BASE_COMMAND    0x00010000UL        // ID and IDM
#define HID_BASE_RESOURCE   0x00020000UL        // IDR and IDD
#define HID_BASE_PROMPT     0x00030000UL        // IDP
#define HID_BASE_NCAREAS    0x00040000UL
#define HID_BASE_CONTROL    0x00050000UL        // IDC
#define HID_BASE_DISPATCH   0x00060000UL        // IDispatch help codes


class CPairName
{
public:
	CPairName() {}
	CPairName(LPCTSTR src, LPCTSTR dst) : m_src(src), m_dst(dst), m_err(0) {}
	~CPairName() {}
	void set(LPCTSTR src, LPCTSTR dst) { m_src = src; m_dst = dst; }

	CString m_src;
	CString m_dst;
	int m_err;
};

////////////////////////////////////////////////////////////////////
// FILE ATTRIBUTES
//
#define FA_SUBDIR		FILE_ATTRIBUTE_DIRECTORY
#define FA_SYSTEM		FILE_ATTRIBUTE_SYSTEM
#define FA_HIDDEN		FILE_ATTRIBUTE_HIDDEN
#define FA_RDONLY		FILE_ATTRIBUTE_READONLY
#define FA_ARCH			FILE_ATTRIBUTE_ARCHIVE
#define FA_NORMAL		FILE_ATTRIBUTE_NORMAL
#define FA_TEMP			FILE_ATTRIBUTE_TEMPORARY
#define FA_CMPRSD		FILE_ATTRIBUTE_COMPRESSED
#define FA_ENCRYPT		FILE_ATTRIBUTE_ENCRYPTED
#define FA_ATOMIC		FILE_ATTRIBUTE_ATOMIC_WRITE
#define FA_SPARSE		FILE_ATTRIBUTE_SPARSE_FILE
#define FA_OFFLINE		FILE_ATTRIBUTE_OFFLINE
#define FA_REPARSE		FILE_ATTRIBUTE_REPARSE_POINT
#define FA_NCINDEX		FILE_ATTRIBUTE_NOT_CONTENT_INDEXED


////////////////////////////////////////////////////////////////////
// 文字列マクロ

#define VALIDSTR(str)			(*(LPCTSTR)(str) != '\0')
#define ISALPHA(a)				(((a)>='a'&&(a)<='z')||((a)>='A'&&(a)<='Z'))
#define TOUPPER(a)				(((a)>='a'&&(a)<='z')?(a)-('a'-'A'):(a))
#define TOLOWER(a)				(((a)>='A'&&(a)<='Z')?(a)+('a'-'A'):(a))

////////////////////////////////////////////////////////////////////
// char search

#define _CHECKDOT(a)	((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))
#define _ISNETPATH(a)	(lstrlen((a)) > 3 && *(a) == '\\' && *((a) + 1) == '\\' && *((a) + 2) != '?')


//----------------------------------------------
// MessageBox ACTION
//
#define MB_INFORMATION	(MB_OK | MB_ICONINFORMATION)
#define MB_ATTENTION	(MB_OK | MB_ICONEXCLAMATION)
#define MB_QUESTION		(MB_YESNOCANCEL | MB_ICONQUESTION)
#define MB_QYESNO		(MB_YESNO | MB_ICONQUESTION)
#define MB_QOKCANCEL	(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QABORT		(MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE)
#define MB_HAND			(MB_OKCANCEL | MB_ICONHAND)
#define MB_STOP			(MB_OK | MB_ICONSTOP)
#define MB_QAST			(MB_OKCANCEL | MB_ICONASTERISK)
#define MB_QEXCM		(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QRETRY		(MB_RETRYCANCEL | MB_ICONEXCLAMATION)


enum {
	DSO_YES,
	DSO_YESTOALL,
	DSO_SKIP,
	DSO_CANCEL
};

typedef struct tagDISCSTREAM {
	TCHAR szName[MAX_PATH * 2];
	int numfiles;
	int operation;
} DISCSTREAM;

typedef struct _tagRESLISTHEAD {
	LPCTSTR label;
	int fmt;
	int cx;
} RESLISTHEAD;

////////////////////////////////////////////////////////////////////
//

class _RES_STRING
{
private:
	LPTSTR m_strAlloc;
public:
	LPCTSTR m_str;

public:
	_RES_STRING(LPCTSTR pstr)
	{
		m_strAlloc = NULL;
		m_str = pstr;
	}

	_RES_STRING(UINT id)
	{
		m_strAlloc = _loadstring(id);
		m_str = m_strAlloc;
	}

	~_RES_STRING()
	{
		if (m_strAlloc) delete [] m_strAlloc;
	}

	LPTSTR _loadstring(UINT id)
	{
		LPTSTR pstr = NULL;
		HINSTANCE hInst = RESINST;
		for(int len = 256; ; len *= 2) {
			pstr = new TCHAR[len];
			if (pstr == NULL) return NULL;

			int res = ::LoadString(hInst, id, pstr, len);
			if (res < len - 1) break;

			delete [] pstr;
			pstr = NULL;
		}
		return pstr;
	}

	operator LPCTSTR() const { return m_str; }
};


//////////////////////////////////////////////////////////////////////
// メモリアロケート用

template <class T>
class CEasyArray
{
private:
	T* m_aT;
	int m_size;
public:
	__inline CEasyArray(int size) { m_size = size; m_aT = (T*)malloc(sizeof(T) * size); }
	__inline ~CEasyArray() { if (m_aT) free(m_aT); }
	__inline operator T*() const { return m_aT; }
	bool SetAt(int index, T t)
	{
		ATLASSERT(index >= 0 && index < m_size);
		if (0 <= index && index < m_size) {
			m_aT[index] = t;
			return true;
		}
		_AtlRaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
		return false;
	}
	const T& operator[] (int index) const
	{
		ATLASSERT(index >= 0 && index < m_size);
		if (0 <= index && index < m_size)
			return m_aT[index];

		_AtlRaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
	}
	T& operator[] (int index)
	{
		ATLASSERT(index >= 0 && index < m_size);
		if (0 <= index && index < m_size)
			return m_aT[index];

		_AtlRaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
	}
};

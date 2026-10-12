//////////////////////////////////////////////////////////////////////
// iconres.h

#pragma once

#define ICOVER					0x00030000
#define WIDTHBYTES(bits)		((((bits) + 31)>>5)<<2)

#pragma pack( push )
#pragma pack( 2 )
typedef struct {
	BYTE	width;					// Width of the image
	BYTE	height;					// Height of the image (times 2)
	BYTE	colors;					// Number of colors in image (0 if >=8bpp)
	BYTE	reserved;				// Reserved
	WORD	planes;					// Color Planes
	WORD	bits;					// Bits per pixel
	DWORD	numbytes;				// how many bytes in this resource?
	WORD	id;						// the ID
} MEMICONDIRENTRY, *LPMEMICONDIRENTRY;

typedef struct {
	WORD			reserved;		// Reserved
	WORD			type;			// resource type (1 for icons)
	WORD			num;			// how many images?
	MEMICONDIRENTRY	memdir[1];		// the entries for each image
} MEMICONDIR, *LPMEMICONDIR;
#pragma pack( pop )

// 次の２つの構造体は、どのようにアイコン情報が.icoファイルに格納されて
// いるかを示す
typedef struct {
	BYTE	width;					// Width of the image
	BYTE	height;					// Height of the image (times 2)
	BYTE	colors;					// Number of colors in image (0 if >=8bpp)
	BYTE	reserved;				// Reserved
	WORD	planes;					// Color Planes
	WORD	bits;					// Bits per pixel
	DWORD	numbytes;				// how many bytes in this resource?
	DWORD	offset;					// where in the file is this image
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct {
	WORD			reserved;		// Reserved
	WORD			type;			// resource type (1 for icons)
	WORD			num;			// how many images?
	ICONDIRENTRY	dir[1];			// the entries for each image
} ICONDIR, *LPICONDIR;


// 次の２つの構造体は、本プログラムでアイコンを取り扱うために利用するもの
// である
// これらは上の構造体よりこのプログラムのオペレーションに、より緊密に拘束
// されます。主要な違いのうちの1つは、マスクのDIB情報にそれらがポインタを
// 供給するということです。

typedef struct tagICONIMAGE {
	UINT			width;
	UINT			height;
	UINT			colors;					// bpp
	LPBYTE			pBitData;				// ptr to DIB bits
	DWORD			numbytes;				// how many bytes?
	__inline LPBITMAPINFO BitmapInfoPtr() { return (LPBITMAPINFO)pBitData; }
	__inline bool IsPng() { return (pBitData != NULL && (*((DWORD*)pBitData) == 0x474e5089)); }
} ICONIMAGE, *LPICONIMAGE;

typedef struct tagICONRESOURCE {
	BOOL bUpdate;							// Has image changed?
	TCHAR szName[MAX_PATH * 2];				// Original name
	BOOL bIconFile;
	UINT num;								// How many images?
	ICONIMAGE images[1];					// Image entries
} ICONRESOURCE, *LPICONRESOURCE;

typedef struct tagICONDATAOBJECT {
	HGLOBAL hFileGroupDescriptor;
	HGLOBAL hFileContents;
} ICONDATAOBJECT;

////////////////////////////////////////////////////////////////////////
//

class _STRINGorINT
{
public:
	_STRINGorINT() : m_pstr(NULL)
	{ }
	_STRINGorINT(LPCTSTR pstr)
	{
		if (IS_INTRESOURCE(pstr)) {
			m_pstr = pstr;
		} else {
			m_pstr = _tcsdup(pstr);
		}
	}
	_STRINGorINT(int num) : m_pstr(MAKEINTRESOURCE(num))
	{ }
	~_STRINGorINT()
	{
		if (!IS_INTRESOURCE(m_pstr)) {
			free((void*)m_pstr);
		}
	}
	bool operator==(const _STRINGorINT& a) const { return (m_pstr == a.m_pstr); }
	LPCTSTR m_pstr;
};

////////////////////////////////////////////////////////////////////////////////

class CIconItem
{
public:
	CSimpleMap<int, HICON> m_mapIcon;
	bool m_bIconFile;
	_STRINGorINT m_resid;

public:
	CIconItem() : m_bIconFile(false)
	{
	}
	~CIconItem()
	{
		// 不要になったアイコンを削除する
		int cnt = m_mapIcon.GetSize();
		for (int i = 0; i < cnt; i++) {
			HICON hic = m_mapIcon.GetValueAt(i);
			if (hic) {
				DestroyIcon(hic);
			}
		}
	}

	void LoadIcon(HMODULE hmod, LPTSTR resid);
	HICON GetIcon(int cx) { return m_mapIcon.Lookup(cx); }
	void SetIcon(int cx, HICON hic)
	{
		int index = m_mapIcon.FindKey(cx);
		if (index < 0) {
			if (hic) {
				m_mapIcon.Add(cx, hic);
			}
		} else {
			if (hic) {
				m_mapIcon.SetAt(cx, hic);
			} else {
				m_mapIcon.RemoveAt(index);
			}
		}
	}
	bool IsEmpty() { return m_mapIcon.GetSize() == 0; }
};

ICONRESOURCE* AllocIconRes(int num);
void FreeIconRes(ICONRESOURCE* pir);
HBITMAP MakeBitmapFromData(void* data, size_t size);
HICON MakeIconFromResource(ICONIMAGE* pii);
ICONRESOURCE* ReadIconFromResource(HMODULE hmod, LPCTSTR pid);
ICONRESOURCE* ReadIconFromICOFile(LPCTSTR pszName);
int EnumIconFromFile(LPCTSTR pszName, CSimpleArray<CIconItem*>& arr);
ICONRESOURCE* GetIconResourceFromFile(LPCTSTR pszName, int index);
int ExportMultipleIcons(LPCTSTR pszName, LPCTSTR pszTo, int* pnIndex, int entries);
BOOL ExportSingleIcon(LPCTSTR pszName, LPCTSTR pszExportIconName, int index);
BOOL WriteIconToStream(IStream* pstrm, LPICONRESOURCE pir, DWORD* pnumWritten);
HGLOBAL WriteIconToMemory(HGLOBAL hmem, LPICONRESOURCE pir, DWORD* pnumWritten);
BOOL WriteIconToICOFile(LPICONRESOURCE pir, LPCTSTR szFileName);
BOOL WriteIconToPNGFile(ICONIMAGE* pii, LPCTSTR szFileName);
int MakeIconDataObject(LPCTSTR pszName, CSimpleArray<CIconItem*>& parrIcon, ICONDATAOBJECT* pido);
void CopyIconsToClipboard(HICON* phics, int numIcons, COLORREF clrBack);
BOOL GetDefaultIconPath(LPCTSTR pszName, LPTSTR pszIconPath, int* pIndex);


////////////////////////////////////////////////////////////////////////
//

class CIconListManager
{
public:
	CString m_name;
	CString m_strIconPath;
	CSimpleMap<int, HIMAGELIST> m_mapHim;
	CSimpleArray<CIconItem*> m_arrIcon;
	CSimpleArray<CString> m_arrTempFiles;				// 作業用アイコンファイルのリスト

	CIconListManager() {}
	~CIconListManager()
	{
		int i, cnt = m_arrIcon.GetSize();
		for (i = 0; i < cnt; i++) {
			delete m_arrIcon[i];
		}
		// 不要になったHIMAGELISTを削除する
		cnt = m_mapHim.GetSize();
		for (i = 0; i < cnt; i++) {
			HIMAGELIST him = m_mapHim.GetValueAt(i);
			if (him) {
				ImageList_Destroy(him);
			}
		}
		// 不要になった作業用アイコンファイルを削除する
		cnt = m_arrTempFiles.GetSize();
		for (i = 0; i < cnt; i++) {
			::DeleteFile(m_arrTempFiles[i]);
		}
	}

	int Initialize(LPCTSTR pszFileName)
	{
		m_name = pszFileName;
		m_strIconPath = m_name;
		int numIcons = EnumIconFromFile(m_name, m_arrIcon);
		if (numIcons < 1) {
			int index;
			TCHAR buf[MAX_PATH * 2];
			if (GetDefaultIconPath(m_name, buf, &index)) {
				if (index == INT_MAX) {
					// アイコンファイルそのもの
					m_strIconPath = buf;
				} else {
					TCHAR temp[MAX_PATH * 2];
					GetTempPath(ARRAYSIZE(temp), temp);
					PathAppend(temp, PathFindFileName(buf));
					wsprintf(&temp[lstrlen(temp)], _T(".(#%d).ico"), index);
					// 作業用アイコンファイルを作成する
					if (ExportSingleIcon(buf, temp, index)) {
						if (m_arrTempFiles.Find(temp) < 0)
							m_arrTempFiles.Add(temp);
						m_strIconPath = temp;
					}
				}
				numIcons = EnumIconFromFile(m_strIconPath, m_arrIcon);
			} else {
				m_strIconPath.Empty();
			}
		}
		return numIcons;
	}

	HIMAGELIST GetImageList(int width)
	{
		HIMAGELIST him = m_mapHim.Lookup(width);
		if (!him) {
			int numIcons = m_arrIcon.GetSize();
			him = ImageList_Create(width, width, ILC_COLOR32 | ILC_MASK | ILC_HIGHQUALITYSCALE, numIcons, 0);
			ImageList_SetBkColor(him, CLR_NONE);
			for (int i = 0; i < numIcons; i++) {
				HICON hic = m_arrIcon[i]->GetIcon(width);
				ImageList_ReplaceIcon(him, -1, hic);
			}
			m_mapHim.Add(width, him);
		}
		return him;
	}

	int GetIconCount() { return m_arrIcon.GetSize(); }
	LPCTSTR GetIconPath() { return m_strIconPath.IsEmpty() ? NULL : m_strIconPath.GetString(); }
};


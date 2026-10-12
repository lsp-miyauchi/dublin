// file.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include <commoncontrols.h>

#include "fv7.h"
#include "shellexec.h"
#include "mainfrm.h"
#include "execdlg.h"
#include "wildcard.h"
#include "htmlout.h"
#include "match.h"
#include "sort.h"
#include "codepage_output.h"
#include "drawimage_sup.h"

#define IMAGE_DRAW_USING_GDIPLUS
#define EXIF_LABEL_HEIGHT			13
#define STYLE_OTL_GAP_Y				2
#define MAX_NAME_COLUMN_LENGTH		800


_inline float CalcScaleRatio(uint32_t width, uint32_t height, uint32_t max_scaled_width, uint32_t max_scaled_height)
{
	float xr = (float)max_scaled_width / (float)width;
	float yr = (float)max_scaled_height / (float)height;
	return min(1.0f, min(xr, yr));
}

static const PROPERTYKEY PKEY_DosFileName = { { 0x41CF5AE0, 0xF75A, 0x4806, { 0xBD, 0x87, 0x59, 0xC7, 0xD9, 0x24, 0x8E, 0xB9 } }, 999 };


typedef int (__cdecl * QSORTCOMPPROC)(const void*, const void*, const void*);
void __cdecl qsort_ex(void *base, size_t n, size_t size, QSORTCOMPPROC, const void* param);

static SHELLCOLUMNINFO sciDefault[] = {
	{ 0, LVCFMT_LEFT,  20, SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_STR,  _T(""), PKEY_ItemNameDisplay }, 
	{ 1, LVCFMT_RIGHT, 10, SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_INT,  _T(""), PKEY_Size },
	{ 2, LVCFMT_LEFT,  18, SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_DATE, _T(""), PKEY_DateModified },
	{ 3, LVCFMT_LEFT,  16, SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_STR,  _T(""), PKEY_ItemTypeText },
	{ 4, LVCFMT_LEFT,   6, SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_STR,  _T(""), PKEY_FileAttributes },
	{ 5, LVCFMT_LEFT,  14, SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_STR,  _T(""), PKEY_DosFileName }
};
static int sciWidth[6] = { -1, -1, -1, -1, -1, -1 };

static SHELLCOLUMNINFO sciUnknown[2] = {
	{ 0, LVCFMT_LEFT,  20, SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_STR,  _T(""), PKEY_ItemNameDisplay }, 
	{ 1, LVCFMT_LEFT,  40, SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_STR,  _T(""), PKEY_ItemTypeText },
};

/////////////////////////////////////////////////////////////////////////////
//

#define RETRIEVE_TIMEOUT		800
#define TIMERFLAG_RETRIEVE		0x00000001
#define TIMERFLAG_RECALC		0x00000002
#define TIMERFLAG_RESORT		0x00000004
#define DTEX_SINGLE				(DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS)
#define DTEX_SINGLE_NE			(DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE)
#define MAX_FILESTACK_SIZE		512


//////////////////////////////////////////////////////////////////////

class CShellCompare
{
public:
	CComPtr<IShellFolder> m_psf;
	int m_type;
	int m_seq;
	ULONG m_column;

public:
	CShellCompare(IShellFolder* psf, int sort, int seq, int col)
		: m_psf(psf)
		, m_type(sort)
		, m_seq(seq)
		, m_column(col)
	{
		if (m_psf.p != NULL)
			m_column = 0;
	}

	~CShellCompare()
	{
	}

	_inline int Compare(CFileItem* p1, CFileItem* p2) const
	{
		if (m_psf.p != NULL && m_column == 0)
			return (int)(short)HRESULT_CODE(m_psf->CompareIDs(0, p1->m_pidl, p2->m_pidl));
		else if (p1->m_columnvalue[m_column] != p2->m_columnvalue[m_column])
			return (p1->m_columnvalue[m_column] < p2->m_columnvalue[m_column]) ? -1 : 1;
		else {
			int (WINAPI* fncmp)(LPCTSTR, LPCTSTR);
			fncmp = (m_type == SORT_SIZE) ? StrCmpLogicalW : CompareFileName;
			int r = fncmp(p1->m_column[m_column], p2->m_column[m_column]);
			if (r == 0)
				r = (int)(short)HRESULT_CODE(m_psf->CompareIDs(m_column, p1->m_pidl, p2->m_pidl));
			return r;
		}
	}

	_inline bool isSortDown() const
	{
		return (m_seq == SORT_DOWN);
	}
};

static int __cdecl compare_name(const void* arg1, const void* arg2, const void* plm)
{
	CFileItem* p1 = *(CFileItem**)arg1;
	CFileItem* p2 = *(CFileItem**)arg2;
	CShellCompare* pci = (CShellCompare*)plm;

	if (p1->isUpdir()) return -1;
	if (p2->isUpdir()) return 1;

	int r;
	if (!p1->isRawFile() || !p2->isRawFile()) {
		r = pci->Compare(p1, p2);
	} else {
		if (p1->m_fileattr & FAT_SUBDIR) {
			if (!(p2->m_fileattr & FAT_SUBDIR)) {
				return -1;
			}
		} else if (p2->m_fileattr & FAT_SUBDIR) {
			return 1;
		}
		r = CompareFileName(p1->m_column[0], p2->m_column[0]);
	}

	// 昇順・降順の決定
	if (pci->isSortDown())
		r *= -1;
    return r;
}

static int __cdecl compare_ext(const void* arg1, const void* arg2, const void* plm)
{
	CFileItem* p1 = *(CFileItem**)arg1;
	CFileItem* p2 = *(CFileItem**)arg2;
	CShellCompare* pci = (CShellCompare*)plm;

	if (p1->isUpdir())		return -1;
	else if (p2->isUpdir())	return 1;

	int r;
	if (!p1->isRawFile() || !p2->isRawFile()) {
		r = pci->Compare(p1, p2);
	}
	else {
		if (p1->m_fileattr & FAT_SUBDIR) {
			if (!(p2->m_fileattr & FAT_SUBDIR))
				return -1;
			else {
				r = CompareFileName(p1->m_column[0], p2->m_column[0]);
				goto name_end;
			}
		}
		else if (p2->m_fileattr & FAT_SUBDIR)
			return 1;

		LPCTSTR ps1 = p1->m_column[0];
		LPCTSTR ps2 = (LPCTSTR)StrRChr(ps1, NULL, _T('.'));
		LPCTSTR pt1 = p2->m_column[0];
		LPCTSTR pt2 = (LPCTSTR)StrRChr(pt1, NULL, _T('.'));
		if (!ps2 || !pt2) {
			if (ps2)
				r = 1;
			else if (pt2)
				r = -1;
			else
				r = 0;
		}
		else
			r = CompareFileName(ps2, pt2);

		if (!r)
			r = CompareFileName(ps1, pt1);
	}

	// 昇順・降順の決定
name_end:
	if (pci->isSortDown())
		r *= -1;
    return r;
}

static int __cdecl compare_size(const void* arg1, const void* arg2, const void* plm)
{
	CFileItem* p1 = *(CFileItem**)arg1;
	CFileItem* p2 = *(CFileItem**)arg2;
	CShellCompare* pci = (CShellCompare*)plm;

	if (p1->isUpdir()) return -1;
	if (p2->isUpdir()) return 1;

	int r;
	if (!p1->isRawFile() || !p2->isRawFile()) {
		r = pci->Compare(p1, p2);
	}
	else {
		if (p1->m_fileattr & FAT_SUBDIR) {
			if (p2->m_fileattr & FAT_SUBDIR) {
				if      (p1->m_filesize < p2->m_filesize)  r = -1;
				else if (p1->m_filesize == p2->m_filesize) r = CompareFileName(p1->m_column[0], p2->m_column[0]);
				else                                       r = 1;
				goto size_end;
			}
			else
				return -1;
		}
		else if (p2->m_fileattr & FAT_SUBDIR)
			return 1;

		if      (p1->m_filesize < p2->m_filesize)  r = -1;
		else if (p1->m_filesize == p2->m_filesize) r = CompareFileName(p1->m_column[0], p2->m_column[0]);
		else                                       r = 1;
	}

	// 昇順・降順の決定
size_end:
	if (pci->isSortDown())
		r *= -1;
    return r;
}

static int __cdecl compare_time(const void* arg1, const void* arg2, const void* plm)
{
	CFileItem* p1 = *(CFileItem**)arg1;
	CFileItem* p2 = *(CFileItem**)arg2;
	CShellCompare* pci = (CShellCompare*)plm;

	if (p1->isUpdir()) return -1;
	if (p2->isUpdir()) return 1;

	int r;
	if (!p1->isRawFile() || !p2->isRawFile()) {
		r = pci->Compare(p1, p2);
	}
	else {
		if (p1->m_fileattr & FAT_SUBDIR) {
			if (!(p2->m_fileattr & FAT_SUBDIR))
				return -1;
		}
		else if (p2->m_fileattr & FAT_SUBDIR)
			return 1;

		LONG l;
		l = CompareFileTime(&p1->m_ftLastWrite, &p2->m_ftLastWrite);
		r = (l == 0) ? CompareFileName(p1->m_column[0], p2->m_column[0]) : (int)l;
	}

	// 昇順・降順の決定
	if (pci->isSortDown())
		r *= -1;
    return r;
}

static int __cdecl compare_attr(const void* arg1, const void* arg2, const void* plm)
{
	CFileItem* p1 = *(CFileItem**)arg1;
	CFileItem* p2 = *(CFileItem**)arg2;
	CShellCompare* pci = (CShellCompare*)plm;

	if (p1->isUpdir()) return -1;
	if (p2->isUpdir()) return 1;

	int r;
	if (!p1->isRawFile() || !p2->isRawFile()) {
		r = pci->Compare(p1, p2);
	}
	else {
		if (p1->m_fileattr & FAT_SUBDIR) {
			if (!(p2->m_fileattr & FAT_SUBDIR))
				return -1;
		}
		else if (p2->m_fileattr & FAT_SUBDIR)
			return 1;

		if (p1->m_fileattr < p2->m_fileattr)
			r = -1;
		else if (p1->m_fileattr == p2->m_fileattr)
			r = CompareFileName(p1->m_column[0], p2->m_column[0]);
		else
			r = 1;
	}

	// 昇順・降順の決定
	if (pci->isSortDown())
		r *= -1;
    return r;
}

static int __cdecl compare_dos(const void* arg1, const void* arg2, const void* plm)
{
	CFileItem* p1 = *(CFileItem**)arg1;
	CFileItem* p2 = *(CFileItem**)arg2;
	CShellCompare* pci = (CShellCompare*)plm;

	if (p1->isUpdir())		return -1;
	else if (p2->isUpdir())	return 1;

	int r;
	if (!p1->isRawFile() || !p2->isRawFile()) {
		r = pci->Compare(p1, p2);
	}
	else {
		if (p1->m_fileattr & FAT_SUBDIR) {
			if (!(p2->m_fileattr & FAT_SUBDIR))
				return -1;
		}
		else if (p2->m_fileattr & FAT_SUBDIR)
			return 1;

		LPCTSTR pstr1 = p1->m_column[5] ? p1->m_column[5] : &gchNil;
		LPCTSTR pstr2 = p2->m_column[5] ? p2->m_column[5] : &gchNil;
		r = CompareFileName(pstr1, pstr2);
		if (!r)
			r = CompareFileName(p1->m_column[0], p2->m_column[0]);
	}

	// 昇順・降順の決定
	if (pci->isSortDown())
		r *= -1;
    return r;
}

static int __cdecl compare_etc(const void* arg1, const void* arg2, const void* plm)
{
	CFileItem* p1 = *(CFileItem**)arg1;
	CFileItem* p2 = *(CFileItem**)arg2;

	if (p1->isUpdir()) return -1;
	if (p2->isUpdir()) return 1;

	int r = p1->m_serial - p2->m_serial;
    return r;
}

static int __cdecl compare_column(const void* arg1, const void* arg2, const void* plm)
{
	CFileItem* p1 = *(CFileItem**)arg1;
	CFileItem* p2 = *(CFileItem**)arg2;

	if (p1->isUpdir()) return -1;
	if (p2->isUpdir()) return 1;

	CShellCompare* pci = (CShellCompare*)plm;
	if (p1->isFolder()) {
		if (!p2->isFolder())
			return -1;
	}
	else if (p2->isFolder())
		return 1;

	int r = pci->Compare(p1, p2);

	// 昇順・降順の決定
	if (pci->isSortDown())
		r *= -1;

    return r;
}

int CFileBrowseView::MapSortTypeToIndex(int nsType)
{
	int index = -1;
	// ソート対象をカラム情報から決定する
	switch (nsType) {
	case SORT_NAME:
		index = m_stack.FindScid(PKEY_ItemNameDisplay);
		if (index < 0)
			index = m_stack.FindScid(PKEY_FileName);
		if (index < 0)
			index = 0;			// 強制
		break;
	case SORT_EXT:
		index = m_stack.FindScid(PKEY_ItemTypeText);
		if (index < 0)
			index = m_stack.FindScid(PKEY_ItemType);
		if (index < 0)
			index = m_stack.FindScid(PKEY_Category);
		if (index < 0)
			index = m_stack.FindScid(PKEY_FontStyle);
		break;
	case SORT_TIME:
		index = m_stack.FindScid(PKEY_DateModified);
		break;
	case SORT_SIZE:
		index = m_stack.FindScid(PKEY_Size);
		if (index < 0)
			index = m_stack.FindScid(PKEY_Capacity);
		break;
	case SORT_ATTR:
		index = m_stack.FindScid(PKEY_FileAttributes);
		if (index < 0)
			index = m_stack.FindScid(PKEY_Comment);
		if (index < 0)
			index = m_stack.FindScid(PKEY_FreeSpace);
		if (index < 0)
			index = m_stack.FindScid(PKEY_RecycleOriginalFolder);
		if (index < 0)
			index = m_stack.FindScid(PKEY_FontDesignTarget);
		break;
	case SORT_DOS:
		index = m_stack.FindScid(PKEY_DosFileName);
		break;
	case SORT_PATH:
		index = m_stack.FindScid(PKEY_ItemFolderNameDisplay);
		if (index < 0)
			index = m_stack.FindScid(PKEY_ItemFolderPathDisplay);
		if (index < 0)
			index = m_stack.FindScid(PKEY_ItemFolderPathDisplayNarrow);
		break;
	default:
		break;
	}

	if (index < 0) {
		switch (nsType) {
		case SORT_EXT:
			index = m_stack.FindColumnName(_T("種類;Type"), _T(";"), 1);
			break;
		case SORT_TIME:
			index = m_stack.FindColumnState(SHCOLSTATE_TYPE_DATE, 1);
			if (index <0)
				index = m_stack.FindColumnName(_T("日時;Modified;Accessed;Checked"), _T(";"), 1);
			break;
		case SORT_SIZE:
			index = m_stack.FindColumnState(SHCOLSTATE_TYPE_INT, 1);
			if (index <0)
				index = m_stack.FindColumnName(_T("サイズ;Size"), _T(";"), 1);
			break;
		}
	}

	return index;
}

static bool __find_keywords(LPCTSTR pszTarget, LPCTSTR pszKeywords)
{
	CString keywords = pszKeywords;
	int pos = 0;
	CString token = keywords.Tokenize(_T(";"), pos);
	while (token != _T("")) {
		if (StrStrI(pszTarget, token) != NULL)
			return true;
		token = keywords.Tokenize(_T(";"), pos);
	}
	return false;
}

int CFileBrowseView::MapSortIndexToType(int index)
{
	int nsType = SORT_NONE;
	// ソート対象をカラム情報から決定する
	if (0 <= index && index < (int)m_stack.m_numcol) {
		if		(index == 0 ||														// 強制
				 m_stack.m_colinfo[index].scid == PKEY_ItemNameDisplay ||
				 m_stack.m_colinfo[index].scid == PKEY_FileName)					nsType = SORT_NAME;
		else if (m_stack.m_colinfo[index].scid == PKEY_ItemTypeText ||
				 m_stack.m_colinfo[index].scid == PKEY_ItemType ||
				 m_stack.m_colinfo[index].scid == PKEY_Category ||
				 m_stack.m_colinfo[index].scid == PKEY_FontStyle)					nsType = SORT_EXT;
		else if (m_stack.m_colinfo[index].scid == PKEY_DateModified)				nsType = SORT_TIME;
		else if (m_stack.m_colinfo[index].scid == PKEY_Size ||
				 m_stack.m_colinfo[index].scid == PKEY_Capacity)					nsType = SORT_SIZE;
		else if (m_stack.m_colinfo[index].scid == PKEY_FileAttributes ||
				 m_stack.m_colinfo[index].scid == PKEY_Comment ||
				 m_stack.m_colinfo[index].scid == PKEY_FreeSpace ||
				 m_stack.m_colinfo[index].scid == PKEY_RecycleOriginalFolder ||
				 m_stack.m_colinfo[index].scid == PKEY_FontDesignTarget)			nsType = SORT_ATTR;
		else if (m_stack.m_colinfo[index].scid == PKEY_DosFileName)					nsType = SORT_DOS;
		else if (m_stack.m_colinfo[index].scid == PKEY_ItemFolderNameDisplay ||
				 m_stack.m_colinfo[index].scid == PKEY_ItemFolderPathDisplay ||
				 m_stack.m_colinfo[index].scid == PKEY_ItemFolderPathDisplayNarrow)	nsType = SORT_PATH;

		if (nsType == SORT_NONE) {
			if      (m_stack.GetColumnType(index) == SHCOLSTATE_TYPE_DATE)			nsType = SORT_TIME;
			else if (m_stack.GetColumnType(index) == SHCOLSTATE_TYPE_INT)			nsType = SORT_SIZE;
			// 以下、邪道
			else if (__find_keywords(m_stack.GetColumnName(index), _T("種類;Type")))
																					nsType = SORT_EXT;
			else if (__find_keywords(m_stack.GetColumnName(index), _T("日時;Modified;Accessed;Checked")))
																					nsType = SORT_TIME;
			else if (__find_keywords(m_stack.GetColumnName(index), _T("サイズ;Size")))
																					nsType = SORT_SIZE;
		}
	}
	return nsType;
}

class CSetPriority
{
private:
	HANDLE m_hThread;
	int m_prevPriority;
public:
	CSetPriority(int nPriority = THREAD_PRIORITY_ABOVE_NORMAL)
	{
		m_hThread = GetCurrentThread();
		m_prevPriority = GetThreadPriority(m_hThread);
		if (m_prevPriority != THREAD_PRIORITY_ERROR_RETURN && m_prevPriority != nPriority)
			SetThreadPriority(m_hThread, nPriority);
		else
			m_hThread = NULL;
	}
	~CSetPriority()
	{
		if (m_hThread)
			SetThreadPriority(m_hThread, m_prevPriority);
	}
};

void CFileBrowseView::SortItemArray(bool bUseShellCompare, int column, int nsSeq)
{
	m_sortColumn = column;
	int cnt = m_stack.GetSize();
	if (cnt > 1) {
		QSORTCOMPPROC pfn;
		// カラムインデクスからソート種別を取得
		int nsType = MapSortIndexToType(column);
		if (bUseShellCompare) {
			pfn = (column < 0) ? compare_etc : compare_column;
		}
		else {
			switch (nsType) {
				case SORT_NAME:	pfn = compare_name; break;
				case SORT_EXT:	pfn = compare_ext; break;
				case SORT_TIME:	pfn = compare_time; break;
				case SORT_SIZE:	pfn = compare_size; break;
				case SORT_ATTR:	pfn = compare_attr; break;
				case SORT_DOS:	pfn = compare_dos; break;
				default:		pfn = compare_etc; break;
			}
		}
#ifdef __HIPRIORITY_SORT
		CSetPriority priority(THREAD_PRIORITY_HIGHEST;
#endif
		CShellCompare sc(m_psf, nsType, nsSeq, column);
		qsort_ex(m_stack.GetData(), cnt, sizeof(CFileItem*), pfn, &sc);
	}
}

static HFONT makeBoldOrUnderlineFont(HDC hdc, bool bold = true, bool underline = false);

static HFONT makeBoldOrUnderlineFont(HDC hdc, bool bold, bool underline)
{
	LOGFONT lf = { 0 };
	HFONT hfont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
	if (hfont) {
		GetObject(hfont, sizeof(LOGFONT), &lf);
		if (bold) lf.lfWeight = FW_BOLD;
		if (underline) lf.lfUnderline = TRUE;
		hfont = CreateFontIndirect(&lf);
	}
	return hfont;
}

/////////////////////////////////////////////////////////////////////////////
// CFileBrowseView

CFileBrowseView::CFileBrowseView()
	: m_pDocument(NULL)
	, m_psf(NULL)
	, m_curattr(0)
	, m_numColumns(0)
	, m_sortColumn(-1)
	, m_bRetrieving(false)
	, m_bHeaderTracking(false)
	, m_timerid(0)
	, m_timerflag(0)
	, m_numMarks(0)
{
	m_stack.SetSize(MAX_FILESTACK_SIZE);

	// リストスタイル情報
	if (GetRegBinary(IDS_SEC_OPERATIONS, IDS_KEY_ICONLISTSTYLE, &m_ListStyle, sizeof(m_ListStyle)) != sizeof(m_ListStyle))
		m_ListStyle = gDefaultIconListStyle;

	// カラム名初期化
	if (sciDefault[0].title[0] == 0) {
		::LoadString(RESINST, IDS_COLUMN_NAME, sciDefault[0].title, 80);
		::LoadString(RESINST, IDS_COLUMN_SIZE, sciDefault[1].title, 80);
		::LoadString(RESINST, IDS_COLUMN_DATE, sciDefault[2].title, 80);
		::LoadString(RESINST, IDS_COLUMN_TYPE, sciDefault[3].title, 80);
		::LoadString(RESINST, IDS_COLUMN_ATTR, sciDefault[4].title, 80);
		::LoadString(RESINST, IDS_COLUMN_DOS,  sciDefault[5].title, 80);
		::LoadString(RESINST, IDS_COLUMN_NAME, sciUnknown[0].title, 80);
		::LoadString(RESINST, IDS_COLUMN_INFO, sciUnknown[1].title, 80);
	}

	// 初期化中の無用な0除算を防止
	for (int i = 0; i < MAXCOLUMNINFO; i++)
		m_sdt[i].cxChar = 1;

	// sys-image & icon size
	int cx, cy;
	SHGetImageList(SHIL_SMALL, IID_PPV_ARGS(&m_pilSmall));
	m_pilSmall->GetIconSize(&cx, &cy);
	m_sizeSmall.SetSize(cx, cy);

	SHGetImageList(SHIL_LARGE, IID_PPV_ARGS(&m_pilLarge));
	m_pilLarge->GetIconSize(&cx, &cy);
	m_sizeLarge.SetSize(cx, cy);

	SHGetImageList(SHIL_EXTRALARGE, IID_PPV_ARGS(&m_pilExtra));
	m_pilExtra->GetIconSize(&cx, &cy);
	m_sizeExtra.SetSize(cx, cy);
}

CFileBrowseView::~CFileBrowseView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFileBrowseView クラスの初期化

static UINT GetCharFromVirtualKey(UINT nVKey)
{
	BYTE kbState[256];
	GetKeyboardState(kbState);

	WCHAR wszCode[2];
	int res = ToUnicode(nVKey, MapVirtualKey(nVKey, 0), kbState, wszCode, 2, 0);
	return (res == 1) ? wszCode[0] : 0;
}

BOOL CFileBrowseView::ProcessMessage(const MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_ESCAPE) {
			ToggleFindBar();
		}
	}
	return FALSE;
}

int CFileBrowseView::OnCreate(LPCREATESTRUCT pcs)
{
ImmAssociateContext(m_hWnd, NULL);

	m_pDocument = (CDocument*)pcs->lpCreateParams;
	ATLASSERT(m_pDocument != NULL);

#ifdef _DEBUG
	CString tit = APP_PRODUCT;
	tit += _T(" - FileBrowser");
	SetWindowText(tit);
#else
	SetWindowText(APP_PRODUCT);
#endif // _DEBUG

	// 詳細表示のカラム幅
	int vals[6];
	if (GetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_LISTCOLUMN, (BYTE*)vals, sizeof(vals)) == sizeof(sciWidth)) {
		for (int i = 0; i < 6; i++)
			sciWidth[i] = vals[i];
	}

	// 検索バー
	HWND hwnd = lccCreateToolBar(m_hWnd, RESINST, MAKEINTRESOURCE(IDB_FINDBAR), ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, IDW_FINDBAR);
	if (hwnd) {
		m_finder.SubclassWindow(hwnd);
		m_finder.Initialize(0, 2110);
		AddCustomDrawBar(hwnd);
		// チェックボタン
		m_finder.CheckButton(IDM_FINDER_REGEXP, (gnFopStat & FOS_REXFINDER) != 0);
		// 最初は非表示
		m_finder.ShowWindow(SW_HIDE);
	}

	// リストウィンドウ
	CreateListWindow();
	ResetFont();

	// イメージGrep
	m_ImageGrep.UseSystemCache(MAKETHUMBCACHECODE(gnFileStat));				// システムキャッシュの優先利用
	m_ImageGrep.SetImageSize(CSize(X_IMAGESIZE_MAX, Y_IMAGESIZE_MAX));		// 160x160 固定

	// CIdleHandlerを登録する  --> OnDestroyで解除
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddIdleHandler(this);

	return 0;
}

BOOL CFileBrowseView::OnIdle()
{
	if (m_finder.IsWindowVisible())
		m_finder.OnIdle();
	return FALSE;
}

void CFileBrowseView::OnShellChangeNotify(long lEvent, PIDLIST_ABSOLUTE* pidls)
{
	int index;
	CFileItem* pf;

	switch (lEvent) {
	case SHCNE_MKDIR:
	case SHCNE_RENAMEFOLDER:
	case SHCNE_CREATE:
	case SHCNE_RENAMEITEM:
	case SHCNE_UPDATEDIR:
		ResetTimer(TIMERFLAG_RETRIEVE);
		break;

	case SHCNE_RMDIR:
	case SHCNE_DELETE:
	case SHCNE_UPDATEITEM:
		index = FindItemIndex(pidls[0], true);
		if (index >= 0) {
			pf = m_stack[index];
			if (lEvent == SHCNE_UPDATEITEM)
				ModifyItem(pf);
			else {
				m_stack.RemoveAt(index);
				if (pf->m_filesize)
					m_dlTotal -= pf->m_filesize;
				m_list.DeleteItem(index);
				delete pf;
			}
		}
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CFileBrowseView クラスの更新

void CFileBrowseView::SetExtendedListStyle()
{
	// フラグ設定を元に、DLViewの拡張設定をおこなう
	DWORD mask = DLS_EX_ONECLICKACTIVATE | DLS_EX_HOTTRACK | DLS_EX_WRAPCURSOR;
	DWORD exStyle = 0;
	if ((gnMosStat & MOS_LBTN_MASK) == MOS_LBTN_ONECLICK)	exStyle |= DLS_EX_ONECLICKACTIVATE;
	if (gnMosStat & MOS_HOTTRACK)							exStyle |= DLS_EX_HOTTRACK;
	if (gnListStat & LTS_WRAPCURSOR)						exStyle |= DLS_EX_WRAPCURSOR;
	m_list.SetExtendedStyle(exStyle, mask);
}

void CFileBrowseView::UpdateView(LPARAM lHint)
{
	int index;
	switch (lHint) {
	case DCM_CHANGEFORMAT:
	{
		m_ImageGrep.UseSystemCache(MAKETHUMBCACHECODE(gnFileStat));		// システムキャッシュの優先利用

		ResetFont();
		CWaitCursor wc;

		index = m_list.GetTopIndex();
		m_list.SetRedraw(FALSE);
		RecalcColumn();

		SetExtendedListStyle();

		m_list.SetTopIndex(index);

		m_list.SetRedraw(TRUE);
		m_list.RedrawWindow();
		break;
	}

	case DCM_VIEWSTYLE:
	{
		int style = 0;
		if      (m_ListStyle.isStyleReport()) style = DLS_STYLE_REPORT;
		else if (m_ListStyle.isStyleNormal()) style = DLS_STYLE_LIST;
		else                                  style = DLS_STYLE_ICON;

		index = m_list.GetNextItem(-1, DLNI_FOCUSED);
		m_list.SetRedraw(FALSE);
		m_list.SetListStyle(style);
		RecalcColumn();

		if (m_stack.GetSize() > 0 && m_ListStyle.isStyleImage() && !m_ImageGrep.isRunning()) {
			m_ImageGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep, 2);
		}
		m_list.SetRedraw(TRUE);

		CRect rc;
		GetClientRect(&rc);
		if (m_finder.IsWindowVisible()) {
			// サーチファインダー分を補正
			CRect rcw;
			m_finder.GetWindowRect(&rcw);
			rc.bottom -= rcw.Height() + 4;
		}
		m_list.Layout(&rc);

		if (index >= 0) m_list.EnsureVisible(index, FALSE);
		break;
	}

	default:
		break;
	}
}

void CFileBrowseView::ResetFont()
{
	if (!m_font.IsNull())
		m_font.DeleteObject();

	LOGFONT lf;
	if (!LoadFontInfo(&lf, IDS_SEC_SETTINGS, IDS_KEY_FILEFONT))
		_GCDATA.GetIconTitleLogFont(&lf);
	m_font.CreateFontIndirect(&lf);
	m_list.SetBkColor(gColors[colBack]);

	CClientDC dc(NULL);
	HFONT hfont = dc.SelectFont(m_font);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	m_sizeChar.cx = tm.tmAveCharWidth;
	m_sizeChar.cy = tm.tmHeight + tm.tmExternalLeading;
	dc.SelectFont(hfont);
}

void CFileBrowseView::SelectItem(LPITEMIDLIST pidlSingle)
{
	int index = FindItemIndex(pidlSingle);
	if (index >= 0)
		m_list.SetCurSel(index, FALSE);
}

void CFileBrowseView::OnSort(WPARAM wParam, LPARAM)
{
	// 現在のブラウザのソート情報を取得
	int seq = m_ListStyle.GetSortSeq();
	int index = m_sortColumn;
	switch (wParam) {
	case IDM_SORT_NONE: index = -1; break;
	case IDM_SORT_NAME: index = MapSortTypeToIndex(SORT_NAME); break;
	case IDM_SORT_SIZE: index = MapSortTypeToIndex(SORT_SIZE); break;
	case IDM_SORT_TIME: index = MapSortTypeToIndex(SORT_TIME); break;
	case IDM_SORT_TYPE: index = MapSortTypeToIndex(SORT_EXT); break;
	case IDM_SORT_ATTR: index = MapSortTypeToIndex(SORT_ATTR); break;
	case IDM_SORT_DOS:  index = MapSortTypeToIndex(SORT_DOS); break;
	case IDM_SORT_PATH: index = MapSortTypeToIndex(SORT_PATH); break;
	case IDM_SORT_COL0: index = 0; break;
	case IDM_SORT_COL1: index = 1; break;
	case IDM_SORT_COL2: index = 2; break;
	case IDM_SORT_COL3: index = 3; break;
	case IDM_SORT_COL4: index = 4; break;
	case IDM_SORT_COL5: index = 5; break;
	case IDM_SORT_COL6: index = 6; break;
	case IDM_SORT_COL7: index = 7; break;
	case IDM_SORT_COL8: index = 8; break;
	default:
		if (wParam == IDM_SORT_UP)
			seq = SORT_UP;
		else if (wParam == IDM_SORT_DOWN)
			seq = SORT_DOWN;
		if (seq != m_ListStyle.GetSortSeq())
			SortItems(index, seq);
		return;
	}

	if (index == m_sortColumn)
		index = -1;		// SORT_NONE
	SortItems(index, seq);
}

void CFileBrowseView::SortItems(int column, int newSeq)
{
	int oldSeq  = m_ListStyle.GetSortSeq();
	if (m_sortColumn == column && newSeq == oldSeq)
		return;

	int newSort = MapSortIndexToType(column);
	if (newSort < 0 && column >= 0)
		newSort = SORT_UNK;
	m_ListStyle.SetSortType(newSort);
	m_ListStyle.SetSortSeq(newSeq);
	SetHeaderBoundary(column);
	UpdateSort(column);
}

void CFileBrowseView::UpdateSort(int column)
{
	int cnt = m_stack.GetSize();
	if (cnt < 2) return;

	m_magicGrep = GetTickCount64();
	bool runFile = m_FileGrep.isRunning();
	bool runCol = m_ColGrep.isRunning();
	bool runDir = m_DirGrep.isRunning();
	bool runImage = m_ImageGrep.isRunning();
	bool runIcon = m_IconGrep.isRunning();

	// 各スレッドのアイドル状態のチェック
	if (runFile)	m_FileGrep.Stop();
	if (runCol)		m_ColGrep.Stop();
	if (runDir)		m_DirGrep.Stop();
	if (runImage)	m_ImageGrep.Stop();
	if (runIcon)	m_IconGrep.Stop();

	m_list.EnableToolTips(FALSE);

	CWaitCursor wc;
	m_list.SetRedraw(FALSE);

	// キャレットを持つデータを保存
	CFileItem* pfCaret;
	int i = m_list.GetCaretIndex();
	if (i >= 0)
		pfCaret = m_stack[i];
	else
		pfCaret = NULL;

	// ファイルの選択状態を保存
	CFileItem* pf;
	for (i = 0; i < cnt; i++) {
		pf = m_stack[i];
		pf->m_fChecked = (m_list.GetSel(i) != FALSE);
		if (pf->m_fChecked)
			m_list.SetSel(i, FALSE);
	}

	// ファイルエントリを再リストアップ
	SortItemArray(!isFileSystemFolder(), column, m_ListStyle.GetSortSeq());

	// ファイルの選択状態とキャレットを持つデータを復元
	for (i = 0; i < cnt; i++) {
		pf = m_stack[i];
		if (pf->m_fChecked) {
			m_list.SetSel(i);
			pf->m_fChecked = false;
		}
		if (pf == pfCaret) {
			m_list.SetCaretIndex(i);
			m_list.SetAnchorIndex(i);
		}
	}

	m_list.SetRedraw(TRUE);
	m_list.RedrawWindow();

	if (runFile) m_FileGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep);
	if (runCol) m_ColGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep);
	if (runDir) m_DirGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep);
	if (runImage) m_ImageGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep, 2);
	if (runIcon) m_IconGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep);
}

void CFileBrowseView::StopDirectoryParsing()
{
	// フォルダのロックを無効にするため、ディレクトリパースを中止する
	m_DirGrep.Stop();

	// ファイルのロックを無効にするためイメージパースを中止する
	if (m_ImageGrep.isRunning())
		m_ImageGrep.Stop();
}

void CFileBrowseView::OnNotifySetFocus(NMHDR* pNMHDR, LRESULT* pResult)
{
	fvApp.SetActiveView(m_hWnd);
	RedrawSelItems();
}

void CFileBrowseView::OnNotifyKillFocus(NMHDR* pNMHDR, LRESULT* pResult)
{
	RedrawSelItems();
}

void CFileBrowseView::OnTimer(UINT_PTR nIDEvent)
{
	KillTimer(m_timerid);
	m_timerid = 0;
	if (nIDEvent == m_magicNotify) {
		if (m_timerflag & TIMERFLAG_RETRIEVE)
			ShellBrowse(NULL);
		else if (m_timerflag & TIMERFLAG_RESORT) {
			// 現在のカラムインデクスでソートをおこなう
			UpdateSort(m_sortColumn);
		}
		else if (m_timerflag & TIMERFLAG_RECALC) {
			m_list.SetRedraw(FALSE);
			RecalcColumn();
			m_list.SetRedraw(TRUE);
			m_list.Invalidate(FALSE);
		}
	}
	m_timerflag = 0;
}

void CFileBrowseView::ResetTimer(DWORD dwAdd)
{
	if (m_timerid) {
		KillTimer(m_timerid);
		m_timerid = 0;
	}

	if (dwAdd) {
		m_timerflag |= dwAdd;
		m_timerid = SetTimer(m_magicNotify, RETRIEVE_TIMEOUT);
	}
}

void CFileBrowseView::ModifyItem(CFileItem* pf)
{
	if (!pf) return;

	CString str;
	CWin32FindData fd;
	idl_GetName(m_psf, pf->m_pidl, SHGDN_FORPARSING, str);
	_overridefileattributes(str, &fd);

	bool bResort = false;

	// 属性が変わった
	if (fd.dwFileAttributes != pf->m_fileattr) {
		pf->m_fileattr = fd.dwFileAttributes;
		pf->m_column[4] = fd.FormatFileAttributes(str);

		if (m_ListStyle.isSortAttr())
			bResort = true;
	}

	// ファイルサイズが変わった
	DWORDLONG size;
	SETDWORDLONG(size, fd.nFileSizeLow, fd.nFileSizeHigh);
	if (!fd.isFolder() && size != pf->m_filesize) {
		m_dlTotal += (size - pf->m_filesize);
		FormatFileSize(size, pf->m_column[1], gnListStat & LTS_FMTSIZE);
		pf->m_filesize = size;

		if (m_ListStyle.isSortSize())
			bResort = true;
	}

	// 時刻が変わった
	FILETIME lt;
	FileTimeToLocalFileTime(&fd.ftLastWriteTime, &lt);
	if (CompareFileTime(&lt, &pf->m_ftLastWrite))  {
		FormatFileTime(&lt, pf->m_column[2], gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
		pf->m_ftLastWrite = lt;

		if (m_ListStyle.isSortTime())
			bResort = true;
	}

	ResetTimer(bResort ? TIMERFLAG_RESORT : TIMERFLAG_RECALC);
}

void CFileBrowseView::RecalcColumn()
{
	CClientDC dc(m_hWnd);
	HFONT hfont = dc.SelectFont(m_font);
/* --- 不要？
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	m_sizeChar.cx = tm.tmAveCharWidth;
	m_sizeChar.cy = tm.tmHeight + tm.tmExternalLeading;
---------- */
	if      (m_ListStyle.isStyleNormal())	RecalcColumnList(dc);
	else if (m_ListStyle.isStyleOutline())	RecalcColumnOutline(dc);
	else if (m_ListStyle.isStyleReport()) 	RecalcColumnReport(dc);
	else if (m_ListStyle.isStyleIcon())		RecalcColumnIcon(dc);
	else if (m_ListStyle.isStyleImage())	RecalcColumnImage(dc);

	RebuildHeaderColumns();

	dc.SelectFont(hfont);
}

static TCHAR sszDirSizeSample[] = _T("*9999 バイト");

void CFileBrowseView::RecalcColumnList(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);
	int maxNameColumnWidth = max(MAX_NAME_COLUMN_LENGTH, rc.Width());

	CSize sizeIcon = m_ListStyle.isEnlargeNormal() ? m_sizeLarge : m_sizeSmall;

	m_nMaxName = m_sizeChar.cx * 8;
	m_nMaxExt = 0;

	int maxFile = 0;
	int maxFolder = 0;
	int maxSize = m_sizeChar.cx * 6;
	int maxDate = 0;
	int maxAttr = 0;
	bool bHasFSdir = false;

	CSize size;
	CFileItem *pf;

	bool bHasDrives = false;
	bool bHasFileSize = false;
	bool bHasLastWriteTime = false;
	int len;
	int cnt = m_stack.GetSize();
	for (int i = 0; i < cnt; i++) {
		pf = m_stack[i];
		CString str = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			str.MakeLower();
		len = str.GetLength();

		// ファイル名長
		CalcTextDimension(hdc, str, len, &size);
		int dx = size.cx;
		if (pf->isFolder())
			dx = min(dx, maxNameColumnWidth);
		else {
			if (gnListStat & LTS_ALIGNEXT) {
				int pos = str.ReverseFind(_T('.'));
				if (pos >= 0) {
					LPCTSTR p = str;
					CalcTextDimension(hdc, p + pos, len - pos, &size);
					dx -= size.cx;
					m_nMaxExt = max(m_nMaxExt, size.cx);
				}
			}
			maxFile = max(maxFile, dx);
		}
		m_nMaxName = max(m_nMaxName, dx);

		if (pf->isFSDir())
			bHasFSdir = true;
		maxFolder = max(maxFolder, dx);

if (pf->isUpdir())
	continue;

		if (m_rawfiles > 0 && pf->isRawFile()) {
			// サイズ長
			if (!pf->m_column[1].IsEmpty()) {
				CalcTextDimension(hdc, pf->m_column[1], -1, &size);
				maxSize = max(size.cx, maxSize);
			}

			// 日付長
			if (!pf->m_column[2].IsEmpty()) {
				CalcTextDimension(hdc, pf->m_column[2], -1, &size);
				maxDate = max(size.cx, maxDate);
			}
			// 属性長
			if (!pf->m_column[4].IsEmpty()) {
				CalcTextDimension(hdc, pf->m_column[4], -1, &size);
				maxAttr = max(size.cx, maxAttr);
			}
		} else if (pf->isDrive()) {
			bHasDrives = true;
			// 日付長は書式化されたスペース情報
			if (pf->GetDriveUsage(str)) {
				CalcTextDimension(hdc, str, -1, &size);
				maxSize = max(size.cx, maxSize);
			}
		} else {
			if (pf->m_filesize > 0) {
				// サイズ
				bHasFileSize = true;
				TCHAR temp[64];
				StrFormatByteSize64(pf->m_filesize, temp, ARRAYSIZE(temp));
				CalcTextDimension(hdc, temp, -1, &size);
				maxSize = max(size.cx, maxSize);
			}
			if (pf->hasLastWriteTime()) {
				// 日付
				bHasLastWriteTime = true;
				FormatFileTime(&pf->m_ftLastWrite, str, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
				CalcTextDimension(hdc, str, -1, &size);
				maxDate = max(size.cx, maxDate);
			}
		}
	}

	if (gnListStat & LTS_ALIGNEXT) {
		if ((maxFile > 0 && maxFolder > 0) && (maxFile + m_nMaxExt < maxFolder))
			m_nMaxName = maxFolder - m_nMaxExt;

		if (m_ListStyle.isShowEllp()) {
			m_nMaxName = min(m_nMaxName, m_sizeChar.cx * gnElmaxNameLength);
			m_nMaxExt = min(m_nMaxExt, m_sizeChar.cx * gnElmaxExtLength);
		}
		m_nMaxExt += 8;
	} else {
		if (m_ListStyle.isShowEllp())
			m_nMaxName = min(m_nMaxName, m_sizeChar.cx * (gnElmaxNameLength + gnElmaxExtLength));
		m_nMaxName += 8;
	}

	m_sdt[0].cxChar = m_nMaxName + m_nMaxExt;
	m_sdt[0].cxChar += sizeIcon.cx + 4;
	len = m_sdt[0].cxChar;

	int addlen = 0;
	if (m_rawfiles > 0) {
		// サイズ長
		if (m_ListStyle.isShowSize()) {
			if (bHasFSdir) {
				CalcTextDimension(hdc, sszDirSizeSample, -1, &size);
				maxSize = max(maxSize, size.cx);
			}
			m_sdt[1].cxChar = maxSize + 8;
			addlen += m_sdt[1].cxChar;
		}
		// 日付時刻長
		if (m_ListStyle.isShowDate()) {
			m_sdt[2].cxChar = maxDate + 8;
			addlen += m_sdt[2].cxChar;
		}
		// 属性長
		if (m_ListStyle.isShowAttr()) {
			m_sdt[4].cxChar = maxAttr + 8;
			addlen += m_sdt[4].cxChar;
		}
		if (addlen > 0) addlen += 4;
	} else if (bHasDrives) {
		// サイズ長
		if (m_ListStyle.isShowSize()) {
			m_sdt[1].cxChar = maxSize + 8;
			addlen += m_sdt[1].cxChar;
		}
		// 日付時刻長
		if (m_ListStyle.isShowDate()) {
			m_sdt[2].cxChar = maxDate + 8;
			addlen += m_sdt[2].cxChar;
		}
		// 属性長（使用率バーを出す）
		if (m_ListStyle.isShowAttr()) {
			m_sdt[4].cxChar = 96 + 8;
			addlen += m_sdt[4].cxChar;
		}
	} else {
		// サイズ長
		if (bHasFileSize && m_ListStyle.isShowSize()) {
			m_sdt[1].cxChar = maxSize + 8;
			addlen += m_sdt[1].cxChar;
		}
		if (bHasLastWriteTime && m_ListStyle.isShowDate()) {
			// 日付長
			m_sdt[2].cxChar = maxDate + 8;
			addlen += m_sdt[2].cxChar;
		}
	}
	// カラムの幅と高さ
	size = CSize(len + addlen, max(sizeIcon.cy + 1, m_sizeChar.cy));
	if (gnListStat & LTS_RELAX) {
		size.cy += size.cy >> 1;
	} else {
		size.cy += size.cy >> 3;
	}
	m_list.SetItemSize(size);
}

void CFileBrowseView::RecalcColumnOutline(HDC hdc)
{
	CSize sizeIcon = m_ListStyle.isEnlargeOutline() ? m_sizeExtra : m_sizeLarge;

	// 日付文字列を基準に最低幅を求める
	FILETIME ft = { 0 };
	CString str;
	FormatFileTime(&ft, str, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
	CSize size;
	CalcTextDimension(hdc, str, -1, &size);
	m_nMaxName = size.cx;
	m_nMaxExt = 0;

	// ファイル名はBoldフォント
	HFONT hfont = (gnListStat & LTS_BOLDNAME) ? makeBoldOrUnderlineFont(hdc) : NULL;

	CFileItem* pf;
	int cnt = m_stack.GetSize();
	for (int i = 0; i < cnt; i++) {
		pf = m_stack[i];
if (pf->isUpdir())
	continue;
		CString s = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			s.MakeLower();
		// 名前長
		if (hfont) hfont = (HFONT)SelectObject(hdc, hfont);
		CalcTextDimension(hdc, s, -1, &size);
		if (hfont) hfont = (HFONT)SelectObject(hdc, hfont);
		m_nMaxName = max(m_nMaxName, size.cx);
	}
	if (hfont)
		DeleteObject(hfont);

	// 最大の名前長を適用
	m_nMaxName = min(m_nMaxName, m_sizeChar.cx * (gnElmaxNameLength + gnElmaxExtLength));
	m_sdt[0].cxChar = m_nMaxName + sizeIcon.cx + 4 + 4 + 4 + 2;
	int width = m_sdt[0].cxChar;

	CRect rc;
	gpMainFrame->GetBrowserPaneRect(&rc);
	int realwidth = rc.Width() - GetSystemMetrics(SM_CXVSCROLL);
	int numcol = realwidth / width;
	int delta = (numcol == 0) ? realwidth : realwidth / numcol;
	width = min(delta, width + 32);
	int height = max(sizeIcon.cy, m_sizeChar.cy * min(4, m_numColumns) + 2) + STYLE_OTL_GAP_Y * 2 + 2;

	// カラムの幅と高さ
	size = CSize(width, height);
	m_list.SetItemSize(size);
}

void CFileBrowseView::RecalcColumnReport(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);
	int maxNameColumnWidth = max(MAX_NAME_COLUMN_LENGTH, rc.Width());

	CSize sizeIcon = m_ListStyle.isEnlargeReport() ? m_sizeLarge : m_sizeSmall;

	m_nMaxName = m_sizeChar.cx * 8;
	m_nMaxExt = 0;

	int maxFile = 0;
	int maxFolder = 0;

	memset(m_colWidthMax, 0, sizeof(m_colWidthMax));
	int i;
	if (m_rawfiles == 0) {
		// 実フォルダで無い場合はカラム情報を参照
		for (i = 0; i < m_numColumns; i++) {
//			m_colWidthMax[i] = max(m_sizeChar.cx * 6, m_stack.m_colinfo[i].cxChar);
			m_colWidthMax[i] = m_sizeChar.cx * 8;
		}
	}
	CSize size;
	CFileItem *pf;
	int len;
	int cnt = m_stack.GetSize();
	for (i = 0; i < cnt; i++) {
		pf = m_stack[i];
		CString str = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			str.MakeLower();
		len = str.GetLength();

		// ファイル名長
		CalcTextDimension(hdc, str, len, &size);
		int dx = size.cx;
		if (pf->isFSFile()) {
			if (gnListStat & LTS_ALIGNEXT) {
				int pos = str.ReverseFind(_T('.'));
				if (pos >= 0) {
					LPCTSTR p = str;
					CalcTextDimension(hdc, p + pos, len - pos, &size);
					dx -= size.cx;
					m_nMaxExt = max(m_nMaxExt, size.cx);
				}
			}
			maxFile = max(maxFile, dx);
		}
		m_nMaxName = max(m_nMaxName, dx);

		if (pf->isFSDir())
			maxFolder = max(maxFolder, dx);

if (pf->isUpdir())
	continue;

		int col;
		for (col = 1; col < m_numColumns; col++) {
			if (pf->m_column[col]) {
				CalcTextDimension(hdc, pf->m_column[col], -1, &size);
				m_colWidthMax[col] = max(m_colWidthMax[col], size.cx);
			}
		}
	}

	if (gnListStat & LTS_ALIGNEXT) {
		if ((maxFile > 0 && maxFolder > 0) && (maxFile + m_nMaxExt < maxFolder))
			m_nMaxName = maxFolder - m_nMaxExt;
		if (m_ListStyle.isShowEllp()) {
			m_nMaxName = min(m_nMaxName, m_sizeChar.cx * gnElmaxNameLength);
			m_nMaxExt = min(m_nMaxExt, m_sizeChar.cx * gnElmaxExtLength);
		}
	} else if (m_ListStyle.isShowEllp()) {
		m_nMaxName = min(m_nMaxName, m_sizeChar.cx * (gnElmaxNameLength + gnElmaxExtLength));
	}
	m_colWidthMax[0] = max(m_colWidthMax[0], m_nMaxName + m_nMaxExt);

	// 自動調整ＯＮか、初期表示時の場合
	int xWidth = 0;

	if (!(gnListStat & LTS_AUTOADJUST) && sciWidth[0] >= 0) {
		for (i = 0; i < m_numColumns; i++) {
			int type = MapSortIndexToType(i);
			if (type != SORT_NONE)
				m_sdt[i].cxChar = sciWidth[type];
			else
				m_sdt[i].cxChar = max(m_sizeChar.cx * 5, min(maxNameColumnWidth, m_colWidthMax[i])) + 8;
			xWidth += m_sdt[i].cxChar;
		}
	} else {
		for (i = 0; i < m_numColumns; i++) {
			if (i == 0) {
				m_sdt[0].cxChar = min(maxNameColumnWidth, m_nMaxName + m_nMaxExt);
				m_sdt[0].cxChar += sizeIcon.cx + 4 + 8;
			} else {
				m_sdt[i].cxChar = max(m_sizeChar.cx * 5, min(maxNameColumnWidth, m_colWidthMax[i])) + 8;
			}
			xWidth += m_sdt[i].cxChar;
			m_list.SetHeaderColumnWidth(i, m_sdt[i].cxChar);

			int type = MapSortIndexToType(i);
			if (type != SORT_NONE)
				sciWidth[type] = m_sdt[i].cxChar;
		}
	}

	m_colWidthMax[0] += 8 + sizeIcon.cx + 4;
	for (i = 1; i < m_numColumns; i++)
		m_colWidthMax[i] += 8;

	m_list.SetHorizontalExtent(xWidth);
	// カラムの高さ
	size = CSize(xWidth, max(sizeIcon.cy + 1, m_sizeChar.cy));
	if (gnListStat & LTS_RELAX) {
		size.cy += size.cy >> 1;
	}
	else {
		size.cy += size.cy >> 3;
	}
	m_list.SetItemSize(size);
}

void CFileBrowseView::RecalcColumnIcon(HDC hdc)
{
	CSize sizeIcon = m_ListStyle.isEnlargeIcon() ? m_sizeExtra : m_sizeLarge;

	// 日付文字列を基準に幅を求める
	FILETIME ft = { 0 };
	CString str;
	FormatFileTime(&ft, str, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
	CSize size;
	CalcTextDimension(hdc, str, str.GetLength() - 3, &size);
	int width = max(_GCDATA.m_IconMetrics.iHorzSpacing, size.cx) + 2 + 4;	// 項目矩形間のギャップと文字列描画時の左右paddingを付加
	m_sdt[0].cxChar = width;		// ベース幅を記録 OnSize での再計算の際に重要

	CRect rc;
	gpMainFrame->GetBrowserPaneRect(&rc);
	int realwidth = rc.Width() - GetSystemMetrics(SM_CXVSCROLL);
	int numcol = realwidth / width;
	int delta = (numcol == 0) ? realwidth : realwidth / numcol;
	width = min(delta, width + 32);

	CFileItem* pf;
	int showDate = 0;
	int showAttr = 0;
	int maxNameWidth = 0;
	int maxNameHeight = m_sizeChar.cy;
	int cnt = m_stack.GetSize();
	for (int i = 0; i < cnt; i++) {
		pf = m_stack[i];
		CString s = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			s.MakeLower();

		// ファイル名長（左右2px分を引いて計算）
		CalcTextRectDimension(hdc, s, s.GetLength(), DT_WORDBREAK | DT_NOPREFIX | DT_CENTER, width - 4, &size);
		maxNameWidth = max(maxNameWidth, size.cx);
		maxNameHeight = max(maxNameHeight, size.cy);

if (pf->isUpdir())
	continue;

		if (!pf->isDrive()) {
			// ドライブ項目以外
			if (m_ListStyle.isShowDate() && pf->hasLastWriteTime()) showDate++;
			if (m_ListStyle.isShowAttr() && pf->m_fileattr) showAttr++;
		}
	}
	m_nMaxName = width;
	m_nMaxExt = 0;

	int numline = min(3, maxNameHeight / m_sizeChar.cy);
	if (m_ListStyle.isShowSize()) numline++;
	if (showDate) numline++;
	if (showAttr) numline++;
	// 上下マージン1px + アイコン上下 3pxマージン + 名前上下 2pxマージン
	int height = sizeIcon.cy + m_sizeChar.cy * numline + 2 + 6 + 4;

	// カラムの幅と高さ
	size = CSize(width, height);
	m_list.SetItemSize(size);
}

void CFileBrowseView::RecalcColumnImage(HDC hdc)
{
	m_nMaxName = 0;
	m_nMaxExt = 0;

	CSize size;
	CFileItem* pf;

	HFONT hfont = NULL;
	if (gnListStat & LTS_BOLDNAME)
		hfont = makeBoldOrUnderlineFont(hdc);

	// シャドウ 5px 上下左右padding 3px 上下左右margin 1px
	int width = m_ListStyle.size_thumb.cx + 3 + 3 + 1 + 1 + m_dropshadow.delta();
	// クライアント矩形の幅から妥当なアイテム幅を計算
	CRect rc;
	gpMainFrame->GetBrowserPaneRect(&rc);
	int realwidth = rc.Width() - GetSystemMetrics(SM_CXVSCROLL);
	int numcol = realwidth / width;
	width = (numcol == 0) ? realwidth : realwidth / numcol;
	m_sdt[0].cxChar = width;

	int maxNameHeight = m_sizeChar.cy;
	int cnt = m_stack.GetSize();
	int showDate = 0;
	int showExif = 0;
	for (int i = 0; i < cnt; i++) {
		pf = m_stack[i];
		CString str = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			str.MakeLower();

		// ファイル名長
		if (hfont) hfont = (HFONT)SelectObject(hdc, hfont);
		CalcTextRectDimension(hdc, str, str.GetLength(), DT_WORDBREAK | DT_NOPREFIX | DT_CENTER, width, &size);
		if (hfont) hfont = (HFONT)SelectObject(hdc, hfont);
		m_nMaxName = max(m_nMaxName, size.cx);
		maxNameHeight = max(maxNameHeight, size.cy);

if (pf->isUpdir())
	continue;

		if (m_ListStyle.isShowDate() && pf->hasLastWriteTime()) showDate++;
		if ((gnFileStat & FTS_EXIF) && pf->GetPerceivedType() == PERCEIVED_TYPE_IMAGE) showExif++;
	}
	if (hfont) DeleteObject(hfont);

	// 上下マージン 1 + 1 = 2px
	// イメージエリア上下パディング 3 + 3 = 6px
	// 文字列の上下マージン 2 + 2 = 4px
	// シャドウ 5px
	int height = m_ListStyle.size_thumb.cy + min(maxNameHeight, m_sizeChar.cy * 2) + 12 + m_dropshadow.delta();
	if (showDate) height += m_sizeChar.cy;
	if (m_ListStyle.isShowSize()) height += m_sizeChar.cy;
	// EXIF情報を表示する場合、1行分をさらに追加
	if (showExif) height += m_sizeChar.cy;

	size = CSize(width, height);
	m_list.SetItemSize(size);
}

static int __lookupScid(const CSimpleArray<SHELLCOLUMNINFO>& arr, SHCOLUMNID scid)
{
	int cnt = arr.GetSize();
	for (int i = 0; i < cnt; i++) {
		if (arr[i].scid == scid)
			return i;
	}
	return -1;
}

int CFileBrowseView::ShellBrowse(LPCITEMIDLIST pidlBaseFolder)
{
	if (!m_psf && !pidlBaseFolder)
		return -1;

	// 再入チェック
	if (m_bRetrieving)
		return 0;
	m_bRetrieving = true;

	// 引数のpidlBaseFolderがNULLなら、カレントフォルダの再読み
	CIDListPtr idlFolder;
	if (pidlBaseFolder)
		idlFolder.Clone(pidlBaseFolder);

	m_magicGrep = 0;
	m_list.EnableToolTips(FALSE);
	m_FileGrep.Stop();
	m_ColGrep.Stop();
	m_DirGrep.Stop();
	m_ImageGrep.Stop();
	m_IconGrep.Stop();
	m_WatchFolder.Stop();				// ※※※他スレッドより後に終了させる必要がある
	m_arrSaveSelection.RemoveAll();		// 選択情報破棄

	int iLastCaret = m_list.GetCaretIndex();
	int iTopIndex = m_list.GetTopIndex();

	CWaitCursor wc;
	m_list.SetRedraw(FALSE);
	m_list.ResetContent();

	// 検索バーを隠す
	if (m_finder.IsWindowVisible())
		ToggleFindBar();

	// 既に取得したイメージを保存
	CSimpleMap<CString, CFileItem*> arrBackup;
	if (idlFolder.isNull() && isFileSystemFolder()) {
		int cnt = m_stack.GetSize();
		for (int s = 0; s < cnt; s++) {
			CFileItem* pf = m_stack[s];
			if (pf && pf->m_bmp) {
				arrBackup.Add(pf->m_column[0], pf);
				m_stack[s] = (CFileItem*)NULL;
			}
		}
	}

	m_stack.DeleteAll(!idlFolder.isNull());

	int i;
	if (!idlFolder.isNull()) {
		m_stack.SetSize(MAX_FILESTACK_SIZE);

		// 当該フォルダのシェルフォルダインタフェース取得
		m_idlFolder.Free();
		if (m_psf) {
			m_psf->Release();
			m_psf = NULL;
		}

		m_idlFolder.Clone(idlFolder);
		if (m_idlFolder.isDesktop())
			SHGetDesktopFolder(&m_psf);
		else
			m_psf = m_idlFolder.GetShellFolder();
		ATLASSERT(m_psf != NULL);

		// SFGAO_attributesを取得
		m_curattr = m_idlFolder.GetAttributes(SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM | SFGAO_STREAM | SFGAO_LINK | SFGAO_CAPABILITYMASK);
		// 現在のパスを取得
		m_strCurPath.Empty();
		if (m_idlFolder.GetPath(m_strCurPath)) {
			CString str;
			if (getlinkinfo(m_strCurPath, str) && PathIsURL(str)) {
				m_strCurPath.Empty();
			}
			else {
				// ドライブ単位のアクセス先を記録
				int drive = PathGetDriveNumber(m_strCurPath);
				if (drive >= 0)
					fopSetDirRemind(drive, m_strCurPath);
			}
		}

		ULONG sortDefaultColumn = 0;
		m_numColumns = 0;
		if (isFileSystemFolder()) {
			// ファイルシステムならばデフォルトのカラム
			for (i = 0; i < ARRAYSIZE(sciDefault); i++) {
				m_stack.m_colinfo[m_numColumns++] = sciDefault[i];
			}
			if (gnListStat & LTS_NODOSNAME)
				m_numColumns--;
			CSimpleArray<CString> a, b;
			a = b;
		}
		else {
			CSimpleArray<SHELLCOLUMNINFO> arr;
			int cols = idl_GetShellColumnInfo(m_psf, arr, &sortDefaultColumn, NULL);

			// いくつか標準項目を抽出する
			static const SHCOLUMNID scidStandard[] = {
				PKEY_ItemNameDisplay,			// 名前
				PKEY_Size,						// サイズ
				PKEY_DateModified,				// 更新日時
				PKEY_ItemTypeText				// 種別
			};
			for (i = 0; i < ARRAYSIZE(scidStandard); i++) {
				int index = __lookupScid(arr, scidStandard[i]);
				if (index >= 0) {
					// 表示カラムが実カラムの何番目に対応するかを保存
					m_stack.m_colinfo[m_numColumns++] = arr[index];
					arr.RemoveAt(index);
				}
			}

			// 残りのデータを処理
			cols = arr.GetSize();
			for (i = 0; i < cols; i++) {
				// とりあえず8カラムまで
				if (m_numColumns >= 8) break;

// ---- 取得したscidの内容ダンプ ----
// dbg.printf(L"%d: %s\n   %s\n", i, arr[i].title, ScidNameDump(arr[i].scid));
				if (arr[i].state & SHCOLSTATE_ONBYDEFAULT) {
					// 表示カラムが実カラムの何番目に対応するかを保存
					m_stack.m_colinfo[m_numColumns++] = arr[i];
				}
			}
			if (!m_numColumns) {
				// カラム情報を取得できないときは次の２カラムのみ
				m_stack.m_colinfo[0] = sciUnknown[0];
				m_stack.m_colinfo[1] = sciUnknown[1];
				m_numColumns = 2;
			}
		}
		m_stack.m_numcol = m_numColumns;

		// ソートしない設定以外の場合で、以前と一致するソート種別がない場合には
		// 標準のソートカラムに対応する種別をセット
		int sortType = m_ListStyle.GetSortType();
		if (sortType != SORT_NONE && MapSortTypeToIndex(sortType) < 0)
			m_ListStyle.SetSortType(MapSortIndexToType(sortDefaultColumn));

		// カラムの文字幅をピクセルに変換
		for (i = 0; i < m_numColumns; i++) {
			m_stack.m_colinfo[i].cxChar *= m_sizeChar.cx;
			m_sdt[i] = m_stack.m_colinfo[i];
		}
	}

	// ファイルテーブルの初期化
	m_dlTotal     = 0L;
	m_dlSelTotal = 0L;

	// フォルダの内容を取得
	int numItems = BrowseFolder(TRUE);
	if (numItems > 1 && !m_ListStyle.isSortNone()) {
		int sort = MapSortTypeToIndex(m_ListStyle.GetSortType());
		if (sort < 0) sort = m_sortColumn;
		SortItemArray(!isFileSystemFolder(), sort, m_ListStyle.GetSortSeq());
	}

	// バックアップしたイメージを戻す
	if (arrBackup.GetSize() > 0) {
		CFileItem* pfw;
		int pos;
		for (pos = 0; pos < numItems; pos++) {
			CFileItem* pf = m_stack[pos];
			pfw = arrBackup.Lookup(pf->m_column[0]);
			if (pfw && pfw->isMatch(*pf)) {
				pf->m_perceived = pfw->m_perceived;
				pf->m_bmp = pfw->m_bmp; pfw->m_bmp = NULL;
				pf->m_infotip = pfw->m_infotip;
				pf->m_totalsize = pfw->m_totalsize;
				if (pfw->m_exifptr)
					pf->m_exifptr = new CExifInformation(*pfw->m_exifptr);
			}
		}

		int left = arrBackup.GetSize();
		for (pos = 0; pos < left; pos++) {
			pfw = arrBackup.GetValueAt(pos);
			delete pfw;
		}
		arrBackup.RemoveAll();
	}

	m_list.SetCount(numItems);

	// カラムサイズを変更
	RecalcColumn();

	m_list.SetRedraw(TRUE);
	if (!idlFolder.isNull() && numItems > 0)
		m_list.SetCurSel(0, TRUE);
	else if (idlFolder.isNull() && iLastCaret >= 0 && iTopIndex >= 0 && numItems > 0) {
		iLastCaret = min(iLastCaret, numItems - 1);
		iTopIndex = min(iTopIndex, numItems - 1);
		m_list.SetTopIndex(iTopIndex);
		m_list.SetCurSel(iLastCaret, TRUE);
	}
	m_list.Invalidate(FALSE);

	m_magicNotify = GetTickCount64();
	if (_pathisdirectory(m_strCurPath)) {
		// ディレクトリ監視
		m_WatchFolder.Start(m_hWnd, m_strCurPath, m_magicNotify);
	}
	else if (!idlFolder.isNull()) {
		// シェル更新追跡機能の初期化
		bool bWatchFolder = true;
		if (gnFileStat & FOS_NOWATCHRECYCLEFOLDER) {
			// ごみ箱を監視しない？
			CIDListPtr idlRecycle;
			idlRecycle.GetKnownFolder(FOLDERID_RecycleBinFolder);
			bWatchFolder = (idlFolder != idlRecycle);
		}
		if (bWatchFolder) {
			StartShellChangeNotify(idlFolder,
						SHCNE_MKDIR |
						SHCNE_RMDIR |
						SHCNE_RENAMEFOLDER |
						SHCNE_CREATE |
						SHCNE_DELETE |
						SHCNE_RENAMEITEM |
						SHCNE_UPDATEDIR |						// たくさんのファイルを一度に削除した場合などで通知
						SHCNE_UPDATEITEM,
						FALSE);
		}
	}

	if (gnListStat & LTS_INFOTIP) m_list.EnableToolTips(TRUE);

	m_magicGrep = GetTickCount64();
	if (numItems > 0) {
		// イメージ情報
		if (m_ListStyle.isStyleImage())
			m_ImageGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep, 2);
		// アイコン情報
		m_IconGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep);
		if (isFileSystemFolder()) {
			// ファイル情報
			m_FileGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep);
			// フォルダサイズ
			if (gnListStat & LTS_DSZ_MASK) {
				DWORD state = 0;
				if (PathIsUNC(m_strCurPath))
					state |= LTS_DSZ_REMOTE;
				else {
					int type = DriveType(PathGetDriveNumber(m_strCurPath));
					switch (type) {
						case DRIVE_RAMDISK:
						case DRIVE_FIXED: state = LTS_DSZ_FIXED; break;
						case DRIVE_CDROM: state = LTS_DSZ_CDROM; break;
						case DRIVE_REMOTE: state = LTS_DSZ_REMOTE; break;
						case DRIVE_REMOVABLE: state = LTS_DSZ_REMOVABLE; break;
					}
				}
				if (state & gnListStat)
					m_DirGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep);
			}
		}
		else
			m_ColGrep.Start(m_hWnd, m_psf, &m_stack, m_magicGrep);
	}

	m_bRetrieving = false;
	return numItems;
}

int CFileBrowseView::BrowseFolder(BOOL /*bRoot*/)
{
	m_rawfiles = 0;

	CFilterSetting* wildcard = m_pDocument->GetFilter();
	ATLASSERT(wildcard != NULL);

	// マッチ方法
	int matchType = (gnFopStat & FOS_PREFIX_MATCH) ? MATCHTYPE_PREFIX : MATCHTYPE_PARTIAL;
	if (wildcard->m_strFilter.Compare(gszAst) && wildcard->m_bUseRegExp)
		matchType = MATCHTYPE_REGEXP;
	CMatchSpec ms(wildcard->m_strFilter, matchType, wildcard->m_bCaseSense);

	int i, nSerial = 0;
	CFileItem* pf;
	CFolderTreeWindow tree = gpMainFrame->GetFolderTree();
	ATLASSERT(tree.IsWindow());

	// 「ひとつ上へ」を追加表示するかどうかの判定
	if (gnListStat & LTS_UPDIR) {
		bool bUpdir = false;
		if (!(gnSysStat & STS_DRIVEROOT)) {
			if (!m_idlFolder.isDesktop()) {
				// ツリーが「デスクトップ以下を表示」且つ現在フォルダがデスクトップでない場合
				bUpdir = true;
				if ((gnListStat & LTS_UPDIR_NOROOT) &&
					!m_strCurPath.IsEmpty() && PathIsRoot(m_strCurPath) && !PathIsUNC(m_strCurPath)) {
					bUpdir = false;
				}
			}
		}
		else {
			// ツリーが「デスクトップ以下を表示」でない且つ現在フォルダがツリーのルートフォルダの子フォルダの場合
			CIDListPtr idlRoot;
			idlRoot.Clone(m_idlFolder);
			idlRoot.MakeRoot();
			if ((idlRoot != m_idlFolder) && m_idlFolder.isParent(idlRoot))
				bUpdir = true;
		}

		if (bUpdir) {
			CString str; str.LoadString(IDS_UPDIR);

			pf = new CFileItem;
			pf->m_column[0] = str;
			pf->m_fUpdir = true;
			pf->m_serial = nSerial++;
			pf->m_iImage = Shell_GetCachedImageIndex(fvApp.GetMyModuleName(), -IDI_UPDIR, FALSE);
			m_stack.Add(pf);
		}
	}

//@ SHCONTF_FASTITEMSを指定するとftpサイトのデータが取得できない
//@	DWORD shcontf = SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FASTITEMS | SHCONTF_INCLUDESUPERHIDDEN;
	DWORD shcontf = SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN;
	if (m_curattr & SFGAO_FILESYSANCESTOR)
		shcontf |= SHCONTF_FASTITEMS;
	if (!(gnFileStat & FTS_FOLDER))
		shcontf |= SHCONTF_FOLDERS;

	CComPtr<IEnumIDList> pe;
	if (m_psf->EnumObjects(GetTopLevelParent(), shcontf, &pe) == S_OK) {
		CComQIPtr<IShellFolder2> psf2 = m_psf;
		CComQIPtr<IShellIcon> psic = m_psf;
		CComQIPtr<IShellIconOverlay> psio = m_psf;

		LPITEMIDLIST pidl;
		CWin32FindData wfd;
		CString str;

		// time-zone取得
		TIME_ZONE_INFORMATION tz;
		GetTimeZoneInformation(&tz);

		VARIANT vr;
		while (pe->Next(1, &pidl, NULL) == NOERROR) {
			// WindowsXPのZIP書庫サポートだと、SFGAO_HASSUBFOLDERフラグを
			// 指定すると、属性取得に時間がかかる！
			SFGAOF ula = SFGAO_FOLDER | SFGAO_STREAM | SFGAO_SYSTEM | SFGAO_FILESYSTEM | SFGAO_HIDDEN | SFGAO_LINK | SFGAO_CAPABILITYMASK;
			// エラーチェック & 表示ファイルタイプの制限をチェック
			if (m_psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &ula) != NOERROR ||
				((gnFileStat & FTS_SUPERHIDDEN) && (ula & SFGAO_HIDDEN) && (ula & SFGAO_SYSTEM)) ||
				((gnFileStat & FTS_HIDDEN) && (ula & SFGAO_HIDDEN)))
			{
				CoTaskMemFree((void*)pidl);
				continue;
			}

			pf = new CFileItem;
			pf->m_attr = ula;
			pf->m_pidlFq = ILCombine(m_idlFolder, pidl);
			pf->m_pidl = ILFindLastID(pf->m_pidlFq);
			pf->GetDescriptionId(m_psf);

//@			if (isFileSystemFolder() && (pf->isFSDir() || pf->isFSFile())) {
			if (isFileSystemFolder()) {
				if (pf->m_attr & SFGAO_FILESYSTEM) {
					m_rawfiles++;
					pf->m_fRawFile = true;

					wfd.cAlternateFileName[0] = 0;
					// 4GB以上のファイルは下位データが0xFFFFFFFFになる
					// バグじゃねーか
					if (SHGetDataFromIDList(m_psf, pidl, SHGDFIL_FINDDATA, &wfd, sizeof(wfd)) != S_OK ||
						wfd.nFileSizeLow == 0xFFFFFFFF)
					{
						idl_GetName(m_psf, pidl, SHGDN_FORPARSING, str);
						_getfileinfo(str, &wfd);
					}

					// desktop.ini, thumbs.db, .DS_Store の非表示
					if ((gnFileStat & FTS_SYSDB) &&
						(!lstrcmpi(wfd.cFileName, _T("desktop.ini")) ||
						 !lstrcmpi(wfd.cFileName, _T("thumbs.db")) ||
						 !lstrcmpi(wfd.cFileName, _T(".ds_store")))) {
						delete pf;
						continue;
					}

					pf->m_fileattr = wfd.dwFileAttributes;
					// 表示ファイルタイプの制限をファイル属性でもチェック
					if (((gnFileStat & FTS_SUPERHIDDEN) && pf->hasFileAttr(FILE_ATTRIBUTE_SYSTEM) && pf->hasFileAttr(FILE_ATTRIBUTE_HIDDEN)) ||
						((gnFileStat & FTS_HIDDEN) && pf->hasFileAttr(FILE_ATTRIBUTE_HIDDEN)))
					{
						delete pf;
						continue;
					}

					// 書庫はフォルダ扱いしない（拡張子をそろえる表示ができないため）
					if (!pf->isFSDir() && pf->isFolder() && pf->hasStream() && !pf->isLink())
						pf->m_attr &= ~SFGAO_FOLDER;

					SETDWORDLONG(pf->m_filesize, wfd.nFileSizeLow, wfd.nFileSizeHigh);
					wfd.LocalLastWriteTime(&pf->m_ftLastWrite);
					m_dlTotal += pf->m_filesize;
					// 名前
					pf->m_column[0] = wfd.cFileName;
					// サイズ
					if (!(pf->m_fileattr & FAT_SUBDIR))
						FormatFileSize(pf->m_filesize, pf->m_column[1], gnListStat & LTS_FMTSIZE);
					// 更新日時
					FormatFileTime(&pf->m_ftLastWrite, pf->m_column[2], gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
					// 種別
					pf->m_column[3] = pf->GetTypeName(str);
					// 属性
					pf->m_column[4] = wfd.FormatFileAttributes(str);
					// DOS
					if (wfd.cAlternateFileName[0] && StrCmpI(wfd.cFileName, wfd.cAlternateFileName))
						pf->m_column[5] = wfd.cAlternateFileName;
				}
				else if (psf2.p) {
					// 名前
					if (psf2->GetDetailsEx(pidl, &PKEY_ItemNameDisplay, &vr) == S_OK) {
						pf->m_column[0] = vr.bstrVal;
						VariantClear(&vr);
					}
					// サイズ
					if (psf2->GetDetailsEx(pidl, &PKEY_Size, &vr) == S_OK) {
						pf->m_filesize = vr.ullVal;
						m_dlTotal += pf->m_filesize;
						FormatFileSize(vr.ullVal, pf->m_column[1], gnListStat & LTS_FMTSIZE);
						StrFormatByteSize64(vr.ullVal, str.GetBuffer(128), 128);
						VariantClear(&vr);
					}
					// 日時
					if (psf2->GetDetailsEx(pidl, &PKEY_DateModified, &vr) == S_OK) {
						SYSTEMTIME st, lt;
						VariantTimeToSystemTime(vr.date, &st);
						SystemTimeToLocalTime(&st, &lt);
						FormatSystemTime(lt, pf->m_column[2], gnListStat & LTS_FMTDATE);
						SystemTimeToFileTime(&lt, &pf->m_ftLastWrite);
						VariantClear(&vr);
					}
					// 種別
					if (psf2->GetDetailsEx(pidl, &PKEY_ItemTypeText, &vr) == S_OK) {
						pf->m_column[3] = vr.bstrVal;
						VariantClear(&vr);
					}
				}
			}
			else {
				if (psf2.p) {
					for (i = 0; i < m_numColumns; i++) {
						// カラム情報を取得するのに時間がかかるカラムについては
						// CColGrepの別スレッドで情報を取得する。ただし最初のエントリは取得必須
						str.Empty();
						if (i == 0 || pf->isDrive() || !(m_stack.m_colinfo[i].state & SHCOLSTATE_SLOW)) {
							if (m_stack.m_colinfo[i].scid == PKEY_Null ||
								(!idl_GetShellPropertyText(psf2, pidl, m_stack.m_colinfo[i].scid, str, &pf->m_columnvalue[i]) &&
								 !idl_GetPropertyStoreText(pf->m_pidlFq, m_stack.m_colinfo[i].scid, str, &pf->m_columnvalue[i])))
								idl_GetShellColumnText(psf2, pidl, m_stack.m_colinfo[i].iCol, str);
							pf->m_column[i] = str;
							if (m_stack.m_colinfo[i].scid == PKEY_Size) {					// サイズ
								pf->m_filesize = pf->m_columnvalue[i];
								m_dlTotal += pf->m_filesize;
							}
							else if (m_stack.m_colinfo[i].scid == PKEY_DateModified)		// 日時
								*((DWORDLONG*)&pf->m_ftLastWrite) = pf->m_columnvalue[i];

							if (pf->isDrive()) {
								// ドライブ使用状況
								if (m_stack.m_colinfo[i].scid == PKEY_Capacity)				// トータルキャパ
									pf->m_totalsize = pf->m_columnvalue[i];
								if (m_stack.m_colinfo[i].scid == PKEY_FreeSpace)			// 空き容量
									pf->m_filesize = pf->m_columnvalue[i];
							}
						}
					}
				}
				else {
					idl_GetName(m_psf, pidl, SHGDN_INFOLDER, str);
					pf->m_column[0] = str;
					pf->m_column[1] = pf->GetTypeName(str);
				}

				if (!pf->isDrive() && pf->m_attr & SFGAO_FILESYSTEM) {
					// 物理ファイルなら情報を取得
					pf->m_fRawFile = true;
					wfd.cAlternateFileName[0] = 0;
					if (SHGetDataFromIDList(m_psf, pidl, SHGDFIL_FINDDATA, &wfd, sizeof(wfd)) != S_OK ||
						wfd.nFileSizeLow == 0xFFFFFFFF)
					{
						idl_GetName(m_psf, pidl, SHGDN_FORPARSING, str);
						_getfileinfo(str, &wfd);
					}

					pf->m_fileattr = wfd.dwFileAttributes;
					SETDWORDLONG(pf->m_filesize, wfd.nFileSizeLow, wfd.nFileSizeHigh);
					wfd.LocalLastWriteTime(&pf->m_ftLastWrite);
				}
			}

			// フィルタチェック
			if (wildcard->m_bIncludeFolder || !pf->isFolder()) {
				BOOL bMatch = ms.Match(pf->m_column[0]);
				if (wildcard->m_bUnmatchFilter == bMatch) {
					delete pf;
					continue;
				}
			}

			// カスタムアイコン指定をチェック
			int index;
			if (pf->isFSFile()) {
				index = fvApp.LookupIconMap(pf->m_column[0]);
				if (index >= 0)
					pf->m_iImage = index;
			}

			if (pf->m_iImage < 0 && psic.p) {
				// 非同期読み込みを指定して読んで、読めたときはそのまま（高速に読めた）
				if (psic->GetIconOf(pidl, GIL_FORSHELL | GIL_ASYNC, &index) == NOERROR)
					pf->m_iImage = index;
			}
			if (pf->m_iImage < 0) {
				// 通常のファイルなら、デフォルトアイコンを取得してダーティフラグをセット
				if (pf->isFSFile()) {
					SHFILEINFO sfi;
					SHGetFileInfo(pf->m_column[0], pf->m_fileattr, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
					pf->m_iImage = sfi.iIcon;
					pf->m_fDirtyIcon = true;
				}
			}

			// オーバーレイアイコンをチェック
			if (psio.p) {
				psio->GetOverlayIndex(pidl, &pf->m_iOverlayImage);
			}
			pf->m_serial = nSerial++;
			m_stack.Add(pf);
		}
	}
	return m_stack.GetSize();
}

void CFileBrowseView::GetItemIconImage(CFileItem* pf)
{
	CComQIPtr<IShellIcon> psi = m_psf;
	if (!psi || (S_OK != psi->GetIconOf(pf->m_pidl, GIL_FORSHELL, &pf->m_iImage))) {
		pf->m_iImage = SHMapPIDLToSystemImageListIndex(m_psf, pf->m_pidl, NULL);
	}
}

void CFileBrowseView::GetTotalInformation(LPTSTR pszText, int cchMax)
{
	if (!pszText) return;

	m_dlSelTotal = CalcSelectedFileSize();

	CString str;
	str.Format(IDS_INDICATOR_TOTAL, m_stack.GetSize());
	if (m_dlTotal > 0) {
		TCHAR buf[80];
		StrFormatByteSize64(m_dlTotal, buf, ARRAYSIZE(buf));
		str.AppendFormat(_T("(%s)"), buf);
	}
	StrCpyN(pszText, str, cchMax);
}

void CFileBrowseView::GetSelectInformation(LPTSTR pszText, int cchMax)
{
	if (!pszText) return;

	bool nowMatching = (m_finder.IsWindowVisible() != FALSE);
	int numSelect = nowMatching ? m_numMarks : m_list.GetSelCount();
	CString str;
	if (numSelect == 0) {
		str = L"選択なし";
	}
	else {
		str.Format(nowMatching ? IDS_INDICATOR_MATCH : IDS_INDICATOR_SELECT, numSelect);
		if (m_dlTotal > 0 && numSelect > 0) {
			TCHAR buf[80];
			StrFormatByteSize64(nowMatching ? m_dlMarkTotal : m_dlSelTotal, buf, ARRAYSIZE(buf));
			str.AppendFormat(_T(" (%s)"), buf);
		}
	}
	StrCpyN(pszText, str, cchMax);
}

DWORDLONG CFileBrowseView::CalcSelectedFileSize()
{
	DWORDLONG total = 0;
	int num = m_list.GetSelCount();
	if (num > 0) {
		CTypeAlloc<int> sels(num);
		num = m_list.GetSelItems(num, sels);
		CFileItem *pf;
		for (register int i = 0; i < num; i++) {
			pf = m_stack[sels[i]];
			total += pf->m_filesize;
		}
	}
	return total;
}

DWORDLONG CFileBrowseView::CalcMarkedFileSize()
{
	DWORDLONG total = 0;
	int num = m_list.GetMarkCount();
	if (num > 0) {
		CTypeAlloc<int> sels(num);
		num = m_list.GetMarkItems(num, sels);
		CFileItem *pf;
		for (register int i = 0; i < num; i++) {
			pf = m_stack[sels[i]];
			total += pf->m_filesize;
		}
	}
	return total;
}

/////////////////////////////////////////////////////////////////////////////
// CFileBrowseView メッセージ ハンドラ

BOOL CFileBrowseView::CreateListWindow()
{
	int style = 0;
	if      (m_ListStyle.isStyleReport())	style = DLS_STYLE_REPORT;
	else if (m_ListStyle.isStyleNormal())	style = DLS_STYLE_LIST;
	else									style = DLS_STYLE_ICON;

	CRect rc;
	GetClientRect(&rc);
	m_list.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | DLS_NODATA, 0, IDW_FILE);
	m_list.SetListStyle(style);
	m_list.EnableToolTips(FALSE);
	m_list.CreateHeaderCtrlEx(m_hWnd, IDW_HEADER, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | HDS_FULLDRAG | HDS_HORZ | HDS_BUTTONS);
	m_list.SetFocus();

	SetExtendedListStyle();

	// IMEを無効に
	ImmAssociateContext(m_list, NULL);
	return TRUE;
}

void CFileBrowseView::OnClose(BOOL& bHandled)
{
	if (isDragging()) {
		SetDropWaitingForDestroyWindow(true);
		bHandled = TRUE;
	}
}

void CFileBrowseView::SaveSettings()
{
	SetRegBinary(IDS_SEC_OPERATIONS, IDS_KEY_ICONLISTSTYLE, &m_ListStyle, sizeof(m_ListStyle));
	// カラム0の幅
	if (!(gnListStat & LTS_AUTOADJUST) && sciWidth[0] >= 0)
		SetRegBinary(IDS_SEC_SETTINGS, IDS_KEY_LISTCOLUMN, sciWidth, sizeof(sciWidth));
}

void CFileBrowseView::OnDestroy()
{
	ResetTimer(0);
	m_list.EnableToolTips(FALSE);

	m_WatchFolder.Stop();
	m_FileGrep.Stop();
	m_ColGrep.Stop();
	m_DirGrep.Stop();
	m_ImageGrep.Stop();
	m_IconGrep.Stop();

	m_list.ResetContent();
	m_stack.DeleteAll();

	m_idlFolder.Free();
	if (m_psf) {
		m_psf->Release();
		m_psf = NULL;
	}

	// CIdleHandlerの登録を解除する
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveIdleHandler(this);
}

int CFileBrowseView::OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message)
{
	LRESULT lr = DefWindowProc();
	if (lr != MA_NOACTIVATE && lr != MA_NOACTIVATEANDEAT) {		// frame does not want to activate
		if (nHitTest == HTCLIENT)
			fvApp.SetActiveView(m_hWnd);
	}
	return (int)lr;
}

void CFileBrowseView::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED) {
		CRect rc(0, 0, cx, cy);

		if (m_finder.IsWindow() && m_finder.IsWindowVisible()) {
			CRect rcw;
			m_finder.GetWindowRect(&rcw);
			rc.bottom -= rcw.Height() + 4;
			m_finder.MoveWindow(rc.left, rc.bottom + 2, rc.Width(), rcw.Height());
		}

		int width, realwidth, numcol, delta;
		DWORD dw = m_list.GetItemSize();
		if (m_ListStyle.isStyleOutline() ||
			m_ListStyle.isStyleImage() ||
			m_ListStyle.isStyleIcon()) {
			width = m_sdt[0].cxChar;
			realwidth = (cx - GetSystemMetrics(SM_CXVSCROLL));
			numcol = realwidth / width;
			delta = (numcol == 0) ? realwidth : realwidth / numcol;
			width = min(delta, width + 32);

			if ((int)LOWORD(dw) != width) {
				m_list.SetRedraw(FALSE);
				m_list.SetItemSize(width, (int)HIWORD(dw));
				m_list.SetRedraw(TRUE);
			}
		}
		m_list.Layout(&rc);
	}
}

BOOL CFileBrowseView::OnEraseBkgnd(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);
	CDCHandle dc = hdc;
	dc.FillRect(&rc, COLOR_3DFACE);
	return TRUE;
}

static void _RoundRect(Graphics& g, Brush& brush, CRect& rc, CSize size)
{
	// Create a GraphicsPath, and add an ellipse.
	GraphicsPath p;
	p.AddLine(rc.left + size.cx / 2, rc.top, rc.right - size.cx / 2, rc.top);
	p.AddArc(rc.right - size.cx, rc.top, size.cx, size.cy, 270, 90);
	p.AddLine(rc.right, rc.top + size.cy / 2, rc.right, rc.bottom - size.cy / 2);
	p.AddArc(rc.right - size.cx, rc.bottom - size.cy, size.cx, size.cy, 0, 90);
	p.AddLine(rc.right - size.cx / 2, rc.bottom, rc.left + size.cx / 2, rc.bottom);
	p.AddArc(rc.left, rc.bottom - size.cy, size.cx, size.cy, 90, 90);
	p.AddLine(rc.left, rc.bottom - size.cy / 2, rc.left, rc.top + size.cy / 2);
	p.AddArc(rc.left, rc.top, size.cx, size.cy, 180, 90);

	// Draw Path.
	g.FillPath(&brush, &p);
}

static __inline void _FillHot(CDCHandle& dc, const CRect& rc, COLORREF clrFrom, COLORREF clrTo, int per = 33)
{
	dc.FillSolidRect(&rc, mix_color(clrFrom, clrTo, per));
}

static void _FillSel(HDC hdc, const CRect& rc, COLORREF clrFrom, COLORREF clrTo)
{
	Graphics g(hdc);
	Rect r(rc.left, rc.top, rc.Width(), rc.Height());
	Color cF(255, GetRValue(clrFrom), GetGValue(clrFrom), GetBValue(clrFrom));
	Color cT(255, GetRValue(clrTo), GetGValue(clrTo), GetBValue(clrTo));
	LinearGradientBrush lbr(r, cF, cT, LinearGradientModeVertical);

	REAL fact[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	REAL pos[4] = { 0.0f, 0.25f, 0.5f, 0.75f };
	lbr.SetBlend(fact, pos, 4);
	g.FillRectangle(&lbr, r);
}

void CFileBrowseView::OnDrawItem(int /*nCtlID*/, LPDRAWITEMSTRUCT pdi)
{
	if ((int)pdi->itemID >= m_stack.GetSize() || !::RectVisible(pdi->hDC, &pdi->rcItem))
		return;

	CFileItem *pf = m_stack[pdi->itemID];
	if (!pf) return;

	pf->m_fNameCompacted = false;				// 省略表示されたか否かのフラグをOFF
												// ICON or IMAGEモードで利用する
	bool bFocused = (GetFocus() == m_list) || m_finder.hasFocus();
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bMarked = ((pdi->itemState & ODS_MARKED) != 0);
	CDCHandle dc = pdi->hDC;
	HFONT hfont = dc.SelectFont(m_font);
	int bkMode = dc.GetBkMode();

	COLORREF clrAttr = GetAttribColor(pf->m_fileattr, bSelected && bFocused, bMarked);
	COLORREF clrText, clrBack;
	if (bSelected && bFocused) {
		clrText = dc.SetTextColor(gColors[colSelect]);
		clrBack = dc.SetBkColor(clrAttr);
	} else if (bMarked) {
		clrText = dc.SetTextColor(gColors[colMark]);
		clrBack = dc.SetBkColor(clrAttr);
	} else {
		clrText = dc.SetTextColor(clrAttr);
		clrBack = dc.SetBkColor(gColors[colBack]);
	}

	if (pf->m_iImage == -1) GetItemIconImage(pf);

	if      (m_ListStyle.isStyleNormal())	DrawItemStyleList(dc, pdi);
	else if (m_ListStyle.isStyleOutline())	DrawItemStyleOutline(dc, pdi);
	else if (m_ListStyle.isStyleReport())	DrawItemStyleReport(dc, pdi);
	else if (m_ListStyle.isStyleIcon())		DrawItemStyleIcon(dc, pdi);
	else if (m_ListStyle.isStyleImage())	DrawItemStyleImage(dc, pdi);

	dc.SetBkMode(bkMode);
	dc.SetTextColor(clrText);
	dc.SetBkColor(clrBack);
	dc.SelectFont(hfont);
}

static COLORREF clrShadeGage_DeepBlue[4] = {
	RGB(164, 181, 205), RGB( 37,  81, 142),
	RGB( 13,  37,  90), RGB( 41,  75, 165),
};
static COLORREF clrShadeGage_Green[4] = {
	RGB(168, 220, 166), RGB( 50, 165,  44),
	RGB( 22, 114,  26), RGB( 48, 188,  58),
};
static COLORREF clrShadeGage_Aqua[4] = {
	RGB(226, 250, 255), RGB(143, 210, 244),
	RGB( 81, 163, 212), RGB(167, 223, 251),
};
static COLORREF clrShadeGage_MacAqua[4] = {
	RGB(208, 224, 244), RGB(135, 184, 234),
	RGB(106, 170, 235), RGB(184, 250, 255),
};
static COLORREF clrShadeGage_Emerald[4] = {
	RGB(200, 244, 188), RGB( 82, 206,  48),
	RGB( 50, 118,  40), RGB( 64, 216,  22),
};
static COLORREF clrShadeGage_Gray[4] = {
	RGB(251, 254, 254), RGB(224, 225, 225),
	RGB(184, 191, 200), RGB(231, 234, 236),
};

static Color _RGB2RGBA(COLORREF clr, int alpha)
{
	return Color((BYTE)alpha, GetRValue(clr), GetGValue(clr), GetBValue(clr));
}

HBITMAP CFileBrowseView::GetBarMeterBitmap()
{
	if (m_bmBarMeter.IsNull())
		m_bmBarMeter = lccLoadImage(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_BARMETER));
	return m_bmBarMeter;
}

void CFileBrowseView::DrawDriveUsage(HDC hdc, LPRECT prc, DWORDLONG sfree, DWORDLONG stotal, bool bShading = false)
{
	CRect rc = *prc;
	CDCHandle dc = hdc;

	COLORREF clrBack = dc.GetBkColor();
	dc.FillSolidRect(&rc, GetSysColor(COLOR_3DSHADOW));
	rc.DeflateRect(1, 1);
	dc.FillSolidRect(&rc, GetSysColor(COLOR_WINDOW));

	int per = 100 - (int)(sfree * 100 / stotal);
	rc.right = rc.left + rc.Width() * per / 100;
	if (bShading) {
		Graphics g(hdc);
		Bitmap bmp(GetBarMeterBitmap(), (HPALETTE)NULL);
		g.DrawImage(&bmp, rc.left, rc.top, rc.Width(), rc.Height());
	} else {
		rc.DeflateRect(1, 1);
		dc.FillSolidRect(&rc, RGB(0, 0xac, 0xed));
	}
	dc.SetBkColor(clrBack);
}

void CFileBrowseView::DrawItemStyleList(CDCHandle& dc, LPDRAWITEMSTRUCT pdi)
{
	bool bFocused = (GetFocus() == m_list) || m_finder.hasFocus();
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bHot = ((pdi->itemState & ODS_HOTLIGHT) != 0);
	bool bMarked = ((pdi->itemState & ODS_MARKED) != 0);
	CFileItem *pf = m_stack[pdi->itemID];
	CRect rc(&pdi->rcItem);

	IImageList* pil;
	CSize sizeIcon;
	if (m_ListStyle.isEnlargeNormal()) {
		pil = m_pilLarge;
		sizeIcon = m_sizeLarge;
	} else {
		pil = m_pilSmall;
		sizeIcon = m_sizeSmall;
	}

	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		// フォーカスを持ってなく且つ選択状態の場合
		if (!bFocused && bSelected)
			dc.SetBkColor(GetSysColor(COLOR_BTNFACE));

		if (bHot) {
			dc.SetBkMode(TRANSPARENT);
			COLORREF clr = (bSelected ? gColors[colSelect] : (bMarked ? gColors[colMark] : dc.GetBkColor()));
			_FillHot(dc, rc, GetAttribColor(pf->m_fileattr, true, bMarked), clr, (bSelected || bMarked) ? 66 : 33);
			if (!bSelected)
				dc.SetTextColor(GetAttribColor(pf->m_fileattr, false));
		} else {
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
		}
		CRect rcw = rc;
		rcw.right = rcw.left + m_sdt[0].cxChar;

		// アイコン
		CPoint ptIcon(rcw.left + 2, (rcw.top + rcw.bottom - sizeIcon.cy) >> 1);
		IImageList_DrawIcon(pil, dc, pf->m_iImage, ptIcon, sizeIcon, ILD_TRANSPARENT | pf->OverlayImageMask());

		rcw.left += sizeIcon.cx + 2;
		rcw.InflateRect(-4, 0);
		rcw.left -= 2;

		// 名前
		CString str = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			str.MakeLower();

		DRAWTEXTPARAMS dtp;
		memset(&dtp, 0, sizeof(dtp));
		dtp.cbSize = sizeof(dtp);
		int outlen = 0;
		int len = str.GetLength();
		if (!pf->isFolder() && gnListStat & LTS_ALIGNEXT) {
			DWORD flags = (m_ListStyle.isShowEllp() ? DTEX_SINGLE : DTEX_SINGLE_NE);
			int pos = str.ReverseFind(_T('.'));
			if (pos < 0) {
				dc.DrawTextEx((LPTSTR)str.GetString(), len, &rcw, flags | DT_LEFT, &dtp);
				outlen = dtp.uiLengthDrawn;
			} else {
				LPTSTR p = (LPTSTR)str.GetString();
				if (pos > 0) {
					long right = rcw.right;
					rcw.right = rcw.left + m_nMaxName;
					dc.DrawTextEx((LPTSTR)p, pos, &rcw, flags | DT_LEFT, &dtp);
					outlen = dtp.uiLengthDrawn;
					rcw.right = right;
					rcw.left += m_nMaxName;
				}
				dc.DrawTextEx(p + pos, len - pos, &rcw, flags | DT_LEFT, &dtp);
				outlen += dtp.uiLengthDrawn;
			}
		}
		else {
			dc.DrawTextEx((LPTSTR)str.GetString(), len, &rcw, DTEX_SINGLE | DT_LEFT, &dtp);
			outlen = dtp.uiLengthDrawn;
		}
		pf->m_fNameCompacted = (outlen < len);

		rcw.InflateRect(4, 0);
		if (m_rawfiles > 0 && pf->isRawFile()) {
			// サイズ
			if (m_ListStyle.isShowSize()) {
				rcw.left = rcw.right;
				rcw.right += m_sdt[1].cxChar;
				rcw.InflateRect(-4, 0);
				dc.DrawText(pf->m_column[1], -1, &rcw, DTEX_SINGLE | DT_RIGHT);
				rcw.InflateRect(4, 0);
			}
			// 日付
			if (m_ListStyle.isShowDate()) {
				rcw.left = rcw.right;
				rcw.right += m_sdt[2].cxChar;
				rcw.InflateRect(-4, 0);
				dc.DrawText(pf->m_column[2], -1, &rcw, DTEX_SINGLE);
				rcw.InflateRect(4, 0);
			}
			// 属性
			if (m_ListStyle.isShowAttr()) {
				rcw.left = rcw.right;
				rcw.right += m_sdt[4].cxChar;
				rcw.InflateRect(-4, 0);
				dc.DrawText(pf->m_column[4], -1, &rcw, DTEX_SINGLE);
				rcw.InflateRect(4, 0);
			}
		}
		else if (pf->isDrive()) {
			if (m_ListStyle.isShowSize()) {
				// 使用量を右寄せで
				rcw.left = rcw.right;
				rcw.right += m_sdt[1].cxChar;
				rcw.InflateRect(-4, 0);
				if (pf->GetDriveUsage(str))
					dc.DrawText(str, -1, &rcw, DTEX_SINGLE | DT_RIGHT);
				rcw.InflateRect(4, 0);
			}
			if (m_ListStyle.isShowAttr()) {
				if (pf->m_totalsize) {
					rcw.left = rcw.right;
					rcw.right += m_sdt[4].cxChar;
					rcw.DeflateRect(4, 1);

					DrawDriveUsage(dc, &rcw, pf->m_filesize, pf->m_totalsize);

					rcw.InflateRect(4, 1);
				}
			}
		}
		else {
			if (m_ListStyle.isShowSize() && pf->m_filesize > 0) {
				// サイズ
				TCHAR temp[64];
				StrFormatByteSize64(pf->m_filesize, temp, ARRAYSIZE(temp));

				rcw.left = rcw.right;
				rcw.right += m_sdt[1].cxChar;
				rcw.InflateRect(-4, 0);
				dc.DrawText(temp, -1, &rcw, DTEX_SINGLE | DT_RIGHT);
				rcw.InflateRect(4, 0);
			}
			if (m_ListStyle.isShowDate() && pf->hasLastWriteTime()) {
				// 日付
				FormatFileTime(&pf->m_ftLastWrite, str, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);

				rcw.left = rcw.right;
				rcw.right += m_sdt[2].cxChar;
				rcw.InflateRect(-4, 0);
				dc.DrawText(str, -1, &rcw, DTEX_SINGLE);
				rcw.InflateRect(4, 0);
			}
		}
	}
	if (bFocused && pdi->itemState & ODS_FOCUS)
		dc.DrawFocusRect(&pdi->rcItem);
}

void CFileBrowseView::DrawItemStyleOutline(CDCHandle& dc, LPDRAWITEMSTRUCT pdi)
{
	CFileItem *pf = m_stack[pdi->itemID];
	bool bFocused = (GetFocus() == m_list) || m_finder.hasFocus();
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bHot = ((pdi->itemState & ODS_HOTLIGHT) != 0);
	bool bMarked = ((pdi->itemState & ODS_MARKED) != 0);
	CRect rcItem(&pdi->rcItem);
	rcItem.DeflateRect(1, 1);

	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		// フォーカスを持ってなく且つ選択状態の場合
		if (!bFocused && bSelected)
			dc.SetBkColor(GetSysColor(COLOR_BTNFACE));

		// シェーディング
		if (bHot) {
			dc.SetBkMode(TRANSPARENT);
			COLORREF clr = (bSelected ? gColors[colSelect] : (bMarked ? gColors[colMark] : dc.GetBkColor()));
			_FillHot(dc, rcItem, GetAttribColor(pf->m_fileattr, true, bMarked), clr, (bSelected || bMarked) ? 66 : 33);
			if (!bSelected)
				dc.SetTextColor(GetAttribColor(pf->m_fileattr, false));
		} 
		else {
/* @@@ --- 市松模様
			if ((gnListStat & LTS_REPCHECKER) && (pdi->itemState & ODS_ODDPOSITION)) {
				COLORREF cbt = dc.GetBkColor();
				cbt = PixelAlpha(cbt, 94);
				dc.SetBkColor(cbt);
			}
--- @@@ */
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL);
		}

		// アイコン描画
		CRect rcc = rcItem;
		rcc.DeflateRect(4, STYLE_OTL_GAP_Y);

		IImageList* pil;
		CSize sizeIcon;
		if (m_ListStyle.isEnlargeOutline()) {
			pil = m_pilExtra;
			sizeIcon = m_sizeExtra;
		}
		else {
			pil = m_pilLarge;
			sizeIcon = m_sizeLarge;
		}
		IImageList_DrawIcon(pil, dc, pf->m_iImage, rcc.TopLeft(), CSize(0, 0), ILD_TRANSPARENT | pf->OverlayImageMask());

		rcc.left += sizeIcon.cx + 4;	// rccはテキスト部描画領域全体を指す
		CRect rcl = rcc;	// rclは行ごとの描画用
		rcl.bottom = rcl.top + m_sizeChar.cy;

		// 名前
		CString str = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			str.MakeLower();

		// ファイル名はBoldフォント
		HFONT hfont = (gnListStat & LTS_BOLDNAME) ? dc.SelectFont(makeBoldOrUnderlineFont(dc)) : NULL;

		int len = str.GetLength();
		DRAWTEXTPARAMS dtp;
		memset(&dtp, 0, sizeof(dtp));
		dtp.cbSize = sizeof(dtp);
		dc.DrawTextEx(str.GetBuffer(), len, pf->isUpdir() ? &rcc : &rcl, DTEX_SINGLE | DT_VCENTER, &dtp);
		pf->m_fNameCompacted = (len != (int)dtp.uiLengthDrawn);

		::DeleteObject(dc.SelectFont(hfont));

		rcl.OffsetRect(0, m_sizeChar.cy + 2);

		if (pf->isRawFile()) {
			if (!pf->isFolder()) {
				// 種別
				dc.DrawText(pf->m_column[3], -1, &rcl, DTEX_SINGLE | DT_VCENTER);
				rcl.OffsetRect(0, m_sizeChar.cy);
			}
			// 日付
//			dc.DrawText(pf->m_column[2], -1, &rcl, DTEX_SINGLE | DT_VCENTER);
			CString s;
			if (*(DWORDLONG*)&pf->m_ftLastWrite) {
				FormatFileTime(&pf->m_ftLastWrite, s, 0, true);
			}
			else {
				s = pf->m_column[2];
			}
			dc.DrawText(s, -1, &rcl, DTEX_SINGLE | DT_VCENTER);
			rcl.OffsetRect(0, m_sizeChar.cy);
			// サイズ
			if (!pf->isFolder() || pf->m_filesize > 0) {
				TCHAR temp[64];
				StrFormatByteSize64(pf->m_filesize, temp, ARRAYSIZE(temp));
				dc.DrawText(temp, -1, &rcl, DTEX_SINGLE | DT_VCENTER);
			}
			rcl.OffsetRect(0, m_sizeChar.cy);
		}
		else if (pf->isDrive()) {
			if (pf->m_totalsize) {
				COLORREF clrBack = dc.GetBkColor();
				CRect rcm = rcl;
				rcm.right -= 8;
				rcm.bottom -= 1;
				DrawDriveUsage(dc, &rcm, pf->m_filesize, pf->m_totalsize);
				rcl.OffsetRect(0, m_sizeChar.cy);
				dc.SetBkColor(clrBack);
				if (pf->GetDriveUsage(str)) {
					dc.DrawText(str, -1, &rcl, DTEX_SINGLE | DT_VCENTER);
					rcl.OffsetRect(0, m_sizeChar.cy);
				}
			}
		}
		else {
			int last = min(4, m_numColumns);
			for (int i = 1; i < last; i++) {
				if (!pf->m_column[i].IsEmpty()) {
					dc.DrawText(pf->m_column[i], -1, &rcl, DTEX_SINGLE | DT_VCENTER);
					rcl.OffsetRect(0, m_sizeChar.cy);
				}
			}
		}
	}
	if (bFocused && pdi->itemState & ODS_FOCUS)
		dc.DrawFocusRect(&rcItem);
}

void CFileBrowseView::DrawItemStyleReport(CDCHandle& dc, LPDRAWITEMSTRUCT pdi)
{
	IImageList* pil;
	CSize sizeIcon;
	if (m_ListStyle.isEnlargeReport()) {
		pil = m_pilLarge;
		sizeIcon = m_sizeLarge;
	}
	else {
		pil = m_pilSmall;
		sizeIcon = m_sizeSmall;
	}

	bool bFocused = (GetFocus() == m_list) || m_finder.hasFocus();
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bHot = ((pdi->itemState & ODS_HOTLIGHT) != 0);
	bool bMarked = ((pdi->itemState & ODS_MARKED) != 0);
	CFileItem *pf = m_stack[pdi->itemID];
	CRect rcItem(&pdi->rcItem);

	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		// アイテム部背景
		if (bSelected && !bFocused)
			dc.SetBkColor(GetSysColor(COLOR_BTNFACE));

		// シェーディング
		COLORREF clrBack = dc.GetBkColor();
		COLORREF clrCheck = clrBack;
		dc.SetBkMode(TRANSPARENT);
		if (bHot) {
			COLORREF clr = (bSelected ? gColors[colSelect] : (bMarked ? gColors[colMark] : dc.GetBkColor()));
			_FillHot(dc, rcItem, GetAttribColor(pf->m_fileattr, true, bMarked), clr, (bSelected || bMarked) ? 66 : 33);
			if (!bSelected)
				dc.SetTextColor(GetAttribColor(pf->m_fileattr, false));
		}
		else {
			if ((gnListStat & LTS_REPCHECKER) && (pdi->itemState & ODS_ODDPOSITION)) {
				clrCheck = PixelAlpha(clrCheck, 94);
				dc.SetBkColor(clrCheck);
			}
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL);
			dc.SetBkColor(clrBack);
		}

		// ソート対象のカラムを別色で描画する
		clrCheck = PixelAlpha(clrCheck, (clrCheck == clrBack) ? 94 : 98);
		BOOL bCheckPattern = (	!m_ListStyle.isSortNone() &&
								!bSelected &&
								!bHot &&
								!bMarked	);

		// カラム
		CRect rcw = rcItem;
		rcw.right = rcw.left + m_sdt[0].cxChar;
		if (bCheckPattern && m_sortColumn == 0)
			dc.FillSolidRect(&rcw, clrCheck);

		// アイコン
		CPoint ptIcon(rcItem.left + 2, (rcItem.top + rcItem.bottom - sizeIcon.cy) / 2);
		IImageList_DrawIcon(pil, dc, pf->m_iImage, ptIcon, sizeIcon, ILD_TRANSPARENT | pf->OverlayImageMask());

		rcw.left += sizeIcon.cx;
		rcw.InflateRect(-4, 0);

		CString str = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			str.MakeLower();
		int len = str.GetLength();

		// 名前
		int outlen = 0;
		DRAWTEXTPARAMS dtp;
		memset(&dtp, 0, sizeof(dtp));
		dtp.cbSize = sizeof(dtp);
		if (isFileSystemFolder() &&
			pf->isFSFile() &&
			gnListStat & LTS_ALIGNEXT)
		{
			// 拡張子をそろえる
			int pos = str.ReverseFind(_T('.'));
			if (pos < 0) {
				dc.DrawTextEx((LPTSTR)(LPCTSTR)str, len, &rcw, DTEX_SINGLE | DT_LEFT, &dtp);
				outlen = (int)dtp.uiLengthDrawn;
			}
			else {
				LPCTSTR p = str.GetString();
				if (pos > 0) {
					CRect rcx = rcw;
					rcx.right -= m_nMaxExt;
					dc.DrawTextEx((LPTSTR)p, pos, &rcx, DTEX_SINGLE | DT_LEFT, &dtp);
					outlen = (int)dtp.uiLengthDrawn;
					rcw.left = rcx.right;
				}
				dc.DrawText(p + pos, len - pos, &rcw, DTEX_SINGLE | DT_LEFT);
				outlen += len - pos;
			}
		}
		else {
			dc.DrawTextEx((LPTSTR)(LPCTSTR)str, len, &rcw, DTEX_SINGLE | DT_LEFT, &dtp);
			outlen = (int)dtp.uiLengthDrawn;
		}

		pf->m_fNameCompacted = (outlen < len);

		rcw.InflateRect(4, 0);
		for (int col = 1; col < m_numColumns; col++) {
			rcw.left = rcw.right;
			rcw.right += m_sdt[col].cxChar;

			if (!bHot && bCheckPattern && m_sortColumn == col)
				dc.FillSolidRect(&rcw, clrCheck);

			rcw.InflateRect(-4, 0);
			if (!pf->m_column[col].IsEmpty()) {
				UINT fmt = DTEX_SINGLE;
				if (m_sdt[col].fmt == LVCFMT_RIGHT)
					fmt |= DT_RIGHT;
				else if (m_sdt[col].fmt == LVCFMT_CENTER)
					fmt |= DT_CENTER;
				dc.DrawText(pf->m_column[col], -1, &rcw, fmt);
			}
			rcw.InflateRect(4, 0);
		}
	}

	if (bFocused && pdi->itemState & ODS_FOCUS)
		dc.DrawFocusRect(&rcItem);
}

void CFileBrowseView::DrawItemStyleIcon(CDCHandle& dc, LPDRAWITEMSTRUCT pdi)
{
	CSize sizeIcon = m_ListStyle.isEnlargeIcon() ? m_sizeExtra : m_sizeLarge;
	CFileItem *pf = m_stack[pdi->itemID];
	bool bFocused = (GetFocus() == m_list) || m_finder.hasFocus();
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bHot = ((pdi->itemState & ODS_HOTLIGHT) != 0);
	bool bMarked = ((pdi->itemState & ODS_MARKED) != 0);
	CRect rcItem(&pdi->rcItem);
	rcItem.DeflateRect(1, 1);

	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		// フォーカスを持ってなく且つ選択状態の場合
		if (!bFocused && bSelected)
			dc.SetBkColor(GetSysColor(COLOR_BTNFACE));

		if (bHot) {
			dc.SetBkMode(TRANSPARENT);
			COLORREF clr = (bSelected ? gColors[colSelect] : (bMarked ? gColors[colMark] : dc.GetBkColor()));
			_FillHot(dc, rcItem, GetAttribColor(pf->m_fileattr, true, bMarked), clr, (bSelected || bMarked) ? 66 : 33);
			if (!bSelected)
				dc.SetTextColor(GetAttribColor(pf->m_fileattr, false));
		} else {
/* @@@ --- 市松模様
			if ((gnListStat & LTS_REPCHECKER) && (pdi->itemState & ODS_ODDPOSITION)) {
				COLORREF cbt = dc.GetBkColor();
				cbt = PixelAlpha(cbt, 94);
				dc.SetBkColor(cbt);
			}
--- @@@ */
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL);
		}

		CRect rcw = rcItem;
		rcw.DeflateRect(3, 3);
		rcw.bottom = rcw.top + sizeIcon.cy;

		// ドライブディスク使用量グラフ
		if (pf->isDrive() && pf->m_totalsize && m_ListStyle.isShowAttr()) {
			// サイズ（空き、トータル）
			CRect rcm = rcw;
			rcm.DeflateRect(8, 0);
			rcm.top = rcm.bottom - 10;
			rcm.bottom += 2;
			DrawDriveUsage(dc, &rcm, pf->m_filesize, pf->m_totalsize);
		}

		CPoint pt;
		pt.x = rcw.left + (rcw.Width() - sizeIcon.cx) / 2;
		pt.y = rcw.top;
		IImageList* pil = m_ListStyle.isEnlargeIcon() ? m_pilExtra : m_pilLarge;
		IImageList_DrawIcon(pil, dc, pf->m_iImage, pt, CSize(0, 0), ILD_TRANSPARENT | pf->OverlayImageMask());

		rcw.InflateRect(3, 3);
		rcw.top = rcw.bottom;
		rcw.bottom = rcItem.bottom;
		rcw.DeflateRect(2, 2);

		CString str = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			str.MakeLower();

		int len = str.GetLength();
		DRAWTEXTPARAMS dtp;
		memset(&dtp, 0, sizeof(dtp));
		dtp.cbSize = sizeof(dtp);
		int height = dc.DrawTextEx((LPTSTR)str.GetString(), len, &rcw, DT_WORDBREAK | DT_NOPREFIX | DT_WORD_ELLIPSIS | DT_CENTER, &dtp);
		pf->m_fNameCompacted = (len != (int)dtp.uiLengthDrawn);

		rcw.top += height;
		rcw.bottom = rcw.top + m_sizeChar.cy;
		if (m_ListStyle.isShowDate() &&  pf->hasLastWriteTime()) {
			// 日付 分まで
			FormatFileTime(&pf->m_ftLastWrite, str, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
			dc.DrawText(str, lstrlen(str) - 3, &rcw, DTEX_SINGLE | DT_CENTER);
			rcw.OffsetRect(0, m_sizeChar.cy);
		}
		if (m_ListStyle.isShowSize()) {
			if (pf->m_filesize > 0) {
				// サイズ
				TCHAR temp[64];
				StrFormatByteSize64(pf->m_filesize, temp, ARRAYSIZE(temp));
				dc.DrawText(temp, -1, &rcw, DT_CENTER | DTEX_SINGLE);
				rcw.OffsetRect(0, m_sizeChar.cy);
			}
			else if (pf->isDrive() && pf->GetDriveUsage(str)) {
				dc.DrawText(str, -1, &rcw, DTEX_SINGLE | DT_CENTER);
				rcw.OffsetRect(0, m_sizeChar.cy);
			}
		}

		if (pf->isRawFile()) {
			if (m_ListStyle.isShowAttr() && pf->m_fileattr) {
				// 属性
				CWin32FindData wfd;
				wfd.dwFileAttributes = pf->m_fileattr;
				wfd.FormatFileAttributes(str);
				dc.DrawText(str, -1, &rcw, DTEX_SINGLE | DT_CENTER);
			}
		}
	}
	if (bFocused && pdi->itemState & ODS_FOCUS)
		dc.DrawFocusRect(&rcItem);
}

extern "C" int WINAPI imgSpecifyImageType(LPCTSTR lpName);

void CFileBrowseView::DrawItemStyleImage(CDCHandle& dc, LPDRAWITEMSTRUCT pdi)
{
	CFileItem* pf = m_stack[pdi->itemID];
	bool bFocused = (GetFocus() == m_list) || m_finder.hasFocus();
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bHot = ((pdi->itemState & ODS_HOTLIGHT) != 0);
	bool bMarked = ((pdi->itemState & ODS_MARKED) != 0);
	bool bExif = ((gnFileStat & FTS_EXIF) && pf->isEXIF());
	CRect rc(&pdi->rcItem);
	// margin分を縮小
	rc.DeflateRect(1, 1);

	Graphics g(dc);
	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		// フォーカスを持ってなく且つ選択状態の場合
		if (!bFocused && bSelected)
			dc.SetBkColor(GetSysColor(COLOR_BTNFACE));

		if (bHot) {
			dc.SetBkMode(TRANSPARENT);
			COLORREF clr = (bSelected ? gColors[colSelect] : (bMarked ? gColors[colMark] : dc.GetBkColor()));
			_FillHot(dc, rc, GetAttribColor(pf->m_fileattr, true, bMarked), clr, (bSelected || bMarked) ? 66 : 33);
			if (!bSelected)
				dc.SetTextColor(GetAttribColor(pf->m_fileattr, false));
		} else {
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
		}
		// イメージ部の矩形 image-heght + padding 3px + shadow 5px
		CRect rci = rc;
		rci.bottom = rci.top + m_ListStyle.size_thumb.cy + 6 + m_dropshadow.delta();
		// padding分を縮小
		rci.DeflateRect(3, 3);

		// アイコンまたは縮小イメージ描画
		CRect rcw;
		if (pf->m_bmp) {
			int shadow = 0;
			switch (pf->GetPerceivedType()) {
				case PERCEIVED_TYPE_IMAGE:
				case PERCEIVED_TYPE_VIDEO:
					shadow = m_dropshadow.delta();
					break;
			}

			// 縮小イメージ
			uint32_t cx = pf->m_bmp->GetWidth();
			uint32_t cy = pf->m_bmp->GetHeight();
			float scale = CalcScaleRatio(cx, cy, m_ListStyle.size_thumb.cx, m_ListStyle.size_thumb.cy);
			if (scale < 1.0f) {
				cx = (uint32_t)floor(cx * scale);
				cy = (uint32_t)floor(cy * scale);
			}

			Graphics gw((HDC)dc);
			gw.SetInterpolationMode(InterpolationModeHighQualityBicubic/*InterpolationModeDefault*/);
			gw.SetPixelOffsetMode(PixelOffsetModeHalf);

			rcw = rci;
			rcw.left += (rci.Width() - cx - shadow) / 2;
			rcw.right = rcw.left + cx;
			rcw.top = rci.bottom - cy - shadow;
			rcw.bottom = rcw.top + cy;
			gw.DrawImage(pf->m_bmp, rcw.left, rcw.top, cx, cy);

			// ドロップシャドウ適用
			if (shadow) {
				rcw.right += shadow;
				rcw.bottom += shadow;
				m_dropshadow.Apply(gw, &rcw);
			}
		} else {
			// アイコン
			CSize size = m_ListStyle.size_thumb;
			CPoint pt((rci.left + rci.right - size.cx) / 2, (rci.top + rci.bottom - size.cy) / 2);
			IImageList_DrawIcon(SHIL_JUMBO, dc, pf->m_iImage, pt, size, ILD_TRANSPARENT | ILD_SCALE);
			// オーバーレイアイコン
			if (pf->m_iOverlayImage > 0) {
				int iImage = Shell_GetCachedImageIndex(fvApp.GetMyModuleName(), -IDI_EMPTY, FALSE);
				pt.Offset(0, size.cy - m_sizeExtra.cy);
				IImageList_DrawIcon(m_pilExtra, dc, iImage, pt, CSize(0, 0), ILD_TRANSPARENT | pf->OverlayImageMask());
			}
		}

		// 使用率など
		if (pf->isDrive() && pf->m_totalsize && m_ListStyle.isShowAttr()) {
			rcw = rci;
			rcw.left += rci.Width() * 2 / 5;
			rcw.bottom -= rci.Height() / 2;
			rcw.DeflateRect(2, 2);

			RectF r((REAL)rcw.left, (REAL)rcw.top, (REAL)rcw.Width(), (REAL)rcw.Height());

			g.SetSmoothingMode(SmoothingModeHighQuality);
			// Create a SolidBrush object.
			SolidBrush brUnused(Color(240, 32, 32, 32));
			SolidBrush brUsed(Color(240, 0, 128, 192));
			REAL per = (REAL)(int)(360 - pf->m_filesize * 360 / pf->m_totalsize);
			g.FillPie(&brUsed, r, -90, per);
			g.FillPie(&brUnused, r, -90, per - 360);

			LOGFONT lf;
			m_font.GetLogFont(&lf);
			Font myFont(_T("Segoe UI"), (REAL)m_sizeChar.cy * 0.8F, FontStyleBold, UnitPixel);
			StringFormat fmt;
			fmt.SetAlignment(StringAlignmentCenter);					// DT_CENTER
			fmt.SetFormatFlags(StringFormatFlagsNoWrap);				// DT_SINGLELINE
			fmt.SetLineAlignment(StringAlignmentCenter);				// DT_VCENTER
			SolidBrush brFont(Color(255, 255, 255, 255));
			g.SetTextRenderingHint(TextRenderingHintAntiAlias);
			CString str;
			str.Format(_T("%d%%"), (int)(100 - pf->m_filesize * 100 / pf->m_totalsize));
			g.DrawString(str, str.GetLength(), &myFont, r, &fmt, &brFont);
		}

		// padding分を元に戻す
		rci.InflateRect(3, 3);

		// ラベル描画
		CRect rct = rc;
		rct.top = rci.bottom;
		// テキスト部の上下マージンを縮小
		rct.InflateRect(-2, 0);
		rct.bottom = rct.top + m_sizeChar.cy * 2;

		// オブジェクト名
		CString str = pf->m_column[0];
		if (gnListStat & LTS_LOWERCASE)
			str.MakeLower();

		DRAWTEXTPARAMS dtp;
		memset(&dtp, 0, sizeof(dtp));
		dtp.cbSize = sizeof(dtp);

		HFONT hfont = (gnListStat & LTS_BOLDNAME) ? dc.SelectFont(makeBoldOrUnderlineFont(dc)) : NULL;
		int len = str.GetLength();
		int height = dc.DrawTextEx((LPTSTR)str.GetString(), len, &rct, DT_WORDBREAK | DT_NOPREFIX | DT_WORD_ELLIPSIS | DT_CENTER, &dtp);
		rct.top += min(height, m_sizeChar.cy * 2);
		rct.bottom = rct.top + m_sizeChar.cy;
		::DeleteObject(dc.SelectFont(hfont));

		pf->m_fNameCompacted = (len != (int)dtp.uiLengthDrawn);

		if (bExif) {
			// EXIF情報を表示
			// シャッター速度、絞り
			str.Empty();
			TCHAR buf[MAX_PATH];
			// 露出時間
			pf->m_exifptr->FormatExposureTime(buf, true);
			if (buf[0]) {
				str += buf;
				str += _T(' ');
			}
			// F値
			pf->m_exifptr->FormatAperture(buf, true);
			if (buf[0]) {
				str += buf;
				str += _T(' ');
			}
			// 露出補正
			pf->m_exifptr->FormatExposureBias(buf);
			if (buf[0]) {
				str += buf;
				if (pf->m_exifptr->IsFlashFired()) {
					str += _T(" \xb6");
				}
			}
			// 焦点距離
			pf->m_exifptr->FormatFocalLength35(buf);
			if (buf[0]) {
				str += buf;
				str += _T(' ');
			}

			// GDI+で文字列出力
			LOGFONT lf;
			m_font.GetLogFont(&lf);

			Font myFont(lf.lfFaceName, (Gdiplus::REAL)(fabs((float)lf.lfHeight) * 0.9F), FontStyleRegular, UnitPixel);
			RectF r((Gdiplus::REAL)rct.left, (Gdiplus::REAL)rct.top, (Gdiplus::REAL)rct.Width(), (Gdiplus::REAL)rct.Height());
			StringFormat fmt;
			fmt.SetAlignment(StringAlignmentCenter);					// DT_CENTER
			fmt.SetFormatFlags(StringFormatFlagsNoWrap);				// DT_SINGLELINE
			fmt.SetLineAlignment(StringAlignmentCenter);				// DT_VCENTER
			fmt.SetTrimming(StringTrimmingEllipsisCharacter);			// followed by "..."
			COLORREF clr = dc.GetTextColor();
			SolidBrush sbr(Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
			g.SetTextRenderingHint(TextRenderingHintAntiAlias);
			g.DrawString(str, str.GetLength(), &myFont, r, &fmt, &sbr);

			if (m_ListStyle.isShowDate()) {
				// 撮影日時
				r.Offset(0, (Gdiplus::REAL)m_sizeChar.cy);
				pf->m_exifptr->FormatDigitizedTime(buf, ARRAYSIZE(buf));
				g.DrawString(buf, lstrlen(buf), &myFont, r, &fmt, &sbr);
			}
			if (m_ListStyle.isShowSize()) {
				// サイズ
				r.Offset(0, (Gdiplus::REAL)m_sizeChar.cy);
				pf->m_exifptr->FormatImageSize(buf);
				g.DrawString(buf, lstrlen(buf), &myFont, r, &fmt, &sbr);
			}
		} else {
			if (m_ListStyle.isShowDate() &&  pf->hasLastWriteTime()) {
				// 日付 分まで
				FormatFileTime(&pf->m_ftLastWrite, str, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
				dc.DrawText(str, lstrlen(str) - 3, &rct, DTEX_SINGLE | DT_CENTER);
				rct.OffsetRect(0, m_sizeChar.cy);
			}
			if (m_ListStyle.isShowSize()) {
				// サイズ
				if (pf->isDrive()) {
					// ドライブ
					if (pf->GetDriveUsage(str)) {
						dc.DrawText(str, -1, &rct, DT_CENTER | DTEX_SINGLE);
						rct.OffsetRect(0, m_sizeChar.cy);
					}
				} else if (pf->m_filesize > 0) {
					TCHAR temp[64];
					StrFormatByteSize64(pf->m_filesize, temp, ARRAYSIZE(temp));
					dc.DrawText(temp, -1, &rct, DT_CENTER | DTEX_SINGLE);
					rct.OffsetRect(0, m_sizeChar.cy);
				}
			}
		}
	}

	if (bFocused && pdi->itemState & ODS_FOCUS)
		dc.DrawFocusRect(&rc);
}

void CFileBrowseView::OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = FALSE;
	NMDLCLICK* pcl = (NMDLCLICK*)pNMHDR;
	if (pcl->iItem >= 0) {
		if ((pcl->typeClick == MK_RBUTTON && (gnMosStat & MOS_RBTN_MASK) == MOS_RBTN_SINGLE_SEL) ||
			(pcl->typeClick == MK_MBUTTON && (gnMosStat & MOS_CBTN_MASK) == MOS_CBTN_SINGLE_SEL)) {
			int index = (int)pcl->iItem;
			BOOL bSel = m_list.GetSel(index);
			m_list.SetSel(index, !bSel);
			m_list.SetCaretIndex(index);
			m_list.SetAnchorIndex(index);
			*pResult = TRUE;
		}
		else if ((pcl->typeClick == MK_MBUTTON && (gnMosStat & MOS_CBTN_MASK) == MOS_CBTN_SHORTCUTMENU)) {
			CPoint pt = pcl->ptAction;
			m_list.ClientToScreen(&pt);
			gpMainFrame->TrackMenuForFileBrowser(pt);
			*pResult = TRUE;
		}
	}
}

void CFileBrowseView::OnNotifyGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMDLGETINFOTIP* pgit = (NMDLGETINFOTIP*)pNMHDR;
	if (pgit->iItem >= 0 && pgit->iItem < m_list.GetCount()) {
		CFileItem* pf = m_stack[pgit->iItem];
		CString str;
		if (pf->m_fNameCompacted)
			str = pf->m_column[0];

		if (pf->m_infotip.IsEmpty()) {
			if (pf->isFSFile() && fvApp.IsFileOmitInfoTips(pf->m_column[0])) {
				// 例えば .avi のように取得に時間がかかるものが指定されている場合は
				// チップ情報を省略
				pf->m_infotip.Format(_T("%s: %s\n%s: %s\n%s: %s"),
						m_stack.m_colinfo[3].title, (LPCTSTR)pf->m_column[3],	// 種類
						m_stack.m_colinfo[2].title, (LPCTSTR)pf->m_column[2],	// 更新日時
						m_stack.m_colinfo[1].title, (LPCTSTR)pf->m_column[1]);	// サイズ
			} else {
				CString tips;
				if (idl_GetInfoTip(m_hWnd, m_psf, pf->m_pidl, tips) > 0)
					pf->m_infotip = tips;

				if (pf->m_fileattr & FILE_ATTRIBUTE_REPARSE_POINT) {
					// ジャンクション
					pf->GetPathName(m_psf, tips, NULL);
					CMemTask target;
					DWORD type;
					DWORD res = fopGetReparsePointInfo(tips, (LPTSTR*)&target, &type);
					if (res == NOERROR) {
						if (!pf->m_infotip.IsEmpty()) pf->m_infotip += _T('\n');
						pf->m_infotip.AppendFormat(IDS_JUNCTION_TIP, (PWSTR)target);
					}
				}
			}
		}
		if (!str.IsEmpty()) str += _T('\n');
		str += pf->m_infotip;

		if (!str.IsEmpty()) {
			lstrcpyn(pgit->pszText, str, pgit->cchTextMax - 1);
		}
	}
}

void CFileBrowseView::OnNotifyBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	// ドラッグ＆ドロップ処理
	*pResult = StartDragDrop();
}

void CFileBrowseView::DoHeaderTracking(HD_NOTIFY* pHDN)
{
	m_sdt[pHDN->iItem].cxChar = pHDN->pitem->cxy;
	CRect rc;
	m_list.GetClientRect(&rc);
	int i;
	for (i = 0; i < pHDN->iItem; i++)
		rc.left += m_sdt[i].cxChar;

	int xWidth = 0;
	for (i = 0; i < m_numColumns; i++)
		xWidth += m_sdt[i].cxChar;

	rc.left -= m_list.GetScrollPos(SB_HORZ);
	m_list.InvalidateRect(&rc, FALSE);
	m_list.SetHorizontalExtent(xWidth);

	// カラム幅
	if (!(gnListStat & LTS_AUTOADJUST)) {
		for (i = 0; i < m_numColumns; i++) {
			int type = MapSortIndexToType(i);
			if (type != SORT_NONE)
				sciWidth[type] = m_sdt[i].cxChar;
		}
	}
}

void CFileBrowseView::OnHeaderNotifyBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_bHeaderTracking = m_ListStyle.isStyleReport();
	*pResult = !m_bHeaderTracking;
}

void CFileBrowseView::OnHeaderNotifyItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_bHeaderTracking)
		DoHeaderTracking((HD_NOTIFY*)pNMHDR);
}

void CFileBrowseView::OnHeaderNotifyTracking(NMHDR* pNMHDR, LRESULT* pResult)
{
	DoHeaderTracking((HD_NOTIFY*)pNMHDR);
	*pResult = FALSE;
}

void CFileBrowseView::OnHeaderNotifyEndTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	DoHeaderTracking((HD_NOTIFY*)pNMHDR);
	m_bHeaderTracking = false;
	*pResult = 0;
}

void CFileBrowseView::OnHeaderAutoAdjust(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!m_ListStyle.isStyleReport())
		return;

	HD_NOTIFY* phdr = (HD_NOTIFY*)pNMHDR;
	m_sdt[phdr->iItem].cxChar = m_colWidthMax[phdr->iItem];
	RECT rc;
	m_list.GetClientRect(&rc);
	int i;
	for (i = 0; i < phdr->iItem; i++)
		rc.left += m_sdt[i].cxChar;

	int xWidth = 0;
	for (i = 0; i < m_numColumns; i++) {
		xWidth += m_sdt[i].cxChar;
		m_list.SetHeaderColumnWidth(i, m_sdt[i].cxChar);

		// カラム幅
		if (!(gnListStat & LTS_AUTOADJUST)) {
			int type = MapSortIndexToType(i);
			if (type != SORT_NONE)
				sciWidth[type] = m_sdt[i].cxChar;
		}
	}

	rc.left -= m_list.GetScrollPos(SB_HORZ);
	m_list.InvalidateRect(&rc, FALSE);
	m_list.SetHorizontalExtent(xWidth);

	*pResult = 0;
}

void CFileBrowseView::OnHeaderNotifyItemClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	HD_NOTIFY* pHDN = (HD_NOTIFY*)pNMHDR;
	int index = pHDN->iItem;

	HDITEM hd;
	hd.mask = HDI_FORMAT;
	m_list.GetHeaderColumn(index, &hd);
	int seq = m_ListStyle.GetSortSeq();
	if (hd.fmt & (HDF_SORTDOWN | HDF_SORTUP))
		seq ^= TRUE;
	SortItems(index, seq);
	*pResult = 0;
}

BOOL CFileBrowseView::ProcessKeyDown(UINT nVKey)
{
	if (nVKey == VK_CONTROL || nVKey == VK_SHIFT)
		return FALSE;

	int vkeyCtrl = _ISKEYDOWN(VK_CONTROL);
	int vkeyShift = _ISKEYDOWN(VK_SHIFT);
	if (vkeyShift && ISVKEYALNUM(nVKey) && (gnFopStat & FOS_SHIFTSEARCH) && !vkeyCtrl && !m_finder.IsWindowVisible()) {
		ToggleFindBar();
		CString str = (TCHAR)nVKey;
		m_finder.SetText(str);
		return TRUE;
	}

	int key = 0;
	switch (nVKey) {
	case VK_RETURN:					// ファイル実行
		key = IDM_FILE_OPEN;
		break;
	case VK_DELETE:					// ファイル削除
		key = vkeyCtrl ? IDM_EDIT_CUT : IDM_FILE_DELETE;
		break;
	default:
		break;
	}

	if (key) {
		PostMessage(WM_COMMAND, MAKEWPARAM(key, 1), 0L);
		return TRUE;
	}

	return FALSE;
}

void CFileBrowseView::OnNotifyKeyDown(NMHDR* pHdr, LRESULT* pResult)
{
	*pResult = (LRESULT)ProcessKeyDown(((NMKEY*)pHdr)->nVKey);
}

int CFileBrowseView::FindNextMatchSpec(int iStart, LPCTSTR pszFilter, bool bRegExp)
{
	int cnt = m_list.GetCount();
	if (!pszFilter || *pszFilter == 0 || cnt < 2)
		return -1;

	if (iStart >= cnt) iStart = 0;

	// マッチ方法
	int matchType = bRegExp ? MATCHTYPE_REGEXP : ((gnFopStat & FOS_PREFIX_MATCH) ? MATCHTYPE_PREFIX : MATCHTYPE_PARTIAL);
	CMatchSpec ms(pszFilter, matchType, false);
	CFileItem* pf;
	int i;
	for (i = iStart; i < cnt; i++) {
		pf = m_stack[i];
		if (ms.Match(pf->m_column[0]))
			return i;
	}
	for (i = 0; i < iStart; i++) {
		pf = m_stack[i];
		if (ms.Match(pf->m_column[0]))
			return i;
	}
	return -1;
}

int CFileBrowseView::FindPrevMatchSpec(int iStart, LPCTSTR pszFilter, bool bRegExp)
{
	int cnt = m_list.GetCount();
	if (!pszFilter || *pszFilter == 0 || cnt < 2)
		return -1;

	if (iStart < 0) iStart = cnt - 1;
	int matchType = bRegExp ? MATCHTYPE_REGEXP : ((gnFopStat & FOS_PREFIX_MATCH) ? MATCHTYPE_PREFIX : MATCHTYPE_PARTIAL);
	CMatchSpec ms(pszFilter, matchType, false);
	CFileItem* pf;
	int i;
	for (i = iStart; i >= 0; i--) {
		pf = m_stack[i];
		if (ms.Match(pf->m_column[0]))
			return i;
	}
	for (i = cnt - 1; i > iStart; i--) {
		pf = m_stack[i];
		if (ms.Match(pf->m_column[0]))
			return i;
	}
	return -1;
}

void CFileBrowseView::RedrawSelItems()
{
	m_list.RedrawSelItems();
}

void CFileBrowseView::DoFileOpen(bool bByMouseAction)
{
	if (_ISKEYDOWN(VK_MENU)) {
		OnFileProperty();
		return;
	}

	int sel;
	if (m_list.GetCount() < 1 || (sel = m_list.GetCaretIndex()) < 0 || m_list.GetSel(sel) <= 0) {
		if (bByMouseAction && (gnMosStat & MOS_SPC_DBLCLK_UPDIR))
			gpMainFrame->SendMessage(WM_COMMAND, IDM_DIR_UPDIR);
		return;
	}

	CFileItem* pf = m_stack[sel];

	// ひとつ上
	if (pf->isUpdir()) {
		gpMainFrame->SendMessage(WM_COMMAND, IDM_DIR_UPDIR);
		return;
	}

	// フォルダ
	if (pf->isFolder()) {
		if (_ISKEYDOWN(VK_CONTROL)) {
			// Ctrlキー併用なら、新しいタブで非アクティブ状態で開く
			fvApp.OpenTabFolder(pf->m_pidlFq, FALSE);
			return;
		}

		int descid = pf->GetDescriptionId(m_psf);
		if (descid == SHDID_FS_DIRECTORY) {
			// ファイルフォルダであればツリーを開く
			gpMainFrame->OpenChildFolder(pf->m_pidlFq);
			return;
		}

		// WindowsXPでは ZIP書庫は SFGAO_BROWSABLE のフラグが付く
		SFGAOF attr = SFGAO_FILESYSANCESTOR | SFGAO_BROWSABLE | SFGAO_NONENUMERATED | SFGAO_STREAM | SFGAO_STORAGEANCESTOR | SFGAO_STORAGE;
		HRESULT hr = m_psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pf->m_pidl, &attr);
		if (hr == NOERROR) {
			if (attr & SFGAO_FILESYSANCESTOR) {
				// ファイルフォルダであればツリーを開く
				gpMainFrame->OpenChildFolder(pf->m_pidlFq);
				return;
			}

			if (!(attr & SFGAO_BROWSABLE)) {
				if (attr & SFGAO_NONENUMERATED) {
					// エクスプローラで開こう!
					SHOpenFolderAndSelectItems(m_idlFolder, 1, (LPCITEMIDLIST*)&pf->m_pidl, 0);
				}
				else {
					// シェルフォルダとして認識可能か
					CComPtr<IShellFolder> psf;
					if (m_psf->BindToObject(pf->m_pidl, 0, IID_PPV_ARGS(&psf)) == S_OK) {
						gpMainFrame->OpenChildFolder(pf->m_pidlFq);
					}
					else {
						// どうしよう?
					}
				}
				return;
			}
		}
	}

	// 通常のファイル
	if (_ISKEYDOWN(VK_SHIFT)) {
		switch (gnFopStat & FOS_EXEC_MASK) {
		case FOS_EXEC_VIEWER: OnExecViewer(); return;
		case FOS_EXEC_EDITOR: OnExecEditor(); return;
		}
	}

	CSimpleArray<LPCITEMIDLIST> arr;
	int items = PackIDListArray(arr);
	if (items > 4) {
		if (AppMessage(IDS_OPEN_MULTIWINDOW, NULL, MB_QOKCANCEL) != IDOK)
			return;
	}

	CWindow wndTop = GetTopLevelParent();
	for (int i = 0; i < items; i++) {
		CIDListPtr idl;
		if (idl.Combine(m_idlFolder, arr[i])) {
			bool bDone = false;
			CString name;

			if (idl.GetPath(name)) {
				int type = CheckExecType(name);
				if (type == CE_LNK) {
					CString target;
					if (getlinkinfo(name, target)) {
						if ((PathIsUNC(target) && !_pathisfile(target)) || _pathisdirectory(target)) {
							fvApp.OpenTabFolder(target, TRUE);
							bDone = true;
						}
					}
				} else if (type < 0 && !lccRegCheckShellVerb(name.GetString())) {
					if (!ExecAddin(m_hWnd, name, idl)) {
						// デフォルトビュアを最後に呼び出す
						if (_pathfileexists(name) && !_pathisdirectory(name)) {
							AddinViewer(m_hWnd, name);
						} else {
							InvokeIDList(wndTop, idl, gszVerbProp);
						}
						bDone = true;
					}
				}
			}
			if (!bDone) {
				if (!InvokeIDList(wndTop, idl))
					InvokeIDList(wndTop, idl, gszVerbProp);
			}
			idl.Free();
		}
	}
}

void CFileBrowseView::DoObjectMoveCopy(bool bCopy)
{
	// コピー可能か
	SFGAOF attr = GetAttributesOf(SFGAO_CAPABILITYMASK, FALSE);
	if (!(attr & (bCopy ? SFGAO_CANCOPY : SFGAO_CANMOVE))) return;

	// dataobject
	CComPtr<IDataObject> pdo;
	if (GetUIObjectOf(IID_PPV_ARGS(&pdo), FALSE) > 0) {
		BOOL bTabOpen = FALSE;
		BOOL bThreaded = FALSE;
		BOOL bAsAdmin = FALSE;
		TCHAR dst[MAX_PATH * 2];
		if (QueryCopy(m_hWnd, pdo, m_idlFolder, dst, TRUE, bTabOpen, bThreaded, bAsAdmin) == IDOK) {
			// コピー先ディレクトリが存在しなければ、ディレクトリ作成コピーなのか
			// 名前変更コピーなのかを決める
			if (!_pathisdirectory(dst)) {
				if (AppMessage(IDS_QUERY_MKDIR, dst, MB_QYESNO) != IDYES || !fopMakeTree(dst))
					return;
			}

			CIDListPtr idlTarget(dst);
			if (idl_CopyTo(m_hWnd, idlTarget, pdo, bCopy)) {
				if (_pathisdirectory(dst)) RegSaveHistory(IDS_SEC_HISTORY, IDS_KEY_COPYTARGET, MAXACCESS, dst);
				if (bTabOpen) fvApp.OpenTabFolder(dst, TRUE);
			}
		}
	}
}

void CFileBrowseView::OnFileCopy()
{
	TDropFiles df;
	if (MakeDropFiles(df) < 1) {
		DoObjectMoveCopy(true);
		return;
	}

	BOOL bTabOpen = FALSE;
	BOOL bThreaded = FALSE;
	BOOL bAsAdmin = FALSE;
	TCHAR dst[MAX_PATH * 2];
	if (QueryCopy2(m_hWnd, df, m_idlFolder, dst, TRUE, bTabOpen, bThreaded, bAsAdmin) != IDOK) return;

	// コピー先ディレクトリが存在しなければ、ディレクトリ作成コピーなのか
	// 名前変更コピーなのかを決める
	TCHAR src[MAX_PATH * 2];
	if (!_pathisdirectory(dst)) {
		// '\'で終わっていれば、フォルダが指定されたとみなす
		int len = lstrlen(dst) - 1;
		BOOL bSep = (dst[len] == _T('\\'));

		INT_PTR nAction;
		if (!bSep && df.GetCount() == 1 && !df.IsFolderInclude()) {
			nAction = QueryMakeDir(m_hWnd, dst);
			if (nAction == QMD_RENAME) {
				df.DragQueryFile(0, src, ARRAYSIZE(src));

				CComPtr<ILSFileOperation> pfo;
				HRESULT hr = CoCreateInstanceAsAdmin(m_hWnd, bAsAdmin, __uuidof(LSFileOperation), __uuidof(ILSFileOperation), (void**)&pfo);
				if (hr == S_OK) {
					if (pfo->FileCopy(GetTopLevelParent(), src, dst, TRUE) == S_OK && bTabOpen) {
						PathRemoveFileSpec(dst);
						fvApp.OpenTabFolder(dst, TRUE);
					}
					return;
				}
				else {
					lccErrorBox(m_hWnd, hr, APP_PRODUCT, NULL, MB_STOP);
				}
			}
			else if (nAction != QMD_MAKEDIR)
				return;
		}
		else {
			if (AppMessage(IDS_QUERY_MKDIR, dst, MB_QYESNO) != IDYES)
				return;
			nAction = QMD_MAKEDIR;
		}

		switch (nAction) {
		case QMD_MAKEDIR:					// ディレクトリ作成してコピー
			if (!fopMakeTree(dst))
				return;
			break;

		case QMD_RENAME:					// 名前変更してコピー
			break;

		default:
			return;
		}
	}

	AcceptFiles(df, DROPEFFECT_COPY, dst, bThreaded, bAsAdmin);
	if (bTabOpen) fvApp.OpenTabFolder(dst, TRUE);
}

void CFileBrowseView::OnFileMove()
{
	TDropFiles df;
	if (MakeDropFiles(df) < 1) {
		DoObjectMoveCopy(false);
		return;
	}

	BOOL bJumpToTarget = FALSE;
	BOOL bThreaded = FALSE;
	BOOL bAsAdmin = FALSE;
	TCHAR dst[MAX_PATH];
	if (QueryCopy2(m_hWnd, df, m_idlFolder, dst, FALSE, bJumpToTarget, bThreaded, bAsAdmin) != IDOK) return;

	// 移動先ディレクトリが存在しなければ作成する
	if (!fopValidateDir(m_hWnd, dst)) return;

	// ディレクトリパースを中止する
	StopDirectoryParsing();

	AcceptFiles(df, DROPEFFECT_MOVE, dst, bThreaded, bAsAdmin);
	if (bJumpToTarget) fvApp.OpenTabFolder(dst, TRUE);
}

void CFileBrowseView::OnFileDelete()
{
	SFGAOF eff = GetAttributesOf(SFGAO_CANDELETE);
	if (eff & SFGAO_CANDELETE) {
		TDropFiles df;
		int cnt = MakeDropFiles(df);

		// ディレクトリパースを中止する
		StopDirectoryParsing();

		if (gnFopStat & FOS_RECYCLE || _ISKEYDOWN(VK_SHIFT) || cnt < 1)
			InvokeContextMenuCommand(_T("delete"));
		else
			fopDeleteFile(GetTopLevelParent(), df);
	}
}

void CFileBrowseView::OnFileShred()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		// ディレクトリパースを中止する
		StopDirectoryParsing();
		fopFileShredder(GetTopLevelParent(), df, TRUE);
	}
}

void CFileBrowseView::OnFileRename()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		// ディレクトリパースを中止する
		StopDirectoryParsing();
		fopRenameFiles(GetTopLevelParent(), df);
	}
	else {
		// リネーム可能か
		SFGAOF attr = GetAttributesOf(SFGAO_CANRENAME, FALSE);
		if (attr & SFGAO_CANRENAME) {
			// dataobject
			CComPtr<IDataObject> pdo;
			if (GetUIObjectOf(IID_PPV_ARGS(&pdo), FALSE) > 0) {
				fopShellRenameItems(GetTopLevelParent(), pdo);
			}
		}
	}
}

void CFileBrowseView::OnFileAttrib()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		if (fopFileAttributes(GetTopLevelParent(), df)) {
			m_list.SetRedraw(FALSE);
			RecalcColumn();
			m_list.SetRedraw(TRUE);
			m_list.Invalidate(FALSE);
		}
	}
}

void CFileBrowseView::OnExecEditor()
{
	bool bExecEditor = false;
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		if (df.GetCount() > 4) {
			if (AppMessage(IDS_OPEN_MULTIWINDOW, NULL, MB_QOKCANCEL) != IDOK)
				return;
		}

		TCHAR name[MAX_PATH * 2];
		for (int i = 0; i < df.GetCount(); i++) {
			df.DragQueryFile(i, name, ARRAYSIZE(name));
			if (_pathfileexists(name) && !_pathisdirectory(name)) {
				AddinEditor(m_hWnd, name, NULL);
				bExecEditor = true;
			}
		}
	}

	if (!bExecEditor) {
		if (AppMessage(IDS_ASK_EXEC_EDITOR, NULL, MB_QYESNO) == IDYES)
			AddinEditor(m_hWnd, NULL, m_strCurPath);
	}
}

void CFileBrowseView::OnExecViewer()
{
	TCHAR name[MAX_PATH * 2];

	CFileItem* pf = GetCurrentSingleFileItem();
	if (pf &&
		(pf->isUpdir() || pf->isFolder()) &&
		!pf->isFSFile())								// OSがZIP書庫などをフォルダ扱いする設定の場合を回避
	{
		DoFileOpen();
		return;
	}

	TDropFiles df;
	int cnt = MakeDropFiles(df);
	if (cnt > 0) {
		if (cnt > 4) {
			if (AppMessage(IDS_OPEN_MULTIWINDOW, NULL, MB_QOKCANCEL) != IDOK)
				return;
		}

		for (int i = 0; i < cnt; i++) {
			df.DragQueryFile(i, name, ARRAYSIZE(name));
			if (!_pathisdirectory(name) && _pathfileexists(name)) {
				if (!ExecAddin(m_hWnd, name))
					AddinViewer(m_hWnd, name);
			}
		}
	}
}

void CFileBrowseView::OnExecBinDump()
{
	TDropFiles df;
	int cnt = MakeDropFiles(df);
	if (cnt > 0) {
		if (cnt > 4) {
			if (AppMessage(IDS_OPEN_MULTIWINDOW, NULL, MB_QOKCANCEL) != IDOK)
				return;
		}

		TCHAR name[MAX_PATH * 2];
		for (int i = 0; i < cnt; i++) {
			df.DragQueryFile(i, name, ARRAYSIZE(name));
			if (!_pathisdirectory(name) && _pathfileexists(name))
				AddinHexEditor(m_hWnd, name);
		}
	}
}

void CFileBrowseView::OnViewFileInfo()
{
/*
	TDropFiles df;
	if (MakeDropFiles(df) > 0) fopShowFileInfo(GetTopLevelParent(), df);
*/
	CSimpleArray<LPCITEMIDLIST> arr;
	int items = PackIDListArray(arr, FALSE, TRUE);
	fopShowFileInformation(GetTopLevelParent(), (LPITEMIDLIST*)arr.GetData(), items, 0);
}

void CFileBrowseView::OnViewFileSize()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) fopEnumFileSize(GetTopLevelParent(), df);
}

void CFileBrowseView::OnFileMerge()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) fopMergeFile(GetTopLevelParent(), df);
}

void CFileBrowseView::OnFileShortcut() 
{
	int sel;
	if (m_list.GetSelCount() == 1 && m_list.GetSelItems(1, &sel) == 1) {
		CFileItem* pf = m_stack[sel];
		if (!pf->isUpdir() && pf->m_attr & SFGAO_CANLINK)
			MakeShortcutObject(m_hWnd, m_psf, pf->m_pidl, pf->m_column[0]);
	}
}

void CFileBrowseView::OnFileExecute()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		if (df.GetCount() == 1) {
			CExecuteInfo ei;

			TCHAR name[MAX_PATH * 2];
			df.DragQueryFile(0, name, ARRAYSIZE(name));
			int exectype = CheckExecType(name);
			PathQuoteSpaces(name);
			ei.m_strCommand = name;

			if (exectype >= 0 && exectype != CE_LNK) {
				ei.m_bExecType = TRUE;
				ei.m_strCommand += _T(' ');
			}

			CExecDlg dlg;
			dlg.m_pei = &ei;
			if (dlg.DoModal() == IDOK) {
				lstrcpy(name, ei.m_strCommand);
				if (name[0] == _T('"')) {
					LPTSTR p = StrChr(name + 1, _T('"'));
					if (p) *++p = 0;
					PathUnquoteSpaces(name);
				}

				if (PathIsUNC(name) || _pathisdirectory(name)) {
					ExecNewFrame(name, _T("/force"));
					return;
				}
				ExecuteAsAdmin(m_hWnd, ei.m_strCommand, m_strCurPath, SW_SHOWNORMAL, ei.m_bAsAdmin);
			}
		}
		else
			MessageBeep(0);
	}
}

void CFileBrowseView::OnFileDivide()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		if (df.GetCount() == 1 && !df.IsFolderInclude()) {
			DIVIDEDATA dd;
			memset(&dd, 0, sizeof(dd));
			df.DragQueryFile(0, dd.szTarget, ARRAYSIZE(dd.szTarget));
			if (fopQueryDivide(GetTopLevelParent(), &dd))
				fopDivideFile(GetTopLevelParent(), &dd);
		}
	}
}

void CFileBrowseView::OnFilePack()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) ArcCompress(m_hWnd, m_strCurPath, df);
}

void CFileBrowseView::OnFileUnpack()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		TCHAR name[MAX_PATH * 2];
/* --- 最初のファイルのみチェックするようにして簡略化
		TDropFiles dfw;
		for (int i = 0; i < df.GetCount(); i++) {
			df.DragQueryFile(i, name, ARRAYSIZE(name));
			if (_pathisdirectory(name))
				continue;

			if (SpecifyArchiveTypeByFileHeader(name) == ARC_ERROR) {
				LPTSTR pe = PathFindExtension(name);
				if (!pe || *pe != '.' || StrCmpI(++pe, _T("EXE")))
					continue;
			}
			dfw.DragAppendFile(name);
		}
		ArcExtract(m_hWnd, m_strCurPath, dfw);
--- */
		df.DragQueryFile(0, name, ARRAYSIZE(name));
		if (_pathisdirectory(name)) return;

		if (SpecifyArchiveTypeByFileHeader(name) == ARC_ERROR) {
			LPTSTR pe = PathFindExtension(name);
			if (*pe != '.' || StrCmpI(++pe, _T("EXE")))
				return;
		}
		ArcExtract(m_hWnd, m_strCurPath, df);
	}
}

void CFileBrowseView::OnFileCheckArchive()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) ArcCheck(m_hWnd, df);
}

void CFileBrowseView::OnCopyNameRange(UINT idCmd, UINT /*code*/, HWND /*hwnd*/)
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		switch (idCmd) {
		case IDM_COPY_UNCNAME:	df.CopyPathToClipboard(m_hWnd, DFCP_UNCPATH); break;
		case IDM_COPY_FULLNAME: df.CopyPathToClipboard(m_hWnd, DFCP_FULLPATH); break;
		case IDM_COPY_NAMEONLY: df.CopyPathToClipboard(m_hWnd, DFCP_NAMEONLY); break;
		case IDM_COPY_SHORTNAME: df.CopyPathToClipboard(m_hWnd, DFCP_SHORTPATH); break;
		case IDM_COPY_DETAIL: df.CopyToClipboard(m_hWnd); break;
		}

		if (_ISKEYDOWN(VK_SHIFT))
			_openclipboardbyeditor(GetTopLevelParent());
	}
}

void CFileBrowseView::InvokeContextMenuCommand(LPCTSTR pszVerb)
{
	CComPtr<IContextMenu> pcm;
	if (GetUIObjectOf(IID_PPV_ARGS(&pcm)) > 0) {
		ctx_Execute(GetTopLevelParent(), pcm, pszVerb);
	}
}

void CFileBrowseView::OnFileProperty()
{
//@	InvokeContextMenuCommand(gszVerbProp);
	CComPtr<IDataObject> pdo;
	if (GetUIObjectOf(IID_PPV_ARGS(&pdo)) > 0) {
		SHMultiFileProperties(pdo, 0);
	}
}

void CFileBrowseView::SelectAll(int nSelMode)
{
	int cnt = m_list.GetCount();
	if (cnt < 1)
		return;

	// 全選択状態であれば、非選択状態にする
	if (m_list.GetSelCount() == cnt && nSelMode == FILESEL_ALL)
		nSelMode = FILESEL_NON;

	m_list.SetRedraw(FALSE);
	int caret = m_list.GetTopIndex();
	for (int i = 0; i < cnt; i++) {
		switch (nSelMode) {
		case FILESEL_ALL: m_list.SetSel(i, TRUE); break;
		case FILESEL_NON: m_list.SetSel(i, FALSE); break;
		default: m_list.SetSel(i, !m_list.GetSel(i)); break;
		}
	}
	m_list.SetTopIndex(caret);
	m_list.SetRedraw(TRUE);
	m_list.Invalidate(FALSE);
}

void CFileBrowseView::SelectFiles(LPCTSTR pszFilter, bool bUnmatch, bool bRegExp, bool bCaseSense, bool bMark)
{
	if (!m_rawfiles) return;

	// 一旦全選択または全解除する
	m_list.SetRedraw(FALSE);

	if (bMark) {
		m_list.SetMark(-1, bUnmatch);
		m_numMarks = 0;
		m_dlMarkTotal = 0;
	}
	else
		m_list.SetSel(-1, bUnmatch);

	if (pszFilter && *pszFilter) {
		// マッチ方法
		int matchType = bRegExp ? MATCHTYPE_REGEXP : ((gnFopStat & FOS_PREFIX_MATCH) ? MATCHTYPE_PREFIX : MATCHTYPE_PARTIAL);
		CMatchSpec ms(pszFilter, matchType, bCaseSense);
		int cnt = m_list.GetCount();
		for (int i = 0; i < cnt; i++) {
			CFileItem* pf = m_stack[i];
			if (ms.Match(pf->m_column[0])) {
				if (bMark) {
					m_list.SetMark(i, !bUnmatch);
					m_numMarks++;
					m_dlMarkTotal += pf->m_filesize;
				}
				else
					m_list.SetSel(i, !bUnmatch);
			}
		}
	}

	m_list.SetRedraw(TRUE);
	m_list.Invalidate(FALSE);
}

void CFileBrowseView::GetSelectionFilter(LPTSTR pszFilter, bool useRegExp)
{
	if (!pszFilter) return;

	// 現在選択されているファイルの拡張子をフィルタとして切り出す
	*pszFilter = 0;
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		TCHAR name[MAX_PATH * 2];
		CString flt, ext;
		CSimpleArray<CString> arr;		// 既に登録したもののリスト
		for (int i = 0; i < df.GetCount(); i++) {
			df.DragQueryFile(i, name, ARRAYSIZE(name));
			LPTSTR p = PathFindExtension(name);
			ext = p;
			ext.MakeUpper();
			if (*p == '.' && arr.Find(ext) < 0) {
				if (arr.GetSize() > 0)
					flt += _T(";");
				flt.AppendFormat(useRegExp ? _T(".*\\%s$") : _T("*%s"), p);
				arr.Add(ext);
			}
		}
		if (arr.GetSize() > 0)
			lstrcpy(pszFilter, flt);
	}
}

void CFileBrowseView::OnBuildSortMenu(WPARAM wParam, LPARAM lParam)
{
	if (!::IsMenu((HMENU)lParam)) return;

	CMenuHandle pop = (HMENU)lParam;

	int cnt = pop.GetMenuItemCount();
	int i;
	for (i = 0; i < cnt; i++) {
		CMenuItemInfo mii;					// loop内で初期化しないとvistaでは機能しない XX
		mii.fMask = MIIM_TYPE;
		if (!pop.GetMenuItemInfo(0, TRUE, &mii) || mii.fType & MFT_SEPARATOR)
			break;
		pop.DeleteMenu(0, MF_BYPOSITION);
	}

	static const TCHAR sszSortPrefix[] = _T("NESTAOBCFGHIJKLM");
	UINT baseid = isFileSystemFolder() ? IDM_SORT_NAME : IDM_SORT_COL0;

	CString str;
	for (i = 0; i < m_numColumns; i++) {
		str.Format(_T("%s (&%C)"), m_sdt[i].title, sszSortPrefix[i]);
		pop.InsertMenu(i, MF_BYPOSITION | MF_STRING, i + baseid, str);
	}
}

void CFileBrowseView::OnQuerySort(WPARAM wParam, LPARAM lParam)
{
	CSortDlg dlg;
	dlg.m_sortindex = MapSortTypeToIndex(m_ListStyle.GetSortType());
	dlg.m_sortseq = m_ListStyle.GetSortSeq();
	for (int i = 0; i < m_numColumns; i++)
		dlg.AddSortType(m_sdt[i].title);

	if (dlg.DoModal() == IDOK)
		SortItems(dlg.m_sortindex, dlg.m_sortseq);
}

void CFileBrowseView::OnFileRenameExtension()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0) {
		TCHAR name[MAX_PATH * 2];
		df.DragQueryFile(0, name, ARRAYSIZE(name));
		fopRenameExtension(GetTopLevelParent(), name);
	}
}

int CFileBrowseView::FindItemIndex(LPCITEMIDLIST pidl, bool bCompareFullPidl)
{
	int cnt = m_list.GetCount();
	if (cnt > 0) {
		for (int i = 0; i < cnt; i++) {
			if (bCompareFullPidl) {
				if (ILIsEqual(m_stack[i]->m_pidlFq, pidl)) return i;
			}
			else {
				if (ILIsEqual(m_stack[i]->m_pidl, pidl)) return i;
			}
		}
	}
	return -1;
}

CFileItem* CFileBrowseView::FindItem(LPCITEMIDLIST pidl, bool bCompareFullPidl)
{
	int index = FindItemIndex(pidl, bCompareFullPidl);
	return ((index < 0) ? NULL : m_stack[index]);
}

void CFileBrowseView::OnFileContextMenu()
{
	CComPtr<IContextMenu> pcm;
	if (GetUIObjectOf(IID_PPV_ARGS(&pcm)) > 0) {
		CPoint pt;
		const _ATL_MSG* msg = GetCurrentMessage();
		GetPopupMenuPoint(LOWORD(msg->wParam), &pt);

		SFGAOF attr = GetAttributesOf(SFGAO_FOLDER|SFGAO_FILESYSTEM);
		int cmdBack = TrackContextMenu(m_hWnd, pcm, &pt, attr);

		if (cmdBack > 0) {
			int index = m_list.GetCaretIndex();
			CFileItem* pf = m_stack[index];
			switch (cmdBack) {
			case IDM_TAB_NEW:
				// アクティブで開く
				fvApp.OpenTabFolder(pf->m_pidlFq, TRUE);
				break;
			case IDM_DIR_ADDFAVORITE:
				// しおりへ追加
				fvApp.AddFavorite(pf->m_pidlFq);
				break;
			default:
				break;
			}
		}
	}
}

LRESULT CFileBrowseView::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	if ((HWND)wParam != m_list.m_hWnd) return 0;

	CPoint pt((short)LOWORD(lParam), (short)HIWORD(lParam));
	CPoint ptClient = pt;
	m_list.ScreenToClient(&ptClient);
	int index = m_list.ItemFromPoint(ptClient);

	int cmdBack;
	bool bDoDefaultMenu = true;
	if (index >= 0) {
		if ((gnMosStat & MOS_RBTN_MASK) == MOS_RBTN_SINGLE_SEL) return 0;

		if (m_psf && (gnMosStat & MOS_RBTN_MASK) == MOS_RBTN_CONTEXTMENU) {
			CComPtr<IContextMenu> pcm;
			if (GetUIObjectOf(IID_PPV_ARGS(&pcm)) > 0) {
				SFGAOF attr = GetAttributesOf(SFGAO_FOLDER);
				cmdBack = TrackContextMenu(m_hWnd, pcm, &pt, attr);

				if (cmdBack > 0) {
					CFileItem* pf = m_stack[index];
					switch (cmdBack) {
					case IDM_TAB_NEW:
						// アクティブで開く
						fvApp.OpenTabFolder(pf->m_pidlFq, TRUE);
						break;
					case IDM_DIR_ADDFAVORITE:
						// しおりへ追加
						fvApp.AddFavorite(pf->m_pidlFq);
						break;
					default:
						break;
					}
				}
				bDoDefaultMenu = false;
			}
		}
	}
	else if (gnMosStat & MOS_SPC_FOLDER_CTXMENU && !m_idlFolder.isNull()) {
		index = m_list.GetCaretIndex();
		m_list.SetSel(-1, false);
		m_list.SetAnchorIndex(index);
		cmdBack = TrackContextMenuByIDList(m_hWnd, m_idlFolder, &pt);

		CIDListPtr idl;
		if (cmdBack > 0 && idl.Clone(m_idlFolder)) {
			switch (cmdBack) {
			case IDM_TAB_NEW:
				// アクティブで開く
				fvApp.OpenTabFolder(idl, TRUE);
				break;
			case IDM_DIR_ADDFAVORITE:
				// しおりへ追加
				fvApp.AddFavorite(idl);
				break;
			default:
				break;
			}
		}
		bDoDefaultMenu = false;
	}

	if (bDoDefaultMenu)
		gpMainFrame->TrackMenuForFileBrowser(pt);
	return 0;
}

BOOL CFileBrowseView::StartDragDrop()
{
	// ドラッグ＆ドロップ処理
	CPoint pt;
	GetCursorPos(&pt);

	BOOL bOK = FALSE;
	CComPtr<IDataObject> pdo;
	int cnt = GetUIObjectOf(IID_PPV_ARGS(&pdo));
	if (cnt > 0) {
		SFGAOF eff = GetAttributesOf(DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK);
		if (eff) {
			if (lccBeginTrack(m_hWnd, pt)) {
				// ディレクトリパースを中止する
				StopDirectoryParsing();

				DragStart(m_hWnd, pdo, cnt, &eff);
				bOK = TRUE;
			}
		}
	}
	return bOK;
}

void CFileBrowseView::OnFileFilterDelete()
{
	TDropFiles df;
	int cnt = MakeDropFiles(df);
	if (cnt > 0) {
		TCHAR name[MAX_PATH * 2];
		for (int i = 0; i < cnt; i++) {
			df.DragQueryFile(i, name, ARRAYSIZE(name));
			if (!_pathisdirectory(name))
				return;
		}
		fopFilterDeleteEx(GetTopLevelParent(), df);
	}
}


void CFileBrowseView::OnFileSeriesName()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0)
		fopSeriesName(GetTopLevelParent(), df);
}

void CFileBrowseView::OnFileRepNames()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0)
		fopRepStrNames(GetTopLevelParent(), df);
}

void CFileBrowseView::OnFileChangeCaps()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0)
		fopChangeFileCaps(GetTopLevelParent(), df);
}

/*
void CFileBrowseView::OnFileEncrypt()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0)
		fopCryptFiles(GetTopLevelParent(), df, TRUE);
}
*/
/*
void CFileBrowseView::OnFileDecrypt()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0)
		fopCryptFiles(GetTopLevelParent(), df, FALSE);
}
*/
void CFileBrowseView::OnFileAssociate()
{
	TCHAR name[MAX_PATH * 2];
	name[0] = '\0';
	TDropFiles df;
	if (MakeDropFiles(df) > 0)
		df.DragQueryFile(0, name, ARRAYSIZE(name));
	fopAssociateExtension(GetTopLevelParent(), name);
}

void CFileBrowseView::OnSendTo()
{
	CComPtr<IDataObject> pdo;
	if (GetUIObjectOf(IID_PPV_ARGS(&pdo)) > 0) {
		CPoint pt;
		GetPopupMenuPoint(IDM_FILE_SENDTO, &pt);
		SendTo(m_hWnd, pdo, pt);
	}
}

void CFileBrowseView::GetSelectionCenterPoint(LPPOINT ppt)
{
	int index = m_list.GetCaretIndex();
	if (index >= 0) {
		CRect rc;
		m_list.GetItemRect(index, &rc);
		ppt->x = rc.left + rc.Width() / 2;
		ppt->y = rc.top + rc.Height() / 2;
		m_list.ClientToScreen(ppt);
	}
	else
		GetCursorPos(ppt);
}

void CFileBrowseView::GetPopupMenuPoint(UINT nIDCommand, LPPOINT ppt)
{
	const _ATL_MSG* msg = GetCurrentMessage();
	GetCursorPos(ppt);
	if (!IsCommandTranslateAccel(msg) || !GetToolBarButtonPoint(msg, nIDCommand, ppt))
		GetSelectionCenterPoint(ppt);
}

void CFileBrowseView::OnEditCopy()
{
	InvokeContextMenuCommand(_T("copy"));
}

void CFileBrowseView::OnEditCut()
{
	InvokeContextMenuCommand(_T("cut"));
}

static TCHAR sszTdLt[] = _T("<td>%s</td>\n");
static TCHAR sszTdRt[] = _T("<td align=\"right\">%s</td>\n");
static TCHAR sszSpace[] = _T("&nbsp;");
static TCHAR sszDir[] = _T("&lt;dir&gt;");

void CFileBrowseView::OutputHtml(LPCTSTR pszName, BOOL bListAll, BOOL bLaunchApp)
{
	TDropFiles df;
	if (MakeDropFiles(df, bListAll) < 1) return;

	TCHAR html[MAX_PATH], buf[MAX_PATH];
	lstrcpy(html, pszName);
	PathRemoveExtension(html);
	lstrcat(html, _T(".html"));
	if (_pathfileexists(html))
		fopRemoveFileNoConfirm(html);

	HANDLE hf = _createfile(html, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FAT_NORMAL, NULL);
	if (hf == INVALID_HANDLE_VALUE) return;

	CWaitCursor wc;

	CString str;
	CCodePageOutput cpo(hf, CP_UTF8);
	str.Format(_T("<html><head>\n<title>%s</title>\n"), m_strCurPath.GetString());
	cpo.puts(str);
	cpo.puts(_T("<meta http-equiv=\"Content-Type\" "));
	cpo.puts(_T(" content=\"text/html; charset=utf-8\">\n"));
	str.Format(_T("<meta name=\"GENERATOR\" content=\"%s\">\n"), APP_PRODUCT);
	cpo.puts(str);
	cpo.puts(_T("<meta http-equiv=\"Content-Language\" content=\"ja\">\n"));
	cpo.puts(_T("<style type=\"text/css\">\n<!--\n"));
	cpo.puts(_T("a { text-decoration:none; }\n"));
	cpo.puts(_T("a:link { color:#003399; text-decoration:none; }\n"));
	cpo.puts(_T("a:visited {color:#003399; text-decoration:none; }\n"));
	cpo.puts(_T("a:hover { color:#ffffff; background-color:#003399; "));
	cpo.puts(_T("text-decoration:none; }\n"));
	cpo.puts(_T("body { font: , , ; font-size: 10pt; }\n"));
	cpo.puts(_T("td { font: , , ; font-size: 10pt; vertical-align: baseline;\n}\n"));
	cpo.puts(_T("//-->\n</style>\n"));
	cpo.puts(_T("</head>\n"));
	cpo.puts(_T("<body bgcolor=\"#ffffff\" text=\"#000000\">\n"));

	SYSTEMTIME st;
	GetLocalTime(&st);
	StrFormatByteSize64(bListAll ? m_dlTotal : m_dlSelTotal, buf, ARRAYSIZE(buf));
	str.Format(_T("<p>Folder of <a href=\"file:///%s\"><b>%s</b></a><p><br></p>"),
			m_strCurPath.GetString(), m_strCurPath.GetString());
	cpo.puts(str);
	str.Format(_T("%d files, Total %s&nbsp;&nbsp;&nbsp;&nbsp;%d/%02d/%02d %02d:%02d<br>\n"),
			df.GetCount(), buf, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	cpo.puts(str);
	cpo.puts(_T("<table border=\"1\" cellspacing=\"0\" "));
	cpo.puts(_T("cellpadding=\"2\" bordercolorlight=\"#C0C0C0\" "));
	cpo.puts(_T("bordercolordark=\"#C0C0C0\" bordercolor=\"#C0C0C0\">\n"));
		TCHAR name[MAX_PATH * 2];
		cpo.puts(_T("<tr bgcolor=\"#3366CC\">\n"));
		for (int i = 0; i < 6; i++) {
			str.Format(_T("<td><font color=\"#FFFFFF\">%s</font></td>\n"), sciDefault[i].title);
			cpo.puts(str);
		}
		cpo.puts(_T("</tr>\n"));

		CFileFinder ff;
		CString fmt;
		for (int i = 0; i < df.GetCount(); i++) {
			df.DragQueryFile(i, name, ARRAYSIZE(name));
			if (!GetFileData(name, &ff))
				continue;

			cpo.puts(_T("<tr>\n"));
				// ファイル名
				str.Format(sszTdLt, (LPCTSTR)ff.cFileName);
				cpo.puts(str);

				// サイズ
				if (!ff.isFolder()) {
					DWORDLONG dl;
					SETDWORDLONG(dl, ff.nFileSizeLow, ff.nFileSizeHigh);
					FormatFileSize(dl, fmt, gnListStat & LTS_FMTSIZE);
				}
				else
					fmt = sszDir;
				str.Format(sszTdRt, fmt.GetString());
				cpo.puts(str);

				// 更新日時
				FormatFileTime(&ff.ftLastWriteTime, fmt, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
				str.Format(sszTdLt, fmt.GetString());
				cpo.puts(str);

				// 種類
				SHFILEINFO sfi;
				SHGetFileInfo(name, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);
				str.Format(sszTdLt, sfi.szTypeName[0] ? sfi.szTypeName : sszSpace);
				cpo.puts(str);

				// 属性
				ff.FormatFileAttributes(fmt);
				str.Format(sszTdRt, fmt.GetLength() > 0 ? fmt.GetString() : sszSpace);
				cpo.puts(str);

				// DOS名
				str.Format(sszTdLt, ff.cAlternateFileName[0] ? ff.cAlternateFileName : sszSpace);
				cpo.puts(str);

			cpo.puts(_T("</tr>\n"));
		}
	cpo.puts(_T("</table>\n"));
	cpo.puts(_T("</body></html>\n"));

	CloseHandle(hf);

	if (bLaunchApp)
		ShellExecute(m_hWnd, NULL, html, NULL, m_strCurPath, SW_SHOWNORMAL);
}

void CFileBrowseView::OutputXml(LPCTSTR pszName, BOOL bListAll, BOOL bLaunchApp)
{
	TDropFiles df;
	if (MakeDropFiles(df, bListAll) < 1) return;

	TCHAR xml[MAX_PATH * 2], buf[MAX_PATH * 2];
	lstrcpy(xml, pszName);
	PathRemoveExtension(xml);
	lstrcat(xml, _T(".xml"));
	if (_pathfileexists(xml)) {
		CString str; str.Format(IDS_ASK_ITEM_OVERWRITE, xml);
		if (MessageBox(str, APP_PRODUCT, MB_QYESNO) != IDYES)
			return;
		fopRemoveFileNoConfirm(xml);
	}

	CWaitCursor wc;

	// Save XML
	TiXmlDocument xmlDoc;
	xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

	// --------------------------
	// XML directory header
	// --------------------------
	TiXmlNode* pDoc = xmlDoc.InsertEndChild( TiXmlElement( "directory" ) );
	TiXmlElement* pDir = pDoc->ToElement();

	// directory attributes
	CStringA utf;
	// 現在のパス
	UnicodeToUTF8(m_strCurPath, utf);
	pDir->SetAttribute( "path", utf );
	// ファイルサイズ
	StrFormatByteSize64(bListAll ? m_dlTotal : m_dlSelTotal, buf, ARRAYSIZE(buf));
	UnicodeToUTF8(buf, utf);
	pDir->SetAttribute( "total_file_size", utf );
	// ファイル個数
	wsprintf(buf, _T("%d"), df.GetCount());
	UnicodeToUTF8(buf, utf);
	pDir->SetAttribute( "total_file_count", utf );
	// リストした日付
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(buf, _T("%04d/%02d/%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	UnicodeToUTF8(buf, utf);
	pDir->SetAttribute( "generated_time", utf );
	// Generater
	UnicodeToUTF8(APP_PRODUCT, utf);
	pDir->SetAttribute( "generater", utf );

	// XML file or folder
	TiXmlElement* pElement;
	CFileFinder ff;
	for (int i = 0; i < df.GetCount(); i++) {
		TCHAR name[MAX_PATH * 2];
		df.DragQueryFile(i, name, ARRAYSIZE(name));
		if (!GetFileData(name, &ff))
			continue;

		TiXmlNode* pItem = pDir->InsertEndChild( TiXmlElement( ff.isFolder() ? "folder" : "file" ) );
		pElement = pItem->ToElement();

		// ファイル名
		UnicodeToUTF8(ff.cFileName, utf);
		pElement->SetAttribute( "name", utf );

		// サイズ
		CString fmt;
		if (!ff.isFolder()) {
			DWORDLONG dl;
			SETDWORDLONG(dl, ff.nFileSizeLow, ff.nFileSizeHigh);
			FormatFileSize(dl, fmt, gnListStat & LTS_FMTSIZE);
		}
		UnicodeToUTF8(fmt, utf);
		pElement->SetAttribute( "size", utf );

		// 更新日時
		FormatFileTime(&ff.ftLastWriteTime, fmt, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
		UnicodeToUTF8(fmt, utf);
		pElement->SetAttribute( "lastwritetime", utf );

		// 種類
		SHFILEINFO sfi;
		SHGetFileInfo(name, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);
		UnicodeToUTF8(sfi.szTypeName, utf);
		pElement->SetAttribute( "type", utf );

		// 属性
		ff.FormatFileAttributes(fmt);
		UnicodeToUTF8(fmt, utf);
		pElement->SetAttribute( "attributes", utf );

		// DOS名
		UnicodeToUTF8(ff.cAlternateFileName, utf);
		pElement->SetAttribute( "dos", utf );
	}

	// Save XML
	char tmp[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, xml, -1, tmp, ARRAYSIZE(tmp), NULL, NULL);
	// 書き出し可能か？
	if (!CheckFileWritable(tmp)) {
		;	// とりあえずreadonly属性がついていれば解除される
	}
	xmlDoc.SaveFile(tmp);

	if (bLaunchApp)
		ShellExecute(m_hWnd, NULL, xml, NULL, m_strCurPath, SW_SHOWNORMAL);
}

void CFileBrowseView::OutputCsv(LPCTSTR pszName, BOOL bListAll, BOOL bLaunchApp)
{
	TDropFiles df;
	if (MakeDropFiles(df, bListAll) < 1) return;

	TCHAR csv[MAX_PATH * 2], buf[MAX_PATH * 2];
	lstrcpy(csv, pszName);
	PathRemoveExtension(csv);
	lstrcat(csv, _T(".csv"));
	if (_pathfileexists(csv))
		fopRemoveFileNoConfirm(csv);

	HANDLE hf = _createfile(csv, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FAT_NORMAL, NULL);
	if (hf == INVALID_HANDLE_VALUE) return;

	CWaitCursor wc;
	CCodePageOutput cpo(hf, CP_ACP);

	// フォルダパス,総ファイル数,総サイズ
	CString str;
	StrFormatByteSize64(bListAll ? m_dlTotal : m_dlSelTotal, buf, ARRAYSIZE(buf));
	str.Format(_T("Folder,%s\nTotal size,%s\nTotal files,%d\n\n"), m_strCurPath.GetString(), buf, df.GetCount());
	cpo.puts(str);

	str.Empty();
	int i;
	for (i = 0; i < 6; i++) {
		str += sciDefault[i].title;
		if (i < 5)
			str += _T(',');
	}
	str += _T('\n');
	cpo.puts(str);

	CFileFinder ff;
	for (i = 0; i < df.GetCount(); i++) {
		TCHAR name[MAX_PATH * 2];
		df.DragQueryFile(i, name, ARRAYSIZE(name));
		if (!GetFileData(name, &ff))
			continue;

		// ファイル名
		str = ff.cFileName;
		str += _T(',');

		// サイズ
		if (!ff.isFolder()) {
			DWORDLONG dl;
			SETDWORDLONG(dl, ff.nFileSizeLow, ff.nFileSizeHigh);
			_ui64tot_s(dl, buf, ARRAYSIZE(buf), 10);
		}
		else
			lstrcpy(buf, _T("<dir>"));
		str += buf;
		str += _T(',');

		// 更新日時
		CString fmt;
		FormatFileTime(&ff.ftLastWriteTime, fmt, gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
		str += fmt;
		str += _T(',');

		// 種類
		SHFILEINFO sfi;
		SHGetFileInfo(name, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);
		str += sfi.szTypeName;
		str += _T(',');

		// 属性
		ff.FormatFileAttributes(fmt);
		str += fmt;
		str += _T(',');

		// DOS名
		str += ff.cAlternateFileName;
		str += _T('\n');

		cpo.puts(str);
	}

	// 作成日時
	SYSTEMTIME st;
	GetLocalTime(&st);
	str.Format(_T("\nGenerarted on %04d/%02d/%02d %02d:%02d by %s\n"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, APP_PRODUCT);
	cpo.puts(str);
	CloseHandle(hf);

	if (bLaunchApp)
		ShellExecute(m_hWnd, NULL, csv, NULL, m_strCurPath, SW_SHOWNORMAL);
}

void CFileBrowseView::OnEditListOutput()
{
	CListOutputDlg dlg;
	dlg.m_strPath = m_strCurPath;
	if (dlg.DoModal() != IDOK) return;

	switch (dlg.m_nOutputType) {
	case 0: OutputHtml(dlg.m_strPath, dlg.m_bListAll, dlg.m_bLaunchApp); break;
	case 1: OutputXml(dlg.m_strPath, dlg.m_bListAll, dlg.m_bLaunchApp); break;
	case 2: OutputCsv(dlg.m_strPath, dlg.m_bListAll, dlg.m_bLaunchApp); break;
	}
}

void CFileBrowseView::OnToolCalcHash()
{
	TDropFiles df;
	if (MakeDropFiles(df) > 0)
		fopCalcFileHash(GetTopLevelParent(), df);
}

BOOL CFileBrowseView::OnGetDragImage(SHDRAGIMAGE* image)
{
	CSize sizeIcon = m_sizeExtra;
	IImageList* pil = m_pilExtra;
	if (m_ListStyle.isStyleNormal() ||
		m_ListStyle.isStyleReport()) {
		sizeIcon = m_sizeLarge;
		pil = m_pilLarge;
	}
	else if (m_ListStyle.isStyleOutline()) {
		sizeIcon = m_ListStyle.isEnlargeOutline() ? m_sizeExtra : m_sizeLarge;
		pil = m_ListStyle.isEnlargeOutline() ? m_pilExtra : m_pilLarge;
	}
	else if (m_ListStyle.isStyleIcon()) {
		sizeIcon = m_ListStyle.isEnlargeIcon() ? m_sizeExtra : m_sizeLarge;
		pil = m_ListStyle.isEnlargeOutline() ? m_pilExtra : m_pilLarge;
	}
	int xDelta = 3;
	int yDelta = 3;

	memset(image, 0, sizeof(SHDRAGIMAGE));
	int numSelect = m_list.GetSelCount();
	if (numSelect > 0) {
		numSelect = min(4, numSelect);
		int sel[4];
		numSelect = m_list.GetSelItems(numSelect, sel);

		image->crColorKey = CLR_NONE;
		image->sizeDragImage.cx = sizeIcon.cx + xDelta * (numSelect - 1) + 6;
		image->sizeDragImage.cy = sizeIcon.cy + xDelta * (numSelect - 1) + 6;
		image->sizeDragImage.cx += m_dropshadow.delta();
		image->sizeDragImage.cy += m_dropshadow.delta();
//		image->ptOffset.x = image->sizeDragImage.cx - (sizeIcon.cx / 2);
		image->ptOffset.x = image->sizeDragImage.cx * 4 / 5;
		image->ptOffset.y = image->sizeDragImage.cy * 4 / 5;

		CClientDC dc(m_list);
		HBITMAP hbm = CreateCompatibleBitmap(dc, image->sizeDragImage.cx, image->sizeDragImage.cy);
		if (hbm) {
			CDC dcw;
			dcw.CreateCompatibleDC(dc);
			hbm = dcw.SelectBitmap(hbm);

			CRect rc(0, 0, image->sizeDragImage.cx, image->sizeDragImage.cy);
			// ドロップシャドウ処理
			m_dropshadow.Apply(dcw, &rc);
			rc.right -= m_dropshadow.delta();
			rc.bottom -= m_dropshadow.delta();

			dcw.FillSolidRect(&rc, GetSysColor(COLOR_WINDOW));
static COLORREF clrDrag[] = {
	RGB(148, 194, 248),
	RGB(148, 194, 248),
	RGB(148, 194, 248),
	RGB(148, 194, 248),
};
			lccDrawSelectionFrameEx(dcw, &rc, clrDrag, 128);

			CFileItem *pf;
			int x = 3;
			int y = 3;
			for (int i = 0; i < numSelect; i++) {
				pf = m_stack[sel[numSelect - i - 1]];
				if (pf->isUpdir()) continue;

				IImageList_DrawIcon(pil, dcw, pf->m_iImage, CPoint(x, y), CSize(0, 0), ILD_IMAGE | pf->OverlayImageMask());
				x += xDelta;
				y += yDelta;
			}
			image->hbmpDragImage = dcw.SelectBitmap(hbm);
			dcw.DeleteDC();
		}
	}

	return (image->hbmpDragImage != NULL);
}

DWORD CFileBrowseView::OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	// ドロップ可能ファイルを特定する
	m_list.ScreenToClient(&point);

	DROPEFFECT eff = DROPEFFECT_NONE;

	// VISTAのエアロD&Dテキスト
	CString strTargetDisplayName;

	CFileItem* pf = NULL;
	int index = m_list.ItemFromPoint(point);
	if (index >= 0) {
		pf = m_stack[index];
		strTargetDisplayName = pf->m_column[0];

		if (pf->m_attr & SFGAO_DROPTARGET) {
			if (!isDragMyself() || m_list.GetSel(index) == 0) {
				eff = idl_GetDropEffect(m_hWnd, m_psf, pf->m_pidl, pdo, keyState, dropEffect);
			}
		}
		if (eff == DROPEFFECT_NONE && isDropTargetFolder()) {
			// 現在のフォルダへのＤ＆Ｄチェック
			if (!isDragMyself() || !m_list.GetSel(index)) {
				index = -1;
				eff = idl_GetDropEffect(m_hWnd, m_idlFolder, pdo, keyState, dropEffect);
			}
		}
	}
	else {
		// 現在のフォルダへのＤ＆Ｄチェック
		if (isDropTargetFolder()) {
			eff = idl_GetDropEffect(m_hWnd, m_psf, pdo, keyState, dropEffect);

			// ドロップターゲットがドロップ不可を返しても、コピー移動できる場合がある
			if (!eff && IsDropFormatsSupported(SDO_HDROP) && !m_strCurPath.IsEmpty()) {
				eff = GetDropEffect(dropEffect, keyState);
			}
		}
	}

	if (index < 0) {
		// 新しいドロップ位置がエントリに無い場合、現在のフォルダ名称をセット
		SHFILEINFO sfi;
		m_idlFolder.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
		strTargetDisplayName = sfi.szDisplayName;
	}

	if (eff == DROPEFFECT_NONE)
		index = -1;

	// ドロップ可能ファイルに変化がある場合
	if (index != m_list.GetDropTargetIndex()) {
		ShowDropHelper(FALSE);
		m_list.SetDropTargetIndex(index);
		ShowDropHelper(TRUE);
	}

	SetDropTargetDescription(pdo, strTargetDisplayName, eff);
	return eff;
}

void CFileBrowseView::OnDragLeave()
{
	// ドロップ待機状態になっているパスを元に戻す
	m_list.SetDropTargetIndex(-1);
}

DWORD CFileBrowseView::OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	int dropTarget = m_list.GetDropTargetIndex();
	if (dropTarget < 0)
		return LocalDropFile(GetSupportedDropFormats(), pdo, keyState, dropEffect, point);

	BOOL bThreaded = FALSE;
	BOOL bAsAdmin = FALSE;
	DROPEFFECT eff = DROPEFFECT_NONE;

	HDROP hdrop = NULL;
//@ 一部のFTPクライアントAPPからのD&DはSHELLURLが使われる
//@	if (IsDropFormatsSupported(SDO_HDROP|SDO_FILENAMEMAPA|SDO_FNAMEMAPW) == SDO_HDROP)
	if (IsDropFormatsSupported(SDO_HDROP|SDO_FILENAMEMAPA|SDO_FILENAMEMAPW|SDO_SHELLURL) == SDO_HDROP)
		hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);

	CFileItem* pf = m_stack[dropTarget];
	if (pf->m_pidlFq) {
		if (!hdrop || !pf->isFolder() || !pf->isFSDir() || (keyState & MK_RBUTTON)) {
			// このフォルダがファイルフォルダではないか、
			// ドロップされるデータがファイルデータではないか、
			// 右ボタンＤ＆Ｄならば
			eff = AcceptDataObject(GetTopLevelParent(), pf->m_pidlFq, pdo, keyState, dropEffect, point);
		} else {
			CString path;
			if (idl_GetPathFromIDList(pf->m_pidlFq, path) &&
				QueryDrop(m_hWnd, pf->m_column[0], dropEffect, bThreaded, bAsAdmin)) {
				AcceptFiles(hdrop, dropEffect, path, bThreaded, bAsAdmin);
				eff = dropEffect;
			}
		}
	}

	// 選択状態を元に戻す
	m_list.SetDropTargetIndex(-1);

	if (hdrop) DragFinish(hdrop);
	return eff;
}

DWORD CFileBrowseView::LocalDropFile(DWORD dropFormats, IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
{
	HWND hwnd = ::IsWindowVisible(m_hWnd) ? m_hWnd : GetParent();

	BOOL bThreaded = FALSE;
	BOOL bAsAdmin = FALSE;
	DROPEFFECT eff = DROPEFFECT_NONE;

	HDROP hdrop = NULL;
	if ((dropFormats & (SDO_HDROP | SDO_FILENAMEMAPA | SDO_FILENAMEMAPW | SDO_SHELLURL)) == SDO_HDROP)
		hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);

	if (!hdrop || !_pathfileexists(m_strCurPath) || (keyState & MK_RBUTTON)) {
		// このフォルダがファイルフォルダではないか、
		// ドロップされるデータがファイルデータではないか、
		// 右ボタンＤ＆Ｄならば
		eff = AcceptDataObject(GetTopLevelParent(), m_idlFolder, pdo, keyState, dropEffect, point);
	}
	else {
		SHFILEINFO sfi;
		m_idlFolder.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, sfi.szDisplayName);
		if (QueryDrop(hwnd, buf, dropEffect, bThreaded, bAsAdmin)) {
			AcceptFiles(hdrop, dropEffect, m_strCurPath, bThreaded, bAsAdmin);
			eff = DROPEFFECT_COPY;
		}
		if (hdrop)
			DragFinish(hdrop);
	}
	return eff;
}

void CFileBrowseView::ProcessIDListNotify(UINT action, LPCITEMIDLIST pidlTarget)
{
	switch (action) {
	case 0:									// sambaサーバ上でファイルを削除する
	case FILE_ACTION_ADDED:
	case FILE_ACTION_REMOVED:
	case FILE_ACTION_RENAMED_OLD_NAME:
	case FILE_ACTION_RENAMED_NEW_NAME:
		ResetTimer(TIMERFLAG_RETRIEVE);
		break;
	case FILE_ACTION_MODIFIED:
		if (pidlTarget) {
			CIDListPtrHandle idl(pidlTarget);
			CFileItem* pf = FindItem(idl.FindLastID());
			if (pf) {
				ModifyItem(pf);
				ResetTimer(TIMERFLAG_RETRIEVE);
			}
		}
		break;
	default:
		break;
	}
}

LRESULT CFileBrowseView::OnFileNotify(WPARAM wParam, LPARAM lParam)
{
	WATCHFOLDERINFO* pi = (WATCHFOLDERINFO*)lParam;
	if (pi) {
/* ---
switch (pi->action) {
	case FILE_ACTION_ADDED: dbg.printf(_T("add")); break;
	case FILE_ACTION_REMOVED: dbg.printf(_T("removed")); break;
	case FILE_ACTION_MODIFIED:  dbg.printf(_T("modified")); break;
	case FILE_ACTION_RENAMED_OLD_NAME:  dbg.printf(_T("ren_old")); break;
	case FILE_ACTION_RENAMED_NEW_NAME:  dbg.printf(_T("ren_new")); break;
	default:  dbg.printf(_T("(unkown=%d)"), pi->action); break;
}
dbg.printf(_T(" : %s\n"), pi->name);
---- */
		if ((wParam == m_magicNotify) || _pathisdirectory(m_strCurPath)) {
			CIDListPtr idl(pi->name);
			ProcessIDListNotify(pi->action, idl);
		}
		free(pi);
	}
	return 0;
}

LRESULT CFileBrowseView::OnIDListNotify(WPARAM wParam, LPARAM lParam)
{
	if (lParam) {
		CIDListPtr idl = (LPCITEMIDLIST)lParam;
		if (m_idlFolder.isChild(idl)) {
			ProcessIDListNotify((UINT)wParam, idl);
		}
	}
	return 0;
}

LRESULT CFileBrowseView::OnFileGrepDone(WPARAM wParam, LPARAM lParam)
{
	DWORD magic = (DWORD)lParam;
	int index = (int)wParam;
	CRect rc;
	if (m_magicGrep == magic) {
		if (m_ListStyle.isStyleReport() || m_ListStyle.isShow(ITS_SIZE | ITS_DATE)) {
			if (index < 0) {
				if (m_timerid)
					KillTimer(m_timerid);
				m_timerid = SetTimer(m_magicNotify, RETRIEVE_TIMEOUT);
				m_timerflag |= TIMERFLAG_RECALC;
			}
			else if (index < m_stack.GetSize()) {
				if (m_list.GetItemRect(index, &rc) >= 0) {
					m_list.InvalidateRect(&rc, FALSE);
				}
			}
		}
	}
	return 0;
}

LRESULT CFileBrowseView::OnColumnGrepDone(WPARAM wParam, LPARAM lParam)
{
	DWORD magic = (DWORD)lParam;
	int index = (int)wParam;
	CRect rc;
	if (m_magicGrep == magic) {
		if (m_ListStyle.isStyleReport() || m_ListStyle.isShow(ITS_SIZE | ITS_DATE)) {
			if (index < 0) {
				if (m_timerid)
					KillTimer(m_timerid);
				m_timerid = SetTimer(m_magicNotify, RETRIEVE_TIMEOUT);
				m_timerflag |= TIMERFLAG_RECALC;
			}
			else if (index < m_stack.GetSize()) {
				if (m_list.GetItemRect(index, &rc) >= 0) {
					m_list.InvalidateRect(&rc, FALSE);
				}
			}
		}
	}
	return 0;
}

LRESULT CFileBrowseView::OnDirGrepDone(WPARAM wParam, LPARAM lParam)
{
	DWORD magic = (DWORD)lParam;
	int index = (int)wParam;
	CRect rc;
	if (m_magicGrep == magic) {
		if (m_ListStyle.isStyleReport() || m_ListStyle.isShowSize()) {
			if (index < 0) {
				if (m_timerid)
					KillTimer(m_timerid);
				m_timerid = SetTimer(m_magicNotify, RETRIEVE_TIMEOUT);
				m_timerflag |= TIMERFLAG_RECALC;
			}
			else if (index < m_stack.GetSize()) {
				if (m_list.GetItemRect(index, &rc) >= 0) {
					m_list.InvalidateRect(&rc, FALSE);
				}
			}
		}
	}
	return 0;
}

LRESULT CFileBrowseView::OnImageGrepDone(WPARAM wParam, LPARAM lParam)
{
	DWORD magic = (DWORD)lParam;
	int index = (int)wParam;
	CRect rc;
	if (m_magicGrep == magic) {
		if (m_ListStyle.isStyleImage()) {
			if (0 <= index && index < m_stack.GetSize()) {
				if (m_list.GetItemRect(index, &rc) >= 0) {
					m_list.InvalidateRect(&rc, FALSE);
				}
			}
		}
	}
	return 0;
}

LRESULT CFileBrowseView::OnIconGrepDone(WPARAM wParam, LPARAM lParam)
{
	DWORD magic = (DWORD)lParam;
	int index = (int)wParam;
	CRect rc;
	if (m_magicGrep == magic) {
		if (0 <= index && index < m_stack.GetSize()) {
			if (m_list.GetItemRect(index, &rc) >= 0) {
				m_list.InvalidateRect(&rc, FALSE);
			}
		}
	}
	return 0;
}

void CFileBrowseView::RebuildHeaderColumns()
{
	bool bColumnTypeChanged = false;

	int i, numColumn = m_list.GetHeaderColumnCount();
	HDITEM hi;
	if (m_numColumns != numColumn)
		bColumnTypeChanged = true;
	else {
		TCHAR buf[MAX_PATH];
		for (i = 0; i < numColumn; i++) {
			hi.mask = HDI_TEXT;
			hi.pszText = buf;
			hi.cchTextMax = ARRAYSIZE(buf);
			m_list.GetHeaderColumn(i, &hi);
			if (StrCmp(m_sdt[i].title, hi.pszText)) {
				bColumnTypeChanged = true;
				break;
			}
		}
	}

	m_list.ResetHeaderWindowPosition();
	for (i = 0; i < m_numColumns; i++) {
		hi.mask = HDI_FORMAT | HDI_WIDTH;
		hi.fmt = m_sdt[i].fmt | HDF_STRING;
		hi.cxy = m_sdt[i].cxChar;
		if (bColumnTypeChanged) {
			hi.mask |= HDI_TEXT;
			hi.pszText = m_sdt[i].title;
			hi.cchTextMax = lstrlen(m_sdt[i].title);
		}
		if (i < numColumn)
			m_list.SetHeaderColumn(i, &hi);
		else
			m_list.InsertHeaderColumn(i, &hi);
	}

	if (i < numColumn) {
		while (m_list.DeleteHeaderColumn(i));
			;
	}
	SetHeaderBoundary(-1);
}

void CFileBrowseView::SetHeaderBoundary(int index)
{
	if (index < 0 && !m_ListStyle.isSortNone()) {
		index = m_sortColumn;
		if (index < 0)
			MapSortTypeToIndex(m_ListStyle.GetSortType());
	}
	HDITEM hd;
	int i, num = m_list.GetHeaderColumnCount();
	for (i = 0; i < num; i++) {
		hd.mask = HDI_FORMAT;
		m_list.GetHeaderColumn(i, &hd);
		bool update = false;
		if (hd.fmt & (HDF_SORTDOWN | HDF_SORTUP)) {
			// 昇・降順マークをOFF
			hd.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
			update = true;
		}
		if (i == index) {
			// 昇・降順マークをON
			hd.fmt |= (m_ListStyle.isSortDown() ? HDF_SORTDOWN : HDF_SORTUP);
			update = true;
		}
		if (update) {
			hd.mask = HDI_FORMAT;
			m_list.SetHeaderColumn(i, &hd);
		}
	}
}

CFileItem* CFileBrowseView::GetCurrentSingleFileItem()
{
	if (m_list.GetSelCount() == 1) {
		int sels;
		if (m_list.GetSelItems(1, &sels) == 1)
			return m_stack[sels];
	}
	return NULL;
}

int CFileBrowseView::PackFileItemArray(CSimpleArray<CFileItem*>& array, BOOL bListAll)
{
	int cnt = m_list.GetCount();
	if (!m_psf || cnt < 1) return 0;

	array.RemoveAll();
	CFileItem* pf;
	if (bListAll) {
		for (int i = 0; i < cnt; i++) {
			pf = m_stack[i];
			if (!pf->isUpdir())
				array.Add(pf);
		}
	}
	else {
		cnt = m_list.GetSelCount();
		if (cnt > 0) {
			CTypeAlloc<int> sels(cnt);
			cnt = m_list.GetSelItems(cnt, sels);
			if (cnt > 0) {
				for (int i = 0; i < cnt; i++) {
					pf = m_stack[sels[i]];
					if (!pf->isUpdir())
						array.Add(pf);
				}
			}
		}
	}
	return array.GetSize();
}

int CFileBrowseView::PackIDListArray(CSimpleArray<LPCITEMIDLIST>& array, BOOL bListAll, BOOL bFullyQual)
{
	CSimpleArray<CFileItem*> list;
	int cnt = PackFileItemArray(list, bListAll);
	for (int i = 0; i < cnt; i++) {
		array.Add(bFullyQual ? list[i]->m_pidlFq : list[i]->m_pidl);
	}
	return cnt;
}

int CFileBrowseView::PackShellItemArray(BOOL bListAll, IShellItemArray** ppsia)
{
	CSimpleArray<LPCITEMIDLIST> array;
	int cnt = PackIDListArray(array, bListAll, true);
	if (cnt > 0) {
		if (SHCreateShellItemArrayFromIDLists(cnt, array.GetData(), ppsia) != S_OK)
			cnt = 0;
	}
	return cnt;
}

int CFileBrowseView::GetUIObjectOf(REFIID riid, void** ppUnk, BOOL bListAll)
{
	*ppUnk = NULL;
	CSimpleArray<LPCITEMIDLIST> arr;
	int items = PackIDListArray(arr, bListAll);
	if (items > 0) {
		if (NOERROR != m_psf->GetUIObjectOf(m_hWnd, items, arr.GetData(), riid, NULL, ppUnk)) {
			*ppUnk = NULL;
			items = 0;
		}
	}

	// フォルダで“何かしようとした”ので、このフォルダを記録
	gpMainFrame->RecordAccessFolder(m_idlFolder);

	return items;
}

SFGAOF CFileBrowseView::GetAttributesOf(SFGAOF attr, BOOL bListAll)
{
	SFGAOF attr_sup = 0;
	CSimpleArray<LPCITEMIDLIST> arr;
	int items = PackIDListArray(arr, bListAll);
	if (items > 0) {
		attr_sup = attr;
		if (m_psf->GetAttributesOf(items, arr.GetData(), &attr_sup) != NOERROR)
			attr_sup = 0;
		else
			attr_sup &= attr;
	}
	return attr_sup;
}

HDROP CFileBrowseView::MakeDropFiles(BOOL bListAll)
{
	HDROP hdrop = NULL;
	CComPtr<IDataObject> pdo;
	if (GetUIObjectOf(IID_PPV_ARGS(&pdo), bListAll) > 0) {
		hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);
	}
	return hdrop;
}

int CFileBrowseView::MakeDropFiles(TDropFiles& df, BOOL bListAll)
{
	HDROP hdrop = MakeDropFiles(bListAll);
	if (hdrop) {
		df.Attach(hdrop);
		return df.GetCount();
	}
	return 0;
}

void CFileBrowseView::OnFinderRange(UINT idCmd, UINT code, HWND /*hwnd*/)
{
	bool bRegExp = ((gnFopStat & FOS_REXFINDER) != 0);
	CString filter;
	int cchMax = MAX_PATH * 2;
	m_finder.GetText(filter.GetBuffer(cchMax), cchMax);
	filter.ReleaseBuffer();

	if (idCmd == IDM_FINDER_SHOW)
		ToggleFindBar();
	else if (idCmd == IDM_FINDER_REGEXP) {
		if (bRegExp)
			gnFopStat &= ~FOS_REXFINDER;
		else
			gnFopStat |= FOS_REXFINDER;
		bRegExp ^= true;
		fvApp.SaveStates();
		MarkFiles(filter, false, bRegExp, false);
	}
	else {
		if (filter.GetLength() > 0) {
			int index = m_list.GetCaretIndex();
			switch (idCmd) {
			case IDM_FINDER_ENTER:
				ToggleFindBar();
				DoFileOpen();
				break;
			case IDM_FINDER_SELECT_ENTER:
				ToggleFindBar();
				SelectFiles(filter, false, bRegExp, false);
				DoFileOpen();
				break;
			case IDM_FINDER_NEXT:
				MarkFiles(filter, false, bRegExp, false);
				if (code != EN_CHANGE)
					index++;
				index = FindNextMatchSpec(index, filter, bRegExp);
				if (index >= 0)
					m_list.SetCurSel(index, FALSE);
				else
					MessageBeep(0);
				break;
			case IDM_FINDER_PREV:
				MarkFiles(filter, false, bRegExp, false);
				if (code != EN_CHANGE)
					index--;
				index = FindPrevMatchSpec(index, filter, bRegExp);
				if (index >= 0)
					m_list.SetCurSel(index, FALSE);
				else
					MessageBeep(0);
				break;
			case IDM_FINDER_SELECT:
				ToggleFindBar();
				SelectFiles(filter, false, bRegExp, false);
				break;
			case IDM_FINDER_FILTER:
				{
				ToggleFindBar();
				CFilterSetting fs;
				fs.Set(NULL, filter, false, bRegExp, false);
				gpMainFrame->SetFileFilter(&fs);
				break;
				}
			}
		}
		else {
			if (m_list.GetMarkCount() > 0)
				UnmarkFiles();
			MessageBeep(0);
		}
	}
}

void CFileBrowseView::ToggleFindBar()
{
	if (m_finder.IsWindow()) {
		if (m_finder.IsWindowVisible()) {
			m_finder.ShowWindow(SW_HIDE);
			UnmarkFiles();
			m_list.SetFocus();
		}
		else {
			m_finder.ShowWindow(SW_SHOWNORMAL);
			TCHAR filter[MAX_PATH * 2];
			if (m_finder.GetText(filter, ARRAYSIZE(filter)) > 0) {
				bool bRegExp = ((gnFopStat & FOS_REXFINDER) != 0);
				MarkFiles(filter, false, bRegExp, false);
			}
			m_finder.SetFocus();
		}
		CRect rc;
		GetClientRect(&rc);
		OnSize(SIZE_RESTORED, rc.Width(), rc.Height());
	}
}

void CFileBrowseView::OnExecHotKeyRange(UINT idCmd, UINT /*code*/, HWND /*hwnd*/)
{
	CHotKeyItem item;
	if (fvApp.menuHotKey.GetCommandLine(idCmd, item))
		ExecMacro(item);
}

void CFileBrowseView::ExecMacro(const CHotKeyItem& item)
{
	CStringZ exe = item.m_exec;
	exe.Trim(_T("\" "));

	CFileItem* pf;
	int i, sel;
	CStringZ cmd, tmp;
	CString param = item.m_param.GetString();
	LPTSTR q, p = (LPTSTR)param.GetString();
	while (p) {
		q = StrChr(p, _T('$'));
		if (q)
			*q++ = 0;
		cmd += p;
		if (!q)
			break;

		switch (ToUpper(*q)) {
			case _T('P'):
				tmp = m_strCurPath;
				tmp.AddBackslash();
				cmd += tmp;
				break;

			case _T('F'):
				if ((sel = m_list.GetSelCount()) != 1) {
					AppMessage(IDS_SELECTSINGLEFILE);
					return;
				}
				m_list.GetSelItems(1, &sel);
				pf = m_stack[sel];

				cmd += pf->m_column[0];
				break;

			case _T('M'):
			{
				int cnt = m_list.GetSelCount();
				if (cnt < 1) break;

				CTypeAlloc<int> sels(cnt);
				cnt = m_list.GetSelItems(cnt, sels);
				for (i = 0; i < cnt; i++) {
					pf = m_stack[sels[i]];
					if (pf->m_column[0].Find(_T(' ')) >= 0)
						cmd.AppendFormat(_T("\"%s\" "), pf->m_column[0].GetString());
					else
						cmd.AppendFormat(_T("%s "), pf->m_column[0].GetString());
				}
				break;
			}

			case _T('E'):
				if ((sel = m_list.GetSelCount()) != 1) {
					AppMessage(IDS_SELECTSINGLEFILE);
					return;
				}
				m_list.GetSelItems(1, &sel);
				pf = m_stack[sel];
//				i = pf->m_column[0].ReverseFind(_T('.'));
//				if (i >= 0)
//					cmd += pf->m_column[0].Mid(i + 1);
				cmd += CStringZ::FileExtension(pf->m_column[0]);
				break;

			case _T('N'):
				if ((sel = m_list.GetSelCount()) != 1) {
					AppMessage(IDS_SELECTSINGLEFILE);
					return;
				}
				m_list.GetSelItems(1, &sel);
				pf = m_stack[sel];
//				tmp = pf->m_column[0];
//				i = tmp.ReverseFind(_T('.'));
//				if (i >= 0)
//					tmp = tmp.Left(i);
//				cmd += tmp;
				cmd += CStringZ::FileBody(pf->m_column[0]);
				break;

			default:
				break;
		}
		p = q + 1;
	}

	// 作業フォルダ
	CString dir;
	if (item.m_workdir.IsEmpty())
		dir = m_strCurPath;
	else {
		tmp.Empty();
		dir = item.m_workdir.GetString();
		p = (LPTSTR)dir.GetString();
		while (p) {
			q = StrChr(p, _T('$'));
			if (q)
				*q++ = 0;
			tmp += p;
			if (!q)
				break;

			switch (ToUpper(*q)) {
			case _T('P'):
				tmp += m_strCurPath;
				tmp.AddBackslash();
				break;
			}
			p = q + 1;
		}
		dir = tmp;
	}

	// マクロ指定
	if (exe[0] == _T('%')) {
		exe.Trim(_T('%'));
		if (!cmd.IsEmpty()) {
			if (!exe.CompareNoCase(_T("sort"))) {
				int seq = SORT_UP;
				int col = m_sortColumn;
				CSimpleArray<CStringZ> a = cmd.split(_T(","));
				if (a.GetSize() > 0) {
					if      (!a[0].cmpi(L"name"))		col = MapSortTypeToIndex(SORT_NAME);
					else if (!a[0].cmpi(L"ext"))		col = MapSortTypeToIndex(SORT_EXT);
					else if (!a[0].cmpi(L"size"))		col = MapSortTypeToIndex(SORT_SIZE);
					else if (!a[0].cmpi(L"time"))		col = MapSortTypeToIndex(SORT_TIME);
					else if (!a[0].cmpi(L"attr"))		col = MapSortTypeToIndex(SORT_ATTR);
					else if (!a[0].cmpi(L"dos"))		col = MapSortTypeToIndex(SORT_DOS);
					if (col < 0)								col = m_sortColumn;
					else if (!a[0].cmpi(L"none"))		col = -1;

					if (a.GetSize() > 1) {
						if (!a[1].cmpi(L"up"))			seq = SORT_UP;
						else if (!a[1].cmpi(L"down"))	seq = SORT_DOWN;
					}
				}
				SortItems(col, seq);
			} else if (!exe.cmpi(_T("filter"))) {
				CFilterSetting* wildcard = m_pDocument->GetFilter();
				ATLASSERT(wildcard != NULL);

				wildcard->Set(NULL, cmd, FALSE, FALSE, FALSE);
				ShellBrowse(NULL);
			} else if (!exe.cmpi(_T("select"))) {
				SelectFiles(cmd);
			} else if (!exe.cmpi(_T("selectregexp"))) {
				SelectFiles(cmd, false, true, false);
			} else if (!exe.cmpi(_T("jump"))) {
				tmp = cmd;
				tmp.Trim(_T("\" "));
				CIDListPtr idl;
				if (idl.CreateFromPath(tmp.GetString())) {
					CFolderTreeWindow tree = gpMainFrame->GetFolderTree();
					ATLASSERT(tree.IsWindow());
					fvApp.OpenFolder(idl);
				}
			} else if (!exe.cmpi(gszTabGroup + 1)) {
				// TabGroupファイル名を取得
				i = cmd.ReverseFind(_T('.'));
				if (i < 0 || !cmd.Mid(i).CompareNoCase(gszTabGroup))
					cmd += gszTabGroup;
				CString str;
				GetAppDataFolder(str, cmd);

				if (!_pathfileexists(str))
					AppMessage(IDS_FILE_NOTFOUND, cmd);
				else {
					gpMainFrame->TabGroupLoad(str);
				}
			} else if (!exe.cmpni(_T("new"), 3)) {
				// 新しいTab or Window
				exe.Delete(0, 3);

				tmp = cmd;
				tmp.Trim(_T("\" "));
				if (_pathisdirectory(tmp)) {
					if (!exe.cmpi(_T("tab")))
						fvApp.OpenTabFolder(tmp, TRUE);
					else if (!exe.cmpi(_T("window")) || !exe.cmpi(_T("frame")))
						ExecNewFrame(tmp, _T("/force"), item.m_bAsAdmin);
				}
			}
		}
		return;
	}

	// コマンドライン編集
	if (item.m_bEdit) {
		CExecuteInfo ei;
		ei.m_strCommand = cmd;

		CExecDlg dlg;
		dlg.m_pei = &ei;
		if (dlg.DoModal() != IDOK)
			return;
		cmd = ei.m_strCommand;
	}

	SHELLEXECUTEINFO se = {0};
	se.cbSize = sizeof(se);
	se.fMask = 0;
	se.hwnd = GetTopLevelParent();
	se.lpVerb = item.m_bAsAdmin ? _T("runas") : NULL;
	se.lpFile = exe;
	se.lpParameters = cmd;
	se.lpDirectory = dir;
	se.nShow = SW_SHOWNORMAL;
	if (!ShellExecuteEx(&se)) {
		DWORD code = GetLastError();
		lccErrorBox(GetTopLevelParent(), code, APP_PRODUCT, exe, MB_STOP);
	}
}

LRESULT CFileBrowseView::OnHandlerGetListStyle(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*((ICONLISTSTYLE*)lParam) = *m_ListStyle;
	return (LRESULT)m_sortColumn;
}

LRESULT CFileBrowseView::OnHandlerSetListStyle(WPARAM wParam, LPARAM lParam)
{
	if (lParam) {
		CIconListStyle old = m_ListStyle;
		*m_ListStyle = *((ICONLISTSTYLE*)lParam);

		if ((old.style != m_ListStyle.style) || (old.enlarge != m_ListStyle.enlarge))
			UpdateView(DCM_VIEWSTYLE);
		else if ((old.status != m_ListStyle.status) || (old.size_thumb.cx != m_ListStyle.size_thumb.cx))
			UpdateView(DCM_CHANGEFORMAT);

		gpMainFrame->BrowserListStyleChanged(&m_ListStyle);
		return TRUE;
	}
	return FALSE;
}

void CFileBrowseView::SaveSelection()
{
	m_arrSaveSelection.RemoveAll();
	int cnt = m_list.GetSelCount();
	if (cnt > 0) {
		int* sels = new int[cnt];
		cnt = m_list.GetSelItems(cnt, sels);
		for (int i = 0; i < cnt; i++) {
			m_arrSaveSelection.Add(sels[i]);
		}
	}
	m_arrSaveSelection.GetSize();
}

void CFileBrowseView::RestoreSelection()
{
	int cnt = m_arrSaveSelection.GetSize();
	if (cnt > 0) {
		// 一旦全選択または全解除する
		m_list.SetRedraw(FALSE);
		m_list.SetSel(-1, FALSE);
		for (int i = 0; i < cnt; i++) {
			m_list.SetSel(m_arrSaveSelection[i], TRUE);
		}
		m_list.SetRedraw(TRUE);
		m_list.Invalidate(FALSE);
	}
	m_arrSaveSelection.RemoveAll();
}

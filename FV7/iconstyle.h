////////////////////////////////////////////////////////////////////////
// iconstyle.h

#pragma once
#ifndef __ICONSTYLE_H__
#define __ICONSTYLE_H__

////////////////////////////////////////////////////////////////////////
// ICON DISPLAY MODE DEFINITION
//
enum IconType {
	ICON_NORMAL = 0,				// 通常
	ICON_OUTLINE,					// 概要
	ICON_REPORT,					// 詳細表示
	ICON_IMAGE,						// イメージ表示
	ICON_ICON						// アイコン表示
};

#define ICON_NORMAL_LARGE			0x00000001
#define ICON_REPORT_LARGE			0x00000002
#define ICON_OUTLINE_LARGE			0x00000004
#define ICON_ICON_LARGE				0x00000008

// アイコンリストスタイル
#define ITS_DATE					0x00000001		// 日付時刻を表示
#define ITS_SIZE					0x00000002		// サイズを表示
#define ITS_ATTR					0x00000004		// 属性を表示
#define ITS_ELLP					0x00000008		// 長い名前を省略して表示


////////////////////////////////////////////////////////////////////////
// SORT MODE DEFINITION
//

enum SortType {
	SORT_NONE = -1,
	SORT_NAME = 0,
	SORT_SIZE,
	SORT_TIME,
	SORT_EXT,
	SORT_ATTR,
	SORT_DOS,
	SORT_PATH,
	SORT_COL7,
	SORT_UNK,
};

enum SortSeq {
	SORT_UP,
	SORT_DOWN
};


////////////////////////////////////////////////////////////////////////
//

typedef struct tagICONLISTSTYLE {
	DWORD status;
	int style;
	int enlarge;
	int sort_type;
	int sort_seq;
	SIZE size_thumb;
} ICONLISTSTYLE, *PICONLISTSTYLE;

class CIconListStyle : public tagICONLISTSTYLE
{
public:
	CIconListStyle()
	{
		status = (ITS_ELLP << (ICON_REPORT << 2)) | (ITS_ELLP << (ICON_NORMAL << 2));
		style = ICON_REPORT;
		enlarge = ICON_OUTLINE_LARGE | ICON_ICON_LARGE;
		sort_type = SORT_NAME;
		sort_seq = SORT_UP;
		size_thumb.cx = 128;
		size_thumb.cy = 128;
	}
	CIconListStyle(const ICONLISTSTYLE& src)
	{
		Copy(&src);
	}
	CIconListStyle(const ICONLISTSTYLE* src)
	{
		ATLASSERT(src != NULL);
		Copy(src);
	}
	void Copy(const ICONLISTSTYLE* src)
	{
		if (src != this) {
			status = src->status;
			style = src->style;
			enlarge = src->enlarge;
			sort_type = src->sort_type;
			sort_seq = src->sort_seq;
			size_thumb = src->size_thumb;
		}
	}
	CIconListStyle& operator =(const ICONLISTSTYLE& src)
	{
		Copy(&src);
		return *this;
	}
	bool operator ==(const ICONLISTSTYLE& src) const
	{
		return ((status == src.status) &&
				(style == src.style) &&
				(enlarge == src.enlarge) &&
				(sort_type == src.sort_type) &&
				(sort_seq == src.sort_seq) &&
				(size_thumb.cx == src.size_thumb.cx));
	}

	inline operator PICONLISTSTYLE() { return this; }

	inline int GetStyle() const { return style; }
	inline void SetStyle(int newStyle) { style = newStyle; }
	inline bool isStyle(int compStyle) const { return (style == compStyle); }

	inline bool isStyleNormal() const { return (style == ICON_NORMAL); }
	inline bool isStyleOutline() const { return (style == ICON_OUTLINE); }
	inline bool isStyleReport() const { return (style == ICON_REPORT); }
	inline bool isStyleImage() const { return (style == ICON_IMAGE); }
	inline bool isStyleIcon() const { return (style == ICON_ICON); }

	inline DWORD GetStatus() const { return status; }
	inline void SetStatus(DWORD s) { status = s; }

	inline bool isShowDate() const { return ((status & (ITS_DATE << (style << 2))) != 0); }
	inline bool isShowSize() const { return ((status & (ITS_SIZE << (style << 2))) != 0); }
	inline bool isShowAttr() const { return ((status & (ITS_ATTR << (style << 2))) != 0); }
	inline bool isShowEllp() const { return ((status & (ITS_ELLP << (style << 2))) != 0); }
	inline bool isShow(UINT flg) const { return ((status & (flg << (style << 2))) != 0); }
	inline void ClearDate() { status &= ~(ITS_DATE << (style << 2)); }
	inline void ClearSize() { status &= ~(ITS_SIZE << (style << 2)); }
	inline void ClearAttr() { status &= ~(ITS_ATTR << (style << 2)); }
	inline void ClearEllp() { status &= ~(ITS_ELLP << (style << 2)); }
	inline void SetDate() { status |= (ITS_DATE << (style << 2)); }
	inline void SetSize() { status |= (ITS_SIZE << (style << 2)); }
	inline void SetAttr() { status |= (ITS_ATTR << (style << 2)); }
	inline void SetEllp() { status |= (ITS_ELLP << (style << 2)); }
	inline void ToggleDate() { status ^= (ITS_DATE << (style << 2)); }
	inline void ToggleSize() { status ^= (ITS_SIZE << (style << 2)); }
	inline void ToggleAttr() { status ^= (ITS_ATTR << (style << 2)); }
	inline void ToggleEllp() { status ^= (ITS_ELLP << (style << 2)); }

	inline int GetEnlarge() const { return enlarge; }
	inline void SetEnlarge(int flags) { enlarge = flags; }

	inline bool isEnlargeNormal() const { return ((enlarge & ICON_NORMAL_LARGE) != 0); }
	inline bool isEnlargeReport() const { return ((enlarge & ICON_REPORT_LARGE) != 0); }
	inline bool isEnlargeOutline() const { return ((enlarge & ICON_OUTLINE_LARGE) != 0); }
	inline bool isEnlargeIcon() const { return ((enlarge & ICON_ICON_LARGE) != 0); }
	inline void SetEnlargeNormal() { enlarge |= ICON_NORMAL_LARGE; }
	inline void SetEnlargeReport() { enlarge |= ICON_REPORT_LARGE; }
	inline void SetEnlargeOutline() { enlarge |= ICON_OUTLINE_LARGE; }
	inline void SetEnlargeIcon() { enlarge |= ICON_ICON_LARGE; }
	inline void ResetEnlargeNormal() { enlarge &= ~ICON_NORMAL_LARGE; }
	inline void ResetEnlargeReport() { enlarge &= ~ICON_REPORT_LARGE; }
	inline void ResetEnlargeOutline() { enlarge &= ~ICON_OUTLINE_LARGE; }
	inline void ResetEnlargeIcon() { enlarge &= ~ICON_ICON_LARGE; }
	inline void ToggleEnlargeNormal() { enlarge = TOGGLEFLAG(enlarge, ICON_NORMAL_LARGE); }
	inline void ToggleEnlargeReport() { enlarge = TOGGLEFLAG(enlarge, ICON_REPORT_LARGE); }
	inline void ToggleEnlargeOutline() { enlarge = TOGGLEFLAG(enlarge, ICON_OUTLINE_LARGE); }
	inline void ToggleEnlargeIcon() { enlarge = TOGGLEFLAG(enlarge, ICON_ICON_LARGE); }

	inline bool isSortUp() const { return (sort_seq == SORT_UP); }
	inline bool isSortDown() const { return (sort_seq == SORT_DOWN); }
	inline bool isSortNone() const { return (sort_type == SORT_NONE); }
	inline bool isSortName() const { return (sort_type == SORT_NAME); }
	inline bool isSortSize() const { return (sort_type == SORT_SIZE); }
	inline bool isSortTime() const { return (sort_type == SORT_TIME); }
	inline bool isSortExt() const { return (sort_type == SORT_EXT); }
	inline bool isSortAttr() const { return (sort_type == SORT_ATTR); }
	inline bool isSortDos() const { return (sort_type == SORT_DOS); }
	inline bool isSortPath() const { return (sort_type == SORT_PATH); }
	inline bool isSortCol7() const { return (sort_type == SORT_COL7); }
	inline bool isSortUnk() const { return (sort_type == SORT_UNK); }
	inline void SetSortType(int type) { sort_type = type; }
	inline void SetSortSeq(int seq) { sort_seq = seq; }
	inline int GetSortType() const { return sort_type; }
	inline int GetSortSeq() const { return sort_seq; }

	inline void SetThumbSize(const SIZE size) { size_thumb = size; }
	inline void GetThumbSize(SIZE* psize) const { *psize = size_thumb; }

	void toString(CString& str) const
	{
		str.Empty();
		if      (isStyleNormal())	str = _T("一覧");
		else if (isStyleOutline())	str = _T("概要");
		else if (isStyleReport())	str = _T("詳細");
		else if (isStyleIcon())		str = _T("アイコン");
		else if (isStyleImage())	str = _T("イメージ");

		if (!isStyleOutline() && !isStyleReport()) {
			if (isShowDate())		str += _T("+日付");
			if (isShowSize())		str += _T("+サイズ");
			if (!isStyleImage() && isShowAttr()) str += _T("+属性");
		}
		str += _T(", ");

		if          (isSortNone())	str += _T("ソートなし");
		else {
			if      (isSortName())	str += _T("名前でソート");
			else if (isSortSize())	str += _T("サイズでソート");
			else if (isSortTime())	str += _T("時刻でソート");
			else if (isSortExt())	str += _T("拡張子でソート");
			else if (isSortAttr())	str += _T("属性でソート");
			else if (isSortDos())	str += _T("DOS名でソート");
			else if (isSortPath())	str += _T("場所でソート");
			else if (isSortCol7())	str += _T("カラム7でソート");
			else if (isSortUnk())	str += _T("（不明）でソート");
			str += (isSortDown() ? _T("(↓)") : _T("(↑)"));
		}

		if ((isStyleNormal() || isStyleReport()) && isShowEllp())
			str += _T(", 短縮名");
	}
};

#endif			// __ICONSTYLE_H__

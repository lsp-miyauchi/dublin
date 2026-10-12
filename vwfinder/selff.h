// selff.h : header file
//

#pragma once
#ifndef __SELFF_H__
#define __SELFF_H__

#include "fileitem.h"

// #define SELFF_MAGIC				0x11280001
#define SELFF_MAGIC				0x11280004

#pragma pack(1)
typedef struct tagSEARCHPARAM {
	DWORD magic;
	INT32 nSearchType;			// 全て／変更／作成
	INT32 nDateType;			// 日付の指定（範囲、過去ｘヶ月、過去ｘ日）
	FILETIME ftStart;			// 開始日付
	FILETIME ftEnd;				// 終了日付
	INT32 nLimitDate;			// 月日数
	INT32 nSizeType;			// サイズ範囲で検索
	INT32 nLimitSize;			// サイズ
	DWORD attr;					// 属性
	BOOL bFilterExclude;		// 対象フィルタに一致しないもの
	BOOL bRegExp;				// 正規表現で検索
	BOOL bCaseSense;			// 大文字小文字の区別
} SEARCHPARAM;
#pragma pack()

typedef       SEARCHPARAM* LPSEARCHPARAM;
typedef const SEARCHPARAM* LPCSEARCHPARAM;

class CSearchParam : public SEARCHPARAM
{
public:
	CSearchParam()
	{
		magic = SELFF_MAGIC;
		nSearchType = 0;
		nDateType = 0;
		nLimitDate = 0;
		nSizeType = 0;
		nLimitSize = 0;
		attr = 0;
		bFilterExclude = FALSE;
		bRegExp = FALSE;
		bCaseSense = FALSE;

		nTargetMode = 0;
		strStartPath.Empty();
		strSearchMask.Empty();
	}
	__inline CSearchParam(const SEARCHPARAM& srcData) { *this = srcData; }

	__inline operator LPSEARCHPARAM() { return this; }
	__inline operator LPCSEARCHPARAM() const { return this; }
	__inline void operator=(const SEARCHPARAM& srcData) { memcpy(this, &srcData, sizeof(SEARCHPARAM)); }

	bool MatchFileSize(DWORDLONG dstsize)
	{
		DWORDLONG srcsize = UInt32x32To64(nLimitSize, 1024);
		if (nSizeType == 1) {			// 最低サイズ
			if ((attr & FILE_ATTRIBUTE_DIRECTORY) || (dstsize < srcsize))
				return false;
		}
		else if (nSizeType == 2) {		// 最高サイズ
			if ((attr & FILE_ATTRIBUTE_DIRECTORY) || (dstsize > srcsize))
				return false;
		}
		return true;
	}

	bool MatchFileTime(FILETIME* pftCreation, FILETIME* pftLastWrite)
	{
		if (nSearchType == 1) {			// 変更されたファイル
			if (CompareFileTime(&ftStart, pftLastWrite) > 0 ||
				CompareFileTime(&ftEnd, pftLastWrite) < 0)
				return false;
		}
		else if (nSearchType == 2) {	// 作成されたファイル
			if (CompareFileTime(&ftStart, pftCreation) > 0 ||
				CompareFileTime(&ftEnd, pftCreation) < 0)
				return false;
		}
		return true;
	}

	bool MatchFileAttributes(DWORD dstattr)
	{
		return (!attr || (dstattr & attr)) ? true : false;
	}

	void Save(IStream* pstrm)
	{
		ULONG len;
		pstrm->Write((LPSEARCHPARAM)this, sizeof(SEARCHPARAM), &len);

		pstrm->Write(&nTargetMode, sizeof(nTargetMode), &len);
		WriteStringToStream(pstrm, strStartPath);
		WriteStringToStream(pstrm, strSearchMask);
	}

	BOOL Load(IStream* pstrm)
	{
		ULONG len;
		SEARCHPARAM sp;
		if (pstrm->Read(&sp, sizeof(sp), &len) == S_OK &&
			len == sizeof(sp) &&
			sp.magic == SELFF_MAGIC) {
			*this = sp;

			pstrm->Read(&nTargetMode, sizeof(nTargetMode), &len);
			ReadStringFromStream(pstrm, strStartPath);
			ReadStringFromStream(pstrm, strSearchMask);
			return TRUE;
		}
		return FALSE;
	}

	INT32 nTargetMode;					// 0 = single path, 1 = multi drive
	CString strStartPath;
	CString strSearchMask;
};

/////////////////////////////////////////////////////////////////////////////
// CSelFFDlg dialog

class CSelFFDlg : public CDialogImpl<CSelFFDlg>
{
public:
	CSelFFDlg();
	~CSelFFDlg();

	enum { IDD = IDD_SEARCHSETTINGS };

	bool m_fSearchRetry;
	CTabCtrl m_tab;
	CListViewCtrl m_list;
	CFolderTreeCtrl m_tree;
	CSpinButtonCtrl m_spinSize;
	CSpinButtonCtrl m_spinPast;
	CComboBox m_comboPast;
	CSimpleArray<CSearchParam> m_history;
	CComboBox m_comboFilter;
	CComboBox m_comboFindmode;
	CComboBox m_comboSizetype;
	CDateTimePickerCtrl m_dtFrom;
	CDateTimePickerCtrl m_dtTo;

	CSearchParam* m_psp;
	HIMAGELIST m_hImage;

BEGIN_MSG_MAP(CSelFFDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelp)
	ON_BN_CLICKED(IDC_DATERANGE, OnDateRange)
	ON_CBN_SELCHANGE(IDC_COMBO, OnHistoryChange)
	ON_CBN_SELCHANGE(IDC_FILETYPE, OnFindType)
	ON_BN_CLICKED(IDC_PAST_DATE, OnDateRange)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_NOTIFY(TCN_SELCHANGE, IDC_FOLDER, OnNotifyTabChange)
	ON_NOTIFY(FTWN_SELCHANGED, IDC_TREE, OnNotifyTreeChanged)
	ON_NOTIFY(FTWN_QUERYHIDDEN, IDC_TREE, OnNotifyTreeQueryHidden)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelp();
	void OnDateRange();
	void OnHistoryChange();
	void OnFindType();
	void OnClear();
	inline void OnNotifyTabChange(NMHDR* pNMHDR, LRESULT* pResult) { SelectTab(); *pResult = 0; }
	void OnNotifyTreeChanged(NMHDR* phdr, LRESULT* pResult);
	inline void OnNotifyTreeQueryHidden(NMHDR* phdr, LRESULT* pResult) { *pResult = FTH_SYSHIDDEN; }
	void SelectTab();
	void ApplyCondition();

	void LoadHistory();
	void SaveHistory();
};

/////////////////////////////////////////////////////////////////////////////
// CDirWalker

class CDirWalker
{
public:
	CDirWalker()
		: m_nFound(0)
		, m_bCanceled(FALSE)
		, m_progress(NULL)
	{ }

	~CDirWalker() { ResetItems(); }

	void Initialize(HWND hOwner, CSearchParam* psp);
	void ResetItems();
	void MoveItemTo(CAtlArray<CFileInfo*>& moveto, size_t iStart);
	BOOL SearchTree(LPCTSTR pszStartPath);
	void Start();
	int GetItemCount() { return (int)m_array.GetCount(); }
	BOOL IsCanceled() const { return m_bCanceled; }

	HWND m_hOwner;
	int m_nFound;
	CAtlArray<CFileInfo*> m_array;
	CSearchParam* m_psp;
	int m_serial;
	IProgressDialog* m_progress;
	BOOL m_bCanceled;
};


#endif		// __SELFF_H__

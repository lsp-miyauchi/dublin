// mapico.h : ヘッダー ファイル
//

#pragma once
#ifndef __MAPICON_H__
#define __MAPICON_H__

/////////////////////////////////////////////////////////////////////////////
//

class CIconMap
{
public:
	CString m_ext;
	CString m_file;
	int m_index;

public:
	CIconMap() : m_index(-1) { }
	CIconMap(LPCTSTR ext, LPCTSTR file = NULL, int index = -1) { Initialize(ext, file, index); }
	CIconMap(const CIconMap& src) { copyfrom(src); }
	CIconMap& operator =(const CIconMap& src) { copyfrom(src); return *this; }
	void Initialize(LPCTSTR ext, LPCTSTR file, int index)
	{
		m_ext = ext;
		m_ext.TrimLeft(_T('.'));
		m_file = file;
		m_index = index;
	}
	inline LPCTSTR GetExtension() { return (m_ext.IsEmpty() ? NULL : m_ext.GetString()); }
	inline LPCTSTR GetIconPath() { return (m_file.IsEmpty() ? NULL : m_file.GetString()); }
	inline int GetIconIndex() { return m_index; }
	void SetIconPath(LPCTSTR path, int index = -1)
	{
		m_file = path;
		if (index >=0)
			m_index = index;
	}
	inline void SetIconIndex(int index) { m_index = index; }

private:
	CIconMap& copyfrom(const CIconMap& src)
	{
		if (this != &src) {
			m_ext = src.m_ext.GetString();
			m_file = src.m_file.GetString();
			m_index = src.m_index;
		}
		return (*this);
	}
};

void BuildMapIconFileName(CString& str);
bool SaveMapIcon(const CSimpleArray<CIconMap*>& array, LPCTSTR pszFileName);
int LoadMapIcon(CSimpleArray<CIconMap*>& array, LPCTSTR pszFileName);
int RestoreMapIcon(CSimpleArray<CIconMap*>& array);

/////////////////////////////////////////////////////////////////////////////
//

class CHIconArray : public CSimpleArray<HICON>
{
public:
	~CHIconArray() { DeleteAll(); }
	void DeleteAll()
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			HICON hic = (*this)[i];
			if (hic != NULL)
				DestroyIcon(hic);
		}
		RemoveAll();
	}
	void SetSize(int cnt)
	{
		DeleteAll();
		while (cnt > 0) {
			Add((HICON)NULL);
			cnt--;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CSelectIconDlg ダイアログ

class CSelectIconDlg : public CDialogImpl<CSelectIconDlg>, public CDialogResize<CSelectIconDlg>
{
public:
	CSelectIconDlg() { }

	enum { IDD = IDD_SELECTICON };

	CWindow m_list;
	CIconMap m_im;
	CHIconArray m_iconSmall;
	CHIconArray m_iconLarge;

BEGIN_MSG_MAP(CSelectIconDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CSelectIconDlg>)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_CHECK, OnCheck)
	ON_EN_KILLFOCUS(IDC_EDIT, OnUpdatePath)
	ON_NOTIFY(SEN_GETTEXT, IDC_EDIT, OnNotifyBrowse)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CSelectIconDlg)
	DLGRESIZE_CONTROL(IDC_EDIT,			DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_CHECK,		DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnDrawItem(int id, DRAWITEMSTRUCT* pdi);
	void OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnCheck();
	void OnHelpCall();
	void OnUpdatePath();

	void IconStamp();
};


#endif		// __MAPICON_H__

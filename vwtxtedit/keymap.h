// keymap.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyPage ダイアログ

class CKeyPage : public CPropertyPageImpl<CKeyPage>
{
public:
	enum { IDD = IDD_KEYMAP };
	HIMAGELIST m_himChk;
	HIMAGELIST m_himKey;
	CListBox m_keylist;
	CListBox m_cmdlist;
	BOOL m_bUpdate;
	BOOL m_bUseCtrl;
	BOOL m_bShift;
	BOOL m_bCtrl;
	int m_numkeys;
	UINT m_cmds[26 * 3];
	CSize m_sizeImage;
	CSize m_sizeBarButton;
	CSimpleArray<UINT> m_arrToolbarCommand;
	HIMAGELIST m_himTool;

	CKeyPage();
	~CKeyPage();

	BEGIN_MSG_MAP(CKeyPage)
		ON_WM_INITDIALOG()
		ON_WM_MEASUREITEM()
		ON_WM_DRAWITEM()
		ON_LBN_SELCHANGE(IDC_CMDLIST, OnSelChangeCmdList)
		ON_LBN_SELCHANGE(IDC_KEYLIST, OnSelChangeKeyList)
		ON_BN_CLICKED(IDC_USECTRL, OnCtrlCheck)
		ON_BN_CLICKED(IDC_USESHIFT, OnShiftCheck)
		CHAIN_MSG_MAP(CPropertyPageImpl<CKeyPage>)
	END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	void OnSelChangeCmdList();
	void OnSelChangeKeyList();
	void OnCtrlCheck();
	void OnShiftCheck();

	BOOL OnApply();
	UINT GetCommand(int nIndex);
	void SelectCommand();
};

/////////////////////////////////////////////////////////////////////////////
// CKeySheet

namespace WTL {


// for non-customized sheets
class CKeySheet : public CPropertySheetImpl<CKeySheet>
{
private:
	bool m_bInitCompleted;
	HIMAGELIST m_hImage;

public:
	CKeySheet(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheetImpl<CKeySheet>(title, uStartPage, hWndParent)
	{ m_bInitCompleted = FALSE; }

	BEGIN_MSG_MAP(CKeySheet)
		MESSAGE_HANDLER(WM_COMMAND, CPropertySheetImpl<CKeySheet>::OnCommand)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		CHAIN_MSG_MAP(CPropertySheetImpl<CKeySheet>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Centre?
		if (wParam == TRUE) Init();
		// Done
		bHandled = FALSE;
		return 0;
	}

	void Init(void)
	{
		// Only do this once
		if (!m_bInitCompleted) {
			CenterWindow();

			HINSTANCE hInst = RESINST;
			m_hImage = lccLoadImageList(hInst, RT_RCDATA, MAKEINTRESOURCE(IDB_KEYMAP), 14, RGB(255, 0, 255));
			CTabCtrl tab = GetTabControl();
			ATLASSERT(tab.IsWindow());
			tab.SetImageList(m_hImage);

			TCHAR buf[MAX_PATH];
			LoadString(hInst, IDS_FOLDER_KEYMAP, buf, ARRAYSIZE(buf));
			TCHAR* p = buf;
			int index = 0;
			TC_ITEM tci;
			tci.mask = TCIF_TEXT | TCIF_IMAGE;
			while (p && *p) {
				TCHAR* q = StrChr(p, '|');
				if (q)
					*q++ = '\0';
				tci.pszText = p;
				tci.iImage = index;
				tab.SetItem(index, &tci);
				index++;
				p = q;
			}
			m_bInitCompleted = true;
		}
	}
};

};

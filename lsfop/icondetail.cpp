////////////////////////////////////////////////////////////////////////
// icondetail.cpp

#include "stdafx.h"
#include <atldlgs.h>

#include "fopstd.h"
#include "iconres.h"
#include "finfo.h"

/////////////////////////////////////////////////////////////////////////////
// CAssociateDlg dialog

class CIconDetailDlg : public CDialogImpl<CIconDetailDlg>
{
public:
	ICONRESOURCE* m_pir;
	CIcon m_ic;
	CInfoListViewCtrl m_list;

public:
	CIconDetailDlg() : m_pir(NULL) { }
	~CIconDetailDlg() { }

	enum { IDD = IDD_ICONDETAIL };

BEGIN_MSG_MAP(CIconDetailDlg)
	ON_WM_INITDIALOG()
	ON_WM_DRAWITEM()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnNotifyItemChanged)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_SAVE, OnExport)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	REFLECT_NOTIFY_ID(IDC_LIST)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	inline void OnNotifyItemChanged(LPNMHDR pnmh, LRESULT* pResult) { OnSelChange(); }

	void OnCancel() { EndDialog(IDCANCEL); }
	void OnExport();
	void OnCopy();

	void OnSelChange();
};

BOOL CIconDetailDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pir = (ICONRESOURCE*)lParam;
	if (!m_pir || !m_pir->num) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	m_list.Initialize(GetDlgItem(IDC_LIST), _T("サイズ\0色数\0\0"));

	m_list.AddItemOrGroup(true, _T("アイコン一覧"), NULL);
	CString strSize, strBits;
	for (UINT i = 0; i < m_pir->num; i++) {
		strSize.Format(_T("%d x %d"), m_pir->images[i].width, m_pir->images[i].height);
		switch (m_pir->images[i].colors) {
		case 1:		strBits = _T("Monochrome (1 bit)"); break;
		case 4:		strBits = _T("16 Colors (4 bits)"); break;
		case 8:		strBits = _T("256 Colors (8 bits)"); break;
		case 24:	strBits = _T("True Colors (RGB 24 bits)"); break;
		case 32:	strBits = _T("True Colors (RGB/A 32 bits)"); break;
		default:	strBits.Format(_T("%d bits"), m_pir->images[i].colors);
		}
		m_list.AddItemOrGroup(false, strSize, strBits, (LPARAM)&m_pir->images[i]);
	}
	m_list.AutoSizeColumns();
	m_list.SelectItem(0);

	OnSelChange();
	return TRUE;
}

void CIconDetailDlg::OnSelChange()
{
	int sel = m_list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (sel >= 0) {
		if (!m_ic.IsNull()) {
			m_ic.DestroyIcon();
		}

		ICONIMAGE* pii = (ICONIMAGE*)m_list.GetItemData(sel);
		if (pii)
			m_ic.Attach(MakeIconFromResource(pii));
		GetDlgItem(IDC_ICONFACTORY).InvalidateRect(NULL, TRUE);
	}

	BOOL bEnable = !m_ic.IsNull();
	GetDlgItem(IDC_COPY).EnableWindow(bEnable);
	GetDlgItem(IDC_SAVE).EnableWindow(bEnable);
}

void CIconDetailDlg::OnCopy()
{
	if (!m_ic.IsNull())
		CopyIconsToClipboard(&m_ic.m_hIcon, 1, CLR_ICONBACK);
}

static const TCHAR extIco[] = _T(".ico");
static const TCHAR extPng[] = _T(".png");

void CIconDetailDlg::OnExport()
{
	int sel = m_list.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (sel >= 0) {
		ICONIMAGE* pii = (ICONIMAGE*)m_list.GetItemData(sel);
		if (pii) {
			TCHAR name[MAX_PATH * 2], dir[MAX_PATH * 2];
			lstrcpy(name, m_pir->szName);
			PathGetParent(name, dir);

			TCHAR* p = PathFindExtension(name);
			if (*p == '.')
				*p = 0;
			wsprintf(&name[lstrlen(name)], _T("(%dx%d %dbpp)"), pii->width, pii->height, pii->colors);

			CString flt; flt.LoadString(IDS_FILTER_ICON_PNG); flt.Replace(_T('|'), 0);
			CFileDialog dlg(FALSE, NULL, name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, flt, m_hWnd);
			dlg.m_ofn.lpstrInitialDir = dir;
			if (dlg.DoModal() == IDOK) {
				lstrcpyn(name, dlg.m_szFileName, ARRAYSIZE(name));
				if (dlg.m_ofn.nFilterIndex == 2 || (dlg.m_ofn.nFilterIndex == 3 && !StrCmpI(PathFindExtension(name), extPng))) {
					if (StrCmpI(PathFindExtension(name), extPng))
						PathRenameExtension(name, extPng);
					WriteIconToPNGFile(pii, name);
				} else {
					if (StrCmpI(PathFindExtension(name), extIco))
						PathRenameExtension(name, extIco);

					ICONRESOURCE ir;
					memcpy(&ir, m_pir, sizeof(ir));
					memcpy(ir.images, pii, sizeof(ICONIMAGE));
					ir.num = 1;
					WriteIconToICOFile(&ir, name);
				}
			}
		}
	}
}

void CIconDetailDlg::OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi)
{
	if (idCtrl == IDC_ICONFACTORY) {
		CRect rc(&pdi->rcItem);
		lccPatB(pdi->hDC, rc.left, rc.top, rc.Width(), rc.Height(), GetSysColor(COLOR_WINDOW));
		if (!m_ic.IsNull()) {
			ICONINFO ii;
			if (m_ic.GetIconInfo(&ii)) {
				BITMAP bm;
				::GetObject(ii.hbmColor, sizeof(bm), &bm);
				if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
				if (ii.hbmMask) ::DeleteObject(ii.hbmMask);

				DrawIconEx(pdi->hDC,
						(rc.left + rc.right - bm.bmWidth) / 2,
						(rc.top + rc.bottom - bm.bmHeight) / 2,
						m_ic, 0, 0, 0, NULL, DI_NORMAL);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
//

void ShowIconDetail(HWND hwnd, const ICONRESOURCE* pir)
{
	CIconDetailDlg dlg;
	dlg.DoModal(hwnd, (LPARAM)pir);
}

extern "C"
void WINAPI fopShowIconDetail(HWND hwnd, LPCTSTR pszFile, int index)
{
	ICONRESOURCE* pir;
	if (!StrCmpI(PathFindExtension(pszFile), _T(".ico"))) {
		pir = ReadIconFromICOFile(pszFile);
	} else {
		pir = GetIconResourceFromFile(pszFile, index);
	}
	if (pir) {
		lstrcpy(pir->szName, pszFile);
		ShowIconDetail(hwnd, pir);
		FreeIconRes(pir);
	}
}


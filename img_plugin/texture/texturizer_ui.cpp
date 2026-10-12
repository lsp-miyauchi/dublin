// texture.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "..\filterdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_RESOURCE_TEXTURE		42

typedef struct tagTEXTURIZERINFO {
	int depth;
	int direction;
	UINT resource_id;
} TEXTURIZERINFO;

static HDIB texturizer(HDIB hdib, LPRECT prc, UINT idRes, int dir, int depth)
{
	mif image(hdib);
	image.texturize_by_resource(prc, RESINST, (UINT)RT_RCDATA, idRes, dir, depth);
	return image.createdib();
}


/////////////////////////////////////////////////////////////////////////////
// CTextureDlg ダイアログ

class CTextureDlg : public CDialogImpl<CTextureDlg>, public CFilterDialogImpl<CTextureDlg>
{
public:
	CTextureDlg();

	enum { IDD = IDD_TEXTURE };
	CListBox m_list;
	CComboBox m_dircombo;
	CTrackCtrl m_slider;
	TEXTURIZERINFO m_ti;
	HDIB m_hdib;
	CSimpleArray<UINT> m_txrid;

BEGIN_MSG_MAP(CTextureDlg)
	CHAIN_MSG_MAP(CFilterDialogImpl<CTextureDlg>)
	ON_LBN_SELCHANGE(IDC_LIST, OnTxrChange)
	ON_CBN_SELCHANGE(IDC_DIRCOMBO, OnDirectionChange)
END_MSG_MAP()

	void OnTxrChange();
	void OnDirectionChange();

	BOOL InitDialog(LPARAM lParam);
	void HScroll(int val, HWND hScrollBar);
	HDIB GetPreview(HDIB hdib);

	void ListTexture();
};

CTextureDlg::CTextureDlg()
{
	m_ti.depth = 8;
	m_ti.direction = 0;
	m_ti.resource_id = ID_TXR_FIRST;
}

/////////////////////////////////////////////////////////////////////////////
// CTextureDlg メッセージ ハンドラ

BOOL CTextureDlg::InitDialog(LPARAM lParam)
{
	m_list = GetDlgItem(IDC_LIST);
	m_dircombo = GetDlgItem(IDC_DIRCOMBO);
	m_slider = GetDlgItem(IDC_SLIDER);

	m_slider.SetRange(1, 40);
	m_slider.SetPos(m_ti.depth);
	m_slider.SetTicFreq(5);

	ListTexture();
	if (m_txrid.GetSize() < 1) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	int sel = 0;
	for (int i = 0; i < m_txrid.GetSize(); i++) {
		if (m_txrid[i] == m_ti.resource_id) {
			sel = i;
			break;
		}
	}
	m_list.SetCurSel(sel);
	m_dircombo.SetCurSel(m_ti.direction);

	SetPreviewImage(m_hdib);
	UpdatePreview();

	return TRUE;
}

void CTextureDlg::OnTxrChange() 
{
	int sel = m_list.GetCurSel();
	if (sel != LB_ERR) {
		m_ti.resource_id = m_txrid[sel];
		UpdatePreview();
	}
}

void CTextureDlg::OnDirectionChange() 
{
	m_ti.direction = m_dircombo.GetCurSel();
	UpdatePreview();
}

void CTextureDlg::HScroll(int val, HWND hScrollBar) 
{
	m_ti.depth = val;
	UpdatePreview();
}

HDIB CTextureDlg::GetPreview(HDIB hdib)
{
	return texturizer(hdib, NULL, m_ti.resource_id, m_ti.direction, m_ti.depth);
}

void CTextureDlg::ListTexture()
{
	char comment[MAX_PATH];
	for (int i = 0; i < MAX_RESOURCE_TEXTURE; i++) {
		if (imgGetResourceTxrInfo(RESINST, (UINT)RT_RCDATA, ID_TXR_FIRST + i, comment, NULL, NULL, NULL)) {
			WCHAR wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, comment, -1, wsz, _COUNTOF(wsz));
			m_list.AddString(wsz);
			m_txrid.Add(ID_TXR_FIRST + i);
		}
	}
}

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("texture\\texturizer");
static TCHAR keyLast[] = _T("last");

HANDLE texturizer_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	CTextureDlg dlg;
	mif image(hdib);
	image.crop(prc);
	dlg.m_hdib = image.createdib();
	image.destroy();

	regkey reg(pfe->hkey, secRoot);
	TEXTURIZERINFO ti;
	if (reg.getbinary(keyLast, &ti, sizeof(ti)) == sizeof(ti))
		memcpy(&dlg.m_ti, &ti, sizeof(ti));

	BOOL bOK = (dlg.DoModal(pfe->hparent) == IDOK);
	GlobalFree(dlg.m_hdib);
	if (bOK) {
		reg.setbinary(keyLast, &dlg.m_ti, sizeof(dlg.m_ti));
		return texturizer(hdib, prc, dlg.m_ti.resource_id, dlg.m_ti.direction, dlg.m_ti.depth);
	}
	return NULL;
}

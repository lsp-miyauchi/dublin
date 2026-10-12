// Setup.cpp : CSetup の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "Setup.h"

///////////////////////////////////////////////////////////////
// CPageGeneric

LRESULT CPageGeneric::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_flag & SHLE_SUBMENU)		CheckDlgButton(IDC_SHELL_USESUBMENU, TRUE);
	if (m_flag & SHLE_SYNC)			CheckDlgButton(IDC_SHELL_DIRSYNC, TRUE);
	if (m_flag & SHLE_BDUMP)		CheckDlgButton(IDC_SHELL_BDUMP, TRUE);
	if (m_flag & SHLE_ATTR)			CheckDlgButton(IDC_SHELL_ATTRIB, TRUE);
	if (m_flag & SHLE_DIV)			CheckDlgButton(IDC_SHELL_DIVMRG, TRUE);
	if (m_flag & SHLE_DFLUSH)		CheckDlgButton(IDC_SHELL_DFLUSH, TRUE);
	if (m_flag & SHLE_RENAME)		CheckDlgButton(IDC_SHELL_RENAME, TRUE);
	if (m_flag & SHLE_FILTERDEL)	CheckDlgButton(IDC_SHELL_FILTERDEL, TRUE);
	if (m_flag & SHLE_SHREDDER)		CheckDlgButton(IDC_SHELL_SHREDDER, TRUE);
	if (m_flag & SHLE_FSIZE)		CheckDlgButton(IDC_SHELL_FSIZE, TRUE);
	if (m_flag & SHLE_DRVINFO)		CheckDlgButton(IDC_SHELL_DRVINFO, TRUE);
	if (m_flag & SHLE_FILEINFO)		CheckDlgButton(IDC_SHELL_FILEINFO, TRUE);
	if (m_flag & SHLE_PATHCLIP)		CheckDlgButton(IDC_SHELL_PATHCLIP, TRUE);
	if (m_flag & SHLE_FV)			CheckDlgButton(IDC_SHELL_FILEVISOR, TRUE);
	if (m_flag & SHLE_CRYPT)		CheckDlgButton(IDC_SHELL_CRYPT, TRUE);
	if (m_flag & SHLE_CALCHASH)		CheckDlgButton(IDC_SHELL_CALCHASH, TRUE);
	if (m_flag & SHLE_DROPCOPYTO)	CheckDlgButton(IDC_SHELL_DROPCOPYTO, TRUE);
	if (m_flag & SHLE_DROPCRYPTTO)	CheckDlgButton(IDC_SHELL_DROPCRYPTTO, TRUE);
/*
	if (m_flag & SHLE_DROPREPARSE)	CheckDlgButton(IDC_SHELL_DROPREPARSE, TRUE);
	if (_AtlModule.OsVISTA()) {
		if (m_flag & SHLE_DROPSYMLINK)	CheckDlgButton(IDC_SHELL_DROPSYMLINK, TRUE);
	} else {
		GetDlgItem(IDC_SHELL_DROPSYMLINK).EnableWindow(FALSE);
	}
*/
	if (m_flag & SHLE_DROPSYSLINK)	CheckDlgButton(IDC_SHELL_DROPSYSLINK, TRUE);
	return TRUE;
}

BOOL CPageGeneric::OnApply()
{
	DWORD flag = 0;
	if (IsDlgButtonChecked(IDC_SHELL_USESUBMENU))	flag |= SHLE_SUBMENU;
	if (IsDlgButtonChecked(IDC_SHELL_DIRSYNC))		flag |= SHLE_SYNC;
	if (IsDlgButtonChecked(IDC_SHELL_BDUMP))		flag |= SHLE_BDUMP;
	if (IsDlgButtonChecked(IDC_SHELL_ATTRIB))		flag |= SHLE_ATTR;
	if (IsDlgButtonChecked(IDC_SHELL_DIVMRG))		flag |= SHLE_DIV;
	if (IsDlgButtonChecked(IDC_SHELL_DFLUSH))		flag |= SHLE_DFLUSH;
	if (IsDlgButtonChecked(IDC_SHELL_RENAME))		flag |= SHLE_RENAME;
	if (IsDlgButtonChecked(IDC_SHELL_FILTERDEL))	flag |= SHLE_FILTERDEL;
	if (IsDlgButtonChecked(IDC_SHELL_SHREDDER))		flag |= SHLE_SHREDDER;
	if (IsDlgButtonChecked(IDC_SHELL_FSIZE))		flag |= SHLE_FSIZE;
	if (IsDlgButtonChecked(IDC_SHELL_DRVINFO))		flag |= SHLE_DRVINFO;
	if (IsDlgButtonChecked(IDC_SHELL_FILEINFO))		flag |= SHLE_FILEINFO;
	if (IsDlgButtonChecked(IDC_SHELL_PATHCLIP))		flag |= SHLE_PATHCLIP;
	if (IsDlgButtonChecked(IDC_SHELL_FILEVISOR))	flag |= SHLE_FV;
	if (IsDlgButtonChecked(IDC_SHELL_CRYPT))		flag |= SHLE_CRYPT;
	if (IsDlgButtonChecked(IDC_SHELL_CALCHASH))		flag |= SHLE_CALCHASH;
	if (IsDlgButtonChecked(IDC_SHELL_DROPCOPYTO))	flag |= SHLE_DROPCOPYTO;
	if (IsDlgButtonChecked(IDC_SHELL_DROPCRYPTTO))	flag |= SHLE_DROPCRYPTTO;
/*
	if (IsDlgButtonChecked(IDC_SHELL_DROPREPARSE))	flag |= SHLE_DROPREPARSE;
	if (_AtlModule.OsVISTA() && IsDlgButtonChecked(IDC_SHELL_DROPSYMLINK)) {
		flag |= SHLE_DROPSYMLINK;
	}
*/
	if (IsDlgButtonChecked(IDC_SHELL_DROPSYSLINK))	flag |= SHLE_DROPSYSLINK;
	if (flag != m_flag) {
		m_flag = flag;
		m_update = TRUE;
	}
	return TRUE;
}

LRESULT CPageGeneric::OnPopupHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	HELPINFO* phi = (HELPINFO*)lParam;
	if (phi->iContextType == HELPINFO_WINDOW && phi->iCtrlId) {
		ShowCtrlHelp(m_hWnd, phi->iCtrlId);
	}
	return 0;
}

///////////////////////////////////////////////////////////////
// CPagePack

LRESULT CPagePack::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_flag & SHLA_PACK)			CheckDlgButton(IDC_SHELL_PACK, TRUE);
	if (m_flag & SHLA_CHECK)		CheckDlgButton(IDC_SHELL_ARCCHECK, TRUE);
	if (m_flag & SHLA_UNPACK)		CheckDlgButton(IDC_SHELL_UNPACK, TRUE);
	if (m_flag & SHLA_DROPPACK)		CheckDlgButton(IDC_SHELL_DROPPACK, TRUE);
	if (m_flag & SHLA_DROPUNPACK)	CheckDlgButton(IDC_SHELL_DROPUNPACK, TRUE);

	CWindow wnd = GetDlgItem(IDC_SHELL_ARC_LEGACY);
	while (wnd.IsWindow()) {
		wnd.ShowWindow(SW_HIDE);
		wnd = ::GetNextWindow(wnd, GW_HWNDNEXT);
	}
	return TRUE;
}

BOOL CPagePack::OnApply()
{
	DWORD flag = 0;
	if (IsDlgButtonChecked(IDC_SHELL_PACK))			flag |= SHLA_PACK;
	if (IsDlgButtonChecked(IDC_SHELL_ARCCHECK))		flag |= SHLA_CHECK;
	if (IsDlgButtonChecked(IDC_SHELL_UNPACK))		flag |= SHLA_UNPACK;
	if (IsDlgButtonChecked(IDC_SHELL_DROPPACK))		flag |= SHLA_DROPPACK;
	if (IsDlgButtonChecked(IDC_SHELL_DROPUNPACK))	flag |= SHLA_DROPUNPACK;
	if (flag != m_flag) {
		m_flag = flag;
		m_update = TRUE;
	}
	return TRUE;
}

void CPagePack::OnPackAssociation()
{
	IApplicationAssociationRegistrationUI* p = NULL;
	HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI, NULL, CLSCTX_INPROC, __uuidof(IApplicationAssociationRegistrationUI), (void**)&p);
	if (SUCCEEDED(hr)) {
		p->LaunchAdvancedAssociationUI(L"ArcManager");
		p->Release();
	}
}

LRESULT CPagePack::OnPopupHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	HELPINFO* phi = (HELPINFO*)lParam;
	if (phi->iContextType == HELPINFO_WINDOW && phi->iCtrlId) {
		ShowCtrlHelp(m_hWnd, phi->iCtrlId);
	}
	return 0;
}

///////////////////////////////////////////////////////////////
// CPageImage

LRESULT CPageImage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_flag & SHLI_COPY)				CheckDlgButton(IDC_SHELL_IMAGE_COPY, TRUE);
	if (m_flag & SHLI_CONVERT)			CheckDlgButton(IDC_SHELL_IMAGE_CONVERT, TRUE);
	if (m_flag & SHLI_JPEG2PDF)			CheckDlgButton(IDC_SHELL_IMAGE_JPEG2PDF, TRUE);
	if (m_flag & SHLI_ALBUM)			CheckDlgButton(IDC_SHELL_IMAGE_ALBUM, TRUE);
	if (m_flag & SHLI_RENAME)			CheckDlgButton(IDC_SHELL_IMAGE_RENAME, TRUE);
	if (m_flag & SHLI_THUMBOUT)			CheckDlgButton(IDC_SHELL_IMAGE_THUMBOUT, TRUE);
	if (m_flag & SHLI_SLIDESHOW)		CheckDlgButton(IDC_SHELL_IMAGE_SLIDESHOW, TRUE);
	if (m_flag & SHLI_ORIENTATION)		CheckDlgButton(IDC_SHELL_IMAGE_ORIENTATION, TRUE);
	if (m_flag & SHLI_DROPTHUMBOUT)		CheckDlgButton(IDC_SHELL_IMAGE_DROPTHUMBOUT, TRUE);
	if (m_flag & SHLI_PROPEXIF)			CheckDlgButton(IDC_SHELL_IMAGE_PROPEXIF, TRUE);
	if (m_flag & SHLI_CROPIMAGE)		CheckDlgButton(IDC_SHELL_IMAGE_CROPIMAGE, TRUE);
	return TRUE;
}

BOOL CPageImage::OnApply()
{
	DWORD flag = 0;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_COPY))			flag |= SHLI_COPY;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_CONVERT))		flag |= SHLI_CONVERT;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_JPEG2PDF))		flag |= SHLI_JPEG2PDF;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_ALBUM))			flag |= SHLI_ALBUM;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_RENAME))			flag |= SHLI_RENAME;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_THUMBOUT))		flag |= SHLI_THUMBOUT;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_SLIDESHOW))		flag |= SHLI_SLIDESHOW;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_ORIENTATION))	flag |= SHLI_ORIENTATION;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_DROPTHUMBOUT))	flag |= SHLI_DROPTHUMBOUT;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_PROPEXIF))		flag |= SHLI_PROPEXIF;
	if (IsDlgButtonChecked(IDC_SHELL_IMAGE_CROPIMAGE))		flag |= SHLI_CROPIMAGE;

	if (flag != m_flag) {
		m_flag = flag;
		m_update = TRUE;
	}
	return TRUE;
}

LRESULT CPageImage::OnPopupHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	HELPINFO* phi = (HELPINFO*)lParam;
	if (phi->iContextType == HELPINFO_WINDOW && phi->iCtrlId) {
		ShowCtrlHelp(m_hWnd, phi->iCtrlId);
	}
	return 0;
}

///////////////////////////////////////////////////////////////
// CPageIcon

LRESULT CPageIcon::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_flag &= (SHLE_ICONDUMP | SHLE_DROPICONPICK | SHLE_PROPICON);
	if (m_flag & SHLE_ICONDUMP)		CheckDlgButton(IDC_SHELL_ICONDUMP, TRUE);
	if (m_flag & SHLE_DROPICONPICK)	CheckDlgButton(IDC_SHELL_DROPICONPICK, TRUE);
	if (m_flag & SHLE_PROPICON)		CheckDlgButton(IDC_SHELL_ICON_PROPICON, TRUE);
	return TRUE;
}

BOOL CPageIcon::OnApply()
{
	DWORD flag = 0;
	if (IsDlgButtonChecked(IDC_SHELL_ICONDUMP))			flag |= SHLE_ICONDUMP;
	if (IsDlgButtonChecked(IDC_SHELL_DROPICONPICK))		flag |= SHLE_DROPICONPICK;
	if (IsDlgButtonChecked(IDC_SHELL_ICON_PROPICON))	flag |= SHLE_PROPICON;
	if (flag != m_flag) {
		m_flag = flag;
		m_update = TRUE;
	}
	return TRUE;
}

LRESULT CPageIcon::OnPopupHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	HELPINFO* phi = (HELPINFO*)lParam;
	if (phi->iContextType == HELPINFO_WINDOW && phi->iCtrlId) {
		ShowCtrlHelp(m_hWnd, phi->iCtrlId);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////
//

void DoSetup(HWND hwnd)
{
	CSetupSheet sheet(IDS_GROUP_SHELL);
	CPageGeneric pageGeneric;
	CPagePack pagePack;
	CPageImage pageImage;
	CPageIcon pageIcon;

	GetShellSetting(pagePack.m_flag, pageGeneric.m_flag, pageImage.m_flag);
	pageIcon.m_flag = pageGeneric.m_flag;
	sheet.AddPage(pageGeneric);
	sheet.AddPage(pagePack);
	sheet.AddPage(pageImage);
	sheet.AddPage(pageIcon);
	if (sheet.DoModal(hwnd) == IDOK && (pagePack.m_update || pageGeneric.m_update || pageImage.m_update || pageIcon.m_update)) {
		// レジストリ設定をいったんクリア
		RegisterTerm();
		// レジストリ再設定
		RegisterInit(pagePack.m_flag, pageGeneric.m_flag | pageIcon.m_flag, pageImage.m_flag);
	}
}


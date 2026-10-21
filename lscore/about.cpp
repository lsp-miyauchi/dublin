////////////////////////////////////////////////////////////////////////
// ABOUT.CPP

#include "stdafx.h"
#include <commoncontrols.h>
#include "gdiplus_sup.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"
#include "resource.h"
#include "lccctrls.h"

#include "colored_dialog.h"

////////////////////////////////////////////////////////////////////////
//

static WCHAR sszUnreg[] = L"ユーザ未登録";
static TCHAR sszVer[] = _T("ver%d.%02d");

/*-------------------------------------------------------------------------
  IsCurrentUserLocalAdministrator ()

  This function checks the token of the calling thread to see if the caller
  belongs to the Administrators group.

  Return Value:
     TRUE if the caller is an administrator on the local machine.
     Otherwise, FALSE.
--------------------------------------------------------------------------*/

static BOOL IsCurrentUserLocalAdministrator(void)
{
	BOOL   fReturn         = FALSE;
	DWORD  dwStatus;
	DWORD  dwAccessMask;
	DWORD  dwAccessDesired;
	DWORD  dwACLSize;
	DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
	PACL   pACL            = NULL;
	PSID   psidAdmin       = NULL;

	HANDLE hToken              = NULL;
	HANDLE hImpersonationToken = NULL;

	PRIVILEGE_SET   ps;
	GENERIC_MAPPING GenericMapping;

	PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
	SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

	/*
      Determine if the current thread is running as a user that is a member of
      the local admins group.  To do this, create a security descriptor that
      has a DACL which has an ACE that allows only local aministrators access.
      Then, call AccessCheck with the current thread's token and the security
      descriptor.  It will say whether the user could access an object if it
      had that security descriptor.  Note: you do not need to actually create
      the object.  Just checking access against the security descriptor alone
      will be sufficient.
	*/
	const DWORD ACCESS_READ  = 1;
	const DWORD ACCESS_WRITE = 2;

   __try {
		/*
		 AccessCheck() requires an impersonation token.  We first get a primary
         token and then create a duplicate impersonation token.  The
         impersonation token is not actually assigned to the thread, but is
         used in the call to AccessCheck.  Thus, this function itself never
         impersonates, but does use the identity of the thread.  If the thread
         was impersonating already, this function uses that impersonation context.
		*/
		if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY, TRUE, &hToken)) {
			if (GetLastError() != ERROR_NO_TOKEN)
				__leave;
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE|TOKEN_QUERY, &hToken))
				__leave;
		}

		if (!DuplicateToken (hToken, SecurityImpersonation, &hImpersonationToken))
			__leave;


		/*
         Create the binary representation of the well-known SID that
         represents the local administrators group.  Then create the security
         descriptor and DACL with an ACE that allows only local admins access.
         After that, perform the access check.  This will determine whether
         the current user is a local admin.
		*/
		if (!AllocateAndInitializeSid(&SystemSidAuthority, 2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0, &psidAdmin))
			__leave;

		psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (psdAdmin == NULL)
			__leave;

		if (!InitializeSecurityDescriptor(psdAdmin, SECURITY_DESCRIPTOR_REVISION))
			__leave;

		// Compute size needed for the ACL.
		dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidAdmin) - sizeof(DWORD);
		pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
		if (pACL == NULL)
			__leave;
		if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
			__leave;

		dwAccessMask= ACCESS_READ | ACCESS_WRITE;
		if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, psidAdmin))
			__leave;

		if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
			__leave;

		/*
         AccessCheck validates a security descriptor somewhat; set the group
         and owner so that enough of the security descriptor is filled out to
         make AccessCheck happy.
		*/
		SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
		SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

		if (!IsValidSecurityDescriptor(psdAdmin))
			__leave;

		dwAccessDesired = ACCESS_READ;

		/*
         Initialize GenericMapping structure even though you
         do not use generic rights.
		*/
		GenericMapping.GenericRead    = ACCESS_READ;
		GenericMapping.GenericWrite   = ACCESS_WRITE;
		GenericMapping.GenericExecute = 0;
		GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;
		if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired,
                       &GenericMapping, &ps, &dwStructureSize, &dwStatus,
                       &fReturn)) {
			fReturn = FALSE;
			__leave;
		}
	}
   __finally {
		// Clean up.
		if (pACL) LocalFree(pACL);
		if (psdAdmin) LocalFree(psdAdmin);
		if (psidAdmin) FreeSid(psidAdmin);
		if (hImpersonationToken) CloseHandle (hImpersonationToken);
		if (hToken) CloseHandle (hToken);
	}
	return fReturn;
}

////////////////////////////////////////////////////////////////////////
//

class CAppAboutInfo : public APPABOUTINFO_V2
{
public:
	CAppAboutInfo()
	{
		magic = MAGIC_APPABOUTINFO_V2;
		hInst = NULL;
		szUser[0] = 0;
		szCompany[0] = 0;
		szUrl[0] = 0;
	}
	CAppAboutInfo(_In_ const APPABOUTINFO& src) { CopyInfo(src); }
	CAppAboutInfo(_In_ const APPABOUTINFO_V2& src) { CopyInfo(src); }

	void CopyInfo(_In_ const APPABOUTINFO& src)
	{
		magic = MAGIC_APPABOUTINFO_V2;
		hInst = src.hInst;
		StrCpyN(szUser, src.szUser, ARRAYSIZE(szUser));
		StrCpyN(szCompany, src.szCompany, ARRAYSIZE(szCompany));
		szUrl[0] = 0;
	}
	void CopyInfo(_In_ const APPABOUTINFO_V2& src)
	{
		magic = MAGIC_APPABOUTINFO_V2;
		hInst = src.hInst;
		StrCpyN(szUser, src.szUser, ARRAYSIZE(szUser));
		StrCpyN(szCompany, src.szCompany, ARRAYSIZE(szCompany));
		StrCpyN(szUrl, src.szUrl, ARRAYSIZE(szUrl));
	}
	operator LPAPPABOUTINFO_V2()
	{
		return this;
	}
	operator LPAPPABOUTINFO()
	{
		return (LPAPPABOUTINFO)&this->hInst;
	}
};

////////////////////////////////////////////////////////////////////////
//

/*
 *
 */
static HICON getAppIcon(LPCTSTR pszModule, int width, int height)
{
	HICON hic = NULL;
#ifdef USE_IID_IMAGELIST
	IImageList* pii;
	if (SHGetImageList(SHIL_JUMBO, IID_IImageList, (void**)&pii) == S_OK) {
#ifdef Shell_GetCachedImageIndex
#undef Shell_GetCachedImageIndex
#endif
		int iIcon = Shell_GetCachedImageIndex(pszModule, 0, FALSE);
		if (pii->GetIcon(iIcon, ILD_TRANSPARENT, &hic) != S_OK) {
			hic = NULL;
		}
		pii->Release();
	}
#else
	if (PrivateExtractIcons(pszModule, 0, width, height, &hic, NULL, 1, LR_CREATEDIBSECTION) == 0) {
		hic = NULL;
	}
#endif
	return hic;
}


class CAppIcon : public CIcon
{
public:
	CSize m_sizeIcon;
	CAppIcon* SetAppIcon(LPCTSTR pszModule, int width, int height)
	{
		Attach(getAppIcon(pszModule, width, height));
		ICONINFO ii;
		if (GetIconInfo(&ii)) {
			BITMAP bm;
			GetObject(ii.hbmColor, sizeof(bm), &bm);
			m_sizeIcon.cx = bm.bmWidth;
			m_sizeIcon.cy = bm.bmHeight;
			if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
			if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
		}
		return this;
	}
	int Width() { return m_sizeIcon.cx; }
	int Height() { return m_sizeIcon.cy; }
};

class CAppAboutDlg
	: public CDialogImpl<CAppAboutDlg>
	, public CColoredDialogImpl<CAppAboutDlg>
{
private:
	CAppAboutInfo m_aai;
	CAppIcon m_appIcon;
	CBitmap m_bmp;
	int m_bmx;
	int m_bmy;
	CString m_moduleName;
	CString m_apptitle;
	CString m_version;
	CString m_copyright;

public:
	CAppAboutDlg() : m_bmx(0), m_bmy(0)
	{}
	~CAppAboutDlg()
	{}

	enum { IDD = IDD_ABOUTBOX };

BEGIN_MSG_MAP(CAppAboutDlg)
	CHAIN_MSG_MAP(CColoredDialogImpl<CAppAboutDlg>)
	ON_WM_INITDIALOG()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDOK, OnEndDialog)
	ON_BN_CLICKED(IDCANCEL, OnEndDialog)
	ON_BN_CLICKED(IDC_URL, OnEndDialog)
	ON_NOTIFY_ID(IDC_SYSLINK, OnNotifySysLink)
END_MSG_MAP()

	void OnNotifySysLink(NMHDR* pnmhdr, LRESULT* plResult)
	{
		PNMLINK plink = (PNMLINK)pnmhdr;
		switch (pnmhdr->code) {
			case NM_CLICK:
			case NM_RETURN:
				*plResult = 0;
				ShellExecute(NULL, _T("open"), plink->item.szUrl, NULL, NULL, SW_SHOW);
				EndDialog(IDOK);
				break;
		}
	}

	void MakeAppLabel()
	{
		CRect rcc;
		GetDlgItem(IDC_ABOUTAPP).GetClientRect(&rcc);

		CClientDC dc(nullptr);
		m_bmp.CreateCompatibleBitmap(dc, rcc.Width(), rcc.Height());
		m_bmx = rcc.Width();
		m_bmy = rcc.Height();

		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		HBITMAP hbmw = dcw.SelectBitmap(m_bmp);

		CRect rc = rcc;
		dcw.FillRect(&rc, COLOR_BTNHILIGHT);

		Graphics g(dcw);
		StringFormat fmt;
		fmt.SetFormatFlags(StringFormatFlagsNoWrap);				// DT_SINGLELINE
		fmt.SetAlignment(StringAlignmentNear);						// DT_LEFT
		fmt.SetTrimming(StringTrimmingEllipsisCharacter);			// followed by "..."
		fmt.SetLineAlignment(StringAlignmentNear);					// DT_TOP
		SolidBrush sbr(Color(255, 36, 130, 198));
		g.SetTextRenderingHint(TextRenderingHintAntiAlias);

		// タイトル
		RectF r((REAL)rc.left - 8, (REAL)rc.top, (REAL)rc.Width(), (REAL)40);
		if (m_apptitle.GetLength() > 0) {
			REAL pixel = 40;
			Font fntw(L"Verdana", pixel, FontStyleRegular, UnitPixel);
			// 描画矩形内に収めるようにサイズを計測
			RectF br;
			g.MeasureString(m_apptitle, m_apptitle.GetLength(), &fntw, br, &fmt, &br);
			if (br.Width > r.Width) {
				pixel *= (r.Width / br.Width);
			}
			Font font(L"Verdana", pixel, FontStyleRegular, UnitPixel);
			g.DrawString(m_apptitle, m_apptitle.GetLength(), &font, r, &fmt, &sbr);
			r.Offset(0, 50);
		}
		r.X += 8;

		sbr.SetColor(Color(255, 0, 0, 0));
		Font myFont(L"Segoe UI Semibold", 12, FontStyleRegular, UnitPixel);
		// ファイルバージョン
		if (m_version.GetLength() > 0) {
			g.DrawString(m_version, m_version.GetLength(), &myFont, r, &fmt, &sbr);
			r.Offset(0, 18);
		}
		// 著作権
		if (m_copyright.GetLength() > 0) {
			g.DrawString(m_copyright, m_copyright.GetLength(), &myFont, r, &fmt, &sbr);
		}
		dcw.SelectBitmap(hbmw);
	}

	BOOL OnInitDialog(LPARAM lParam)
	{
		if (!lParam) {
			EndDialog(IDOK);
			return TRUE;
		}

		if (((LPAPPABOUTINFO)lParam)->hInst == MAGIC_APPABOUTINFO_V2) {
			m_aai.CopyInfo(*((LPAPPABOUTINFO_V2)lParam));
		} else {
			m_aai.CopyInfo(*((LPAPPABOUTINFO)lParam));
		}
		GetModuleFileName(m_aai.hInst, m_moduleName.GetBufferSetLength(MAX_PATH), MAX_PATH);

		// get my app icon 
		m_appIcon.SetAppIcon(m_moduleName, 96, 96); 

		CenterWindow();
		// 利用者名・組織名の取得
		CString usr = m_aai.szUser;
		if (usr.GetLength() > 0) {
			usr.Trim();		// ここでTrim!
			CString cm = m_aai.szCompany;
			cm.Trim();
			if (cm.GetLength() > 0) {
				usr.AppendFormat(_T(" (%s)"), cm.GetString());
			}

			if (usr.GetLength() > 0) {
				SetDlgItemText(IDC_USERNAME, usr);
			} else {
				GetDlgItem(IDC_LICENSE_TITLE).ShowWindow(SW_HIDE);
			}
		}
		else {
			SetDlgItemText(IDC_USERNAME, sszUnreg);
		}

		// バージョン情報など
		CFileVersion ver;
		if (ver.Open(m_moduleName)) {
			TCHAR buf[MAX_PATH * 2];
			if (ver.GetFileDescription(buf, ARRAYSIZE(buf))) {
				m_apptitle = buf;
				CString cap = _T("About ") + m_apptitle;
				SetWindowText(cap);
			}
			// ファイルバージョン
			if (ver.GetFileVersion(buf)) {
				m_version.Format(_T("version %s"), buf);
				if (OsVISTA() && SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_ADMINS)) {
					m_version += _T(" (run as admin)");
				}
			}
			if (ver.GetLegalCopyright(buf, ARRAYSIZE(buf))) {
				m_copyright = buf;
			}
			ver.Close();
		}

		MakeAppLabel();

		// syslink
		if (lstrlen(m_aai.szUrl) > 0) {
			CString url;
			url.Format(_T("<a href=\"%s\">%s</a>"), m_aai.szUrl, m_aai.szUrl);
			GetDlgItem(IDC_SYSLINK).SetWindowText(url);
		}

		CString str;
		// 警告文
		str.LoadString(IDS_LAWINFO_EN);
		SetDlgItemText(IDC_LAW, str);

		return TRUE;
	}

	void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi)
	{
		CDCHandle dc(pdi->hDC);
		CRect rc(pdi->rcItem);
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		HBITMAP hbm, hbmw;
		if (idCtrl == IDC_ABOUTAPP) {
			hbmw = dcw.SelectBitmap(m_bmp);
			dc.BitBlt(0, 0, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
			dcw.SelectBitmap(hbmw);
		}
		else if (idCtrl == IDC_APPICON) {
			hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
			hbmw = dcw.SelectBitmap(hbm);
			dcw.FillSolidRect(&rc, GetSysColor(COLOR_BTNHILIGHT));

			if (!m_appIcon.IsNull()) {
				CRect rci = rc;
				rci.right = rci.left + m_appIcon.Width();
				rci.bottom = rci.top + m_appIcon.Height();
				m_appIcon.DrawIconEx(dcw, rci.left, rci.top, m_appIcon.Width(), m_appIcon.Height());
			}
			dc.BitBlt(0, 0, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
			dcw.SelectBitmap(hbmw);
		}
	}

	__inline void OnEndDialog() { EndDialog(IDOK); }
};

////////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI lccShowAppAbout(HWND hwndOwner, APPABOUTINFO* pai)
{
	CAppAboutDlg dlg;
	dlg.DoModal(hwndOwner, (LPARAM)pai);
}

extern "C"
int WINAPI lccShowAppInfo(HWND hwndOwner, KMAPPINFO* pai)
{
	return IDOK;
}


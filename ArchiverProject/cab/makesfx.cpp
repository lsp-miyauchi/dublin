#include "stdafx.h"
#include "cab.h"
#include "sfx.h"

#ifdef _CABSFX_SUPPORT

//////////////////////////////////////////////////////////////////////
//

static const WCHAR sszSfx[] = L"Sfx";

class CMakeSfxDlg : public CDialogImpl<CMakeSfxDlg>
{
public:
	CMakeSfxDlg() : m_psfx(NULL), m_x64(false) { }
	~CMakeSfxDlg() { }

	enum { IDD = IDD_SFXCFG };

	LPSFX m_psfx;
	bool m_x64;

BEGIN_MSG_MAP(CMakeSfxDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		m_psfx = (LPSFX)lParam;
		if (!m_psfx) {
			EndDialog(IDCANCEL);
			return TRUE;
		}

		m_x64 = TRUE;
		CheckRadioButton(IDC_I86SFX, IDC_X64SFX, IDC_X64SFX);

		CenterWindow();
		return TRUE;
	}

	void OnOK()
	{
		m_psfx->wStat = 0;
		m_psfx->wSize = 0;
		m_psfx->wszString[0] = 0;

		int total_length = 0;

		CString str;
		int len = GetDlgItemText(IDC_TITLE, str);
		if (len > 0) {
			m_psfx->wStat |= SFX_TITLE;
			wcscpy(&m_psfx->wszString[total_length], str);
			total_length += len + 1;
		}

		len = GetDlgItemText(IDC_MESSAGE, str);
		if (len > 0) {
			m_psfx->wStat |= SFX_MESSAGE;
			wcscpy(&m_psfx->wszString[total_length], str);
			total_length += len + 1;
		}

		len = GetDlgItemText(IDC_PATH, str);
		if (len > 0) {
			m_psfx->wStat |= SFX_PATH;
			wcscpy(&m_psfx->wszString[total_length], str);
			total_length += len + 1;
		}

		len = GetDlgItemText(IDC_COMMAND, str);
		if (len > 0) {
			m_psfx->wStat |= SFX_COMMAND;
			wcscpy(&m_psfx->wszString[total_length], str);
			total_length += len + 1;
			if (IsDlgButtonChecked(IDC_CHASE))
				m_psfx->wStat |= SFX_AUTODELETE;
		}
		else if (IsDlgButtonChecked(IDC_CHASE)) {
			AppMessage(IDS_CAB_NOEXEC);
			return;
		}

		// 64bit版を作成するかどうか
		m_x64 = IsDlgButtonChecked(IDC_X64SFX);

		m_psfx->wSize = (WORD)(total_length * sizeof(WCHAR) + sizeof(SFX));
		EndDialog(IDOK);
	}

	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}
};


//////////////////////////////////////////////////////////////////////
//

BOOL CCabEnv::MakeSfxFile(LPCWSTR pszSfx, void* pvStubData, DWORD sizeStub, SFX* info)
{
	BOOL bOK = FALSE;

	HANDLE (WINAPI* rBeginUpdateResource)(LPCWSTR, BOOL) = 0;
	BOOL (WINAPI* rUpdateResource)(HANDLE, LPCWSTR, LPCWSTR, WORD, LPVOID, DWORD) = 0;
	BOOL (WINAPI* rEndUpdateResource)(HANDLE, BOOL) = 0;

	HMODULE hmod = GetModuleHandle(_T("kernel32"));
	if (hmod) {
		(FARPROC&)rBeginUpdateResource = GetProcAddress(hmod, "BeginUpdateResourceW");
		(FARPROC&)rUpdateResource      = GetProcAddress(hmod, "UpdateResourceW");
		(FARPROC&)rEndUpdateResource   = GetProcAddress(hmod, "EndUpdateResourceW");
	}
	if (!rBeginUpdateResource || !rUpdateResource || !rEndUpdateResource) {
		msgout(L"  error. cannot found resource API in kernel32.\n");
		return FALSE;
	}

	// CAB書庫を追加
	HANDLE hfc = CreateFile(m_strCabName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hfc != INVALID_HANDLE_VALUE) {
		// CAB書庫の先頭が正規のヘッダになっているかどうか確認
		DWORD dw;
		DWORD marker[2];
		if (ReadFile(hfc, marker, sizeof(marker), &dw, NULL)) {
			if (marker[0] == 0x4643534d && marker[1] == 0) {
				SetFilePointer(hfc, 0, NULL, FILE_BEGIN);

				// CAB書庫ファイルサイズ取得
				BY_HANDLE_FILE_INFORMATION bhi;
				memset(&bhi, 0, sizeof(bhi));
				GetFileInformationByHandle(hfc, &bhi);
				info->lCabDataSize = bhi.nFileSizeLow;

				// SFXファイル作成
				HANDLE hfs = CreateFile(pszSfx, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hfs != INVALID_HANDLE_VALUE) {
					// STUBデータ書き出し
					bOK = WriteFile(hfs, pvStubData, sizeStub, &dw, NULL);
					CloseHandle(hfs);
					if (bOK) {
						// CAB書庫書き出し
						HANDLE hmap = CreateFileMapping(hfc, NULL, PAGE_READONLY, 0, 0, NULL);
						if (hmap) {
							void* baseptr = (void*)MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0);
							HANDLE hur = rBeginUpdateResource(pszSfx, FALSE);
							if (hur) {
								WORD idLang = MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT);
								rUpdateResource(hur, RT_RCDATA, MAKEINTRESOURCE(ID_SFX_INFO), idLang, info, info->wSize);
								rUpdateResource(hur, RT_RCDATA, MAKEINTRESOURCE(ID_SFX_CABDATA), idLang, baseptr, bhi.nFileSizeLow);
								rEndUpdateResource(hur, FALSE);
							}
							UnmapViewOfFile(baseptr);
							CloseHandle(hmap);
						}
					}
					else
						DeleteFile(pszSfx);
				}
				else
					msgout(L"  error. cannot create SFX %s.\n", pszSfx);
			}
			else
				msgout(L"  error. %s is NOT a cabinet.\n", (LPCWSTR)m_strCabName);
		}
		else
			msgout(L"  error. cannot read cabinet header.\n");
		CloseHandle(hfc);
	}
	else
		msgout(L"  error. cannot open cabinet.\n");
	return bOK;
}

int CCabEnv::CmdMakeSfx()
{
	size_t size = sizeof(SFX) + MAX_PATH * 4;
	SFX* psfx = (SFX*)malloc(size);
	memset(psfx, 0, size);
	psfx->wMagic = SFXMAGIC;

	msgout(L"Creating SFX...\n");

	CMakeSfxDlg dlg;
	if (dlg.DoModal(m_hwndOwner, (LPARAM)psfx) != IDOK) {
		msgout(L"  User aborted.\n");
		free(psfx);
		return -1;
	}

	BOOL bOK = FALSE;
	FDICABINETINFO fdici;
	if (CabGetCabinetInfoW(m_strCabName, &fdici)) {
		psfx->lTotalUncompressedSize = CabGetUncompressedTotalSizeW(m_strCabName);
		HRSRC hr = FindResource(RESINST, (dlg.m_x64 ? L"STUBx64" : L"STUBi86"), RT_RCDATA);
		if (hr) {
			DWORD dwStubSize = SizeofResource(RESINST, hr);
			HGLOBAL hglb = LoadResource(RESINST, hr);
			if (hglb) {
				BYTE* pStub = (BYTE*)LockResource(hglb);
				if (pStub) {
					WCHAR buf[MAX_PATH * 2], sfx[MAX_PATH * 2];
					WCHAR* p = PathFindFileName(m_strCabName);
					wcscpy(buf, p);
					PathRenameExtension(buf, L".exe");

					if (!m_strBasePath.IsEmpty())
						PathCombine(sfx, m_strBasePath, buf);
					else {
						WCHAR dir[MAX_PATH * 2];
						GetCurrentDirectory(ARRAYSIZE(dir), dir);
						PathCombine(sfx, dir, buf);
					}
					if (!_wcsicmp(sfx, m_strCabName))
						msgout(L"  error. sfx and cab are same name.\n");
					else {
						bOK = MakeSfxFile(sfx, pStub, dwStubSize, psfx);
						if (bOK)
							msgout(L"  created %s from %s.\n", buf, m_strCabName.GetString());
					}
				}
			}
		}
	}
	else
		msgout(L"  error. '%s' is invalid cabinet.\n", (LPCWSTR)m_strCabName);
	free(psfx);

	return (bOK != FALSE) ? 0 : -1;
}

#endif		// _CABSFX_SUPPORT

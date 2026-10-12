///////////////////////////////////////////////////////////////////////
// cp encrypt

#include "stdafx.h"
#include <limits.h>
#include "fopstd.h"
#include "cperelay.h"

class CCrypt
{
public:
	CCrypt();
	~CCrypt() {}

	BOOL m_bEncrypt;
	CString m_csPassword;
	CString m_csComment;
	BOOL m_bReplace;
	HDROP m_hDrop;
	int m_nEntries;
};

CCrypt::CCrypt()
{
	m_bEncrypt = TRUE;
	m_csPassword.Empty();
	m_csComment.Empty();
	m_bReplace = FALSE;
	m_hDrop = NULL;
	m_nEntries = 0;
}

static TCHAR szCryptExt[] = _T(".cpe");


#define CPEMAGIC_V5			(MAKELONG(MAKEWORD('~','C'),MAKEWORD('P','E')))
#define CPEMAGIC			(MAKELONG(MAKEWORD('~','C'),MAKEWORD('P','F')))
#define FILEMAGIC			(MAKELONG(MAKEWORD('~','F'),MAKEWORD('H',0)))
#define ENDMAGIC			(MAKELONG(MAKEWORD('~','E'),MAKEWORD('R',1)))

#pragma pack(1)

typedef struct tagDOSTIME {
	WORD wDate;
	WORD wTime;
} DOSTIME;

typedef struct tagCOMHEADER {
	DWORD magic;
	WORD size;
} COMHEADER;

typedef struct tagCPEHEADER {
	COMHEADER hdr;
	WORD flg;
	WORD crcPass;
	WORD lenPass;
	WORD lenComment;
	TCHAR szComment[1];
} CPEHEADER;

typedef struct tagCPEHEADER_V5 {
	COMHEADER hdr;
	WORD flg;
	WORD crcPass;
	WORD lenPass;
	WORD lenComment;
	char szComment[1];
} CPEHEADER_V5;

typedef struct tagFILEHEADER {
	COMHEADER hdr;
	WORD flg;
	WORD crc;
	DOSTIME dtc;
	DOSTIME dtl;
	DWORD attr;
	DWORD sizeFile;
	DWORD sizeData;
	WORD lenName;
	TCHAR name[1];
} FILEHEADER;

typedef struct tagFILEHEADER_V5 {
	COMHEADER hdr;
	WORD flg;
	WORD crc;
	DOSTIME dtc;
	DOSTIME dtl;
	DWORD attr;
	DWORD sizeFile;
	DWORD sizeData;
	WORD lenName;
	char name[1];
} FILEHEADER_V5;

#define FHF_IMPLODING		0x0001


#pragma pack()

static BYTE cbCPtbl[] = {
	0x10, 0x17, 0x13, 0x15, 0x09,
	0x08, 0x0a, 0x14, 0x06, 0x05,
	0x16, 0x02, 0x0d, 0x03, 0x01,
	0x04, 0x19, 0x0c, 0x0f, 0x0e,
	0x12, 0x07, 0x0b, 0x18, 0x11, 0x1a
};

typedef struct tagCPS {
	int index;
	BOOL flg;
} CPS, *LPCPS;

static LPVOID cptable(int count, LPCSTR pszCode)
{
	register int i, k, v1, v2, len;
	register int* tbl;
	CPS* cps;

	cps = (CPS*)LocalAlloc(LMEM_FIXED, sizeof(CPS) * count);
	tbl = (int*)LocalAlloc(LMEM_FIXED, sizeof(int) * count);
	for (i = 0; i < count; i++) {
		tbl[i] = -1;
		cps[i].index = i;
		cps[i].flg = FALSE;
	}

	len = lstrlenA(pszCode);
	v1 = count - 1;
	v2 = len + count % len;
	for (k = 0; k < count; k++) {
		v1 = cbCPtbl[pszCode[k % len] - 'A'] + v1 + v2;
		if (v1 >= count)
			v1 %= count;

		while (tbl[v1] != -1) {
			if (k & 01) {
				if (v1 == 0)
					v1 = count;
				v1--;
			}
			else {
				if (++v1 >= count)
					v1 = 0;
			}
		}
		tbl[v1] = k;
		v2++;
	}
	for (i = 0, k = count - 1; i < k; i++, k--) {
		cps[tbl[i]].index = tbl[k];
		cps[tbl[k]].index = tbl[i];
		if ((tbl[i] ^ tbl[k]) & 0x01)
			cps[tbl[i]].flg = cps[tbl[k]].flg = TRUE;
	}
	LocalFree(tbl);
	return cps;
}

static void cpmask(LPBYTE pcb, CPS* cps, int nSize)
{
	BYTE cb;
	LPBYTE pcbw = new BYTE[nSize];
	memcpy(pcbw, pcb, nSize);
	for (int i = 0; i < nSize; i++) {
		cb = pcbw[cps[i].index];
		if (cps[i].flg)
			cb = ((cb & 0x0F) << 4) | ((cb & 0xF0) >> 4);
		pcb[i] = cb;
	}
	delete [] pcbw;
}

extern "C"
BOOL WINAPI fopGetCryptFileInfo(LPCTSTR pszFile, WIN32_FIND_DATA* pwfd, LPTSTR pszText, int cchText)
{
	WIN32_FIND_DATA wfd;
	if (!pszFile || _getfileinfo(pszFile, &wfd) != NOERROR ||
		wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return FALSE;

	CFileHandle file;
	file.CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0);
	if (file.IsInvalid())
		return FALSE;

	// CPEヘッダを読む
	DWORD r;
	CPEHEADER ch;
	if (!file.ReadFile(&ch, sizeof(ch), &r))
		return FALSE;

	bool typeV5 = false;
	if (ch.hdr.magic != CPEMAGIC) {
		if (ch.hdr.magic != CPEMAGIC_V5)
			return FALSE;
		typeV5 = true;
	}

	// コメントの取得
	if (pszText) {
		*pszText = '\0';
		if (ch.lenComment > 0) {
			BYTE* p = new BYTE[ch.hdr.size];
			file.SetFilePointer(0, NULL, FILE_BEGIN);
			file.ReadFile(p, ch.hdr.size, &r);
			if (typeV5)
				MultiByteToWideChar(CP_ACP, 0, ((CPEHEADER_V5*)p)->szComment, -1, pszText, cchText - 1);
			else
				lstrcpyn(pszText, ((CPEHEADER*)p)->szComment, cchText - 1);
			pszText[cchText - 1] = '\0';
		}
	}

	file.SetFilePointer(ch.hdr.size, NULL, FILE_BEGIN);

	// ファイルヘッダを読む
	BYTE wb[sizeof(FILEHEADER) + MAX_PATH];
	if (!file.ReadFile(&wb, sizeof(wb), &r) || ((FILEHEADER*)wb)->hdr.magic != FILEMAGIC)
		return FALSE;

	if (pwfd) {
		FILEHEADER* pfh = (FILEHEADER*)wb;
		if (typeV5)
			MultiByteToWideChar(CP_ACP, 0, ((FILEHEADER_V5*)pfh)->name, -1, pwfd->cFileName, ARRAYSIZE(pwfd->cFileName));
		else
			lstrcpy(pwfd->cFileName, pfh->name);
		DosDateTimeToFileTime(pfh->dtc.wDate, pfh->dtc.wTime, &pwfd->ftCreationTime);
		DosDateTimeToFileTime(pfh->dtl.wDate, pfh->dtl.wTime, &pwfd->ftLastWriteTime);
		pwfd->cAlternateFileName[0] = '\0';
		pwfd->nFileSizeHigh = 0;
		pwfd->nFileSizeLow = pfh->sizeFile;
		pwfd->dwFileAttributes = pfh->attr;
	}
	return TRUE;
}


extern "C"
int WINAPI fopEncryptFile(LPCTSTR pszSrc, LPCTSTR pszDst, LPCTSTR pszPass, LPCTSTR pszComment)
{
	WORD tbl[UCHAR_MAX];
	initcrc16tbl(tbl);

	if (!pszPass || lstrlen(pszPass) < 1)
		return CPEE_NOPASS;

	CWin32FindData wfd;
	if (!_getfileinfo(pszSrc, &wfd) == NOERROR || wfd.IsFolder())
		return CPEE_FILENOTFOUND;

	if (_pathfileexists(pszDst) &&
		AppMessage(IDS_TITLE_ENCRYPT, IDS_QUERY_OVERWRITE, pszDst, MB_QYESNO) != IDYES)
		return NOERROR;

	// パスワード初期化 -- ANSI --
	char code[MAX_PATH];
	int lenCode = lstrlen(pszPass);
	int i;
	for (i = 0; i < lenCode; i++) {
		if (!ISALPHA(pszPass[i]))
			return CPEE_PASSCHARSET;
		code[i] = TOUPPER(pszPass[i]);
	}
	code[i] = 0;

	CCpeRelay relay;
	relay.Create(GetFocus());
	CString str;
	str.LoadString(IDS_TITLE_ENCRYPT);
	relay.SetCaption(str);

	// CPEヘッダ初期化
	DWORD w = sizeof(CPEHEADER);
	int lenComment = (pszComment && *pszComment) ? lstrlen(pszComment) : 0;
	w += lenComment * sizeof(TCHAR);
	CPEHEADER* pch = (CPEHEADER*)LocalAlloc(LMEM_FIXED, w);
	ZeroMemory(pch, w);
	pch->hdr.magic = CPEMAGIC;
	pch->crcPass = crc16(tbl, lenCode, (BYTE*)code, 0xFFFF);
	pch->lenPass = lenCode;
	pch->lenComment = lenComment;
	lstrcpy(pch->szComment, pszComment);
	pch->hdr.size = LOWORD(w);

	// FILEヘッダ初期化
	int lenName = lstrlen(wfd.cFileName);
	w = sizeof(FILEHEADER) + lenName * sizeof(TCHAR);
	FILEHEADER* pfh = (FILEHEADER*)LocalAlloc(LMEM_FIXED, w);
	ZeroMemory(pfh, w);
	pfh->hdr.magic = FILEMAGIC;
	pfh->crc = 0xFFFF;
	FileTimeToDosDateTime(&wfd.ftCreationTime, &pfh->dtc.wDate, &pfh->dtc.wTime);
	FileTimeToDosDateTime(&wfd.ftLastWriteTime, &pfh->dtl.wDate, &pfh->dtl.wTime);
	pfh->attr = wfd.dwFileAttributes;
	pfh->sizeFile = wfd.nFileSizeLow;
	pfh->sizeData = wfd.nFileSizeLow;
	pfh->lenName = lenName;
	lstrcpy(pfh->name, wfd.cFileName);
	pfh->hdr.size = LOWORD(w);

	CFileHandle src, dst;
	src.CreateFile(pszSrc, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0);
	dst.CreateFile(pszDst, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);

	if (src.IsInvalid() || dst.IsInvalid()) {
		LocalFree(pch);
		LocalFree(pfh);
		relay.Destroy();
		return CPEE_OPEN;
	}

	relay.SetTitle(pszDst);
	relay.SetFile(wfd.cFileName);
	relay.SetRange(wfd.nFileSizeLow);

	// CPEヘッダの書き込みと現在位置の取得
	DWORD r;
	dst.WriteFile(pch, pch->hdr.size, &r);
	long posFileHeader = dst.SetFilePointer(0, NULL, FILE_CURRENT);

	// ファイルヘッダの書き込み
	dst.WriteFile(pfh, pfh->hdr.size, &w);

	BOOL bAbort = FALSE;
	DWORD done = 0;
	CPS* cps;
	LPBYTE pcb = new BYTE[1024];
	while (!bAbort && src.ReadFile(pcb, 1024, &r) && r > 0) {
		pfh->crc = crc16(tbl, r, pcb, pfh->crc);

		cps = (CPS*)cptable(r, code);
		cpmask(pcb, cps, r);
		dst.WriteFile(pcb, r, &w);
		LocalFree(cps);

		done += r;
		relay.SetComplete(done);
		bAbort = relay.isCanceled();
	}
	delete [] pcb;

	// ファイルヘッダの書き直し
	dst.SetFilePointer(posFileHeader, NULL, FILE_BEGIN);
	dst.WriteFile(pfh, pfh->hdr.size, &w, NULL);
	LocalFree(pch);
	LocalFree(pfh);

	// 終了ヘッダの書き込み
	dst.SetFilePointer(0, NULL, FILE_END);
	COMHEADER hdr;
	hdr.magic = ENDMAGIC;
	hdr.size = sizeof(hdr);
	dst.WriteFile(&hdr, hdr.size, &w);

	src.CloseHandle();
	dst.CloseHandle();

	relay.Destroy();

	if (bAbort) {
		fopRemoveFileNoConfirm(pszDst);
		return CPEE_USERCANCEL;
	}

	return NOERROR;
}

static int decrypt_file_to(LPCTSTR pszSrc, LPCTSTR pszPass, LPCTSTR pszTo)
{
	WORD tbl[UCHAR_MAX];
	initcrc16tbl(tbl);

	if (!pszPass) return CPEE_NOPASS;

	int lenCode = lstrlen(pszPass);
	if (lenCode < 1) return CPEE_NOPASS;

	CWin32FindData wfd;
	if (!_getfileinfo(pszSrc, &wfd) == NOERROR || wfd.IsFolder())
		return CPEE_FILENOTFOUND;

	// 暗号化されたファイルを開く
	CFileHandle src;
	src.CreateFile(pszSrc, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0);
	if (src.IsInvalid())
		return CPEE_OPEN;

	// CPEヘッダの取得
	CPEHEADER ch;
	DWORD r, w;
	if (!src.ReadFile(&ch, sizeof(ch), &r) || r != sizeof(ch))
		return CPEE_NOTCPE;

	bool typeV5 = false;
	if (ch.hdr.magic != CPEMAGIC) {
		if (ch.hdr.magic != CPEMAGIC_V5)
			return CPEE_NOTCPE;
		typeV5 = true;
	}

	char code[MAX_PATH], pass[MAX_PATH];			// ANSI
	int i;
	for (i = 0; i < lenCode; i++) {
		if (!ISALPHA(pszPass[i]))
			return CPEE_PASSCHARSET;
		code[i] = TOUPPER(pszPass[i]);
		if (typeV5)
			pass[i] = (char)pszPass[i];
		else
			pass[i] = code[i];
	}
	code[i] = '\0';
	pass[i] = '\0';
	WORD crc = crc16(tbl, lenCode, (BYTE*)pass, 0xFFFF);

	if (ch.crcPass != crc)
		return CPEE_PASSINCORRECT;

	// FILEヘッダ位置まで移動してヘッダを取得
	FILEHEADER fh;
	src.SetFilePointer(ch.hdr.size, NULL, FILE_BEGIN);
	if (!src.ReadFile(&fh, sizeof(fh), &r) || r != sizeof(fh) || fh.hdr.magic != FILEMAGIC)
		return CPEE_NOTCPE;

	// FILEヘッダの読み直し
	FILEHEADER* pfh = (FILEHEADER*)LocalAlloc(LMEM_FIXED, fh.hdr.size);
	src.SetFilePointer(ch.hdr.size, NULL, FILE_BEGIN);
	src.ReadFile(pfh, fh.hdr.size, &r);

	// ファイル名
	TCHAR body[MAX_PATH];
	if (typeV5)
		MultiByteToWideChar(CP_ACP, 0, ((FILEHEADER_V5*)pfh)->name, -1, body, ARRAYSIZE(body));
	else
		lstrcpy(body, pfh->name);

	TCHAR dstname[MAX_PATH];
	if (pszTo == NULL || *pszTo == 0) {
		// 作成先が未指定
		PathGetParent(pszSrc, dstname);
		PathAppend(dstname, body);
	}
	else {
		// 作成先パス指定あり
		if (!_pathisdirectory(pszTo))
			return CPEE_PATHNOTFOUND;
		PathCombine(dstname, pszTo, body);
	}

	if (_pathfileexists(dstname) && AppMessage(IDS_TITLE_DECRYPT, IDS_QUERY_OVERWRITE, dstname, MB_QYESNO) != IDYES) {
		LocalFree(pfh);
		return CPEE_USERCANCEL;
	}

	CFileHandle dst;
	dst.CreateFile(dstname, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
	if (dst.IsInvalid()) {
		LocalFree(pfh);
		return CPEE_OPEN;
	}

	CCpeRelay relay;
	relay.Create(GetFocus());
	CString str;
	str.LoadString(IDS_TITLE_DECRYPT);
	relay.SetCaption(str);
	relay.SetTitle(pszSrc);
	relay.SetFile(body);
	relay.SetRange(pfh->sizeData);

	BOOL bAbort = FALSE;
	crc = 0xFFFF;
	CPS* cps;
	LPBYTE pcb = new BYTE[1024];
	DWORD left = fh.sizeData;
	while (!bAbort && src.ReadFile(pcb, min(left, 1024), &r) && r > 0) {
		left -= r;
		cps = (CPS*)cptable(r, code);
		cpmask(pcb, cps, r);
		dst.WriteFile(pcb, r, &w, NULL);

		crc = crc16(tbl, r, pcb, crc);
		LocalFree(cps);

		relay.SetComplete(pfh->sizeData - left);
		bAbort = relay.isCanceled();
	}
	delete [] pcb;

	src.CloseHandle();

	FILETIME ftc, ftl;
	DosDateTimeToFileTime(pfh->dtc.wDate, pfh->dtc.wTime, &ftc);
	DosDateTimeToFileTime(pfh->dtl.wDate, pfh->dtl.wTime, &ftl);
	dst.SetFileTime(&ftc, NULL, &ftl);
	dst.CloseHandle();

	SetFileAttributes(dstname, pfh->attr);
	LocalFree(pfh);

	relay.Destroy();

	if (fh.crc != crc) {
		fopRemoveFileNoConfirm(dstname);
		return CPEE_CRC;
	}

	if (bAbort) {
		fopRemoveFileNoConfirm(dstname);
		return CPEE_USERCANCEL;
	}
	return NOERROR;
}

extern "C"
int WINAPI fopDecryptFile(LPCTSTR pszSrc, LPCTSTR pszPass)
{
	return decrypt_file_to(pszSrc, pszPass, NULL);
}

//////////////////////////////////////////////////////////////////////
//

class CCryptDlg : public CDialogImpl<CCryptDlg>
{
public:
	CCryptDlg(UINT idResource) { IDD = idResource; m_pcr = NULL; }
	~CCryptDlg() { }

	UINT IDD;

BEGIN_MSG_MAP(CCryptDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	CCrypt* m_pcr;
};

BOOL CCryptDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pcr = (CCrypt*)lParam;
	return TRUE;
}

BOOL CCryptDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD_ENCRYPT);
		return TRUE;
	}
	return FALSE;
}

void CCryptDlg::OnOK()
{
	CString pass;
	if (GetDlgItemText(IDC_PASSWORD, pass) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_PASSWORD).SetFocus();
		return;
	}
	for (int i = 0; i < pass.GetLength(); i++) {
		if (!ISALPHA(pass[i])) {
			MessageBeep(0);
			GetDlgItem(IDC_PASSWORD).SetFocus();
			return;
		}
	}

	if (m_pcr->m_bEncrypt) {
		CString passRetype;
		if (GetDlgItemText(IDC_PASSWORD_RETYPE, passRetype) < 1 || pass.Compare(passRetype)) {
			MessageBeep(0);
			GetDlgItem(IDC_PASSWORD_RETYPE).SetFocus();
			return;
		}
		if (GetDlgItem(IDC_EDIT).GetWindowTextLength() > 0)
			GetDlgItemText(IDC_EDIT, m_pcr->m_csComment);
	}
	m_pcr->m_csPassword = pass.GetString();
	m_pcr->m_bReplace = IsDlgButtonChecked(IDC_REPLACE);
	EndDialog(IDOK);
}


//////////////////////////////////////////////////////////////////////
//

static BOOL encrypt_files_to(CCrypt& cr, LPCTSTR pszTo)
{
	CWaitCursor wc;
	LPCTSTR pszComment = cr.m_csComment.IsEmpty() ? NULL : (LPCTSTR)cr.m_csComment;
	int res;
	CWin32FindData wfd;
	TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
	for (int i = 0; i < cr.m_nEntries; i++) {
		DragQueryFile(cr.m_hDrop, i, src, ARRAYSIZE(src));
		if (GetFileData(src, wfd)) {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			if (pszTo && _pathisdirectory(pszTo))
				PathCombine(dst, pszTo, wfd.cFileName);
			else {
				PathGetParent(src, dst);
				PathAppend(dst, wfd.cFileName);
			}
			PathRenameExtension(dst, szCryptExt);
			res = fopEncryptFile(src, dst, cr.m_csPassword, pszComment);
			if (res == NOERROR) {
				if (cr.m_bReplace)
					fopRemoveFileNoConfirm(src);
			}
			else
				fopGetCryptErrorString(res, dst, ARRAYSIZE(dst));

			if (res != NOERROR) {
				CString csTitle;
				csTitle.LoadString(IDS_TITLE_CRYPT);
				MessageBeep(0);
				MessageBox(GetActiveWindow(), dst, csTitle, MB_STOP);
			}
		}
	}
	return TRUE;
}

static BOOL decrypt_files(CCrypt& cr, LPCTSTR pszTo)
{
	CWaitCursor wc;
	LPCTSTR pszComment = cr.m_csComment.IsEmpty() ? NULL : (LPCTSTR)cr.m_csComment;
	int res;
	CWin32FindData wfd;
	TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
	for (int i = 0; i < cr.m_nEntries; i++) {
		DragQueryFile(cr.m_hDrop, i, src, ARRAYSIZE(src));
		if (fopGetCryptFileInfo(src, NULL, NULL, 0)) {
			res = decrypt_file_to(src, cr.m_csPassword, pszTo);
			if (res == NOERROR) {
				if (cr.m_bReplace)
					fopRemoveFileNoConfirm(src);
			}
			else {
				fopGetCryptErrorString(res, dst, ARRAYSIZE(dst));
				CString csTitle;
				csTitle.LoadString(IDS_TITLE_CRYPT);
				MessageBeep(0);
				MessageBox(GetActiveWindow(), dst, csTitle, MB_STOP);
			}
		}
		else {
			AppMessage(IDS_TITLE_CRYPT, CPEE_NOTCPE, NULL, MB_STOP);
		}
	}
	return TRUE;
}

extern "C"
BOOL WINAPI fopDecryptFilesTo(HWND hwnd, HDROP hDrop, LPCTSTR pszTo)
{
	CCrypt cr;
	cr.m_bEncrypt = FALSE;
	cr.m_hDrop = hDrop;

	if (!cr.m_hDrop ||
		(cr.m_nEntries = DragQueryFile(hDrop, 0xffffffff, NULL, 0)) < 1)
		return FALSE;

	CCryptDlg dlg(IDD_DECRYPT);
	if (dlg.DoModal(hwnd, (LPARAM)&cr) != IDOK || cr.m_csPassword.IsEmpty())
		return FALSE;

	return decrypt_files(cr, pszTo);
}

extern "C"
BOOL WINAPI fopEncryptFilesTo(HWND hwnd, HDROP hDrop, LPCTSTR pszTo)
{
	CCrypt cr;
	cr.m_bEncrypt = TRUE;
	cr.m_hDrop = hDrop;

	if (!cr.m_hDrop ||
		(cr.m_nEntries = DragQueryFile(hDrop, 0xffffffff, NULL, 0)) < 1)
		return FALSE;

	CCryptDlg dlg(IDD_ENCRYPT);
	if (dlg.DoModal(hwnd, (LPARAM)&cr) != IDOK || cr.m_csPassword.IsEmpty())
		return FALSE;

	return encrypt_files_to(cr, NULL);
}

extern "C"
BOOL WINAPI fopCryptFiles(HWND hwnd, HDROP hDrop, BOOL bCrypt)
{
	if (bCrypt)
		return fopEncryptFilesTo(hwnd, hDrop, NULL);
	else
		return fopDecryptFilesTo(hwnd, hDrop, NULL);
}

extern "C"
void WINAPI fopRunDecryptW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	if (lpCmdLine && *lpCmdLine) {
		TCHAR cmd[MAX_PATH * 2];
		lstrcpy(cmd, lpCmdLine);
		PathUnquoteSpaces(cmd);
		if (_pathisfile(cmd)) {
			HDROP hDrop = fopDragInitBlock(CPoint(0, 0), FALSE);
			fopDragAppendFile(hDrop, cmd);
			fopDecryptFilesTo(hwnd, hDrop, NULL);
			DragFinish(hDrop);
		}
	}
}

extern "C"
void WINAPI fopRunDecrypt(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR cmd[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, ARRAYSIZE(cmd));
	fopRunDecryptW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
int WINAPI fopGetCryptErrorString(int nErrorCode, LPTSTR pszText, int cchText)
{
	int len = 0;
	if (nErrorCode != NOERROR) {
		UINT nID = IDS_CPEE_GENERAL;
		if      (nErrorCode == CPEE_FILENOTFOUND)		nID = IDS_CPEE_FILENOTFOUND;
		else if (nErrorCode == CPEE_OPEN)				nID = IDS_CPEE_OPEN;
		else if (nErrorCode == CPEE_READ)				nID = IDS_CPEE_READ;
		else if (nErrorCode == CPEE_WRITE)				nID = IDS_CPEE_WRITE;
		else if (nErrorCode == CPEE_NOMEMORY)			nID = IDS_CPEE_NOMEMORY;
		else if (nErrorCode == CPEE_PASSINCORRECT)		nID = IDS_CPEE_PASSINCORRECT;
		else if (nErrorCode == CPEE_CRC)				nID = IDS_CPEE_CRC;
		else if (nErrorCode == CPEE_NOTCPE)				nID = IDS_CPEE_NOTCPE;
		else if (nErrorCode == CPEE_NOPASS)				nID = IDS_CPEE_NOPASS;
		else if (nErrorCode == CPEE_PASSCHARSET)		nID = IDS_CPEE_PASSCHARSET;
		else if (nErrorCode == CPEE_USERCANCEL)			nID = IDS_CPEE_USERCANCEL;
		else if (nErrorCode == CPEE_PATHNOTFOUND)		nID = IDS_CPEE_PATHNOTFOUND;

		len = LoadString(RESINST, nID, pszText, cchText);
	}
	return len;
}

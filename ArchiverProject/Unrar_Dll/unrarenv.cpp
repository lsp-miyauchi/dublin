#include "stdafx.h"
#include "common.h"
#include "password.h"
#include "nextrar.h"
#include "relay.h"
#include "unrarenv.h"

static WCHAR sszRarExt[] = L".rar";
static WCHAR* sszListHeader[] = {
	L"\x0d\x0aListing of cabinet file : %s\x0d\x0a\x0d\x0a",
	L"  Original   Date     Time    Attr  Type    Name\x0d\x0a",
	L"---------- -------- -------- ------ ------  ------------------------------\x0d\x0a",
	L"%10s %8s %8s %6s %-6s  %s\x0d\x0a",
	L"%10s %-10s                       %d files\x0d\x0a\x0d\x0a",
};

//////////////////////////////////////////////////////////////////////
//

class CRunning
{
public:
	CRunning() { _G.AddModuleRef(); }
	~CRunning() { _G.DecModuleRef(); }
};

//////////////////////////////////////////////////////////////////////
//

static const WCHAR* sszResultForm[] = {
	L"succeeded: %s\n",
	L"skipped:   %s\n",
	L"error:     (%s)%s\n",
	L"nextcab:   %s\n",
};

static const WCHAR sszAbort[] = L"abort";

#define RESFORM_OK			(sszResultForm[0])
#define RESFORM_SKIP		(sszResultForm[1])
#define RESFORM_ERR			(sszResultForm[2])
#define RESFORM_NEXT		(sszResultForm[3])

//////////////////////////////////////////////////////////////////////
//

class CUnpackState
{
public:
	CUnrarEnv* m_pev;
	int m_nAction;
	BOOL m_bYesToAll;
	CStringW m_strNextArchive;
	INT_PTR m_hfWrite;
	EXTRACTINGINFOEX m_eie;
	CRelayDlg m_relay;

public:
	CUnpackState()
		: m_pev(NULL)
		, m_nAction(C_QUERY)
		, m_bYesToAll(FALSE)
		, m_hfWrite(NULL)
	{
		memset(&m_eie, 0, sizeof(EXTRACTINGINFOEX));
	}

	~CUnpackState()
	{
		RelDestroy();
	}

	inline bool CheckFlags(DWORD dwFlags) { return m_pev->isOption(dwFlags); }
	inline DWORD GetFlags() { return m_pev->m_cmdOption; }
	inline void SetFlags(DWORD dwFlags) { m_pev->m_cmdOption = dwFlags; }

	inline void RelCreate() { m_relay.Create(m_pev->m_hwndOwner); }
	inline void RelDestroy() { if (m_relay.IsWindow()) m_relay.DestroyWindow(); }
	inline void RelSetTitle(LPCWSTR pszTitle) { m_relay.SetTitle(pszTitle); }
	inline void RelSetFile(LPCWSTR pszFile) { m_relay.SetFile(pszFile); }
	inline void RelSetMessage(LPCWSTR pszMessage) { m_relay.SetMessage(pszMessage); }
	inline void RelSetRange(DWORDLONG dlTotal) { m_relay.SetRange(dlTotal); }
	inline void RelSetComplete(DWORDLONG dlDone) { m_relay.SetComplete(dlDone); }
	inline bool RelIsCanceled() { return m_relay.IsCanceled(); }
};

////////////////////////////////////////////////////////////
//

class CPathTreatDlg : public CDialogImpl<CPathTreatDlg>
{
public:
	int m_treat;
	bool m_yesall;

public:
	CPathTreatDlg()
		: m_treat(BRT_QUERY)
		, m_yesall(false)
	{
	}
	~CPathTreatDlg()
	{
	}

	enum { IDD = IDD_PATHTREAT };

BEGIN_MSG_MAP(CPathTreatDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDC_YESTOALL, OnYesToAll)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM /*lParam*/)
	{
		CenterWindow();

		UINT id = IDC_TREAT_ERROR;
		switch (m_treat) {
		case BRT_EXTRACT:	id = IDC_TREAT_EXTRACT; break;
		case BRT_IGNORE:	id = IDC_TREAT_IGNORE; break;
		case BRT_STRIPPATH:	id = IDC_TREAT_STRIPPATH; break;
		case BRT_RELATIVE:	id = IDC_TREAT_RELATIVE; break;
		}

		CheckDlgButton(id, TRUE);
		return TRUE;
	}
	void OnOK()
	{
		if      (IsDlgButtonChecked(IDC_TREAT_EXTRACT))		m_treat = BRT_EXTRACT;
		else if (IsDlgButtonChecked(IDC_TREAT_IGNORE))		m_treat = BRT_IGNORE;
		else if (IsDlgButtonChecked(IDC_TREAT_STRIPPATH))	m_treat = BRT_STRIPPATH;
		else if (IsDlgButtonChecked(IDC_TREAT_RELATIVE))	m_treat = BRT_RELATIVE;
		else if (IsDlgButtonChecked(IDC_TREAT_ERROR))		m_treat = BRT_ERROR;
		EndDialog(IDOK);
	}
	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}
	void OnYesToAll()
	{
		m_yesall = true;
		OnOK();
	}
};

////////////////////////////////////////////////////////////
//

CUnrarEnv::CUnrarEnv()
	: m_hwndOwner(NULL)
	, m_chCmd(RARCMD_NONE)
	, m_cmdOption(0)
	, m_nNumOfFiles(0)
	, m_brittle(BRT_QUERY)
{
}

CUnrarEnv::~CUnrarEnv()
{
}

int CUnrarEnv::BuildTempPathName(LPCWSTR pszFile)
{
	// 戻り値 -1=エラー, 0=無視, 1=正常

	if (!pszFile || *pszFile == 0) return -1;

	LPWSTR pw;
	if (*pszFile == '\\' ||
		*(pszFile + 1) == ':' ||
		*pszFile == '.' && (*(pszFile + 1) == ':' || (*(pszFile + 1) == '.' && *(pszFile + 2) == '\\'))) {

		if (!(m_brittle & BRT_YESALL)) {
			// 問い合わせる
			CPathTreatDlg dlg;
			dlg.m_treat = m_brittle;
			if (dlg.DoModal(m_hwndOwner) != IDOK)
				return -1;
			m_brittle = dlg.m_treat;
			if (dlg.m_yesall)
				m_brittle |= BRT_YESALL;
		}

		switch ((m_brittle & BRT_TYPEMASK)) {
		case BRT_ERROR:
			return -1;
		case BRT_IGNORE:
			return 0;
		case BRT_EXTRACT:
			if (*(pszFile + 1) == ':')
				m_strTemp = pszFile;
			else {
				WCHAR buf[MAX_PATH * 2];
				PathCombine(buf, m_strBasePath, pszFile);
				pw = m_strTemp.GetBuffer(MAX_PATH * 2);
				PathCanonicalize(pw, buf);
				m_strTemp.ReleaseBuffer();
			}
			break;
		case BRT_STRIPPATH:
			pw = m_strTemp.GetBuffer(MAX_PATH * 2);
			PathCombine(pw, m_strBasePath, PathFindFileName(pszFile));
			m_strTemp.ReleaseBuffer();
			break;
		case BRT_RELATIVE:
			while (*pszFile == '.' || *pszFile == '\\')
				pszFile++;
			if (*(pszFile + 1) == ':')
				pszFile = PathSkipRoot(pszFile);
			pw = m_strTemp.GetBuffer(MAX_PATH * 2);
			PathCombine(pw, m_strBasePath, pszFile);
			m_strTemp.ReleaseBuffer();
			break;
		}
	}
	else {
		pw = m_strTemp.GetBuffer(MAX_PATH * 2);
		PathCombine(pw, m_strBasePath, pszFile);
		m_strTemp.ReleaseBuffer();
	}
	return 1;
}

void CUnrarEnv::msgout(LPCWSTR fmt, ...)
{
	WCHAR* pOutput = new WCHAR[1024];
	va_list ap;
	va_start(ap, fmt);
	wvsprintf(pOutput, fmt, ap);
	va_end (ap);

	m_strResult += pOutput;

	delete [] pOutput;
}

BOOL CUnrarEnv::GetWorkingPath(LPWSTR pszPath, int cchMax)
{
	BOOL bOK = FALSE;
	if (pszPath && cchMax > 0) {
		if (!m_strWorkPath.IsEmpty() &&
			PathIsDirectory(m_strWorkPath) &&
			m_strWorkPath.GetLength() < cchMax) {
			wcscpy(pszPath, m_strWorkPath);
			bOK = TRUE;
		}
	}
	return bOK;
}

void CUnrarEnv::ErrorReport(struct RARHeaderDataEx& hdr, int code)
{
	switch (code) {
	case 0: break;
	case ERAR_BAD_DATA:			msgout(L"File CRC error: %s\r\n", hdr.FileNameW); break;
	case ERAR_BAD_ARCHIVE:		msgout(L"Volume is not valid RAR archive: %s\r\n", hdr.ArcNameW); break;
	case ERAR_UNKNOWN_FORMAT:	msgout(L"Unknown archive format: %s\r\n", hdr.FileNameW); break;
	case ERAR_EOPEN:			msgout(L"Volume open error: %s\r\n", hdr.ArcNameW); break;
	case ERAR_ECREATE:			msgout(L"File create error: %s\r\n", hdr.FileNameW); break;
	case ERAR_ECLOSE:			msgout(L"File close error: %s\r\n", hdr.FileNameW); break;
	case ERAR_EREAD:			msgout(L"Read error: %s\r\n", hdr.FileNameW); break;
	case ERAR_EWRITE:			msgout(L"Write error: %s\r\n", hdr.FileNameW); break;
	case ERAR_END_ARCHIVE:		msgout(L"End of archive\r\n"); break;
	case ERAR_NO_MEMORY:		msgout(L"No memory\r\n"); break;
	case ERAR_SMALL_BUF:        msgout(L"Buffer is too small\r\n"); break;
	case ERAR_MISSING_PASSWORD:	msgout(L"Password missing\r\n"); break;
	case ERAR_UNKNOWN:
	default:					msgout(L"Unknown error(%d): %s\r\n", code, hdr.FileNameW); break;
	}
}

////////////////////////////////////////////////////////////
//

static int CALLBACK unrar_cbproc(UINT msg, LPARAM userdata, LPARAM p1, LPARAM p2)
{
	CUnpackState* ps = (CUnpackState*)userdata;
	switch (msg) {
	case UCM_CHANGEVOLUMEW:
		if (p2 == RAR_VOL_ASK) {
			CNextRarDlg dlg;
			dlg.m_vol = (LPCWSTR)p1;
			if (dlg.DoModal() != IDOK)
				return -1;
			lstrcpyn((LPWSTR)p1, dlg.m_name, 1024);
		} else if (p2 == RAR_VOL_NOTIFY) {
			CStringW str((char*)p1);
			ps->RelSetTitle(PathFindFileName(str));
		}
		break;
	case UCM_PROCESSDATA:
		ps->RelSetComplete(p2);
		if (ps->RelIsCanceled()) { return -1; }
		break;
	case UCM_NEEDPASSWORDW:
		{
		CPasswordDlg dlg;
		if (dlg.DoModal() == IDOK)
			lstrcpyn((LPWSTR)p1, dlg.m_pass, p2);
		break;
		}
	}
	return 0;
}

int CUnrarEnv::CmdUnpack()
{
	CRunning running;

	CUnpackState cus;
	cus.m_pev = this;

	if (PathIsDirectory(m_strRarName)) {
		msgout(RESFORM_ERR, L"extract", (LPCWSTR)m_strRarName);
		return FALSE;
	}

	if (isOption(RAROPT_NEWONLY)) {
		cus.m_nAction = C_NEWONLY;
		cus.m_bYesToAll = TRUE;
	}
	if (isOption(RAROPT_OVERWRITE)) {
		cus.m_nAction = C_OVERWRITE;
		cus.m_bYesToAll = TRUE;
	}

	if (!isOption(RAROPT_SILENT)) {
		cus.RelCreate();
		cus.RelSetTitle(PathFindFileName(m_strRarName));
	}

	// 解凍対象ファイルが指定されていない場合には、
	// 全ファイルが解凍の対象となる
	if (m_arrMask.GetSize() < 1) {
		m_arrMask.Add(L"*.*");
		m_cmdOption |= RAROPT_FILTERPRESENTS;
	}

	// マッチスペック
	CStringW filter;
	int i, numMasks = m_arrMask.GetSize();
	for (i = 0; i < numMasks; i++) {
		if (i > 0)
			filter += ';';
		filter += m_arrMask[i];
	}
	CMatchSpec ms(filter, isOption(RAROPT_REGEXP), FALSE, ';');

	CStringW str;
	str.LoadString(IDS_EXTRACTING);
	cus.RelSetMessage(str);

	// パス名
	WCHAR buf[MAX_PATH * 2];
	wcscpy(buf, m_strRarName);
	PathRemoveFileSpec(buf);
	PathAddBackslash(buf);
	CStringW wstrRarPath = buf;
	// ファイル名
	CStringW wstrRarName = PathFindFileName(m_strRarName);

	// 書庫を解凍モード(RAR_OM_EXTRACT)で開く
	int res = 0;
	HARC harc = UnrarOpenArchiveW(m_hwndOwner, m_strRarName, RAR_OM_EXTRACT);
	if (harc) {
		CRarArchive* rar = (CRarArchive*)harc;

		_G.RARSetCallback(rar->m_harcdata, unrar_cbproc, (LPARAM)&cus);

		struct RARHeaderDataEx hdr;
		memset(&hdr, 0, sizeof(hdr));
		while (_G.RARReadHeaderEx(rar->m_harcdata, &hdr) == 0) {
			// ユーザによる中断のチェック
			if (cus.RelIsCanceled()) {
				msgout(L"User canceled\r\n"); res = 1; break;
			}
			// 読んだファイルがフィルタに一致するかチェック
			if (!ms.Match(hdr.FileNameW)) {
				// スキップ
				if (_G.RARProcessFile(rar->m_harcdata, RAR_SKIP, NULL, NULL) != 0) break;
				continue;
			}

			// 一致するので解凍
			CStringW name;
			if (isOption(RAROPT_IGNOREDIR)) {
				// フォルダを無視の場合
				PathCombine(buf, m_strBasePath, PathFindFileName(hdr.FileNameW));
				name = buf;
			} else {
				PathCombine(buf, m_strBasePath, hdr.FileNameW);
				name = buf;
			}

			if (!(hdr.FileAttr & _A_SUBDIR)) {
				cus.RelSetFile(hdr.FileNameW);
				DWORDLONG fsize = hdr.UnpSize + (((DWORDLONG)hdr.UnpSizeHigh) << 32);
				cus.RelSetRange(fsize);
			}

			// 同名ファイルの存在
			CWin32FindData wfdDst;
			if (GetFileData(name, &wfdDst)) {
				if (hdr.FileAttr & _A_SUBDIR) {
					DWORD attr = 0;
					if (hdr.FileAttr & _A_SYSTEM)	attr |= FILE_ATTRIBUTE_SYSTEM;
					if (hdr.FileAttr & _A_HIDDEN)	attr |= FILE_ATTRIBUTE_HIDDEN;
					if (hdr.FileAttr & _A_RDONLY)	attr |= FILE_ATTRIBUTE_READONLY;
					if (hdr.FileAttr & _A_ARCH)		attr |= FILE_ATTRIBUTE_ARCHIVE;
					if (attr == 0)					attr  = FILE_ATTRIBUTE_NORMAL;

					FILETIME ft;
					DosDateTimeToFileTime(HIWORD(hdr.FileTime), LOWORD(hdr.FileTime), &ft);
					SetFileDateTimes(name, attr, &ft, &ft);
				} else {
					WIN32_FIND_DATA wfdSrc;
					memset(&wfdSrc, 0, sizeof(wfdSrc));
					wcscpy(wfdSrc.cFileName, PathFindFileName(hdr.FileNameW));
					wfdSrc.nFileSizeLow = hdr.UnpSize;
					DosDateTimeToFileTime(HIWORD(hdr.FileTime), LOWORD(hdr.FileTime), &wfdSrc.ftLastWriteTime);
					if (!cus.m_bYesToAll) {
						// 同名処理の選択
						REPLDATA rd;
						memset(&rd, 0, sizeof(rd));
						rd.lpSrc = hdr.FileNameW;
						rd.lpDst = name;
						rd.pwfdSrc = &wfdSrc;
						rd.pwfdDst = &wfdDst;
						rd.bMultiDest = TRUE;
						rd.nAction = cus.m_nAction;
						if (!QueryReplace(m_hwndOwner, &rd)) {
							msgout(RESFORM_ERR, sszAbort, name);
							res = 1;
							break;
						}

						cus.m_bYesToAll = rd.bYesToAll;
						cus.m_nAction = rd.nAction;
					}

					if (cus.m_nAction == C_IGNORE) {
						msgout(RESFORM_SKIP, hdr.FileNameW);
						// スキップ
						if (_G.RARProcessFile(rar->m_harcdata, RAR_SKIP, NULL, NULL) != 0) break;
						continue;
					} else if (cus.m_nAction == C_NEWONLY) {
						// 新しいファイルのみの場合
						if (CompareFileTime(&wfdDst.ftLastWriteTime, &wfdSrc.ftLastWriteTime) >= 0) {
							msgout(RESFORM_SKIP, hdr.FileNameW);
							// スキップ
							if (_G.RARProcessFile(rar->m_harcdata, RAR_SKIP, NULL, NULL) != 0) break;
							continue;
						}
					}
				}
			}

			int code = _G.RARProcessFileW(rar->m_harcdata, RAR_EXTRACT, NULL, name);
			if (code != 0) {
				ErrorReport(hdr, code);
				res = 1; break;
			}
		}
		UnrarCloseArchive(harc);
	}

	if (!isOption(RAROPT_SILENT))
		cus.RelDestroy();
	return res;
}

int CUnrarEnv::CmdList()
{
	INDIVIDUALINFOW idi;
	HARC harc = UnrarOpenArchiveW(m_hwndOwner, m_strRarName, 0);
	if (harc) {
		int nFiles = 0;
		long lTotalSize = 0;
		if (!UnrarFindFirstW(harc, NULL, &idi)) {
			msgout(sszListHeader[0], (LPCWSTR)m_strRarName);
			msgout(sszListHeader[1]);
			msgout(sszListHeader[2]);

			WCHAR s[32], t[32], u[32];
			do {
				swprintf(s, L"%d", idi.dwOriginalSize);
				FormatDosDate(idi.wDate, t);
				FormatDosTime(idi.wTime, u);
				msgout(sszListHeader[3],
							s,
							t,
							u,
							idi.szAttribute,
							idi.szMode,
							idi.szFileName);
				nFiles++;
				lTotalSize += idi.dwOriginalSize;
			} while (!UnrarFindNextW(harc, &idi));

			msgout(sszListHeader[2]);

			swprintf(s, L"%d", lTotalSize);
			StrFormatByteSize(lTotalSize, t, ARRAYSIZE(t));
			swprintf(u, L"(%s)", t);
			msgout(sszListHeader[4], s, u, nFiles);
		}
		UnrarCloseArchive(harc);
		return 0;
	}
	return -1;
}

int CUnrarEnv::CmdCheck()
{
	return -1;
}

int CUnrarEnv::CmdTest()
{
	return 0;
}

BOOL CUnrarEnv::PerseResponseFile(LPCWSTR pFileList)
{
	CStringA str(pFileList);

	FILE* fp = fopen(str, "r");
	if (fp == NULL) return FALSE;

	WORD w;
	fread(&w, 2, 1, fp);
	if (w == 0xfeff || w == 0xfffe) {
		// UTF16 LE/BEの判定
		fclose(fp);
		return PerseResponseFileW(pFileList);
	}
	fseek(fp, 0, SEEK_SET);

	int i, len;
	char* textptr = new char[MAX_PATH * 4];
	while (fgets(textptr, MAX_PATH * 4, fp)) {
		len = (int)strlen(textptr);
		if (len > 0) {
			for (i = 0; i < len; i++) {
				if (textptr[i] == '\n' || textptr[i] == '\r') {
					textptr[i] = 0;
					break;
				}
			}
		}

		strseparatorA ssp;
		len = ssp.set(textptr, ' ');
		for (i = 0; i < len; i++) {
			CStringW wstr(ssp[i]);
			wstr.Replace('/', '\\');
			m_arrMask.Add(wstr.Trim(L' '));
		}
	}
	delete [] textptr;
	fclose(fp);
	return TRUE;
}

BOOL CUnrarEnv::PerseResponseFileW(LPCWSTR pFileList)
{
	FILE* fp = _wfopen(pFileList, L"rb");
	if (fp == NULL) return FALSE;

	BYTE cb[2];
	if (fread(cb, 1, 2, fp) != 2) return FALSE;

	// UTF16 LE/BEの判定 指定無し = 16LE
	bool b16le = true;
	if (cb[0] == 0xfe && cb[1] == 0xff)
		b16le = false;
	else if (cb[0] != 0xff || cb[1] != 0xfe)
		fseek(fp, 0, SEEK_SET);

	int i, len;
	WCHAR* textptr = new WCHAR[MAX_PATH * 4];
	while (fgetws(textptr, MAX_PATH * 4, fp)) {
		len = (int)wcslen(textptr);

		if (len > 0) {
			for (i = 0; i < len; i++) {
				if (!b16le) {
					WCHAR ch = textptr[i];
					textptr[i] = (((ch >> 8) & 0xff) | ((ch & 0xff) << 8));
				}

				if (textptr[i] == '\n' || textptr[i] == '\r') {
					textptr[i] = 0;
					break;
				}
			}
		}

		strseparatorW ssp;
		len = ssp.set(textptr, ' ');
		for (i = 0; i < len; i++) {
			CStringW wstr = ssp[i];
			wstr.Replace('/', '\\');
			m_arrMask.Add(wstr.Trim(L' '));
		}
	}
	delete [] textptr;
	fclose(fp);
	return TRUE;
}

BOOL CUnrarEnv::PerseSwitch(LPCWSTR pszSwitch)
{
	WCHAR ch, buf[MAX_PATH * 2];
	int n, y, m, d, H, M, S;
	SYSTEMTIME st;

	LPCWSTR p = pszSwitch;
	while (*p) {
		switch (*p) {
		case 'h':		// 作業フォルダ
			if (*++p) {
				wcscpy(buf, p);
				PathUnquoteSpacesW(buf);
				m_strWorkPath = buf;
				return TRUE;
			}
			return FALSE;
		case 'i':		// サイレントモード
			m_cmdOption |= RAROPT_SILENT;
			break;
		case 'j':		// フォルダを無視して解凍または圧縮
			m_cmdOption |= RAROPT_IGNOREDIR;
			break;
		case 'n':		// 新しいファイルのみ
			m_cmdOption |= RAROPT_NEWONLY;
			break;
		case 'o':		// 無条件に上書き
			m_cmdOption |= RAROPT_OVERWRITE;
			break;
		case 'w':		// 正規表現を利用
			m_cmdOption |= RAROPT_REGEXP;
			break;
		case 'y':		// 日時指定
			ch = *++p;
			if (ch != 's' && ch != 'e')
				return FALSE;
			n = (int)wcslen(++p);
			if (n < 8 || n & 1)
				return FALSE;

			memset(&st, 0, sizeof(st));
			n = swscanf(p, L"%4d%2d%2d%2d%2d%2d", &y, &m, &d, &H, &M, &S);
			if (n == EOF || n < 3) { return FALSE; }
			st.wYear = y; st.wMonth = m; st.wDay = d; st.wHour = H; st.wMinute = M; st.wSecond = S;
			switch (n) {
			case 3:	st.wHour         = (ch == 's') ? 0 : 23;
			case 4:	st.wMinute       = (ch == 's') ? 0 : 59;
			case 5: st.wSecond       = (ch == 's') ? 0 : 59;
			case 6: st.wMilliseconds = (ch == 's') ? 0 : 999;
			default:	break;
			}

			if (ch == 's') {
				m_cmdOption |= RAROPT_DTSTART;
				SystemTimeToFileTime(&st, &m_ftStart);
			} else {
				m_cmdOption |= RAROPT_DTEND;
				SystemTimeToFileTime(&st, &m_ftEnd);
			}
			return TRUE;
		case '~':		// デバッグオプション
			m_cmdOption |= RAROPT_DEBUG;
			break;
		default:
			return FALSE;
		}
		p++;
	}
	return TRUE;
}

BOOL CUnrarEnv::PerseCommand(LPCWSTR pszCmd)
{
	WCHAR buf[MAX_PATH];
	GetCurrentDirectory(ARRAYSIZE(buf), buf);
	m_strDefaultPath = buf;

	CStringW str = pszCmd;
	str.Replace('/', '\\');

	strseparator ssp;
	int cnt = ssp.set(str, ' ');
	if (cnt < 1) {
		msgout(L"invalid command line\n");
		return FALSE;
	}

	int index = 0;
	const WCHAR* p = ssp[index];

	// 命令を特定する
	if (*p == '-')
		*p++;
	if (wcslen(p) == 1) {
		switch (*p) {
		case 'x':				// 解凍
		case 'l':				// 格納一覧
		case 'c':				// チェック
		case 't':				// テスト（厳密チェック）
			m_chCmd = *p;
			break;
		default:
			msgout(L"invalid command '%c'\n", *p);
			return FALSE;
		}
		++index;
	}
	else
		m_chCmd = RARCMD_LIST;

	// オプションスイッチ
	m_cmdOption = 0;
	BOOL bOK;
	for (; index < cnt; index++) {
		p = ssp[index];
		if (*p == '-') {
			if (!PerseSwitch(++p)) {
				msgout(L"invalid switch '%s'\n", p);
				return FALSE;
			}
		} else if (*p == '@') {		// ファイルリスト
			p++;
			if (*p == '@') {
				// "@@<respfile>" … UNICODEファイル
				p++;
				bOK = PerseResponseFileW(p);
			} else {
				// "@<respfile>" … ASCII(SJIS)ファイル
				bOK = PerseResponseFile(p);
			}

			if (!bOK) {
				msgout(L"invalid response file '%s'\n", p);
				return FALSE;
			}
		} else {
			LPWSTR q = CharPrevW(p, p + wcslen(p));
			if (*q == '\\') {
				// 行末が \ で終わっていれば基準／ターゲットフォルダ
				m_strBasePath = p;
			} else {
				if (m_strRarName.IsEmpty()) {
					// 最初に現れたものが書庫ファイル
					m_strRarName = p;
				} else {
					// 以降はフィルタ または 格納ファイル名
					m_arrMask.Add(p);
					if (wcschr(p, '*') || wcschr(p, '?'))
						m_cmdOption |= RAROPT_FILTERPRESENTS;
				}
			}
		}
	}

	// キャビネットファイルが指定されていなければエラー
	if (m_strRarName.IsEmpty()) {
		msgout(L"cabinet file not specified.\n");
		return FALSE;
	}

	if (m_chCmd == RARCMD_UNPACK) {
		if (m_strBasePath.IsEmpty())
			m_strBasePath = L".\\";
	}
	return TRUE;
}

int CUnrarEnv::Execute(HWND hwnd, LPCWSTR pszCmd)
{
	m_hwndOwner = hwnd;
	m_strResult.Empty();
	if (!PerseCommand(pszCmd))
		return -1;

	// 書庫がパス未指定の場合
	if (m_strRarName[0] != '\\' && m_strRarName[1] != ':') {
		// 現在のフォルダを付加してフルパス名とする
		WCHAR buf[MAX_PATH * 2];
		PathCombine(buf, m_strDefaultPath, m_strRarName);
		m_strRarName = buf;
	}

	switch (m_chCmd) {
	case RARCMD_UNPACK:
		return CmdUnpack();
	case RARCMD_LIST:
		return CmdList();
	case RARCMD_CHECK:
		return CmdCheck();
	case RARCMD_TEST:
		return CmdTest();
	default:
		break;
	}
	return -1;
}

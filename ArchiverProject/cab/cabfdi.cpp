#include "stdafx.h"
#include "cab.h"
#include "relay.h"


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
	CCabEnv* m_pev;
	int m_nAction;
	BOOL m_bYesToAll;
	CStringW m_strNextCabinet;
	long m_lCopyDone;
	INT_PTR m_hfWrite;
	EXTRACTINGINFOEX m_eie;
	CRelayDlg m_relay;

public:
	CUnpackState()
		: m_pev(NULL)
		, m_nAction(C_QUERY)
		, m_bYesToAll(FALSE)
		, m_hfWrite(NULL)
		, m_lCopyDone(0)
	{
		m_strNextCabinet.Empty();
		memset(&m_eie, 0, sizeof(EXTRACTINGINFOEX));
	}

	~CUnpackState()
	{
		RelDestroy();
	}

	__inline BOOL CheckFlags(DWORD dwFlags) { return (m_pev->m_cmdOption & dwFlags); }
	__inline DWORD GetFlags() { return m_pev->m_cmdOption; }
	__inline void SetFlags(DWORD dwFlags) { m_pev->m_cmdOption = dwFlags; }

	__inline void RelCreate() { m_relay.Create(m_pev->m_hwndOwner); }
	__inline void RelDestroy() { if (m_relay.IsWindow()) m_relay.DestroyWindow(); }
	__inline void RelSetTitle(LPCWSTR pszTitle) { m_relay.SetTitle(pszTitle); }
	__inline void RelSetFile(LPCWSTR pszFile) { m_relay.SetFile(pszFile); }
	__inline void RelSetMessage(LPCWSTR pszMessage) { m_relay.SetMessage(pszMessage); }
	__inline void RelSetRange(long lTotal) { m_relay.SetRange(lTotal); }
	__inline void RelSetComplete(long lDone) { m_relay.SetComplete(lDone); }
	__inline BOOL RelIsCanceled() { return m_relay.IsCanceled(); }

	// FDI コールバック
	static INT_PTR DIAMONDAPI NotifyCallback(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
	{
		CUnpackState* pus = (CUnpackState*)pfdin->pv;
		return  pus->NotifyProc(fdint, pfdin);
	}
	static int DIAMONDAPI DecryptCallback(PFDIDECRYPT pfdid)
	{
		CUnpackState* pus = (CUnpackState*)pfdid->pvUser;
		return  pus->DecryptProc(pfdid);
	}

	INT_PTR NotifyProc(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin);
	int DecryptProc(PFDIDECRYPT pfdid);

	INT_PTR NtCabinetInfo(PFDINOTIFICATION pfdin);
	INT_PTR NtPartialFile(PFDINOTIFICATION pfdin);
	INT_PTR NtNextCabinet(PFDINOTIFICATION pfdin);
	INT_PTR NtCopyFile(PFDINOTIFICATION pfdin);
	INT_PTR NtCloseFileInfo(PFDINOTIFICATION pfdin);
	INT_PTR NtEnumerate(PFDINOTIFICATION pfdin);

	// 復号化
	int DcNewCabinet(PFDIDECRYPT pfdid);
	int DcNewFolder(PFDIDECRYPT pfdid);
	int DcDecrypt(PFDIDECRYPT pfdid);
};


static CUnpackState* spUnpackState = NULL;

//////////////////////////////////////////////////////////////////////
//
//

#define MAXCHECKSIZE				0x40000L		// check 256KB

class CCabReader
{
protected:
	size_t m_size;
	size_t m_orgsize;
	HANDLE m_hmap;
	void* m_map_ptr;
	BYTE* m_head_ptr;
	ULONG_PTR m_cur;

public:
	CStringA m_name;

	CCabReader() :
			m_size(0),
			m_hmap(NULL),
			m_map_ptr(NULL),
			m_head_ptr(NULL),
			m_cur(NULL)
	{
	}
	~CCabReader()
	{
		close();
	}
	BOOL open(LPCSTR pszCabFile)
	{
		if (!pszCabFile || *pszCabFile == 0) return FALSE;

		HANDLE hf = CreateFileA(pszCabFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hf == INVALID_HANDLE_VALUE)
			return FALSE;

		ULARGE_INTEGER orgsize;
		orgsize.LowPart = GetFileSize(hf, &orgsize.HighPart);

		m_name = pszCabFile;
		m_hmap = CreateFileMapping(hf, NULL, PAGE_READONLY, 0, 0, NULL);
		if (m_hmap != NULL) {
			size_t s = (size_t)min(MAXCHECKSIZE, orgsize.QuadPart);
			m_map_ptr = MapViewOfFile(m_hmap, FILE_MAP_READ, 0, 0, 0);
			if (m_map_ptr) {
				register BYTE* pb = (BYTE*)m_map_ptr;
				while (s-- >= sizeof(CFHEADER)) {
					if (*((DWORD*)pb) == 0x4643534d &&
						*((DWORD*)(pb + 4)) == 0 &&
						*((DWORD*)(pb + 8)) != 0)
					{
						m_head_ptr = pb;
						m_size = *((DWORD*)(pb + 8));
						m_cur = 0;
						break;
					}
					pb++;
				}
			}
		}
		CloseHandle(hf);

		if (!m_size) {
			close();
			return FALSE;
		}
		return TRUE;
	}
	UINT read(void* pv, UINT cb)
	{
		if (m_size <= m_cur)
			return 0;

#ifdef _WIN64
		cb = min(cb, (int)(m_size - m_cur));
#else
		cb = min(cb, PtrToUint(m_size - m_cur));
#endif
		memcpy(pv, m_head_ptr + m_cur, cb);
		m_cur += cb;

		return cb;
	}
	long seek(long dist, int seektype)
	{
		switch (seektype) {
		case FILE_BEGIN:	m_cur = dist;			break;
		case FILE_END:		m_cur = m_size + dist;	break;
		case FILE_CURRENT:	m_cur += dist;			break;
		default:									break;
		}
		m_cur = min(m_size, max(0, m_cur));
#ifdef _WIN64
		return (long)m_cur;
#else
		return PtrToLong(m_cur);
#endif
	}
	void close()
	{
		if (m_hmap) {
			if (m_map_ptr)
				UnmapViewOfFile(m_map_ptr);
			CloseHandle(m_hmap);
		}
		m_size = m_orgsize = 0;
		m_hmap = NULL;
		m_map_ptr = NULL;
		m_head_ptr = NULL;
		m_cur = 0;
		m_name.Empty();
	}
};

//////////////////////////////////////////////////////////////////////
//
//

static void fdiGetError(int err, CString& str)
{
	UINT idRes = IDS_FDIERR_UNKNOWN;
	switch (err) {
	case FDIERROR_NONE:
	case FDIERROR_CABINET_NOT_FOUND:
	case FDIERROR_NOT_A_CABINET:
	case FDIERROR_UNKNOWN_CABINET_VERSION:
	case FDIERROR_CORRUPT_CABINET:
	case FDIERROR_ALLOC_FAIL:
	case FDIERROR_BAD_COMPR_TYPE:
	case FDIERROR_MDI_FAIL:
	case FDIERROR_TARGET_FILE:
	case FDIERROR_RESERVE_MISMATCH:
	case FDIERROR_WRONG_CABINET:
	case FDIERROR_USER_ABORT:
		idRes = err + IDS_FDIERR_FIRST;
		break;
	default:
		break;
	}
	str.LoadString(idRes);
}

//////////////////////////////////////////////////////////////////////
//

static void* DIAMONDAPI memAlloc(ULONG cb)
{
	void* pv = malloc(cb);
	if (!pv && spUnpackState)
		spUnpackState->m_pev->msgout(RESFORM_ERR, L"memory", L"insufficient");
	return pv;
}

static void DIAMONDAPI memFree(void* pv)
{
	free(pv);
}

static INT_PTR DIAMONDAPI fdiCreate(LPCWSTR pszFile)
{
	if (!pszFile) return -1;

	HANDLE hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
						CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hf == INVALID_HANDLE_VALUE) {
		if (spUnpackState)
			spUnpackState->m_pev->msgout(RESFORM_ERR, L"create", pszFile);
		return -1;
	}
	return (INT_PTR)hf;
}

static INT_PTR DIAMONDAPI fdiOpen(char* pszFile, int /*oflag*/, int /*pmode*/)
{
	CCabReader* pcr = new CCabReader();
	if (!pcr->open(pszFile)){
		if (spUnpackState)
			spUnpackState->m_pev->msgout(RESFORM_ERR, L"open", pszFile);
		delete pcr;
		return -1;
	}
	return (INT_PTR)pcr;
}

static UINT DIAMONDAPI fdiRead(INT_PTR hf, void* pv, UINT cb)
{
	CCabReader* pcr = (CCabReader*)hf;
	UINT size = pcr->read(pv, cb);
	if (spUnpackState && size != cb) {
		WCHAR wsz[MAX_PATH * 2];
		MultiByteToWideChar(CP_ACP, 0, pcr->m_name, -1, wsz, ARRAYSIZE(wsz));
		spUnpackState->m_pev->msgout(RESFORM_ERR, L"read", wsz);
	}
	return size;
}

static UINT DIAMONDAPI fdiWrite(INT_PTR hf, void* pv, UINT cb)
{
	DWORD dw;
	if (WriteFile((HANDLE)hf, (LPCVOID)pv, cb, &dw, NULL) && dw == cb) {
		if (spUnpackState && spUnpackState->m_hfWrite == hf) {
			spUnpackState->m_lCopyDone += cb;
			spUnpackState->RelSetComplete(spUnpackState->m_lCopyDone);
			spUnpackState->m_eie.exinfo.dwWriteSize = spUnpackState->m_lCopyDone;

			// コールバックチェック
			if (!NotifyOwnerMessage(ARCMES_PROCESSING_FILE, &spUnpackState->m_eie)) {
				spUnpackState->m_pev->msgout(RESFORM_ERR, sszAbort, spUnpackState->m_pev->m_szTemp);
				return (UINT)-1;
			}
		}
	}
	else {
		spUnpackState->m_pev->msgout(RESFORM_ERR, L"write", spUnpackState->m_pev->m_szTemp);
		dw = (UINT)-1;
	}
	return (UINT)dw;
}

static int DIAMONDAPI fdiClose(INT_PTR hf)
{
	if (spUnpackState && spUnpackState->m_hfWrite == hf) {
		if (!CloseHandle((HANDLE)hf)) {
			spUnpackState->m_pev->msgout(RESFORM_ERR, L"close", spUnpackState->m_pev->m_szTemp);
			return -1;
		}
		return 0;
	}

	CCabReader* pcr = (CCabReader*)hf;
	pcr->close();
	delete pcr;
	return 0;
}

static long DIAMONDAPI fdiSeek(INT_PTR hf, long dist, int seektype)
{
	CCabReader* pcr = (CCabReader*)hf;
	return pcr->seek(dist, seektype);
}

INT_PTR CUnpackState::NtCabinetInfo(PFDINOTIFICATION pfdin)
{
	CStringW str1, str2, str3;
	StrToWStr(pfdin->psz1, str1);
	StrToWStr(pfdin->psz2, str2);
	StrToWStr(pfdin->psz3, str3);

	if (CheckFlags(CABOPT_DEBUG)) {
		m_pev->msgout(
				L"[cabinet information for %s]\n"
				L"  next cabinet     = %s\n"
				L"  next disk        = %s\n"
				L"  cabinet path     = %s\n"
				L"  cabinet set ID   = %d\n"
				L"  cabinet # in set = %d\n\n",
				(LPCWSTR)m_pev->m_strCabName,
				(LPCWSTR)str1,
				(LPCWSTR)str2,
				(LPCWSTR)str3,
				pfdin->setID,
				pfdin->iCabinet	);
	}

	return 0;
}

INT_PTR CUnpackState::NtPartialFile(PFDINOTIFICATION pfdin)
{
	if (CheckFlags(CABOPT_DEBUG)) {
		CStringW str1, str2, str3;
		StrToWStr(pfdin->psz1, str1);
		StrToWStr(pfdin->psz2, str2);
		StrToWStr(pfdin->psz3, str3);
		m_pev->msgout(
				L"[partial file]\n"
				L"  name of continued file            = %s\n"
				L"  name of cabinet where file starts = %s\n"
				L"  name of disk where file starts    = %s\n\n",
				(LPCWSTR)str1,
				(LPCWSTR)str2,
				(LPCWSTR)str3);
	}
	return 0;
}

INT_PTR CUnpackState::NtNextCabinet(PFDINOTIFICATION pfdin)
{
	CStringW str1, str2, str3;
	StrToWStr(pfdin->psz1, str1);
	StrToWStr(pfdin->psz2, str2);
	StrToWStr(pfdin->psz3, str3);

	if (CheckFlags(CABOPT_DEBUG)) {
		m_pev->msgout(
				L"[next cabinet]\n"
				L"  name of next cabinet where file continued = %s\n"
				L"  name of next disk where file continued    = %s\n"
				L"  cabinet path name                         = %s\n\n",
				(LPCWSTR)str1,
				(LPCWSTR)str2,
				(LPCWSTR)str3);
	}

	// 次のキャビネットファイルが見つからなければ問い合わせる
	WCHAR nextcab[MAX_PATH * 2];
	PathCombine(nextcab, str3, str1);

	if (!PathFileExists(nextcab)) {
		if (!FindNextCab(m_pev->m_hwndOwner, nextcab)) {
			m_pev->msgout(RESFORM_ERR, sszAbort, nextcab);
			return -1;
		}

		if (wcsrchr(nextcab, L'\\')) {
			str1 = nextcab;
			WCHAR path[MAX_PATH * 2];
			wcscpy(path, str1);
			PathRemoveFileSpec(path);
			PathAddBackslash(path);
			WideCharToMultiByte(CP_ACP, 0, path, -1, pfdin->psz3, 256, NULL, NULL);
		}
	}
	RelSetTitle(nextcab);

	if (CheckFlags(CABOPT_KEEPNEXT))
		m_strNextCabinet = nextcab;

	return 0;
}

INT_PTR CUnpackState::NtCopyFile(PFDINOTIFICATION pfdin)
{
	WCHAR buf[MAX_PATH * 2], cabin[MAX_PATH * 2];

	if (pfdin->attribs & _A_NAME_IS_UTF)
		DecodeUTFToWStr((BYTE*)pfdin->psz1, -1, cabin, ARRAYSIZE(cabin));
	else
		MultiByteToWideChar(CP_ACP, 0, pfdin->psz1, -1, cabin, ARRAYSIZE(cabin));

	// マッチング処理
	CMatchSpec ms(m_pev->m_strMask, CheckFlags(CABOPT_REGEXP), FALSE, ';');
	if (!ms.Match(cabin))
		return 0;		// 一致しなければこのフィルタorファイルをスキップ

	// 一致するファイルが見つかったので、これを解凍するための準備をおこなう
	RelSetFile(cabin);
	RelSetRange(pfdin->cb);

	if (CheckFlags(CABOPT_IGNOREDIR)) {
		// フォルダを無視して解凍する場合
		WCHAR* pw = wcsrchr(cabin, L'\\');
		if (pw)
			pw++;
		else
			pw = cabin;
		m_pev->BuildTempPathName(pw);
	}
	else {
		int n = m_pev->BuildTempPathName(cabin);
		if (n < 0) {
			m_pev->msgout(RESFORM_ERR, L"validate", cabin);
			return -1;
		}
		else if (n == 0)
			return 0;

		// 解凍先にフォルダが見つからなければ作成する
		wcscpy(buf, m_pev->m_szTemp);
		PathRemoveFileSpec(buf);
		if (!PathIsDirectory(buf)) {
			if (!MakeTree(buf)) {
				m_pev->msgout(RESFORM_ERR, L"mkdir", m_pev->m_szTemp);
				return -1;
			}
		}
	}

	// コールバックチェック
	memset(&m_eie, 0, sizeof(EXTRACTINGINFOEX));
	m_eie.exinfo.dwFileSize = pfdin->cb;
	m_eie.wDate = pfdin->date;
	m_eie.wTime = pfdin->time;
	wcscpy(m_eie.exinfo.szSourceFileName, cabin);
	wcscpy(m_eie.exinfo.szDestFileName, m_pev->m_szTemp);
	AttrToString(m_eie.szAttribute, pfdin->attribs);
	wcscpy(m_eie.szMode, L"Cab");
	if (!NotifyOwnerMessage(ARCMES_START_FILE, &m_eie)) {
		m_pev->msgout(RESFORM_ERR, sszAbort, m_pev->m_szTemp);
		return -1;
	}

	// 同名ファイルの存在
	CWin32FindData wfdDst;
	if (GetFileData(m_pev->m_szTemp, &wfdDst)) {
		if (!m_bYesToAll) {
			// 同名処理の選択
			WIN32_FIND_DATA wfdSrc;
			wfdSrc.nFileSizeLow = pfdin->cb;
			DosDateTimeToFileTime(pfdin->date, pfdin->time, &wfdSrc.ftLastWriteTime);

			REPLDATA rd;
			memset(&rd, 0, sizeof(rd));
			rd.lpSrc = cabin;
			rd.lpDst = m_pev->m_szTemp;
			rd.pwfdSrc = &wfdSrc;
			rd.pwfdDst = &wfdDst;
			rd.bMultiDest = TRUE;
			rd.nAction = m_nAction;
			if (!QueryReplace(m_pev->m_hwndOwner, &rd)) {
				m_pev->msgout(RESFORM_ERR, sszAbort, m_pev->m_szTemp);
				return -1;
			}

			m_bYesToAll = rd.bYesToAll;
			m_nAction = rd.nAction;
		}

		if (m_nAction == C_IGNORE) {
			m_pev->msgout(RESFORM_SKIP, m_pev->m_szTemp);
			return 0;
		}
		else if (m_nAction == C_NEWONLY) {
			// 新しいファイルのみの場合
			FILETIME ft;
			DosDateTimeToFileTime(pfdin->date, pfdin->time, &ft);
			if (CompareFileTime(&wfdDst.ftLastWriteTime, &ft) >= 0) {
				m_pev->msgout(RESFORM_SKIP, m_pev->m_szTemp);
				return 0;
			}
		}

		if (!RemoveFileNoConfirm(m_pev->m_szTemp)) {
			m_pev->msgout(RESFORM_ERR, L"delete", m_pev->m_szTemp);
			return -1;
		}
	}

	m_hfWrite = fdiCreate(m_pev->m_szTemp);
	return m_hfWrite;
}

INT_PTR CUnpackState::NtCloseFileInfo(PFDINOTIFICATION pfdin)
{
	if (CheckFlags(CABOPT_DEBUG)) {
		CStringW str;
		StrToWStr(pfdin->psz1, str);
		m_pev->msgout(
				L"[close file information]\n"
				L"  file name in cabinet = %s\n\n",
				(LPCWSTR)str);
	}

	fdiClose(pfdin->hf);
	m_hfWrite = 0;

	FILETIME ft;
	DosDateTimeToFileTime(pfdin->date, pfdin->time, &ft);
	SetFileDateTime(m_pev->m_szTemp, &ft, &ft);
	DWORD attr = pfdin->attribs;
	attr &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
	SetFileAttributes(m_pev->m_szTemp, attr);

	m_pev->msgout(RESFORM_OK, m_pev->m_szTemp);

	// コールバックチェック
	m_eie.exinfo.dwWriteSize = m_eie.exinfo.dwFileSize;
	if (!NotifyOwnerMessage(ARCMES_EXIT_ARCHIVE, &m_eie))
		return -1;

	return TRUE;
}

INT_PTR CUnpackState::NtEnumerate(PFDINOTIFICATION pfdin)
{
	if (CheckFlags(CABOPT_DEBUG)) {
		m_pev->msgout(
				L"[enumerate]\n"
				L"  CFFILE position      = %d\n"
				L"  # of files remaining = %d\n"
				L"  cabinet setID        = %d\n\n",
				pfdin->cb,
				pfdin->iFolder,
				pfdin->setID);
	}
	return 0;
}

INT_PTR CUnpackState::NotifyProc(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
{
	switch (fdint) {
	case fdintCABINET_INFO:		return NtCabinetInfo(pfdin);
	case fdintPARTIAL_FILE:		return NtPartialFile(pfdin);		// first file in cabinet is continuation
	case fdintNEXT_CABINET:		return NtNextCabinet(pfdin);		// file continued to next cabinet
	case fdintCOPY_FILE:		return NtCopyFile(pfdin);
	case fdintCLOSE_FILE_INFO:	return NtCloseFileInfo(pfdin);
	case fdintENUMERATE:		return NtEnumerate(pfdin);
	}
	return 0;
}

int CUnpackState::DcNewCabinet(PFDIDECRYPT pfdid)
{
	if (CheckFlags(CABOPT_DEBUG)) {
		m_pev->msgout(
				L"[dec_cabinet]\n"
				L"  CFHEADER reserve = %d\n"
				L"  cabinet number   = %d\n"
				L"  cabinet setID    = %d\n\n",
				pfdid->cabinet.cbHeaderReserve,
				pfdid->cabinet.iCabinet,
				pfdid->cabinet.setID);
	}
	return 1;
}

int CUnpackState::DcNewFolder(PFDIDECRYPT pfdid)
{
	if (CheckFlags(CABOPT_DEBUG)) {
		m_pev->msgout(
				L"[dec_folder]\n"
				L"  CFFOLDER reserve = %d\n"
				L"  folder number    = %d\n\n",
				pfdid->folder.cbFolderReserve,
				pfdid->folder.iFolder);
	}
	return 1;
}

int CUnpackState::DecryptProc(PFDIDECRYPT pfdid)
{
	switch (pfdid->fdidt) {
	case fdidtNEW_CABINET:	return DcNewCabinet(pfdid);			// New cabinet
	case fdidtNEW_FOLDER:	return DcNewFolder(pfdid);			// New folder
	case fdidtDECRYPT:		return DcDecrypt(pfdid);			// Decrypt a data block
	}
	return 1;
}

int CUnpackState::DcDecrypt(PFDIDECRYPT pfdid)
{
	if (CheckFlags(CABOPT_DEBUG)) {
		m_pev->msgout(
				L"[dec_decrypt]\n"
				L"  CFDATA reserve   = %d\n"
				L"  CFDATA size      = %d\n"
				L"  split data block = %d\n"
				L"  partial numbetr  = %d\n\n",
				pfdid->decrypt.cbDataReserve,
				pfdid->decrypt.cbData,
				pfdid->decrypt.fSplit,
				pfdid->decrypt.cbPartial);
	}
	return 1;
}

INT_PTR DIAMONDAPI FDIEnumerateCallback(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
{
	CCabItem* item;
	CCabFolder* pcf = (CCabFolder*)pfdin->pv;

	switch (fdint) {
	case fdintCABINET_INFO:		// General information about cabinet
		if (*pfdin->psz1) {
			char buf[MAX_PATH * 2];
			PathCombineA(buf, pfdin->psz3, pfdin->psz1);

			StrToWStr(buf, pcf->m_strNextCabinet);
			pcf->m_cabState |= CABSTS_NEXTCABINET;
		}
		break;

	case fdintPARTIAL_FILE:		// First file in cabinet is continuation
		pcf->m_cabState |= CABSTS_PREVCABINET;
		break;

	case fdintCOPY_FILE:
	{
		WCHAR wsz[MAX_PATH * 2];
		MultiByteToWideChar(pfdin->attribs & _A_NAME_IS_UTF ? CP_UTF8 : CP_ACP, 0, pfdin->psz1, -1, wsz, ARRAYSIZE(wsz));

		item = new CCabItem;
		if (item) {
			item->SetItem(wsz, pfdin);
			pcf->AddItem(item);
		}
		break;
	}

	case fdintCLOSE_FILE_INFO:	// close the file, set relevant info
	case fdintNEXT_CABINET:		// File continued to next cabinet
	case fdintENUMERATE:		// Enumeration status
	default:
		break;
	}
	return 0;
}

CCabFolder* FDIOpenCabinet(HWND hwndOwner, LPCWSTR pszCabinetFile, DWORD mode)
{
	CCabFolder* pcf = new CCabFolder();
	if (pcf) {
		GetFileData(pszCabinetFile, &pcf->m_wfd);
		if (!pcf->Initialize(hwndOwner, mode, pszCabinetFile)) {
			delete pcf;
			return NULL;
		}

		CStringW path;
		ERF erf;
		HFDI hfdi = FDICreate(memAlloc, memFree, fdiOpen, fdiRead, fdiWrite, fdiClose, fdiSeek, cpu80386, &erf);
		if (hfdi) {
			CStringW name;
			path = pszCabinetFile;
			int pos = path.ReverseFind(_T('\\'));
			if (pos >= 0) {
				name = path.Mid(pos + 1);
				path.ReleaseBuffer(pos);
				if (path[path.GetLength() - 1] != _T('\\'))
					path += _T('\\');
			}
			else {
				name = pszCabinetFile;
				path.Empty();
			}

			CStringA strCabPath(path);
			CStringA strCabName(name);
			if (!FDICopy(hfdi, (char*)strCabName.GetString(), (char*)strCabPath.GetString(), 0, FDIEnumerateCallback, NULL, pcf)) {
				delete pcf;
				pcf = NULL;
			}
			FDIDestroy(hfdi);
		}
	}
	return pcf;
}

BOOL FDIGetCabinetInfo(LPCWSTR pszCab, FDICABINETINFO* pinfo)
{
	if (!pinfo || IsBadWritePtr(pinfo, sizeof(FDICABINETINFO)))
		return FALSE;

	BOOL bOK = FALSE;
	ERF erf;
	HFDI hfdi = FDICreate(
						memAlloc,
						memFree,
						fdiOpen,
						fdiRead,
						fdiWrite,
						fdiClose,
						fdiSeek,
						cpu80386,
						&erf);
	if (hfdi) {
		CStringA strCab(pszCab);
		INT_PTR hf = fdiOpen((char*)strCab.GetString(), 0, 0);
		if (hf != -1) {
			bOK = FDIIsCabinet(hfdi, hf, pinfo);
			fdiClose(hf);
		}
		FDIDestroy(hfdi);
	}
	return bOK;
}

BOOL CCabEnv::FdiExtract()
{
	CRunning running;

	CUnpackState cus;
	cus.m_pev = this;

	if (PathIsDirectory(m_strCabName)) {
		msgout(RESFORM_ERR, L"extract", (LPCWSTR)m_strCabName);
		return FALSE;
	}

	if (cus.CheckFlags(CABOPT_NEWONLY)) {
		cus.m_nAction = C_NEWONLY;
		cus.m_bYesToAll = TRUE;
	}
	if (cus.CheckFlags(CABOPT_OVERWRITE)) {
		cus.m_nAction = C_OVERWRITE;
		cus.m_bYesToAll = TRUE;
	}

	if (!cus.CheckFlags(CABOPT_SILENT)) {
		cus.RelCreate();
		cus.RelSetTitle(m_strCabName);
	}

	// 解凍対象ファイルが指定されていない場合には、
	// 全ファイルが解凍の対象となる
	if (m_arrMask.GetSize() < 1) {
		m_arrMask.Add(gszAst);
		m_cmdOption |= CABOPT_FILTERPRESENTS;
	}
	m_strMask.Empty();
	int i, cnt = m_arrMask.GetSize();
	for (i = 0; i < cnt; i++) {
		if (i > 0)
			m_strMask += ';';
		m_strMask += m_arrMask[i];
	}

	spUnpackState = &cus;

	CStringW str;
	str.LoadString(IDS_CAB_EXTRACTING);
	cus.RelSetMessage(str);

	BOOL bOK = FALSE;
	WCHAR buf[MAX_PATH * 2];
	CStringA strCabPath, strCabName;
	while (1) {
		// パス名
		wcscpy(buf, m_strCabName);
		PathRemoveFileSpec(buf);
		PathAddBackslash(buf);
		WStrToStr(buf, strCabPath);
		// ファイル名
		strCabName = PathFindFileName(m_strCabName);

		CWaitingCursor wc;

		bOK = FALSE;
		ERF erf;
		HFDI hfdi = FDICreate(memAlloc, memFree, fdiOpen, fdiRead, fdiWrite, fdiClose, fdiSeek, cpu80386, &erf);
		if (hfdi) {
			bOK = FDICopy(
						hfdi,
						(char*)strCabName.GetString(),
						(char*)strCabPath.GetString(),
						0,
						cus.NotifyCallback,
						cus.DecryptCallback,
						&cus);
			if (!bOK) {
				CString err;
				fdiGetError(erf.erfOper, err);
				cus.m_pev->msgout(RESFORM_ERR, sszAbort, err.GetString());
			}
			FDIDestroy(hfdi);
		}

		if (bOK &&
			cus.CheckFlags(CABOPT_KEEPNEXT) &&
			cus.CheckFlags(CABOPT_FILTERPRESENTS))
		{
			// マルチ書庫の終了判定
			// 現在の書庫と次の書庫が同一なら終了
			if (!m_strCabName.CompareNoCase(cus.m_strNextCabinet))
				break;
			m_strCabName = cus.m_strNextCabinet;
		}
		else
			break;
	}
	spUnpackState = NULL;
	return bOK;
}

extern "C"
BOOL WINAPI CabFdiA(HWND hwnd, LPCSTR pszCab, LPCSTR pszPath, LPCSTR pszFiles, DWORD dwFlags)
{
	CStringW cab, path, files;
	StrToWStr(pszCab, cab);
	StrToWStr(pszPath, path);
	StrToWStr(pszFiles, files);

	return CabFdiW(hwnd, cab, path, files, dwFlags);
}

extern "C"
BOOL WINAPI CabFdi(HWND hwnd, LPCSTR pszCab, LPCSTR pszPath, LPCSTR pszFiles, DWORD dwFlags)
{
	return CabFdiA(hwnd, pszCab, pszPath, pszFiles, dwFlags);
}

extern "C"
BOOL WINAPI CabFdiW(HWND hwnd, LPCWSTR pszCab, LPCWSTR pszPath, LPCWSTR pszFiles, DWORD dwFlags)
{
	CCabEnv ce;
	ce.m_hwndOwner = hwnd;

	WCHAR buf[MAX_PATH * 2];
	wcscpy(buf, pszPath);
	PathAddBackslash(buf);
	ce.m_strBasePath = buf;
	ce.m_strCabName = pszCab;

	const WCHAR* p = pszFiles;
	while (p && *p) {
		ce.m_arrMask.Add(p);
		p += wcslen(p) + 1;
	}

	ce.m_chCmd = CABCMD_UNPACK;
	ce.m_cmdOption = dwFlags;
	return ce.FdiExtract();
}

extern "C"
void WINAPI CabFdiHereA(HWND hwnd, HINSTANCE hInst, LPCSTR lpCmdLine, int nCmdShow)
{
	CStringW cmd;
	StrToWStr(lpCmdLine, cmd);
	CabFdiHereW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI CabFdiHere(HWND hwnd, HINSTANCE hInst, LPCSTR lpCmdLine, int nCmdShow)
{
	CabFdiHereA(hwnd, hInst, lpCmdLine, nCmdShow);
}

extern "C"
void WINAPI CabFdiHereW(HWND hwnd, HINSTANCE /*hInst*/, LPCWSTR lpCmdLine, int /*nCmdShow*/)
{
	CStringW str;
	str.Format(L"-x %s", lpCmdLine);
	CabW(hwnd, str, NULL, 0);
}

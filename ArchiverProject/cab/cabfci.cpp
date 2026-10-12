//////////////////////////////////////////////////////////////////////
// fciutl.cpp

#include "stdafx.h"
#include "cab.h"
#include "relay.h"


//////////////////////////////////////////////////////////////////////
// When a CAB file reaches this size, a new CAB will be created
// automatically.  This is useful for fitting CAB files onto disks.
//
// If you want to create just one huge CAB file with everything in
// it, change this to a very very large number.
//


//////////////////////////////////////////////////////////////////////
// When a folder has this much compressed data inside it,
// automatically flush the folder.
//
// Flushing the folder hurts compression a little bit, but
// helps random access significantly.
//
#define FOLDER_THRESHOLD	900000

enum {
	CST_NONE,
	CST_COMPRESS,
	CST_COPY
};

static const WCHAR* sszMsg[] = {
	L"Creating: %s\n",
	L"  adding(%s): %s\n",
	L"  moving(%s): %s\n",
	L"  error: %s\n",
	L"  placed: %s on %s\n",
	L"  (segment continued)\n",
	L"  skipped: %s\n",
	L"  not found: %s\n",
};
static const WCHAR* sszCompressionType[] = {
	L"stored",
	L"mszip",
	L"quantum",
	L"lzx",
};


//////////////////////////////////////////////////////////////////////
// Our internal state
//
// The FCI APIs allow us to pass back a state pointer of our own
//
class CPackState
{
public:
	CPackState()
		: m_pev(NULL)
		, m_lCopyDone(0)
		, m_nProcessingStat(CST_NONE)
		, m_hfci(NULL)
	{
	}

	~CPackState()
	{
		RelDestroy();
	}

	void SetCabFileName(PCCAB pccb);
	void SetCabParams(CCAB& ccb, LPCWSTR pDiskName, LPCWSTR pszPath);
	BOOL DoCompress(LPCWSTR pszPath, LPCWSTR pszFilter);

	__inline BOOL CheckFlags(DWORD dwFlags) { return (m_pev->m_cmdOption & dwFlags); }

	__inline void RelCreate() { m_relay.Create(m_pev->m_hwndOwner); }
	__inline void RelDestroy() { if (m_relay.IsWindow()) m_relay.DestroyWindow(); }
	__inline void RelSetTitle(LPCWSTR pszTitle) { m_relay.SetTitle(pszTitle); }
	__inline void RelSetFile(LPCWSTR pszFile) { m_relay.SetFile(pszFile); }
	__inline void RelSetMessage(UINT idResource) { m_relay.SetMessage(idResource); }
	__inline void RelSetMessage(LPCWSTR pszMessage) { m_relay.SetMessage(pszMessage); }
	__inline void RelSetRange(long lTotal) { m_relay.SetRange(lTotal); }
	__inline void RelSetComplete(long lDone) { m_relay.SetComplete(lDone); }
	__inline BOOL RelIsCanceled() { return m_relay.IsCanceled(); }

	void AddList(LPCWSTR pName)
	{
		if (pName && *pName)
			m_packlist.Add(pName);
	}

	bool isListed(LPCWSTR pName)
	{
		if (pName && *pName) {
			int cnt = m_packlist.GetSize();
			for (int i = 0; i < cnt; i++) {
				if (!_wcsicmp(pName, m_packlist[i]))
					return true;
			}
		}
		return false;
	}

	CCabEnv* m_pev;
	ULONG m_lCopyDone;
	int m_nProcessingStat;
	HFCI m_hfci;
	ERF m_erf;
	CStringW m_strFilter;
	CSimpleArray<CStringW> m_packlist;
	CRelayDlg m_relay;
};


//////////////////////////////////////////////////////////////////////
//

void fciGetError(int err, CString& str)
{
	UINT idRes = IDS_FCIERR_UNKNOWN;
	switch (err) {
	case FCIERR_NONE:
	case FCIERR_OPEN_SRC:
	case FCIERR_READ_SRC:
	case FCIERR_ALLOC_FAIL:
	case FCIERR_TEMP_FILE:
	case FCIERR_BAD_COMPR_TYPE:
	case FCIERR_CAB_FILE:
	case FCIERR_USER_ABORT:
	case FCIERR_MCI_FAIL:
		idRes = err + IDS_FCIERR_FIRST;
		break;
	default:
		break;
	}
	str.LoadString(idRes);
}

void* DIAMONDAPI fciAlloc(ULONG cb)
{
	return malloc(cb);
}

void DIAMONDAPI fciFree(void* pv)
{
	free(pv);
}

INT_PTR DIAMONDAPI fciOpen(char* pszFile, int oflag, int pmode, int* err, void* /*pv*/)
{
	int result = _open(pszFile, oflag, pmode);
	if (result == -1)
		*err = errno;
	return (INT_PTR)result;
}

UINT DIAMONDAPI fciRead(INT_PTR hf, void* memory, UINT cb, int* err, void* /*pv*/)
{
	unsigned int result = (unsigned int)_read((int)hf, memory, cb);
	if (result != cb)
		*err = errno;
	return result;
}

UINT DIAMONDAPI fciWrite(INT_PTR hf, void* memory, UINT cb, int* err, void* /*pv*/)
{
	unsigned int result = (unsigned int)_write((int)hf, memory, cb);
	if (result != cb)
		*err = errno;
	return result;
}

int DIAMONDAPI fciDelete(char* pszFile, int* err, void* /*pv*/)
{
	int result = remove(pszFile);
	if (result != 0)
		*err = errno;
	return result;
}

long DIAMONDAPI fciSeek(INT_PTR hf, long dist, int seektype, int* err, void* /*pv*/)
{
	long result = _lseek((int)hf, dist, seektype);
	if (result == -1)
		*err = errno;
	return result;
}

int DIAMONDAPI fciClose(INT_PTR hf, int* err, void* /*pv*/)
{
	int result = _close((int)hf);
	if (result != 0)
		*err = errno;
	return result;
}

/*
 * File placed function called when a file has been committed
 * to a cabinet
 */
int DIAMONDAPI fciFilePlaced(PCCAB pccb, char* pszFile, long /*cbFile*/, BOOL fContinuation, void* pv)
{
	CPackState* pps = (CPackState*)pv;

	if (pps->CheckFlags(CABOPT_MULTIVOL)) {
		CString cab, file;
		StrToWStr(pccb->szCab, cab);
		StrToWStr(pszFile, file);

		CString str;
		str.Format(sszMsg[4], file.GetString(), cab.GetString());
		if (fContinuation)
			str += sszMsg[5];
		pps->m_pev->msgout(str);
	}
	return 0;
}

BOOL DIAMONDAPI fciGetTempFile(char* pszTempName, int cbTempName, void* pv)
{
	CPackState* pps = (CPackState*)pv;

	WCHAR path[MAX_PATH * 2];
	if (!pps->m_pev->GetWorkingPath(path, ARRAYSIZE(path)))
		GetTempPath(ARRAYSIZE(path), path);

	for (int i = 0; i < 0x1000; i++) {
		CString str;
		str.Format(L"~cb%03X.tmp", i);

		WCHAR temp[MAX_PATH * 2];
		PathCombine(temp, path, str);
		if (!PathFileExists(temp)) {
			WideCharToMultiByte(CP_ACP, 0, temp, -1, pszTempName, cbTempName, NULL, NULL);
			return TRUE;
		}
	}
    return FALSE;
}

long DIAMONDAPI fciStatus(UINT typeStatus, ULONG cb1, ULONG cb2, void* pv)
{
	CPackState* pps = (CPackState*)pv;
	if (pps->RelIsCanceled())
		return -1;

	if (typeStatus == statusFile) {
		if (pps->m_nProcessingStat != CST_COMPRESS) {
			pps->RelSetMessage(IDS_CAB_COMPRESSING);
			pps->m_nProcessingStat = CST_COMPRESS;
		}

		pps->m_lCopyDone += cb2;
		pps->RelSetComplete(pps->m_lCopyDone);
	}
	else if (typeStatus == statusFolder) {
		if (pps->m_nProcessingStat != CST_COPY) {
			pps->RelSetMessage(IDS_CAB_COPYING);
			pps->m_nProcessingStat = CST_COPY;
			pps->RelSetRange(cb2);
		}

		// Adding a folder to a cabinet
		//     cb1 = amount of folder copied to cabinet so far
		//     cb2 = total size of folder
		pps->RelSetComplete(cb1);
	}
	return 0;
}

BOOL DIAMONDAPI fciGetNextCabinet(PCCAB pccb, ULONG /*cbPrevCab*/, void* pv)
{
	CPackState* pps = (CPackState*)pv;

	// Cabinet counter has been incremented already by FCI

	// Store next cabinet name
	pps->SetCabFileName(pccb);

	CString cabpath, cab;
	StrToWStr(pccb->szCabPath, cabpath);
	StrToWStr(pccb->szCab, cab);

	pps->RelSetTitle(cab);

	// 書庫本体は格納の対象外
	WCHAR buf[MAX_PATH * 2];
	PathCombine(buf, cabpath, cab);
	pps->AddList(buf);

	// You could change the disk name here too, if you wanted

	return TRUE;
}

INT_PTR DIAMONDAPI fciGetOpenInfo(char* pszName, USHORT* pdt, USHORT* ptm, USHORT* pattr, int* /*err*/, void* /*pv*/)
{
	// pszName は WCHAR型
	LPCWSTR pwzName = (LPCWSTR)pszName;

	// Need a Win32 type handle to get file date/time
	// using the Win32 APIs, even though the handle we
	// will be returning is of the type compatible with _open

	INT_PTR res = -1;
	*pattr = 0;
	*pdt = 0;
	*ptm = 0;
	CFileFinder ff;
	if (ff.BeginFind(pwzName)) {
		ff.EndFind();

		FILETIME ft;
		ff.LocalLastWriteTime(&ft);
		FileTimeToDosDateTime(&ft, pdt, ptm);

		// Mask out all other bits except these four, since other
		// bits are used by the cabinet format to indicate a
		// special meaning.
		*pattr = (USHORT)(ff.Attrib() & (_A_RDONLY | _A_SYSTEM | _A_HIDDEN | _A_ARCH));
		if (!IsLosslessUnicodeName(pwzName))
			*pattr |= _A_NAME_IS_UTF;

		res = (INT_PTR)_wopen(pwzName, _O_RDONLY | _O_BINARY);
	}
	return res;
}

void CPackState::SetCabFileName(PCCAB pccb)
{
	if (CheckFlags(CABOPT_MULTIVOL) || pccb->iCab > 1)
		wsprintfA(pccb->szCab, "%s%02d.cab", pccb->szDisk, pccb->iCab);
	else
		wsprintfA(pccb->szCab, "%s.cab", pccb->szDisk);
}

void CPackState::SetCabParams(CCAB& ccb, LPCWSTR pDiskName, LPCWSTR pszPath)
{
	memset(&ccb, 0, sizeof(CCAB));

	ccb.cb = m_pev->m_nVolumeSize;
	ccb.cbFolderThresh = FOLDER_THRESHOLD;

	// Don't reserve space for any extensions
	ccb.cbReserveCFHeader = sizeof(HEADEREX);
	ccb.cbReserveCFFolder = 0;
	ccb.cbReserveCFData   = 0;

	// We use this to create the cabinet name
	ccb.iCab = 1;

	// If you want to use disk names, use this to count disks
	ccb.iDisk = 0;

	// Choose your own number
	ccb.setID = m_pev->m_sSetID;

	// 書庫フルパス名を構築
	WCHAR full[MAX_PATH * 2];
	PathCombine(full, pszPath, pDiskName);
	// 書庫本体は格納の対象外
	AddList(full);

	// Only important if CABs are spanning multiple
	// disks, in which case you will want to use a
	// real disk name.
	// Can be left as an empty string.
	WideCharToMultiByte(CP_ACP, 0, pDiskName, -1, ccb.szDisk, ARRAYSIZE(ccb.szDisk), NULL, NULL);
	// where to store the created CAB files
	WideCharToMultiByte(CP_ACP, 0, pszPath, -1, ccb.szCabPath, ARRAYSIZE(ccb.szCabPath), NULL, NULL);
	// store name of first CAB file
	SetCabFileName(&ccb);
}

BOOL CPackState::DoCompress(LPCWSTR pszPath, LPCWSTR pszFilter)
{
	// ワイルドカードを使ったマッチング処理
	CMatchSpec ms(pszFilter, CheckFlags(CABOPT_REGEXP), FALSE, ';');
	CMatchSpec msexc(m_pev->m_strExcludes, CheckFlags(CABOPT_REGEXP), FALSE, ';');
	CMatchSpec msinc(m_pev->m_strStored, CheckFlags(CABOPT_REGEXP), FALSE, ';');

	WCHAR buf[MAX_PATH * 2];
//@	PathCombineW(buf, pszPath, L"*.*");
	PathCombineW(buf, pszPath, pszFilter);

	CFileFinder ff;
	if (ff.BeginFind(buf)) {
		do {
			// System/Hidden属性のチェック （ファイル・フォルダの区別はしない）
			if (CheckFlags(CABOPT_EXC_SYSHIDDEN) && ff.IsSysHidden())
				continue;

			if (ff.IsFolder()) {
				if (!ff.IsDot() && CheckFlags(CABOPT_RECURSEDIR)) {
					PathCombineW(buf, pszPath, ff.cFileName);
					if (!DoCompress(buf, pszFilter)) {
						ff.EndFind();
						return FALSE;
					}
				}
			}
			else {
				bool bMatched = false;
				bool bAltName = false;
				if (ms.Match(ff.cFileName))
					bMatched = true;
				else {
					if (ff.cAlternateFileName[0] && ms.Match(ff.cAlternateFileName)) {
						bMatched = true;
						bAltName = true;
					}
				}

				if (!bMatched) continue;

				// 日時範囲のチェック
				FILETIME ft;
				FileTimeToLocalFileTime(&ff.ftLastWriteTime, &ft);
				if (CheckFlags(CABOPT_DTSTART)) {
					if (CompareFileTime(&m_pev->m_ftStart, &ft) >= 0) {
						m_pev->msgout(sszMsg[6], ff.cFileName);
						continue;
					}
				}
				if (CheckFlags(CABOPT_DTEND)) {
					if (CompareFileTime(&m_pev->m_ftEnd, &ft) <= 0) {
						m_pev->msgout(sszMsg[6], ff.cFileName);
						continue;
					}
				}

				WCHAR rel[MAX_PATH * 2], full[MAX_PATH * 2], *p;
				PathCombineW(rel, pszPath, ff.cFileName);
				GetFullPathName(rel, ARRAYSIZE(full), full, &p);

				// 短い名前で記録
				if (CheckFlags(CABOPT_DOSNAME)) {
					GetShortPathName(full, full, ARRAYSIZE(full));
					CharUpperW(full);
					GetShortPathName(rel, rel, ARRAYSIZE(rel));
					CharUpperW(rel);
				}

				// リスト登録済みかチェック
				if (isListed(full))
					continue;

				if (CheckFlags(CABOPT_IGNOREDIR))		// パスを記録しない
					wcscpy(buf, PathFindFileNameW(full));
				else if (!_wcsicmp(rel, full))
					wcscpy(buf, rel + 3);
				else if (pszPath && *pszPath)
					wcscpy(buf, rel);
				else
					wcscpy(buf, PathFindFileNameW(full));

				// 格納しないファイルのチェック
				if (!m_pev->m_strExcludes.IsEmpty()) {
					WCHAR* pn = (!bAltName) ? ff.cFileName : ff.cAlternateFileName;
					if (msexc.Match(pn)) {
						m_pev->msgout(sszMsg[6], buf);
						continue;
					}
				}

				m_lCopyDone = 0;
				RelSetFile(buf);
				RelSetRange(ff.nFileSizeLow);

				TCOMP tc = 0;
				if (CheckFlags(CABOPT_LZX))
					tc = (TCOMP)TCOMPfromLZXWindow(m_pev->m_nCompressionLevel);
				else if (CheckFlags(CABOPT_QUANTUM))
					tc = (TCOMP)TCOMPfromTypeLevelMemory(tcompTYPE_QUANTUM,
											m_pev->m_nMemoryLevel,
											m_pev->m_nCompressionLevel);
				else if (CheckFlags(CABOPT_MSZIP))
					tc = tcompTYPE_MSZIP;
				else
					tc = tcompTYPE_NONE;

				// 無圧縮格納するファイルのチェック
				if ((tc != tcompTYPE_NONE) && !m_pev->m_strStored.IsEmpty()) {
					WCHAR* pn = (!bAltName) ? ff.cFileName : ff.cAlternateFileName;
					if (msinc.Match(pn))
						tc = tcompTYPE_NONE;
				}

				int m = CompressionTypeFromTCOMP(tc);
				m_pev->msgout(sszMsg[(m_pev->m_chCmd == CABCMD_PACK) ? 1 : 2], sszCompressionType[m], buf);

				// relをchar配列バッファとして使う
				if (!IsLosslessUnicodeName(buf))
					EncodeWStrToUTF(buf, -1, (BYTE*)rel, ARRAYSIZE(rel));
				else
					WideCharToMultiByte(CP_ACP, 0, buf, -1, (char*)rel, sizeof(rel), NULL, NULL);
				if (!FCIAddFile(m_hfci,
								(char*)full,
								(char*)rel,
								FALSE,		// file is not executable
								fciGetNextCabinet,
								fciStatus,
								fciGetOpenInfo,
								tc))
				{
					ff.EndFind();
					return FALSE;
				}
				m_pev->m_nNumOfFiles++;
				AddList(full);
			}
		} while (ff.NextFind());
		ff.EndFind();
	}
	return TRUE;
}

BOOL CCabEnv::FciCompress()
{
	CRunning running;

	if (PathIsDirectory(m_strCabName)) {
		msgout(sszMsg[3], (LPCWSTR)m_strCabName);
		return FALSE;
	}

	if (PathFileExists(m_strCabName) && !(m_cmdOption & CABOPT_OVERWRITE)) {
		if (AppMessage(IDS_QUERY_OVERWRITE, m_strCabName, MB_QYESNO) != IDYES) {
			msgout(sszMsg[3], L"cab file already exists.");
			return FALSE;
		}
	}

	int cnt = m_arrMask.GetSize();
	if (cnt < 1) {
		msgout(sszMsg[3], L"target file(s) not specified.");
		return FALSE;
	}

	// Initialise our internal state
	CPackState cps;
	cps.m_pev = this;

	CString name, path;
	int pos = m_strCabName.ReverseFind(_T('\\'));
	if (pos >= 0) {
		name = m_strCabName.Mid(pos + 1);
		path = m_strCabName.Left(pos);
		// 書庫作成先のフォルダが存在しない場合
		if (!PathIsDirectory(path)) {
			if (AppMessage(IDS_QUERY_MKDIR, path, MB_QYESNO) != IDYES)
				return FALSE;
			if (!MakeTree(path))
				return FALSE;
		}

		if (path[path.GetLength() - 1] != _T('\\'))
			path += _T('\\');
	}
	else {
		path.Empty();
		name = m_strCabName;
	}

	PathRemoveExtension((LPTSTR)name.GetString());
	name.ReleaseBuffer();

	CCAB ccb;
	cps.SetCabParams(ccb, name, path);
	msgout(sszMsg[0], (LPCWSTR)m_strCabName);

	CString str;
	BOOL bOK = FALSE;
	cps.m_hfci = FCICreate(&cps.m_erf,
							fciFilePlaced,
							fciAlloc,
							fciFree,
							fciOpen,
							fciRead,
							fciWrite,
							fciClose,
							fciSeek,
							fciDelete,
							fciGetTempFile,
							&ccb,
							&cps);
	if (cps.m_hfci) {
		if (!cps.CheckFlags(CABOPT_SILENT)) {
			cps.RelCreate();
			CString cab;
			StrToWStr(ccb.szCab, cab);
			cps.RelSetTitle(cab);
		}

		// 基準ディレクトリに移動
		CEnterFolder ef;
		ef.enter(m_strBasePath);

		// 格納済みファイル数カウンタ初期化
		int i;
		m_nNumOfFiles = 0;
		for (i = 0; i < cnt; i++) {
			str = m_arrMask[i];
			int index = str.ReverseFind(_T('\\'));
			if (index >= 0) {
				path = str;
				name = str.Mid(index + 1);
				path.ReleaseBuffer(index);
			}
			else {
				name = str;
				path.Empty();
			}

			if (!cps.DoCompress(path, name)) {
				fciGetError(cps.m_erf.erfOper, str);
				msgout(sszMsg[3], str.GetString());
				FCIDestroy(cps.m_hfci);
				return FALSE;
			}
		}

		// This will automatically flush the folder first
		bOK = FCIFlushCabinet(cps.m_hfci, FALSE, fciGetNextCabinet, fciStatus);
		if (!bOK) {
			fciGetError(cps.m_erf.erfOper, str);
			msgout(sszMsg[3], str.GetString());
		}
		FCIDestroy(cps.m_hfci);

		if (m_nNumOfFiles < 1) {
			// 格納されたファイルがないので、書庫を削除
			bOK = FALSE;
			DeleteFile(m_strCabName);

			name.LoadString(IDS_NOFILES_TO_COMPRESS);
			msgout(name);
		}
		else {
			if (m_chCmd == CABCMD_MOVE) {
				int size = cps.m_packlist.GetSize();
				for (i = 0; i < size; i++) {
					DeleteFile(cps.m_packlist[i]);
				}
			}
		}
	}
	else {
		fciGetError(cps.m_erf.erfOper, str);
		msgout(sszMsg[3], str.GetString());
	}
	return bOK;
}

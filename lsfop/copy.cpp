/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "fopstd.h"
#include "copy.h"

BOOL QueryReplace(HWND hwnd, CReplaceFileInfo* prd);
BOOL QueryDiscardStream(HWND hwnd, DISCSTREAM* pds);
BOOL QueryDecryptCopy(HWND hwnd, DISCSTREAM* pds);

/////////////////////////////////////////////////////////////////////////////
//

_inline static DWORD _get_copybuffer_size(int type)
{
	// コピー先のメディアがネットワークまたは取り外し可能メディアなら、
	// コピーに利用するバッファサイズは最小限にする
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	DWORD dwBlockSize = si.dwPageSize << 2;
	if ((type != DRIVE_REMOTE) && (type != DRIVE_REMOVABLE))
		dwBlockSize <<= 2;
	return dwBlockSize;
}

/////////////////////////////////////////////////////////////////////////////
//

DWORD CALLBACK CCopyProgressDlg::copy_progress(
	LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred,
	LARGE_INTEGER StreamSize,
	LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber,
	DWORD dwCallbackReason,
	HANDLE hSourceFile,
	HANDLE hDestinationFile,
	LPVOID param)
{
	CCopyProgressDlg* pThis = (CCopyProgressDlg*)param;
	if (dwCallbackReason == CALLBACK_STREAM_SWITCH)
		pThis->SetTotalSize((DWORDLONG)StreamSize.QuadPart);
	else if (dwCallbackReason == CALLBACK_CHUNK_FINISHED)
		pThis->SetCompletedSize((DWORDLONG)StreamBytesTransferred.QuadPart);
	return pThis->isCanceled() ? PROGRESS_CANCEL : PROGRESS_CONTINUE;
}

class virtmem
{
private:
	void* m_pv;
public:
	virtmem(DWORD blockSize) { m_pv = ::VirtualAlloc(NULL, blockSize, MEM_COMMIT, PAGE_READWRITE); }
	~virtmem() { if (m_pv) ::VirtualFree(m_pv, 0, MEM_RELEASE); }
	void* GetData() { return m_pv; }
};

BOOL MergeCopy(LPCTSTR pSrc, LPTSTR pDst, BOOL bCutEof, CCopyProgressDlg* progress)
{
	CFileHandle src;
	src.CreateFile(pSrc, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	if (src.IsInvalid())
		return FALSE;

	CFileHandle dst;
	dst.CreateFile(pDst, GENERIC_READ | GENERIC_WRITE, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	if (dst.IsInvalid())
		return FALSE;

	DWORD endpos = dst.SetFilePointer(0, NULL, FILE_END);

	if (bCutEof && endpos > 0) {
		dst.SetFilePointer(-1, NULL, FILE_END);
		BYTE cb[8];
		DWORD n;
		if (dst.ReadFile(cb, 1, &n) && cb[0] == 0x1A) {
			// EOFをカット
			dst.SetFilePointer(-1, NULL, FILE_END);
			dst.SetEndOfFile();
		}
	}

	CDiskSpace di(pDst);
	DWORD blocksize = _get_copybuffer_size(di.m_nDriveType);

	// コピーバッファを確保
	virtmem vm(blocksize);
	void* pv = vm.GetData();

	DWORD lasterror = 0;
	if (pv) {
		DWORD dwRead, dwWrite;
		while (1) {
			if (!progress->ZapMessage()) {
				lasterror = ERROR_CANCELLED;
				break;
			}

			if (!src.ReadFile(pv, blocksize, &dwRead)) {
				lasterror = GetLastError();
				break;
			}

			if (!dwRead)	// end of file
				break;

			progress->SetPartCompleted(dwRead / 2);

			if (!dst.WriteFile(pv, dwRead, &dwWrite)) {
				lasterror = GetLastError();
				break;
			}

			progress->SetPartCompleted(dwWrite / 2);
		}
	} else {
		lasterror = GetLastError();
	}
	SetLastError(lasterror);
	return (lasterror == 0);
}

/////////////////////////////////////////////////////////////////////////////
//

CZapInfo::CZapInfo()
{
	m_hOwner = NULL;
	m_nAction = FAC_QUERY;
	m_idTitle = 0;
	m_bCopy = true;
	m_bYesToAll = false;
	m_bMultiDest = false;
	m_bUseRecycle = false;
	m_bAutoRename = false;
	m_diskstream.operation = DSO_YES;
	m_decryptcopy.operation = DSO_YES;
}

CZapInfo::~CZapInfo()
{
}

void CZapInfo::Initialize(HWND hOwner, BOOL fCopy, BOOL fSaveFolderHierarchy, BOOL yes2all, FOPACTION action)
{
	m_hOwner = hOwner;
	m_bCopy = (fCopy != FALSE);
	m_bSaveFolderHierarchy = (fSaveFolderHierarchy != FALSE);
	m_bYesToAll = (yes2all != FALSE);
	m_nAction = action;
	m_idTitle = fCopy ? IDS_TITLE_FILECOPY : IDS_TITLE_FILEMOVE;
}


/////////////////////////////////////////////////////////////////////////////
//

void CZapInfo::Start()
{
	CCopyProgressDlg::Start(m_hOwner, m_bCopy ? IDS_DO_COPY : IDS_DO_MOVE, 0);
}

BOOL CZapInfo::GetReplaceAction(LPCTSTR src, LPCTSTR dst, CWin32FindData& srcWfd, CWin32FindData& dstWfd)
{
	CReplaceFileInfo info;
	info.m_src = src;
	info.m_dst = dst;
	info.m_srcWfd = srcWfd;
	info.m_dstWfd = dstWfd;
	info.m_fopAction = GetAction();
	info.m_bCopy = isCopy();
	info.m_bMultiDest = isMultiDest();
	info.m_bYesToAll = isYesToAll();
	info.m_bUseRecycle = isUseRecycle();
	info.m_bAutoRename = isAutoRename();

	BOOL ok = QueryReplace(GetOwner(), &info);
	if (ok) {
		m_nAction = info.m_fopAction;
		m_bYesToAll = info.m_bYesToAll;
		m_bUseRecycle = info.m_bUseRecycle;
		m_bAutoRename = info.m_bAutoRename;
	}
	return ok;
}

int CZapInfo::FileMove(LPTSTR src, LPTSTR dst)
{
	TCHAR buf[MAX_PATH * 2];
	CWin32FindData wfdSrc, wfdDst;

	if (!_pathcompare(src, dst)) {
		Message(IDS_SAMEPATH);
		return CR_ERROR;
	}

	// 最新のファイル情報を取得
	int r;
	while (!GetFileData(src, wfdSrc)) {
		r = Error(src, MB_QABORT);
		if      (r == IDABORT)  return CR_ERROR;
		else if (r == IDIGNORE) return CR_IGNORED;
	}

	SetFile(wfdSrc, src);
	if (_pathfileexists(dst)) {
		// 移動先ファイルの最新情報を取得
		while (!GetFileData(dst, wfdDst)) {
			r = Error(dst, MB_QABORT);
			if      (r == IDABORT)  return CR_ERROR;
			else if (r == IDIGNORE) return CR_IGNORED;
		}

		if (!isYesToAll()) {
			if (!GetReplaceAction(src, dst, wfdSrc, wfdDst))
				return CR_ERROR;
		}

		switch (GetAction()) {
		case FAC_NEWONLY:
			// ------------------------------------
			// 新しいファイルのみ
			// ------------------------------------
			// ファイル日付のチェック
			if (wfdSrc.CompareLastWriteTime(wfdDst, FALSE) <= 0)
			    return CR_IGNORED;
			// fall down...

		case FAC_OVERWRITE:
			// ------------------------------------
			// 上書き
			// ------------------------------------
			if (isUseRecycle())
				TrashOneFile(GetOwner(), dst, FOF_ALLOWUNDO | FOF_NOCONFIRMATION);

			if (_pathfileexists(dst)) {
				// コピー先ファイルが書き込み可能かチェック
				if (wfdDst.IsAttrib(FA_RDONLY | FA_SYSTEM | FA_HIDDEN))
					SetFileAttributes(dst, FA_NORMAL);
			}
			break;

		case FAC_RENAMEDST:
			// ------------------------------------
			// 新しい名前でコピー
			// ------------------------------------
			if (_pathfileexists(dst)) {
				lstrcpy(buf, dst);

				if (isAutoRename())
					PathSetUnique(buf);
				else if (!fopGetNewName(GetOwner(), buf))
					return CR_ERROR;
				lstrcpy(dst, buf);
			}
			break;

		case FAC_RENAMESRC:
			// ------------------------------------
			// 相手先ファイルの名前変更
			// ------------------------------------
			lstrcpy(buf, dst);

			if (isAutoRename()) {
				PathSetUnique(buf);
				if (!RenameFileOrDirectory(dst, buf))
					return CR_ERROR;
			}
			else if (!fopRename(GetOwner(), buf))
				return CR_ERROR;
			break;

		case FAC_MERGE:
			// ------------------------------------
			// 連結
			// ------------------------------------
			if (!MergeCopy(src, dst, FALSE, this)) {
				Error(NULL, MB_STOP);
				return CR_ERROR;
			}

			if (isUseRecycle())
				TrashOneFile(GetOwner(), src, FOF_ALLOWUNDO | FOF_NOCONFIRMATION);
			else {
				if (!DeleteFile(src)) {
					Error(src);
					return CR_ERROR;
				}
				SHChangeNotify(SHCNE_DELETE, SHCNF_PATH | SHCNF_FLUSHNOWAIT, src, 0);
			}
			return CR_DONE;

		case FAC_IGNORE:
			// ------------------------------------
			// 無視
			// ------------------------------------
			return CR_IGNORED;

		default:
			return CR_ERROR;
		}
	}

	if (!MoveFileEx(src, dst,
				MOVEFILE_WRITE_THROUGH |
				MOVEFILE_COPY_ALLOWED |
				MOVEFILE_REPLACE_EXISTING)) {
		Error(src);
		return CR_ERROR;
	}
	SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, src, dst);
    return CR_DONE;
}

int CZapInfo::FileCopy(LPTSTR src, LPTSTR dst)
{
	TCHAR buf[MAX_PATH * 2];
	CWin32FindData wfdSrc, wfdDst;

	if (!_pathcompare(src, dst)) {
		// 同一名であれば、名前を変えてコピーするか問い合わせる
		if (Message(IDS_SAMEFILENAME_RENAME, PathFindFileName(src), MB_QYESNO) != IDYES)
			return CR_ERROR;

		// 新しい名前の取得
		lstrcpy(buf, dst);
		if (!fopGetNewName(GetOwner(), buf))
			return CR_ERROR;
		lstrcpy(dst, buf);
	}

	CDiskSpace ss(src, true);
FC_RETRYDISK:
	// コピー先空きディスク容量の取得とステータスメッセージ表示
	CDiskSpace ds(dst);
	if (!ds.GetDriveInfo()) {
		Error(ds.m_strRoot, MB_STOP);
		return CR_ERROR;
	}

	// コピー元ファイルの最新情報を取得
	int r;
	while (!GetFileData(src, wfdSrc)) {
		r = Error(src, MB_QABORT);
		if      (r == IDABORT)  return CR_ERROR;
		else if (r == IDIGNORE) return CR_IGNORED;
	}

	if (!ds.isNTFS()) {
		// コピー先がNTFSでない場合
		if (ss.isNTFS()) {
			// ファイルがサブストリームを持っているか、ストリーム損失のチェック
			if (GetAnotherDataStream(src) > 0 && m_diskstream.operation != DSO_YESTOALL) {
				lstrcpy(m_diskstream.szName, src);
				m_diskstream.numfiles = isMultiDest() ? 2 : 1;
				QueryDiscardStream(GetOwner(), &m_diskstream);
				if (m_diskstream.operation == DSO_SKIP)
					return CR_IGNORED;
				else if (m_diskstream.operation == DSO_CANCEL)
					return CR_ERROR;
			}

			// ファイルがNTFS暗号化をされていれば、暗号解除をしてもよいかチェック
			if (wfdSrc.IsAttrib(FILE_ATTRIBUTE_ENCRYPTED) && m_decryptcopy.operation != DSO_YESTOALL) {
				lstrcpy(m_decryptcopy.szName, src);
				m_decryptcopy.numfiles = isMultiDest() ? 2 : 1;
				QueryDecryptCopy(GetOwner(), &m_decryptcopy);
				if (m_decryptcopy.operation == DSO_SKIP)
					return CR_IGNORED;
				else if (m_decryptcopy.operation == DSO_CANCEL)
					return CR_ERROR;
			}
		}
	}

	DWORDLONG sizeSrc, sizeDst;
	SETDWORDLONG(sizeSrc, wfdSrc.nFileSizeLow, wfdSrc.nFileSizeHigh);
	SetFile(wfdSrc, src);
	if (_pathfileexists(dst)) {
		// コピー先のファイル情報取得
		while (!GetFileData(dst, wfdDst)) {
			r = Error(dst, MB_QABORT);
			if      (r == IDABORT)  return CR_ERROR;
			else if (r == IDIGNORE) return CR_IGNORED;
		}

		if (!isYesToAll()) {
			if (!GetReplaceAction(src, dst, wfdSrc, wfdDst))
				return CR_ERROR;
		}

		switch (GetAction()) {
		case FAC_NEWONLY:
			// ------------------------------------
			// 新しいファイルのみ
			// ------------------------------------
			if (wfdSrc.CompareLastWriteTime(wfdDst, FALSE) <= 0)
				return CR_IGNORED;
			// C_OVERWRITEへ流下

		case FAC_OVERWRITE:
			// ------------------------------------
			// 上書き
			// ------------------------------------
			if (isUseRecycle())
				TrashOneFile(GetOwner(), dst, FOF_ALLOWUNDO | FOF_NOCONFIRMATION);

			if (_pathfileexists(dst)) {
				// コピー先ファイルが書き込み可能かチェック
				if (wfdDst.IsAttrib(FA_RDONLY | FA_SYSTEM | FA_HIDDEN))
					SetFileAttributes(dst, FA_NORMAL);

				// コピー先の残容量を調整
				SETDWORDLONG(sizeDst, wfdDst.nFileSizeLow, wfdDst.nFileSizeHigh);
				sizeSrc = (sizeSrc > sizeDst) ? sizeSrc - sizeDst : 0;
			}
			break;

		case FAC_RENAMEDST:
			// ------------------------------------
			// 新しい名前で
			// ------------------------------------
			if (_pathfileexists(dst)) {
				lstrcpy(buf, dst);

				if (isAutoRename())
					PathSetUnique(buf);
				else if (!fopGetNewName(GetOwner(), buf))
					return CR_ERROR;
				lstrcpy(dst, buf);
			}
			break;

		case FAC_RENAMESRC:
			// ------------------------------------
			// 相手先ファイルの名前変更
			// ------------------------------------
			lstrcpy(buf, dst);

			if (isAutoRename()) {
				PathSetUnique(buf);
				if (!RenameFileOrDirectory(dst, buf))
					return CR_ERROR;
			}
			else if (!fopRename(GetOwner(), buf))
				return CR_ERROR;
			break;

		case FAC_MERGE:
			// ------------------------------------
			// 連結
			// ------------------------------------
			if (!MergeCopy(src, dst, FALSE, this)) {
				Error(NULL, MB_STOP);
				return CR_ERROR;
			}
			return CR_DONE;

		case FAC_IGNORE:
			// ------------------------------------
			// 無視
			// ------------------------------------
			return CR_IGNORED;

		default:
			return CR_ERROR;
		}
	}

	// 残容量のチェック（含む同名コピー時の調整）
	if (ds.m_dlTotal && !ds.isSpaceAvailable(sizeSrc)) {
		if (GetDriveType(ds.m_strRoot) == DRIVE_REMOVABLE) {
			while (1) {
				if (Message(IDS_CHANGEFD, NULL, MB_QYESNO) == IDNO)
					return CR_ERROR;

				// 認識に多少時間のかかるメディアの場合があるので、
				// 再試行をおこなう
				while (!ds.GetDriveInfo()) {
					if (Error(ds.m_strRoot, MB_QRETRY) == IDCANCEL)
						return CR_ERROR;
					fopFlushDrive(ds.m_strRoot);
				}

				SETDWORDLONG(sizeDst, wfdSrc.nFileSizeLow, wfdSrc.nFileSizeHigh);
				if (!ds.isSpaceAvailable(sizeDst))
					continue;

				// コピー先ディレクトリが存在しなければ作成する
				TCHAR path[MAX_PATH * 2];
				PathGetParent(dst, path);
				if (!fopMakeTree(path))
					return CR_ERROR;

				break;
			}
			goto FC_RETRYDISK;
		}
		else {
			Message(IDS_DISKFULL);
			return CR_ERROR;
		}
	}

	// NT CopyFileEx
	static BOOL bUserCanceled = FALSE;

	while (!CopyFileEx(src, dst, copy_progress, this, &bUserCanceled, COPY_FILE_ALLOW_DECRYPTED_DESTINATION)) {
		r = Error(dst, isCanceled() ? MB_QRETRY : MB_QABORT);
		switch (r) {
			case IDCANCEL:
			case IDABORT: return CR_ERROR;
			case IDIGNORE: return CR_IGNORED;
			case IDRETRY: default: break;
		}
	}

/*
	// 双方がACLサポートならsidをコピー
	if (ss.isPersistentAcls() && ds.isPersistentAcls())
		CopyAccessRights(NULL, src, dst);
*/
	bUserCanceled = FALSE;
	return CR_DONE;
}

int CZapInfo::CopyFolder(LPTSTR pSrc, LPTSTR pDst)
// pSrc = 移動するディレクトリパス
// pDst = 移動先パス
{
	if (!pSrc || !pDst || *pSrc == 0 || *pDst == 0)
		return CR_ERROR;

	// コピー元とコピー先が同一パスならば中止
	if (!_pathcompare(pSrc, pDst)) {
		Message(IDS_SAMEPATH);
		return CR_ERROR;
	}

	// コピー先パスがコピー元パスの子孫ならば中止
	if (!_pathisparent(pSrc, pDst)) {
		Message(IDS_PATH_INCLUDE);
		return CR_ERROR;
	}

	int r = TreeCopy(pSrc, pDst);

	// ファイルの移動が処理正常終了したら
	if (!isCopy() && r == CR_DONE) {
		if (_pathfileexists(pSrc)) {
			r = (fopRemoveTreeNoConfirm(pSrc) != FALSE) ? CR_DONE : CR_ERROR;
			if (r == CR_ERROR)
				Message(IDS_ERROR_DELFOLDER, pSrc);
		}
	}
	return r;
}

int CZapInfo::TreeCopy(LPTSTR src, LPTSTR dst)
{
	// 同一ドライブか否か
	bool bSameDrive = PathIsCommonPrefixFolder(src, dst);

	// 存在しなければディレクトリを作成する
	CWin32FindData wfd;
	bool bDirectoryCreated = false;
	if (!_pathisdirectory(dst)) {
		if (!isCopy()) {
			if (_movefile(src, dst)) {
				SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH | SHCNF_FLUSH, src, dst);
				return CR_DONE;
			}
		}

		if (!fopMakeTree(dst))
			return CR_ERROR;

		// コピー元のフォルダの属性を取得
		// ここではタイムスタンプをセットせず、最後におこなう。
		// XP sp2以降？では、フォルダ内でファイル操作をおこなうと、フォルダのタイムスタンプが
		// OSによって現在日時に更新されてしまうため。
		bDirectoryCreated = true;
		GetFileData(src, wfd);
	}

	LPTSTR pSrcEnd = src + lstrlen(src);
	LPTSTR pDstEnd = dst + lstrlen(dst);

	// 最初にリストアップを完了させる
	CSimpleMap<CString, BYTE> arr;

	CFileFinder ff;
	if (ff.BeginFind(src, gszAst)) {
		do {
			if (ff.IsFolder() && ff.IsDot()) continue;
			arr.Add(ff.cFileName, (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0);
		} while (ff.NextFind());
		ff.EndFind();
	}

	int res = CR_DONE;
	int cnt = (int)arr.GetSize();
	for (int i = 0; i < cnt; i++) {
		// 元と先のフルパス名の生成
		*pSrcEnd = 0;
		*pDstEnd = 0;

		CString& rstr = arr.GetKeyAt(i);
		PathAppend(src, rstr);
		PathAppend(dst, rstr);

		if (arr.GetValueAt(i)) {
			// フォルダを処理
			if (!isSaveFolderHierarchy())
				*pDstEnd = 0;
			res = TreeCopy(src, dst);
			if (res == CR_ERROR) break;
		}
		else {
			// ファイルを処理
			if (isCopy())
				res = FileCopy(src, dst);
			else {
				// 同一ドライブなら移動、別ドライブならコピーして削除
				if (bSameDrive)
					res = FileMove(src, dst);
				else {
					if ((res = FileCopy(src, dst)) == CR_DONE)
						fopRemoveFileNoConfirm(src);
				}
			}
			if (res == CR_ERROR) break;
		}
	}

	*pSrcEnd = 0;
	*pDstEnd = 0;

	// コピー元のフォルダとコピー先のフォルダの属性を同一にする
	if (bDirectoryCreated) {
		SetFileProperties(dst, wfd.dwFileAttributes, &wfd.ftLastWriteTime, &wfd.ftCreationTime);
	}
	return res;
}

HRESULT CreateLinkObject(LPCTSTR pszSrc, LPCTSTR pszDst, LPCTSTR pszDesc, LPCTSTR pszWorkdir, BOOL bSrcIsIDList)
{
	IShellLink* psl;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
	if (SUCCEEDED(hr)) {
		if (bSrcIsIDList)
			psl->SetIDList((LPITEMIDLIST)pszSrc);
		else
			psl->SetPath(pszSrc);
		psl->SetDescription(pszDesc);

		if (pszWorkdir && *pszWorkdir)
			psl->SetWorkingDirectory(pszWorkdir);

		IPersistFile* ppf;
		hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
		if (SUCCEEDED(hr)) {
			hr = ppf->Save(pszDst, TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	return hr;
}

static BOOL __fastcall _make_link(LPCTSTR pszLinkTo, LPCTSTR pszDir, BOOL bIDList)
{
	TCHAR desc[MAX_PATH], lnk[MAX_PATH * 2], dir[MAX_PATH * 2];

	dir[0] = 0;
	LPITEMIDLIST pidl = NULL;
	if (!bIDList) {
		if (!_pathisdirectory(pszLinkTo)) {
			lstrcpy(dir, pszLinkTo);
			PathRemoveFileSpec(dir);
		}
		pidl = ILCreateFromPath(pszLinkTo);
		if (pidl)
			bIDList = TRUE;
	} else {
		if (SHGetPathFromIDList(pidl, dir) &&
			_pathfileexists(dir) &&
			!_pathisdirectory(dir)) {
			PathRemoveFileSpec(dir);
		}
	}

	if (bIDList) {
		if (!pidl)
			pidl = (LPITEMIDLIST)pszLinkTo;
		SHFILEINFO sfi;
		SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME);
		lstrcpy(desc, sfi.szDisplayName);
		ReplaceInvalidToken(desc, _T('_'));
	} else {
		lstrcpy(desc, PathFindFileName(pszLinkTo));
		LPTSTR p = PathFindExtension(desc);
		if (*p == '.')
			*p = 0;
	}

	CString str;
	str.LoadString(IDS_LINKSTRING);
	lstrcat(desc, str);
	PathCombine(lnk, pszDir, desc);

	CString prf = lnk;
	lstrcat(lnk, gszLNK);
	int num = 1;
	while (_pathfileexists(lnk)) {
		wsprintf(lnk, _T("%s(%d)%s"), prf.GetString(), ++num, gszLNK);
	}

	HRESULT hr = CreateLinkObject(bIDList ? (LPCTSTR)pidl : pszLinkTo, lnk, desc, dir, bIDList);
	if (pidl && (pidl != (LPITEMIDLIST)pszLinkTo)) {
		CoTaskMemFree(pidl);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL CreateLinkFromDropFiles(HDROP hDrop, LPCTSTR pDst)
{
	TCHAR buf[MAX_PATH * 2];
	int num = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < num; i++) {
		DragQueryFile(hDrop, i, buf, ARRAYSIZE(buf));
		_make_link(buf, pDst, FALSE);
	}
	return TRUE;
}

BOOL CreateLinkFromShellIDList(LPIDA pida, LPCTSTR pDst)
{
	LPITEMIDLIST pidl, pidlRoot, pidlFq;
	pidlRoot = (LPITEMIDLIST)((LPTSTR)(pida) + pida->aoffset[0]);
	for (UINT u = 1; u < pida->cidl + 1; u++) {
		pidl = (LPITEMIDLIST)((LPTSTR)(pida) + pida->aoffset[u]);
		if (pida->aoffset[0])
			pidlFq = idl_ConcatPidls(pidlRoot, pidl);
		else
			pidlFq = idl_Copy(pidl);

		_make_link((LPCTSTR)pidlFq, pDst, TRUE);
		CoTaskMemFree(pidlFq);
	}
	return TRUE;
}

extern "C"
BOOL WINAPI fopMakeTree(LPCTSTR pszPath)
{
	if (PathIsRoot(pszPath) || lstrlen(pszPath) < 3) {
		SetLastError(ERROR_BAD_PATHNAME);
		return TRUE;
	}

	while (1) {
		int res = SHCreateDirectory(NULL, pszPath);
		switch (res) {
		case ERROR_SUCCESS:
			break;
		case ERROR_FILE_EXISTS:		// 同名のファイルが存在する
		case ERROR_ALREADY_EXISTS:
			if (_pathisdirectory(pszPath)) return TRUE;
			// fall
		default:
			SetLastError(res);
			if (LastError(IDS_TITLE_MKDIR, pszPath, MB_QRETRY) == IDCANCEL)
				return FALSE;
		}
	}

	SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH | SHCNF_FLUSH, pszPath, 0);
	return TRUE;
}

static BOOL __exec_dropfile(HWND hOwner, LPCTSTR strDropTo, CDropFiles& df, CZapInfo& zap)
{
	zap.Start();

	int n;
	WIN32_FIND_DATA wfd;
	TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
	for (int i = 0; i < df.size(); i++) {
		// ファイル名を取得
		df.DragQueryFile(i, src, ARRAYSIZE(src));

		if (!GetFileData(src, &wfd)) {
			if (PathIsRoot(src) && PathGetDriveNumber(src) >= 0) {
				memset(&wfd, 0, sizeof(wfd));
				wfd.dwFileAttributes = FA_SUBDIR;
			}
			else {
				n = CR_ERROR;
				break;
			}
		} else {
			// NextFTPのフックチェック
			if (i == 0
				&& df.size() == 1
				&& (wfd.dwFileAttributes & FA_SUBDIR)
				&& !StrCmpI(wfd.cFileName, _T("NextFTP"))
				) {

				// フォルダが空かチェック
				CFileFinder ff;
				if (ff.BeginFind(src, gszAst)) {
					bool bEmptyFolder = true;
					do {
						if (!ff.IsFolder() || !ff.IsDot()) {
							bEmptyFolder = false;
							break;
						}
					} while (ff.NextFind());
					ff.EndFind();

					if (bEmptyFolder) {
						LPTSTR str = df.MakeFileListBuffer();
						if (str) {
							SHFILEOPSTRUCT fo;
							memset(&fo, 0, sizeof(fo));
							fo.hwnd = hOwner;
							fo.wFunc = (df.effect() == DROPEFFECT_COPY) ? FO_COPY : FO_MOVE;
							fo.pFrom = str;
							fo.pTo = strDropTo;
							BOOL bOK = SHFileOperation(&fo);
							LocalFree(str);
							return bOK;
						}
					}
				}
			}
		}

		lstrcpy(dst, strDropTo);
		if (zap.isSaveFolderHierarchy())			// 階層を保持してコピー
			PathAppend(dst, wfd.cFileName);

		// ディレクトリチェック
		if (wfd.dwFileAttributes & FA_SUBDIR)
			n = zap.CopyFolder(src, dst);
		else {
			if (zap.isCopy())
				n = zap.FileCopy(src, dst);
			else {
				// 同一ドライブ内ならば移動
				if (PathIsCommonPrefixFolder(src, dst))
					n = zap.FileMove(src, dst);
				else {
					n = zap.FileCopy(src, dst);
					if (n == CR_DONE)
						fopRemoveFileNoConfirm(src);
				}
			}
		}
		if (n == CR_ERROR)
			break;
	}
	df.Detach();
	zap.Close();

	return (n == CR_ERROR) ? FALSE : TRUE;
}

BOOL ExecDropFileOperation(HWND hOwner, HDROP hdrop, DWORD eff, LPCTSTR pszDropPath, bool bSaveFolderHierarchy)
{
	// ファイルの処理中に pszDropPathが、いつどこで変更されるかわからない
	// ので いったんバッファに待避する
	// なお、この処理が完了した後は、当初のフォルダにいるとは限らない
	// 呼び出す側は、事前にカレントフォルダを保存する必要がある
	CString strDropTo;
	strDropTo = pszDropPath;

	// コピー・移動のための進捗情報を初期化
	CDropFiles df(hdrop, eff);
	if (eff == DROPEFFECT_NONE || df.size() == 0) return FALSE;
	if (eff == DROPEFFECT_LINK) {
		// ショートカットの作成
		CreateLinkFromDropFiles(df, strDropTo);
		return TRUE;
	}

	CZapInfo zap;
	zap.Initialize(hOwner, (eff == DROPEFFECT_COPY), bSaveFolderHierarchy);
	if (df.size() > 1 || df.IsFolderInclude())
		zap.SetMultiDest(TRUE);

	return __exec_dropfile(hOwner, strDropTo, df, zap);
}

BOOL ExecDropFileOperationEx(DROPFILEOPSTRUCT* info)
{
	// ファイルの処理中に pszDropPathが、いつどこで変更されるかわからない
	// ので いったんバッファに待避する
	// なお、この処理が完了した後は、当初のフォルダにいるとは限らない
	// 呼び出す側は、事前にカレントフォルダを保存する必要がある
	CString strDropTo;
	strDropTo = info->pszDestination;

	// コピー・移動のための進捗情報を初期化
	CDropFiles df(info->hdrop, info->dropEffect);
	if (df.effect() == DROPEFFECT_NONE || df.size() == 0) return FALSE;
	if (df.effect() == DROPEFFECT_LINK) {
		// ショートカットの作成
		CreateLinkFromDropFiles(df, strDropTo);
		return TRUE;
	}

	CZapInfo zap;
	zap.Initialize(info->hwnd, (df.effect() == DROPEFFECT_COPY), info->bSaveFolderHierarchy, info->bYesToAll, info->fAction);
	if (df.size() > 1 || df.IsFolderInclude())
		zap.SetMultiDest(TRUE);
	return __exec_dropfile(info->hwnd, strDropTo, df, zap);
}

static BOOL __copymove(HWND hwnd, LPCTSTR pSrc, LPCTSTR pDst, CZapInfo& zap)
{
	TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
	lstrcpy(src, pSrc);
	lstrcpy(dst, pDst);

	zap.Start();

	// ディレクトリチェック
	int n = CR_ERROR;
	if (_pathisdirectory(src)) {
		zap.SetMultiDest(TRUE);
		n = zap.CopyFolder(src, dst);
	} else if (_pathfileexists(src)) {
		if (zap.isCopy()) {
			// コピー先のフォルダを確認
			TCHAR dir[MAX_PATH * 2];
			PathGetParent(dst, dir);
			if (!validate_dir(hwnd, dir, TRUE))
				return CR_ERROR;
			n = zap.FileCopy(src, dst);
		} else {
			// 同一ドライブ内ならば移動
//@			if (_totupper(*src) == _totupper(*dst))
			if (PathIsCommonPrefixFolder(src, dst)) {
				n = zap.FileMove(src, dst);
			} else {
				n = zap.FileCopy(src, dst);
				if (n == CR_DONE) {
					fopRemoveFileNoConfirm(src);
				}
			}
		}
	}
	zap.Close();
	return (n == CR_ERROR) ? FALSE : TRUE;
}

BOOL CopyOrMoveFile(HWND hOwner, LPCTSTR pSrc, LPCTSTR pDst, BOOL bCopy)
{
	// コピー・移動のための進捗情報を初期化
	CZapInfo zap;
	zap.Initialize(hOwner, bCopy);
	return __copymove(hOwner, pSrc, pDst, zap);
}

BOOL CopyOrMoveFileEx(COPYORMOVEINFOSTRUCT* info)
{
	// コピー・移動のための進捗情報を初期化
	CZapInfo zap;
	zap.Initialize(info->hwnd, info->bCopy, info->bSaveFolderHierarchy, info->bYesToAll, info->fAction);
	return __copymove(info->hwnd, info->pszSource, info->pszDestination, zap);
}

DWORD CopyFileSilent(LPCTSTR pSrc, LPCTSTR pDst)
{
	// NT CopyFileEx
	static BOOL bCancel = FALSE;
	if (!CopyFileEx(pSrc, pDst, NULL, NULL, &bCancel, COPY_FILE_ALLOW_DECRYPTED_DESTINATION))
		return GetLastError();
	return NOERROR;
}

////////////////////////////////////////////////////////////////////////
// 別プロセスでファイルのコピー・移動をおこなう
//

static const TCHAR sszRegApp[] = _T("Software\\LightShip\\");
static const TCHAR sszRegSec[] = _T("GuidOperation");

typedef struct _ACCEPTFILESINFO {
	DROPEFFECT effect;
	bool bAsAdmin;
	bool bSaveFolderHierarchy;
	TCHAR path[MAX_PATH * 2];
	size_t hdrop_size;
	BYTE hdrop_data[1];
} ACCEPTFILESINFO;

static BOOL LaunchDropFileOperation(LPCTSTR pszFunction, HDROP hDrop, DWORD eff, LPCTSTR pszDropPath, bool bAsAdmin, bool bSaveFolderHierarchy)
{
	if (!hDrop || eff == DROPEFFECT_NONE || !pszDropPath || *pszDropPath == 0) return FALSE;

	size_t size = GlobalSize((HGLOBAL)hDrop);
	size_t totalsize = sizeof(ACCEPTFILESINFO) + size;
	ACCEPTFILESINFO* info = (ACCEPTFILESINFO*)malloc(totalsize);
	if (!info) return FALSE;

	info->effect = eff;
	info->bAsAdmin = bAsAdmin;
	info->bSaveFolderHierarchy = bSaveFolderHierarchy;
	lstrcpy(info->path, pszDropPath);
	info->hdrop_size = size;

	void* p = GlobalLock((HGLOBAL)hDrop);
	memcpy(info->hdrop_data, p, size);
	GlobalUnlock((HGLOBAL)hDrop);

	BOOL bOK = FALSE;
	CString str = sszRegApp;
	str += gszApp;
	CRegKey reg;
	if (reg.Create(HKEY_CURRENT_USER, str) == ERROR_SUCCESS) {
		CRegKey sec;
		if (sec.Create(reg, sszRegSec, REG_NONE, REG_OPTION_VOLATILE, KEY_WRITE) == ERROR_SUCCESS) {
			GUID g;
			if (CoCreateGuid(&g) == S_OK) {
				OLECHAR oz[128];
				StringFromGUID2(g, oz, ARRAYSIZE(oz));
				sec.SetBinaryValue(oz, info, (ULONG)totalsize);

				TCHAR buf[MAX_PATH * 2];
				GetModuleFileName(MODINST, buf, ARRAYSIZE(buf));
				bOK = lccRunDllEx(NULL, PathFindFileName(buf), pszFunction, OLE2T(oz), bAsAdmin);
			}
			sec.Close();
		}
		reg.Close();
	}

	free(info);
	return bOK;
}

extern "C"
BOOL WINAPI fopDropFileOperation(const DROPFILEOPERATION* pdfo)
{
	BOOL bOK = FALSE;
	if (pdfo && pdfo->cbSize >= sizeof(DROPFILEOPERATION)) {
		if (pdfo->bAsyncOperation) {
			bOK = LaunchDropFileOperation(_T("RunAccept"), pdfo->hDrop, pdfo->dwEffect, pdfo->pszDropPath, (pdfo->bAsAdmin != 0), (pdfo->bSaveFolderHierarchy != 0));
		} else {
			ILSFileOperation* pfo;
			HRESULT hr = CoCreateInstanceAsAdmin(pdfo->hwnd, (pdfo->bAsAdmin != 0), __uuidof(LSFileOperation), __uuidof(ILSFileOperation), (void**)&pfo);
			if (hr == S_OK) {
				hr = pfo->DropFileOperation(pdfo->hwnd, pdfo->hDrop, pdfo->dwEffect, pdfo->pszDropPath, pdfo->bSaveFolderHierarchy);
				pfo->Release();
			}
			bOK = (hr == S_OK);
		}
	}
	return bOK;
}

class CRegExec
{
public:
	ACCEPTFILESINFO* m_info;
	HGLOBAL m_hmem;

	CRegExec()
	{
		m_info = NULL;
		m_hmem = NULL;
	}
	~CRegExec()
	{
		if (m_hmem) GlobalFree(m_hmem);
		if (m_info) free(m_info);
	}
	bool Initialize(LPCTSTR pszRegName)
	{
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, sszRegApp);
		PathAppend(buf, gszApp);
		PathAppend(buf, sszRegSec);

		bool ok = false;
		CRegKey reg;
		if (reg.Open(HKEY_CURRENT_USER, buf) == ERROR_SUCCESS) {
			ULONG len = 0;
			if (reg.QueryBinaryValue(pszRegName, NULL, &len) == ERROR_SUCCESS && len > sizeof(ACCEPTFILESINFO)) {
				m_info = (ACCEPTFILESINFO*)malloc(len);
				if (m_info) {
					if (reg.QueryBinaryValue(pszRegName, m_info, &len) == ERROR_SUCCESS) {
						m_hmem = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, m_info->hdrop_size);
						if (m_hmem) {
							void* p = GlobalLock(m_hmem);
							memcpy(p, m_info->hdrop_data, m_info->hdrop_size);
							GlobalUnlock(m_hmem);
							ok = true;
						}
					}
					if (!ok) {
						free(m_info);
						m_info = NULL;
					}
				}
			}
			reg.DeleteValue(pszRegName);
			reg.Close();
		}
		return ok;
	}
};

extern "C"
void WINAPI RunAcceptW(HWND hwnd, HINSTANCE hInst, LPCWSTR lpCmdLine, int nCmdShow)
{
	CRegExec re;
	if (re.Initialize(lpCmdLine)) {
		ILSFileOperation* pfo;
		if (CoCreateInstanceAsAdmin(hwnd, re.m_info->bAsAdmin, __uuidof(LSFileOperation), __uuidof(ILSFileOperation), (void**)&pfo) == S_OK) {
			pfo->DropFileOperation(hwnd, (HDROP)re.m_hmem, re.m_info->effect, re.m_info->path, re.m_info->bSaveFolderHierarchy);
			pfo->Release();
		}
	}
}

extern "C"
void WINAPI RunAccept(HWND hwnd, HINSTANCE hInst, LPCSTR lpCmdLine, int nCmdShow)
{
	CStringW cmd(lpCmdLine);
	RunAcceptW(hwnd, hInst, cmd, nCmdShow);
}


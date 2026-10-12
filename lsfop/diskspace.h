/////////////////////////////////////////////////////////////////////
// dskspace.h

#pragma once
#ifndef __DSKSPACE_H__
#define __DSKSPACE_H__

#define UNC_DRIVE			100

BOOL ValidateFolderConnection(HWND hwnd, LPCTSTR pszFolder);


/////////////////////////////////////////////////////////////////////
//

typedef struct _div64_t {
	DWORDLONG quot;
	DWORDLONG rem;
} div64_t;

inline div64_t div64(DWORDLONG numer, DWORDLONG denom)
{
	div64_t result;
	result.quot = numer / denom;
	result.rem = numer % denom;
	if (numer < 0 && result.rem > 0) {
		// did division wrong; must fix up
		++result.quot;
		result.rem -= denom;
	}
	return result;
}


/////////////////////////////////////////////////////////////////////
// CDiskSpace

class CDiskSpace
{
private:
	DWORD m_errorcode;

public:
	CString m_strRoot;
	CString m_strVolume;
	CString m_strFileSysName;			// ファイル システムの名前
	DWORD m_dwVolser;					// ボリュームシリアルナンバ
	DWORD m_dwFileSystemFlags;			// ファイルシステムフラグ
	DWORD m_dwBytesPerSector;			// セクタ当たりのバイト数
	DWORD m_dwBytesPerCluster;			// クラスタ当たりのバイト数
	DWORD m_dwFreeClusters;				// 空きクラスタ数
	DWORD m_dwClusters;					// クラスタ総数
	DWORDLONG m_dlTotal;
	DWORDLONG m_dlFree;
	UINT m_nDriveType;
	int m_nFileSystem;

private:
	void __initparam()
	{
		m_dwVolser = 0;
		m_dwFileSystemFlags = 0;
		m_dwFreeClusters = 0;
		m_dwClusters = 0;
		m_nFileSystem = FSTYPE_UNKNOWN;
		m_dlTotal = 0;
		m_dlFree = 0;
		m_dwBytesPerSector = 0;
		m_dwBytesPerCluster = 0;
		m_dwFreeClusters = 0;
		m_dwClusters = 0;
		m_errorcode = NOERROR;
		m_nDriveType = DRIVE_UNKNOWN;
	}

public:
	CDiskSpace()
	{
		__initparam();
	}

	CDiskSpace(LPCTSTR pszRoot, bool bParseInfo = false)
	{
		__initparam();
		SetRoot(pszRoot);
		if (bParseInfo) GetDriveInfo();
	}

	void SetRoot(LPCTSTR pszRoot)
	{
		// ReparsePointを考慮
		TCHAR buf[MAX_PATH];
		GetVolumePathName(pszRoot, buf, ARRAYSIZE(buf));
		PathAddBackslash(buf);

		m_nDriveType = GetDriveType(buf);
		// CD・DVD±RWの場合は、ルートディレクトリを指定しないと
		// GetDiskInfoのGetVolumeInformationでエラーになる場合がある
		switch (m_nDriveType) {
		case DRIVE_CDROM:
			PathStripToRoot(buf);
			break;
		}
		m_strRoot = buf;
	}

	DWORDLONG ToClusters(DWORDLONG dlSize)
	{
		if (m_dlTotal == 0 || dlSize == 0 || m_dwBytesPerCluster == 0) return 0;

		div64_t dt = div64(dlSize, (DWORDLONG)m_dwBytesPerCluster);
		if (dt.rem > 0) dt.quot++;
		return dt.quot;
	}

	int UsagePercent()
	{
		if (m_dlTotal) {
			DWORDLONG dlt = Int64ShrlMod32(m_dlTotal, 10);
			DWORDLONG dlf = Int64ShrlMod32(m_dlFree, 10);
			return (100 - MulDiv((int)LODWORD(dlf), 100, (int)LODWORD(dlt)));
		}
		return 0;
	}

	BOOL GetDriveInfo()
	{
		if (GetDriveNumber() == UNC_DRIVE) {
			TCHAR net[MAX_PATH * 2];
			_tcscpy_s(net, ARRAYSIZE(net), m_strRoot.GetString());
			net[_tcslen(net) - 1] = 0;
			if (!ValidateFolderConnection(GetFocus(), net)) {
				m_errorcode = ERROR_NETWORK_ACCESS_DENIED;
				return FALSE;
			}
		}

		// ラベル名
		DWORD dw;
		TCHAR vol[MAX_PATH], fsys[MAX_PATH];
		vol[0] = fsys[0] = 0;

		BOOL bOK = GetVolumeInformation(m_strRoot, vol, ARRAYSIZE(vol), &m_dwVolser, &dw, &m_dwFileSystemFlags, fsys, ARRAYSIZE(fsys));
		m_errorcode = GetLastError();
		if (!bOK && m_strRoot.GetLength() > 3) {
			// ネットワークドライブ内でマウントポイントフォルダの場合にエラーが起きる
			// ボリュームパスをルートパスにして再トライ
			TCHAR root[MAX_PATH];
			_tcscpy_s(root, ARRAYSIZE(root), m_strRoot.GetString()); 
			PathStripToRoot(root);
			bOK = GetVolumeInformation(root, vol, ARRAYSIZE(vol), &m_dwVolser, &dw, &m_dwFileSystemFlags, fsys, ARRAYSIZE(fsys));
			m_errorcode = GetLastError();
		}

		if (bOK) {
			m_strVolume = vol;
			m_strFileSysName = fsys;
			if      (!StrCmpI(fsys, _T("FAT")))		m_nFileSystem = FSTYPE_FAT;
			else if (!StrCmpI(fsys, _T("FAT32")))	m_nFileSystem = FSTYPE_FAT32;
			else if (!StrCmpI(fsys, _T("EXFAT")))	m_nFileSystem = FSTYPE_EXFAT;
			else if (!StrCmpI(fsys, _T("NTFS")))	m_nFileSystem = FSTYPE_NTFS;
			else if (!StrCmpI(fsys, _T("CDFS")))	m_nFileSystem = FSTYPE_CDFS;
			else if ( StrStrI(fsys, _T("UDF")))		m_nFileSystem = FSTYPE_UDF;	// UDF, UDFFS, FS_UDF ...
			else if (!StrCmpI(fsys, _T("HPFS")))	m_nFileSystem = FSTYPE_HPFS;
			else									m_nFileSystem = FSTYPE_UNKNOWN;
		}
		else {
			m_strVolume.Empty();
			m_strFileSysName.Empty();
			return FALSE;
		}
		GetDiskSize();
		return TRUE;
	}

	BOOL GetDiskSize()
	{
		m_dlTotal = 0;
		m_dlFree = 0;

		ULARGE_INTEGER fc, t, f;
		if (SHGetDiskFreeSpaceEx(m_strRoot, &fc, &t, &f)) {
			m_dlTotal = t.QuadPart;
			m_dlFree = fc.QuadPart;
		}

		DWORD spc, bps;
		m_dwBytesPerCluster = 0;
		if (GetDiskFreeSpace(m_strRoot, &spc, &bps, &m_dwFreeClusters, &m_dwClusters)) {
			m_dwBytesPerSector = bps;
			m_dwBytesPerCluster = spc * bps;
			// FAT32は512bytes/sector 8sectors/cluster決め打ち??
			if (m_nFileSystem == FSTYPE_FAT32) {
				m_dwBytesPerSector = 512;
				m_dwBytesPerCluster = 4096;
			}

			if (m_dlTotal == 0) {
				m_dlTotal = UInt32x32To64(m_dwClusters, m_dwBytesPerCluster);
				m_dlFree = UInt32x32To64(m_dwFreeClusters, m_dwBytesPerCluster);
			}
		}
		m_errorcode = GetLastError();
		return (m_dlTotal != 0) ? TRUE : FALSE;
	}

	DWORD GetDescriptionId()
	{
		DWORD descid = 0;
		LPITEMIDLIST pidlFq = ILCreateFromPath(m_strRoot);
		if (pidlFq) {
			LPSHELLFOLDER psf;
			LPITEMIDLIST pidl;
			if (SUCCEEDED(SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, (LPCITEMIDLIST*)&pidl))) {
				SHDESCRIPTIONID sdi;
				memset(&sdi, 0, sizeof(sdi));
				if (SHGetDataFromIDList(psf, pidl, SHGDFIL_DESCRIPTIONID, &sdi, sizeof(sdi)) == NOERROR)
					descid = sdi.dwDescriptionId;
				psf->Release();
			}
			CoTaskMemFree(pidlFq);
		}
		return descid;
	}

	inline int GetDriveNumber() { return (m_strRoot[0] != _T('\\')) ? PathGetDriveNumber(m_strRoot) : UNC_DRIVE; }
	inline bool isSpaceAvailable(DWORDLONG dlSize) { return (m_dlTotal != 0 && dlSize != 0) ? (m_dlFree >= dlSize) : true; }
	inline DWORDLONG ToClustersBytes(DWORDLONG dlSize) { return (ToClusters(dlSize) * (DWORDLONG)m_dwBytesPerCluster); }
	inline LPTSTR SizeToStr(DWORDLONG dwlNum, LPTSTR pszFormat) { return StrFormatByteSize64(dwlNum, pszFormat, 24); }
	inline LPTSTR TotalSizeToStr(LPTSTR pszFormat) { return StrFormatByteSize64(m_dlTotal, pszFormat, 24); }
	inline LPTSTR FreeSizeToStr(LPTSTR pszFormat) { return StrFormatByteSize64(m_dlFree, pszFormat, 24); }
	// filesystem
	inline bool isFAT() { return (m_nFileSystem == FSTYPE_FAT); }
	inline bool isFAT32() { return (m_nFileSystem == FSTYPE_FAT32); }
	inline bool isEXFAT() { return (m_nFileSystem == FSTYPE_EXFAT); }
	inline bool isNTFS() { return (m_nFileSystem == FSTYPE_NTFS); }
	inline bool isCDFS() { return (m_nFileSystem == FSTYPE_CDFS); }
	inline bool isUDF() { return (m_nFileSystem == FSTYPE_UDF); }
	inline bool isHPFS() { return (m_nFileSystem == FSTYPE_HPFS); }
	inline bool isDosDateTime() { return (isFAT() || isFAT32()); }
	// filesystem funcitons
	inline bool isUnicodeStored() { return ((m_dwFileSystemFlags & FS_UNICODE_STORED_ON_DISK) != 0); }
	inline bool isPersistentAcls() { return ((m_dwFileSystemFlags & FS_PERSISTENT_ACLS) != 0); }
	inline DWORD GetError() { return m_errorcode; }
	inline bool hasError() { return (m_errorcode != NOERROR); }
};


#endif		// __DSKSPACE_H__

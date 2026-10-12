////////////////////////////////////////////////////////////
//

#pragma once
#ifndef __CABFOLDER_H__
#define __CABFOLDER_H__

#include "filemap.h"
#include "match_exp.h"

#define CABMAGIC				0xCAB
#define CABSTS_PREVCABINET		0x00000001				// 前のキャビネット有り
#define CABSTS_NEXTCABINET		0x00000002				// 次のキャビネット有り

////////////////////////////////////////////////////////////
//

extern WCHAR gszAst[];
void AttrToString(LPWSTR pszAttr, DWORD attr);


///////////////////////////////////////////////////////////
//

#define cfhdrPREV_CABINET		0x0001
#define cfhdrNEXT_CABINET		0x0002
#define cfhdrRESERVE_PRESENT	0x0004


#pragma pack(1)

/* @@@
	CFHEADER
		CFFOLDER
		  :
		CFFOLDER
			CFILE
			  :
			CFILE
				CFDATA
				  :
				CFDATA
@@@ */

struct CFHEADER
{
	DWORD signature;		/* cabinet file signature */
	DWORD reserved1;		/* reserved */
	DWORD cbCabinet;		/* size of this cabinet file in bytes */
	DWORD reserved2;		/* reserved */
	DWORD coffFiles;		/* offset of the first CFFILE entry */
	DWORD reserved3;		/* reserved */
	BYTE versionMinor;		/* cabinet file format version, minor */
	BYTE versionMajor;		/* cabinet file format version, major */
	WORD cFolders;			/* number of CFFOLDER entries in this cabinet */
	WORD cFiles;			/* number of CFFILE entries in this cabinet */
	WORD flags;				/* cabinet file option indicators */
	WORD setID;				/* must be the same for all cabinets in a set */
	WORD iCabinet;			/* number of this cabinet file in a set */
//	WORD cbCFHeader; 		/* (optional) size of per-cabinet reserved area */
//	BYTE cbCFFolder; 		/* (optional) size of per-folder reserved area */
//	BYTE cbCFData; 			/* (optional) size of per-datablock reserved area */
//	BYTE abReserve[1];		/* (optional) per-cabinet reserved area */
//	BYTE szCabinetPrev[];	/* (optional) name of previous cabinet file */
//	BYTE szDiskPrev[];		/* (optional) name of previous disk */
//	BYTE szCabinetNext[];	/* (optional) name of next cabinet file */
//	BYTE szDiskNext[];		/* (optional) name of next disk */
};

struct CFFOLDER
{
	DWORD coffCabStart;		/* offset of the first CFDATA block in this folder */
	WORD cCFData;			/* number of CFDATA blocks in this folder */
	WORD typeCompress;		/* compression type indicator */
//	BYTE abReserve[1];		/* (optional) per-folder reserved area */
};

struct CFFILE
{
	DWORD cbFile;			/* uncompressed size of this file in bytes */
	DWORD uoffFolderStart;	/* uncompressed offset of this file in the folder */
	WORD iFolder;			/* index into the CFFOLDER area */
	WORD date;				/* date stamp for this file */
	WORD time;				/* time stamp for this file */
	WORD attribs;			/* attribute flags for this file */
    BYTE szName[1];			/* name of this file */
};

struct CFDATA
{
	DWORD csum;				/* checksum of this CFDATA entry */
	WORD cbData;			/* number of compressed bytes in this block */
	WORD cbUncomp;			/* number of uncompressed bytes in this block */
//	BYTE abReserve[];		/* (optional) per-datablock reserved area */
//	BYTE ab[cbData];		/* compressed data bytes */
};

#pragma pack()


////////////////////////////////////////////////////////////
//

class CCabItem
{
public:
	CCabItem()
	{
		m_name.Empty();
		m_size = 0;
		m_attr = 0;
		memset(&m_ft, 0, sizeof(FILETIME));
	}
	~CCabItem() { }

	void SetItem(LPCWSTR pszName, PFDINOTIFICATION pfdin)
	{
		m_name = pszName;
		m_size = pfdin->cb;
		m_attr = pfdin->attribs;
		m_iFolder = pfdin->iFolder;
		DosDateTimeToFileTime(pfdin->date, pfdin->time, &m_ft);
	}

	LPWSTR FormatAttributes(LPWSTR pszFormat)
	{
		if (pszFormat)
			AttrToString(pszFormat, m_attr);
		return pszFormat;
	}

	__inline BOOL GetDosDateTime(WORD* pdt, WORD* ptm) { return FileTimeToDosDateTime(&m_ft, pdt, ptm); }

	CStringW m_name;
	DWORD m_size;
	DWORD m_attr;
	FILETIME m_ft;
	int m_iFolder;				// file's folder index
};

////////////////////////////////////////////////////////////
//

#define CAB_MAXMAPSIZE				0x40000L		// check 256KB

class CCabFolder
{
public:
	CCabFolder()
	{
		m_magic = CABMAGIC;
		m_hwndOwner = NULL;
		m_numItems = 0;
		m_posIndex = 0;
		m_liTotalUncompressedSize.QuadPart = 0;
		m_liTotalCompressedSize.QuadPart = 0;
		m_liMatchedUncompressedSize.QuadPart = 0;
		m_modeOperation = 0;
		m_strCabPath.Empty();
		m_strCabName.Empty();
		m_strFilter.Empty();
		m_cabState = 0;
		memset(&m_wfd, 0, sizeof(WIN32_FIND_DATA));
	}

	~CCabFolder()
	{
		DeleteAllItems();
	}

	BOOL Initialize(HWND hwndOwner, DWORD modeOperation, LPCWSTR pszCabinetFile)
	{
		m_hwndOwner = hwndOwner;
		m_modeOperation = modeOperation;
		m_strCabName = pszCabinetFile;

		WCHAR path[MAX_PATH * 2];
		wcscpy(path, pszCabinetFile);
		PathRemoveFileSpec(path);
		m_strCabPath = path;

		return CheckCabHeader();
	}

	void AddItem(CCabItem* item)
	{
		m_array.Add(item);
		m_liTotalUncompressedSize.QuadPart += item->m_size;
		m_numItems++;
	}

	void DeleteAllItems()
	{
		int cnt = m_array.GetSize();
		for (int i = 0; i < cnt; i++) {
			CCabItem* item = m_array[i];
			if (item)
				delete item;
		}
		m_array.RemoveAll();
		m_numItems = 0;
		m_posIndex = 0;
		m_liTotalUncompressedSize.QuadPart = 0;
		m_liTotalCompressedSize.QuadPart = 0;
	}

	BOOL GetNextCabinet(LPSTR pszNextCab, int cchMax)
	{
		if (pszNextCab && cchMax > 0) {
			if (m_cabState & CABSTS_NEXTCABINET) {
				WideCharToMultiByte(CP_ACP, 0, m_strNextCabinet, -1, pszNextCab, cchMax, NULL, NULL);
				return TRUE;
			}
		}
		return FALSE;
	}

	int FindFirst(LPCWSTR pszFilter, INDIVIDUALINFO* pidi)
	{
		m_posIndex = -1;
		m_liMatchedUncompressedSize.QuadPart = 0;
		if (pszFilter && *pszFilter)
			m_strFilter = pszFilter;
		else
			m_strFilter = gszAst;
		return FindNext(pidi);
	}

	int FindNext(INDIVIDUALINFO* pidi)
	{
		while (++m_posIndex < m_numItems) {
			CCabItem* item = m_array[m_posIndex];
			if (item && match_specW(item->m_name, m_strFilter) == 1) {
				m_liMatchedUncompressedSize.QuadPart += item->m_size;
				if (pidi) {
					memset(pidi, 0, sizeof(INDIVIDUALINFOA));
					pidi->dwOriginalSize = item->m_size;
					WORD dt, tm;
					item->GetDosDateTime(&dt, &tm);
					pidi->wDate = dt;
					pidi->wTime = tm;
					wcscpy(pidi->szFileName, item->m_name);
					item->FormatAttributes(pidi->szAttribute);
					GetCompressionType(item->m_iFolder, pidi->szMode);
				}
				return 0;
			}
		}
		return -1;
	}

	int GetArcFileName(LPWSTR pszName, int cchMax)
	{
		if (pszName && cchMax > 0) {
			wcsncpy(pszName, m_strCabName, cchMax);
			return 0;
		}
		return -1;
	}

	void GetArcDosDateTime(WORD* pdt, WORD* ptm)
	{
		WORD dt, tm;
		FileTimeToDosDateTime(&m_wfd.ftLastWriteTime, &dt, &tm);
		if (pdt) *pdt = dt;
		if (ptm) *ptm = tm;
	}

	BOOL GetArcFileSize(LARGE_INTEGER* pliSize)
	{
		if (pliSize) {
			pliSize->HighPart = m_wfd.nFileSizeHigh;
			pliSize->LowPart = m_wfd.nFileSizeLow;
			return TRUE;
		}
		return FALSE;
	}

	BOOL GetArcOriginalSize(LARGE_INTEGER* pliSize)
	{
		if (pliSize) {
			*pliSize = m_liMatchedUncompressedSize;
			return TRUE;
		}
		return FALSE;
	}

	CCabItem* GetCurrentItem()
	{
		if (m_posIndex < 0 || m_posIndex >= m_numItems)
			return NULL;
		return m_array[m_posIndex];
	}

	int GetFileName(LPWSTR pszName, int cchMax)
	{
		if (pszName && cchMax > 0) {
			*pszName = 0;
			CCabItem* item = GetCurrentItem();
			if (item) {
				wcsncpy(pszName, item->m_name, cchMax);
				return 0;
			}
		}
		return -1;
	}

	int GetMethod(LPWSTR pszMethod, int cchMax)
	{
		if (pszMethod && cchMax > 0) {
			*pszMethod = 0;
			CCabItem* item = GetCurrentItem();
			if (item) {
				WCHAR tmp[32];
				GetCompressionType(item->m_iFolder, tmp);
				wcsncpy(pszMethod, tmp, cchMax);
				return 0;
			}
		}
		return -1;
	}

	DWORD GetOriginalSize()
	{
		CCabItem* item = GetCurrentItem();
		if (item) return item->m_size;

		return (DWORD)-1;
	}

	WORD GetDate()
	{
		CCabItem* item = GetCurrentItem();
		if (item) {
			WORD dt, tm;
			item->GetDosDateTime(&dt, &tm);
			return dt;
		}
		return (WORD)-1;
	}

	WORD GetTime()
	{
		CCabItem* item = GetCurrentItem();
		if (item) {
			WORD dt, tm;
			item->GetDosDateTime(&dt, &tm);
			return tm;
		}
		return (WORD)-1;
	}

	BOOL GetFileTime(FILETIME* pft)
	{
		if (pft) {
			CCabItem* item = GetCurrentItem();
			if (item) {
				*pft = item->m_ft;
				return TRUE;
			}
		}
		return FALSE;
	}

	DWORD GetAttributes()
	{
		CCabItem* item = GetCurrentItem();
		if (item)
			return (item->m_attr & (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH));
		return (DWORD)-1;
	}

	BOOL CheckCabHeader()
	{
		m_comparray.RemoveAll();

		filemap fm;
		if (!fm.open(m_strCabName))
			return FALSE;

		register long s = min(CAB_MAXMAPSIZE, fm.m_size);
		register BYTE* pb = fm.m_base;

		BOOL bFound = FALSE;
		while (s-- >= sizeof(CFHEADER)) {
			if (*((DWORD*)pb) == 0x4643534d &&
				*((DWORD*)(pb + 4)) == 0 &&
				*((DWORD*)(pb + 8)) != 0)
			{
				bFound = TRUE;
				break;
			}
			pb++;
		}

		if (!bFound)
			return FALSE;

		// CFHEADER
		// シグネチャ
		CFHEADER* ph = (CFHEADER*)pb;
		if (ph->signature != 0x4643534d)
			return FALSE;

		// ヘッダサイズ分の読み書きができるかチェック
		if (IsBadReadPtr(pb, sizeof(CFHEADER)))
			return FALSE;

		// フォルダ数を取得
		int numFolder = ph->cFolders;

		pb += sizeof(CFHEADER);
		int rsvHeader = 0, rsvFolder = 0, rsvData = 0;
		if (ph->flags & cfhdrRESERVE_PRESENT) {
			rsvHeader = *(WORD*)pb;
			rsvFolder = *(pb + 2);
			rsvData = *(pb + 3);
			pb += 4 + rsvHeader;
		}

		if (ph->flags & cfhdrPREV_CABINET) {
			m_strHeadPrevCabinet = (char*)pb;
			pb += strlen((char*)pb) + 1;			// skip szCabinetPrev
			m_strHeadPrevDisk = (char*)pb;
			pb += strlen((char*)pb) + 1;			// skip szDiskPrev
		}

		if (ph->flags & cfhdrNEXT_CABINET) {
			m_strHeadNextCabinet = (char*)pb;
			pb += strlen((char*)pb) + 1;			// skip szCabinetNext
			m_strHeadNextDisk = (char*)pb;
			pb += strlen((char*)pb) + 1;			// skip szDiskNext
		}

		// 指定のCFFOLDERを探す
		int i;
		for (i = 0; i < numFolder; i++) {
			CFFOLDER* pf = (CFFOLDER*)pb;
			// フォルダサイズ分の読み書きができるかチェック
			if (IsBadReadPtr(pf, sizeof(CFFOLDER)))
				return FALSE;

			m_comparray.Add(pf->typeCompress);
			pb += sizeof(CFFOLDER) + rsvFolder;
		}
		return TRUE;
	}

	LPWSTR GetCompressionType(int iFolder, LPWSTR pszFormat)
	{
		*pszFormat = 0;
		if (iFolder < m_comparray.GetSize()) {
			unsigned typeCompress = m_comparray[iFolder];
			LPWSTR p;
			switch (typeCompress & tcompMASK_TYPE) {
			case 0:  p = L"stored"; break;
			case 1:  p = L"mszip"; break;
			case 2:  p = L"qtm"; break;
			case 3:  p = L"lzx"; break;
			default: p = L"unk"; break;
			}
			wcscpy(pszFormat, p);
		}
		return pszFormat;
	}

	DWORD m_magic;
	HWND m_hwndOwner;
	WIN32_FIND_DATA m_wfd;
	CSimpleArray<CCabItem*> m_array;
	int m_numItems;
	int m_posIndex;
	LARGE_INTEGER m_liTotalUncompressedSize;
	LARGE_INTEGER m_liTotalCompressedSize;
	LARGE_INTEGER m_liMatchedUncompressedSize;
	DWORD m_modeOperation;
	CStringW m_strCabPath;
	CStringW m_strCabName;
	CStringW m_strNextCabinet;
	CStringW m_strFilter;
	DWORD m_cabState;

	// get from cab_file_header
	CSimpleArray<unsigned> m_comparray;
	CStringA m_strHeadPrevCabinet;
	CStringA m_strHeadPrevDisk;
	CStringA m_strHeadNextCabinet;
	CStringA m_strHeadNextDisk;
};


#endif			//  __CABFOLDER_H__

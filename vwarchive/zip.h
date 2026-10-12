/////////////////////////////////////////////////////////////////////////////
// zip.h
//

//#define ENUMZIP_FROM_V

/////////////////////////////////////////////////////////////////////////////
// Structures

#pragma pack(1)
typedef struct tagLOCALFILEHEADER {
    DWORD   lfhSignature;               // 0x04034B50
    WORD    lfhVerToExtract;
    WORD    lfhFlag;
    WORD    lfhMethod;
    WORD    lfhTime;
    WORD    lfhDate;
    DWORD   lfhCrc;
    DWORD   lfhSizeCompressed;
    DWORD   lfhSizeUncompressed;
    WORD    lfhFileNameLength;
    WORD    lfhExtraLength;
} LOCALFILEHEADER, *PLOCALFILEHEADER;

typedef struct tagFILEHEADER {
    DWORD   fhSignature;                // 0x02014B50
    WORD    fhVerMadeBy;
    WORD    fhVerToExtract;
    WORD    fhFlag;
    WORD    fhMethod;
    WORD    fhTime;
    WORD    fhDate;
    DWORD   fhCrc;
    DWORD   fhSizeCompressed;
    DWORD   fhSizeUncompressed;
    WORD    fhFileNameLength;
    WORD    fhExtraLength;
    WORD    fhCommentLength;
    WORD    fhDiskStart;
    WORD    fhAttrInternal;
    DWORD   fhAttrExternal;
    DWORD   fhOffset;
} FILEHEADER, *PFILEHEADER;

typedef struct tagZIPFILEINFO {
	char    zfiFileName[MAX_PATH];
	WORD    zfiDate;
	WORD    zfiTime;
	DWORD   zfiSizeCompressed;
	DWORD   zfiSizeUncompressed;
	WORD    zfiMethod;
	WORD    zfiFlag;
	DWORD   zfiCrc;
	DWORD   zfiOffset;
	PBYTE   zfiStart;
} ZIPFILEINFO, *PZIPFILEINFO;
#pragma pack()


int GetZipFile(LPCSTR pszFile, CPtrArr& arr);
BOOL __iszipfile(LPCSTR pszName);


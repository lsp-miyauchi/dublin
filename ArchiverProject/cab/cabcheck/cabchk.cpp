#include <windows.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "user32.lib")


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


DWORD CSUMCompute(void *pv, UINT cb, DWORD seed)
{
    int cUlong;                 // Number of ULONGs in block
    DWORD csum;                   // Checksum accumulator
    BYTE* pb;
    ULONG ul;

    cUlong = cb / 4;                    // Number of ULONGs
    csum = seed;                        // Init checksum
    pb = (BYTE*)pv;                            // Start at front of data block

    //** Checksum integral multiple of ULONGs
    while (cUlong-- > 0) {
        //** NOTE: Build ULONG in big/little-endian independent manner
        ul = *pb++;                     // Get low-order byte
        ul |= (((ULONG)(*pb++)) <<  8); // Add 2nd byte
        ul |= (((ULONG)(*pb++)) << 16); // Add 3nd byte
        ul |= (((ULONG)(*pb++)) << 24); // Add 4th byte

        csum ^= ul;                     // Update checksum
    }

    //** Checksum remainder bytes
    ul = 0;
    switch (cb % 4) {
        case 3:
            ul |= (((ULONG)(*pb++)) << 16); // Add 3nd byte
        case 2:
            ul |= (((ULONG)(*pb++)) <<  8); // Add 2nd byte
        case 1:
            ul |= *pb++;                    // Get low-order byte
        default:
            break;
    }
    csum ^= ul;                         // Update checksum

    //** Return computed checksum
    return csum;
}


static void docheck(LPBYTE pBase)
{
	LPBYTE pb = pBase;

	// CFHEADER
	CFHEADER* ph = (CFHEADER*)pb;

	// シグネチャ
	if (ph->signature != 0x4643534d)
	{
		printf("not a cabinet!\n");
		return;
	}

	printf("CFHEADER(0x%08X)\n", pb - pBase);
	printf("  cabinet size = %d\n", ph->cbCabinet);
	printf("  CFFILE offset = 0x%08X\n", ph->coffFiles);
	printf("  version = %d.%d\n", ph->versionMajor, ph->versionMinor);
	printf("  CFFOLDER entries = %d\n", ph->cFolders);
	printf("  CFFILE entries = %d\n", ph->cFiles);
	printf("  flags = 0x%04X\n", ph->flags);
	printf("  setID = 0x%04X\n", ph->setID);
	printf("  set number = %d\n", ph->iCabinet);

	pb += sizeof(CFHEADER);
	int rsvHeader = 0, rsvFolder = 0, rsvData = 0;
	if (ph->flags & cfhdrRESERVE_PRESENT)
	{
		rsvHeader = *(WORD*)pb;
		rsvFolder = *(pb + 2);
		rsvData = *(pb + 3);
		printf("  CFHEADER reserved area = %d\n", rsvHeader);
		printf("  CFFOLDER reserved area = %d\n", rsvFolder);
		printf("  CFDATA reserved area = %d\n", rsvData);
		pb += 4 + rsvHeader;
	}

	if (ph->flags & cfhdrPREV_CABINET)
	{
		printf("  prev cabinet = %s\n", pb);
		pb += lstrlen((LPSTR)pb) + 1;
		printf("  prev disk = %s\n", pb);
		pb += lstrlen((LPSTR)pb) + 1;
	}

	if (ph->flags & cfhdrNEXT_CABINET)
	{
		printf("  next cabinet = %s\n", pb);
		pb += lstrlen((LPSTR)pb) + 1;
		printf("  next disk = %s\n", pb);
		pb += lstrlen((LPSTR)pb) + 1;
	}

	// CFFOLDER
	CFFOLDER* pf;
	for (int iFolder = 0; iFolder < ph->cFolders; iFolder++)
	{
		pf = (CFFOLDER*)pb;
		printf("\nCFFOLDER #%d (0x%08X)\n", iFolder + 1, pb - pBase);
		printf("  CFDATA offset = 0x%08X\n", pf->coffCabStart);
		printf("  CFDATA blocks = %d\n", pf->cCFData);
		printf("  compression type = 0x%04X\n", pf->typeCompress);
		pb += sizeof(CFFOLDER) + rsvFolder;

		// CFDATA
		CFDATA* pd;
		LPBYTE pbw = pBase + pf->coffCabStart;
		for (int i = 0; i < pf->cCFData; i++)
		{
			pd = (CFDATA*)pbw;
			printf("\nCFDATA #%d (0x%08X)\n", i + 1, (LPBYTE)pd - pBase);
			printf("  CFDATA compressed bytes = 0x%08X\n", pd->cbData);
			printf("  CFDATA uncompressed bytes = 0x%08X\n", pd->cbUncomp);
			printf("  src checksum = 0x%08X\n", pd->csum);
			pbw += sizeof(CFDATA) + rsvData;

			DWORD csumPartial = CSUMCompute(pbw, pd->cbData, 0);
			DWORD csum = CSUMCompute(&pd->cbData,
							sizeof(CFDATA) + rsvData - sizeof(DWORD), csumPartial);
			printf("  dst checksum = 0x%08X\n", csum);
			pbw += pd->cbData;
		}
	}

	// CFFILE
	CFFILE* pi;
	for (int iFile = 0; iFile < ph->cFiles; iFile++)
	{
		pi = (CFFILE*)pb;
		printf("\nCFFILE #%d (0x%08X)\n", iFile + 1, pb - pBase);
		printf("  CFFOLDER index = %d\n", pi->iFolder);
		printf("  CFFOLDER uncompressed offset = 0x%08X\n", pi->uoffFolderStart);
		printf("  file name = %s\n", pi->szName);
		printf("  file size = %d\n", pi->cbFile);
		pb += sizeof(CFFILE) + lstrlen((LPCSTR)pi->szName);
	}
}

void main(int argc, char** argv)
{
	if (argc < 2)
		return;

	HANDLE hf = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE)
	{
		HANDLE hm = CreateFileMapping(hf, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hm)
		{
			LPBYTE pBase = (LPBYTE)MapViewOfFile(hm, FILE_MAP_READ, 0, 0, 0);
			if (pBase)
			{
				docheck(pBase);
				UnmapViewOfFile(pBase);
			}
			CloseHandle(hm);
		}
		CloseHandle(hf);
	}
}

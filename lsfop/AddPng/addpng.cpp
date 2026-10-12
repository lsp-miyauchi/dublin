////////////////////////////////////////////////////////////////////////
// addpng.cpp

#include <windows.h>
#include <stdio.h>
#include <string.h>


#define ICOVER					0x00030000
#define WIDTHBYTES(bits)		((((bits) + 31)>>5)<<2)

#pragma pack( push )
#pragma pack( 2 )
typedef struct {
	BYTE	width;					// Width of the image
	BYTE	height;					// Height of the image (times 2)
	BYTE	colors;					// Number of colors in image (0 if >=8bpp)
	BYTE	reserved;				// Reserved
	WORD	planes;					// Color Planes
	WORD	bits;					// Bits per pixel
	DWORD	numbytes;				// how many bytes in this resource?
	WORD	id;						// the ID
} MEMICONDIRENTRY, *LPMEMICONDIRENTRY;

typedef struct {
	WORD			reserved;		// Reserved
	WORD			type;			// resource type (1 for icons)
	WORD			num;			// how many images?
	MEMICONDIRENTRY	memdir[1];		// the entries for each image
} MEMICONDIR, *LPMEMICONDIR;
#pragma pack( pop )

// 次の２つの構造体は、どのようにアイコン情報が.icoファイルに格納されて
// いるかを示す
typedef struct {
	BYTE	width;					// Width of the image
	BYTE	height;					// Height of the image (times 2)
	BYTE	colors;					// Number of colors in image (0 if >=8bpp)
	BYTE	reserved;				// Reserved
	WORD	planes;					// Color Planes
	WORD	bits;					// Bits per pixel
	DWORD	numbytes;				// how many bytes in this resource?
	DWORD	offset;					// where in the file is this image
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct {
	WORD			reserved;		// Reserved
	WORD			type;			// resource type (1 for icons)
	WORD			num;			// how many images?
	ICONDIRENTRY	dir[1];			// the entries for each image
} ICONDIR, *LPICONDIR;


// 次の２つの構造体は、本プログラムでアイコンを取り扱うために利用するもの
// である
// これらは上の構造体よりこのプログラムのオペレーションに、より緊密に拘束
// されます。主要な違いのうちの1つは、マスクのDIB情報にそれらがポインタを
// 供給するということです。

typedef struct {
	UINT			width;
	UINT			height;
	UINT			colors;					// bpp
	LPBYTE			pBitData;				// ptr to DIB bits
	DWORD			numbytes;				// how many bytes?
	__inline LPBITMAPINFO BitmapInfoPtr() { return (LPBITMAPINFO)pBitData; }
	__inline bool IsPng() { return (pBitData != NULL && (*((DWORD*)pBitData) == 0x474e5089)); }
} ICONIMAGE, *LPICONIMAGE;

typedef struct {
	BOOL bUpdate;							// Has image changed?
	TCHAR szName[MAX_PATH * 2];				// Original name
	BOOL bIconFile;
	UINT num;								// How many images?
	ICONIMAGE images[1];					// Image entries
} ICONRESOURCE, *LPICONRESOURCE;


#pragma pack(1)
typedef struct tagEZPNGHEADER {
	DWORD magic;
	DWORD dummy[3];
	DWORD msbl_width;
	DWORD msbl_height;
} EZPNGHEADER;
#pragma pack()


WORD dib_numcolors(void* pbi)
{
	WORD wBitCount;
	DWORD dwClrUsed = ((BITMAPINFOHEADER*)pbi)->biClrUsed;
	if (dwClrUsed)
		return (WORD)dwClrUsed;

	wBitCount = ((BITMAPINFOHEADER*)pbi)->biBitCount;
	switch (wBitCount) {
		case 1: return 2;
		case 4: return 16;
		case 8:	return 256;
		default:return 0;
	}
	return 0;
}

__forceinline WORD dib_palsize(void* pbi)
	{ return (dib_numcolors(pbi) * sizeof(RGBQUAD)); }
__forceinline void* dib_findbits(void* pbi)
	{ return (void*)((BYTE*)pbi + *(DWORD*)pbi + dib_palsize(pbi)); }
__forceinline DWORD dib_bytesline(LPBITMAPINFOHEADER pbh)
	{ return WIDTHBYTES(pbh->biWidth * pbh->biPlanes * pbh->biBitCount); }

ICONRESOURCE* AllocIconRes(int num)
{
	DWORD size = sizeof(ICONRESOURCE) + (num - 1) * sizeof(ICONIMAGE);
	ICONRESOURCE* pir = (ICONRESOURCE*)malloc(size);
	if (pir) {
		memset(pir, 0, size);
		pir->num = num;
	}
	return pir;
}

void FreeIconRes(ICONRESOURCE* pir)
{
	for (UINT i = 0; i < pir->num; i++) {
		if (pir->images[i].pBitData)
			free(pir->images[i].pBitData);
	}
	free(pir);
}

UINT read_icoheader(HANDLE hf)
{
	WORD w[3];
	DWORD dw;
	if (ReadFile(hf, w, sizeof(w), &dw, NULL) &&
		dw == sizeof(w) && w[0] == 0 && w[1] == 1)
		return w[2];
	return (UINT)-1;
}

static BOOL adjust_iconimage(ICONIMAGE* pii)
{
	if (!pii) return FALSE;

	// VistaではPNG形式でアイコンが保存されていることがある
	if (pii->IsPng()) {
		EZPNGHEADER* pnghdr = (EZPNGHEADER*)pii->pBitData;

		pii->width = MAKELONG(
							MAKEWORD(HIBYTE(HIWORD(pnghdr->msbl_width)), LOBYTE(HIWORD(pnghdr->msbl_width))),
							MAKEWORD(HIBYTE(LOWORD(pnghdr->msbl_width)), LOBYTE(LOWORD(pnghdr->msbl_width)))
						);
		pii->height = MAKELONG(
							MAKEWORD(HIBYTE(HIWORD(pnghdr->msbl_height)), LOBYTE(HIWORD(pnghdr->msbl_height))),
							MAKEWORD(HIBYTE(LOWORD(pnghdr->msbl_height)), LOBYTE(LOWORD(pnghdr->msbl_height)))
						);
		pii->colors = 24;
	}
	else {
		BITMAPINFOHEADER* hdr = (BITMAPINFOHEADER*)pii->pBitData;
		if (hdr->biPlanes != 1) return FALSE;

		switch (hdr->biBitCount) {
		case 1:
		case 4:
		case 8:
		case 16:
		case 24:
		case 32: break;
		default: return FALSE;
		}
		switch (hdr->biCompression) {
		case BI_RGB:
		case BI_RLE8:
		case BI_RLE4:
		case BI_BITFIELDS: break;
		default: return FALSE;
		}

	    // BITMAPINFO is at beginning of bits
		pii->width = hdr->biWidth;

		// Icons are stored in funky format where height is doubled
		pii->height = (hdr->biHeight) / 2;
		pii->colors = hdr->biPlanes * hdr->biBitCount;
	}
	return TRUE;
}

ICONRESOURCE* ReadIconFromICOFile(LPCTSTR pszName)
{
	HANDLE hf = CreateFile(pszName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return NULL;

    // Read in the header
	UINT num = read_icoheader(hf);
	if (num == (UINT)-1) {
		CloseHandle(hf);
		return NULL;
	}

	// Adjust the size of the struct to account for the images
	ICONRESOURCE* pir = AllocIconRes(num);
	if (!pir) {
		CloseHandle(hf);
		return NULL;
    }

	// Read in the icon directory entries
	ICONDIRENTRY* pde = new ICONDIRENTRY[num];
	DWORD dw;
	if (!ReadFile(hf, pde, num * sizeof(ICONDIRENTRY), &dw, NULL) ||
		dw != num * sizeof(ICONDIRENTRY))
	{
		CloseHandle(hf);
		FreeIconRes(pir);
		return NULL;
	}

	// Loop through and read in each image
	for (UINT i = 0; i < num; i++) {
		// Allocate memory for the resource
		if ((pir->images[i].pBitData = (BYTE*)malloc(pde[i].numbytes)) == NULL) {
			CloseHandle(hf);
			FreeIconRes(pir);
			delete [] pde;
			return NULL;
		}
		pir->images[i].numbytes = pde[i].numbytes;

		// Read it in
		if (SetFilePointer(hf, pde[i].offset, NULL, FILE_BEGIN) == 0xFFFFFFFF ||
			!ReadFile(hf, pir->images[i].pBitData, pde[i].numbytes, &dw, NULL) ||
			dw != pde[i].numbytes ||
			!adjust_iconimage(&pir->images[i]))
        {
            CloseHandle(hf);
			FreeIconRes(pir);
			delete [] pde;
			return NULL;
		}
	}
	delete [] pde;
	CloseHandle(hf);
	return pir;
}

static BOOL write_icoheader(HANDLE hf, UINT entries)
{
	WORD w[3];
    DWORD dw;

	w[0] = 0;
	w[1] = 1;
	w[2] = (WORD)entries;
	return (WriteFile(hf, w, sizeof(w), &dw, NULL) && dw == sizeof(w));
}

static DWORD calc_imgoffset(ICONRESOURCE* pir, int index)
{
    // Calculate the ICO header size
	DWORD size = 3 * sizeof(WORD);

	// Add the ICONDIRENTRY's
	size += pir->num * sizeof(ICONDIRENTRY);

	// Add the sizes of the previous images
	for (int i = 0; i < index; i++)
		size += pir->images[i].numbytes;
	return size;
}

BOOL WriteIconToICOFile(LPICONRESOURCE pir, LPCTSTR szFileName)
{
    HANDLE    	hFile;
    UINT        i;
    DWORD    	dwWritten;

    // open the file
	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

    // Write the header
	if (!write_icoheader(hFile, pir->num)) {
		CloseHandle( hFile );
		return FALSE;
    }

    // Write the ICONDIRENTRY's
	LPBITMAPINFO pbi;
	for (i = 0; i < pir->num; i++) {
        // Convert internal format to ICONDIRENTRY
		ICONDIRENTRY ide;
		ide.width = pir->images[i].width;
		ide.height = pir->images[i].height;
		ide.reserved = 0;

		if (pir->images[i].IsPng()) {
			ide.planes = 1;
			ide.bits = 24;
			ide.colors = 0;
		}
		else {
			pbi = pir->images[i].BitmapInfoPtr();
			ide.planes = pbi->bmiHeader.biPlanes;
			ide.bits = pbi->bmiHeader.biBitCount;
			if ((ide.planes * ide.bits) >= 8)
				ide.colors = 0;
        	else
				ide.colors = 1 << (ide.planes * ide.bits);
		}

		ide.numbytes = pir->images[i].numbytes;
		ide.offset = calc_imgoffset(pir, i);

		// Write the ICONDIRENTRY out to disk
		if (!WriteFile(hFile, &ide, sizeof(ICONDIRENTRY), &dwWritten, NULL))
			return FALSE;

        // Did we write a full ICONDIRENTRY ?
		if (dwWritten != sizeof(ICONDIRENTRY))
            return FALSE;
	}

    // Write the image bits for each image
	for (i = 0; i < pir->num; i++ ) {
		bool bPng = pir->images[i].IsPng();

		pbi = pir->images[i].BitmapInfoPtr();
		DWORD dwTemp = pbi->bmiHeader.biSizeImage;

		// Set the sizeimage member to zero
		if (!bPng) pbi->bmiHeader.biSizeImage = 0;

		// Write the image bits to file
		if (!WriteFile(hFile, pbi, pir->images[i].numbytes, &dwWritten, NULL))
			return FALSE;

		if (dwWritten != pir->images[i].numbytes)
            return FALSE;

        // set it back
		if (!bPng) pbi->bmiHeader.biSizeImage = dwTemp;
	}
	CloseHandle( hFile );
	return TRUE;
}

void main(int argc, char** argv)
{
	if (argc != 3) {
		printf("usage: addpng <ico-file> <png-file>\n");
		return;
	}

	ICONRESOURCE* pir = ReadIconFromICOFile(argv[1]);
	if (!pir) {
		printf("cannot load icon data\n");
		return;
	}

	HANDLE hf = CreateFile(argv[2], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf == INVALID_HANDLE_VALUE) {
		FreeIconRes(pir);
		printf("cannot open png file\n");
		return;
	}

	DWORD size = GetFileSize(hf, NULL);
	void* pngdata = malloc(size);
	DWORD dw;
	if (!ReadFile(hf, pngdata, size, &dw, NULL) || dw != size) {
		CloseHandle(hf);
		FreeIconRes(pir);
		free(pngdata);
		printf("cannot read png file\n");
		return;
	}
	CloseHandle(hf);
	if (*((DWORD*)pngdata) != 0x474e5089) {
		FreeIconRes(pir);
		free(pngdata);
		printf("%s is NOT png file\n", argv[2]);
		return;
	}

	int entries = pir->num + 1;
	ICONRESOURCE* pirT = AllocIconRes(entries);
	if (!pirT) {
		FreeIconRes(pir);
		free(pngdata);
		printf("memory error\n");
		return;
	}

	memcpy(pirT, pir, sizeof(ICONRESOURCE));
	pirT->num = entries;
	for (int i = 0; i < pir->num; i++) {
		memcpy(&pirT->images[i + 1], &pir->images[i], sizeof(ICONIMAGE));
	}
	free(pir);
	pir = pirT;

	EZPNGHEADER* pnghdr = (EZPNGHEADER*)pngdata;

	pir->images[0].width =
		MAKELONG(
			MAKEWORD(HIBYTE(HIWORD(pnghdr->msbl_width)), LOBYTE(HIWORD(pnghdr->msbl_width))),
			MAKEWORD(HIBYTE(LOWORD(pnghdr->msbl_width)), LOBYTE(LOWORD(pnghdr->msbl_width)))
		);
	pir->images[0].height = 
		MAKELONG(
			MAKEWORD(HIBYTE(HIWORD(pnghdr->msbl_height)), LOBYTE(HIWORD(pnghdr->msbl_height))),
			MAKEWORD(HIBYTE(LOWORD(pnghdr->msbl_height)), LOBYTE(LOWORD(pnghdr->msbl_height)))
		);
	pir->images[0].colors = 24;
	pir->images[0].pBitData = (BYTE*)pngdata;
	pir->images[0].numbytes = size;

	char name[MAX_PATH];
	lstrcpy(name, argv[1]);
	lstrcat(name, "_png.ico");
	WriteIconToICOFile(pir, name);

	FreeIconRes(pir);
}

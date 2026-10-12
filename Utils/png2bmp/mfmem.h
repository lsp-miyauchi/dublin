//{{NO_DEPENDENCIES}}
//
// FUNCTION PROTOTYPE DEFINITION
//


////////////////////////////////////////////////////////////////
// local memoryfile
//

typedef struct tagMF {
	HGLOBAL hmem;
	LPBYTE pTop;
	LPBYTE pRead;
	DWORD dwSize;
	DWORD dwOffset;
} MF;
typedef MF* LPMF;


////////////////////////////////////////////////////////////////
// function prototypes
//

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

HGLOBAL readfileall(LPCTSTR lpFile, LPDWORD pdwRead);

LPMF _mfopen(LPCTSTR pszName);
LPMF _mfmemopen(HGLOBAL hmem, DWORD dwSize);
LPMF _mfattach(HGLOBAL hmem);
HGLOBAL _mfdetach(LPMF pmf, DWORD* pdwSize);
LPMF _mfnew(DWORD dwSize);
void _mfclose(LPMF lp);
BYTE _mfgetc(LPMF lp);
BYTE _mfgetc_ns(LPMF lp);
WORD _mfgetw(LPMF lp);
WORD _mfgetw_ns(LPMF lp);
int _mfread(LPMF lp, LPVOID lpv, DWORD size);
BOOL _mfputc(LPMF lp, BYTE ch);
BOOL _mfputw(LPMF lp, WORD ch);
DWORD _mfwrite(LPMF lp, LPVOID lpv, DWORD size);
long _mfseek(LPMF lp, long lOffset, int nOrign);
BOOL _mfeof(LPMF lp);
BOOL _mfsetsize(LPMF lp, DWORD dwSize);
BOOL _mfsave(LPMF lp, LPSTR pszName);
HGLOBAL _mfdup(LPMF lp, DWORD* pdwSize = NULL);
ULONG _mf_msb1streadlong(LPMF lp);
USHORT _mf_msb1streadshort(LPMF lp);
DWORD _mf_msb1stwritelong(LPMF pmf, ULONG value);
DWORD _mf_msb1stwriteshort(LPMF pmf, ULONG value);

#ifdef __cplusplus
}
#endif

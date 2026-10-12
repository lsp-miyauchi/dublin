////////////////////////////////////////////////////////////////////
// common.h


////////////////////////////////////////////////////////////////////
// FUNCTIONS
//

int AppMessage(HWND hwnd, UINT idCaption, UINT idMessage, LPCTSTR pszInsert = NULL, UINT flags = MB_ICONSTOP | MB_OK);
HGLOBAL __fastcall copyhandle(HGLOBAL hsrc);
int __fastcall dib_numcolors(void* pv);
int __fastcall dib_palsize(void* pv);
BOOL __fastcall dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits);

_inline BYTE* __fastcall dib_findbits(void* pv)
	{ return ((BYTE*)pv + *(DWORD*)pv + dib_palsize(pv)); }


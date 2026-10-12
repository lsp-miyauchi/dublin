//////////////////////////////////////////////////////////////////////
// fastopt.h

#define _SLEN(a)		(_slen((a)))
#define _SCPY(a,b)		(_scpy((a),(b)))
#define _SCAT(a,b)		(_scat((a),(b)))


int __fastcall _slen(LPCTSTR pstr);
LPTSTR __fastcall _scpy(LPTSTR pdst, LPCTSTR psrc);
LPTSTR _scat(LPTSTR pdst, LPCTSTR psrc);


__forceinline LPTSTR _scat(LPTSTR pdst, LPCTSTR psrc)
	{ _scpy(pdst + _slen(pdst), psrc); return pdst; }

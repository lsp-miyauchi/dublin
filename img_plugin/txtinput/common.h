////////////////////////////////////////////////////////////////////
// common.h



/////////////////////////////////////////////////////////////////////////////
// GLOBAL MEMORY MACRO 
//
#ifndef GlobalAllocPtr
#define     GetInstanceModule(hInstance) \
                GetModuleHandle((LPCTSTR)MAKELP(0, hInstance))

#define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalLockPtr(lp)           \
                ((BOOL)SELECTOROF(GlobalLock(GlobalPtrHandle(lp))))
#define     GlobalUnlockPtr(lp)         \
                GlobalUnlock(GlobalPtrHandle(lp))

#define     GlobalAllocPtr(flags, cb)   \
                (GlobalLock(GlobalAlloc((flags), (cb))))
#define     GlobalReAllocPtr(lp, cbNew, flags)  \
                (GlobalUnlockPtr(lp), GlobalLock(GlobalReAlloc(GlobalPtrHandle(lp) , (cbNew), (flags))))
#define     GlobalFreePtr(lp)           \
                (GlobalUnlockPtr(lp), (BOOL)GlobalFree(GlobalPtrHandle(lp)))
#endif

////////////////////////////////////////////////////////////////////
// FUNCTIONS
//

HGLOBAL __fastcall copyhandle(HGLOBAL hsrc);
int __fastcall dib_numcolors(void* pv);
int __fastcall dib_palsize(void* pv);
BOOL __fastcall dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits);

_inline BYTE* __fastcall dib_findbits(void* pv)
	{ return ((BYTE*)pv + *(DWORD*)pv + dib_palsize(pv)); }


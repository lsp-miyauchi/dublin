////////////////////////////////////////////////////////////////////////
// shellimagestore_sup.h

#define USE_SHELLIMAGE_STORE

#pragma once
#ifndef __SHELLIMAGESTORE_SUP_H__
#define __SHELLIMAGESTORE_SUP_H__

// {48C8118C-B924-11d1-98D5-00C04FB687DA}
//DEFINE_GUID(IID_IShellImageStore,            0x48c8118c, 0xb924, 0x11d1, 0x98, 0xd5, 0x0, 0xc0, 0x4f, 0xb6, 0x87, 0xda);

// {1EBDCF80-A200-11d0-A3A4-00C04FD706EC}
//DEFINE_GUID(CLSID_ShellThumbnailDiskCache,   0x1ebdcf80, 0xa200, 0x11d0, 0xa3, 0xa4, 0x0, 0xc0, 0x4f, 0xd7, 0x6, 0xec);
const CLSID CLSID_ShellThumbnailDiskCache = {0x1ebdcf80, 0xa200, 0x11d0, {0xa3, 0xa4, 0x0, 0xc0, 0x4f, 0xd7, 0x6, 0xec}};


#include <pshpack8.h>

typedef struct _EnumImageStoreDATAtag
{
    WCHAR     szPath[MAX_PATH];
    FILETIME  ftTimeStamp;
} ENUMSHELLIMAGESTOREDATA, * PENUMSHELLIMAGESTOREDATA;

#include <poppack.h>        /* Return to byte packing */

#undef  INTERFACE
#define INTERFACE   IEnumShellImageStore

DECLARE_INTERFACE_IID_( IEnumShellImageStore, IUnknown, "6DFD582B-92E3-11D1-98A3-00C04FB687DA" )
{
    STDMETHOD ( QueryInterface ) ( THIS_ REFIID riid, void **ppv ) PURE;
    STDMETHOD_( ULONG, AddRef ) ( THIS ) PURE;
    STDMETHOD_( ULONG, Release ) ( THIS ) PURE;

    STDMETHOD ( Reset ) ( THIS ) PURE;
    STDMETHOD ( Next ) ( THIS_ ULONG celt, PENUMSHELLIMAGESTOREDATA * prgElt, ULONG * pceltFetched ) PURE;
    STDMETHOD ( Skip ) ( THIS_ ULONG celt ) PURE;
    STDMETHOD ( Clone ) ( THIS_ IEnumShellImageStore ** ppEnum ) PURE;
};

typedef IEnumShellImageStore * LPENUMSHELLIMAGESTORE;


// flags used to determine the capabilities of the storage for the images
#define SHIMSTCAPFLAG_LOCKABLE    0x0001       // does the store require/support locking
#define SHIMSTCAPFLAG_PURGEABLE   0x0002       // does the store require dead items purging externally ?

#undef  INTERFACE
#define INTERFACE   IShellImageStore

// this interface is used to manipulate the Image cache. It can potentially be used
// in a free threaded manner in conjunction with the Lock parameter to Open and close
DECLARE_INTERFACE_IID_( IShellImageStore, IUnknown, "48C8118C-B924-11D1-98D5-00C04FB687DA" )
{
    STDMETHOD ( QueryInterface )( THIS_ REFIID riid, void **ppv ) PURE;
    STDMETHOD_( ULONG, AddRef ) ( THIS ) PURE;
    STDMETHOD_( ULONG, Release ) ( THIS ) PURE;

    // if the lock parameter is used, then all other calls into
    // open and/or create will block until the lock is released.
    STDMETHOD ( Open ) ( THIS_ DWORD dwMode, DWORD * pdwLock ) PURE;
    STDMETHOD ( Create ) ( THIS_ DWORD dwMode, DWORD * pdwLock ) PURE;

    // if the lock is passed to either of these two methods, it releases the lock
    // once the operation is complete.
    STDMETHOD ( ReleaseLock ) ( THIS_ DWORD const * pdwLock ) PURE;
    STDMETHOD ( Close ) ( THIS_ DWORD const * pdwLock ) PURE;
    STDMETHOD ( Commit ) ( THIS_ DWORD const * pdwLock ) PURE;
    STDMETHOD ( IsLocked ) ( THIS ) PURE;

    STDMETHOD ( GetMode ) ( THIS_ DWORD * pdwMode ) PURE;
    STDMETHOD ( GetCapabilities ) ( THIS_ DWORD * pdwCapMask ) PURE;

    STDMETHOD ( AddEntry ) ( THIS_ LPCWSTR pszName, const FILETIME * pftTimeStamp, DWORD dwMode, HBITMAP hImage ) PURE;
    STDMETHOD ( GetEntry ) ( THIS_ LPCWSTR pszName, DWORD dwMode, HBITMAP * phImage ) PURE;
    STDMETHOD ( DeleteEntry ) ( THIS_ LPCWSTR pszName ) PURE;
    STDMETHOD ( IsEntryInStore ) ( THIS_ LPCWSTR pszName, FILETIME * pftTimeStamp ) PURE;

    STDMETHOD ( Enum ) ( THIS_ LPENUMSHELLIMAGESTORE * ppEnum ) PURE;
};

typedef IShellImageStore * LPSHELLIMAGESTORE;



#endif		// __SHELLIMAGESTORE_SUP_H__

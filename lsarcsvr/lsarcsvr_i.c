

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for lsarcsvr.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        EXTERN_C __declspec(selectany) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif // !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_ICommArchiver,0xBECA4F33,0xDA70,0x489C,0x8A,0xB7,0xC7,0x69,0x04,0x4C,0xAE,0xD5);


MIDL_DEFINE_GUID(IID, IID_IArcManager,0xC5B0F895,0x28CE,0x4063,0x8E,0x32,0x8C,0x12,0x2B,0x52,0xDB,0xCD);


MIDL_DEFINE_GUID(IID, LIBID_lsarcsvrLib,0xB6796ACE,0xD61D,0x41DE,0xAD,0xB2,0x14,0x21,0x16,0x2B,0x79,0xBF);


MIDL_DEFINE_GUID(CLSID, CLSID_CommArchiver,0x26D6C970,0xA507,0x4943,0xB2,0xD5,0xD0,0xD1,0xAC,0x4E,0xA7,0xDF);


MIDL_DEFINE_GUID(CLSID, CLSID_ArcManager,0xE51300C0,0xDF07,0x4ED4,0xB6,0x29,0x2F,0x34,0x58,0xA3,0x84,0x96);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif




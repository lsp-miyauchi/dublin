

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for lspexif.idl:
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

MIDL_DEFINE_GUID(IID, IID_IExifParser,0x53C45C07,0xF5F1,0x4773,0x97,0x00,0x0F,0xA3,0xBA,0x8D,0x34,0x99);


MIDL_DEFINE_GUID(IID, IID_IRawThmubnail,0x057A7B27,0x9C4D,0x4C48,0x82,0xBF,0x8F,0x40,0xF0,0x4F,0x1F,0xA7);


MIDL_DEFINE_GUID(IID, LIBID_LSExifLib,0xD7FC57C2,0xDC5B,0x4249,0x9F,0x98,0x63,0xD2,0x86,0xF8,0x70,0x9D);


MIDL_DEFINE_GUID(CLSID, CLSID_ExifParser,0xE5B0C991,0xE0F9,0x4CC2,0x81,0x2A,0xBD,0xE4,0x22,0x93,0x5E,0x0F);


MIDL_DEFINE_GUID(CLSID, CLSID_RawThmubnail,0x12524D4C,0x6E12,0x4615,0x98,0x7D,0x94,0xBB,0x7E,0x1C,0x1D,0x8B);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif




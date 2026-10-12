

/* this ALWAYS GENERATED file contains the proxy stub code */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for lsshell.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#if defined(_M_AMD64)


#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#include "ndr64types.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif /* __RPCPROXY_H_VERSION__ */


#include "lsshell_i.h"

#define TYPE_FORMAT_STRING_SIZE   37                                
#define PROC_FORMAT_STRING_SIZE   39                                
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

typedef struct _lsshell_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } lsshell_MIDL_TYPE_FORMAT_STRING;

typedef struct _lsshell_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } lsshell_MIDL_PROC_FORMAT_STRING;

typedef struct _lsshell_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } lsshell_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax_2_0 = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};

static const RPC_SYNTAX_IDENTIFIER  _NDR64_RpcTransferSyntax_1_0 = 
{{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}};

#if defined(_CONTROL_FLOW_GUARD_XFG)
#define XFG_TRAMPOLINES(ObjectType)\
NDR_SHAREABLE unsigned long ObjectType ## _UserSize_XFG(unsigned long * pFlags, unsigned long Offset, void * pObject)\
{\
return  ObjectType ## _UserSize(pFlags, Offset, (ObjectType *)pObject);\
}\
NDR_SHAREABLE unsigned char * ObjectType ## _UserMarshal_XFG(unsigned long * pFlags, unsigned char * pBuffer, void * pObject)\
{\
return ObjectType ## _UserMarshal(pFlags, pBuffer, (ObjectType *)pObject);\
}\
NDR_SHAREABLE unsigned char * ObjectType ## _UserUnmarshal_XFG(unsigned long * pFlags, unsigned char * pBuffer, void * pObject)\
{\
return ObjectType ## _UserUnmarshal(pFlags, pBuffer, (ObjectType *)pObject);\
}\
NDR_SHAREABLE void ObjectType ## _UserFree_XFG(unsigned long * pFlags, void * pObject)\
{\
ObjectType ## _UserFree(pFlags, (ObjectType *)pObject);\
}
#define XFG_TRAMPOLINES64(ObjectType)\
NDR_SHAREABLE unsigned long ObjectType ## _UserSize64_XFG(unsigned long * pFlags, unsigned long Offset, void * pObject)\
{\
return  ObjectType ## _UserSize64(pFlags, Offset, (ObjectType *)pObject);\
}\
NDR_SHAREABLE unsigned char * ObjectType ## _UserMarshal64_XFG(unsigned long * pFlags, unsigned char * pBuffer, void * pObject)\
{\
return ObjectType ## _UserMarshal64(pFlags, pBuffer, (ObjectType *)pObject);\
}\
NDR_SHAREABLE unsigned char * ObjectType ## _UserUnmarshal64_XFG(unsigned long * pFlags, unsigned char * pBuffer, void * pObject)\
{\
return ObjectType ## _UserUnmarshal64(pFlags, pBuffer, (ObjectType *)pObject);\
}\
NDR_SHAREABLE void ObjectType ## _UserFree64_XFG(unsigned long * pFlags, void * pObject)\
{\
ObjectType ## _UserFree64(pFlags, (ObjectType *)pObject);\
}
#define XFG_BIND_TRAMPOLINES(HandleType, ObjectType)\
static void* ObjectType ## _bind_XFG(HandleType pObject)\
{\
return ObjectType ## _bind((ObjectType) pObject);\
}\
static void ObjectType ## _unbind_XFG(HandleType pObject, handle_t ServerHandle)\
{\
ObjectType ## _unbind((ObjectType) pObject, ServerHandle);\
}
#define XFG_TRAMPOLINE_FPTR(Function) Function ## _XFG
#define XFG_TRAMPOLINE_FPTR_DEPENDENT_SYMBOL(Symbol) Symbol ## _XFG
#else
#define XFG_TRAMPOLINES(ObjectType)
#define XFG_TRAMPOLINES64(ObjectType)
#define XFG_BIND_TRAMPOLINES(HandleType, ObjectType)
#define XFG_TRAMPOLINE_FPTR(Function) Function
#define XFG_TRAMPOLINE_FPTR_DEPENDENT_SYMBOL(Symbol) Symbol
#endif



extern const lsshell_MIDL_TYPE_FORMAT_STRING lsshell__MIDL_TypeFormatString;
extern const lsshell_MIDL_PROC_FORMAT_STRING lsshell__MIDL_ProcFormatString;
extern const lsshell_MIDL_EXPR_FORMAT_STRING lsshell__MIDL_ExprFormatString;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO ICtx_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICtx_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IAddPack_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IAddPack_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IUnpackTo_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IUnpackTo_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IPackTo_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPackTo_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IJunctionTo_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IJunctionTo_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO ICopyTo_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICopyTo_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IThumbOutTo_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IThumbOutTo_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IExifProp_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IExifProp_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IImageTools_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IImageTools_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO ILSDecrypt_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ILSDecrypt_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IIconProp_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IIconProp_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE NDR64_UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const lsshell_MIDL_PROC_FORMAT_STRING lsshell__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure Setup */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x7 ),	/* 7 */
/*  8 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 16 */	0xa,		/* 10 */
			0x45,		/* Ext Flags:  new corr desc, srv corr check, has range on conformance */
/* 18 */	NdrFcShort( 0x0 ),	/* 0 */
/* 20 */	NdrFcShort( 0x1 ),	/* 1 */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter hwnd */

/* 26 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 28 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 30 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Return value */

/* 32 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 34 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 36 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const lsshell_MIDL_TYPE_FORMAT_STRING lsshell__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x12, 0x0,	/* FC_UP */
/*  4 */	NdrFcShort( 0x2 ),	/* Offset= 2 (6) */
/*  6 */	
			0x2a,		/* FC_ENCAPSULATED_UNION */
			0x48,		/* 72 */
/*  8 */	NdrFcShort( 0x4 ),	/* 4 */
/* 10 */	NdrFcShort( 0x2 ),	/* 2 */
/* 12 */	NdrFcLong( 0x48746457 ),	/* 1215587415 */
/* 16 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 18 */	NdrFcLong( 0x52746457 ),	/* 1383359575 */
/* 22 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 24 */	NdrFcShort( 0xffff ),	/* Offset= -1 (23) */
/* 26 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 28 */	NdrFcShort( 0x0 ),	/* 0 */
/* 30 */	NdrFcShort( 0x8 ),	/* 8 */
/* 32 */	NdrFcShort( 0x0 ),	/* 0 */
/* 34 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (2) */

			0x0
        }
    };

XFG_TRAMPOLINES(HWND)

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            (USER_MARSHAL_SIZING_ROUTINE)XFG_TRAMPOLINE_FPTR(HWND_UserSize)
            ,(USER_MARSHAL_MARSHALLING_ROUTINE)XFG_TRAMPOLINE_FPTR(HWND_UserMarshal)
            ,(USER_MARSHAL_UNMARSHALLING_ROUTINE)XFG_TRAMPOLINE_FPTR(HWND_UserUnmarshal)
            ,(USER_MARSHAL_FREEING_ROUTINE)XFG_TRAMPOLINE_FPTR(HWND_UserFree)
            
            }
            

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: ICtx, ver. 0.0,
   GUID={0x71372505,0xF383,0x4235,{0xB6,0x29,0xCA,0x22,0x5A,0x80,0x39,0x52}} */

#pragma code_seg(".orpc")
static const unsigned short ICtx_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: IAddPack, ver. 0.0,
   GUID={0x77AECA58,0x2B8B,0x4785,{0xBB,0xA0,0x28,0x89,0x5B,0x7B,0xA7,0x8A}} */

#pragma code_seg(".orpc")
static const unsigned short IAddPack_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: IUnpackTo, ver. 0.0,
   GUID={0x171571BA,0x93C0,0x463c,{0x9C,0xAF,0x39,0xD6,0xC2,0x88,0xE6,0xFC}} */

#pragma code_seg(".orpc")
static const unsigned short IUnpackTo_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: IPackTo, ver. 0.0,
   GUID={0x60DA54A5,0xDC0E,0x4e5e,{0xBE,0x6E,0x6D,0xC3,0xB8,0x69,0xE9,0x1A}} */

#pragma code_seg(".orpc")
static const unsigned short IPackTo_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: IJunctionTo, ver. 0.0,
   GUID={0x489CF064,0xF875,0x4de0,{0xAB,0x60,0x07,0xC5,0x78,0x28,0x10,0x93}} */

#pragma code_seg(".orpc")
static const unsigned short IJunctionTo_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: ICopyTo, ver. 0.0,
   GUID={0x214C449A,0xB4E7,0x47b1,{0x80,0x8C,0x00,0x10,0x3A,0xCA,0xAD,0x11}} */

#pragma code_seg(".orpc")
static const unsigned short ICopyTo_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: IThumbOutTo, ver. 0.0,
   GUID={0x5FD66BBD,0x8A60,0x4528,{0xB7,0x67,0x34,0xC9,0x43,0xFB,0x1C,0x54}} */

#pragma code_seg(".orpc")
static const unsigned short IThumbOutTo_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: IExifProp, ver. 0.0,
   GUID={0x10390866,0xB8B4,0x442b,{0xAC,0x14,0x7D,0xED,0x4F,0x5F,0xAA,0xB4}} */

#pragma code_seg(".orpc")
static const unsigned short IExifProp_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: IImageTools, ver. 0.0,
   GUID={0x2D3143EC,0x66B4,0x4cf4,{0xBF,0xE8,0xCB,0x68,0x4B,0xDB,0xA8,0xD3}} */

#pragma code_seg(".orpc")
static const unsigned short IImageTools_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: ILSDecrypt, ver. 0.0,
   GUID={0x034C8401,0x6DE8,0x4860,{0x9A,0xD3,0x64,0xEA,0x29,0x20,0x7A,0xD1}} */

#pragma code_seg(".orpc")
static const unsigned short ILSDecrypt_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



/* Object interface: IIconProp, ver. 0.0,
   GUID={0xC6E2709B,0x4D74,0x4858,{0xA7,0xA6,0xF7,0x26,0x41,0x17,0x5D,0xB1}} */

#pragma code_seg(".orpc")
static const unsigned short IIconProp_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };



#endif /* defined(_M_AMD64)*/



/* this ALWAYS GENERATED file contains the proxy stub code */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for lsshell.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#if defined(_M_AMD64)



extern const USER_MARSHAL_ROUTINE_QUADRUPLE NDR64_UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif


#include "ndr64types.h"
#include "pshpack8.h"
#ifdef __cplusplus
namespace {
#endif


typedef 
NDR64_FORMAT_CHAR
__midl_frag8_t;
extern const __midl_frag8_t __midl_frag8;

typedef 
struct 
{
    struct _NDR64_ENCAPSULATED_UNION frag1;
    struct _NDR64_UNION_ARM_SELECTOR frag2;
    struct _NDR64_UNION_ARM frag3;
    struct _NDR64_UNION_ARM frag4;
    NDR64_UINT32 frag5;
}
__midl_frag5_t;
extern const __midl_frag5_t __midl_frag5;

typedef 
struct _NDR64_POINTER_FORMAT
__midl_frag4_t;
extern const __midl_frag4_t __midl_frag4;

typedef 
struct _NDR64_USER_MARSHAL_FORMAT
__midl_frag3_t;
extern const __midl_frag3_t __midl_frag3;

typedef 
struct 
{
    struct _NDR64_PROC_FORMAT frag1;
    struct _NDR64_PARAM_FORMAT frag2;
    struct _NDR64_PARAM_FORMAT frag3;
}
__midl_frag2_t;
extern const __midl_frag2_t __midl_frag2;

typedef 
NDR64_FORMAT_UINT32
__midl_frag1_t;
extern const __midl_frag1_t __midl_frag1;

static const __midl_frag8_t __midl_frag8 =
0x5    /* FC64_INT32 */;

static const __midl_frag5_t __midl_frag5 =
{ 
/* RemotableHandle */
    { 
    /* RemotableHandle */
        0x50,    /* FC64_ENCAPSULATED_UNION */
        (NDR64_UINT8) 3 /* 0x3 */,
        (NDR64_UINT8) 0 /* 0x0 */,
        0x5,    /* FC64_INT32 */
        (NDR64_UINT32) 4 /* 0x4 */,
        (NDR64_UINT32) 8 /* 0x8 */,
        (NDR64_UINT32) 0 /* 0x0 */
    },
    { 
    /* struct _NDR64_UNION_ARM_SELECTOR */
        (NDR64_UINT8) 0 /* 0x0 */,
        (NDR64_UINT8) 3 /* 0x3 */,
        (NDR64_UINT16) 0 /* 0x0 */,
        (NDR64_UINT32) 2 /* 0x2 */
    },
    { 
    /* struct _NDR64_UNION_ARM */
        (NDR64_INT64) 1215587415 /* 0x48746457 */,
        &__midl_frag8,
        (NDR64_UINT32) 0 /* 0x0 */
    },
    { 
    /* struct _NDR64_UNION_ARM */
        (NDR64_INT64) 1383359575 /* 0x52746457 */,
        &__midl_frag8,
        (NDR64_UINT32) 0 /* 0x0 */
    },
    (NDR64_UINT32) 4294967295 /* 0xffffffff */
};

static const __midl_frag4_t __midl_frag4 =
{ 
/* *RemotableHandle */
    0x21,    /* FC64_UP */
    (NDR64_UINT8) 0 /* 0x0 */,
    (NDR64_UINT16) 0 /* 0x0 */,
    &__midl_frag5
};

static const __midl_frag3_t __midl_frag3 =
{ 
/* wireHWND */
    0xa2,    /* FC64_USER_MARSHAL */
    (NDR64_UINT8) 128 /* 0x80 */,
    (NDR64_UINT16) 0 /* 0x0 */,
    (NDR64_UINT16) 7 /* 0x7 */,
    (NDR64_UINT16) 8 /* 0x8 */,
    (NDR64_UINT32) 8 /* 0x8 */,
    (NDR64_UINT32) 0 /* 0x0 */,
    &__midl_frag4
};

static const __midl_frag2_t __midl_frag2 =
{ 
/* Setup */
    { 
    /* Setup */      /* procedure Setup */
        (NDR64_UINT32) 2883907 /* 0x2c0143 */,    /* auto handle */ /* IsIntrepreted, [object], ClientMustSize, HasReturn, ServerCorrelation */
        (NDR64_UINT32) 24 /* 0x18 */ ,  /* Stack size */
        (NDR64_UINT32) 0 /* 0x0 */,
        (NDR64_UINT32) 8 /* 0x8 */,
        (NDR64_UINT16) 0 /* 0x0 */,
        (NDR64_UINT16) 0 /* 0x0 */,
        (NDR64_UINT16) 2 /* 0x2 */,
        (NDR64_UINT16) 0 /* 0x0 */
    },
    { 
    /* hwnd */      /* parameter hwnd */
        &__midl_frag3,
        { 
        /* hwnd */
            1,
            1,
            0,
            1,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            (NDR64_UINT16) 0 /* 0x0 */,
            0
        },    /* MustSize, MustFree, [in], ByValue */
        (NDR64_UINT16) 0 /* 0x0 */,
        8 /* 0x8 */,   /* Stack offset */
    },
    { 
    /* HRESULT */      /* parameter HRESULT */
        &__midl_frag8,
        { 
        /* HRESULT */
            0,
            0,
            0,
            0,
            1,
            1,
            1,
            1,
            0,
            0,
            0,
            0,
            0,
            (NDR64_UINT16) 0 /* 0x0 */,
            0
        },    /* [out], IsReturn, Basetype, ByValue */
        (NDR64_UINT16) 0 /* 0x0 */,
        16 /* 0x10 */,   /* Stack offset */
    }
};

static const __midl_frag1_t __midl_frag1 =
(NDR64_UINT32) 0 /* 0x0 */;
#ifdef __cplusplus
}
#endif


#include "poppack.h"


XFG_TRAMPOLINES64(HWND)

static const USER_MARSHAL_ROUTINE_QUADRUPLE NDR64_UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            (USER_MARSHAL_SIZING_ROUTINE)XFG_TRAMPOLINE_FPTR(HWND_UserSize64)
            ,(USER_MARSHAL_MARSHALLING_ROUTINE)XFG_TRAMPOLINE_FPTR(HWND_UserMarshal64)
            ,(USER_MARSHAL_UNMARSHALLING_ROUTINE)XFG_TRAMPOLINE_FPTR(HWND_UserUnmarshal64)
            ,(USER_MARSHAL_FREEING_ROUTINE)XFG_TRAMPOLINE_FPTR(HWND_UserFree64)
            
            }
            

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: ICtx, ver. 0.0,
   GUID={0x71372505,0xF383,0x4235,{0xB6,0x29,0xCA,0x22,0x5A,0x80,0x39,0x52}} */

#pragma code_seg(".orpc")
static const FormatInfoRef ICtx_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    &__midl_frag2
    };


static const MIDL_SYNTAX_INFO ICtx_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &ICtx_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &ICtx_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO ICtx_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &ICtx_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)ICtx_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO ICtx_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &ICtx_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)ICtx_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(8) _ICtxProxyVtbl = 
{
    &ICtx_ProxyInfo,
    &IID_ICtx,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* ICtx::Setup */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION ICtx_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall3
};

CInterfaceStubVtbl _ICtxStubVtbl =
{
    &IID_ICtx,
    &ICtx_ServerInfo,
    8,
    &ICtx_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IAddPack, ver. 0.0,
   GUID={0x77AECA58,0x2B8B,0x4785,{0xBB,0xA0,0x28,0x89,0x5B,0x7B,0xA7,0x8A}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IAddPack_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO IAddPack_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &IAddPack_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IAddPack_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IAddPack_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &IAddPack_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IAddPack_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IAddPack_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &IAddPack_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IAddPack_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _IAddPackProxyVtbl = 
{
    0,
    &IID_IAddPack,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION IAddPack_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IAddPackStubVtbl =
{
    &IID_IAddPack,
    &IAddPack_ServerInfo,
    7,
    &IAddPack_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IUnpackTo, ver. 0.0,
   GUID={0x171571BA,0x93C0,0x463c,{0x9C,0xAF,0x39,0xD6,0xC2,0x88,0xE6,0xFC}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IUnpackTo_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO IUnpackTo_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &IUnpackTo_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IUnpackTo_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IUnpackTo_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &IUnpackTo_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IUnpackTo_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IUnpackTo_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &IUnpackTo_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IUnpackTo_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _IUnpackToProxyVtbl = 
{
    0,
    &IID_IUnpackTo,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION IUnpackTo_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IUnpackToStubVtbl =
{
    &IID_IUnpackTo,
    &IUnpackTo_ServerInfo,
    7,
    &IUnpackTo_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IPackTo, ver. 0.0,
   GUID={0x60DA54A5,0xDC0E,0x4e5e,{0xBE,0x6E,0x6D,0xC3,0xB8,0x69,0xE9,0x1A}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IPackTo_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO IPackTo_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &IPackTo_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IPackTo_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IPackTo_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &IPackTo_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IPackTo_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IPackTo_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &IPackTo_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IPackTo_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _IPackToProxyVtbl = 
{
    0,
    &IID_IPackTo,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION IPackTo_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IPackToStubVtbl =
{
    &IID_IPackTo,
    &IPackTo_ServerInfo,
    7,
    &IPackTo_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IJunctionTo, ver. 0.0,
   GUID={0x489CF064,0xF875,0x4de0,{0xAB,0x60,0x07,0xC5,0x78,0x28,0x10,0x93}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IJunctionTo_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO IJunctionTo_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &IJunctionTo_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IJunctionTo_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IJunctionTo_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &IJunctionTo_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IJunctionTo_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IJunctionTo_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &IJunctionTo_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IJunctionTo_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _IJunctionToProxyVtbl = 
{
    0,
    &IID_IJunctionTo,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION IJunctionTo_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IJunctionToStubVtbl =
{
    &IID_IJunctionTo,
    &IJunctionTo_ServerInfo,
    7,
    &IJunctionTo_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: ICopyTo, ver. 0.0,
   GUID={0x214C449A,0xB4E7,0x47b1,{0x80,0x8C,0x00,0x10,0x3A,0xCA,0xAD,0x11}} */

#pragma code_seg(".orpc")
static const FormatInfoRef ICopyTo_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO ICopyTo_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &ICopyTo_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &ICopyTo_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO ICopyTo_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &ICopyTo_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)ICopyTo_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO ICopyTo_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &ICopyTo_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)ICopyTo_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _ICopyToProxyVtbl = 
{
    0,
    &IID_ICopyTo,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION ICopyTo_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _ICopyToStubVtbl =
{
    &IID_ICopyTo,
    &ICopyTo_ServerInfo,
    7,
    &ICopyTo_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IThumbOutTo, ver. 0.0,
   GUID={0x5FD66BBD,0x8A60,0x4528,{0xB7,0x67,0x34,0xC9,0x43,0xFB,0x1C,0x54}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IThumbOutTo_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO IThumbOutTo_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &IThumbOutTo_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IThumbOutTo_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IThumbOutTo_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &IThumbOutTo_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IThumbOutTo_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IThumbOutTo_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &IThumbOutTo_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IThumbOutTo_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _IThumbOutToProxyVtbl = 
{
    0,
    &IID_IThumbOutTo,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION IThumbOutTo_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IThumbOutToStubVtbl =
{
    &IID_IThumbOutTo,
    &IThumbOutTo_ServerInfo,
    7,
    &IThumbOutTo_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IExifProp, ver. 0.0,
   GUID={0x10390866,0xB8B4,0x442b,{0xAC,0x14,0x7D,0xED,0x4F,0x5F,0xAA,0xB4}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IExifProp_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO IExifProp_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &IExifProp_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IExifProp_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IExifProp_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &IExifProp_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IExifProp_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IExifProp_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &IExifProp_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IExifProp_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _IExifPropProxyVtbl = 
{
    0,
    &IID_IExifProp,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION IExifProp_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IExifPropStubVtbl =
{
    &IID_IExifProp,
    &IExifProp_ServerInfo,
    7,
    &IExifProp_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IImageTools, ver. 0.0,
   GUID={0x2D3143EC,0x66B4,0x4cf4,{0xBF,0xE8,0xCB,0x68,0x4B,0xDB,0xA8,0xD3}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IImageTools_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO IImageTools_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &IImageTools_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IImageTools_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IImageTools_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &IImageTools_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IImageTools_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IImageTools_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &IImageTools_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IImageTools_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _IImageToolsProxyVtbl = 
{
    0,
    &IID_IImageTools,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION IImageTools_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IImageToolsStubVtbl =
{
    &IID_IImageTools,
    &IImageTools_ServerInfo,
    7,
    &IImageTools_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: ILSDecrypt, ver. 0.0,
   GUID={0x034C8401,0x6DE8,0x4860,{0x9A,0xD3,0x64,0xEA,0x29,0x20,0x7A,0xD1}} */

#pragma code_seg(".orpc")
static const FormatInfoRef ILSDecrypt_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO ILSDecrypt_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &ILSDecrypt_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &ILSDecrypt_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO ILSDecrypt_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &ILSDecrypt_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)ILSDecrypt_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO ILSDecrypt_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &ILSDecrypt_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)ILSDecrypt_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _ILSDecryptProxyVtbl = 
{
    0,
    &IID_ILSDecrypt,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION ILSDecrypt_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _ILSDecryptStubVtbl =
{
    &IID_ILSDecrypt,
    &ILSDecrypt_ServerInfo,
    7,
    &ILSDecrypt_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IIconProp, ver. 0.0,
   GUID={0xC6E2709B,0x4D74,0x4858,{0xA7,0xA6,0xF7,0x26,0x41,0x17,0x5D,0xB1}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IIconProp_Ndr64ProcTable[] =
    {
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    (FormatInfoRef)(LONG_PTR) -1,
    0
    };


static const MIDL_SYNTAX_INFO IIconProp_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    lsshell__MIDL_ProcFormatString.Format,
    &IIconProp_FormatStringOffsetTable[-3],
    lsshell__MIDL_TypeFormatString.Format,
    UserMarshalRoutines,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IIconProp_Ndr64ProcTable[-3],
    0,
    NDR64_UserMarshalRoutines,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IIconProp_ProxyInfo =
    {
    &Object_StubDesc,
    lsshell__MIDL_ProcFormatString.Format,
    &IIconProp_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IIconProp_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IIconProp_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    lsshell__MIDL_ProcFormatString.Format,
    (unsigned short *) &IIconProp_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IIconProp_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(7) _IIconPropProxyVtbl = 
{
    0,
    &IID_IIconProp,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


EXTERN_C DECLSPEC_SELECTANY const PRPC_STUB_FUNCTION IIconProp_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IIconPropStubVtbl =
{
    &IID_IIconProp,
    &IIconProp_ServerInfo,
    7,
    &IIconProp_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

#ifdef __cplusplus
namespace {
#endif
static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    lsshell__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x60001, /* Ndr library version */
    0,
    0x8010274, /* MIDL Version 8.1.628 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x2000001, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };
#ifdef __cplusplus
}
#endif

const CInterfaceProxyVtbl * const _lsshell_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ILSDecryptProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICtxProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IAddPackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IJunctionToProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IExifPropProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICopyToProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IIconPropProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPackToProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IUnpackToProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IThumbOutToProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IImageToolsProxyVtbl,
    0
};

const CInterfaceStubVtbl * const _lsshell_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ILSDecryptStubVtbl,
    ( CInterfaceStubVtbl *) &_ICtxStubVtbl,
    ( CInterfaceStubVtbl *) &_IAddPackStubVtbl,
    ( CInterfaceStubVtbl *) &_IJunctionToStubVtbl,
    ( CInterfaceStubVtbl *) &_IExifPropStubVtbl,
    ( CInterfaceStubVtbl *) &_ICopyToStubVtbl,
    ( CInterfaceStubVtbl *) &_IIconPropStubVtbl,
    ( CInterfaceStubVtbl *) &_IPackToStubVtbl,
    ( CInterfaceStubVtbl *) &_IUnpackToStubVtbl,
    ( CInterfaceStubVtbl *) &_IThumbOutToStubVtbl,
    ( CInterfaceStubVtbl *) &_IImageToolsStubVtbl,
    0
};

PCInterfaceName const _lsshell_InterfaceNamesList[] = 
{
    "ILSDecrypt",
    "ICtx",
    "IAddPack",
    "IJunctionTo",
    "IExifProp",
    "ICopyTo",
    "IIconProp",
    "IPackTo",
    "IUnpackTo",
    "IThumbOutTo",
    "IImageTools",
    0
};

const IID *  const _lsshell_BaseIIDList[] = 
{
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    0
};


#define _lsshell_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _lsshell, pIID, n)

int __stdcall _lsshell_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _lsshell, 11, 8 )
    IID_BS_LOOKUP_NEXT_TEST( _lsshell, 4 )
    IID_BS_LOOKUP_NEXT_TEST( _lsshell, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _lsshell, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _lsshell, 11, *pIndex )
    
}

EXTERN_C const ExtendedProxyFileInfo lsshell_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _lsshell_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _lsshell_StubVtblList,
    (const PCInterfaceName * ) & _lsshell_InterfaceNamesList,
    (const IID ** ) & _lsshell_BaseIIDList,
    & _lsshell_IID_Lookup, 
    11,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* defined(_M_AMD64)*/


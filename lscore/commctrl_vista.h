
#pragma once
#ifndef __COMMCTRL_VISTA_H__
#define __COMMCTRL_VISTA_H__

////////////////////////////////////////////////////////////////////////
// Windows VISTA ImageList

#define VISTA_ILD_ASYNC					0x00008000

#define VISTA_ILS_EMBOSS				0x00000010
#define VISTA_ILS_REFLECT				0x00000020
#define VISTA_ILS_INVERT				0x00000040
#define VISTA_ILS_EFFECTS				0x0000007F

#define VISTA_ILGT_NORMAL				0x00000000
#define VISTA_ILGT_ASYNC				0x00000001


WINCOMMCTRLAPI HRESULT WINAPI HIMAGELIST_QueryInterface(HIMAGELIST himl, REFIID riid, void** ppv);

////////////////////////////////////////////////////////////////////////
// Windows VISTA ListView

#define VISTA_LVGF_SUBTITLE           0x00000100  // pszSubtitle is valid
#define VISTA_LVGF_TASK               0x00000200  // pszTask is valid
#define VISTA_LVGF_DESCRIPTIONTOP     0x00000400  // pszDescriptionTop is valid
#define VISTA_LVGF_DESCRIPTIONBOTTOM  0x00000800  // pszDescriptionBottom is valid
#define VISTA_LVGF_TITLEIMAGE         0x00001000  // iTitleImage is valid
#define VISTA_LVGF_EXTENDEDIMAGE      0x00002000  // iExtendedImage is valid
#define VISTA_LVGF_ITEMS              0x00004000  // iFirstItem and cItems are valid
#define VISTA_LVGF_SUBSET             0x00008000  // pszSubsetTitle is valid

#define VISTA_LVGS_COLLAPSED      0x00000001
#define VISTA_LVGS_HIDDEN         0x00000002
#define VISTA_LVGS_NOHEADER       0x00000004
#define VISTA_LVGS_COLLAPSIBLE    0x00000008
#define VISTA_LVGS_FOCUSED        0x00000010
#define VISTA_LVGS_SELECTED       0x00000020
#define VISTA_LVGS_SUBSETED       0x00000040

#pragma pack(1)

typedef struct tagLVGROUP_VISTA {
    UINT    cbSize;
    UINT    mask;
    LPWSTR  pszHeader;
    int     cchHeader;

    LPWSTR  pszFooter;
    int     cchFooter;

    int     iGroupId;

    UINT    stateMask;
    UINT    state;
    UINT    uAlign;
    LPWSTR  pszSubtitle;
    UINT    cchSubtitle;
    LPWSTR  pszTask;
    UINT    cchTask;
    LPWSTR  pszDescriptionTop;
    UINT    cchDescriptionTop;
    LPWSTR  pszDescriptionBottom;
    UINT    cchDescriptionBottom;
    int     iTitleImage;
    int     iExtendedImage;
    int     iFirstItem;         // Read only
    UINT    cItems;             // Read only in non-owner data mode
    LPWSTR  pszSubsetTitle;     // NULL if group is not subset
    UINT    cchSubsetTitle;
} LVGROUP_VISTA, *PLVGROUP_VISTA;

#pragma pack()


////////////////////////////////////////////////////////////////////////
// Windows VISTA TreeView

#define VISTA_TVM_GETSELECTEDCOUNT       (TV_FIRST + 57)

#define VISTA_TVS_EX_MULTISELECT          0x0002
#define VISTA_TVS_EX_DOUBLEBUFFER         0x0004
#define VISTA_TVS_EX_NOINDENTSTATE        0x0008
#define VISTA_TVS_EX_RICHTOOLTIP          0x0010

#define VISTA_TVGN_NEXTSELECTED       0x000B

#define VISTA_TVIF_STATEEX            0x0100
#define VISTA_TVIF_EXPANDEDIMAGE      0x0200

#define VISTA_TVIS_EX_FLAT            0x0001
#define VISTA_TVIS_EX_HWND            0x0002
#define VISTA_TVIS_EX_ALL             0x0002

#pragma pack(1)

typedef struct tagTVITEMEX_VISTA {
    UINT      mask;
    HTREEITEM hItem;
    UINT      state;
    UINT      stateMask;
    LPWSTR    pszText;
    int       cchTextMax;
    int       iImage;
    int       iSelectedImage;
    int       cChildren;
    LPARAM    lParam;
    int       iIntegral;
    UINT      uStateEx;
    HWND      hwnd;
    int       iExpandedImage;
} TVITEMEX_VISTA, *LPTVITEMEX_VISTA;

typedef struct tagTVDISPINFOEX_VISTA {
    NMHDR hdr;
    TVITEMEX_VISTA item;
} NMTVDISPINFOEX_VISTA, *LPNMTVDISPINFOEX_VISTA;

#pragma pack()


#endif		// __COMMCTRL_VISTA_H__

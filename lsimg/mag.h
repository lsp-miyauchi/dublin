/*
     MAG header

     1994 by Ken SAKUMA
*/

#pragma pack(1)
struct MAG_HEADER {
    BYTE top;
    BYTE machine;
    BYTE machine_depend;
    BYTE screen_mode;
    WORD ltx;
    WORD lty;
    WORD rbx;
    WORD rby;
    DWORD fa_ofs;
    DWORD fb_ofs;
    DWORD fb_size;
    DWORD pxl_ofs;
    DWORD pxl_size;
};
#pragma pack()

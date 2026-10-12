////////////////////////////////////////////////////////////////
// lspmask.h

#ifndef __LSPMASK_H__
#define __LSPMASK_H__


#define Q0_NEGATIVE			0x00000001
#define Q0_GLASSVERT		0x00000002
#define Q0_GLASSHORZ		0x00000004
#define Q0_XOR				0x00000008
#define Q0_MIRROR			0x00000010
#define Q0_FLIP				0x00000020
#define Q0_RGB				0x00000040

#define GLASS_VERT			0x00000001
#define GLASS_HORZ			0x00000002
#define GLASS_BOTH			(GLASS_VERT | GLASS_HORZ)


#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

HDIB WINAPI mask_meko(HDIB hSrc, LPRECT prc, BOOL bPlus);
HDIB WINAPI mask_q0(HDIB hSrc, LPRECT prc, DWORD stat);
HDIB WINAPI mask_cp(HDIB hSrc, LPRECT prc, LPCSTR pszCode);
HDIB WINAPI mask_cp_ex(HDIB hSrc, LPRECT prc, int nCount, LPCSTR* pszCode);
HDIB WINAPI mask_fl(HDIB hSrc, LPRECT prc, BOOL bInvert);
HDIB WINAPI mask_win(HDIB hSrc, LPRECT prc);
HDIB WINAPI mask_win_horz(HDIB hSrc, LPRECT prc);
HDIB WINAPI mask_xor128(HDIB hSrc, LPRECT prc);
HDIB WINAPI mask_rgbloop(HDIB hSrc, LPRECT prc);
HDIB WINAPI mask_glass(HDIB hSrc, LPRECT prc, int nBlock, DWORD dwFlags);


#ifdef __cplusplus
}
#endif

#endif			// __LSPMASK_H__

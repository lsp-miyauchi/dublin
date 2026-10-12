/////////////////////////////////////////////////////////////////////////////
// copy.h

#pragma once
#ifndef __FOP_COPY_H__
#define __FOP_COPY_H__

#include "meter.h"

////////////////////////////////////////////////
//

class CReplaceFileInfo
{
public:
	CString m_src;
	CString m_dst;
	CWin32FindData m_srcWfd;
	CWin32FindData m_dstWfd;
	FOPACTION m_fopAction;
	bool m_bCopy:1;
	bool m_bYesToAll:1;
	bool m_bMultiDest:1;
	bool m_bUseRecycle:1;
	bool m_bAutoRename:1;
};

/////////////////////////////////////////////////////////////////////////////
//

class CZapInfo : public CCopyProgressDlg
{
private:
	HWND m_hOwner;
	FOPACTION m_nAction;
	bool m_bCopy:1;					// true:コピー, false:移動
	bool m_bYesToAll:1;
	bool m_bMultiDest:1;
	bool m_bUseRecycle:1;
	bool m_bSaveFolderHierarchy:1;	// true:階層を保持, false:同一階層上に
	bool m_bAutoRename:1;			// true:自動で名前付加

public:
	CZapInfo();
	~CZapInfo();

	DISCSTREAM m_diskstream;
	DISCSTREAM m_decryptcopy;

	// functions
	void Initialize(HWND hOwner, BOOL fCopy = TRUE, BOOL fSaveFolderHierarchy = TRUE, BOOL yes2all = FALSE, FOPACTION action = FAC_QUERY);
	inline bool isYesToAll() { return m_bYesToAll; }
	inline bool isCopy() { return m_bCopy; }
	inline bool isSaveFolderHierarchy() { return m_bSaveFolderHierarchy; }
	inline bool isMultiDest() { return m_bMultiDest; }
	inline bool isUseRecycle() { return m_bUseRecycle; }
	inline bool isAutoRename() { return m_bAutoRename; }
	inline FOPACTION GetAction() { return m_nAction; }
	inline void SetCopy(BOOL fCopy) { m_bCopy = (fCopy != FALSE); }
	inline void SetMultiDest(BOOL fMultiDest) { m_bMultiDest = (fMultiDest != FALSE); }
	inline HWND GetOwner() { return m_hOwner; }
	void Start();

	int CopyFolder(LPTSTR pSrc, LPTSTR pDst);
	int TreeCopy(LPTSTR pSrc, LPTSTR pDst);
	int FileMove(LPTSTR pSrc, LPTSTR pDst);
	int FileCopy(LPTSTR pSrc, LPTSTR pDst);
	BOOL GetReplaceAction(LPCTSTR src, LPCTSTR dst, CWin32FindData& srcWfd, CWin32FindData& dstWfd);
};



#endif		// __FOP_COPY_H__

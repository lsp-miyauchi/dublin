// BJView.h : interface of the CBeView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <imm.h>
#include "resource.h"
#include "msg.h"
#include "common.h"
#include "hexmanager.h"
#include "addressmarker.h"

/////////////////////////////////////////////////////////////////////////////
// custom messages

extern UINT nRM_PROCREAD;
extern UINT nRM_ENDTHREAD;
extern UINT nRM_DATAREQUEST;

/////////////////////////////////////////////////////////////////////////////
//

#define _ISKEYDOWN(a)	(GetKeyState((a)) < 0)
#define LTOP(a)			((a) & ~0xf)
#define LEND(a)			((a) | 0xf)

enum {
	VHT_NCCLIENT,
	VHT_HEADER,
	VHT_ADDR,
	VHT_BYTE,
	VHT_TEXT,
	VHT_CLIENT,
	VHT_SCROLLBAR
};

#pragma pack(1)
typedef struct tagCLIPBINDATA {
	DWORD datasize;
	BYTE data[1];
} CLIPBINDATA;
#pragma pack()



class CMainFrame;

class CBeView : public CWindowImpl<CBeView>
{
public:
	enum {
		m_cyLineMargin = 1,
	};

	CBeView();
	~CBeView();

	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg);
	void OnIdle();

	BEGIN_MSG_MAP(CBeView)
		ON_WM_CREATE()
		ON_WM_SIZE()
		MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
		ON_WM_PAINT()
		ON_WM_LBUTTONDOWN()
		ON_WM_VSCROLL()
		ON_WM_CHAR()
		ON_WM_KEYDOWN()
		ON_WM_HSCROLL()
		ON_WM_SETFOCUS()
		ON_WM_KILLFOCUS()
		ON_WM_DESTROY()
		ON_WM_SETCURSOR()
		ON_WM_CONTEXTMENU()
		ON_WM_ERASEBKGND()
		ON_WM_MOUSEWHEEL()
		ON_COMMAND(ID_EDIT_CUT, OnEditCut)
		ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
		ON_COMMAND(ID_EDIT_TEXTCOPY, OnEditTextCopy)
		ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
		ON_COMMAND(ID_EDIT_FIND, OnFind)
		ON_COMMAND(ID_FIND_PREV, OnFindPrev)
		ON_COMMAND(ID_FIND_NEXT, OnFindNext)
		ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
		ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectAll)
		ON_COMMAND(ID_EDIT_RANGESEL, OnRangeSel)
		ON_COMMAND(ID_EDIT_SELSAVENEW, OnEditCopyToNewFile)
		ON_COMMAND(ID_EDIT_BYNEWEDITOR, OnEditByNewEditor)
		ON_COMMAND(ID_EDIT_SELTO_PREVMARK, OnEditSelToPrevMark)
		ON_COMMAND(ID_EDIT_SELTO_NEXTMARK, OnEditSelToNextMark)
		ON_COMMAND(ID_JUMP_OFFSET, OnJumpOffset)
		ON_COMMAND(ID_JUMP_FILETOP, OnJumpFileTop)
		ON_COMMAND(ID_JUMP_FILEEND, OnJumpFileEnd)
		ON_COMMAND(ID_JUMP_OFFSET, OnJumpOffset)
		ON_COMMAND(ID_JUMP_LASTEDIT, OnJumpLastEdit)
		ON_COMMAND(ID_ENVIRON_SETUP, OnEnvironSetup)
		ON_COMMAND(ID_TEXT_ASCII, OnTextAscii)
		ON_COMMAND(ID_TEXT_SJIS, OnTextSjis)
		ON_COMMAND(ID_MARK_ADDRESS, OnMarkAddress)
		ON_COMMAND(ID_MARK_CLEARALL, OnMarkClearAll)
		ON_COMMAND(ID_MARK_PREV, OnMarkPrev)
		ON_COMMAND(ID_MARK_NEXT, OnMarkNext)
		ON_COMMAND(ID_TOOL_CHECKSUM, OnToolCheckSum)
		ON_MESSAGE(WM_SELECTPOINT, OnJump)
		ON_MESSAGE(WM_IME_COMPOSITION, OnImeComposition)
		ON_MESSAGE(WM_IME_STARTCOMPOSITION, OnImeStartComposition)
		ON_MESSAGE(WM_IME_ENDCOMPOSITION, OnImeEndComposition)
		ON_REGISTERD_MESSAGE(nRM_ENDTHREAD, OnNotifyEndThread)
	END_MSG_MAP()

protected:
	// メッセージハンドラ
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnPaint();
	void OnSize(UINT nType, int cx, int cy);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar);
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSetFocus(HWND hOldWnd);
	void OnKillFocus(HWND hNewWnd);
	void OnDestroy();
	BOOL OnSetCursor(HWND hWnd, UINT nHitTest, UINT message);
	void OnContextMenu(HWND hWnd, CPoint point);
	BOOL OnEraseBkgnd(HDC hdc);
	BOOL OnMouseWheel(UINT fFlags, short zDelta, CPoint point);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnImeComposition(WPARAM wParam, LPARAM lParam);
	LRESULT OnImeStartComposition(WPARAM wParam, LPARAM lParam);
	LRESULT OnImeEndComposition(WPARAM wParam, LPARAM lParam);
	LRESULT OnNotifyEndThread(WPARAM wParam, LPARAM param);

	// コマンドハンドラ
	void OnEditCut();
	void OnEditCopy();
	void OnEditTextCopy();
	void OnEditPaste();
	void OnEditSelectAll();
	void OnEditCopyToNewFile();
	void OnEditByNewEditor();
	void OnRangeSel();
	void OnFind();
	void OnFindPrev();
	void OnFindNext();
	void OnEditUndo();
	void OnEditSelToPrevMark();
	void OnEditSelToNextMark();
	void OnJumpOffset();
	void OnJumpFileTop();
	void OnJumpFileEnd();
	void OnJumpLastEdit();
	void OnEnvironSetup();
	void OnTextAscii();
	void OnTextSjis();
	void OnMarkAddress();
	void OnMarkClearAll();
	void OnMarkPrev();
	void OnMarkNext();
	void OnToolCheckSum();

	LRESULT OnJump(WPARAM wParam, LPARAM lParam);
	LRESULT OnRegisteredMouseWheel(WPARAM wParam, LPARAM lParam);

	// UIアップデータ
//	void OnUIFindPrevNext(CCmdUI* pCmdUI);
//	void OnUITextType(CCmdUI* pCmdUI);

	// メンバ関数
public:
	void OnUpdate(LPARAM lHint, void* pHint);
	BOOL OpenFile(LPCTSTR pszName);
	BOOL DoSave(LPCTSTR pszName);
	__inline CRange GetSelRange() { return m_hex.GetSelRange(); }
	__inline BOOL CanUndo() { return m_hex.CanUndo(); }
	__inline BOOL CanCopy() { return m_hex.CanCopy(); }
	__inline BOOL CanCut() { return m_hex.CanCut(); }
	BOOL CanPaste();
	__inline BOOL IsReadOnly() { return m_hex.IsReadOnly(); }
	__inline BOOL CanContinueFind() { return (lstrlen(m_csSearch) > 0 && m_iSearchLen > 0); }
	void JumpToPosition(UINT_PTR pos);
	UINT_PTR GetCurrentAddress(size_t* pnTotal);
	BOOL IsTextTypeAscii() { return (m_iTextType == CHAR_ASCII); }
	BOOL IsTextTypeSjis() { return (m_iTextType == CHAR_SJIS); }
	__inline HFONT GetTextFont() { return m_font.m_hFont; }
	__inline BOOL HasMark() { return (m_marker.GetCount() > 0); }
	void SendDataToWindow(HWND hwnd);
	__inline BOOL PasteData(void* pb, DWORD size) { return m_hex.LoadAppend(pb, size); }

protected:
	void DrawOneLine(CDC* pdc, int x, UINT_PTR posStart, BYTE* pb, int nLen, bool bNeedUnderline, bool bInvertAddressNumber);
	void CalcBase();
	void SetIMM();
	BOOL DoDragSelect();
	void ResizeCaret(BOOL bMove = FALSE);
	void MoveCaret();
	void DeleteCaret();
	void CreateWorkerBitmaps();
	int MapLineToScrollPos(UINT_PTR line);
	int MapScrollPosToLine(UINT_PTR pos);
	void RegRead();
	void RegWrite();
	void SetTopIndex(UINT_PTR nStartDraw);
	void SetupScrollBars();
	void ReallyGetClientRect(CRect& rc);

	void WaitThreadExit();
	static unsigned WINAPI _ThreadProcEntry(void* param);
	void ParallelReader();

	BOOL BeginDrag();
	void WriteIME();
	void StartIME();
	int HitTest(CPoint pt);
	void CopyToClipboard(LPVOID pv, size_t size);
	CLIPBINDATA* CopyFromClipboard();
	void DoPaste(const BYTE* pPaste, size_t size);

private:
	void OnToolTipFromMOUSEMOVE(CPoint point);
	void KeyInsert();
	void KeyTab();
	void KeyLeft(bool bShiftKey, bool bKeyRepeat);
	void KeyRight(bool bShiftKey, bool bKeyRepeat);
	void KeyUp(bool bShiftKey, bool bKeyRepeat);
	void KeyDown(bool bShiftKey, bool bKeyRepeat);
	void KeyHome();
	void KeyEnd();
	void KeyPageUp();
	void KeyPageDown();
	void KeyBack();
	void KeyDelete(bool bClip);
	BOOL XferChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	BOOL XferKey(UINT nChar, UINT nRepCnt, UINT nFlags);
	int DataHitTest(CPoint pt, UINT_PTR* ppos, int* pnType);
	int OffsetHitTest(CPoint pt, int* pxOfs, int* pyOfs);
	void JumpToOffset(INT_PTR pos);
	BOOL SelRangeToNewFile(LPCTSTR pszFileToSave);

public:
	CBitmap m_bmLine;
	HIMC m_hIMC;			// IMEコンテキストハンドル

	int m_xChar;
	int m_yChar;
	CFont m_font;
	BOOL m_bCaret;
	BOOL m_bCaptured;
	CBitmap m_bmCaret;

	// 検索情報
	CString m_csSearch;
	BYTE m_cbSearchBytes[MAX_PATH];
	int m_iSearchLen;
	BOOL m_bCase;
	BOOL m_bBinarySearch;
	int m_iFindType;
	BOOL m_bFinding;

protected:
	CBitmap m_bmHeader;
	UINT_PTR m_posTop;							// 画面描画開始位置
	int m_iTextType;
	int m_iLines;								// １画面の最大行数（少しでも見えているラインがあればそれも含む）
	size_t m_iRealLines;						// １画面の行数（行全体がすべて見えているライン数）
	size_t m_iPageBytes;						// １画面の最大バイト数（少しでも見えているデータも含む）
	size_t m_iRealPageBytes;					// １画面のバイト数（確実に全部見えているデータの総数）
	HANDLE m_hThread;
	HANDLE m_closing;
	BOOL m_bShow;
	int m_zDelta;
	int m_nWheelLines;
	int m_scroll_padline;						// ↑↓キーでスクロールする際の余裕ライン数
	CString m_strFileName;
	CSimpleArray<void*> m_AppendQue;			// ワーカースレッドで読んだデータをいったん格納しておくキュー
	CCriticalSection m_lock;
	CAddressMarker m_marker;					// アドレスマーカー位置保持用配列

public:
	CMainFrame* m_pFrame;
	CHexManager m_hex;
	VIEWCOLOR m_vc;
};

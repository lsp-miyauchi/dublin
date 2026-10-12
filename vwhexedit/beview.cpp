// BeView.cpp : implementation of the CBeView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "beview.h"
#include "mainfrm.h"
#include "strfinddialog.h"
#include "coloreditdialog.h"
#include "jump.h"
#include "selrange.h"
#include "checksum.h"
#include "drgdrp.h"

extern BOOL gbInsert;

#define SCROLLSIZE 45
#define TOTALY		20000

static char cbKeyTable[256];
UINT gCF_BINARY = RegisterClipboardFormat(_T("vwbindata"));
UINT nRM_PROCREAD = RegisterWindowMessage(_T("wm_vwhexedit_procread"));
UINT nRM_ENDTHREAD = RegisterWindowMessage(_T("wm_vwhexedit_endthread"));
UINT nRM_DATAREQUEST = RegisterWindowMessage(_T("wm_vwhexedit_datarequest"));


#define SCROLLSIZE			45
#define TOTALY				20000
#define OFS_BYTE			9
#define OFS_TEXT			58
#define OFS_ALL				74

static int hex2bytes(LPCTSTR pszText, LPBYTE pByte)
{
	int i = 0;
	while (*pszText) {
		if (cbKeyTable[*pszText] >= 0) {
			if (i & 1)
				*pByte++ |= cbKeyTable[*pszText];
			else
				*pByte = cbKeyTable[*pszText] << 4;
			if (++i >= (MAX_PATH >> 1))
				break;
		}
		pszText++;
	}

	if (i & 1)
		i++;
	return (i >> 1);
}

static void ZapMessages(HWND hwnd)
{
	MSG msg;
	while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	LONG lIdleCount = 0;
//@	CWinApp* pApp = AfxGetApp();
//@	while (pApp->OnIdle(lIdleCount++))
		;
}

CBeView::CBeView()
	: m_iLines(0)
	, m_iRealLines(0)
	, m_iPageBytes(0)
	, m_iRealPageBytes(0)
	, m_hIMC(NULL)
	, m_bBinarySearch(TRUE)
	, m_iSearchLen(0)
	, m_bCase(FALSE)
	, m_xChar(0)
	, m_yChar(0)
	, m_bFinding(FALSE)
	, m_bCaret(FALSE)
	, m_bCaptured(FALSE)
	, m_iTextType(CHAR_ASCII)
	, m_scroll_padline(2)
	, m_closing(NULL)
	, m_hThread (NULL)
	, m_zDelta(0)
{
	m_vc.rgbAddr     = RGB( 51, 102, 255);				// cyan
	m_vc.rgbText     = RGB(  0,   0,   0);				// black
	m_vc.rgbBack     = RGB(255, 255, 255);				// white
	m_vc.rgbLine     = RGB( 51, 102, 255);				// cyan
	m_vc.rgbMarkText = RGB(255, 255, 255);				// white
	m_vc.rgbMarkBack = RGB(255,   0,   0);				// red

	for (int i = 0; i < 256; i++) {
		if ('0' <= i && i <= '9')		cbKeyTable[i] = (char)(i - '0');
		else if ('A' <= i && i <= 'F')	cbKeyTable[i] = (char)(i - 'A' + 10);
		else if ('a' <= i && i <= 'f')	cbKeyTable[i] = (char)(i - 'a' + 10);
		else if (i == ' ')				cbKeyTable[i] = (char)-1;
		else							cbKeyTable[i] = (char)-2;
	}

	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &m_nWheelLines, 0);

	RegRead();
}

CBeView::~CBeView()
{
	while (m_AppendQue.GetSize() > 0) {
		void* dataptr = m_AppendQue[0];
		if (dataptr) FREE(dataptr);
		m_AppendQue.RemoveAt(0);
	}

	RegWrite();
}

BOOL CBeView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

void CBeView::OnIdle()
{
	// ワーカースレッドで並行読み込みしているときは、この呼び出しで読み込んだデータが更新される
	// キューから取り出し
	BYTE* dataptr = NULL;
	m_lock.Enter();
	int numLeft = m_AppendQue.GetSize();
	if (numLeft > 0) {
		dataptr = (BYTE*)m_AppendQue[0];
		m_AppendQue.RemoveAt(0);
		--numLeft;
	}
	m_lock.Leave();

	if (dataptr) {
		ULONG size = *(ULONG*)dataptr;
		m_hex.LoadAppend(dataptr + sizeof(ULONG), size);
		FREE(dataptr);
		// 進捗を更新
		m_pFrame->PostMessage(nRM_PROCREAD, 1, (LPARAM)size);		// メッセージをポストすることで、ここが継続して呼び出される
		if (numLeft == 0) {
			// インジケータ終了
			m_pFrame->PostMessage(nRM_PROCREAD, 2, 0);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CBeView クラスの描画

static bool DrawClientEdge(HWND hWnd, HRGN hRgnUpdate, HBRUSH hBrush)
{
	CWindowDC dc(hWnd);
	if (dc.IsNull())
		return false;

	// Get border size
	int cxBorder = GetSystemMetrics(SM_CXBORDER);
	int cyBorder = GetSystemMetrics(SM_CYBORDER);

	RECT rect;
	::GetWindowRect(hWnd, &rect);

	// Remove the client edge from the update region
	int cxEdge = GetSystemMetrics(SM_CXEDGE);
	int cyEdge = GetSystemMetrics(SM_CYEDGE);
	::InflateRect(&rect, -cxEdge, -cyEdge);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rect);
	if (rgn.IsNull())
		return false;

	if (hRgnUpdate != NULL)
		rgn.CombineRgn(hRgnUpdate, rgn, RGN_AND);

	::OffsetRect(&rect, -rect.left, -rect.top);

	::OffsetRect(&rect, cxEdge, cyEdge);
	dc.ExcludeClipRect(&rect);
	::InflateRect(&rect, cxEdge, cyEdge);

	if (hBrush == NULL) {
// need conditional code because types don't match in winuser.h
#ifdef _WIN64
		hBrush = (HBRUSH)::GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);
#else
		hBrush = (HBRUSH)UlongToPtr(::GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND));
#endif
	}
	dc.FillRect(&rect, hBrush);

	// Use background brush too, since theme border might not cover everything
	if (cxBorder < cxEdge && cyBorder < cyEdge) {
		::InflateRect(&rect, cxBorder - cxEdge, cyBorder - cyEdge);
#ifdef _WIN64
		hBrush = (HBRUSH)::GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);
#else
		hBrush = (HBRUSH)UlongToPtr(::GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND));
#endif
		dc.FillRect(&rect, hBrush);
	}

	::DefWindowProc(hWnd, WM_NCPAINT, (WPARAM)rgn.m_hRgn, 0L);
	return true;
}

LRESULT CBeView::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lRet = 0;
	bHandled = FALSE;

	HRGN hRgn = (wParam != 1) ? (HRGN)wParam : NULL;

	// WS_BORDER または WS_EX_CLIENTEDGE スタイルを独自に描く
	if (DrawClientEdge(m_hWnd, hRgn, GetSysColorBrush(COLOR_3DSHADOW)))
		bHandled = TRUE;
	return lRet;
}

void CBeView::OnPaint()
{
	CPaintDC dc(m_hWnd);

	// スクロールバーの位置を取得
	int x = GetScrollPos(SB_HORZ);

	// デバイスコンテキストを作り、ビットマップを割当てる
	CDC dcw;
	dcw.CreateCompatibleDC(dc);

	CRect rc;
	GetClientRect(&rc);

	CBitmapHandle bmOld;
	CRect rcClip(0, 0, rc.Width(), m_yChar);
/* @@ ---> MoveCaret() で描く
	if (dc.RectVisible(&rcClip) && m_bmHeader.m_hBitmap) {
		// カラムヘッダを描画
		bmOld = dcw.SelectBitmap(m_bmHeader);
		dc.SetTextColor(m_vc.rgbAddr);
		dc.SetBkColor(m_vc.rgbBack);
		dc.BitBlt(-x, 0, rc.Width() + x, m_yChar, dcw, 0, 0, SRCCOPY);
		dcw.SelectBitmap(bmOld);
	}
@@ */

	if (!m_bmLine.m_hBitmap) {
		rc.top += m_yChar;
		dc.FillSolidRect(&rc, m_vc.rgbBack);
		return;
	}

	// データを読む
	LPBYTE pData = (LPBYTE)valloc(m_iPageBytes + 1);
	ATLASSERT(pData != NULL);
	if (!pData) return;

	// データ取得
	size_t numCount;
	m_hex.Read(m_posTop, m_iPageBytes + 1, pData, &numCount);
	numCount--;

	// 描画開始ポイントと終了ポイントを取得
	UINT_PTR posStart = m_posTop;
	UINT_PTR posEnd = min(posStart + m_iPageBytes - 1, m_hex.GetEndAddr());

	// メモリDCに描く
	CFontHandle fontOld = dcw.SelectFont(m_font);
	bmOld = dcw.SelectBitmap(m_bmLine);

	// 選択状態の位置を取得
	CRange sel = m_hex.GetSelRange();
	sel.Normalize();

	// テキストのサイズを取得
	int cy = m_yChar;
	int nx = OFS_BYTE * m_xChar;

	int yDst = m_yChar;
	int i;
	for (i = 0; i < m_iLines; i++) {
		if (posStart > posEnd)
			break;

		rcClip.OffsetRect(0, m_yChar);
		if (!dc.RectVisible(&rcClip)) {
			yDst += cy;
			posStart += 16;
			if (numCount <= 16)
				break;
			numCount -= 16;
			continue;
		}

		// 選択範囲を特定
		int bs = INT_MAX, be = -1;
		bool bSelect = false;
		for (int j = 0; j < 16; j++) {
			if (m_hex.IsSelected() && sel.InRange(posStart + j)) {
				bs = min(bs, j);
				be = max(be, j);
				bSelect = true;
			}
		}

		bool bCurLine = (m_hex.GetCurLineAddr() == posStart);			// 現在行
		bool bNeedUnderline = (bCurLine && !m_bCaptured && !bSelect);	// 下線描画
		bool bInvertAddressNumber = (bSelect || bCurLine);				// 行番号部分の反転
		dcw.FillSolidRect(0, 0, rc.Width(), rc.Height(), m_vc.rgbBack);
		DrawOneLine(&dcw, -x, posStart, &pData[i << 4], min(16, (int)numCount), bNeedUnderline, bInvertAddressNumber);

		if (bSelect) {
			CRange rg;
			rg.from = -x + (OFS_BYTE + bs * 3) * m_xChar;
			rg.to = -x + (OFS_BYTE + be * 3 + 2) * m_xChar;
			dcw.PatBlt((int)rg.from, 0, (int)rg.Range(), cy, DSTINVERT);

			rg.from = -x + (OFS_TEXT + bs) * m_xChar;
			rg.to = -x + (OFS_TEXT + be + 1) * m_xChar;
			dcw.PatBlt((int)rg.from, 0, (int)rg.Range(), cy, DSTINVERT);
		}

		dc.BitBlt(0, yDst, rc.Width(), cy, dcw, 0, 0, SRCCOPY);

		yDst += cy;
		posStart += 16;
		if (numCount <= 16)
			break;
		numCount -= 16;
	}
	vfree(pData);

	if (i < m_iLines) {
		rc.top = yDst;
		dc.FillSolidRect(&rc, m_vc.rgbBack);
	}

	// キャレット表示
	MoveCaret();
	dcw.SelectBitmap(bmOld);
	dcw.SelectFont(fontOld);
}

void CBeView::DrawOneLine(CDC* pdc, int x, UINT_PTR posStart, BYTE* pb, int nLen, bool bNeedUnderline, bool bInvertAddressNumber)
{
	// 番号描画
	TCHAR szb[16], szt[16];
	wsprintf(szb, _T("%07X"), posStart);

	// 行番号部分の反転
	if (bInvertAddressNumber) {
		pdc->SetTextColor(m_vc.rgbBack);
		pdc->SetBkColor(m_vc.rgbLine);
	}
	else {
		pdc->SetTextColor(m_vc.rgbAddr);
		pdc->SetBkColor(m_vc.rgbBack);
	}
	pdc->TextOut(x, 0, szb, 7);

	pdc->SetTextColor(m_vc.rgbText);
	pdc->SetBkColor(m_vc.rgbBack);

	CRange range(0, 0);

	bool bDBCS = false, bWrite = true;
	register int bx = OFS_BYTE * m_xChar + x;
	register int tx = OFS_TEXT * m_xChar + x;
	int cy = m_yChar - m_cyLineMargin;
	for (int i = 0; i < nLen; i++) {
		// マーク位置のチェック
		if (range.IsEmpty()) {
			range = m_marker.Find(posStart + i);
			if (!range.IsEmpty()) {
				pdc->SetTextColor(m_vc.rgbMarkText);
				pdc->SetBkColor(m_vc.rgbMarkBack);

				int dx = 0;
				INT_PTR z = posStart + i;
				INT_PTR end = min(range.to, posStart + nLen - 1);
				while (z <= end) {
					dx += m_xChar * 3;
					z++;
				}
				dx -= m_xChar;

				CRect rcm(bx, 0, bx + dx, m_yChar);
				pdc->FillSolidRect(&rcm, m_vc.rgbMarkBack);
			}
		}
		else {
			if (!range.InRange(posStart + i)) {
				range.SetRange(0, 0);
				pdc->SetTextColor(m_vc.rgbText);
				pdc->SetBkColor(m_vc.rgbBack);
			}
		}

		wsprintf(szb, _T("%02X "), *pb);
		if (IsDBCSLeadByte(*pb) && (i + 1) < nLen && !bDBCS && m_iTextType == CHAR_SJIS) {
			// シフトJIS
			MultiByteToWideChar(CP_ACP, 0, (char*)pb, 2, szt, ARRAYSIZE(szt));
			bDBCS = true;
			bWrite = true;
		}
		else {
			if (bDBCS)
				bWrite = false;
			else {
				if ((m_iTextType == CHAR_SJIS) && (0xa1 <= *pb && *pb <= 0xdf)) {
					// シフトJIS + 半角カナ
					MultiByteToWideChar(CP_ACP, 0, (char*)pb, 1, szt, ARRAYSIZE(szt));
					bWrite = true;
				}
				else {
					bWrite = true;
					szt[0] = isprint(*pb) ? *pb : '.';
				}
			}
			bDBCS = false;
		}

		pdc->TextOut(bx, 0, szb, 2);

		if (bWrite)
			pdc->TextOut(tx, 0, szt, 1);

		if (!m_hex.IsSelected() && posStart + i == m_hex.GetCurAddr()) {
			if (m_hex.m_posType != SST_TEXT)
				pdc->PatBlt(tx, 0, m_xChar, cy, DSTINVERT);
			else
				pdc->PatBlt(bx, 0, m_xChar * 2, cy, DSTINVERT);
		}
		pb++;
		bx += m_xChar * 3;
		tx += m_xChar;
	}

	if (bNeedUnderline)
		pdc->FillSolidRect(x, cy - 1, OFS_ALL * m_xChar, 1, m_vc.rgbLine);
}

void CBeView::SetTopIndex(UINT_PTR posStart)
{
	// 描画開始位置
	if (m_hex.GetTotalSize())
		m_posTop = ROWHEAD(posStart);
}

void CBeView::CalcBase()
{
	CClientDC dc(m_hWnd);
	CFontHandle fontOld = dc.SelectFont(m_font);
	CSize csz;
	dc.GetTextExtent(_T("0123456789ABCDEF"), 16, &csz);
	m_xChar = (csz.cx >> 4);
	m_yChar = csz.cy + m_cyLineMargin;
	dc.SelectFont(fontOld);

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;

	// 行数の計算
	m_iLines = rc.Height() / m_yChar;
	m_iRealLines = m_iLines;
	if (rc.Height() % m_yChar) m_iLines++;

	// 行数から表示しているデータ数を計算
	m_iPageBytes = m_iLines << 4;
	m_iRealPageBytes = m_iRealLines << 4;

	UINT_PTR pd = m_posTop;
	while (m_hex.GetCurAddr() >= pd + m_iRealPageBytes)
		pd += 16;
	SetTopIndex(pd);
}

int CBeView::MapLineToScrollPos(UINT_PTR line)
{
	int total_lines = (int)m_hex.GetTotalLines();
	return (total_lines < 32768) ? line : MulDiv(line, 32768, total_lines);
}

int CBeView::MapScrollPosToLine(UINT_PTR pos)
{
	int total_lines = (int)m_hex.GetTotalLines();
	return (total_lines < 32768) ? pos : MulDiv(pos, total_lines, 32768);
}

void CBeView::ReallyGetClientRect(CRect& rc)
{
	GetClientRect(rc);

	DWORD dwStyle = GetStyle();
	if (dwStyle & WS_HSCROLL)
		rc.bottom += (GetSystemMetrics(SM_CYHSCROLL) - 1);
	if (dwStyle & WS_VSCROLL)
		rc.right  += (GetSystemMetrics(SM_CXVSCROLL) - 1);
}

void CBeView::SetupScrollBars()
{
	CRect rc;
	ReallyGetClientRect(rc);
	int cxWindow = rc.Width();
	int cyWindow = rc.Height();

	SCROLLINFO	sInfo;
	sInfo.cbSize = sizeof(SCROLLINFO);
	sInfo.fMask = SIF_PAGE | SIF_RANGE;
	sInfo.nPage = MapLineToScrollPos(m_iRealPageBytes >> 4);
	sInfo.nMin = 0;
	sInfo.nMax = MapLineToScrollPos(m_hex.GetTotalLines());
	SetScrollInfo(SB_VERT, &sInfo, TRUE);

	sInfo.nPage = cxWindow;
	sInfo.nMax = OFS_ALL * m_xChar;
	SetScrollInfo(SB_HORZ, &sInfo, TRUE);
}

void CBeView::OnUpdate(LPARAM lHint, void* pHint)
{
	switch(lHint) {
	case UVM_NEWFILE:
		SetScrollPos(SB_VERT, 0, FALSE);
		SetScrollPos(SB_HORZ, 0, FALSE);

		CalcBase();						// テキストのサイズを取得
		SetIMM();						// IMMにフォントを設定

		m_hex.MoveTo(0, SST_HIBYTE);
		SetTopIndex(0);					// 描画するテキストの配置をリセット
		ResizeCaret(TRUE);				// キャレット表示

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case UVM_RESETPARAMS:
		CalcBase();						// テキストのサイズを取得
		CreateWorkerBitmaps();
		SetIMM();						// IMMにフォントを設定
		ResizeCaret();					// キャレット表示

		SetupScrollBars();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case UVM_LOADFILE:
		break;

	default:
		break;
	}
}

BOOL CBeView::DoSave(LPCTSTR pszName)
{
	DWORD attr = GetFileAttributes(pszName);
	if (attr != 0xFFFFFFFF && (attr & FILE_ATTRIBUTE_READONLY)) {
		LPCTSTR pN = PathFindFileName(pszName);
		AppMessage(IDS_READONLYFILE, pN, MB_OK | MB_ICONSTOP);
	}
	return m_hex.Save(pszName);
}

/* @@@
void CBeView::OnInitialUpdate()
{
	OnUpdate(NULL, UVM_NEWFILE, NULL);

	m_hex.SetReadOnly(gbReadOnly);
	SetupScrollBars();

	return;
}
@@@ */

void CBeView::WaitThreadExit()
{
	if (m_hThread) {
		DWORD res;
		GetExitCodeThread(m_hThread, &res);
		if (res == STILL_ACTIVE) {
			m_closing = CreateEvent(NULL, FALSE, FALSE, NULL);
			while (WaitForSingleObject(m_closing, 0) == WAIT_TIMEOUT) {
				MSG msg;
				if (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
			CloseHandle(m_closing);
			m_closing = NULL;
		}
		// スレッド終了を待つ
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void CBeView::ParallelReader()
{
	// ファイルをオープン
	size_t block = 0x20000;		// 256KB
	BYTE* buffer = (BYTE*)VirtualAlloc(NULL, block, MEM_COMMIT, PAGE_READWRITE);
	if (buffer) {
		HANDLE hf = CreateFile(m_strFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,  FILE_FLAG_OVERLAPPED, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			// ファイルのサイズを取得
			LARGE_INTEGER fsize;
			GetFileSizeEx(hf, &fsize);

			OVERLAPPED ovl;
			memset(&ovl, 0, sizeof(ovl));
			ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (ovl.hEvent) {

				LARGE_INTEGER done;
				done.QuadPart = 0;
				while (fsize.QuadPart > done.QuadPart) {
					if (m_closing)
						break;

					DWORD numRead = (done.QuadPart + block < fsize.QuadPart) ? block : (DWORD)(fsize.QuadPart - done.QuadPart);
					ovl.Offset = done.LowPart;
					ovl.OffsetHigh = done.HighPart;
					BOOL bOK = ReadFileEx(hf, buffer, numRead, &ovl, NULL);
					if (!bOK)
						break;

					// 読み込み完了まで待つ
					WaitForSingleObjectEx(hf, INFINITE, FALSE);

					// キューに格納
					m_lock.Enter();
					// 最初の4バイトにサイズを格納し、先頭から5バイト目以降にデータが入る
					BYTE* pb = (BYTE*)ALLOC(numRead + sizeof(ULONG));
					if (pb) {
						*(ULONG*)pb = (ULONG)numRead;
						memcpy(pb + sizeof(ULONG), buffer, numRead);
						m_AppendQue.Add((void*)pb);
					}
					m_lock.Leave();
					if (!pb)
						break;

					done.QuadPart += numRead;
					SetupScrollBars();
					SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(m_posTop)), TRUE);
				}
			}
			CloseHandle(hf);
		}
		VirtualFree(buffer, 0, MEM_RELEASE);
	}

	if (m_closing)
		SetEvent(m_closing);

	// スレッド終了を通知
	PostMessage(nRM_ENDTHREAD, 0, 0);
}

unsigned WINAPI CBeView::_ThreadProcEntry(void* param)
{
	((CBeView*)param)->ParallelReader();
	_endthreadex(0);
	return 0;
}

LRESULT CBeView::OnNotifyEndThread(WPARAM wParam, LPARAM param)
{
	if (m_hThread) {
		// スレッド終了を待つ
		DWORD code;
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	return 0;
}

BOOL CBeView::OpenFile(LPCTSTR pszName)
{
	m_strFileName.Empty();
	if (pszName && *pszName && PathFileExists(pszName)) {
		HANDLE hf = CreateFile(pszName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hf == INVALID_HANDLE_VALUE) {
			LastError(PathFindFileName(pszName), MB_OK | MB_ICONSTOP);
			return FALSE;
		}

		// ファイルのサイズを取得
		DWORD dw, size = GetFileSize(hf, &dw);
		CloseHandle(hf);
		m_strFileName = pszName;

		if (PathIsUNC(pszName) || size >= WARNINGSIZE) {
			// ネットワーク先のファイルか、制限サイズ以上のファイルの場合
			m_hex.ClearData();

			// ステータスバーの読み込みインジケータ初期化
			m_pFrame->SendMessage(nRM_PROCREAD, 0, size);

			// ワーカースレッドで並行して読む
			unsigned d;
			m_hThread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &d);
			SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
			ResumeThread(m_hThread);
		}
		else {
			m_hex.Load(pszName);
			SetupScrollBars();
		}
		SetTopIndex(0);					// 描画するテキストの配置をリセット
		ResizeCaret(TRUE);				// キャレット表示
		Invalidate();
	} else {
		m_hex.ClearData();
		SetupScrollBars();
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBeView クラスのメッセージ ハンドラ

int CBeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	SetWindowLongPtr(GWL_STYLE, GetStyle() | (WS_VSCROLL | WS_HSCROLL));

	CalcBase();
	m_hIMC = ImmGetContext(m_hWnd);
	SetIMM();

	OnUpdate(UVM_NEWFILE, NULL);
	return 0;
}

void CBeView::OnDestroy() 
{
	WaitThreadExit();
}

BOOL CBeView::OnSetCursor(HWND hWnd, UINT nHitTest, UINT message) 
{
	HCURSOR hcur = NULL;
	if (m_bFinding)
		hcur = ::LoadCursor(NULL, IDC_WAIT);
	else {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		int nHit = HitTest(pt);
		if (nHit == VHT_ADDR)
			hcur = ::LoadCursor(RESINST, _MKIR(IDC_LINESEL));
		else if (nHit != VHT_NCCLIENT && nHit != VHT_HEADER)
			hcur = ::LoadCursor(NULL, IDC_IBEAM);
	}

	if (hcur) {
		SetCursor(hcur);
		return TRUE;
	}
	return (BOOL)DefWindowProc();
}

void CBeView::OnSetFocus(HWND hOldWnd) 
{
	ResizeCaret(TRUE);
}

void CBeView::OnKillFocus(HWND hNewWnd) 
{
	DeleteCaret();
	m_bCaptured = FALSE;			// 複数選択を解除
}

void CBeView::OnSize(UINT nType, int cx, int cy) 
{
	if (cx && cy && nType != SIZE_MINIMIZED) {
		// 行数の計算
		m_iLines = (cy - m_yChar) / m_yChar;
		m_iRealLines = m_iLines;
		if ((cy - m_yChar) % m_yChar)
			m_iLines++;

		// 行数から表示しているデータ数を計算
		m_iPageBytes = m_iLines << 4;
		m_iRealPageBytes = m_iRealLines << 4;

		CreateWorkerBitmaps();

		UINT_PTR pt = m_posTop;
		while (m_hex.GetCurAddr() >= pt + m_iRealPageBytes)
			pt += 16;
		SetTopIndex(pt);
	}
	SetupScrollBars();
}

int CBeView::DataHitTest(CPoint pt, UINT_PTR* ppos, int* pnType)
{
	CRect rc;
	GetClientRect(&rc);
	rc.OffsetRect(-GetScrollPos(SB_HORZ), 0);

	CRect rcw = rc;
	rcw.bottom = rcw.top + m_yChar;
	if (rcw.PtInRect(pt))
		return VHT_HEADER;

	rcw = rc;
	rcw.top += m_yChar;
	if (!rcw.PtInRect(pt))
		return VHT_NCCLIENT;

	UINT_PTR posEnd = m_hex.GetEndAddr();
	rcw.right = rcw.left + OFS_BYTE * m_xChar;
	if (rcw.PtInRect(pt)) {
		*ppos = m_posTop + (((pt.y - m_yChar) / m_yChar) << 4);
		*pnType = SST_HIBYTE;
		return (*ppos < posEnd) ? VHT_ADDR : VHT_CLIENT;
	}

	rcw.left = rcw.right - (m_xChar >> 1);
	rcw.right = rcw.left + (m_xChar * 3 * 16);
	if (rcw.PtInRect(pt)) {
		*ppos = m_posTop + (((pt.y - m_yChar) / m_yChar) << 4);
		*ppos += (pt.x - rcw.left) / (m_xChar * 3);
		*pnType = ((pt.x - rcw.left) % (m_xChar * 3) <= (m_xChar * 3) / 2) ?
						SST_HIBYTE : SST_LOBYTE;
		return (*ppos < posEnd) ? VHT_BYTE : VHT_CLIENT;
	}

	rcw = rc;
	rcw.left += OFS_TEXT * m_xChar;
	if (rcw.PtInRect(pt)) {
		*ppos = m_posTop + (((pt.y - m_yChar) / m_yChar) << 4);
		*ppos += (pt.x - rcw.left) / m_xChar;
		*pnType = SST_TEXT;
		return (*ppos < posEnd) ? VHT_TEXT : VHT_CLIENT;
	}

	return VHT_CLIENT;
}

int CBeView::OffsetHitTest(CPoint pt, int* pxOfs, int* pyOfs)
{
	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	rc.OffsetRect(-GetScrollPos(SB_HORZ), 0);

	*pyOfs = max(0, (pt.y - rc.top) / m_yChar);

	int ofs = rc.left + OFS_BYTE * m_xChar;
	if (pt.x < ofs) {
		*pxOfs = 0;
		return VHT_ADDR;
	}

	ofs = rc.left + OFS_TEXT * m_xChar;
	if (ofs <= pt.x) {
		*pxOfs = min(15, (pt.x - ofs) / m_xChar);
		return VHT_TEXT;
	}

	ofs = rc.left + OFS_BYTE * m_xChar - (m_xChar >> 1);
	*pxOfs = min(15, (pt.x - ofs) / (m_xChar * 3));
	return VHT_BYTE;
}

void CBeView::OnVScroll(UINT nSBCode, UINT nPos, HWND hBar) 
{
	UINT_PTR posNew = m_hex.GetCurAddr();
	UINT_PTR posEnd = m_hex.GetEndAddr();

	INT_PTR ofs = m_hex.GetCurAddr() - m_posTop;
	switch (nSBCode) {
	case SB_LINEDOWN:
		posNew = m_posTop + 16;
		if (posNew + m_iRealPageBytes > posEnd)
			return;
		break;

	case SB_LINEUP:
		if (!m_posTop) {
			if (ofs < 16)
				return;
			ofs -= 16;
			posNew = 0;
		}
		else
			posNew = m_posTop - 16;
		break;

	case SB_PAGEDOWN:
		posNew = m_posTop + m_iRealPageBytes;
		if (posNew > posEnd)
			return;
		break;

	case SB_PAGEUP:
		if (m_posTop < m_iRealPageBytes) {
			if (!m_posTop)
				return;
			posNew = 0;
		}
		else
			posNew = m_posTop - m_iRealPageBytes;
		break;

	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		// スクロールバーの位置からデータ位置を取得
		posNew = MapScrollPosToLine(nPos) << 4;
		if (posNew + ofs > posEnd)
			return;
		break;

	default:
		return;
	}

	if (posNew > posEnd) return;

	if (posNew + ofs > posEnd)
		ofs = posEnd - posNew;

	m_hex.MoveTo(posNew + ofs, m_hex.m_posType);
	ResizeCaret();				// キャレットのサイズを変更

	JumpToPosition(posNew);
	ZapMessages(NULL);
}

void CBeView::OnHScroll(UINT nSBCode, UINT nPos, HWND hBar) 
{
	int x, xNew, xMax;
	GetScrollRange(SB_HORZ, &x, &xMax);
	x = GetScrollPos(SB_HORZ);

	switch(nSBCode) {
	case SB_LINELEFT:
		if (x == 0) return;
		xNew = max(0, x - m_xChar);
		break;

	case SB_LINERIGHT:
		if (x == xMax) return;
		xNew = min(xMax, x + m_xChar);
		break;

	case SB_PAGELEFT:
		if (x == 0) return;
		xNew = max(0, x - m_xChar * 12);
		break;

	case SB_PAGERIGHT:
		if (x == xMax) return;
		xNew = min(xMax, x + m_xChar * 12);
		break;

	case SB_THUMBTRACK:
		if (x == (int)nPos) return;
		xNew = nPos;
		break;

	default:
		return;
	}

	SetScrollPos(SB_HORZ, xNew, TRUE);

	CRect rc;
	GetClientRect(&rc);
	ScrollWindow(x - xNew, 0, NULL, NULL);
	if (x < xNew)
		rc.right -= xNew - x;
	else
		rc.left += x - xNew;
	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CBeView::JumpToPosition(UINT_PTR pos)
{
	UINT_PTR posEnd = m_hex.GetEndAddr();
	if (pos > posEnd) {
		if (!ISROWHEAD(posEnd - 1))
			pos = ROWHEAD(posEnd - 1);
		else
			pos = posEnd - 15;
	}

	if (pos != m_posTop) {
		// スクロール位置を計算
		int yPos = MapLineToScrollPos(POS2ROW(pos));
		SetScrollPos(SB_VERT, yPos, TRUE);

		// 描画位置を修正
		SetTopIndex(pos);
	}
	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CBeView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!XferChar(nChar, nRepCnt, nFlags))
		DefWindowProc();
}

BOOL CBeView::XferChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Undo機能
	LPBYTE pUndoData;
	if (!m_hex.GetTotalSize() || nChar < VK_SPACE || m_bFinding ||
		m_bCaptured || m_hThread || m_hex.IsReadOnly())
		return FALSE;

	int val = cbKeyTable[nChar];
	UINT type = m_hex.m_posType;
	if (type != SST_TEXT && val < 0)
		return FALSE;

	CRange sel = m_hex.GetSelRange();
	sel.Normalize();

	// 選択中の範囲があれば、それをカット
	size_t size;
	BOOL bRedrawAll = FALSE;
	if (m_hex.IsSelected()) {
		pUndoData = m_hex.CutSelectedRange(&size);
		m_hex.SaveUndo(UNDO_DELETE, sel.from, pUndoData, size);

		type = (type == SST_TEXT) ? SST_TEXT : SST_HIBYTE;
		m_hex.MoveTo(sel.from, type);
		SetTopIndex(min(sel.from, m_posTop));
		bRedrawAll = TRUE;
	}

	// サイズの読み直し
	UINT_PTR posEnd = m_hex.GetEndAddr();

	// 現在位置のデータを保存
	pUndoData = m_hex.AllocRead(sel.from, 1, &size);
	BYTE cb = *pUndoData;

	UINT_PTR posNew = sel.from;
	BOOL bInsert = gbInsert;
	if (type == SST_TEXT) {
		if (!bInsert && sel.from >= posEnd)
			bInsert = TRUE;

		cb = (BYTE)nChar;
		posNew++;
	}
	else {
		if (!bInsert) {
			if (!bInsert && sel.from >= posEnd)
				bInsert = TRUE;

			if (type == SST_HIBYTE)
				cb = (cb & 0x0f) | (val << 4);
			else {
				cb = (cb & 0xf0) | val;
				posNew++;
			}
		}
		else {
			if (m_hex.m_posType == SST_HIBYTE)
				cb = (val << 4) & 0xf0;
			else {
				cb = (cb & 0xf0) | val;
				bInsert = FALSE;
				posNew++;
			}
		}
		type = (type == SST_HIBYTE) ? SST_LOBYTE : SST_HIBYTE;
	}

	if (!bInsert) {
		// 上書き
		m_hex.SaveUndo(UNDO_OVERWRITE, sel.from, pUndoData, 1);
		m_hex.Write(sel.from, &cb, 1);
	}
	else {
		// 挿入
		m_hex.SaveUndo(UNDO_INSERT, sel.from, pUndoData, 1);
		m_hex.Insert(sel.from, &cb, 1);
		SetupScrollBars();			// スクロールバーのサイズを変更
	}
	m_hex.MoveTo(posNew, type);

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;

	if (posNew >= m_posTop + m_iRealPageBytes) {
		CRect rcw = rc;
		rcw.bottom = rc.top + (LONG)(ROW(posNew - m_posTop)) * m_yChar;
		ScrollWindow(0, -m_yChar, NULL, &rcw);

		SetTopIndex(m_posTop + 16);
		SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(m_posTop)), TRUE);
	}

	if (!bRedrawAll) {
		rc.top += (LONG)(ROW(sel.from - m_posTop)) * m_yChar;
		if (!bInsert) {
			rc.bottom = rc.top + m_yChar;
			if (ROWHEAD(sel.from) != ROWHEAD(posNew))
				rc.bottom += m_yChar;
		}
	}
	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	ResizeCaret(TRUE);

	// 編集フラグを立てる
	m_pFrame->Modified(m_hex.IsModified());
	return TRUE;
}


void CBeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	bool bCtrl = (GetKeyState(VK_CONTROL) < 0);
	bool bShift = (GetKeyState(VK_SHIFT) < 0);

	UINT nCmd = 0;
	if (bCtrl) {
		switch (toupper(nChar)) {
		case 'F':		nCmd = ID_EDIT_FIND;	break;
		case 'J':		nCmd = ID_JUMP_OFFSET;	break;
		case 'N':		nCmd = ID_FILE_NEW;		break;
		case 'O':		nCmd = ID_FILE_OPEN;	break;
		case 'S':		nCmd = ID_FILE_SAVE;	break;
		case 'V':		nCmd = ID_EDIT_PASTE;	break;
		case 'X':		nCmd = ID_EDIT_CUT;		break;
		case 'Z':		nCmd = ID_EDIT_UNDO;	break;
		case VK_INSERT:
		case 'C':
			nCmd = bShift ? ID_EDIT_TEXTCOPY : ID_EDIT_COPY;	break;
		default:								break;
		}
	}

	if (nCmd) {
		GetParent().PostMessage(WM_COMMAND, nCmd);
		return;
	}

	switch (nChar) {
	case VK_ESCAPE:
	case VK_RETURN:	nCmd = ID_APP_EXIT;	break;
	case VK_INSERT:	nCmd = bShift ? ID_EDIT_PASTE : 0;	break;
	case VK_BACK:	nCmd = (GetKeyState(VK_MENU) < 0) ? ID_EDIT_UNDO : 0;	break;
	case VK_F3:		nCmd = bShift ? ID_FIND_PREV : ID_FIND_NEXT;	break;
	default:		break;
	}

	if (nCmd) {
		GetParent().PostMessage(WM_COMMAND, nCmd);
		return;
	}

	if (!XferKey(nChar, nRepCnt, nFlags))
		DefWindowProc();
}

BOOL CBeView::XferKey(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	size_t nTotal = m_hex.GetTotalSize();
	if (m_bCaptured ||
		(nChar != VK_TAB && nChar != VK_INSERT && nTotal <= 1) ||
		cbKeyTable[nChar] >= 0)
		return FALSE;

	bool bShift = (GetKeyState(VK_SHIFT) < 0);
	bool bRepeat = ((nFlags & KF_REPEAT) != 0);
	switch (nChar) {
	case VK_LEFT:	KeyLeft(bShift, bRepeat);	break;
	case VK_RIGHT:	KeyRight(bShift, bRepeat);	break;
	case VK_UP:		KeyUp(bShift, bRepeat);		break;
	case VK_DOWN:	KeyDown(bShift, bRepeat);	break;
	case VK_HOME:	KeyHome();		break;
	case VK_END:	KeyEnd();		break;
	case VK_PRIOR:	KeyPageUp();	break;
	case VK_NEXT:	KeyPageDown();	break;
	case VK_TAB:	KeyTab();		break;				// 選択されているビューの変更
	case VK_INSERT:	KeyInsert();	break;				// 挿入、上書きモードの切り替え
	case VK_F5:		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);	break;
	case VK_RETURN:	break;
	case VK_DELETE:	if (!m_bFinding) KeyDelete(bShift); break;
	case VK_BACK:	if (!m_bFinding) KeyBack(); break;
	default:
		return FALSE;
	}
	return TRUE;
}

void CBeView::KeyInsert()
{
	gbInsert ^= TRUE;
	ResizeCaret(TRUE);				// キャレットのサイズ・位置を変更
}

void CBeView::KeyTab()
{
	if (m_hex.IsSelected()) {
		CRange sel = m_hex.GetSelRange();
		if (m_hex.m_posType == SST_TEXT)
			m_hex.m_posType = (sel.from < sel.to) ? SST_LOBYTE : SST_HIBYTE;
		else
			m_hex.m_posType = SST_TEXT;
	}
	else
		m_hex.m_posType = (m_hex.m_posType == SST_TEXT) ? SST_HIBYTE : SST_TEXT;

	MoveCaret();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CBeView::KeyLeft(bool bShiftKey, bool bKeyRepeat)
{
	UINT_PTR posCur = m_hex.GetCurAddr();
	UINT_PTR posNew = posCur;
	UINT posType = m_hex.m_posType;
	if (posNew == 0 && posType != SST_LOBYTE) return;

	bool bRedrawAll = false;
	if (!bShiftKey) {
		if (m_hex.IsSelected())
			bRedrawAll = true;
		posNew = m_hex.MoveToLeft(bKeyRepeat);
	}
	else {
		if (m_hex.IsSelected())
			posNew--;
		m_hex.UpdateSelRange(posNew);
	}

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	if (ROWHEAD(posCur) != ROWHEAD(posNew)) {
		if (posNew < m_posTop + (m_scroll_padline << 4) && m_posTop > 0) {
			if (!bRedrawAll) {
				CRect rcw = rc;
				rcw.top = rc.top + m_yChar * (m_scroll_padline + 1);
				ScrollWindow(0, m_yChar, NULL, &rcw);
				rc.bottom = rcw.top + m_yChar;
			}
			SetTopIndex(m_posTop - 16);
			SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(m_posTop)), TRUE);
		}
		else {
			if (!bRedrawAll) {
				rc.top += (LONG)POS2ROW(posNew - m_posTop) * m_yChar;
				rc.bottom = rc.top + m_yChar * 2;
			}
		}
	}
	else {
		if (!bRedrawAll) {
			rc.top += (LONG)POS2ROW(posNew - m_posTop) * m_yChar;
			rc.bottom = rc.top + m_yChar;
		}
	}

	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	ResizeCaret(TRUE);
}

void CBeView::KeyRight(bool bShiftKey, bool bKeyRepeat)
{
	UINT_PTR posEnd = m_hex.GetEndAddr();
	UINT_PTR posCur = m_hex.GetCurAddr();
	UINT_PTR posNew = posCur;
	if (posNew >= posEnd) return;

	UINT posType = m_hex.m_posType;
	BOOL bRedrawAll = FALSE;
	if (!bShiftKey) {
		if (m_hex.IsSelected())
			bRedrawAll = TRUE;
		posNew = m_hex.MoveToRight(bKeyRepeat);
	}
	else {			// 複数選択されている
		if (m_hex.IsSelected())
			posNew++;
		m_hex.UpdateSelRange(posNew);
	}

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	if (ROWHEAD(posCur) != ROWHEAD(posNew)) {
		if (posNew >= m_posTop + m_iRealPageBytes - (m_scroll_padline << 4)) {
			if (!bRedrawAll) {
				CRect rcw = rc;
				rcw.bottom = rc.top + (LONG)ROW(posCur - m_posTop) * m_yChar;
				ScrollWindow(0, -m_yChar, NULL, &rcw);
				rc.top = rcw.bottom;
			}
			SetTopIndex(m_posTop + 16);
			SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(m_posTop)), TRUE);
		}
		else {
			if (!bRedrawAll) {
				rc.top += (LONG)POS2ROW(posCur - m_posTop) * m_yChar;
				rc.bottom = rc.top + m_yChar * 2;
			}
		}
	}
	else {
		if (!bRedrawAll) {
			rc.top += (LONG)POS2ROW(posNew - m_posTop) * m_yChar;
			rc.bottom = rc.top + m_yChar;
		}
	}

	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	ResizeCaret(TRUE);
}

void CBeView::KeyUp(bool bShiftKey, bool bKeyRepeat)
{
	UINT_PTR posCur = m_hex.GetCurAddr();
	if (posCur < 16) return;					// 先頭行

	size_t curLine = (posCur - m_posTop) >> 4;
	bKeyRepeat = bKeyRepeat && ((curLine - 1) < m_scroll_padline);

	UINT_PTR posNew = posCur;
	bool bRedrawAll = false;
	if (bShiftKey) {
		posNew -= 16;
		m_hex.UpdateSelRange(posNew);
	}
	else {
		if (m_hex.IsSelected())
			bRedrawAll = true;
		posNew = m_hex.MoveToUp(bKeyRepeat);
	}

	int numLines = (posCur - posNew) >> 4;
	size_t newLine = curLine - numLines;

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	if (newLine < m_scroll_padline && m_posTop > 0) {
		if (!bRedrawAll) {
			CRect rcw = rc;
			rcw.top = rc.top + m_yChar * (curLine + numLines);
			ScrollWindow(0, m_yChar * numLines, NULL, &rcw);
			rc.bottom = rcw.top + m_yChar;
		}
		SetTopIndex(m_posTop - (numLines << 4));
		SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(m_posTop)), TRUE);
	}
	else {
		if (!bRedrawAll) {
			rc.top += (LONG)POS2ROW(posNew - m_posTop) * m_yChar;
			rc.bottom = rc.top + m_yChar * (numLines + 1);
		}
	}

	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	ResizeCaret(TRUE);
}

void CBeView::KeyDown(bool bShiftKey, bool bKeyRepeat)
{
	UINT_PTR posEnd = m_hex.GetEndAddr();
	UINT_PTR posCur = m_hex.GetCurAddr();

	if (posCur + 16 >= posEnd) return;			// 最終行

	size_t curLine = (posCur - m_posTop) >> 4;
	bKeyRepeat = bKeyRepeat && ((curLine + 1) >= (m_iRealLines - m_scroll_padline));

	UINT_PTR posNew = posCur;
	bool bRedrawAll = false;
	if (bShiftKey) {							// 選択範囲を更新
		posNew += 16;
		m_hex.UpdateSelRange(posNew);
	}
	else {
		if (m_hex.IsSelected())					// 既に選択中であれば選択を解除
			bRedrawAll = true;
		posNew = m_hex.MoveToDown(bKeyRepeat);
	}
	size_t newLine = (posNew - m_posTop) >> 4;
	int numLines = newLine - curLine;

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	if (newLine >= (m_iRealLines - m_scroll_padline)) {
		if (!bRedrawAll) {
			CRect rcw = rc;
			rcw.bottom = rc.top + curLine * m_yChar;
			ScrollWindow(0, -m_yChar * numLines, NULL, &rcw);
			rc.top = rcw.bottom;
		}
		SetTopIndex(m_posTop + (numLines << 4));
		SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(m_posTop)), TRUE);
	}
	else {
		if (!bRedrawAll) {
			rc.top += curLine * m_yChar;
			rc.bottom = rc.top + m_yChar * (numLines + 1);
		}
	}

	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	ResizeCaret(TRUE);
}

void CBeView::KeyHome()
{
	UINT_PTR posNew = m_hex.GetCurAddr();
	int type = (m_hex.m_posType != SST_TEXT) ? SST_HIBYTE : SST_TEXT;

	// コントロールキーが押下されていれば、ファイルの先頭へ
	if (_ISKEYDOWN(VK_CONTROL)) {
		if (posNew) {
			if (_ISKEYDOWN(VK_SHIFT))
				m_hex.UpdateSelRange(0);
			else
				m_hex.MoveTo(0, type);

			SetTopIndex(0);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			ResizeCaret(TRUE);
		}
		return;
	}

	// 現在行頭にいるかチェック
	if (ISROWHEAD(posNew))
		return;

	BOOL bRedrawAll = FALSE;
	posNew = ROWHEAD(posNew);
	if (_ISKEYDOWN(VK_SHIFT))
		m_hex.UpdateSelRange(posNew);
	else {
		if (m_hex.IsSelected())
			bRedrawAll = TRUE;
		m_hex.MoveTo(posNew, type);
	}

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	if (!bRedrawAll) {
		rc.top += (LONG)ROW(posNew - m_posTop) * m_yChar;
		rc.bottom = rc.top + m_yChar;
	}

	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	ResizeCaret(TRUE);
}

void CBeView::KeyEnd()
{
	UINT_PTR posNew = m_hex.GetCurAddr();
	UINT_PTR posEnd = m_hex.GetEndAddr();
	UINT posType = m_hex.m_posType;

	// コントロールキーが押下されていれば、ファイルの終わりへ
	if (_ISKEYDOWN(VK_CONTROL)) {
		if (posNew == posEnd) return;

		if (_ISKEYDOWN(VK_SHIFT))
			m_hex.UpdateSelRange(posEnd - 1);
		else
			m_hex.MoveTo(posEnd - 1, (posType == SST_TEXT) ? SST_TEXT : SST_LOBYTE);

		posNew = (posEnd > m_iRealPageBytes) ? ROWTAIL(posEnd) + 1 - m_iRealPageBytes : 0;

		SetTopIndex(posNew);
		SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(posNew)), TRUE);

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		ResizeCaret(TRUE);
		return;
	}

	BOOL bRedrawAll = FALSE;
	posNew = ROWTAIL(posNew);
	if (_ISKEYDOWN(VK_SHIFT))
		m_hex.UpdateSelRange(min(posNew, posEnd - 1));
	else {
		if (m_hex.IsSelected())
			bRedrawAll = TRUE;
		m_hex.MoveTo(min(posNew, posEnd), (posType == SST_TEXT) ? SST_TEXT : SST_LOBYTE);
	}

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	if (!bRedrawAll) {
		rc.top += (LONG)ROW(posNew - m_posTop) * m_yChar;
		rc.bottom = rc.top + m_yChar;
	}

	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	ResizeCaret(TRUE);
}

void CBeView::KeyPageUp()
{
	UINT_PTR posStart = m_posTop;
	UINT_PTR posCur = m_hex.GetCurAddr();
	UINT posType = m_hex.m_posType;

	if (posCur == 0)
		return;

	if (posStart >= m_iRealPageBytes) {
		posStart -= m_iRealPageBytes;
		posCur -= m_iRealPageBytes;
	}
	else {
		posStart = 0;
		posCur = 0;
		if (posType != SST_TEXT)
			posType = SST_HIBYTE;
	}

	if (GetKeyState(VK_SHIFT) < 0)
		m_hex.UpdateSelRange(posCur);
	else
		m_hex.MoveTo(posCur, posType);
	ResizeCaret();

	if (posStart != m_posTop)
		JumpToPosition(posStart);
	else
		RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
}

void CBeView::KeyPageDown()
{
	UINT_PTR posEnd = m_hex.GetEndAddr();
	UINT_PTR posStart = m_posTop;
	UINT_PTR posCur = m_hex.GetCurAddr();
	UINT posType = m_hex.m_posType;

	if (posStart == posEnd && posCur == 0) return;

	if (posStart + m_iRealPageBytes <= posEnd)
		posStart += m_iRealPageBytes;

	if (posCur + m_iRealPageBytes > posEnd) {
		posCur = posEnd;
		if (posType != SST_TEXT)
			posType = SST_HIBYTE;
	}
	else
		posCur += m_iRealPageBytes;

	if (GetKeyState(VK_SHIFT) < 0)
		m_hex.UpdateSelRange(posCur);
	else
		m_hex.MoveTo(posCur, posType);
	ResizeCaret();

	if (posStart != m_posTop)
		JumpToPosition(posStart);
	else
		RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
}

void CBeView::KeyBack()
{
	if (m_hex.IsReadOnly()) return;
	if (m_hThread) return;

	CRange sel = m_hex.GetSelRange();
	sel.Normalize();

	UINT type = m_hex.m_posType;

	size_t nUndoSize;
	LPBYTE pUndoData = NULL;
	if (m_hex.IsSelected()) {
		pUndoData = m_hex.CutSelectedRange(&nUndoSize);
		if (pUndoData && GetKeyState(VK_SHIFT) < 0)
			CopyToClipboard(pUndoData, nUndoSize);
	}
	else {
		if (!sel.from)
			return;

		sel.from--;
		pUndoData = m_hex.AllocRead(sel.from, 1, &nUndoSize);
		if (pUndoData && !m_hex.Delete(sel.from, 1)) {
			FREE(pUndoData);
			pUndoData = NULL;
		}
	}

	if (pUndoData) {
		m_hex.SaveUndo(UNDO_DELETE, sel.from, pUndoData, nUndoSize);

		//ドキュメントの変更フラグを立てる
		m_pFrame->Modified(m_hex.IsModified());

		if (type != SST_TEXT)
			type = SST_HIBYTE;

		m_hex.MoveTo(sel.from, type);
		ResizeCaret();

SetupScrollBars();

		if (sel.from < m_posTop)
			JumpToPosition(ROWHEAD(sel.from));
		else
			RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
	}
}

void CBeView::KeyDelete(bool bClip)
{
	if (m_hex.IsReadOnly()) return;
	if (m_hex.GetTotalSize() <= 1) return;
	if (m_hThread) return;

	CRange sel = m_hex.GetSelRange();
	sel.Normalize();

	UINT type = m_hex.m_posType;
	size_t nUndoSize;
	LPBYTE pUndoData = NULL;
	if (m_hex.IsSelected()) {
		pUndoData = m_hex.CutSelectedRange(&nUndoSize);
		if (pUndoData && bClip)
			CopyToClipboard(pUndoData, nUndoSize);
	}
	else {
		if (m_hex.GetEndAddr() == m_hex.GetCurAddr()) return;

		pUndoData = m_hex.AllocRead(sel.from, 1, &nUndoSize);
		if (pUndoData) {
			if (!m_hex.Delete(sel.from, 1)) {
				FREE(pUndoData);
				pUndoData = NULL;
			}
		}
	}

	if (pUndoData) {
		m_hex.SaveUndo(UNDO_DELETE, sel.from, pUndoData, nUndoSize);

		// ドキュメントの変更フラグを立てる
		m_pFrame->Modified(m_hex.IsModified());

		if (type != SST_TEXT)
			type = SST_HIBYTE;

		SetupScrollBars();

		m_hex.MoveTo(sel.from, type);
		if (m_posTop > sel.from) {
			UINT_PTR pd = ROWHEAD(sel.from);
			if (pd > 0) pd -= 16;
			JumpToPosition(pd);
		}
		else
			RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
		ResizeCaret(TRUE);
	}
}

void CBeView::CreateWorkerBitmaps()
{
	static TCHAR ssz[] = _T("+A");

	CClientDC dc(m_hWnd);

	CRect rc;
	GetClientRect(&rc);

	//
	// ライン用ビットマップ
	//
	if (m_bmLine.m_hBitmap) m_bmLine.DeleteObject();
	m_bmLine.CreateCompatibleBitmap(dc, rc.Width(), m_yChar);

	//
	// ヘッダ用ビットマップ
	//
	int sx = max(OFS_ALL * m_xChar, rc.Width());
	if (m_bmHeader.m_hBitmap) m_bmHeader.DeleteObject();
	m_bmHeader.CreateCompatibleBitmap(dc, sx, m_yChar);
	ATLASSERT(m_bmHeader.m_hBitmap != NULL);

	CDC dcw;
	dcw.CreateCompatibleDC(dc);

	CBitmapHandle bmOld = dcw.SelectBitmap(m_bmHeader);
	CFontHandle fontOld = dcw.SelectFont(m_font);

	// ビットマップを初期化
	int cy = m_yChar - m_cyLineMargin;
	dcw.FillSolidRect(0, 0, sx, m_yChar, m_vc.rgbBack);
	dcw.FillSolidRect(OFS_BYTE * m_xChar, 0, m_xChar * 47, cy, m_vc.rgbAddr);
	dcw.FillSolidRect(OFS_TEXT * m_xChar, 0, m_xChar * 16, cy, m_vc.rgbAddr);

	dcw.SetTextColor(m_vc.rgbBack);

	dcw.SetBkMode(TRANSPARENT);
	int x1 = OFS_BYTE * m_xChar;
	int x2 = OFS_TEXT * m_xChar;
	for (int i = 0; i < 16; i++) {
		ssz[1] = (i < 10) ? i + '0' : i + 'A' - 10;

		dcw.TextOut(x1, 0, ssz, 2);
		x1 += m_xChar * 3;
		dcw.TextOut(x2, 0, &ssz[1], 1);
		x2 += m_xChar;
	}

	dcw.SelectFont(fontOld);
	dcw.SelectBitmap(bmOld);
	dcw.DeleteDC();
}

void CBeView::OnLButtonDown(UINT nFlags, CPoint pt)
{
	if (m_bFinding || m_hex.GetTotalSize() <= 1)
		return;

	UINT_PTR posCur;
	int type;
	int nHitTest = DataHitTest(pt, &posCur, &type);
	if (nHitTest == VHT_HEADER ||
		nHitTest == VHT_NCCLIENT ||
		nHitTest == VHT_CLIENT)
		return;

	CRange sel = m_hex.GetSelRange();
	sel.Normalize();

	bool bDragOut = false;
	bool bShift = ((nFlags & MK_SHIFT) != 0);
	if (nHitTest == VHT_ADDR) {				// アドレス領域
		if (bShift) {						// SHIFTキー併用
			if (sel.from > posCur) {
				m_hex.Select(sel.to, posCur);
			} else {
				m_hex.Select(sel.from, ROWTAIL(posCur));
			}
		} else{								// SHIFTキーなし
			if (sel.from <= posCur && posCur <= sel.to) {
				// 外部へD&D
				bDragOut = true;
			} else {
				m_hex.Select(posCur, ROWTAIL(posCur));
			}
		}
	} else if (nHitTest == VHT_BYTE) {		// バイナリデータ領域
		if (bShift) {						// SHIFTキー併用
			if (sel.from > posCur) {
				m_hex.Select(sel.to, posCur);
			} else if (sel.to < posCur) {
				m_hex.Select(sel.from, posCur);
			} else {
				m_hex.Select(posCur, posCur);
			}
		} else {							// SHIFTキーなし
			if (sel.from <= posCur && posCur <= sel.to) {
				// 外部へD&D
				bDragOut = true;
			} else {
				m_hex.MoveTo(posCur, type);
			}
		}
	} else {								// テキストデータ領域
		if (bShift) {						// SHIFTキー併用
			if (sel.from > posCur) {
				m_hex.Select(sel.to, posCur);
			} else if (sel.to < posCur) {
				m_hex.Select(sel.from, posCur);
			} else {
				m_hex.Select(posCur, posCur);
			}
		} else {							// SHIFTキーなし
			if (sel.from <= posCur && posCur <= sel.to) {
				// 外部へD&D
				bDragOut = true;
			} else {
				m_hex.MoveTo(posCur, type);
			}
		}
	}

	if (bDragOut) {
		CRange sel = m_hex.GetSelRange();
		sel.Normalize();
		size_t size;
		BYTE* pb = m_hex.ReadSelectedRange(&size);
		if (pb) {
			GetCursorPos(&pt);
			if (lccBeginTrack(m_hWnd, pt)) {
				CDataObject* pdo = new CDataObject(m_strFileName, pb, size);
				DWORD eff = DROPEFFECT_COPY;
				SHDoDragDrop(m_hWnd, pdo, NULL, eff, &eff);
				pdo->Release();
			}
			FREE(pb);
		}
	} else {
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		ResizeCaret(TRUE);
		if (DoDragSelect()) {
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			ResizeCaret(TRUE);
		}
	}
}

BOOL CBeView::BeginDrag()
{
	BOOL bDragStarted = FALSE;
	CPoint pt = GetCursorPos(&pt);
	CRect rcStart(pt.x - 2, pt.y - 2, pt.x + 2, pt.y + 2);

	DWORD dwButtonCancel = 0;
	DWORD dwButtonDrop = 0;
	if (GetKeyState(VK_LBUTTON) < 0) {
		dwButtonDrop |= MK_LBUTTON;
		dwButtonCancel |= MK_RBUTTON;
	}
	else if (GetKeyState(VK_RBUTTON) < 0) {
		dwButtonDrop |= MK_RBUTTON;
		dwButtonCancel |= MK_LBUTTON;
	}

	SetCapture();
	while (!bDragStarted) {
		if (GetCapture() != m_hWnd)
			break;

		MSG msg;
		if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
			PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
			if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP ||
				msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN)
				break;

			if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
				break;

			bDragStarted = !rcStart.PtInRect(msg.pt);
		}
	}
	ReleaseCapture();
	return bDragStarted;
}

BOOL CBeView::DoDragSelect()
{
	if (!BeginDrag())
		return FALSE;

	DeleteCaret();
	m_bCaptured = TRUE;

	SetCapture();
	CPoint pt;
	UINT_PTR posEnd = m_hex.GetEndAddr();

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;

	UINT typeOld = m_hex.m_posType;
	CRange selOld = m_hex.GetSelRange();
	do {
		ZapMessages(NULL);

		// マウスポイントを取得
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		int x, y;
		int nHitTest = OffsetHitTest(pt, &x, &y);

		UINT_PTR posCur = min(m_posTop + (y << 4) + x, posEnd - 1);

		CRange sel = m_hex.GetSelRange();
		if (nHitTest == VHT_ADDR) {
			if (sel.from <= posCur)
				posCur = min(ROWTAIL(posCur), posEnd - 1);

			if (m_hex.IsSelected())
				m_hex.UpdateSelRange(posCur);
			else
				m_hex.Select(sel.from, posCur);
		} else if (nHitTest == VHT_BYTE) {
			if (m_hex.IsSelected())
				m_hex.UpdateSelRange(posCur);
			else
				m_hex.Select(sel.from, posCur);
		} else if (nHitTest == VHT_TEXT) {
			if (m_hex.IsSelected())
				m_hex.UpdateSelRange(posCur);
			else
				m_hex.Select(sel.from, posCur);
		}

		if (sel != selOld || typeOld != m_hex.m_posType) {
			CRange s1 = selOld;
			s1.Normalize();
			s1.from = max(m_posTop, s1.from);
			s1.to = min(m_posTop + m_iPageBytes, s1.to);

			CRange s2 = sel;
			s2.Normalize();
			s2.from = max(m_posTop, s2.from);
			s2.to = min(m_posTop + m_iPageBytes, s2.to);

			CRect rc1 = rc;
			rc1.top = rc.top + m_yChar * (LONG)ROW(s1.from - m_posTop);
			rc1.bottom = rc.top + m_yChar * (LONG)ROW(s1.to - m_posTop) + m_yChar;

			CRect rc2 = rc;
			rc2.top = rc.top + m_yChar * (LONG)ROW(s2.from - m_posTop);
			rc2.bottom = rc.top + m_yChar * (LONG)ROW(s2.to - m_posTop) + m_yChar;

			CRgn r1, r2, r3;
			r1.CreateRectRgnIndirect(&rc1);
			r2.CreateRectRgnIndirect(&rc2);
			r3.CreateRectRgn(0, 0, 0, 0);
			r3.CombineRgn(r1, r2, RGN_OR);
			RedrawWindow(NULL, r3, RDW_INVALIDATE | RDW_UPDATENOW);
		}

		if (!rc.PtInRect(pt)) {
			int xScroll = 0, yScroll = 0;
			UINT_PTR posStart = m_posTop;
			CRect rcw = rc;

			if (posStart > 0 && pt.y < rc.top) {
				// 前方アドレス
				UINT_PTR nmax = max(0, sel.Min() - posStart);
				rcw.top += m_yChar + m_yChar * (LONG)ROW(nmax);
				yScroll = m_yChar;
				posStart -= 16;

				m_hex.UpdateSelRange(posStart + x);
			} else if (posStart + m_iRealPageBytes < posEnd - 1 &&
					pt.y >= rc.top + m_iRealLines * m_yChar) {
				// 後方アドレス
				rcw.bottom = rcw.top + m_yChar * (LONG)ROW(sel.Max() - posStart);
				yScroll = -m_yChar;
				posStart += 16;

				if (nHitTest == VHT_ADDR)
					x = 15;
				UINT_PTR pd = ROWHEAD(posStart + m_iRealPageBytes) + x;
				pd = min(pd, posEnd - 1);

				m_hex.UpdateSelRange(pd);
			}

			if (yScroll) {
Sleep(20);
				ScrollWindow(0, yScroll, NULL, &rcw);
				SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(posStart)), TRUE);
				SetTopIndex(posStart);

				CRect rcRedraw;
				rcRedraw.SubtractRect(&rc, &rcw);
				RedrawWindow(&rcRedraw, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				sel = m_hex.GetSelRange();
				selOld = sel;
				typeOld = m_hex.m_posType;
			}
		}


		selOld = sel;
		typeOld = m_hex.m_posType;
	} while (_ISKEYDOWN(VK_LBUTTON) && !_ISKEYDOWN(VK_ESCAPE));

	m_bCaptured = FALSE;
	ReleaseCapture();
	return TRUE;
}

int CBeView::HitTest(CPoint pt)
{
	CRect rc;
	GetClientRect(&rc);
	rc.OffsetRect(-GetScrollPos(SB_HORZ), 0);

	CRect rcw = rc;
	rcw.bottom = rcw.top + m_yChar;
	if (rcw.PtInRect(pt))
		return VHT_HEADER;

	rcw = rc;
	rcw.top += m_yChar;
	if (!rcw.PtInRect(pt))
		return VHT_NCCLIENT;

	// スクロールバーの位置を取得
	rcw.right = rcw.left + OFS_BYTE * m_xChar;
	if (rcw.PtInRect(pt))
		return VHT_ADDR;

	rcw.left = rcw.right;
	rcw.right = rc.left + OFS_TEXT * m_xChar;
	if (rcw.PtInRect(pt))
		return VHT_BYTE;

	return VHT_TEXT;
}

void CBeView::DeleteCaret()
{
	// キャレット削除
	DestroyCaret();
	m_bCaret = FALSE;
}

#define BMPWIDTH(bits)    (((bits) + 15) / 16 * 2)

void CBeView::ResizeCaret(BOOL bMove)
{
	DeleteCaret();
	if (m_bCaptured)
		return;

	int xBlock = m_xChar;
	int yBlock = m_yChar - m_cyLineMargin;
	int cx = BMPWIDTH(m_xChar);
	BYTE* pBuffer = (BYTE*)malloc(cx * yBlock);
	if (!pBuffer) return;

	memset(pBuffer, 0, cx * yBlock);

	int i;
	BYTE* pb = pBuffer;
	if (m_hex.IsSelected()) {
		bool bLeftSide = true;
		if (m_hex.m_posType == SST_LOBYTE)
			bLeftSide = false;

		if (bLeftSide) {
			pb = pBuffer;
			for (i = 0; i < yBlock; i++) {
				*pb = 0x80;
				pb += cx;
			}
		}
		else {
			BYTE cb = 0x01;
			pb = pBuffer + (xBlock >> 3);
			int shift = xBlock & 0x7;
			if (!shift)
				pb--;
			else {
				cb = 0x80;
				while (--shift > 0)
					cb >>= 1;
			}

			for (i = 0; i < yBlock; i++) {
				*pb = cb;
				pb += cx;
			}
		}
	}
	else {
		if (!gbInsert)
			memset(pBuffer, 0xff, cx * yBlock);
		else {
			pb = pBuffer + (yBlock >> 1) * cx;
			for (i = yBlock >> 1; i < yBlock; i++) {
				memset(pb, 0xff, cx);
				pb += cx;
			}
		}
	}
	if (m_bmCaret.m_hBitmap) m_bmCaret.DeleteObject();
	m_bmCaret.CreateBitmap(xBlock, yBlock, 1, 1, pBuffer);
	free(pBuffer);

	CreateCaret(m_bmCaret);

	ShowCaret();
	m_bCaret = TRUE;

	if (bMove)
		MoveCaret();
}

void CBeView::MoveCaret()
{
	int x = -GetScrollPos(SB_HORZ);
	int y = m_yChar;

	UINT_PTR pos = m_hex.GetCurAddr() - m_posTop;
	y += (int)ROW(pos) * m_yChar;
	int dx = 0;
	if (m_hex.m_posType == SST_TEXT)
		dx = (OFS_TEXT + (int)COL(pos)) * m_xChar;
	else {
		dx = (OFS_BYTE  + (int)COL(pos) * 3) * m_xChar;
		if (m_hex.m_posType == SST_LOBYTE)
			dx += m_xChar;
	}
	::SetCaretPos(x + dx, y);

	// カラムヘッダを描画
	if (m_bmHeader.m_hBitmap) {
		BITMAP bm;
		m_bmHeader.GetBitmap(&bm);

		CClientDC dc(m_hWnd);

		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		CBitmapHandle bmOld = dcw.SelectBitmap(m_bmHeader);

		CDC dcm;
		dcm.CreateCompatibleDC(dc);
		CBitmap bmHeader;
		bmHeader.CreateCompatibleBitmap(dc, bm.bmWidth, bm.bmHeight);
		CBitmapHandle bmMem = dcm.SelectBitmap(bmHeader);

		dcm.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, dcw, 0, 0, SRCCOPY);
		dcm.PatBlt(dx, 0, m_xChar, m_yChar - m_cyLineMargin, DSTINVERT);

		dc.SetTextColor(m_vc.rgbAddr);
		dc.SetBkColor(m_vc.rgbBack);
		dc.BitBlt(x, 0, bm.bmWidth, bm.bmHeight, dcm, 0, 0, SRCCOPY);

		dcw.SelectBitmap(bmOld);
		dcm.SelectBitmap(bmMem);
	}

	GetParent().SendMessage(KEM_CURSORMOVE, m_hex.GetCurAddr());
}

void CBeView::OnEditCopy() 
{
	if (m_bFinding || !m_hex.IsSelected())
		return;

	size_t size;
	LPBYTE pb = m_hex.ReadSelectedRange(&size);
	if (pb) {
		CopyToClipboard(pb, size);
		FREE(pb);

		m_hex.MoveTo(m_hex.GetCurAddr(), m_hex.m_posType);
		ResizeCaret();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CBeView::OnEditTextCopy() 
{
	if (m_bFinding || !m_hex.IsSelected())
		return;

	CRange sel = m_hex.GetSelRange();
	sel.Normalize();

	size_t size;
	LPBYTE pb = m_hex.ReadSelectedRange(&size);
	if (pb) {
		CString cs, ct, cw;

		// ここでテキスト形式展開をおこなう
		cs.Empty();
		UINT_PTR s = ROWHEAD(sel.from), e = ROWTAIL(sel.to), w = 0;
		for (UINT_PTR i = s; i <= e; i++) {
			if (ISROWHEAD(i))
				ct.Format(_T("%08X: "), i);
			if (sel.InRange(i))
				cw.Format(_T("%02X "), pb[w++]);
			else
				cw = _T("   ");
			ct += cw;
			if (ISROWTAIL(i)) {
				cs += ct;
				cs += _T("\x0d\x0a");
			}
		}
		lccSetStringToClipboard(m_hWnd, cs);
		FREE(pb);

		m_hex.MoveTo(m_hex.GetCurAddr(), m_hex.m_posType);
		ResizeCaret();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CBeView::OnEditCut()
{
	if (m_hex.IsReadOnly() || m_bFinding || !m_hex.IsSelected()) return;

	KeyDelete(true);
}

void CBeView::OnEditPaste() 
{
	if (m_hex.IsReadOnly() || m_bFinding) return;

	CLIPBINDATA* clip = CopyFromClipboard();
	if (clip) {
		DoPaste(clip->data, clip->datasize);
		FREE(clip);
	}
}

BOOL CBeView::SelRangeToNewFile(LPCTSTR pszFileToSave)
{
	BOOL bOK = FALSE;
	size_t size;
	LPBYTE pb = m_hex.ReadSelectedRange(&size);
	if (pb && size > 0) {
		HANDLE hf = CreateFile(pszFileToSave, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			DWORD tmp;
			WriteFile(hf, pb, (DWORD)size, &tmp, NULL);
			SetEndOfFile(hf);
			CloseHandle(hf);
			bOK = TRUE;
		}
		else
			LastError(pszFileToSave, MB_STOP);
		FREE(pb);
	}
	return bOK;
}

void CBeView::SendDataToWindow(HWND hwnd)
{
	COPYDATASTRUCT cds;
	memset(&cds, 0, sizeof(cds));
	cds.dwData = 100;
	size_t size;
	LPBYTE pb = m_hex.ReadSelectedRange(&size);
	if (pb && size > 0) {
		cds.cbData = size;
		cds.lpData = pb;
	}

	// 選択範囲のデータが読み込みできなかったら空を渡す
	::SendMessage(hwnd, WM_COPYDATA, (WPARAM)m_pFrame->m_hWnd, (LPARAM)&cds);

	if (pb)
		FREE(pb);
}


void CBeView::OnEditCopyToNewFile()
{
	if (m_bFinding || !m_hex.IsSelected())
		return;

	TCHAR name[MAX_PATH * 2];
	lstrcpy(name, m_strFileName);
	PathDecorate(name);
	if (ShowOpenSaveFileDialog(m_hWnd, false, MAKEINTRESOURCE(IDS_SAVEAS), NULL, name, ARRAYSIZE(name))) {
		if (PathFileExists(name)) {
			if (PathIsDirectory(name)) {
				AppMessage(IDS_FILEISFOLDER, name, MB_OK | MB_ICONSTOP);
				return;
			}
		}

		if (SelRangeToNewFile(name)) {
			// 選択範囲を解除
			m_hex.MoveTo(m_hex.GetCurAddr(), m_hex.m_posType);
			ResizeCaret();
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
}

void CBeView::OnEditByNewEditor()
{
	if (m_bFinding || !m_hex.IsSelected())
		return;

	CString str;
	str.Format(_T("/V:%p"), m_pFrame->m_hWnd);
	BootNewProcess(m_hWnd, str);
}

void CBeView::DoPaste(const BYTE* pPaste, size_t size)
{
	if (m_hex.IsReadOnly() || m_bFinding || !pPaste || !size) return;

	CRange sel = m_hex.GetSelRange();
	sel.Normalize();
	UINT type = m_hex.m_posType;

	// 選択されている範囲があれば、それを削除
	BOOL bRedrawAll = FALSE;
	LPBYTE pUndoData;
	if (m_hex.IsSelected()) {
		if (type == SST_LOBYTE)
			type = SST_HIBYTE;		// ２バイト目を選択

		// 削除する前にデータを取る
		size_t nUndoSize;
		pUndoData = m_hex.CutSelectedRange(&nUndoSize);
		m_hex.SaveUndo(UNDO_DELETE, sel.from, pUndoData, nUndoSize);
		m_hex.MoveTo(sel.from, type);
		SetTopIndex(min(m_posTop, sel.from));
		bRedrawAll = TRUE;
	}

	// データ挿入
	m_hex.Insert(sel.from, pPaste, size);
	if (sel.from + size >= m_posTop + m_iRealPageBytes) {
		// 挿入データ末尾がクライアント領域に見えるように
		SetTopIndex(ROWHEAD(sel.from + size - m_iRealPageBytes));
		bRedrawAll = TRUE;
	}
	m_hex.MoveTo(sel.from + size, type);

	// アンドゥ保存
	pUndoData = (BYTE*)_memdup(pPaste, size);
	m_hex.SaveUndo(UNDO_INSERT, sel.from, pUndoData, size);

	// スクロールバーのサイズを変更
	SetupScrollBars();
	SetScrollPos(SB_VERT, MapLineToScrollPos(ROW(m_posTop)), TRUE);

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	if (!bRedrawAll)
		rc.top += (LONG)ROW(sel.from - m_posTop) * m_yChar;

	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	ResizeCaret(TRUE);

	// ドキュメントの変更フラグを立てる
	m_pFrame->Modified(m_hex.IsModified());
}

void CBeView::OnEditSelectAll()
{
	UINT_PTR posEnd = m_hex.GetEndAddr();
	if (m_bFinding || posEnd < 1) return;

	m_hex.Select(posEnd - 1, 0);
	JumpToPosition(0);
	ResizeCaret(TRUE);
}

void CBeView::OnRangeSel()
{
	CSelRangeDlg dlg;
	dlg.m_start = m_hex.GetCurAddr();
	if (dlg.DoModal() == IDOK)
		OnJump(dlg.m_start, dlg.m_length);
}


void CBeView::OnFind()
{
	CStrFindDialog dlg;

	dlg.m_bCase = m_bCase;
	dlg.m_bBinarySearch = m_bBinarySearch;

	if (dlg.DoModal() == IDOK) {
		m_csSearch = dlg.m_csSearch;
		m_iFindType = dlg.m_ifindtype;
		m_iSearchLen = lstrlen(m_csSearch);
		m_bCase = dlg.m_bCase;
		m_bBinarySearch = dlg.m_bBinarySearch;

		// 文字列をバイトに変換できるかチェック
		if (m_bBinarySearch) {
			m_iSearchLen = hex2bytes(m_csSearch, m_cbSearchBytes);
			if (!m_iSearchLen)
				return;
		}

		m_bFinding = TRUE;

		//カーソルの読み込み
		HCURSOR hcur = ::LoadCursor(NULL, IDC_WAIT);
		SetCursor(hcur);

		if (m_iFindType == FIND_NEXT)
			OnFindNext();
		else
			OnFindPrev();
	}
	m_bFinding = FALSE;
}

LRESULT CBeView::OnJump(WPARAM wParam, LPARAM lParam)
{
	UINT_PTR posEnd = m_hex.GetEndAddr();
	if (!posEnd) return 0;

	CRange visible(m_posTop, min(m_posTop + m_iRealPageBytes, posEnd - 1));
	CRange fin(wParam, wParam + lParam - 1);

	UINT_PTR posStart;
	bool bJumpNeeded = false;
	if (visible.InRange(fin.from)) {
		if (!visible.InRange(fin.to)) {
			posStart = visible.from + 16;
			bJumpNeeded = true;
		}
	}
	else {
		// 画面中央にカーソルが来るように補正
		posStart = fin.from - m_iRealPageBytes / 2;
		posStart = max(0, posStart);
		bJumpNeeded = true;
	}

	m_hex.Select(fin.to, fin.from);

	if (bJumpNeeded)
		JumpToPosition(posStart);
	else
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	ResizeCaret(TRUE);
	return 0;
}

void CBeView::OnContextMenu(HWND hWnd, CPoint pt) 
{
	if (m_bCaptured || m_bFinding || m_hex.GetTotalSize() <= 1) return;

	CMenuHandle menu = m_pFrame->m_menubar.GetMenu();
	if (menu.m_hMenu) {
		menu = menu.GetSubMenu(1);
		if (menu.m_hMenu)
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_pFrame->m_hWnd, NULL);
	}
}

void CBeView::OnFindPrev()
{
	if (m_iSearchLen < 1)
		return;

	CRange sel = m_hex.GetSelRange();
	sel.Normalize();

	UINT_PTR pos = sel.from;
	if (m_hex.IsSelected() && pos)
		pos--;

	BOOL bFind;
	UINT_PTR posFind;
	if (!m_bBinarySearch)
		bFind = m_hex.FindPrev(pos, m_csSearch, &posFind, m_bCase);
	else
		bFind = m_hex.FindPrev(pos, m_cbSearchBytes, m_iSearchLen, &posFind);

	if (!bFind) {
		MessageBeep(0);
		return;
	}
	else
		OnJump(posFind, m_iSearchLen);
}

void CBeView::OnFindNext()
{
	if (m_iSearchLen < 1)
		return;

	CRange sel = m_hex.GetSelRange();
	sel.Normalize();

	UINT_PTR pos = sel.from;
	if (m_hex.IsSelected())
		pos++;

	UINT_PTR posFind;
	BOOL bFind;
	if (!m_bBinarySearch)
		bFind = m_hex.FindNext(pos, m_csSearch, &posFind, m_bCase);
	else
		bFind = m_hex.FindNext(pos, m_cbSearchBytes, m_iSearchLen, &posFind);

	if (!bFind) {
		MessageBeep(0);
		return;
	}
	else
		OnJump(posFind, m_iSearchLen);
}

void CBeView::SetIMM()
{
	if (m_hIMC) {
		// IMMにフォントを設定
		LOGFONT lf;
		::GetObject(m_font, sizeof(lf), &lf);
		ImmSetCompositionFont(m_hIMC, &lf);
	}
}

void CBeView::StartIME()
{
	if (!m_hIMC)
		return;

	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	rc.OffsetRect(-GetScrollPos(SB_HORZ), 0);

	COMPOSITIONFORM cf;
	CPoint pt;
	::GetCaretPos(&pt);
	size_t ofs = m_hex.GetCurAddr() - m_posTop;
	if (m_hex.m_posType == SST_TEXT) {
		cf.dwStyle = CFS_RECT;
		cf.ptCurrentPos = pt;
		cf.rcArea.left = rc.left + m_xChar * OFS_TEXT;
		cf.rcArea.right = cf.rcArea.left + (m_xChar << 4);
		cf.rcArea.top = rc.top + m_yChar * (LONG)ROW(ofs);
		cf.rcArea.bottom = cf.rcArea.top + rc.Height();
	}
	else {
		cf.dwStyle = CFS_DEFAULT;
		cf.ptCurrentPos.x = 0;
		cf.ptCurrentPos.y = 0;
		cf.rcArea = rc;
	}
	ImmSetCompositionWindow(m_hIMC, &cf);
}

void CBeView::WriteIME()
{
	if (m_hIMC) {
		DWORD size = ImmGetCompositionString(m_hIMC, GCS_RESULTSTR, NULL, 0);
		if (size) {
			TCHAR wstr[512];
			char sstr[512];
			size += sizeof(WCHAR);

			// 文字を取得
			ImmGetCompositionString(m_hIMC, GCS_RESULTSTR, wstr, size);
			size -= sizeof(WCHAR);
			int len = WideCharToMultiByte(CP_ACP, 0, wstr, size / sizeof(WCHAR), sstr, ARRAYSIZE(sstr), NULL, NULL);
			DoPaste((BYTE*)sstr, len);
		}
	}
}

LRESULT CBeView::OnImeComposition(WPARAM wParam, LPARAM lParam)
{
	if (lParam & GCS_RESULTSTR) {
		// メモリに書く
		if (m_hex.m_posType == SST_TEXT)
			WriteIME();
		return 1;
	}
	return DefWindowProc();
}

LRESULT CBeView::OnImeStartComposition(WPARAM wParam, LPARAM lParam)
{
	StartIME();
	return DefWindowProc();
}

LRESULT CBeView::OnImeEndComposition(WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc();
}

BOOL CBeView::OnEraseBkgnd(HDC hdc) 
{
	return TRUE;
}

LRESULT CBeView::OnRegisteredMouseWheel(WPARAM wParam, LPARAM lParam)
{
	WORD keyState = 0;
	keyState |= _ISKEYDOWN(VK_CONTROL) ? MK_CONTROL : 0;
	keyState |= _ISKEYDOWN(VK_SHIFT) ? MK_SHIFT : 0;

	return SendMessage(WM_MOUSEWHEEL, (wParam << 16) | keyState, lParam);
}

BOOL CBeView::OnMouseWheel(UINT fFlags, short delta, CPoint point)
{
	// atlscrl.h を参考に
	size_t nScroll = (m_nWheelLines == WHEEL_PAGESCROLL) ? m_iRealPageBytes : 16;
	if (delta > 0) nScroll *= -1;

	m_zDelta += (int)delta;   // cumulative
	int zTotal = (m_nWheelLines == WHEEL_PAGESCROLL) ? abs(m_zDelta) : abs(m_zDelta) * m_nWheelLines;
	INT_PTR nMove = 0;
	for (int i = 0; i < zTotal; i += WHEEL_DELTA)
		nMove += nScroll;
	int nSteps = m_zDelta / WHEEL_DELTA;
	m_zDelta -= nSteps * WHEEL_DELTA;

	UINT_PTR posNew = m_hex.GetCurAddr();
	UINT_PTR posEnd = m_hex.GetEndAddr();
	size_t ofs = posNew - m_posTop;
	if (nMove > 0) {
		if (m_posTop + m_iRealPageBytes >= posEnd)
			return TRUE;

		posNew = m_posTop + nMove;
	} else {
		if (!m_posTop)
			return TRUE;

		nMove = -nMove;
		if (nMove > m_posTop)
			posNew = 0;
		else
			posNew = m_posTop - nMove;
	}
	if (posNew + ofs > posEnd)
		ofs = posEnd - posNew;

	m_hex.MoveTo(posNew + ofs, m_hex.m_posType);
	ResizeCaret();				// キャレットのサイズを変更

	JumpToPosition(posNew);
	return TRUE;
}

void CBeView::OnEditUndo()
{
	if (m_hex.DoUndo()) {
		SetupScrollBars();

		UINT_PTR posCur = m_hex.GetCurAddr();
		if (posCur < m_posTop ||
			posCur > m_posTop + m_iRealPageBytes) {
			SetTopIndex(posCur);
			JumpToPosition(ROWHEAD(posCur));
		}
		else
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		ResizeCaret(TRUE);

		m_pFrame->Modified(m_hex.IsModified());
	}
}

BOOL CBeView::CanPaste()
{
	return (!IsReadOnly() &&
			(IsClipboardFormatAvailable(gCF_BINARY) ||
			IsClipboardFormatAvailable(CF_TEXT) ||
			IsClipboardFormatAvailable(CF_OEMTEXT) ||
			IsClipboardFormatAvailable(CF_UNICODETEXT)));
}

void CBeView::CopyToClipboard(LPVOID pv, size_t size)
{
	if (pv && size) {
		HGLOBAL hmem = GlobalAlloc(GHND | GMEM_SHARE, size + sizeof(CLIPBINDATA));
		if (hmem) {
			BOOL bOK = FALSE;
			CLIPBINDATA* clip = (CLIPBINDATA*)GlobalLock(hmem);
			if (clip) {
				clip->datasize = (DWORD)size;
				memcpy(clip->data, pv, size);
				GlobalUnlock(hmem);

				if (OpenClipboard()) {
					if (EmptyClipboard())
						bOK = (SetClipboardData(gCF_BINARY, hmem) != NULL);
					CloseClipboard();
				}
			}
			if (!bOK)
				GlobalFree(hmem);
		}
	}
}

CLIPBINDATA* CBeView::CopyFromClipboard()
{
	CLIPBINDATA* clip = NULL;
	if (OpenClipboard()) {
		UINT fmt = 0;
		do {
			fmt = EnumClipboardFormats(fmt);
			if (fmt == gCF_BINARY ||
				fmt == CF_TEXT ||
				fmt == CF_OEMTEXT ||
				fmt == CF_UNICODETEXT)
				break;
		} while (fmt);

		if (fmt) {
			if (fmt != gCF_BINARY) {
				int r = AppMessage(IDS_PASTE_BYSJIS, NULL, MB_QUESTION);
				switch (r) {
				case IDYES: fmt = CF_TEXT; break;
				case IDNO: fmt = CF_UNICODETEXT; break;
				default: CloseClipboard(); return NULL;
				}
			}

			HGLOBAL hmem = GetClipboardData(fmt);
			if (hmem) {
				const CLIPBINDATA* pc = (const CLIPBINDATA*)GlobalLock(hmem);
				if (pc) {
					DWORD size;
					if (fmt == gCF_BINARY) {
						size = pc->datasize;
						clip = (CLIPBINDATA*)malloc(size + sizeof(CLIPBINDATA));
						clip->datasize = size;
						memcpy(clip->data, pc->data, size);
					}
					else if (fmt == CF_TEXT || fmt == CF_OEMTEXT) {
						size = lstrlenA((LPCSTR)pc);
						clip = (CLIPBINDATA*)malloc(size + sizeof(CLIPBINDATA));
						clip->datasize = size;
						memcpy(clip->data, pc, size);
					}
					else if (fmt == CF_UNICODETEXT) {
						size = lstrlen((LPCWSTR)pc) * sizeof(WCHAR);
						clip = (CLIPBINDATA*)malloc(size + sizeof(CLIPBINDATA));
						clip->datasize = size;
						memcpy(clip->data, pc, size);
					}
					GlobalUnlock(hmem);
				}
			}
		}
		CloseClipboard();
	}
	return clip;
}

void CBeView::JumpToOffset(INT_PTR pos)
{
	if (pos < 0 || (UINT_PTR)pos >= m_hex.GetTotalSize()) {
		MessageBeep(0);
		return;
	}

	m_hex.MoveTo((UINT_PTR)pos, SST_HIBYTE);
	ResizeCaret();

	CRange disp(m_posTop, m_posTop + m_iRealPageBytes);
	if (disp.InRange(pos)) {
		// 現在の画面内ならば再描画のみ
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else {
		// カーソルが画面中央に来るように補正
		pos -= m_iRealPageBytes / 2;
		pos = max(0, pos);
		JumpToPosition(ROWHEAD((UINT_PTR)pos));
	}
}

void CBeView::OnJumpOffset()
{
	CJumpDlg dlg;

	// 選択範囲ありの場合、そのデータを取り込む
	if (m_hex.IsSelected()) {
		CRange sel = m_hex.GetSelRange();
		sel.Normalize();
		int len = sel.Range() + 1;
		if (len > 0) {
			size_t size;
			BYTE* pb = m_hex.ReadSelectedRange(&size);
			if (pb) {
				if (size <= 4) {
					BYTE ship[16];
					memset(ship, 0, sizeof(ship));
					memcpy(ship, pb, size);
					switch (size) {
					case 1:
						dlg.m_offset= *ship;
						dlg.m_strOffset.Format(_T("%02X"), dlg.m_offset);
						break;
					case 2:
						dlg.m_offset= *(WORD*)ship;
						dlg.m_strOffset.Format(_T("%04X"), dlg.m_offset);
						break;
					case 3:
					case 4:
						dlg.m_offset= *(DWORD*)ship;
						dlg.m_strOffset.Format(_T("%08X"), dlg.m_offset);
						break;
					}
				}
				FREE(pb);
			}
		}
	}

	if (dlg.DoModal() == IDOK) {
		INT_PTR pos;
		if (dlg.m_origin == SEEK_SET)
			pos = dlg.m_offset;
		else
			pos = m_hex.GetCurAddr() + dlg.m_offset;

		JumpToOffset(pos);
	}
}

void CBeView::OnJumpFileTop() 
{
	if (m_hex.GetCurAddr() > 0) {
		UINT type = m_hex.m_posType;
		if (type != SST_TEXT)
			type = SST_HIBYTE;

		m_hex.MoveTo(0, type);
		ResizeCaret();

		if (m_posTop > 0)
			JumpToPosition(0);
		else
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CBeView::OnJumpFileEnd() 
{
	UINT_PTR pos = m_hex.GetEndAddr();
	if (pos > 0 && m_hex.GetCurAddr() < pos - 1) {
		pos--;
		UINT type = m_hex.m_posType;
		if (type != SST_TEXT)
			type = SST_HIBYTE;

		m_hex.MoveTo(pos, type);
		ResizeCaret();

		if (m_posTop + m_iRealPageBytes < pos) {
			pos = ROWHEAD(pos - m_iRealPageBytes);
			if (pos >= 16)
				pos += 16;
			JumpToPosition(pos);
		}
		else
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CBeView::OnJumpLastEdit()
{
	if (m_hex.CanUndo()) {
		UINT_PTR pos = m_hex.GetLastEditPos();
		JumpToOffset(pos);
	}
}

void CBeView::OnEnvironSetup() 
{
	CColorEditDialog dlg;
	::GetObject(m_font, sizeof(LOGFONT), &dlg.m_lf);

	memcpy(&dlg.m_vc, &m_vc, sizeof(VIEWCOLOR));
	if (dlg.DoModal() == IDOK) {
		memcpy(&m_vc, &dlg.m_vc, sizeof(VIEWCOLOR));

		if (m_font.m_hFont) m_font.DeleteObject();
		m_font.CreateFontIndirect(&dlg.m_lf);
		OnUpdate(UVM_RESETPARAMS, NULL);
	}
}

void CBeView::RegWrite()
{
	CAppReg reg;

	// フォントの情報を書き込む
	LOGFONT lf;
	::GetObject(m_font, sizeof(LOGFONT), &lf);
	reg.SetBinary(IDS_SEC_STATUS, IDS_KEY_FONT, &lf, sizeof(lf));

	// カラー情報を書き込む
	reg.SetBinary(IDS_SEC_STATUS, IDS_KEY_COLORS, &m_vc, sizeof(VIEWCOLOR));

	// テキストタイプ
	reg.SetValue(IDS_SEC_STATUS, IDS_KEY_TEXTTYPE, m_iTextType);
}

void CBeView::RegRead()
{
	CAppReg reg;

	// フォントの情報を読み込む
	LOGFONT lf;
	if (reg.GetBinary(IDS_SEC_STATUS, IDS_KEY_FONT, &lf, sizeof(lf)) != sizeof(lf)) {
		HFONT hf = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
		GetObject(hf, sizeof(LOGFONT), &lf);
	}
	if (m_font.m_hFont) m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf);

	// カラー情報
	VIEWCOLOR vc;
	if (reg.GetBinary(IDS_SEC_STATUS, IDS_KEY_COLORS, &vc, sizeof(vc)) == sizeof(vc))
		m_vc = vc;

	// テキストタイプ
	m_iTextType = reg.GetValue(IDS_SEC_STATUS, IDS_KEY_TEXTTYPE, CHAR_ASCII);
}

void CBeView::OnTextAscii() 
{
	if (m_iTextType != CHAR_ASCII) {
		m_iTextType = CHAR_ASCII;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CBeView::OnTextSjis() 
{
	if (m_iTextType != CHAR_SJIS) {
		m_iTextType = CHAR_SJIS;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CBeView::OnMarkAddress()
{
	UINT_PTR pos = m_hex.GetCurAddr();
	CRange src;
	if (m_hex.IsSelected()) {
		src = m_hex.GetSelRange();
		src.Normalize();
		if (src.Range() >= 0x100) {
			AppMessage(IDS_MARK_RANGELIMIT);
			return;
		}
	}
	else
		src.SetRange(pos, pos);

	size_t index;
	CRange r = m_marker.Find(pos, &index);
	if (r.IsEmpty()) {
		m_marker.Add(src);
		m_marker.Sort();
	}
	else {
		m_marker.RemoveAt(index);
		src = r;
	}

	// マーカー領域を再描画
	CRect rc;
	GetClientRect(&rc);
	rc.top += m_yChar;
	long top = ((ROWHEAD(src.from) - m_posTop) >> 4) * m_yChar;
	if (top < 0) top = 0;
	long bottom = ((ROWHEAD(src.to) - m_posTop) >> 4) * m_yChar + m_yChar;
	rc.bottom = min(rc.bottom, rc.top + bottom);
	rc.top += top;
	RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CBeView::OnMarkClearAll()
{
	if (m_marker.GetCount() > 0) {
		m_marker.RemoveAll();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CBeView::OnMarkPrev()
{
	UINT_PTR pos = m_hex.GetCurAddr();
	CRange src = m_marker.FindPrev(pos);
	if (!src.IsEmpty())
		JumpToOffset(src.from);
	else
		MessageBeep(0);
}

void CBeView::OnMarkNext()
{
	UINT_PTR pos = m_hex.GetCurAddr();
	CRange src = m_marker.FindNext(pos);
	if (!src.IsEmpty())
		JumpToOffset(src.from);
	else
		MessageBeep(0);
}

void CBeView::OnEditSelToPrevMark()
{
	// 現在のカーソル位置
	UINT_PTR pos = m_hex.GetCurAddr();
	if (m_hex.IsSelected()) {
		// 選択状態なら、選択範囲のけつから
		CRange sel = m_hex.GetSelRange();
		pos = sel.Max();
	}

	CRange src = m_marker.FindPrev(pos);
	if (!src.IsEmpty()) {
		m_hex.Select(pos, src.from);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
		MessageBeep(0);
}

void CBeView::OnEditSelToNextMark()
{
	// 現在のカーソル位置
	UINT_PTR pos = m_hex.GetCurAddr();
	if (m_hex.IsSelected()) {
		// 選択状態なら、選択範囲の頭から
		CRange sel = m_hex.GetSelRange();
		pos = sel.Min();
	}

	CRange src = m_marker.FindNext(pos);
	if (!src.IsEmpty()) {
		m_hex.Select(pos, src.to);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
		MessageBeep(0);
}

UINT_PTR CBeView::GetCurrentAddress(size_t* pnTotal)
{
	if (pnTotal) *pnTotal = m_hex.GetTotalSize() - 1;
	return m_hex.GetCurAddr();
}

void CBeView::OnToolCheckSum()
{
	size_t size;
	LPBYTE pb = m_hex.ReadSelectedRange(&size);
	if (pb) {
		CCheckSumDlg dlg;
		// checksum
		int i;
		CString str;
		unsigned long ul = 0;
		for (i = 0; i < size; i++) ul += pb[i];
		str.Format(_T("%02X H"), ul & 0xff);
		dlg.m_sum.Add(_T("checksum"), str);
		// crc16
		str.Format(_T("%04X H"), lccCalcCrc16(pb, size));
		dlg.m_sum.Add(_T("crc16"), str);
		// crc32
		str.Format(_T("%08X H"), lccCalcCrc32(pb, size));
		dlg.m_sum.Add(_T("crc32"), str);
		// md5
		BYTE sum[64];
		lccCalcMD5(pb, size, sum);
		str.Empty();
		for (i = 0; i < 16; i++) str.AppendFormat(_T("%02x"), sum[i]);
		dlg.m_sum.Add(_T("md5"), str);
		// sha1
		lccCalcSHA1(pb, size, sum);
		str.Empty();
		for (i = 0; i < 20; i++) str.AppendFormat(_T("%02x"), sum[i]);
		dlg.m_sum.Add(_T("sha1"), str);
		// sha256
		lccCalcSHA256(pb, size, sum);
		str.Empty();
		for (i = 0; i < 32; i++) str.AppendFormat(_T("%02x"), sum[i]);
		dlg.m_sum.Add(_T("sha256"), str);

		FREE(pb);

		dlg.DoModal();
	}
}

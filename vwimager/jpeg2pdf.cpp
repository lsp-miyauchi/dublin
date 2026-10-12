//////////////////////////////////////////////////////////////////////
// converter.cpp

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "dropexec.h"
#include "jpeg2pdf.h"

////////////////////////////////////////////////////////////////////////
//

static const COMDLG_FILTERSPEC cfSpec[] = { { L"PDF(*.pdf)", L"*.pdf" }, };
static const TCHAR* extPdf = _T(".pdf");

static void Jpeg2Pdf(HWND hwnd, HDROP hdrop)
{
	CSimpleArray<CString> arr;
	int i, cnt = DragQueryFile(hdrop, 0xffffffff, NULL, 0);
	if (cnt > 0) {
		TCHAR buf[MAX_PATH * 2];
		for (i = 0; i < cnt; i++) {
			DragQueryFile(hdrop, i, buf, ARRAYSIZE(buf));
			if (imgCheckImageType(buf) == IMG_JPEG)
				arr.Add(buf);
		}
		cnt = arr.GetSize();
	}

	if (cnt < 1) {
		AppMessage(IDR_VIEW, IDS_NO_JPEGFILE, NULL, MB_STOP);
		return;
	}
	else if (cnt > 256) {
		AppMessage(IDR_VIEW, IDS_MAX_JPEGFILE, NULL, MB_STOP);
		return;
	}

	TCHAR path[MAX_PATH * 2];
	lstrcpy(path, arr[0].GetString());
	PathRemoveFileSpec(path);
	CString str, strFile;
	str.LoadString(IDS_TITLE_JPEG2PDF);
	if (ShowOpenSaveFileDialog(hwnd, false, str, path, strFile, cfSpec, ARRAYSIZE(cfSpec))) {
		if (lstrcmpi(PathFindExtension(strFile.GetString()), extPdf))
			strFile += extPdf;
		// 進捗ダイアログ
		IProgressDialog* ppd;
		HRESULT hr = CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (LPVOID*)&ppd);
		if (SUCCEEDED(hr)) {
			CString str;
			str.LoadString(IDS_TITLE_JPEG2PDF);
			ppd->SetTitle(str);
			str.LoadString(IDS_PROGRESS_JPEG2PDF);
			ppd->SetLine(1, str, FALSE, NULL);
			ppd->SetAnimation(GetModuleHandle(DLL_FOPSRV), 165);
			ppd->StartProgressDialog(hwnd, NULL, PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR, NULL);
		}
		else
			ppd = NULL;

		CJpeg2Pdf jp;
//		jp.BeginDocument(8.5, 11.0);			// Letter is 8.5x11 inch
		jp.BeginDocument(8.268, 11.693);		// A4
		for (i = 0; i < cnt; i++) {
			if (ppd)
				ppd->SetLine(2, PathFindFileName(arr[i].GetString()), FALSE, NULL);
			jp.InsertJPEG(arr[i].GetString());
		}
		jp.EndDocument();
		HANDLE hf = CreateFile(strFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			jp.GetFinalDocument(hf);
			CloseHandle(hf);
		}

		if (ppd) {
			ppd->StopProgressDialog();
			ppd->Release();
		}
	}
}

extern "C"
void WINAPI RunJpeg2PdfW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	CRegExec re;
	if (re.Initialize(lpCmdLine) && re.m_hmem)
		Jpeg2Pdf(hwnd, (HDROP)re.m_hmem);
}

extern "C"
void WINAPI RunJpeg2Pdf(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR cmd[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, _COUNTOF(cmd));
	RunJpeg2PdfW(hwnd, hInst, cmd, nCmdShow);
}



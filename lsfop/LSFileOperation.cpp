// LSFileOperation.cpp : CLSFileOperation �̎���

#include "stdafx.h"
#include "LSFileOperation.h"
#include "fopstd.h"
#include "taskjob.h"

// CLSFileOperation

STDMETHODIMP CLSFileOperation::FileCopy(HWND hwnd, LPCWSTR pszSrc, LPCWSTR pszDst, BOOL bCopy)
{
	HRESULT hr = (CopyOrMoveFile(hwnd, pszSrc, pszDst, bCopy) != FALSE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CLSFileOperation::FileCopyEx(LPARAM param)
{
	HRESULT hr = (CopyOrMoveFileEx((COPYORMOVEINFOSTRUCT*)param) != FALSE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CLSFileOperation::DropFileOperation(HWND hwnd, HGLOBAL hdrop, DWORD eff, LPCWSTR pszDst, BOOL bSaveFolderHierarchy)
{
	HRESULT hr = (ExecDropFileOperation(hwnd, (HDROP)hdrop, eff, pszDst, (bSaveFolderHierarchy != 0)) != FALSE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CLSFileOperation::DropFileOperationEx(LPARAM param)
{
	HRESULT hr = (ExecDropFileOperationEx((DROPFILEOPSTRUCT*)param) != FALSE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CLSFileOperation::MakeTree(LPCWSTR pszDir)
{
	HRESULT hr = (fopMakeTree(pszDir) != FALSE) ? S_OK : E_FAIL;
	return hr;
}

STDMETHODIMP CLSFileOperation::MakeTaskJob(HWND hwnd, LPCWSTR pszSyncFile)
{
	return SetupFolderSyncTaskJob(hwnd, pszSyncFile);
}

STDMETHODIMP CLSFileOperation::MakeSymLinkUI(HWND hwnd, LPCWSTR lpSymlinkBase, LPCWSTR lpTargetFileName)
{
	BOOL ok = fopCreateSymbolicLinkUI(hwnd, lpSymlinkBase, lpTargetFileName);
	return (ok ? S_OK : E_FAIL);
}

STDMETHODIMP CLSFileOperation::MakeSymLink(HWND hwnd, LPCWSTR lpTarget, LPCWSTR lpSrc)
{
	return CreateSymbolicLink(lpTarget, lpSrc, _pathisdirectory(lpSrc) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
}


//////////////////////////////////////////////////////////////////////
// shellexec.h

#pragma once

BOOL __fastcall InvokeFile(HWND hParent, LPCTSTR pszFile, LPCTSTR pszParam = NULL, LPCTSTR pszDir = NULL, LPCTSTR lpVerb = NULL);
BOOL __fastcall InvokeIDList(HWND hParent, LPCITEMIDLIST pidl, LPCTSTR lpVerb = NULL);
BOOL __fastcall InvokeDefaultMenuCommand(HWND hwnd, IShellFolder* psf, LPITEMIDLIST pidl);
BOOL __fastcall ctx_Execute(HWND hwnd, IContextMenu* pcm, LPCTSTR pVerb = NULL);
BOOL __fastcall SendTo(HWND hParent, IDataObject* pdt, CPoint pt);
BOOL __fastcall ShellNewMenu(HWND hwnd, LPITEMIDLIST pidlFolder, LPPOINT ppt);

//////////////////////////////////////////////////////////////////////
// spin_sup.h

_inline UINT Spin_GetAccel(HWND hctl, int nAcc, UDACCEL* pAcc)
	{ return (UINT)LOWORD(SendMessage(hctl, UDM_GETACCEL, nAcc, (LPARAM)pAcc)); }
_inline UINT Spin_GetBase(HWND hctl)
	{ return (UINT)LOWORD(SendMessage(hctl, UDM_GETBASE, 0, 0l)); }
_inline HWND Spin_GetBuddy(HWND hctl)
	{ return (HWND)SendMessage(hctl, UDM_GETBUDDY, 0, 0l); }
_inline int Spin_GetPos(HWND hctl)
	{ return (int)SendMessage(hctl, UDM_GETPOS, 0, 0l); }
_inline DWORD Spin_GetRange(HWND hctl)
	{ return (DWORD)SendMessage(hctl, UDM_GETRANGE, 0, 0l); }
_inline BOOL Spin_SetAccel(HWND hctl, int nAcc, UDACCEL* pAcc)
	{ return (BOOL)LOWORD(SendMessage(hctl, UDM_SETACCEL, nAcc, (LPARAM)pAcc)); }
_inline int Spin_SetBase(HWND hctl, int nBase)
	{ return (int)SendMessage(hctl, UDM_SETBASE, nBase, 0L); }
_inline HWND Spin_SetBuddy(HWND hctl, HWND hBuddy)
	{ return (HWND)SendMessage(hctl, UDM_SETBUDDY, (WPARAM)hBuddy, 0L); }
_inline int Spin_SetPos(HWND hctl, int nPos)
	{ return (int)(short)LOWORD(SendMessage(hctl, UDM_SETPOS, 0, nPos)); }
_inline void Spin_SetRange(HWND hctl, int nLower, int nUpper)
	{ SendMessage(hctl, UDM_SETRANGE, 0, MAKELPARAM(nUpper, nLower)); }


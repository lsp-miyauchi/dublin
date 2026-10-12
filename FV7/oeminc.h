// oeminc.h : header file
//


#define APP_COMPANY		(_T("LightShip"))
#define APP_PRODUCT		(_T("FileVisor7"))
#define APP_EXENAME		(_T("fv7.exe"))
#define APP_APPID		(L"LightShip.FileVisor7")
#define REGPREFIX		0x3c

#ifndef _WIN64

#define APP_FRAMECLASS	(_T("FV7x86Frame"))
#define APP_TRAYCLASS	(_T("FV7x86Tray"))
#define RWM_REGCHECK1ST	(_T("f7x86q23gseebb"))
#define RWM_REGDIALOG	(_T("f7x86r67gsqakl"))
#define RWM_REGTHANKS	(_T("f7x86q23gsernf"))
#define RWM_REGOK		(_T("f7x8654hrajfsl"))
#define RWM_TRAYSERVICE	(_T("f7x86zsdfgnoss"))

#else

#define APP_FRAMECLASS	(_T("FV7x64Frame"))
#define APP_TRAYCLASS	(_T("FV7x64Tray"))
#define RWM_REGCHECK1ST	(_T("f7x641stsdfgbb"))
#define RWM_REGDIALOG	(_T("f7x64dlgggpose"))
#define RWM_REGTHANKS	(_T("f7x64thxmarueq"))
#define RWM_REGOK		(_T("f7x64suyegsfsl"))
#define RWM_TRAYSERVICE	(_T("f7x64sxzsdoiaw"))

#endif // !_WIN64

#define DLL_CORE		(_T("lscore.dll"))
#define DLL_FOPSRV		(_T("lsfop.dll"))
#define DLL_MEDIA		(_T("lsmedia.dll"))
#define DLL_EXIF		(_T("lsexif.dll"))
#define DLL_IMAGE		(_T("lsimg.dll"))
#define DLL_MIL			(_T("vwtxtedit.dll"))
	#define MIL_EDIT		(_T("RunEdit"))
	#define MIL_GREP		(_T("RunGrep"))
#define DLL_BED			(_T("vwhexedit.dll"))
	#define BED_EDIT		(_T("RunEdit"))
#define DLL_GMIL		(_T("vwimager.dll"))
#define DLL_ARC			(_T("vwarchive.dll"))
	#define ARC_VIEW		(_T("RunView"))
#define DLL_FINDER		(_T("vwfinder.dll"))

#define RGI_USERINFO	(_T("UserInfo"))
#define RGI_LICENSE		(_T("Regist#"))

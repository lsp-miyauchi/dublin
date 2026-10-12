// dllmain.h : モジュール クラスの宣言

class ClsarcsvrModule : public CAtlDllModuleT< ClsarcsvrModule >
{
public :
	DECLARE_LIBID(LIBID_lsarcsvrLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_LSARCSVR, "{A9685706-B5B2-47BE-B16C-5863C02452A1}")
};

extern class ClsarcsvrModule _AtlModule;

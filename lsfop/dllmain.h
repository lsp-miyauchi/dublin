// dllmain.h : モジュール クラスの宣言

class CLsFopModule : public CAtlDllModuleT< CLsFopModule >
{
public :
	DECLARE_LIBID(LIBID_lsfopLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_LSFOP, "{464F3799-0F36-47A7-972D-988DF98D1B0F}")
};

extern class CLsFopModule _AtlModule;

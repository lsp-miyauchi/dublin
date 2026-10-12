// dllmain.h : モジュール クラスの宣言

class ClsftcModule : public CAtlDllModuleT< ClsftcModule >
{
public :
	DECLARE_LIBID(LIBID_lsftcLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_LSFTC, "{674ED17F-7245-4D0C-9A73-159C87455372}")
};

extern class ClsftcModule _AtlModule;

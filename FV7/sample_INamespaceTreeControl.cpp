#include <Shlobj.h>
#include <Shlwapi.h>
#pragma comment (lib, "shlwapi.lib")
 
 
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static INameSpaceTreeControl *pNameSpaceTreeControl;
    static HWND hwndTree;
  
    switch(msg){
        case WM_CREATE:{
            //OLEを初期化
            OleInitialize(NULL);
  
            //INameSpaceTreeControlを取得
            HRESULT hr;
            hr = CoCreateInstance(CLSID_NamespaceTreeControl, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pNameSpaceTreeControl));
            if(FAILED(hr))
                return -1;
  
            //INameSpaceTreeControlを初期化
            hr = pNameSpaceTreeControl->Initialize(hwnd, NULL, NSTCS_HASEXPANDOS | NSTCS_SINGLECLICKEXPAND);
            if(FAILED(hr))
                return -1;
     
                 
            //ツリーのルートを指定
            IShellItem *pShellItem;
            SHCreateItemInKnownFolder(FOLDERID_Desktop, 0, NULL, IID_PPV_ARGS(&pShellItem));
            pNameSpaceTreeControl->AppendRoot(pShellItem, SHCONTF_FOLDERS, NSTCRS_VISIBLE | NSTCRS_EXPANDED, NULL);
            pShellItem->Release();
                 
                 
            //INameSpaceTreeControlのウィンドウハンドルを取得
            IUnknown_GetWindow(pNameSpaceTreeControl, &hwndTree);
            }break;
        case WM_SIZE:
            MoveWindow(hwndTree, 0, 0, LOWORD(lp), HIWORD(lp), TRUE);
            break;
        case WM_DESTROY:
            //INameSpaceTreeControlを解放
            pNameSpaceTreeControl->Release();
  
            //OLEを開放
            OleUninitialize();
  
            PostQuitMessage(0);
            break;
        default:
            return (DefWindowProc(hwnd, msg, wp, lp));
    }
    return 0;
}

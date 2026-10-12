/*
UnLha64x.dll は、4GBを超える大容量ファイルに対応するため、内部のサイズ管理メンバが 64bit 整数型（DWORDLONG または ULONGLONG）
に拡張された EXTRACTINGINFO64 構造体を提供しています。
これにあわせて、解凍進捗メッセージも WM_ARCEXTRACT ではなく、64bit構造体のアドレスを載せて通知される WM_ARCEXTRACT64 を
使用する必要があります。
以下に、64bit環境（x64ターゲットビルド）へ完全対応させたWTLダイアログのソースコードを提示します。
*/
/*
 64bit完全対応化における変更ポイントメッセージIDの変更: WM_ARCEXTRACT (WM_USER + 22) から、
 64bit対応版である WM_ARCEXTRACT64 (WM_USER + 38) へマッピングを変更しています。
 これをしないと、UnLha64x.dll から進捗メッセージが通知されません。
 データ型の拡張: DWORD (32bit) から DWORDLONG (64bit) に変わったことで、
 4GB（4,294,967,296 バイト）を超えるファイルサイズや書き込みバイト数を正確に保持・計算できるようになっています。
 計算時の安全設計: 32bit整数型の場合、ファイルサイズに 100 を掛け算した時点で数値が溢れて
 進捗バーが異常な挙動を起こすことがありましたが、DWORDLONG であれば数十テラバイトのファイルでも
 オーバーフローすることなく安全に進捗率（%）が計算できます。
 Visual Studio でのビルドに関する注意このコードを動作させるには、Visual Studio などのビルド構成（プラットフォーム）を
 Win32 (x86) ではなく x64 に設定してビルドしてください。
*/

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlwin.h>
#include <atlstr.h> // CString用
#include <thread>
#include <string>
#include <atomic>

// --- UnLha64x.dll 用の 64bit & 大容量拡張定義 ---
#define WM_ARCEXTRACT64 (WM_USER + 38) 

// Unicode環境のため、ファイル名保持メンバを wchar_t にした構造体を定義
typedef struct {
    DWORDLONG dwlFileSize;       // 64bit整数：展開後の総サイズ
    DWORDLONG dwlWriteSize;      // 64bit整数：展開済みのサイズ
    wchar_t   szSourceFileName[MAX_PATH]; // ✨ Unicode(W)対応のファイル名格納域
    wchar_t   szDestFileName[MAX_PATH];   // ✨ Unicode(W)対応のファイル名格納域
} EXTRACTINGINFO64W, * LPEXTRACTINGINFO64W;

// ✨ 関数ポインタ型を Unicode(W) 仕様に変更
typedef int (WINAPI* LPFNUnlhaW)(const HWND, LPCWSTR, LPWSTR, const DWORD);
typedef BOOL(WINAPI* LPFNUnlhaSetOwnerWindow)(HWND);
typedef BOOL(WINAPI* LPFNUnlhaClearOwnerWindow)();

// アプリケーション固有のカスタムメッセージ
#define WM_APP_EXTRACT_COMPLETE (WM_APP + 1)

// スレッドから結果を戻すための構造体（フォールバック時の情報伝達用）
struct ExtractResult {
    BOOL bSuccess;       // 成功可否
    int nErrorCode;      // DLLの戻り値、または独自定義のエラーコード
    CString strMessage;  // 画面に表示するメッセージ
};

// 独自エラーコード定義
#define ERR_DLL_NOT_FOUND     -101
#define ERR_API_NOT_FOUND     -102

// --- WTL ダイアログクラス ---
class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
    enum { IDD = IDD_MAINDIALOG };

    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_ARCEXTRACT64, OnArcExtract64)
        MESSAGE_HANDLER(WM_APP_EXTRACT_COMPLETE, OnExtractComplete)
        COMMAND_ID_HANDLER(IDC_START_BTN, OnStartExtract)
        COMMAND_ID_HANDLER(IDC_CANCEL_BTN, OnCancelExtract)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

private:
    CProgressBarCtrl m_progressBar;
    CButton m_btnStart;
    CButton m_btnCancel;

    std::atomic<bool> m_bIsExtracting{ false };
    std::atomic<bool> m_bCancelRequested{ false };

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_progressBar.Attach(GetDlgItem(IDC_PROGRESS));
        m_progressBar.SetRange(0, 100);
        m_progressBar.SetPos(0);

        m_btnStart.Attach(GetDlgItem(IDC_START_BTN));
        m_btnCancel.Attach(GetDlgItem(IDC_CANCEL_BTN));
        m_btnCancel.EnableWindow(FALSE);
        return TRUE;
    }

    LRESULT OnStartExtract(WORD, WORD, HWND, BOOL&)
    {
        if (m_bIsExtracting) return 0;

        m_bIsExtracting = true;
        m_bCancelRequested = false;

        m_btnStart.EnableWindow(FALSE);
        m_btnCancel.EnableWindow(TRUE);
        m_progressBar.SetPos(0);

        // バックグラウンドスレッドの起動
        std::thread extractThread(&CMainDlg::ExtractThreadProc, this, m_hWnd);
        extractThread.detach();

        return 0;
    }

    LRESULT OnCancelExtract(WORD, WORD, HWND, BOOL&)
    {
        if (m_bIsExtracting)
        {
            m_bCancelRequested = true;
            m_btnCancel.EnableWindow(FALSE);
        }
        return 0;
    }

    // 🛠️ バックグラウンドで実行される解凍スレッド
    void ExtractThreadProc(HWND hWndUI)
    {
        // 結果通知用の動的メモリ確保（UIスレッド側で delete します）
        ExtractResult* pResult = new ExtractResult{ FALSE, 0, _T("") };

        // 🛡️ 安全なフォールバック処理1: DLLのロード確認
        HMODULE hDll = ::LoadLibrary(TEXT("UnLha64x.dll"));
        if (!hDll)
        {
            pResult->bSuccess = FALSE;
            pResult->nErrorCode = ERR_DLL_NOT_FOUND;
            pResult->strMessage = _T("システムに 'UnLha64x.dll' が見つからないか、読み込めません。\n同じフォルダーに配置されているか確認してください。");

            // 安全にUIへフォールバック通知
            ::PostMessage(hWndUI, WM_APP_EXTRACT_COMPLETE, 0, (LPARAM)pResult);
            return;
        }

        // 🛡️ 安全なフォールバック処理2: Unicode用APIのアドレス取得
        // ※ UnLha64x は UNLHA32 互換のため、Unicode版関数名は "UnlhaW" となります
        LPFNUnlhaW pUnlhaW = (LPFNUnlhaW)::GetProcAddress(hDll, "UnlhaW");
        LPFNUnlhaSetOwnerWindow pSetOwner = (LPFNUnlhaSetOwnerWindow)::GetProcAddress(hDll, "UnlhaSetOwnerWindow");
        LPFNUnlhaClearOwnerWindow pClearOwner = (LPFNUnlhaClearOwnerWindow)::GetProcAddress(hDll, "UnlhaClearOwnerWindow");

        if (!pUnlhaW || !pSetOwner || !pClearOwner)
        {
            pResult->bSuccess = FALSE;
            pResult->nErrorCode = ERR_API_NOT_FOUND;
            pResult->strMessage = _T("DLLはロードできましたが、必要なAPI(UnlhaW等)の取得に失敗しました。\nDLLのバージョンが古い可能性があります。");

            ::FreeLibrary(hDll);
            ::PostMessage(hWndUI, WM_APP_EXTRACT_COMPLETE, 0, (LPARAM)pResult);
            return;
        }

        // --- ここから正常ルートとしての解凍処理 ---
        pSetOwner(hWndUI);

        // ✨ C++標準の std::wstring (Unicode) を用いたパス指定
        std::wstring cmd = L"x \"C:\\ユーザー\\巨大アーカイブ.lzh\" \"D:\\展開先フォルダー\\\"";

        // エラーログやメッセージ出力用のバッファ（wchar_t版）
        wchar_t szOutputW[2048] = { 0 };

        // 💡 64bit・Unicode版APIでのブロッキング実行
        int nDllRet = pUnlhaW(hWndUI, cmd.c_str(), szOutputW, _countof(szOutputW));

        pClearOwner();
        ::FreeLibrary(hDll);

        // 結果の設定
        if (nDllRet == 0)
        {
            pResult->bSuccess = TRUE;
            pResult->strMessage = _T("巨大ファイルの解凍が正常に完了しました。");
        }
        else
        {
            pResult->bSuccess = FALSE;
            pResult->nErrorCode = nDllRet;
            // DLLから返ってきたエラーバッファの内容をメッセージ化
            pResult->strMessage.Format(_T("解凍中にエラーが発生しました。\nコード: %d\n詳細: %s"), nDllRet, szOutputW);
        }

        // UIスレッドへの通知
        ::PostMessage(hWndUI, WM_APP_EXTRACT_COMPLETE, 0, (LPARAM)pResult);
    }

    // 🔥 UIスレッド：64bit・Unicode対応の進捗メッセージ処理
    LRESULT OnArcExtract64(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
    {
        if (m_bCancelRequested)
        {
            return 1; // 1を返してアボート
        }

        // ✨ キャスト先を LPEXTRACTINGINFO64W に変更
        LPEXTRACTINGINFO64W pInfo = (LPEXTRACTINGINFO64W)lParam;
        if (pInfo && pInfo->dwlFileSize > 0)
        {
            int progress = static_cast<int>((pInfo->dwlWriteSize * 100) / pInfo->dwlFileSize);
            m_progressBar.SetPos(progress);

            // 💡 必要であれば、Unicode対応した現在のファイル名をそのまま扱えます
            // 処理が重くなるため、毎回UIを書き換える場合は注意が必要
            // CString strCurrentFile = pInfo->szSourceFileName; 
        }

        return 0; // 続行
    }

    // UIスレッド：処理完了およびフォールバック結果の受け取り
    LRESULT OnExtractComplete(UINT, WPARAM, LPARAM lParam, BOOL&)
    {
        // 渡されたポインタを復元
        ExtractResult* pResult = (ExtractResult*)lParam;

        // UI状態のリセット
        m_bIsExtracting = false;
        m_btnStart.EnableWindow(TRUE);
        m_btnCancel.EnableWindow(FALSE);

        if (pResult)
        {
            if (pResult->bSuccess)
            {
                m_progressBar.SetPos(100);
                ::MessageBox(m_hWnd, pResult->strMessage, _T("完了"), MB_OK | MB_ICONINFORMATION);
            }
            else
            {
                if (m_bCancelRequested && pResult->nErrorCode != ERR_DLL_NOT_FOUND && pResult->nErrorCode != ERR_API_NOT_FOUND)
                {
                    m_progressBar.SetPos(0);
                    ::MessageBox(m_hWnd, _T("ユーザーによって解凍が中断されました。"), _T("キャンセル"), MB_OK | MB_ICONEXCLAMATION);
                }
                else
                {
                    // 🛡️ フォールバックエラー、またはDLL起因のエラーメッセージをダイアログ表示
                    m_progressBar.SetPos(0);
                    ::MessageBox(m_hWnd, pResult->strMessage, _T("エラーが発生しました"), MB_OK | MB_ICONERROR);
                }
            }

            // スレッド側で new したメモリをUIスレッド側で確実に解放
            delete pResult;
        }

        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        if (m_bIsExtracting)
        {
            ::MessageBox(m_hWnd, _T("解凍処理中は終了できません。"), _T("確認"), MB_OK | MB_ICONWARNING);
            return 0;
        }
        EndDialog(wID);
        return 0;
    }
};

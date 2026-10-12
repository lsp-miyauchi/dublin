/*
ユーザーの途中キャンセルへの対応、および解凍完了やエラー発生時の状態変化を安全にUIスレッドで処理する、
実戦仕様の完全なWTLサンプルコードにブラッシュアップしました。
Windowsプログラミングの原則である「UI操作は必ずUIスレッドで行う」を徹底するため、
スレッド完了通知やボタン復帰にはカスタムメッセージ（WM_APP_...）を使用しています。
*/

/*
ブラッシュアップのポイントアボート（中断）の仕組み: DLLの仕様上、
WM_ARCEXTRACT メッセージの戻り値として 1 を返すと解凍処理が中断されます [1]。
ボタンが押されたら m_bCancelRequested フラグを立て、次の進捗メッセージが来た瞬間に 1 を返して
安全に終了させています。
アトミック（std::atomic）の採用: メインスレッド（UI）と作業スレッドの間で状態を共有するフラグには、
スレッド間の競合（データレース）を防ぐために std::atomic<bool> を使用しています。
カスタムメッセージによるUI同期: スレッドが終了した際、直接スレッドから MessageBox を出したりコントロールを触るのではなく、
PostMessage を使って WM_APP_EXTRACT_COMPLETE をUIスレッドに投げています。
これにより、WindowsのUIスレッドセーフの原則を完全に満たしています。
*/

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlwin.h>
#include <thread>
#include <string>
#include <atomic>

// --- UNLHA32.DLL 関連の定義 ---
#define WM_ARCEXTRACT (WM_USER + 22)

typedef struct {
    DWORD dwFileSize;
    DWORD dwWriteSize;
    char  szSourceFileName[MAX_PATH];
    char  szDestFileName[MAX_PATH];
} EXTRACTINGINFO, * LPEXTRACTINGINFO;

typedef int (WINAPI* LPFNUnlha)(const HWND, LPCSTR, LPSTR, const DWORD);
typedef BOOL(WINAPI* LPFNUnlhaSetOwnerWindow)(HWND);
typedef BOOL(WINAPI* LPFNUnlhaClearOwnerWindow)();

// --- アプリケーション固有のカスタムメッセージ ---
#define WM_APP_EXTRACT_COMPLETE (WM_APP + 1) // 解凍完了・エラー通知用

// --- WTL ダイアログクラス ---
class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
    enum { IDD = IDD_MAINDIALOG };

    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_ARCEXTRACT, OnArcExtract)         // DLLからの進捗メッセージ
        MESSAGE_HANDLER(WM_APP_EXTRACT_COMPLETE, OnExtractComplete) // スレッドからの完了メッセージ
        COMMAND_ID_HANDLER(IDC_START_BTN, OnStartExtract)   // 開始ボタン
        COMMAND_ID_HANDLER(IDC_CANCEL_BTN, OnCancelExtract) // キャンセルボタン
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)            // ダイアログの「×」やESC
    END_MSG_MAP()

private:
    CProgressBarCtrl m_progressBar;
    CButton m_btnStart;
    CButton m_btnCancel;

    std::atomic<bool> m_bIsExtracting{ false }; // 解凍中フラグ（スレッドセーフ）
    std::atomic<bool> m_bCancelRequested{ false }; // キャンセル要求フラグ（スレッドセーフ）

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_progressBar.Attach(GetDlgItem(IDC_PROGRESS));
        m_progressBar.SetRange(0, 100);
        m_progressBar.SetPos(0);

        m_btnStart.Attach(GetDlgItem(IDC_START_BTN));
        m_btnCancel.Attach(GetDlgItem(IDC_CANCEL_BTN));

        // 初期状態ではキャンセルボタンは無効
        m_btnCancel.EnableWindow(FALSE);
        return TRUE;
    }

    // 1. 解凍開始ボタン押下
    LRESULT OnStartExtract(WORD, WORD, HWND, BOOL&)
    {
        if (m_bIsExtracting) return 0;

        m_bIsExtracting = true;
        m_bCancelRequested = false;

        // UIコントロールの状態更新
        m_btnStart.EnableWindow(FALSE);
        m_btnCancel.EnableWindow(TRUE);
        m_progressBar.SetPos(0);

        // バックグラウンドスレッドの起動
        std::thread extractThread(&CMainDlg::ExtractThreadProc, this, m_hWnd);
        extractThread.detach(); // スレッドの管理を切り離す

        return 0;
    }

    // 2. キャンセルボタン押下
    LRESULT OnCancelExtract(WORD, WORD, HWND, BOOL&)
    {
        if (m_bIsExtracting)
        {
            // キャンセル要求フラグを立てる（次のWM_ARCEXTRACTのタイミングでDLLに中断が伝わります）
            m_bCancelRequested = true;
            m_btnCancel.EnableWindow(FALSE); // 連打防止
        }
        return 0;
    }

    // 3. 🔥 バックグラウンドで実行される解凍スレッド
    void ExtractThreadProc(HWND hWndUI)
    {
        int nResult = -1; // エラーコード初期値
        HMODULE hDll = ::LoadLibrary(TEXT("UNLHA32.DLL"));

        if (!hDll) {
            // DLL読み込み失敗時は即座にUIスレッドへ通知 (wParamにエラーフラグを格納)
            ::PostMessage(hWndUI, WM_APP_EXTRACT_COMPLETE, FALSE, -1);
            return;
        }

        LPFNUnlha pUnlha = (LPFNUnlha)::GetProcAddress(hDll, "Unlha");
        LPFNUnlhaSetOwnerWindow pSetOwner = (LPFNUnlhaSetOwnerWindow)::GetProcAddress(hDll, "UnlhaSetOwnerWindow");
        LPFNUnlhaClearOwnerWindow pClearOwner = (LPFNUnlhaClearOwnerWindow)::GetProcAddress(hDll, "UnlhaClearOwnerWindow");

        if (pUnlha && pSetOwner && pClearOwner)
        {
            pSetOwner(hWndUI);

            std::string cmd = "x \"C:\\path\\to\\archive.lzh\" \"C:\\extracted\\\"";
            char szOutput[1024] = { 0 };

            // 解凍実行（ブロッキング）
            nResult = pUnlha(hWndUI, cmd.c_str(), szOutput, sizeof(szOutput));

            pClearOwner();
        }

        ::FreeLibrary(hDll);

        // 💡 終了ステータスをUIスレッドに安全に届ける (PostMessage)
        // nResult == 0 の場合は正常終了、それ以外はエラーまたはユーザーキャンセル
        BOOL bSuccess = (nResult == 0) ? TRUE : FALSE;
        ::PostMessage(hWndUI, WM_APP_EXTRACT_COMPLETE, bSuccess, nResult);
    }

    // 4. UIスレッド：DLLから定期的に送られてくる進捗メッセージの処理
    LRESULT OnArcExtract(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
    {
        // 🛑 ユーザーからキャンセル要求があった場合、1 を返してDLLの解凍処理をアボート（中断）させる
        if (m_bCancelRequested)
        {
            return 1;
        }

        LPEXTRACTINGINFO pInfo = (LPEXTRACTINGINFO)lParam;
        if (pInfo && pInfo->dwFileSize > 0)
        {
            int progress = static_cast<int>((pInfo->dwWriteSize * 100) / pInfo->dwFileSize);
            m_progressBar.SetPos(progress);
        }

        return 0; // 0 を返すと解凍をそのまま続行
    }

    // 5. UIスレッド：解凍が完了（またはエラー・キャンセル終了）した時の処理
    LRESULT OnExtractComplete(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
    {
        BOOL bSuccess = (BOOL)wParam;
        int nResult = (int)lParam;

        // UI状態のリセット
        m_bIsExtracting = false;
        m_btnStart.EnableWindow(TRUE);
        m_btnCancel.EnableWindow(FALSE);

        // 結果に応じたメッセージ表示
        if (bSuccess)
        {
            m_progressBar.SetPos(100); // 綺麗に100%にする
            ::MessageBox(m_hWnd, TEXT("解凍が正常に完了しました。"), TEXT("完了"), MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            if (m_bCancelRequested)
            {
                m_progressBar.SetPos(0);
                ::MessageBox(m_hWnd, TEXT("ユーザーによって解凍が中断されました。"), TEXT("キャンセル"), MB_OK | MB_ICONEXCLAMATION);
            }
            else
            {
                // エラーコードに応じた処理（UNLHA32.DLLのエラーコード仕様に基づく）
                TCHAR szErr[64];
                ::wsprintf(szErr, TEXT("解凍中にエラーが発生しました。(エラーコード: %d)"), nResult);
                ::MessageBox(m_hWnd, szErr, TEXT("エラー"), MB_OK | MB_ICONERROR);
            }
        }

        return 0;
    }

    // 6. 安全対策：解凍中にダイアログを閉じさせない制御
    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        if (m_bIsExtracting)
        {
            ::MessageBox(m_hWnd, TEXT("解凍処理中はダイアログを閉じられません。中断する場合はキャンセルボタンを押してください。"), TEXT("確認"), MB_OK | MB_ICONWARNING);
            return 0;
        }
        EndDialog(wID);
        return 0;
    }
};
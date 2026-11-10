#include <iostream>
#include <vector>
#include <string>
#include <sstream>

// 解析結果を安全に管理するための構造体
struct ParsedCommandLine {
  std::vector<std::string> storage; // 実際の文字列データを安全に保持する
  std::vector<char*> argv;          // Cスタイルの関数に渡せる char* のリスト
};

// 1行のコマンドラインを解析する関数
ParsedCommandLine parse_command_line(const std::string& cmdline) {
  ParsedCommandLine result;
  std::string current_token;
  bool in_quotes = false;
  char quote_char = '\0';

  for (size_t i = 0; i < cmdline.length(); ++i) {
    char ch = cmdline[i];

    if (in_quotes) {
      // クォートの閉じ判定
      if (ch == quote_char) {
        in_quotes = false;
        quote_char = '\0';
      }
      else {
        current_token += ch;
      }
    }
    else {
      // クォートの開始判定
      if (ch == '"' || ch == '\'') {
        in_quotes = true;
        quote_char = ch;
      }
      // スペース区切り（トークンの確定）
      else if (std::isspace(static_cast<unsigned char>(ch))) {
        if (!current_token.empty()) {
          result.storage.push_back(current_token);
          current_token.clear();
        }
      }
      // 通常の文字
      else {
        current_token += ch;
      }
    }
  }

  // 最後のトークンを追加
  if (!current_token.empty()) {
    result.storage.push_back(current_token);
  }

  // std::string の所有するメモリから char* リストを構築
  for (auto& s : result.storage) {
    result.argv.push_back(s.data());
  }

  // C言語の慣例（execvpやgetoptなど）に合わせる場合、末尾にnullptrを入れると安全です
  result.argv.push_back(nullptr);

  return result;
}

int test_cmdline() {
  // テスト用のコマンドライン文字列（クォーテーションを含む）
  std::string cmd = "gcc -Wall -O2 main.cpp -o \"my program\" 'additional arg'";

  // 解析の実行
  ParsedCommandLine parsed = parse_command_line(cmd);

  // 取得した std::vector<char*> の中身を確認
  // 末尾の nullptr 手前までループ
  std::cout << "--- 解析結果 ---" << std::endl;
  for (size_t i = 0; i < parsed.argv.size() - 1; ++i) {
    std::cout << "argv[" << i << "]: " << parsed.argv[i] << std::endl;
  }

  return 0;
}

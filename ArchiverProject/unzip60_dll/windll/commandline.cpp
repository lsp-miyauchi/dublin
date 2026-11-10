#include <windows.h>
#include "lspuzp.h"
#include "openarchive.h"

/*
 * parse command line
 */
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

  return result;
}

#include <iostream>
#include <unordered_set>
#include <vector>
#include <string>

#pragma once

template <typename T>
class JsSet {
private:
    std::vector<T> elements;          // 順序維持用
    std::unordered_set<T> quick_set;  // 重複チェック・高速検索用

public:
    // set.add(value)
    bool add(const T& value) {
        // すでに存在していれば追加しない
        if (quick_set.find(value) != quick_set.end()) {
            return false;
        }
        quick_set.insert(value);
        elements.push_back(value);
        return true;
    }

    // set.has(value)
    bool has(const T& value) const {
        return quick_set.find(value) != quick_set.end();
    }

    // set.size
    size_t size() const {
        return elements.size();
    }

    // set.clear()
    void clear() {
        elements.clear();
        quick_set.clear();
    }

    // 順序を維持したイテレーション用 (for-of文の再現)
    auto begin() { return elements.begin(); }
    auto end() { return elements.end(); }
};

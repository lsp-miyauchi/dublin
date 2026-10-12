#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>

#pragma once

template <typename K, typename V>
class JsMap {
private:
    // 順序を維持するためのリスト（キーと値のペア）
    std::vector<std::pair<K, V>> elements;
    // キーからvectorのインデックスへのマップ（高速検索用）
    std::unordered_map<K, size_t> key_to_index;

public:
    // map.set(key, value)
    void set(const K& key, const V& value) {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            // 既にキーが存在する場合は値を更新
            elements[it->second].second = value;
        }
        else {
            // 新規追加
            key_to_index[key] = elements.size();
            elements.push_back({ key, value });
        }
    }

    // map.get(key)
    std::optional<V> get(const K& key) const {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            return elements[it->second].second;
        }
        return std::nullopt; // 見つからない場合は無効値を返す
    }

    // map.get(key)
    std::optional<V> get(const K& key) const {
        auto it = key_to_index.find(key);
        if (it != key_to_index.end()) {
            return elements[it->second].second;
        }
        return std::nullopt; // 見つからない場合は無効値を返す
    }

    // map.has(key)
    bool has(const K& key) const {
        return key_to_index.find(key) != key_to_index.end();
    }

    // map.size
    size_t size() const {
        return elements.size();
    }

    // 順序を維持したイテレーション用
    auto begin() { return elements.begin(); }
    auto end() { return elements.end(); }
};

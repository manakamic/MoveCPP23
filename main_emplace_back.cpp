// =============================================================================
//  main_emplace_back.cpp
//  第11章：emplace_back とコンテナ
// -----------------------------------------------------------------------------
//  対応スライド：「11. emplace_back とコンテナ」
//  この章で確認すること：
//    ・push_back(Enemy(...)) … ①一時オブジェクトを構築 → ②それをムーブ格納（2段階）
//    ・emplace_back(引数...)  … 要素をコンテナ内に「その場で直接構築」（1段階）
//        → 一時オブジェクトの生成＋ムーブを丸ごと省ける（内部は §8 の完全転送）
//    ・reserve で事前に容量確保しておけば、途中の再確保（全要素移動）が起きない
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_emplace_back.cpp
// =============================================================================

#include <print>     // C++23: std::println
#include <vector>
#include <string>
#include <utility>   // std::move

struct Enemy {
    int         hp;
    std::string name;
    Enemy(int h, std::string n) : hp(h), name(std::move(n)) {
        std::println("      Enemy 構築  ({}, \"{}\")", hp, name);
    }
    Enemy(const Enemy& o) : hp(o.hp), name(o.name) {
        std::println("      Enemy コピー (\"{}\")", name);
    }
    Enemy(Enemy&& o) noexcept : hp(o.hp), name(std::move(o.name)) {
        std::println("      Enemy ムーブ (\"{}\")", name);
    }
};

int main() {
    std::println("=== 第11章 push_back vs emplace_back ===\n");

    std::vector<Enemy> v;
    v.reserve(4);   // ★事前確保：以降 4 個までは再確保（＝要素移動）が起きない

    std::println("--- push_back(Enemy(100, \"orc\")) ---");
    std::println("  （①一時オブジェクトを構築 → ②それをムーブして格納）");
    v.push_back(Enemy(100, "orc"));
    std::println("");

    std::println("--- emplace_back(80, \"goblin\") ---");
    std::println("  （引数を完全転送し、コンテナ内で“その場で直接構築”）");
    Enemy& ref = v.emplace_back(80, "goblin");   // C++17 以降は構築した要素への参照を返す
    std::println("  emplace_back の戻り値で直接いじれる: ref.hp = {}", ref.hp);
    std::println("");

    std::println(">>> push_back は「構築＋ムーブ」の2段階、emplace_back は「構築」だけの1段階。");
    std::println(">>> 大量生成では reserve() ＋ emplace_back ＋ noexcept ムーブ（§6）が効く。");
}

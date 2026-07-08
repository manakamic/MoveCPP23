// =============================================================================
//  main_noexcept_move.cpp
//  第6章：noexcept とムーブ（なぜセットで語られるのか）
// -----------------------------------------------------------------------------
//  対応スライド：「6. noexcept とムーブ」
//  この章で確認すること：
//    std::vector は容量を増やす（再確保する）とき move_if_noexcept で判断する：
//      ・ムーブが noexcept        → ムーブで移す（速い）✅
//      ・ムーブが noexcept でない  → コピーで移す（遅い・安全側）❌
//    理由：再確保の途中で例外が出ても元の配列を壊さないため（強い例外保証）。
//
//  Good（noexcept あり）と Bad（noexcept なし）で、再確保時の挙動が
//  「ムーブ」と「コピー」に分かれることを実際の出力で確認する。
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_noexcept_move.cpp
// =============================================================================

#include <print>        // C++23: std::println
#include <vector>
#include <type_traits>  // std::is_nothrow_move_constructible_v

// ムーブが noexcept 「あり」------------------------------------------------------
struct Good {
    int id;
    explicit Good(int i) : id(i) {}
    Good(const Good& o) : id(o.id)     { std::println("      Good  コピー (#{})", id); }
    Good(Good&& o) noexcept : id(o.id) { std::println("      Good  ムーブ (#{})", id); }  // ★noexcept
};

// ムーブが noexcept 「なし」------------------------------------------------------
struct Bad {
    int id;
    explicit Bad(int i) : id(i) {}
    Bad(const Bad& o) : id(o.id) { std::println("      Bad   コピー (#{})", id); }
    Bad(Bad&& o)      : id(o.id) { std::println("      Bad   ムーブ (#{})", id); }  // ★noexcept なし
};

// 容量 1 から順に emplace_back し、再確保（既存要素の移動）を意図的に起こす
template <class T>
void grow(const char* label) {
    std::println("[{}]  is_nothrow_move_constructible = {}",
                 label, std::is_nothrow_move_constructible_v<T>);
    std::vector<T> v;
    v.reserve(1);                       // わざと小さく → 追加のたびに再確保が起きる
    for (int i = 0; i < 4; ++i) {
        std::println("  emplace_back(#{})  … 容量超過なら既存要素が移動される:", i);
        v.emplace_back(i);
    }
    std::println("");
}

int main() {
    std::println("=== 第6章 noexcept とムーブ ===\n");

    std::println("★ Good：ムーブが noexcept → 再確保時も“ムーブ”で移る（速い）");
    grow<Good>("Good");

    std::println("★ Bad：ムーブが noexcept でない → 再確保時は“コピー”で移る（遅い）");
    grow<Bad>("Bad");

    std::println(">>> 結論：ムーブコンストラクタには基本 noexcept を付ける。");
    std::println(">>>       付け忘れると vector 再確保のたびに全コピーが走る。");
    std::println("    （vector の成長率は実装依存。MSVC では約1.5倍ずつ増える）");
}

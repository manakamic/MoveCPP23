// =============================================================================
//  main_forward.cpp
//  第8章：std::forward（完全転送 / perfect forwarding）
// -----------------------------------------------------------------------------
//  対応スライド：「8. std::forward」
//  この章で確認すること：
//    ・転送参照 x は「名前がある」ため、そのまま渡すと常に左辺値＝コピー版が選ばれる
//    ・std::forward<T> で T の記憶どおりに値カテゴリを復元すると、
//        左辺値 → コピー版 / 右辺値 → ムーブ版 が“意図どおり”に選ばれる
//    ・可変長テンプレート＋畳み込み式で、複数引数もまとめて完全転送できる
//        （これが make_unique / emplace_back の内部でやっていること）
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_forward.cpp
// =============================================================================

#include <print>     // C++23: std::println
#include <utility>   // std::forward, std::move
#include <string>

struct Widget {
    std::string tag;
    explicit Widget(std::string t) : tag(std::move(t)) {}
};

// 受け取り側：コピー版 と ムーブ版 の 2 つ。どちらが選ばれたかを表示。
void target(const Widget& w) { std::println("      → target(const Widget&)  [コピー版]  tag=\"{}\"", w.tag); }
void target(Widget&& w)      { std::println("      → target(Widget&&)       [ムーブ版]  tag=\"{}\"", w.tag); }

// forward を使わない版：x は名前付き＝左辺値なので、常にコピー版になる（バグ）
template <class T>
void wrapper_bad(T&& x) {
    target(x);
}

// forward を使う版：T の記憶どおりに値カテゴリを復元 → 正しく振り分けられる
template <class T>
void wrapper_good(T&& x) {
    target(std::forward<T>(x));
}

// 可変長：各引数を、それぞれの値カテゴリのまま target へ（C++17 畳み込み式）
template <class... Args>
void forward_all(Args&&... args) {
    (target(std::forward<Args>(args)), ...);
}

int main() {
    std::println("=== 第8章 std::forward（完全転送）===\n");

    Widget a("lvalue");

    std::println("--- forward を使わない wrapper_bad（情報が消える）---");
    std::println("  wrapper_bad(a)            // 左辺値:");
    wrapper_bad(a);
    std::println("  wrapper_bad(Widget(\"rv\")) // 右辺値を渡したのに…:");
    wrapper_bad(Widget("rvalue"));   // ← 右辺値なのにコピー版！（値カテゴリが失われた）
    std::println("");

    std::println("--- forward を使う wrapper_good（正しく復元）---");
    std::println("  wrapper_good(a)            // 左辺値:");
    wrapper_good(a);
    std::println("  wrapper_good(Widget(\"rv\")) // 右辺値:");
    wrapper_good(Widget("rvalue"));  // ← 右辺値 → ムーブ版！（意図どおり）
    std::println("");

    std::println("--- 可変長テンプレートでまとめて完全転送 ---");
    Widget b("lv2");
    std::println("  forward_all(b, Widget(\"rv2\"))  // 左辺値→コピー版, 右辺値→ムーブ版:");
    forward_all(b, Widget("rv2"));
    std::println("");

    std::println(">>> 使い分け：転送参照 T&&（型推論あり）で受けた引数 → std::forward<T>");
    std::println(">>>          これ以上使わないローカル値・具体的な T&&    → std::move");
}

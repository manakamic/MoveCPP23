// =============================================================================
//  main_copy_elision.cpp
//  第12章：コピー省略（RVO / NRVO / 保証されたコピー省略）
// -----------------------------------------------------------------------------
//  対応スライド：「12. コピー省略」
//  この章で確認すること：
//    ① prvalue を返す        … C++17「保証されたコピー省略」。コピー/ムーブは“絶対に”起きない
//                              （コピー/ムーブが delete されていても返せる＝下の NoMove で証明）
//    ② 名前付きローカルを返す … NRVO。規格上は「任意」の最適化。ただし MSVC 2026 は
//                              最適化オフ(/Od)でも適用するため、本サンプルでは「構築」だけになる
//                              （NRVO 非対応の古い環境ではムーブが入ることがある）
//    ③ return std::move(local) … アンチパターン。xvalue 化して省略が外れ、必ずムーブが走る
//
//  ★ ①「構築だけ」／③「構築＋ムーブ」の対比は、最適化設定に関係なく常に成り立つ（重要）。
//    ② は環境依存（規格上は任意）だが、MSVC では既定で効くので「構築だけ」になる。
//    → NRVO は「効けば嬉しいが、当てにはしない」。だから ③ のように自分で壊さないことが大切。
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_copy_elision.cpp
//    （/O2 を付けても結果は同じ： cl /std:c++latest /utf-8 /EHsc /O2 main_copy_elision.cpp）
// =============================================================================

#include <print>     // C++23: std::println
#include <utility>   // std::move

// 各特殊メンバにログを仕込んだ観察用の型
struct Probe {
    Probe()                 { std::println("    Probe 構築"); }
    Probe(const Probe&)     { std::println("    Probe コピー"); }
    Probe(Probe&&) noexcept { std::println("    Probe ムーブ"); }
    ~Probe()                { std::println("    Probe 破棄"); }
};

Probe make_prvalue() { return Probe{}; }               // ① prvalue を返す（保証された省略）
Probe make_nrvo()    { Probe p; return p; }            // ② 名前付きローカルを返す（NRVO：任意）
Probe make_bad()     { Probe p; return std::move(p); } // ③ アンチパターン（省略が外れる）

// コピーもムーブも禁止した型。① の「保証された省略」が無ければ、そもそも返せない。
struct NoMove {
    NoMove()                = default;
    NoMove(const NoMove&)   = delete;
    NoMove(NoMove&&)        = delete;
};
NoMove make_nomove() { return NoMove{}; }   // prvalue → コピー/ムーブ無しでも構築できる

int main() {
    std::println("=== 第12章 コピー省略（RVO / NRVO）===\n");

    std::println("--- ① prvalue を返す（保証されたコピー省略・C++17）---");
    auto a = make_prvalue();   // 「構築」だけ。コピーもムーブも絶対に起きない
    std::println("");

    std::println("--- ② 名前付きローカルを返す（NRVO：規格上は任意）---");
    auto b = make_nrvo();      // MSVC は既定で NRVO を適用 → 「構築」だけ（環境によってはムーブが入る）
    std::println("");

    std::println("--- ③ return std::move(local)（アンチパターン）---");
    auto c = make_bad();       // 必ず「構築」+「ムーブ」。省略のチャンスを自ら潰している
    std::println("");

    std::println("--- おまけ：コピー/ムーブ禁止の型でも prvalue なら返せる ---");
    NoMove n = make_nomove();  // ① の保証された省略のおかげでコンパイルが通る
    (void)n;
    std::println("  NoMove（コピー/ムーブ = delete）を値で受け取れた ＝ 実体が直接構築された\n");

    std::println(">>> ローカル変数をそのまま返すときは std::move を付けない（③は書かない）。");
    std::println(">>> ① は言語規則なので最適化オフでも保証される点がポイント。");
    std::println("--- スコープ終了：破棄ログが並ぶ ---");
}

// =============================================================================
//  main_deducing_this.cpp
//  第9章：C++23 の新機能 — deducing this と std::forward_like
// -----------------------------------------------------------------------------
//  対応スライド：「9. C++23 の新機能」
//  この章で確認すること：
//    ・deducing this（明示的オブジェクトパラメータ）で、"自分自身" を引数に取れる
//    ・this auto&& self ＝ オブジェクト版の転送参照（呼び出し方を丸ごと覚える）
//    ・std::forward_like で「メンバ」を self の値カテゴリに合わせて返す
//        従来 4 つ必要だった getter が 1 つで済む
//        左辺値で呼ぶ → メンバは参照で返る（コピーなし）
//        右辺値で呼ぶ → メンバはムーブで返る（＝所有権が移り、元は空に）
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_deducing_this.cpp
// =============================================================================

#include <print>        // C++23: std::println
#include <utility>      // std::move, std::forward_like
#include <type_traits>  // std::is_same_v
#include <string>

class Widget {
    std::string data_;
public:
    explicit Widget(std::string s) : data_(std::move(s)) {}

    // ★ C++23: これ 1 つで、従来の 4 オーバーロードを置き換える ------------------
    //   自分自身 self を転送参照(this auto&&)で受け取り、
    //   メンバ data_ を「self の値カテゴリに合わせて」返す。
    auto&& get(this auto&& self) {
        return std::forward_like<decltype(self)>(self.data_);
    }

    // 検証用：値カテゴリを変えずに中身を覗くだけの入口
    const std::string& raw() const { return data_; }
};

/*  ------ 従来（C++20 以前）は同じことに 4 つ必要だった ------
    std::string&        get() &       { return data_; }             // 左辺値
    const std::string&  get() const&  { return data_; }             // const 左辺値
    std::string&&       get() &&      { return std::move(data_); }  // 右辺値
    const std::string&& get() const&& { return std::move(data_); }  // const 右辺値
    -----------------------------------------------------------  */

int main() {
    std::println("=== 第9章 deducing this / std::forward_like（C++23）===\n");

    // 返る型が呼び出し方で変わることを、コンパイル時に確認 ----------------------
    Widget probe("x");
    static_assert(std::is_same_v<decltype(probe.get()),            std::string&>);   // 左辺値 → 左辺値参照
    static_assert(std::is_same_v<decltype(std::move(probe).get()), std::string&&>);  // 右辺値 → 右辺値参照
    std::println("[static_assert] w.get() は string& / std::move(w).get() は string&&（通過）\n");

    // SSO を避けるため十分に長い文字列を使う（短いと heap を使わず観察しにくい）
    Widget w("This string is long enough to be heap-allocated (SSO を超える長さ)");
    const void* orig = static_cast<const void*>(w.raw().data());
    std::println("元の char バッファのアドレス = {}\n", orig);

    // ① 左辺値で get → 参照が返る（コピーもムーブもしない）----------------------
    std::println("--- ① 左辺値 w から get()：参照が返る ---");
    std::string& lref = w.get();
    std::println("  lref.data() = {}", static_cast<const void*>(lref.data()));
    std::println("  元と同じアドレス？ {}  → 参照なのでコピーもムーブも起きていない\n",
                 orig == static_cast<const void*>(lref.data()));

    // ② 右辺値で get → ムーブされる（所有権が移り、w は空になる）----------------
    std::println("--- ② std::move(w) から get()：ムーブされる ---");
    std::string moved = std::move(w).get();     // forward_like が string&& を返す → ムーブ構築
    std::println("  moved = \"{}\"", moved);
    std::println("  moved.data() = {}", static_cast<const void*>(moved.data()));
    std::println("  元と同じアドレス？ {}  → 同じ！ ＝ heap を複製せず所有権だけ移した",
                 orig == static_cast<const void*>(moved.data()));
    std::println("  ムーブ後の w のサイズ = {}（中身は moved へ引っ越して空になった）", w.raw().size());

    std::println("\n>>> deducing this ＝ オブジェクト自身の転送参照（第7章 auto&& の応用）");
    std::println(">>> forward_like     ＝ メンバ用の forward（第8章 std::forward の応用）");
}

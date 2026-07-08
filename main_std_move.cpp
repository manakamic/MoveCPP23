// =============================================================================
//  main_std_move.cpp
//  第4章：std::move の正体
// -----------------------------------------------------------------------------
//  対応スライド：「4. std::move の正体」
//  この章で確認すること：
//    (1) std::move は「右辺値へのキャスト」でしかない（実行時に何も動かさない）
//        → std::move(w) の指す先は w そのもの＝アドレスが同じ／コンストラクタも呼ばれない
//    (2) 自作の my_move（static_cast するだけ）でも、まったく同じ振り分けになる
//    (3) const を付けるとムーブできず、こっそりコピーにフォールバックする（罠）
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_std_move.cpp
// =============================================================================

#include <print>        // C++23: std::println
#include <utility>      // std::move
#include <type_traits>  // std::remove_reference_t
#include <string>

// コピーとムーブのどちらが選ばれたかをログで見せる型
struct Widget {
    std::string tag;
    explicit Widget(std::string t) : tag(std::move(t)) {
        std::println("    Widget(\"{}\") 構築", tag);
    }
    Widget(const Widget& o) : tag(o.tag) {
        std::println("    → コピー構築 (const Widget&)  [copy!]  tag=\"{}\"", tag);
    }
    Widget(Widget&& o) noexcept : tag(std::move(o.tag)) {
        std::println("    → ムーブ構築 (Widget&&)       [move!]  tag=\"{}\"", tag);
    }
};

// std::move の中身は、概念的にはこれだけ（右辺値参照へキャストして返すのみ）
template <class T>
constexpr std::remove_reference_t<T>&& my_move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);
}

int main() {
    std::println("=== 第4章 std::move の正体 ===\n");

    // (1) std::move は何も動かさない（キャストだけ）------------------------------
    std::println("--- (1) std::move はキャストするだけ（オブジェクトを作らない）---");
    Widget w("A");
    Widget&& rr = std::move(w);   // 右辺値参照で束縛するだけ。コンストラクタは呼ばれない！
    std::println("    &w  = {}", static_cast<const void*>(&w));
    std::println("    &rr = {}  ← w と同じアドレス。std::move は新しい実体を作らない\n",
                 static_cast<const void*>(&rr));

    // (2) 自作 my_move でも同じ結果 --------------------------------------------
    std::println("--- (2) 自作 my_move（static_cast のみ）でも「ムーブ構築」が選ばれる ---");
    Widget src("B");
    std::println("    Widget x = my_move(src);");
    Widget x = my_move(src);      // → ムーブ構築
    std::println("");

    // (3) const の罠 ------------------------------------------------------------
    std::println("--- (3) const の罠：move したつもりが copy になる ---");
    const Widget cw("C");         // ★const！
    std::println("    const Widget cw(\"C\");  →  Widget y = std::move(cw);");
    Widget y = std::move(cw);     // const Widget&& はムーブ版に束縛できず、コピー版へ
    std::println("    ↑ [move!] ではなく [copy!] が呼ばれた。const がムーブを殺している\n");

    // (4) const を外せば期待どおりムーブ ---------------------------------------
    std::println("--- (4) const を外した通常ケース ---");
    Widget nw("D");
    std::println("    Widget z = std::move(nw);");
    Widget z = std::move(nw);     // → ムーブ構築

    std::println("\n>>> まとめ：std::move は「奪って良い」という許可証（キャスト）。");
    std::println(">>>        実際に奪うのはムーブコンストラクタ。const だと奪えない。");
}

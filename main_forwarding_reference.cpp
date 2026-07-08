// =============================================================================
//  main_forwarding_reference.cpp
//  第7章：テンプレートと転送参照（forwarding reference）／ auto&&
// -----------------------------------------------------------------------------
//  対応スライド：「7. テンプレートと転送参照」
//  この章で確認すること：
//    (1) テンプレートの T&& は「左辺値も右辺値も」受け取れる（＝転送参照）
//        ・左辺値で来る → T = U&（参照付き）／右辺値で来る → T = U（参照なし）
//    (2) reference collapsing（参照の圧縮）ルールを static_assert で確認
//    (3) auto&& も転送参照。範囲 for でコピーせず受けられる
//
//  ※ static_assert はコンパイルが通る＝主張が正しい、という「コンパイル時の証明」。
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_forwarding_reference.cpp
// =============================================================================

#include <print>        // C++23: std::println（vector の "{}" 出力も C++23）
#include <utility>      // std::move
#include <type_traits>  // std::is_lvalue_reference_v, std::is_same_v
#include <vector>

// 転送参照。渡された値カテゴリによって T の中身が変わる。
template <class T>
void g(T&& x) {
    if constexpr (std::is_lvalue_reference_v<T>)
        std::println("    左辺値で呼ばれた → T = U&（参照付き）, x は左辺値参照");
    else
        std::println("    右辺値で呼ばれた → T = U （参照なし）, x は右辺値参照");
    // x には名前があるので、関数の中では x 自身は常に「左辺値」（アドレスが取れる）
    std::println("      &x = {}（x には名前がある＝この時点では x 自身は左辺値）",
                 static_cast<const void*>(&x));
}

// 見た目は同じ && でも、型が固定なら「ただの右辺値参照」（右辺値専用）
void only_rvalue(int&&) { /* 左辺値は渡せない */ }

int main() {
    std::println("=== 第7章 転送参照 / auto&& ===\n");

    // (1) 転送参照は両方受け取れる ----------------------------------------------
    std::println("--- (1) テンプレートの T&& は左辺値も右辺値も受け取れる ---");
    int a = 5;
    std::println("  g(a)         // 左辺値:");
    g(a);
    std::println("  g(5)         // 右辺値:");
    g(5);
    std::println("  g(std::move(a)) // 右辺値化:");
    g(std::move(a));
    // only_rvalue(a);   // ← NG：型固定の int&& は左辺値 a を受け取れない
    std::println("  （型が固定の void only_rvalue(int&&) には左辺値 a を渡せない）\n");

    // (2) reference collapsing をコンパイル時に確認 -----------------------------
    std::println("--- (2) reference collapsing（& が 1 つでもあれば左辺値参照）---");
    using LRef = int&;
    using RRef = int&&;
    static_assert(std::is_same_v<LRef&,  int&>,  "&  &  -> &");
    static_assert(std::is_same_v<LRef&&, int&>,  "&  && -> &");
    static_assert(std::is_same_v<RRef&,  int&>,  "&& &  -> &");
    static_assert(std::is_same_v<RRef&&, int&&>, "&& && -> &&");
    std::println("    static_assert 4 本すべて通過 → 圧縮ルールは次のとおり:");
    std::println("      &  &  -> &      /   &  && -> &");
    std::println("      && &  -> &      /   && && -> &&\n");

    // (3) auto&& も転送参照 -----------------------------------------------------
    std::println("--- (3) auto&& も転送参照 ---");
    auto&& r1 = a;    // a は左辺値 → decltype(r1) は int&
    auto&& r2 = 5;    // 5 は右辺値 → decltype(r2) は int&&
    static_assert(std::is_same_v<decltype(r1), int&>);
    static_assert(std::is_same_v<decltype(r2), int&&>);
    std::println("    auto&& r1 = a; → int&   /   auto&& r2 = 5; → int&&（static_assert 通過）");

    std::vector<int> v{1, 2, 3};
    for (auto&& e : v) e += 10;   // コピーせず参照で受けて、その場で書き換え
    std::println("    範囲 for の auto&& で書き換えた v = {}\n", v);

    std::println(">>> まとめ：転送参照は「左辺値/右辺値のどちらで来たか」を T に記録する箱。");
    std::println(">>>        ただし箱（x）自身には名前があるので、渡すときは左辺値になる → 第8章へ。");
}

// =============================================================================
//  main_move_only_function.cpp
//  第10章：スマートポインタと関数ラッパ（unique_ptr / std::move_only_function）
// -----------------------------------------------------------------------------
//  対応スライド：「10. スマートポインタと関数ラッパ」
//  この章で確認すること：
//    (1) unique_ptr はコピー禁止・ムーブでのみ所有権を渡す
//        → ムーブしても「指す実体のアドレス」は同じ（新規確保していない）
//    (2) unique_ptr をキャプチャしたラムダは move-only になり、std::function に入らない
//    (3) C++23 の std::move_only_function なら move-only な callable を格納できる
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_move_only_function.cpp
// =============================================================================

#include <print>       // C++23: std::println
#include <memory>      // std::unique_ptr, std::make_unique
#include <functional>  // std::function, std::move_only_function（C++23）
#include <utility>     // std::move

int main() {
    std::println("=== 第10章 unique_ptr / move_only_function（C++23）===\n");

    // (1) unique_ptr は所有権をムーブで渡す ------------------------------------
    std::println("--- (1) unique_ptr の所有権ムーブ ---");
    auto p = std::make_unique<int>(42);
    std::println("  p が指す実体のアドレス = {}  (*p = {})", static_cast<const void*>(p.get()), *p);
    // auto bad = p;                 // ← NG：コピー禁止（deleteされている）
    auto q = std::move(p);           // 所有権を q へムーブ
    std::println("  ムーブ後 q.get() = {}  ← 同じ実体（新規確保していない）",
                 static_cast<const void*>(q.get()));
    std::println("  ムーブ後 p.get() = {}  ← 空(nullptr)。所有者は q ただ 1 人\n",
                 static_cast<const void*>(p.get()));

    // (2)(3) move-only なラムダと関数ラッパ ------------------------------------
    std::println("--- (2)(3) move-only なラムダを関数ラッパに入れる ---");
    auto task = [r = std::make_unique<int>(100)]() {
        std::println("    タスク実行：*r = {}", *r);
    };
    // std::function は「コピー可能」を要求するので、下は【コンパイルエラー】になる：
    //     std::function<void()> f = std::move(task);   // ✗

    // C++23 の move_only_function ならOK（コピー不可・ムーブ専用）
    std::move_only_function<void()> f = std::move(task);   // ✅
    std::println("  std::move_only_function に格納 → 呼び出し:");
    f();

    std::println("\n>>> 所有権が明確なタスクキュー／遅延実行／コールバックでは");
    std::println(">>> shared_ptr で無理に copyable にせず、move_only_function が素直。");
}

// =============================================================================
//  main_rule_of_five.cpp
//  第5章：Rule of Five / Rule of Zero
// -----------------------------------------------------------------------------
//  対応スライド：「5. Rule of Five / Rule of Zero」
//  この章で確認すること：
//    ・資源を直接持つクラスは 5 つの特殊メンバをセットで設計する（Rule of Five）
//        デストラクタ / コピー構築 / コピー代入 / ムーブ構築 / ムーブ代入
//    ・一番良いのは「自分で書かない」＝ Rule of Zero
//        資源管理は string / vector / unique_ptr などの RAII 型に任せる
//        → コンパイラが正しいコピー／ムーブを自動生成してくれる
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_rule_of_five.cpp
// =============================================================================

#include <print>     // C++23: std::println（vector の "{}" 出力も C++23）
#include <utility>   // std::move, std::exchange
#include <string>
#include <vector>
#include <memory>    // std::unique_ptr, std::make_unique

// ============================================================================
//  Rule of Five：5 つ全部を明示的に書いた例（どれが呼ばれたかログを出す）
// ============================================================================
class FiveRule {
    int* p_;   // 生ポインタで資源を直接管理 → だから 5 つを自分で書く必要がある
public:
    explicit FiveRule(int v) : p_(new int(v)) {
        std::println("  [1] 通常コンストラクタ  (*p_={})", *p_);
    }
    ~FiveRule() {                                                   // ① デストラクタ
        std::println("  [5] デストラクタ");
        delete p_;
    }
    FiveRule(const FiveRule& o) : p_(new int(*o.p_)) {              // ② コピー構築
        std::println("  [2] コピーコンストラクタ (新規確保)");
    }
    FiveRule& operator=(const FiveRule& o) {                        // ③ コピー代入
        std::println("  [3] コピー代入演算子");
        if (this != &o) { *p_ = *o.p_; }
        return *this;
    }
    FiveRule(FiveRule&& o) noexcept : p_(std::exchange(o.p_, nullptr)) {  // ④ ムーブ構築
        std::println("  [4] ムーブコンストラクタ (奪取)");
    }
    FiveRule& operator=(FiveRule&& o) noexcept {                    // ⑤ ムーブ代入
        std::println("  [4'] ムーブ代入演算子");
        if (this != &o) { delete p_; p_ = std::exchange(o.p_, nullptr); }
        return *this;
    }
};

// ============================================================================
//  Rule of Zero：特殊メンバを 1 つも書かない
//    各メンバ（string / vector / unique_ptr）が自分でコピー／ムーブを処理する。
// ============================================================================
struct Player {
    std::string          name_;
    std::vector<int>     scores_;
    std::unique_ptr<int> hp_;   // これを持つと「コピーは自動で禁止・ムーブは可能」になる
    // 特殊メンバ関数は書かない！ → コンパイラが正しく自動生成する
};

int main() {
    std::println("=== 第5章 Rule of Five / Rule of Zero ===\n");

    std::println("--- Rule of Five：どの特殊メンバが呼ばれるか観察 ---");
    FiveRule a(1);
    std::println("FiveRule b = a;            // コピー構築");
    FiveRule b = a;
    std::println("FiveRule c = std::move(a); // ムーブ構築");
    FiveRule c = std::move(a);
    FiveRule d(2);
    std::println("d = b;                     // コピー代入");
    d = b;
    std::println("d = std::move(c);          // ムーブ代入");
    d = std::move(c);
    std::println("--- （ここでスコープを抜けると 4 個のデストラクタ）---\n");

    std::println("--- Rule of Zero：特殊メンバを書かずとも正しく動く ---");
    Player p1{ "Hero", {10, 20, 30}, std::make_unique<int>(100) };
    std::println("p1: name=\"{}\" scores={} hp={}", p1.name_, p1.scores_, *p1.hp_);

    std::println("Player p2 = std::move(p1); // メンバごとにムーブ（string/vector/unique_ptr）");
    Player p2 = std::move(p1);
    std::println("p2: name=\"{}\" scores={} hp={}", p2.name_, p2.scores_, *p2.hp_);

    // Player p3 = p2;   // ← NG：unique_ptr を含むのでコピーは自動的に禁止されている
    std::println("");
    std::println("ムーブ後 p1（valid but unspecified）: name=\"{}\", hp は null? {}",
                 p1.name_, (p1.hp_ == nullptr));
    std::println("  ※ unique_ptr は必ず null になるが、string の中身は「未規定」。読まないこと。");
}

// =============================================================================
//  main_move_semantics.cpp
//  第3章：ムーブコンストラクタ / ムーブ代入演算子（＋ std::exchange）
// -----------------------------------------------------------------------------
//  ★★ この教材の“目玉” ★★
//  対応スライド：「3. ムーブ」
//
//  ムーブは「所有権の引っ越し」。中身（heap 上のメモリ）は複製されず、
//  ポインタを付け替えるだけ。→ ムーブ前の元アドレスと、ムーブ後の先アドレスが
//  【同じ】になることを std::println で実証する。
//    同じアドレス = メモリはコピーされていない（＝確保も複写もゼロ）
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_move_semantics.cpp
// =============================================================================

#include <print>      // C++23: std::println
#include <utility>    // std::move, std::exchange
#include <algorithm>  // std::copy（比較用コピー構築で使用）
#include <cstddef>    // std::size_t

class Buffer {
    int*        data_ = nullptr;
    std::size_t size_ = 0;
public:
    explicit Buffer(std::size_t n) : data_(new int[n]{}), size_(n) {
        std::println("  [構築]      data_ = {}", static_cast<const void*>(data_));
    }

    // --- ムーブコンストラクタ --------------------------------------------------
    //   other のポインタを「奪って」自分に付け替え、other は空(nullptr)にする。
    //   std::exchange(obj, newval) … obj に newval を代入し、古い値を返す（C++14）。
    //   noexcept は必須級（理由は第6章）。
    Buffer(Buffer&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)),  // 奪って、元は nullptr
          size_(std::exchange(other.size_, 0)) {
        std::println("  [ムーブ構築] data_ = {}  ← other から所有権を奪取（新規確保なし）",
                     static_cast<const void*>(data_));
    }

    // --- ムーブ代入演算子 ------------------------------------------------------
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {                            // 自己ムーブ対策
            delete[] data_;                               // 先に自分の資源を解放
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
            std::println("  [ムーブ代入] data_ = {}", static_cast<const void*>(data_));
        }
        return *this;
    }

    // --- 比較用：コピーコンストラクタ（呼ばれたら分かるようにログ）-------------
    Buffer(const Buffer& other)
        : data_(new int[other.size_]), size_(other.size_) {
        std::copy(other.data_, other.data_ + size_, data_);
        std::println("  [コピー構築] data_ = {}  ← 新規確保（重い！ アドレスが変わる）",
                     static_cast<const void*>(data_));
    }

    ~Buffer() {
        std::println("  [破棄]      data_ = {}", static_cast<const void*>(data_));
        delete[] data_;
    }

    const void* address() const { return static_cast<const void*>(data_); }
};

int main() {
    std::println("=== 第3章 ムーブセマンティクス（メモリは動いていない！）===\n");

    // ① ムーブコンストラクタ ----------------------------------------------------
    std::println("--- ① ムーブコンストラクタ ---");
    Buffer a(4);
    const void* before = a.address();          // ムーブ前の元アドレスを記録
    std::println("ムーブ前 : a.data = {}\n", before);

    Buffer b = std::move(a);                    // ★ムーブ（a を右辺値化して奪わせる）
    std::println("");
    std::println("ムーブ後 : b.data = {}", b.address());
    std::println("ムーブ後 : a.data = {}   ← 空(nullptr)。中身は b へ引っ越した", a.address());
    std::println("");
    std::println(">>> ムーブ前の a.data と ムーブ後の b.data は同じ？ : {}",
                 (before == b.address())
                     ? "同じ！ → メモリはコピーされていない（ポインタを付け替えただけ）"
                     : "違う");
    std::println("");

    // ② ムーブ代入 --------------------------------------------------------------
    std::println("--- ② ムーブ代入 ---");
    Buffer c(2);
    Buffer d(8);
    const void* cbefore = c.address();
    std::println("代入前 : c.data = {}", cbefore);
    std::println("代入前 : d.data = {}\n", d.address());

    d = std::move(c);                           // ★ムーブ代入（d 元の領域は解放され、c の領域が来る）
    std::println("");
    std::println("代入後 : d.data = {}", d.address());
    std::println(">>> c の元アドレスが d に来た？ : {}\n",
                 (cbefore == d.address()) ? "同じ！ = 引っ越し成功" : "違う");

    std::println("--- スコープ終了（破棄はコンストラクトの逆順：d, c, b, a）---");
}

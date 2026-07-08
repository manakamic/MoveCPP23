// =============================================================================
//  main_deep_copy.cpp
//  第2章：浅いコピー / 深いコピー
// -----------------------------------------------------------------------------
//  対応スライド：「2. コピーの復習」
//  この章で確認すること：
//    ・深いコピーは「新しい領域を確保して中身を複製」する
//      → コピー元とコピー先で heap 上のアドレスが変わる（＝確保＋複写のコスト）
//    ・別実体なので、片方を書き換えても、もう片方は影響を受けない
//    ・（浅いコピーは同じアドレスを共有してしまい二重解放になる…はコメントで説明）
//
//  ビルド： cl /std:c++latest /utf-8 /EHsc main_deep_copy.cpp
// =============================================================================

#include <print>      // C++23: std::println
#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

class Buffer {
    int*        data_ = nullptr;   // heap 上の実体を指す生ポインタ
    std::size_t size_ = 0;
public:
    explicit Buffer(std::size_t n) : data_(new int[n]{}), size_(n) {
        std::println("  Buffer({}) 構築    : data_ = {}", n, static_cast<const void*>(data_));
    }

    // --- コピーコンストラクタ（深いコピー）------------------------------------
    //   ★ new で「新しい領域」を確保し、中身も 1 つずつ複製する。
    Buffer(const Buffer& other)
        : data_(new int[other.size_]), size_(other.size_) {
        std::copy(other.data_, other.data_ + size_, data_);   // 中身も複製
        std::println("  コピー構築         : data_ = {}  ← 新規確保（元 {} とは別アドレス）",
                     static_cast<const void*>(data_),
                     static_cast<const void*>(other.data_));
    }

    // --- コピー代入演算子（深いコピー）----------------------------------------
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {                 // 自己代入対策
            delete[] data_;                    // 自分の資源を解放してから
            data_ = new int[other.size_];      // 改めて確保し
            size_ = other.size_;
            std::copy(other.data_, other.data_ + size_, data_);  // 複製
            std::println("  コピー代入         : data_ = {}", static_cast<const void*>(data_));
        }
        return *this;
    }

    ~Buffer() {
        std::println("  ~Buffer()          : 解放 data_ = {}", static_cast<const void*>(data_));
        delete[] data_;
    }

    void set(std::size_t i, int v) { data_[i] = v; }
    int  get(std::size_t i) const  { return data_[i]; }
    const void* address() const    { return static_cast<const void*>(data_); }
};

int main() {
    std::println("=== 第2章 深いコピー ===\n");

    Buffer a(4);
    a.set(0, 100);
    std::println("a.data = {} / a[0] = {}\n", a.address(), a.get(0));

    std::println("Buffer b = a;   ← コピー（深いコピーが走る）");
    Buffer b = a;
    std::println("");
    std::println("a.data = {}", a.address());
    std::println("b.data = {}   ← 別アドレス！ 実体が丸ごと複製された（重い）\n", b.address());

    std::println("b[0] を 999 に書き換えても、別実体の a[0] は無傷:");
    b.set(0, 999);
    std::println("  a[0] = {} / b[0] = {}\n", a.get(0), b.get(0));

    // 【浅いコピーだったら？】--------------------------------------------------
    //   もし data_ をそのままコピーする「浅いコピー」だと、a と b は同じアドレスを
    //   共有してしまう。→ b を書き換えると a も変わり、スコープ終了時には
    //   同じ領域を 2 回 delete して二重解放（クラッシュ）になる。危険！
    std::println("--- スコープ終了：デストラクタが 2 回、別々のアドレスを解放する ---");
}

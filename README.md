# Move セマンティクス 補助プログラム集（C++23）

講義資料 **「`std::move` と `std::forward` — ムーブセマンティクス 完全再入門」**
（`D:\AMG\2026\std_move_forward_cpp23.md`）の各章に対応した、コンソール実行用の
サンプルプログラムです。

- **環境**：Visual Studio 2026 / MSVC / **C++23**（`/std:c++latest`）
- **方針**：`std::print` / `std::println` など **C++23 の機能**を使い、`iostream` や
  `printf` など古い書き方は使いません（キャストも `static_cast` に統一）。
- **狙い**：`std::move` などで **メモリがコピーされていない（ポインタを付け替えただけ）** ことを、
  `std::println` による **アドレス表示**で目で見て確認できるようにしています。

---

## ファイル一覧（章対応表）

| 章 | ファイル | テーマ | 見どころ |
|---|---|---|---|
| 1 | `main_value_category.cpp` | 値カテゴリ / 右辺値参照 `T&&` | 左辺値だけアドレスが取れる。`std::move(a)` は `a` と同じアドレス |
| 2 | `main_deep_copy.cpp` | 浅い / 深いコピー | コピーは**別アドレス**に新規確保される（重い） |
| 3 | `main_move_semantics.cpp` | ムーブ構築 / 代入・`std::exchange` | **ムーブ前後で同じアドレス**＝コピーしていない証明（★目玉） |
| 4 | `main_std_move.cpp` | `std::move` の正体 | キャストだけ（同じアドレス）／`const` の罠でコピーに落ちる |
| 5 | `main_rule_of_five.cpp` | Rule of Five / Rule of Zero | 5 つの特殊メンバのログ／RAII 任せで自動生成 |
| 6 | `main_noexcept_move.cpp` | `noexcept` とムーブ | `noexcept` 有無で `vector` 再確保が**ムーブ/コピー**に変わる |
| 7 | `main_forwarding_reference.cpp` | 転送参照 / `auto&&` | reference collapsing を `static_assert` で証明 |
| 8 | `main_forward.cpp` | 完全転送 | `forward` 無し＝コピー、有り＝ムーブの対比 |
| 9 | `main_deducing_this.cpp` | deducing this / `forward_like` <sub>C++23</sub> | 4 つの getter を 1 つに。右辺値呼びで所有権が移る |
| 10 | `main_move_only_function.cpp` | `unique_ptr` / `move_only_function` <sub>C++23</sub> | move-only なラムダを格納できる |
| 11 | `main_emplace_back.cpp` | `push_back` vs `emplace_back` | 「構築＋ムーブ」対「その場構築」 |
| 12 | `main_copy_elision.cpp` | RVO / NRVO / 保証された省略 | `return std::move(local)` が損な理由 |
| 13 | — | まとめ・チートシート・落とし穴 | プログラム不要（要点表のため作成していません） |

> 第 13 章はチートシートと落とし穴の一覧なので、サンプルは作成していません。

---

## ビルド方法

各 `.cpp` は **それぞれ独立した `int main()`** を持ちます。目的の章のファイルを 1 本だけ
ビルドして実行する形です。

### 方法A：付属スクリプトで一括ビルド（おすすめ）

通常の PowerShell で（VS の環境は自動で読み込まれます）：

```powershell
# 全部ビルドして、そのまま実行
powershell -ExecutionPolicy Bypass -File .\build_all.ps1 -Run

# 特定の 1 本だけ
powershell -ExecutionPolicy Bypass -File .\build_all.ps1 -Only main_move_semantics.cpp -Run

# 最適化あり（第12章で NRVO の効きを確認したいとき）
powershell -ExecutionPolicy Bypass -File .\build_all.ps1 -Only main_copy_elision.cpp -O2 -Run
```

生成物は `build\` サブフォルダに出力されます。

### 方法B：Developer Command Prompt で個別にコンパイル

スタートメニューの **「x64 Native Tools Command Prompt for VS 2026」** を開き、
このフォルダで：

```bat
cl /std:c++latest /utf-8 /EHsc main_move_semantics.cpp
main_move_semantics.exe
```

- `/std:c++latest` … C++23 の機能（`std::print` / deducing this / `forward_like` /
  `move_only_function`）を有効化します。
- **`/utf-8` は必須**。付けないと日本語の出力が文字化けします。
- `/EHsc` … 標準的な例外処理モデル。

### 方法C：Visual Studio の GUI で開く場合の注意 ⚠️

**1 つのプロジェクトに全 `.cpp` を追加するとリンクエラー**になります
（`main` が多重定義になるため）。次のいずれかで回避してください。

1. **章ごとに別プロジェクト**を作る（最も分かりやすい）。
2. 1 つのプロジェクトに入れ、**使う 1 本以外を「ビルドから除外」**する
   （ソリューションエクスプローラーで対象ファイルを右クリック →
   「プロパティ」→「全般」→ **ビルドから除外＝はい**）。
3. 上記の `build_all.ps1` を使う（プロジェクト不要）。

---

## 出力の読み方（アドレス表示について）

- 生ポインタ（`int*` など）は `std::println("{}", ptr)` で直接は出せないため、
  `static_cast<const void*>(ptr)` で **`const void*`** にキャストして 16 進アドレスを表示しています。
- **同じアドレス** が出たら「メモリは移動していない（＝コピーせず所有権だけ移した）」、
  **違うアドレス** なら「新しく確保して複製した」という意味です。
- 第3章・第9章・第10章が、この「アドレスが変わらない＝ムーブ」を最も分かりやすく示します。

> `std::string` は短い文字列だと SSO（小さな文字列最適化）で内部に直接持つため、
> heap のアドレスが観察できません。第9章では意図的に長い文字列を使っています。

# 観測で確定するタスクカーソル（ライブラリ改修案）

インタラクティブな計画で、行動を提案しても競合や直前の状態変化で実行されない場合に、実際に反映したタスクだけを確定する部品。AHC008の設営試作で、柵の設置提案と設置を観測した時点を分ける必要があった。共通API・解答本体への組み込みは未実施。

- [ヘッダ](../library/observed-task-cursor.hpp) / [使用例](../examples/observed_task_cursor_usage.cpp) / [参照モデルとのテスト](../tests/observed_task_cursor_test.cpp)
- `next()` は次の要求を返し、実際に送る際だけ `submit(request, version)` を使う。発行済みの同じ要求では元のチケット（`fresh=false`）が返るので、再送しない。
- `observe(token, newer_version, ack, attributed)` は一つの未確定操作に厳密に新しい観測だけを紐づける。適用・不適用とも、対象操作に帰属すると呼び出し側が確認した場合だけ確定する。不明な応答は保留のままにする。
- 順序内で保留しても後続を続けられる、かつ安全に保留タスクに戻れる、と呼び出し側が確認したものだけを有限のFIFOへ移す。保留分は通常順序を完了してから処理する。
- 中止を要求すると、新しい通常作業・手直しは止まる。未確定操作が解決した後は退避を発行でき、別の新しい安全確認で完了する。発行済みの操作を取り消したり、経路や合法性を判定したりはしない。

観測版は呼び出し側の単調増加番号、トークンはそのインスタンス内の番号。コピーから同じ実環境へ送らない。他の人の操作で同じ変化が起きうる場合は、物理状態の一致だけで帰属を確認しない。不明が続く場合の打ち切り方や、安全な退避行動は用途側で持つ。すべての合法な操作が必ず反映される問題では、単純な添字で十分。

## 再現

リポジトリのルートで実行する。

```sh
flags="-std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all -Ilibrary"
g++ $flags tests/observed_task_cursor_test.cpp -o /tmp/observed_cursor_test
/tmp/observed_cursor_test
g++ $flags examples/observed_task_cursor_usage.cpp -o /tmp/observed_cursor_usage
/tmp/observed_cursor_usage
```

人工の小規模全列挙・固定乱数によるランダム操作列で、別に書いた参照モデルと10,763,380回の状態遷移が一致。使用例では拒否・保留・第三者起因の観測・再試行・安全退避の確定を一続きで確認した。コンテスト入力や採点結果には依存しない。

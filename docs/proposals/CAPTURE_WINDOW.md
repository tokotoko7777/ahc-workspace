# 有限時間の初到達確率

同じ遷移グラフで、開始地点から移動を続ける対象が、監視時間帯に指定集合へ一度でも入る確率を求めます。終了ターンだけの滞在確率や、各ターンを独立とした計算とは異なります。

```cpp
#include "capture-window.hpp"
#include <vector>

std::vector<std::vector<int>> graph{{1}, {0, 2}, {1}};
ahc::CaptureWindow scratch(graph.size());
// 0から1ターンに2回移動。第2〜5ターンの終了時に2にいれば成功。
double p = scratch.probability(0, 2, 2, 5,
    [&](int x) -> const auto& { return graph[x]; },
    [](int x) { return x == 2; });
```

隣接リスト各要素を等確率で選択します。同じ行に重複があれば重複分の確率が加わり、空行は停止です。すべての遷移先は 0 以上、頂点数未満です。指定範囲は両端込み、時刻0は初期位置です。遷移の途中では成功判定をしません。受付集合は固定で、結果は浮動小数点近似です。時間で遷移が変わる場合や複数対象の同時捕獲には直接使えません。

毎回全頂点を走査せず、確率質量がある頂点だけを列挙します。作業メモリは頂点数に比例し、同じグラフサイズで繰り返し利用できます。

`g++ -std=gnu++23 -O2 -Wall -Wextra tests/capture_window_test.cpp -o /tmp/capture-window-test && /tmp/capture-window-test`

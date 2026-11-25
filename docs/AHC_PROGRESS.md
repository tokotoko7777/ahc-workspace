# AHC 過去問研究の進捗

用途から試作部品を引くには [研究ライブラリの用途別索引](RESEARCH_LIBRARY_INDEX.md) を参照。

**現在の進捗（2026-11-21 JST）:** AHC007は本番同一150件で **14,248,878,730**、本番1位を+7,144,789上回った版を [PR #4](https://github.com/tokotoko7777/ahc-workspace/pull/4) でmainに統合。配布・別シードの点数は本番比較と区別する。

対象は終了済み AHC001〜AHC074。原則として番号順に着手し、長期戦や比較条件待ちの回はベストと次の仮説を保存して先へ進む。公式解説・参加者解説・第三者の提出コードは参照しない。


## 初回の実装・検証を一巡（2026-11-21 JST）

- AHC001〜074の全74回を実装・検証。別seedでの改善と本番比較を分けて記録。AHC009・011は既存出典の取扱い確認待ちで、検証済みの手元コミットを保持し、push保留。
- 本番1位と同一ケース・採点で比較できた16回は未達。最も差が小さいのはAHC003（首位から0.033699%）とAHC001（同0.041435%）。この2回を優先して構造から見直す。AHC013は自身の本番2,000件は検証済みだが首位点を未確認で、比較済み16回に含めない。
- **初回一巡時点の本番1位超えの達成確認は0件。** AHC004の後日のmain統合は、配布ケースでの別目標と正午の個別指示による。 未判定は未達と区別する。引き続き公式ケースで順位を検証でき、差を縮められる回を優先する。

## AHC008 のローカル研究（2026-11-25 JST）

- **翌02:20 犬の2ターン先を読んで先回り:** 公式配布100件で **6,191,777,779→6,142,055,558点／全頭100→99件／全員共有100件／両立100→99件**。先回りを空き担当に限っても **6,188,555,557点／両立100件** で、どちらも不採用。最大の悪化例は、6人が共有する面積が533→518マス、残り0→猫2頭。差44,833,333点のうち面積差は1,666,666点、捕獲漏れは43,166,667点。275ターン終了時から通常・追い込みの両方を公式対話で再生し、試作を通常に戻すと14,388,889→29,833,333点だったが、採用版の59,222,222点には戻らなかった。中盤の配員と終盤の選択は別に調べる。
- **翌02:21 予測だけを動かす対照:** 採用版の操作と得点を配布100件で完全再現し、犬の安全な入室に非ゼロの確率がついた33ケース221件を記録。2ターンとも柵の予測が合ったのは162件。2ターンとも人の位置まで合ったものは0件で、入室確率が正しく評価できたとは言えない。次は同じ採用版の序盤と中盤を保ち、275ターン終了後に全頭・共有の同時成功の多い案を選ぶ基準を実測する。

- **翌01:43 分断を最後の1手では救えない場合:** 人5〜6人・動物16頭以上の検証済み生成100件を再比較（終盤の比較試行は24組上限）。最終的に全頭捕獲したが人が分かれた5件は、すべて最後の捕獲と同じターンに初めて分断。5件を公式再生し、各人が合法な隣接柵または任意の1歩を選ぶ全候補を調べても、その場で両立できる手は0。元の切り方の多数側まで、外側の人はケースにより最長6〜29歩必要。捕獲前に必要な人を合流側へ回す必要がある。
- **翌01:48 先回りの比較:** 250〜274ターンで、捕獲が残り1〜2頭かつ後の柵で分断されそうなときだけ、待機中の人を合流側へ動かした。配布100件は **6,191,777,779→6,187,666,669点／両立100件を維持**（改善1件・悪化3件）。上の少人数100件は **5,041,580,786→5,045,136,341点**、全頭92→91件／共有95→96件／両立87件のまま。犬のそのターンの動きを変えないよう厳密に制限した対照版も、制限なしの先回り版と比べ、両パネルでケース別得点と達成指標は同じだった。捕獲悪化があるためいずれも不採用。
- より早く配置できる担当交換だけを許した試作も、配布100件 **6,129,444,446点／両立99件**。失敗例では4マス区画へ人が早く着いたにもかかわらず、その時点で捕まえられたはずの犬が区画を通らなかった。人の到着時刻だけを守っても捕獲の機会までは守れないため不採用。次は、観測した犬の移動から追っている人を絞り、1手先の予測に反映できるか採点する。

- **翌01:27 全頭捕獲・全員共有が確定したら探索を打ち切る:** 1つの連結成分に全員がいて、そこに動物がいなければ、以降の移動や柵の設置ではその共有空間を広げられない。同時に作る最後の柵の部分集合も確認。配布100件は **6,191,777,779点／全頭100件／共有100件／両立100件**、未使用の生成100件も **6,144,888,894点／全頭99件／共有100件／両立99件** のままでケース別得点まで全件一致。省ける最後の柵は両パネルで0。直列ペア測定のソルバーCPU合計は配布60.57→59.70秒、未使用51.16→49.97秒。性能上の採用で、得点更新ではない。
- **翌01:26 捕獲担当の最適化は不採用:** 一人・二人で行う捕獲から、今すぐ閉められる区画を残しながら代理評価が最大となる割り当てに変更。配布100件 **6,191,777,779→6,126,888,890点／全頭・全員共有・両立はいずれも100→99件**、改善9件・同点77件・悪化14件。同じ配布ケースを公式ツールで再生すると、160ターン目に二つの区画の担当を交換した例では、一方の入口が閉まったのが200→223ターン。最終的に2頭残り、5人が4人と1人に分かれた。この遅れだけで全悪化を説明できるとは限らない。既存の担当を一切動かさず空き人員だけで追加する対照実験は全100件で追加0。次は、先に割り当てた区画へ同じか早いターンに着ける交代だけを計測。
- 配置1の仕切りを1か所だけ入れる案は **6,121,655,557点／全頭99件／全員共有98件／両立97件**。両版で両立した同じ97ケースに限っても最小共有面積438→473マスだが、平均559.00→558.74マスで、両立を失った3件があるため不採用。

- **翌00:16 配置1を実際に採点:** 採用版の同じ配布100件を再生し、配置1の19件は共有面積が平均 **504.4マス**、設置済み柵310.9マス、動物がいる囲い84.6マスと内訳を確認。準備用の柵を290→264枚に減らし、左の担当通路と中央の間に1×4を14室作った試作を、設営時間の目安だけでは落とさず本採点した。配布100件 **6,191,777,779→6,006,305,557点／全頭捕獲100→96件／全員共有100→100件**。変えた19件で7件改善・12件悪化。全頭捕獲の悪化があるため不採用。次は失敗例を再生し、設営の遅れ・左の新しい囲い・既存の担当作業を切り分ける。[到達対象を保つ壁の間引きの追加部品](proposals/PRESERVE_REACHABILITY.md)は8,000グラフの全列挙と一致し、別の少人数100件で組み込み前後の全操作・得点一致。

- **23:38 狭い囲いへの2手目:** 既に安全と判定した内寄せを、動物が2ターンで新しい柵の近くに来られず、隔離対象も変わらない場合に限ってさらに1マス奥にした。配布100件は **6,191,777,779→6,193,444,445点／両立100件**、共有面積の最小438→453マス。新規生成100件では **6,123,866,669→6,123,200,002点**、全頭捕獲98件・全員共有99件・両立97件は同じ。成功した4回の内寄せは合法だったが、1件で最後のウサギを追い込むために広い区画を手放した。現ベストには採用せず、追加の1手が既存作業に重なるなら従来の閉鎖に戻す案を検証中。

- **23:14 上端への先回り:** 同じ担当レーンの空き時間を使い、上端の3〜4マス区画へ最大7手で先回り。調査済みの少人数100件では終盤の探索回数を揃えて **5,041,580,786→5,085,108,565点／全頭捕獲92→93件／全員共有95→95件**。改善例は上端ではなく、既存の囲いで全頭を捕獲。未使用の同条件100件では **5,131,625,000→5,080,583,334点／全頭捕獲88→86件／全員共有100→100件**。上端で1頭の捕獲を確認したが、別の捕獲を落としたため不採用。次は同じレーンを代わりに見張る人が既にいる場合に限定する。現採用版の配布100件 **6,191,777,779点／両立100件** は変更なし。

- **23:00 盤面の追試:** 現採用版は配布100件で **6,191,777,779点／全頭捕獲100件／全員共有100件** のまま。下端に縦長1×3を作るため既存の柵2枚を入れ替えた試作は **6,124,500,000点／全頭捕獲99件／全員共有100件**。捕獲を1件落としたため不採用。上端では既存柵だけで下から閉じられる3マス区画とその4マス拡張を確認。少人数・多動物の100件で観測した動物入り・設置合法は51区画ターンだったが、従来の3手以内・待機中の人だけを使う試作では移動・捕獲を追加できず、全操作が一致。次は担当者の実経路を測り、担当レーンを離れず通常の捕獲作業を優先する先回りを試す。[入口と担当レーンのライブラリ改修案](proposals/RAIL_GATE.md)。

- **22:04 少人数向け分岐を採用:** 終盤に、実際の柵で全員が分断されるときだけ、同じターンに隔離できる動物を変えず柵の一部を保留。その合流口ができてから、人を多数側に誘導する。配布100件は全操作一致で **6,191,777,779点／全頭捕獲100件／全員が同じ空間100件**。先に見た同条件の100件ずつでは、両立88→89件と94→95件。
- 採点前に条件（人5〜6人・動物16頭以上）だけで選んだ**新しい公式ジェネレータ100件**では、合計 **5,074,232,868→5,089,321,527点**、全頭捕獲90→92件、全員が同じ空間93→97件、両立87→89件。捕獲・分断のどちらもケース別の達成→未達はなし。既に未達の一部ケースでは個別得点が下がった。配布・本番採点とは別パネル。本番順位は未判定。

- **21:30 少人数の難条件:** 公式ジェネレータから、採点前に「人5〜6人・動物16頭以上」だけで選んだ100件を診断。採用版は全頭捕獲90件・全員が同じ空間98件・両立88件。終盤に人を分断する同時設置から、捕獲対象はそのターンに変えず、全員をつなげるために不要な柵だけ省く試作は、この100件で **5,141,738,888→5,171,227,777点／両立88→89件**。2組に分かれていた5人が481マスを共有した例を公式リプレイで確認。配布100件では全操作・スコアが一致。同じ条件で選んだ別の未使用100件は **5,342,953,704→5,328,194,444点／全員共有99→100件／全頭捕獲95→94件**。1件で分断は防いだが最後のウサギが残り、未採用。二つの少人数パネルは配布100件・本番採点とは別。

- **柵を2か所同時に内寄せ:** 公式配布100件では **6,196,111,111点／両立100件**、最小共有面積438→481マスの試作。別の未使用100件は両立93件のままだったが、実際は1件救済・1件捕獲失敗（−39,583,333点）で入れ替わり、総得点が下がったため未採用。全頭捕獲が確定する同時内寄せに絞ると発動0。配布100件の現採用版は変わらず **6,191,777,779点／両立100件**。

- **20:51 終盤の比較:** 採用基準は引き続き公式配布100件 **6,191,777,779点／全頭捕獲100件／全員が同じ空間100件**。共有面積を重く見た切り方を試したところ、種類で切り替える版は配布 **6,198,777,780点・両条件100件**、最小438→451マス。ただし未使用100件では全頭捕獲97→93件（4件すべて最後の牛を追わない選択に変化）、点数も6,132,333,337→6,028,611,113。追い込みに切り替える判断を従来どおりに戻し、実際の柵だけ変えた版は配布 **6,196,000,001点・両条件100件**。別の未使用100件は5,811,179,631→5,809,290,741、全頭捕獲は93件同数だが個別に1件救済・1件失敗。いずれも未採用。次は、そのターンに全頭捕獲と全員が同じ空間を確定できる代替手だけを採る。

- [閉鎖マスを最大流に入れない汎用提案](proposals/GRID_VERTEX_CUT_PRUNING.md)は、固定探索量の全100件で全操作・得点が一致、終盤CPU約20.5%減。通常の時間枠では探索1,036→1,328組。一方、追い込みに切り替わった実例で全頭捕獲が1ターン遅くなり、共有面積562→552マス。そのまま既定解には採用せず。高速化の処理と、探索結果を採用する判断を分けて検証する。

- **20:19 高速化の採用:** 終盤の到達範囲を、人別BFSの和集合から距離上限つき多始点BFSに変更。[ライブラリ改修提案・使用例・テスト](proposals/BOUNDED_MULTI_SOURCE_BFS.md)。公式配布100件は **6,191,777,779点／全頭捕獲100件／全員が同じ空間100件** で全操作一致、比較シミュレーション993→1,020組。24組固定では終盤探索17件のソルバーCPU合計22.79→21.81秒（約4.3%削減）。新しい独立100件も全操作一致で **6,020,472,221点／全頭捕獲95件／全員が同じ空間100件**、比較911→931組。別々の100件パネルを相互比較しない。次は、すでに閉じたマスを最大流グラフから外してさらに探索量を増やす。

- **不採用の切り分け:** 最終捕獲ターンの柵を142通り減らして検査したが、全頭捕獲と全員共有を保てる省略は0。両口の先回りで2件閉鎖した試作は点数・面積差なし。壁が小部屋から動物を締め出した後に置き換えやすい静的候補も634捕獲ターンで有効0。[設営方向の対称変換の提案](proposals/GRID_ISOMETRY.md)を分離して保存。無動物の設営が5〜9手早まりそうな6件だけ向きを変えた試作は6,185,333,333点（2件改善／4件悪化）、配布100件で両条件100を維持したが不採用。

- **19:03 採用更新:** 最後の1頭を囲う直前に、より小さな区画だけを切り離せる1マスの候補を探す。作業員が外側から到着するまでに動物が柵の隣へ届かないことを、現在の盤面と最大移動距離で確認してから採用。公式配布100件で **6,191,777,779点／全頭捕獲100件／全員共有100件**、最小共有面積431→438マス。未使用100件は **6,031,277,777→6,032,055,555**、全頭捕獲98件・全員共有100件を維持し、改善2件・悪化0件。

- **18:10 採用更新:** 閉鎖マスをずらす判定を全配置に拡張し、公式配布100件 **6,191,000,001点（配置1限定版比+2,333,334）／全頭捕獲100件／全員共有100件**、共有空間の平均557.19・最小431マス。新しく生成した未使用100件では **6,205,333,337→6,210,666,671**、全頭捕獲99件・全員共有100件で両版同じ、ケース別に捕獲・共有の悪化なし。犬の最大移動を厳密化する追加案は、配布100件で安全に閉鎖できた回数が67→92でも **6,188,444,444点** に下がったため未採用。

- **18:05 採用更新:** 配置1の捕獲区画で、動物が1ターンで新しい閉鎖マスやその隣へ届かないときだけ、片側の閉鎖位置を1マス内側に移動。公式配布100件 **6,188,666,667点（前版比+10,999,999）／全頭捕獲100件／全員共有100件**、共有空間の平均556.98・最小431マス。別の未使用100件は **5,981,603,177→6,005,492,066**、全頭捕獲95件・全員共有99件で両版同じ、該当指標のケース別悪化なし。これらのローカル集計は本番順位と別扱い。

- 公式配布100件を同じ入力順で比較。従来の作業版は **6,170,583,335点／全頭捕獲100件／全員共有99件**。終盤に、閉鎖すると動物のいない1〜4マスへ分断される人を先に出口へ動かす版は **6,177,666,668点／全頭捕獲100件／全員共有100件**。両条件達成の共有空間は平均555.99、最小461マス。変更は1件で全員が540マスを共有した。
- 未使用の公式ジェネレータ100件では動作列が全件一致し、両版とも **6,098,222,222点／全頭捕獲99件／全員共有100件**。この追加100件には回避策の発動例なし。従来の同じ配布100件での最高素点版は **6,186,694,447点／両条件99件**、両条件100件の従来最良は **6,176,000,004点**。本番同一ケースでの順位は未判定。下表の「公開検証済み基準」は公開コード付きの別版。
- CPU型の内側に両端閉鎖の1×4を追加した案は、配布100件中のCPU型適用46件で両条件達成7→9件。しかし別の未使用100件中の適用49件で3→2件となったため未採用。次は全頭捕獲を維持しつつ、終盤に仕切る空間を小さくして共有面積を増やす。

## 1 位超えの判定

公式最終 1 位を、同じ対象ケース、公式の採点・丸め・集計・有効性条件で**厳密に**上回った場合のみ「達成」。相対評価は両者に同一のケース別基準を使う。本番相当を再現できなければ「未判定」。同点や別シードでの改善を達成と混同しない。

| 問題 | 状態 | 本番最終 1 位 | 現在の基準値 | 比較条件 | 問題別ブランチ |
|---|---|---|---|---|---|
| [AHC001](https://atcoder.jp/contests/ahc001/tasks/ahc001_a) | 本番 1,000 件完走・未達 | **993,823,071,683** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc001.zip)） | 凍結 v64: **993,411,282,631** 点 | 本番公開シード全 1,000 件・公式採点。最長 4.838 秒（ローカル） | [codex/ahc001-coordinate-slide-20261120](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc001-coordinate-slide-20261120) |
| [AHC002](https://atcoder.jp/contests/ahc002/tasks/ahc002_a) | 既存版から着手・未判定 | **6,255,977** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc002.zip)） | V10（未調整 20 件 1,254,587） | 本番 100 件。通常配布の 0–99 は別ケース | [codex/ahc002-corridor-repair-20261120](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc002-corridor-repair-20261120) |
| [AHC003](https://atcoder.jp/contests/ahc003/tasks/ahc003_a) | 本番 3,000 件完走・未達 | **2,926,767,743,437** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc003.zip)） | 凍結既存版: **2,925,781,453,112** 点 | 本番公開シード全 3,000 件・公式対話テスタ・最大ローカル 0.897 秒 | [codex/ahc003-probe-design-20261120](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc003-probe-design-20261120) |
| [AHC004](https://atcoder.jp/contests/ahc004/tasks/ahc004_a) | 旧目標達成・90億点は未達、[PR #3でmainに統合](https://github.com/tokotoko7777/ahc-workspace/pull/3) | 配布100件の目標 **9,000,000,000**（本番順位とは別） | 単一C++23 **8,996,036,656**（目標差3,963,344）、全語収録56件 | seed 0–99、公式・独立一致、外部最適化フラグなし、最長2.788662秒。同一バイナリ先行8,994,781,021。別の未使用100件は8,714,472,941（前版比+73,111,703） | [最終単一C++23版](https://github.com/tokotoko7777/ahc-workspace/blob/851552a10dc1ac63210597ebaf5aa928b54a9073/problems/ahc004/experiments/medium_mixed_extend/submission.single.cpp)、[検証・再現](https://github.com/tokotoko7777/ahc-workspace/blob/7f811080ac468f1ddf53e80cdf327b0e6dc7d98a/problems/ahc004/experiments/local100/README.md) |
| [AHC005](https://atcoder.jp/contests/ahc005/tasks/ahc005_a) | 配布100件で目標達成。13:00切替に備え最良版凍結 | 配布目標 **22,318,770**。本番値の出典: [AtCoder提供データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc005.zip) | 2スタート版 **23,020,218**（目標+701,448、基準+96,961）、全件完全可視・帰着 | 配布seed 0–99、公式・独立一致、最長2.812822秒。本番100件とは別。追加未使用100件は22,122,939 | [最良ソース・比較・再現](https://github.com/tokotoko7777/ahc-workspace/tree/6d9d59bf76cb921bba15c12e19ec6f6188bdb401/problems/ahc005/experiments/official100) |
| [AHC006](https://atcoder.jp/contests/ahc006/tasks/ahc006_a) | 公式配布100件で目標達成、未使用100件で検証 | 配布目標 **2,220,425点超え**（[本番1位の参照値](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc006.zip)） | 幾何候補＋厳密な前計算高速化 **2,238,273**（目標+17,848、基準+159,865） | 配布seed 0–99、合法・公式独立一致100件、最長1.856995秒。本番とは別。未使用別100件2,226,481（比較版2,230,236） | [最良コード・ケース別・再現](https://github.com/tokotoko7777/ahc-workspace/tree/cbebfc73c5260374eb0af7c76fe6f10e0754c32a/problems/ahc006/experiments/official100)。配布最良は研究枝に保存。 |
| [AHC007](https://atcoder.jp/contests/ahc007/tasks/ahc007_a) | **本番同一150件で首位超え**・[PR #4](https://github.com/tokotoko7777/ahc-workspace/pull/4) main統合済 | **14,241,733,941** 点（[AtCoder提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc007.zip)） | 符号付きボトルネック推定 **14,248,878,730** (+7,144,789)。公式配布150件 **14,255,819,331** (基準+71,513,945) | [本番同一ケース根拠](https://openreview.net/pdf?id=JCjGvbsOmQ)。本番150件を凍結版で盲検し、対話tester・vis・独立採点一致、最長0.308726秒。未調整別150件も14,249,050,741 | [main上の最良コード・再現](https://github.com/tokotoko7777/ahc-workspace/tree/9fb81532ac1f2092309b1e2887756e6c590c9c4d/problems/ahc007)・[研究枝](https://github.com/tokotoko7777/ahc-workspace/tree/a545fe5aa083eac8a6c6bf4533147df1840a8596/problems/ahc007/experiments/focus)。次の仮説: 安い辺の買いすぎと棄却コストの差を、疎な代替カットに限り厳密化。 |
| [AHC008](https://atcoder.jp/contests/ahc008/tasks/ahc008_a) | ローカル改善・本番未判定 | **121,980,050,607** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc008.zip)） | 公開検証済み基準・公式配布100 **6,152,805,559**・**全頭100/100**（残留0）。[採用コード・ケース別検証](https://github.com/tokotoko7777/ahc-workspace/blob/54ac18edd37c60c891ade6f72f2438c27ab4141a/problems/ahc008/experiments/20261122/article-capture/terry-incoming-room/VALIDATION.md)。目標6,263,333,336（[ユーザー提示の採点結果](https://atcoder.jp/contests/ahc008/submissions/29654603)） | 公式対話テスタ v3、配布100と本番2,000は別扱い。両100加算せず別集計、最大2.772秒（採用版の通常設定3パネル、ローカルテスタ最大を切上げ） | [解答と実験の作業ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc008-article-capture)・[再現と検証](https://github.com/tokotoko7777/ahc-workspace/blob/54ac18edd37c60c891ade6f72f2438c27ab4141a/problems/ahc008/experiments/20261122/article-capture/terry-incoming-room/VALIDATION.md)。次: 全頭捕獲を保ちながら、完成した区画から安全に捕獲を始める。設営完了の時刻だけでなく、実際に守備位置へ着いて合法に閉鎖できる機会と、捕獲までの待ち時間を区別する。関連する独立提案: [進入方向マスク](MUTABLE_GRID_ENTER_MASK_PROPOSAL.md)・[固定グラフ向け CSR 最大流](ORDERED_CSR_MAX_FLOW_PROPOSAL.md)・[目的地グループへの距離表](GRID_TARGET_DISTANCE_PROPOSAL.md)・[複数人作業の割当](GROUPED_MASK_PACKING_PROPOSAL.md) |
| [AHC009](https://atcoder.jp/contests/ahc009/tasks/ahc009_a) | 着手・未判定 | **7,547,834,181** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc009.zip)） | 確率 DP 版: 公開 100 件 7,214,943,739 | 本番との同一性は未確認 | ローカル保存・公開確認待ち |
| [AHC010](https://atcoder.jp/contests/ahc010/tasks/ahc010_a) | ローカル改善・本番未判定 | **20,218,968** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc010.zip)） | 協調回転版: 未調整 10 件平均 56,482 | 本番 100 件、2 秒。配布入力との同一性未確認 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/8e99892f7dc12317ba365750a15a2a7c072b3731) |
| [AHC011](https://atcoder.jp/contests/ahc011/tasks/ahc011_a) | ローカル改善・本番未判定 | **2,691,447,336** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc011.zip)） | 局所回転追加: 未調整 15 件 6,060,235 | 本番公開 3,000 シード、3 秒 | ローカル保存・公開確認待ち |
| [AHC012](https://atcoder.jp/contests/ahc012/tasks/ahc012_a) | ローカル比較・本番未判定 | **99,781,619** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc012.zip)） | 格子版: 未調整 8 件平均 972,456.375 | 本番 100 件・3 秒。同一性未確認 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/dc1a18935430728a5dbca82c14f8bc84c57145e1) |
| [AHC013](https://atcoder.jp/contests/ahc013/tasks/ahc013_a) | 本番 2,000 件完走・順位未判定 | [公式順位](https://atcoder.jp/contests/ahc013/standings) 要ログイン。1 位の点数未確認 | 凍結隣接移動版: **6,951,707** 点 | 本番公開 2,000 シード、3 秒 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/7526fecad74457ce05f786e5127c4e45c0357203) |
| [AHC014](https://atcoder.jp/contests/ahc014/tasks/ahc014_a) | ローカル改善・本番未判定 | **3,122,399,998** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc014.zip)） | 再成長版: 未調整 6 件平均 880,722.83 | 本番公開 2,000 シード、5 秒 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/25c2dea8a3d5f9b07432ee1a2b239cf4bd820cff) |
| [AHC015](https://atcoder.jp/contests/ahc015/tasks/ahc015_a) | ローカル改善・本番未判定 | **161,011,437** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc015.zip)） | 6 手先: 未調整 24 件平均 663,091.75 | 本番 200 件、2 秒。公開入力との同一性未確認 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/a2b1c24d2d58f6e42db8475e62eb7ae00ef9373e) |
| [AHC016](https://atcoder.jp/contests/ahc016/tasks/ahc016_a) | ローカル改善・本番未判定 | **1,780,676,618,015**（相対点） | モデル選択版: 別の未調整 40 件平均 42,457,716.95（絶対点） | 本番 2,000 件・5 秒、ケース別の当時の基準点が必要 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/b84d5b74efaa1410549101730df5a60c1cc3102e) |
| [AHC017](https://atcoder.jp/contests/ahc017/tasks/ahc017_a) | 着手・未判定 | **1,993,227,663,417**（相対点） | 未調整 14 件 189,307,900（絶対点） | 本番 2,000 件・6 秒、ケース別基準が必要 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/6078ab465ffd5a225a464187bd443e4b62fcd38b) |
| [AHC018](https://atcoder.jp/contests/ahc018/tasks/ahc018_a) | ローカル比較・本番未判定 | [公式順位](https://atcoder.jp/contests/ahc018/standings) 要ログイン。1 位の点数未確認 | 別の未調整 32 件で経路限定試掘／一様の比: 平均 0.9202、25 勝 | 本番公開 3,000 件、5 秒、相対方式 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/ff72f9a9577cff6a5ce466081ef959cdde836522)、[経路試掘](https://github.com/tokotoko7777/ahc-workspace/tree/0cca8b704bae33b8ad70a32d5db6ae139fb5c054) |
| [AHC019](https://atcoder.jp/contests/ahc019/tasks/ahc019_a) | 着手・本番未判定 | **1,886,240,066,109**（相対点） | 新規未調整 30 件 30,684,752,148（絶対点・低い方が良い） | 本番公開 2,000 件、6 秒、ケース別基準が必要 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/7252e85e2d8a5d75038c0680f4ba43b4a43b3f05)、[部分再構築](https://github.com/tokotoko7777/ahc-workspace/tree/79fa2df9de9de4de4d658ea3f374ff983d7e4753) |
| [AHC020](https://atcoder.jp/contests/ahc020/tasks/ahc020_a) | 着手・本番未判定 | **504,832,157**（絶対点） | 未調整16件 平均1,687,384.19 | 本番 300 件・2 秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/96a97144e57022f452ce216eddf9c2d76adc3b32)、[配線費の共同評価](https://github.com/tokotoko7777/ahc-workspace/tree/411425e2f761d00f879c4965b8eee3b9e959e56a) |
| [AHC021](https://atcoder.jp/contests/ahc021/tasks/ahc021_a) | 着手・本番未判定 | **13,589,420**（絶対点） | 交換削除：新規未調整20件 1,814,625 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/9306c3d973e9b622e3a35431363f5f7f6ab64225)、[交換削除](https://github.com/tokotoko7777/ahc-workspace/tree/ce5987e9b2769010a4568b3442108f2e02e29909) |
| [AHC022](https://atcoder.jp/contests/ahc022/tasks/ahc022_a) | 凍結版：本番未達 | **2,808,216,532,758**（相対点） | **271,023,954,031**（本番3,000件・全件誤対応0） | 本番3,000件・4秒、公式アーカイブと本番seedの列対応を確認 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/d255c7a5ce936552a98346c8d2bf764f78add849)、[低配置費](https://github.com/tokotoko7777/ahc-workspace/tree/86f0b8b8dc3d75a877626672b36b2dec88da2e05)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/8e7f5ef9c7866da0db52954c549e62fa5fc5c791) |
| [AHC023](https://atcoder.jp/contests/ahc023/tasks/asprocon10_a) | 凍結初版：本番未達／改良版：未判定 | **1,782,120,500**（[公式採点アーカイブ](https://img.atcoder.jp/ahc_standings/index.html?contest=ahc023)） | 本番公開2,000件 **1,100,483,250**（全件合法） | 本番公開2,000件・2秒、絶対点 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/52f76e1f5e905e5f4f11f0613285760073967281)、[植付と収穫の順序](https://github.com/tokotoko7777/ahc-workspace/tree/2b4a9df1aa5ad9cdc0d435bb8271d669d1fe43a6)、[区間流](https://github.com/tokotoko7777/ahc-workspace/tree/ac291d5344ff4f4eca283e694da5ba3737b14218) |
| [AHC024](https://atcoder.jp/contests/ahc024/tasks/ahc024_a) | 着手・本番未判定 | **335,417**（絶対点） | 再始動版：新規未調整24件 平均2,065.79 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/c5b3b9f3d88b74ff5f9fa67446d16555dd509ef1)、[多様な再始動](https://github.com/tokotoko7777/ahc-workspace/tree/a0aa3ad5ff0cf55d5f3f6ea9c8714d8ca19bcfe9) |
| [AHC025](https://atcoder.jp/contests/ahc025/tasks/ahc025_a) | 凍結版：本番未達 | **492,832,027,444**（相対点） | **354,364,239,662**（本番5,000件、当時の分布を固定） | 本番5,000件・2秒、各参加者のケース別結果・公式seedを照合済み | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/21034a6404bafe091bac193b8315b0ffd1ed0686)、[低Q追試](https://github.com/tokotoko7777/ahc-workspace/tree/93cfb59980bf24166658b1b40f60363cd8e63cf3)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/a0f8cbdebfed6d111fead325f78dfed18fe21923) |
| [AHC026](https://atcoder.jp/contests/ahc026/tasks/ahc026_a) | 着手・本番未判定 | **1,423,183**（絶対点） | 既存法：未調整16件 平均9,344.63 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/9d10bdadaa08a7ca84e01228951b4e965082f00b)、[途中状態の追試](https://github.com/tokotoko7777/ahc-workspace/tree/22fe2ca8760d43489acbd895b3d63a1c9933963f) |
| [AHC027](https://atcoder.jp/contests/ahc027/tasks/ahc027_a) | 着手・本番未判定 | **1,996,375,186,737**（相対点） | 新規未調整20件 275,950,662（絶対点・低い方が良い） | 本番2,000件・2秒、ケース別基準と公式seedの列対応を確認 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/3ec2d6d02ab6aea5b461002201803016ce56ecb8)、[訪問間隔の中央を狙う版](https://github.com/tokotoko7777/ahc-workspace/tree/ba4622d04a7fede9644fee8bdf0408a364eaa1ec) |
| [AHC028](https://atcoder.jp/contests/ahc028/tasks/ahc028_a) | 着手・本番未判定 | **1,105,471**（絶対点） | 厳密編集：未調整24件 平均7,139.67 | 本番150件・2秒、同一ケースの証拠なし | [初回](https://github.com/tokotoko7777/ahc-workspace/tree/84d5508c4284fa8e75116f1a038a8124946246ac)、[厳密編集](https://github.com/tokotoko7777/ahc-workspace/tree/a9f558c6c273ccb8f26433e8554a5e1aa8a2dc8d) |
| [AHC029](https://atcoder.jp/contests/ahc029/tasks/ahc029_a) | 着手・本番未判定 | **2,200,575,652,801**（相対点） | 基準版：未調整28件 5,802,068（最終資金合計） | 本番3,000件・2秒、公開採点列と公式seedの対応を確認 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/400b9b1a8b5f302c0d5eccacc48953bed87484bd)、[増資後の追試](https://github.com/tokotoko7777/ahc-workspace/tree/94c34b158aa59b9a2d75153bd452ffa67faab2bd) |
| [AHC030](https://atcoder.jp/contests/ahc030/tasks/ahc030_a) | 凍結版：本番未達 | **2,140,937,483,257**（相対点） | **323,222,495,387**（本番3,000件・全件初回正答） | 本番3,000件・3秒、ケース別基準・公式seedを照合済み | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/ff4f0b02b3b5d35bb7a63748604649304e7c5d15)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/3dfdfcce6f81b2b034351960f8d506b8cdba735b)、[試し掘り追試](https://github.com/tokotoko7777/ahc-workspace/tree/a285e872f70c776469b2fe2780c9664de40bcc2b) |
| [AHC031](https://atcoder.jp/contests/ahc031/tasks/ahc031_a) | 凍結版：本番未達 | **1,722,250,219,489**（相対点） | **126,909,397,521**（本番2,000件・全件合法） | 本番2,000件・3秒、ケース別基準とseed列照合済み | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/c1d15b819b45223d0c27bc3191918083c1b18362)、[横壁整列](https://github.com/tokotoko7777/ahc-workspace/tree/fb4acc4255f313b63cc46c95b3965fc983b2d528)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/fe55a2264364f3c6f25df1fe9b5e12ff314d4186) |
| [AHC032](https://atcoder.jp/contests/ahc032/tasks/ahc032_a) | 着手・本番未判定 | **11,854,207,127,046**（絶対点） | 未調整16件 平均79,106,180,582.44 | 本番150件、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/0f8a8d2946ce1c6691db7ce95eee5ffa8a51006e)、[方向の追試](https://github.com/tokotoko7777/ahc-workspace/tree/890cee4f0a8b60a076d3c98d6d98d6bffed43099) |
| [AHC033](https://atcoder.jp/contests/ahc033/tasks/ahc033_a) | 着手・本番未判定 | bowwowforeach、得点未確認 | 新規未調整24件 合計6,867ターン | 本番2,000件、相対方式 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/131ef411f491cc1b6ed79c905fc98a8d52ffad5d)、[先行搬出](https://github.com/tokotoko7777/ahc-workspace/tree/81b3ab227a32818d6d4350351c5d4db5c9602b0f) |
| [AHC034](https://atcoder.jp/contests/ahc034/tasks/ahc034_a) | 着手・本番未判定 | **9,486,350,384**（絶対点） | 経路短縮：新規未調整16件 平均43,686,637.44 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/6c2ea91bb86ef24c6c84cd99bf76df6355c16ce6)、[積み降ろし再配分](https://github.com/tokotoko7777/ahc-workspace/tree/e2c29731322790e744a17033d3fa937c450cede7)、[費用下限の診断](https://github.com/tokotoko7777/ahc-workspace/tree/c53951ab42ee40ce6a1347f820007bee1729502e)、[経路短縮](https://github.com/tokotoko7777/ahc-workspace/tree/5c08291e69e5a61ed806e8fb740108219f7401f9) |
| [AHC035](https://atcoder.jp/contests/ahc035/tasks/ahc035_a) | 着手・本番未判定 | **274,272,291**（絶対点） | 未調整36件 平均857,950.08 | 本番300件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/5e9d58aed81aae5dbdbe8563408ea91a0fbfdf57)、[終盤分布の追試](https://github.com/tokotoko7777/ahc-workspace/tree/503ea8770af2217fea4f8b9f3a866c144949ac60) |
| [AHC036](https://atcoder.jp/contests/ahc036/tasks/ahc036_a) | 凍結版：本番未達 | **1,872,786,019,642**（相対点） | **1,048,370,275,931**（本番2,000件・全件合法） | 本番2,000件・3秒、基準とseed列を照合済み | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/fb26d23b729e0e30e19da4d6181bd0421588196e)、[更新済み区間の利用](https://github.com/tokotoko7777/ahc-workspace/tree/5f1878dec204f611343c7e8b7a9066b7a0d8e3c3)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/2deddb2a4c35d9bfdbddaa21523042a8f17842c2) |
| [AHC037](https://atcoder.jp/contests/ahc037/tasks/ahc037_a) | 着手・本番未判定 | Rafbill、得点未確認 | 未調整28件 平均35,832,854.21 | 本番150件・2秒、同一ケースの証拠なし | [中継点](https://github.com/tokotoko7777/ahc-workspace/tree/11e02827a706e05022f50a3f7956541fa388f9ea)、[親の付け替え](https://github.com/tokotoko7777/ahc-workspace/tree/38ce41a25380bb0b6d5b409da8631270b8a3f62f) |
| [AHC038](https://atcoder.jp/contests/ahc038/tasks/ahc038_a) | 着手・本番未判定 | **1,986,808,184,140**（公式採点CSVの首位） | 既存ビーム：未調整22件 8,811ターン | 本番2,000件・相対方式 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/12ebdf8c71f19f11fac5b9c2053ae4267a053a18)、[初期解に利用する追試](https://github.com/tokotoko7777/ahc-workspace/tree/5cd3e84bba399d9fbccfc2c20273b840aacd02b5) |
| [AHC039](https://atcoder.jp/contests/ahc039/tasks/ahc039_a) | 着手・本番未判定 | **566,997**（絶対点） | 隣列重複：新規未調整24件 平均3,429.67（追加探索の対照と比較） | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/39ff40a8cce317a2ed00157ebb8cefefeeff0b16)、[隣列重複](https://github.com/tokotoko7777/ahc-workspace/tree/afa01fc73184f6fe316465a5c67f924057b6f4b9) |
| [AHC040](https://atcoder.jp/contests/ahc040/tasks/ahc040_a) | 着手・本番未判定 | **2,981,293,882,216**（相対点） | 棚配置：未調整32件 平均1,123,661.47（低い方が良い） | 本番3,000件・3秒、公式ケース別基準を取得 | [混合配置](https://github.com/tokotoko7777/ahc-workspace/tree/c22499e74fe82e2fb3069820be96bd5ebc00b7bc)、[線形測定](https://github.com/tokotoko7777/ahc-workspace/tree/fcf1bbf603140f033820232afe5fbc5dc7c01e05) |
| [AHC041](https://atcoder.jp/contests/ahc041/tasks/ahc041_a) | 着手・本番未判定 | hitonanode、得点未確認 | 未調整24件 11,915,002 | 本番150件・2秒、同一ケースの証拠なし | [部分木](https://github.com/tokotoko7777/ahc-workspace/tree/6e69ff15a72cc8d3b673248c4335c435bfb344be)、[複合修正](https://github.com/tokotoko7777/ahc-workspace/tree/3e6e4f9ea3ea3d9e65c0bd7e7a07e1e2fdccf908) |
| [AHC042](https://atcoder.jp/contests/ahc042/tasks/ahc042_a) | 着手・本番未判定 | **468,807**（絶対点） | 未調整32件 111.125手/件（全件完了） | 本番150件・2秒、同一ケースの証拠なし | [部分復元](https://github.com/tokotoko7777/ahc-workspace/tree/e8796e286c2e197c2f1f5324755e4fb58da8f3f5)、[垂直方向の事前操作](https://github.com/tokotoko7777/ahc-workspace/tree/98a9e65e998f57a10a8f0b3f2cf2514a0b73b4cf) |
| [AHC043](https://atcoder.jp/contests/ahc043/tasks/ahc043_a) | 着手・本番未判定 | **1,971,352,183,368**（相対点） | 未調整24件 最終資金合計63,926,853 | 本番2,000件・3秒、基準とseed列照合済み | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/2920bc508229a62f10b0070b68f3f1941fae75a4)、[初期ルートの追試](https://github.com/tokotoko7777/ahc-workspace/tree/46cbc1ff5548f82b99e16300ca1a5c3ca51984ca) |
| [AHC044](https://atcoder.jp/contests/ahc044/tasks/ahc044_a) | 着手・本番未判定 | **149,807,848**（絶対点） | 新規未調整24件 平均誤差約19,314 | 本番150件・2秒、同一ケースの証拠なし | [残差探索](https://github.com/tokotoko7777/ahc-workspace/tree/95185faa94950764129d78ae2422bd9b46da73e4)、[定常分布による追試](https://github.com/tokotoko7777/ahc-workspace/tree/39c6fccc3544603af106c420abf983f84ded74b1) |
| [AHC045](https://atcoder.jp/contests/ahc045/tasks/ahc045_a) | 凍結版：本番未達 | **2,988,855,588,054**（相対点） | **1,809,922,568,865**（本番3,000件・全件合法） | 本番3,000件・2秒、基準とseed列照合済み | [順序推定](https://github.com/tokotoko7777/ahc-workspace/tree/ef908953cb79d12f7f5f71ef4aaef53d45fc4c04)、[グループ交換](https://github.com/tokotoko7777/ahc-workspace/tree/f594e37f013614a169ff675440bfcfec1e63d219)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/8d199405dc9bbdc8821997230ee282b8b4af168a) |
| [AHC046](https://atcoder.jp/contests/ahc046/tasks/ahc046_a) | 着手・本番未判定 | **220,494**（絶対点） | 新規未調整30件 平均217.600手 | 本番150件・2秒、同一ケースの証拠なし | [停止ブロック](https://github.com/tokotoko7777/ahc-workspace/tree/00edc1ce4028926dcc2cfb8316e64994f3fe89bb)、[複数区間の探索](https://github.com/tokotoko7777/ahc-workspace/tree/8c258bffae710ee7b3764fa8f0da9e95d59a4f6d) |
| [AHC047](https://atcoder.jp/contests/ahc047/tasks/ahc047_a) | 着手・本番未判定 | itigo、得点未確認 | 新規未調整24件 得点平均26,281.33 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/e8aa0d41b813933b676a6b0cb442f1e51e7a92bc)、[共有する文字列経路](https://github.com/tokotoko7777/ahc-workspace/tree/62f3f7888cb37d53f9c45bc289e04078ccf857d2) |
| [AHC048](https://atcoder.jp/contests/ahc048/tasks/ahc048_a) | 凍結版：本番未達 | **1,129,730,769,846**（相対点） | **121,378,823,586**（本番2,000件・全件合法） | 本番2,000件・3秒、基準とseed列照合済み | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/ad4f7e6bcdfa76f80a9b92052ea060f72085dc06)、[容量3](https://github.com/tokotoko7777/ahc-workspace/tree/2264494b2ec7fa3fa9cc1161c9cc7d3891649bcb)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/bd0d1fd309af7f9ba834e6688b8391a6afa2c3af) |
| [AHC049](https://atcoder.jp/contests/ahc049/tasks/ahc049_a) | 着手・本番未判定 | Rafbill、得点未確認 | 新規未調整28件 平均4,861.64移動 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/a4688d135b0ecabf49fe877895782ced3e0aa5ca)、[土台箱を増やす追試](https://github.com/tokotoko7777/ahc-workspace/tree/508d161add61b6185f17ab86fd2ce819d164009c) |
| [AHC050](https://atcoder.jp/contests/ahc050/tasks/ahc050_a) | 着手・本番未判定 | yosupo、得点未確認 | 新規未調整20件 合計19,486,823 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/c1084c6814a6b1a98fa2b6de854a1a2941f09098)、[末尾の厳密評価](https://github.com/tokotoko7777/ahc-workspace/tree/369761882afe4a63f254cb9752a8563ce55f1697) |
| [AHC051](https://atcoder.jp/contests/ahc051/tasks/ahc051_a) | 凍結版：本番未達 | **1,721,504,233,142**（相対点） | **310,855,698,519**（本番2,000件・全件合法） | 本番2,000件、公式ケース別基準を取得 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/c7a55c1984e3da4992f23c94bdb1a5430db232c4)、[再合流](https://github.com/tokotoko7777/ahc-workspace/tree/15a685950c1423fb5e0dbeb864561b0f36e2e896)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/f5ac7efb491be927e38eaae17aeb4620a3c8a964) |
| [AHC052](https://atcoder.jp/contests/ahc052/tasks/ahc052_a) | 着手・本番未判定 | nikaj、得点未確認 | 未調整24件 57,831点 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/528b3ca276d534fb1ca3ad330d0ab12bb9e8e565)、[適応的なキー変更](https://github.com/tokotoko7777/ahc-workspace/tree/227a790ee8ddad8e2cd9f7af15cdea5fe6fc0612) |
| [AHC053](https://atcoder.jp/contests/ahc053/tasks/ahc053_a) | 着手・本番未判定 | yosupo、得点未確認 | 新規未調整3組×20件すべて改善（組平均約595〜596M） | 本番150件・同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/7f0257819434e8b706195937b14ee7f0b482b3ae)、[2枚組のデッキ](https://github.com/tokotoko7777/ahc-workspace/tree/2f27c9a55ccab9756b345063427fc7d0899fa92b) |
| [AHC054](https://atcoder.jp/contests/ahc054/tasks/ahc054_a) | 凍結版：本番未達 | **2,409,003,089,874**（相対点） | **419,674,926,324**（本番3,000件・全件合法） | 本番3,000件、公式ケース別基準を取得 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/cc2c7b4e86abc712f7ce27bf8a992175adb45ae1)、[行き止まりを追加](https://github.com/tokotoko7777/ahc-workspace/tree/5af1506cdf04eb57d5b756440c63cd02fadc44d6)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/f1e7725908d680be15c2dafaa5117f8eed29a688) |
| [AHC055](https://atcoder.jp/contests/ahc055/tasks/ahc055_a) | 着手・本番未判定 | nikaj、得点未確認 | 新規未調整28件 1,559,892（137,988手） | 本番150件・2秒、同一ケースの証拠なし | [使用回数割当](https://github.com/tokotoko7777/ahc-workspace/tree/56c542146db54bea2ac321420789e560c2706453)、[開封順の追試](https://github.com/tokotoko7777/ahc-workspace/tree/a41469d083df1817c2ff68de449305ff12ee9ba2) |
| [AHC056](https://atcoder.jp/contests/ahc056/tasks/ahc056_a) | 着手・本番未判定 | **1,899,994,171,242**（相対点） | 新規未調整32件 費用平均71.531 | 本番2,000件、公式ケース別基準を取得 | [規則圧縮](https://github.com/tokotoko7777/ahc-workspace/tree/76777c4e88aac27d028dfa893c47db2a540e4f05)、[最短路選択](https://github.com/tokotoko7777/ahc-workspace/tree/fbc469f215b805732151a93d4f8213cb3071663f) |
| [AHC057](https://atcoder.jp/contests/ahc057/tasks/ahc057_a) | 着手・本番未判定 | yochan、得点未確認 | 新規未調整28件 合計130,959,921 | 本番150件・2秒、同一ケースの証拠なし | [合流時刻](https://github.com/tokotoko7777/ahc-workspace/tree/597f72f03de27400f38dd4cff9f5ee779055c0be)、[グループ交換](https://github.com/tokotoko7777/ahc-workspace/tree/743f74ba0a89e40350d866c59c3bb1ce6a036bf1) |
| [AHC058](https://atcoder.jp/contests/ahc058/tasks/ahc058_a) | 着手・本番未判定 | fishylene、得点未確認 | 既存：未調整24件 平均5,187,704.58 | 本番150件、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/c75e684905c14eb10a7d59da3c9e9d81d8f9ce7a)、[後期の購入順](https://github.com/tokotoko7777/ahc-workspace/tree/23fa6eb16cc7acbe74babdf1ff9f57b7f993d483) |
| [AHC059](https://atcoder.jp/contests/ahc059/tasks/ahc059_a) | 着手・本番未判定 | chokudai、得点未確認 | 新規未調整28件 得点425,047（34,153移動） | 本番150件・2秒、同一ケースの証拠なし | [入れ子回収](https://github.com/tokotoko7777/ahc-workspace/tree/4848abd431bd612b8ab15199f2f5562fe1598974)、[部分列移動](https://github.com/tokotoko7777/ahc-workspace/tree/905faa4a34b05a6b0d5ded139f08fd8414e366fd) |
| [AHC060](https://atcoder.jp/contests/ahc060/tasks/ahc060_a) | 着手・本番未判定 | Sweet_Sweet_Soul、得点未確認 | 新規未調整24件 平均992.333 | 本番150件、同一ケースの証拠なし | [経路重複除去](https://github.com/tokotoko7777/ahc-workspace/tree/204933bff927c6bac159926766888181897cc748)、[途中で色を変更](https://github.com/tokotoko7777/ahc-workspace/tree/6e7f42ee36c537681b0db68526aba19077582ad0)、[歩行固定で色を変える時刻](https://github.com/tokotoko7777/ahc-workspace/tree/b0a4bc8d0ba51540d7f457511cb406a67f8e423b) |
| [AHC061](https://atcoder.jp/contests/ahc061/tasks/ahc061_a) | 凍結版：本番未達 | **2,424,667,397,941**（相対点） | **1,207,794,230,850**（本番3,000件・全件合法） | 本番3,000件、公式ケース別基準を取得 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/41ec433123816f9d026a4b719c5aeddbec16c29c)、[相手の同時手](https://github.com/tokotoko7777/ahc-workspace/tree/defe7c3d7e75e3b82da58b624c0b98906c4960dd)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/f119f0ff3e0268f477468ab09112488312198e7b) |
| [AHC062](https://atcoder.jp/contests/ahc062/tasks/ahc062_a) | 着手・本番未判定 | gazelle、得点未確認 | 新規未調整18件 平均467,170,089 | 本番100件・3秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/81151ac7b6335424f69b3bbb9da842061e296fb8)、[高低2本の経路](https://github.com/tokotoko7777/ahc-workspace/tree/083860ab332b39fa278b1736e3ca1e0561dc6765) |
| [AHC063](https://atcoder.jp/contests/ahc063/tasks/ahc063_a) | 凍結版：本番未達 | **1,985,255,142,455**（順位相対点） | **1,189,793,908,234**（本番2,000件、当時の分布を固定） | 本番2,000件、基準とseed列照合済み | [尾の移動](https://github.com/tokotoko7777/ahc-workspace/tree/4a7f7b14ee8c06d079c94d10d19acd9df9d0f139)、[次の色](https://github.com/tokotoko7777/ahc-workspace/tree/6e48e0c6aea92464d043b408df772be340c25470)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/f7cb5923521b00e317025fa5eb2e1e1d2622a75a) |
| [AHC064](https://atcoder.jp/contests/ahc064/tasks/ahc064_a) | 着手・本番未判定 | chokudai、得点未確認 | 新規未調整30件 平均93.833ターン | 本番150件、同一ケースの証拠なし | [同時操作](https://github.com/tokotoko7777/ahc-workspace/tree/63185be0c9a50743d00f846f73ca5f1ffab597e7)、[搬出順](https://github.com/tokotoko7777/ahc-workspace/tree/ab9244e54098611bbb35a322da00472d9bb95527) |
| [AHC065](https://atcoder.jp/contests/ahc065/tasks/ahc065_a) | 着手・本番未判定 | rabot、得点未確認 | 既定：未調整28件 149,371,055 | 本番150件・2秒、同一ケースの証拠なし | [交差ベルト](https://github.com/tokotoko7777/ahc-workspace/tree/37f1f97176dcc631de043411873fa74d279b9a1a)、[2箱先](https://github.com/tokotoko7777/ahc-workspace/tree/f531373eb2eaaee30443e6c6df6296f5a83da40a) |
| [AHC066](https://atcoder.jp/contests/ahc066/tasks/ahc066_a) | 凍結版：本番未達 | **1,893,330,847,366**（相対点） | **1,352,700,378,901**（本番2,000件・全件合法） | 本番2,000件、ケース別最低値 | [経路](https://github.com/tokotoko7777/ahc-workspace/tree/b69f28ac27f1bdfc0e4cf829de16762ea408ae35)、[マクロ候補](https://github.com/tokotoko7777/ahc-workspace/tree/84edb3f6302d8c76beb38d494282f833eb06cc74)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/588082c3b7a07866a338ac201e65b2ac1887bab6) |
| [AHC067](https://atcoder.jp/contests/ahc067/tasks/ahc067_a) | 着手・本番未判定 | udon1206、得点未確認 | 新規未調整28件 103,367,464 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/5799ad6db455dd0d973d5fcdea14cb56a32f98d2)、[壁の交換](https://github.com/tokotoko7777/ahc-workspace/tree/822e6835f0772872f57224924e02be8bace7e852) |
| [AHC068](https://atcoder.jp/contests/ahc068/tasks/ahc068_a) | 着手・本番未判定 | PrussianBlue、得点未確認 | 新規未調整26件 平均1,485.500操作 | 本番150件・2秒、同一ケースの証拠なし | [半分交換](https://github.com/tokotoko7777/ahc-workspace/tree/532a0cf2a83e82849153637b8279bbd0d5c08d20)、[長方形の幅拡張](https://github.com/tokotoko7777/ahc-workspace/tree/02a539aa8e143b7e5f56313c4c1f7235bab8c9be) |
| [AHC069](https://atcoder.jp/contests/ahc069/tasks/ahc069_a) | 凍結版：本番未達 | **1,981,252,894,878**（相対点） | **1,336,637,386,355**（本番2,000件・全件合法） | 本番2,000件、基準とseed列照合済み | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/bf3cc5a9c1c2ffa113f15916340896c6366c1c43)、[受入余裕](https://github.com/tokotoko7777/ahc-workspace/tree/c26dbd7905022272bc37b18beb04e810239b361e)、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/6bd633eade8df2dd1c67602130cc1aa76d52141a) |
| [AHC070](https://atcoder.jp/contests/ahc070/tasks/ahc070_a) | 着手・本番未判定 | eijirou、得点未確認 | 未調整24件 平均2,840,819.13 | 本番150件、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/3737c5016654eb51756f431f8fc90cd54ab58314)、[被覆重複の追試](https://github.com/tokotoko7777/ahc-workspace/tree/2f7c8b2c237219e2af6660023b37a296c0ed8fcd) |
| [AHC071](https://atcoder.jp/contests/ahc071/tasks/ahc071_a) | 着手・本番未判定 | rhoo、得点未確認 | 新規未調整28件 291,785点（費用44,243） | 本番150件、同一ケースの証拠なし | [2行DP](https://github.com/tokotoko7777/ahc-workspace/tree/ee229ea027b79e5e668683714e494d860d998f02)、[スライド再最適化](https://github.com/tokotoko7777/ahc-workspace/tree/58746cd7d8ae2eefb5147984a11c211581cd72cd) |
| [AHC072](https://atcoder.jp/contests/ahc072/tasks/ahc072_a) | 着手・本番未判定 | **1,995,670,086,683**（相対点） | 既定：未調整30件 平均345.67手 | 本番2,000件、公式ケース別基準を取得 | [同色まとめ](https://github.com/tokotoko7777/ahc-workspace/tree/36a890d2c5c19a137f67de5233c58f245d1e01e5)、[取り出し順](https://github.com/tokotoko7777/ahc-workspace/tree/23a0895828bc6d484120851c8669595e5cb4c31e)、[異色を組む追試](https://github.com/tokotoko7777/ahc-workspace/tree/1962da2e33e2d5f72feef3e1b881fc7879690073) |
| [AHC073](https://atcoder.jp/contests/ahc073/tasks/ahc073_a) | 着手・本番未判定 | hitonanode、得点未確認 | 未調整28件 平均11,155.00操作 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/614cf90a321ba056b75894fdb9d00da56428a94c)、[二回反射](https://github.com/tokotoko7777/ahc-workspace/tree/3468c6eddc918ad794084cb891015db9e20524f7) |
| [AHC074](https://atcoder.jp/contests/ahc074/tasks/ahc074_a) | 着手・本番未判定 | Division 1: noimi／Division 2: mech_39、点数未確認 | 未調整28件 操作724,334、全件誤注文0 | 本番2,000件公開seed、部門別基準は未取得 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/b2c94ae0786e0805d6f5e63f3fe62c25ac404ca1)、[紙の割当方法の追試](https://github.com/tokotoko7777/ahc-workspace/tree/96edf8020f4615c7d24c1bff857f1369614f6260) |

### AHC001 の次の仮説

希望点の密集部を含む複数の長方形の共有境界を連動させれば、単独長方形の伸縮で詰まった領域を再配分できる。既存 v64 を C++23 で計測し、同一開発ケース・条件で比較する。採用後は調整に使っていないケースで合法性・得点・実行時間を確認する。公開された公式 1,000 ケースは今回の判定用として開発に使わない。既存 v64 の先頭 30 件は既に計測履歴があるため、独立ホールドアウトとは呼ばない。

問題ごとのコードは `problems/ahcNNN/`。再利用時の課題は [ライブラリ不便メモ](LIBRARY_FEEDBACK.md) に追記する。

### AHC001 の本番比較根拠

- [公式問題文](https://atcoder.jp/contests/ahc001/tasks/ahc001_a): 各ケースで満足度平均の 10 億倍を最も近い整数に丸め、本番 1,000 ケースを合計する（大きいほど良い）。不正出力・TLE のケースは 0 点。制限は 5 秒・1024 MiB。
- [公式コンテスト告知](https://atcoder.jp/contests/ahc001): 最終順位はシステムテストのみ。直接の[公式最終順位表](https://atcoder.jp/contests/ahc001/standings)は未ログインで閲覧不可。
- [AtCoder 提供の公開データセット](https://huggingface.co/datasets/SakanaAI/ALE-Bench)の `ahc001.zip` 内 `standings_scores.csv` の `rank=1`。Lite 版は使用しない。アーカイブ SHA-256 `87d4235e0d1ccccc2b4e4d1eae824c1308a935ee2d23b9e107c9d8ec2ffd762b` は配布元メタデータと一致。順位・得点列のみ確認し、他参加者のコード・解説は参照しない。
- 本番比較には[公式シード](https://img.atcoder.jp/ahc001/seeds.zip)を使う。展開した `seeds.txt` の MD5 `8fc1ce3f4beabac6abc1bdb4206d7f7e` と件数 1,000 を確認済み。凍結 v64（探索上限 4.8 秒・乱数 seed 0）で本番 1,000 件を完走。993,411,282,631 点、1 位まで 411,789,052 点（1 位比 0.04143%）で未達。main へのマージなし。

### AHC001 の初回実験

- 変更案: 接触する長方形の辺を連結成分ごとに連動させ、指定点の包含と非重複を満たす座標区間内だけを探索。
- 同一開発 8 件: 既存 v64 993,052,040.625 → 改良案 993,961,106.250（平均 +909,065.625、6 勝 2 敗）。別シードの未調整 10 件: 995,156,057.5 → 994,949,731.1（平均 −206,326.4、4 勝 6 敗）。既定版に採用しない。全 36 出力で独立の合法性検査と公式点が一致。
- [実験・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/fe7273b40cc8cc8671283ace5d30b6aaf6ceec1a/problems/ahc001/README.md)。C++23 の結合 1 ファイルをビルド・実行し、既存 22 テストと新しい長方形境界のランダム比較 800 レイアウトが通過。実験コード・部品は問題別ブランチに保存。main は未変更。
- 次の仮説: 共有辺だけでなく、順序制約を守りつつ複数の境界座標を同時に再配分する。公式 1,000 ケースの個別結果は調整に使わず、新しい開発シードで試す。

### AHC001 の全本番シード評価（2026-11-20）

- [固定したソースのコミット](https://github.com/tokotoko7777/ahc-workspace/commit/fe7273b40cc8cc8671283ace5d30b6aaf6ceec1a) の既定 v64（共有境界を無効にした通常ビルド）を単一ファイルへ結合し、`g++ -std=c++23 -O3 -DAHC_DEFAULT_SECONDS=4.8` でビルド。
- 本番公開シード 1,000 件すべてで合法・時間内。最大実測 4.838060 秒、中央値 4.805086 秒。公式採点器の各ケース整数点の合計 993,411,282,631。提出済みスコアではなく、本番公開ケースを使うローカル再現。
- 参考 1 位 993,823,071,683 点。差 −411,789,052 点、未達。全シードの入力ハッシュ・件数・ケース別記録から集計値を再確認。
- 凍結バイナリ SHA-256 `c9689986319fe53ac94c0d5567d708274b1aff13bf8cea22e16e424ca354ed7a`。[再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/fe7273b40cc8cc8671283ace5d30b6aaf6ceec1a/problems/ahc001/README.md) の full suite を使用。ケースは公式ツールで生成し、比較時は時間制限 5 秒。
- 1 位を超える改良は継続。次は順序制約を最小カットに落とし、離れた境界も一度に動かす案を、新しい開発用・未使用シードで比較する。本番評価と切り離し、AHC002 の準備も進める。

### AHC002 の比較条件

- [公式問題文](https://atcoder.jp/contests/ahc002/tasks/ahc002_a): 100 ケースの得点の総和を最大化。本番後のシステムテストなし。1 ケースでも不正または時間超過なら提出全体が 0 点。配布入力 0–99 は本番と異なるため、ローカル開発比較で 1 位超えは判定しない。
- [AtCoder 提供データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc002.zip) の `standings_scores.csv` で1位 6,255,977 点を確認。アーカイブ SHA-256 `c39b6db91b1956d3f816e63755103622e0ec090d84c6cf81dba1abb5c631ebd9` は配布元と一致。
- [ALE-Bench の論文 §A.2.2](https://arxiv.org/html/2506.09050v2) は full 版の非公開評価に本番と同じケースを使用すると記載。ケースやシードを直接参照せず、対応する評価インターフェースで公式と同じ集計・合法性を確認できた場合だけ比較する。現在は未判定。

### AHC001 最小カットによる境界再配分

- [実験ブランチと再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/54f9862b1def3e9129385a3acf84a6693d409a12/problems/ahc001/README.md) に保存。離れた境界も、許容幅・順序制約を満たす範囲でまとめて動かす。
- 仕上げ時間を先取りする版は、同一条件の開発 12 件で平均 −124,957 点、独立確認 12 件で平均 −156,719 点。既定を変更しない。仕上げの時間が余り、従来探索の時間が短くなった。
- 既存探索を 4.8 秒維持したあとに適用する版は、別の 8 件で同一実行の仕上げ前から全件改善。平均 +61,350 点、最大実測 4.816 秒。1 位との差（1 件平均約 411,789 点）を埋める本番結果ではなく、既定・main は未変更。
- 次の仮説: 邪魔な長方形を一時的に縮め、直交方向の順序制約を組み替え、複数境界の再配分後に再成長させる。同一開始配置と同じ時間で比較する。

### AHC002 短区間の経路再構築

- [再現手順・実験コード](https://github.com/tokotoko7777/ahc-workspace/blob/c09a5d30611f6991d5cb3761b5a5038f5eb0316d/problems/ahc002/README.md) を問題別ブランチに保存。既存ルートの短い区間を、同じタイル ID を再利用しない制約つきで全探索した。
- 開発用 12 件の総得点: V10 の 744,958 → 746,876（+1,918）。独立した未調整 20 件: V10 の 1,254,587 → 1,254,478（−109）。後者では改善せず、V10 を既定のままにする。64 出力で公式採点と独立検査が一致。持ち出し可能な C++23 一ファイルと部品テストも通過。
- 同一開始配置の切り分けでは、開発用 12 件で平均 +15.75 点、3 件改善。次は隣接する二つの区間をまとめて置換し、単独区間に固定した出入口の制約を緩める。
- 本番 100 件での盲検比較は、この環境に公開評価機能の前提となる Docker がなく未実施。非公開の入力やシードは参照せず、状態は未判定。

### AHC001 単一ブロッカーの配置順序変更診断

- [コードと再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/0c71d1904fb4b9c4dd0f329dc0a5a455679ad35f/problems/ahc001/experiments/topology_v1/README.md) を別ブランチ保存。新しい開発 8 件の同一開始配置で、邪魔な長方形 1 枚を一時的に縮めて既存の最小カットと比較。
- 初回は平均 +511.25 点、再計測は +411.375 点、いずれも 4 勝 4 分。全 32 出力の公式採点・合法性検査が一致。83 試行中 4 回は狙った分離方向が変わったが、採用した最良配置には残らず。小さすぎるため既定版は変更しない。
- 次は少し縮めるだけでなく、途中で一時的な失点を許したうえで、移したい順序を連続変形の制約に明示する。この近傍の反復数を増やすだけにはしない。

### AHC003 の本番比較根拠

- [公式問題文](https://atcoder.jp/contests/ahc003/tasks/ahc003_a) の本番 3,000 件、公式対話テスタでケースごとに採点し絶対得点を合計。各ケース 2 秒、無効・時間超過はそのケースのみ 0 点。[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc003.zip) を確認。
- 公開 `seeds.txt` MD5 `0cf5051d586e7f62c0b3527f6f7fbb1c` と件数 3,000 を確認済み。開発・未使用ケースを別々に生成し、序盤の同程度の経路に限って、観測による推定の改善も評価する案を既存版と比較する。

### AHC003 序盤の観測経路の比較

- [実験と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/0d42d1839a9ff4a1f35b2830ea4b2e2eeba682c4/problems/ahc003/README.md)。既存の経路より少し高い程度の候補に限定して、観測から推定がどれだけ進むかも評価した。
- スコア差（候補−既存）: 開発 16 件 −10,833,676、未調整 24 件 +51,063,960、追加の独立確認 24 件 −21,549,158。判定閾値を変えず追加確認は不合格、既存解を保持。128 出力すべてで対話テスタと独立検査が一致。C++23、単一ファイル、汎用部品の乱択 1,000 件比較を通過。
- 既存解を凍結し、同じ公式対話テスタと本番公開シード 3,000 件で全件評価した。凍結バイナリ SHA-256 `a78c04bdaae066bee7b5e6458106b044e79179e606f607152adb11b4ad76e539`。本番結果は開発には使わない。AHC004 の着手準備も開始。

### AHC004 周期付き文字列の重なりから初期配置を作る

- [実装・ケース別点・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/74c0189dc1bea65370a25ee45a0b74e5b8ddd591/problems/ahc004/README.md)。文字列同士の重なりと相対オフセットから、矛盾しない周期パターンを作ってから盤面へ置き、既存 SA を同じ予算で適用する。
- 開発用 10 件で平均 +2,185,938 点（8 勝）、未調整 10 件で平均 +1,847,067 点（5 勝 5 敗、中央値 +2,062,629 点）。全 40 出力で公式採点と独立検査が一致。最大 2.786604 秒。候補は問題別ブランチの通常ビルドに保存し、既存の使用例と main は変更しない。単一ファイル C++23 と汎用部品を検証済み。
- [公式問題文](https://atcoder.jp/contests/ahc004/tasks/ahc004_a) は 100 ケース、各ケース整数点を合計、いずれかが失敗した提出は 0 点、3 秒。配布ローカルケースと本番は異なるので、順位データの 7,321,305,762 点とは未比較。配布した順位アーカイブ SHA-256 `2c43b467ddfabfa774306c350f397a272f1b4ae6023066a0c44ea93fe62a42c8` は配布元メタデータと一致。
- 次は行の周期シフト・交換で平行方向の語を保ちつつ、直交方向を改善できるか同じ初期配置で検証する。

### AHC004 行・列を循環移動する追加探索

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/88dfd64092fbefe29c5d2e8770d08813ee522452/problems/ahc004/experiments/line_cycle/README.md) を別の問題ブランチに保存。行や列を一つ回転、または同方向の二本を交換し、平行方向の部分列集合を維持。直交方向だけを差分更新する。
- 新規開発 8 件、周期初期化版の同一盤面から両方 105 ms の追加探索で比較。既存操作の追加探索を対照とし、平均 +140,634 点、3 勝 5 分。最長 2.894019 秒（初期解込み・ローカル）。全 24 出力で公式採点と独立検査が一致し、ランダムな差分検査 70 盤面 × 各 100 操作と C++23 ビルドを通した。固定開始の診断だけで、通常版は変更しない。本番比較は未判定。

### AHC005 経路の短縮と観測区間の強制性

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/b684d67d0286d48bd4a34fb7c9136a7a8cab2d40/problems/ahc005/README.md)。実際の歩行列を短い経路に差し替えたときも、各道路セルが縦か横のどちらかの訪問区間で観測されるかを差分更新する。
- 開発 8 件の合計 1,884,887 → 1,889,028、独立確認 8 件は 1,761,160 → 1,750,812。経路差し替えそのものが短縮した例は 0/16、既定は既存のまま。全 32 出力で完全観測、公式採点と独立の視線検査が一致、最大 2.638180 秒。C++23 単一ファイル、被覆・巻き戻しの乱択照合を通過。
- 縦・横区間から観測する集合を二部グラフで調べたところ、16 件すべてで全長区間が単独観測セルまたは出発地により実質必須だった。この実験に対して安い被覆集合を選ぶだけの案は打ち切り。次は区間に専用の立ち寄り点を置くのではなく、順序探索中から経由する最短経路上の観測を組み込む。
- [公式問題文](https://atcoder.jp/contests/ahc005/tasks/ahc005_a) は 100 ケース・3 秒、通常配布の 0–99 と本番は別。順位 `standings_scores.csv` の 1 位は 22,318,770 点。配布元と同じ SHA-256 `2d223bee36c1499c98f1e9e927766bbf135f5d8a7f4e05890b4df044d790b484` の公開順位アーカイブを参照した。本番比較は未判定。

### AHC005 経由する最短路の観測を使う近傍

- [追加実験](https://github.com/tokotoko7777/ahc-workspace/blob/a6b60ce8c6779c638c162ef63c65a4d484dc6dad/problems/ahc005/README.md) を別ブランチに保存。各立ち寄り間の有向最短経路が観測する縦・横区間をキャッシュし、同一開始順から、削除のみと隣接点の移設＋削除を比較。完全観測を維持する場合だけ短縮を採用する。
- 新規開発 8 件はいずれの近傍でも合法な短縮なし。最長 2.652082 秒、公式点と独立視線計算が一致。100×30 回の差分検査と提出用単一ファイルの C++23 ビルドを通過。既定版は変更しない。
- 次は調整終盤だけではなく、主探索中に経由点数の変化と一時的な距離増を許す。また、等距離最短路を一つ固定する代わりに、被覆が薄い区間を観測する経路を優先する。

### AHC003 本番公開 3,000 件の結果（2026-11-20）

- 既存版を凍結し、[実験と採点の再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/0d42d1839a9ff4a1f35b2830ea4b2e2eeba682c4/problems/ahc003/README.md) に沿って公式対話テスタで実行。全 3,000 件が有効。ソルバとテスタを合わせた最大ローカル実測 0.896979 秒、ソルバ最大 CPU 0.825624 秒（制限 2 秒）。
- 合計 2,925,781,453,112 点、1 位 2,926,767,743,437 点に対し −986,290,325 点（0.033699%）。本番公開シードを用いたローカル再現で、提出はしていない。ケース記録の合計と全 3,000 入力ハッシュを独立確認。未達のため main へのマージなし。
- 本番ケース別結果は調整に使わない。次は「全パラメータの平均分散」ではなく、同程度の候補経路の所要コスト差の不確実性だけを減らす観測を、独立開発用ケースで比較する。

### AHC006 空間マルチスタートによる初期解比較

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/761d6839f93d0e347820919714402360766c4ad5/problems/ahc006/README.md)。既存のペア挿入をそのまま使い、中心をずらした複数の近距離注文プールから初期解を作る。初期経路は約 30% 短縮したが、同じ主探索の後は開発 10 件平均 21,827.3 → 21,535.6、未調整 10 件平均 21,463.0 → 21,422.9。いずれも不採用。40 出力で公式採点と独立検査一致、最大 1.877630 秒（2 秒制限）、C++23 単一ビルドを通過。
- 本番 100 ケースと配布シードの同一性を確認できないので未判定。順位データの配布元 SHA-256 `08f54f5ce375baf4a30c9730fdb71cd731619b89e88f63eec1894a850b59bf18` を照合。次は複数注文をまとめて取り除き、入れ替えを同時に再構成する。

### AHC003 経路間の差を直接狙う追加観測

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/196a7404c62279475e6b868a305c949804e31404/problems/ahc003/README.md)。平均分散ではなく、ほぼ同程度の二経路のコスト差が観測でどれだけ再判定できるかを評価した。
- 開発 20 件は +5,214,047 点（12 勝）、未使用 28 件は −48,433,654 点（9 勝）。全 96 出力で公式対話テスタと独立検査一致、最大ローカル実測 0.675 秒未満。未調整セットで悪化し既定版は変更しない。C++23 の単一ファイル化と汎用分散計算の 1,000 件照合を通過。
- 先々の問い合わせで役立つ複数経路差を履歴から集め、今の経路の損失と次の経路への利益を分離する案を次の仮説として保存。公式 3,000 件の個別結果は反映しない。

### AHC006 複数注文の取り外し・ビーム修復

- [ソースと再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/0e7380c86bf2f895cc47b0f44cd976f7bb01e9cd/problems/ahc006/README.md)。新規開発 8 件の同じ開始解で、3/4 注文を抜いて 256 組の貪欲・幅 4 ビーム修復を比較。両方とも全件同点で改善なし。公式採点と独立検査が一致し、最長 1.760048 秒。未使用ケースを消費せず診断を終了し、既定は変更しない。
- 既存挿入 API では最良の隙間一組に固定されるため、まだ注文間の位置の相互作用を扱えていない。次は連続区間内のイベント 8〜12 件を先行制約付き DP で一度に並べ直すか、主探索の中で一時悪化を許す。

### AHC007 将来の辺重みを複数シナリオで評価

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/2793a9a782184c4b77376fab17482d24b353d6bd/problems/ahc007/README.md)。候補辺を採らない場合の接続コストを、複数の将来重みで評価して閾値を決める。未来の実測値は見ない。
- 開発 20 件 +34,154,630（19 勝）、未調整 36 件 3,337,415,004 → 3,393,731,903（+56,316,899、34 勝）、別の公式ツール同梱 150 件 13,878,237,519 → 14,184,305,386（+306,067,867、147 勝）。全 412 実行でローカル逐次対話と独立得点が公式スコアに一致、最大実測 0.280 秒、ソルバ最大 CPU 0.214 秒。C++23 単一結合と汎用テストを通過。
- 問題別ブランチの新規解として保存。本番 150 件と公開 150 件の同一性を証明できないため、最終 1 位 14,241,733,941 点との比較は未判定。main は変更しない。
- 次は同じ将来シナリオで、現在の辺を強制採用／見送りしたあとの短い決定列を追い、遅れて現れる代替辺も考慮する。

### AHC007 採択／見送りの前方ロールアウト

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/93feeae1157993a5b59fbd93c7d0db0381c8ea17/problems/ahc007/README.md) を別ブランチに保存。同じ乱数シナリオで採択／見送りを強制し、先の 16 手を共通の方策で動かして比較。
- 新規開発 20 件 +606,802（11 勝）、独立確認 36 件 −1,083,508（13 勝）。最大 0.365 秒、112 出力で公式採点と独立検査が一致。不採用、通常は元のシナリオ法を維持する。
- 次は一律 16 手ではなく、代替パスの最後の辺が到着するまでの分布を使う。公開入力と本番の同一性は未確認のまま。

### AHC008 仕切り位置を適応的に選ぶ初回構成

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/af41b4e70a89fef3c0389c689aa09dc633fffa16/problems/ahc008/README.md)。合法な仕切り 1 本を基準とし、各ターンで動物分布を見て次の長方形分割を選ぶ解を比較。
- 新規開発 24 件 6,068,929 → 79,174,426、未調整 24 件 1,650,490 → 53,656,251（平均 2,235,677）。全 48 ペアで改善し、96 実行で公式対話テスタ v3 と全ターンの独立検査一致。ローカル最長 0.049386 秒。C++23 を問題別ブランチへ保存。
- 保てる面積が平均約 135 マスと小さく、未使用セットでも中に平均約 3.38 匹残る。現時点では本番目標に遠いので公式 2,000 ケースの評価はまだ実施しない。順位データ 1 位 121,980,050,607 点（配布元と同一 SHA-256 `7dc040eb07f4543a16ef550a198859519a98af899771237b2bf1517c8a419300`）。
- 次は何度も人間側を縮めるのではなく、複数人で小部屋を並行に建て、動物を小部屋へ入れることで人間側の面積を残す。

### AHC008 外周の小部屋を並行して建てる案

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/053d05b2b80aee5da0ebd027ade3079065bbe3e7/problems/ahc008/README.md)。別々の小部屋を複数人で並行構築し、人間側の面積を大きく残す案を比較。
- 開発 24 件の平均 2,697,270 → 1,690,023、独立確認 24 件 2,944,878 → 916,565。後者では平均面積が 121.46 → 587.00 マスに増えた一方、同じ部屋の動物が平均 2.833 → 8.333 匹になり不採用。全 96 出力で公式 v3 と独立リプレイが一致、最長 0.056420 秒。既定の適応分割は行動まで同一、公式 2,000 件は未実施。
- 次は固定数の小部屋を増やすのではなく、既存壁付近であと 1〜3 マスで閉じられる隘路を列挙し、閉鎖したときの連結領域と得点をその場で評価する。

### AHC009 初到達の期待得点を使う座標掃引

- 基準は最短経路と反復の合法解。遷移確率を前後から計算し、各操作を正確に比較する。開発 16 件 298,878,713 → 1,214,896,626、未調整 32 件 324,426,392 → 2,389,017,515、公式ツール同梱の別 100 件 1,133,995,182 → 7,214,943,739。全 148 ペアで勝ち、全 296 実行で公式採点と独立実装一致、最長 0.315 秒未満。C++23 単一ファイルを検証済み。
- 公開 100 件が本番と同じとは確認できず、1 位 7,547,834,181 点との比較は未判定。試作は既存ライブラリを利用し、既存の出典注記を残している。ローカルコミットとテストは保存、GitHub 反映は出典の扱いの確認待ち。次は隣接する二操作の同時変更を新規ケースで比較する。

### AHC010 隣接タイルを協調して回転

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/8e99892f7dc12317ba365750a15a2a7c072b3731/problems/ahc010/README.md)。長い経路を閉じる方向で初期 SA を行い、隣接二枚が共有辺でつながる回転を一緒に提案する。
- 開発 10 件の平均 46,492.4 → 53,175.2（6 勝）、未調整 10 件 33,180 → 56,482（7 勝）。全 40 出力で公式採点と独立検査が一致、最長 1.834601 秒（2 秒制限）。C++23 と汎用走査を問題別ブランチで検証し、通常候補に保存。
- [公式問題文](https://atcoder.jp/contests/ahc010/tasks/ahc010_a) は最大二ループ長の積を 100 件集計。本番の一致を確認できず、1 位 20,218,968 点との比較は未判定。順位アーカイブ SHA-256 `8f2e83fe0b13701d1df45c938b3397a2474518751aa646adf21f1eb0169b8eb6` を配布元と照合した。次はパッチ外の経路を縮約し、2×2 回転を一括で正確に評価する。

### AHC009 隣接する 2 操作の同時変更（ローカル検証）

- 同一開始解から、指示一つずつの掃引に加えて、隣接二つの 16 通りを厳密に評価した。追加時間を初期解の多様化に使う版も比較対象とした。
- 未使用 30 件の合計は 2,278,152,917（基準）／2,297,199,096（多様化）／2,326,160,856（二指示）。二指示は基準に 30 勝、多様化に 29 勝。新規開発 16 件でも両者を上回った。全 138 実行で公式の丸め点と独立検査が一致、最大実測 0.534 秒、C++23 単一ファイル検証済み。
- 公開 100 件の再走も本番条件での評価も未実施。ソースと再現手順はローカルに保存。従来から存在する出典の扱いを確認するまで、ソースの GitHub 反映は保留。

### AHC010 パッチ外を縮約した 2×2 回転

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/38ad5adc05db391f956a457ef47ea3376293d691/problems/ahc010/README.md)。パッチの外を、境界間の経路と既存の閉路に縮約し、2×2 の回転を厳密採点。全ポート走査との 8,400 通りの比較を通過。
- 8 件ずつ同じ初期盤面で、凍結／単枚／2×2 を比較。開発平均 52,258.5／52,492.5／54,069.5、独立確認平均 50,833.5／50,833.5／52,521.5。未使用で 8 勝、平均 +3.3%。パッチ 406 箇所/半面、全 48 出力公式+独立一致、最大 1.842705 秒（2 秒制限）。C++23 単一ファイルも通過し、既定版は変更しない。
- これは固定開始解への仕上げ比較。本番では未判定。次は採用パッチを連鎖し、開経路の端点・長さをキャッシュして主探索に組み込む。

### AHC011 本番条件

- [公式問題文](https://atcoder.jp/contests/ahc011/tasks/ahc011_a) の公開本番 3,000 件（N=6〜10 各 600 件、3 秒）。得点は各ケースを整数に丸めて総和。完全木と未完成で式が異なる。システム失敗はケース単位 0 点。
- 公式公開シード SHA-256 `041256f962c6ba1a60294ad7a575684d6e401163cba316cf978f2e66a4f7b0e3`、順位データ SHA-256 `6d26e9ea3dbd8b97616d9727bdbe0db270d0df9ab54efd44c71f1c03379a03fb` を照合。順位 1 位 2,691,447,336 点。先に新規開発セットで検証する。

### AHC011 空白を元の位置に戻す三枚置換

- 既存の木ビームに、空白を迂回して 2×2 の 3 タイルを回し、同じ経路を逆に戻す手を追加。開発 10 件 3,462,518 → 3,980,103（8 勝）、未調整 15 件 5,229,021 → 6,060,235（14 勝）。同じ 850 ms の探索予算、全 50 実行で公式と独立の各点が一致、実測 0.895 秒以下。C++23 単一ファイル化を検証。
- 完全木は両版とも 0 件なので、公開本番 3,000 件はまだ計測しない。ローカルに実装、全空白位置の検査、再現手順を保存。既存の使用例から引き継いだ出典を保持しており、ブランチ反映は出典の扱いを確認するまで保留する。
- 次はタイルの在庫を満たす木を先に組み、空白移動で到達できる順序・パリティ・手数を分けて検証する。

### AHC011 目標盤面を先につくる診断（ローカル）

- 端の不一致を減らして在庫どおりの目標盤面を作り、パリティを確認してから実際の空白移動で復元。開発 10 件で +226,080、独立確認 10 件で −33,631。開発 1 件の全域木は 304/432 手で合法に復元したが、未使用セットでは完成 0 件。不採用。
- 全 40 出力で公式採点と独立検査が一致し、最長 0.960 秒。境界の欠陥 0 でも 7 件中 6 件は閉路・非連結で、次は探索中から連結性を評価する。既存出典を保持し、ローカルに保存。公式 3,000 件は未評価。

### AHC012 格子切断の方向比較

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/dc1a18935430728a5dbca82c14f8bc84c57145e1/problems/ahc012/README.md)。水平・垂直格子と、斜めを含む方向の候補を同じ予算で比較。開発 8 件平均 988,686.50 → 986,318.875、未調整 8 件平均 972,456.375 → 968,873.75。斜めは不採用。全 32 出力が公式と独立採点一致、最長 2.730955 秒（3 秒制限）。
- 本番 100 件が入手できたローカルケースと同一とは未確認。AtCoder 提供の順位データで 1 位 99,781,619 点、理論上限は 100,000,000 点（1 件 1,000,000 点）。開発 1 件で単件上限を確認したが、順位は未判定。
- 次は余ったカットを使い、不足サイズを狙って一回だけ任意方向に切り出す。符号ごとの領域分割と他領域への影響を正確に集計する。

### AHC012 不足サイズを狙う追加カット

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/247becac000d9343c3d98fa15ce6062d845566df/problems/ahc012/README.md)。凍結格子解へ、24 個の整数法線の全オフセットから 1 本だけ加える。分割で新旧サイズの充足数がどれだけ変わるかを差分で厳密評価。
- 新規開発 8 件中 2 件で 1 人ずつ改善、別の 8 件は全同点。不採用・任意機能として保存。3 種の全 48 出力で公式採点と独立線分判定が一致、最大 2.727876 秒。C++23 と汎用 [capped-partition-sweep.hpp](https://github.com/tokotoko7777/ahc-workspace/blob/247becac000d9343c3d98fa15ce6062d845566df/library/capped-partition-sweep.hpp) の 800 比較を通過。
- 次は一つの既存カットを外してから置き換え、異なる既存区画の統合と再分割を同時に許す。

### AHC013 隣接移動から再配線

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/7526fecad74457ce05f786e5127c4e45c0357203/problems/ahc013/README.md)。移動なしの線分連結を基準に、同型の端点が並ぶ短い隣接移動を試し、移動と配線の残予算を守って配線案を再構築。開発 12 件 7,294 → 37,991、未調整 20 件 11,377 → 64,679（全 32 勝）。基準は最大約 5 ms、新版は最大 0.840538 秒で、比較は同一入力だが使った時間は異なる。
- 全 64 出力で公式採点と独立検査が一致。C++23、一ファイルおよび四方向視界部品の対照テストを通過。研究ブランチの既定に保存。
- 有効な本番公開 2,000 シードを SHA-256 `0a50dccf4689292f69a36be0ea1a99767de055434b854f2b0bc7f1fd40f9033e` で確認し、100 分類に各 20 件で生成。凍結版で公式の自己得点を計測中。順位表の 1 位得点が未取得なので、完走後も直ちに順位を判定しない。次は離れた成分をつなぐ複数移動を検討。

### AHC014 最良手順の接頭辞から再成長する

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/25c2dea8a3d5f9b07432ee1a2b239cf4bd820cff/problems/ahc014/README.md)。貪欲ランダム再始動と、最良手順の途中から別の長方形を追加し直す方法を同一時間予算で比較。
- 開発 6 件平均 788,064.17 → 810,083.17（5 勝 1 敗）、未調整 6 件 862,623.67 → 880,722.83（4 勝 2 敗）。全 24 出力で公式と独立線分検査が一致、最長 4.564538 秒（5 秒制限）。C++23 単一ファイルを含めて検証し、研究ブランチの既定に保存。
- 本番 1 位 3,122,399,998 点、平均約 156 万。順位データ SHA-256 `43624bb9c24795ef1e792bace66bfeb59787265b025750b8708bee692c2e5021` を照合した。公開本番 2,000 件は今の版では未評価。本番比較は未判定。
- 次は手順の末尾を全消去せず、一つの長方形と、それに依存する後続手だけを除き、無関係なほかの手を維持する。

### AHC014 依存先だけを削除して再成長

- [固定開始からの比較・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/b56da5cc4b1c38a5cd7cc4a9a8f90e7661ddb857/problems/ahc014/README.md)。長方形の新しい頂点に依存する後続手だけを除く方式を、削除数を揃えた末尾削除と比較。凍結 4,270 ms の開始解から 1 入力 32 対を実行。
- 開発 6 件の平均 832,521.67 → 842,875.33（5 勝 1 分）、独立確認 6 件 899,786.50 → 909,102.33（6 勝）。全 384 試行対完了、出力 36 件は公式と独立検査一致、最長 4.366745 秒。ランダム 100 盤面の依存関係と C++23 結合を検証。
- 既定の 4,550 ms 版との等時間比較ではないため任意実験。次は同じ予算を再始動と依存削除へ配分し、通常解を対照にする。本番集計はまだ未判定。

### AHC015 同じ落下シナリオによる先読み比較

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/a2b1c24d2d58f6e42db8475e62eb7ae00ef9373e/problems/ahc015/README.md)。一手期待値と、将来のキャンディ位置を同じ割合・乱数で共有する 6 手先比較を、100 ターンで検証。
- 開発 24 件平均 568,462.63 → 663,282.21（18 勝）、独立確認 24 件 600,825.08 → 663,091.75（17 勝）。全 96 出力が公式採点と独立の対話再生で一致。最大 0.204520 秒（2 秒制限）。C++23 一ファイルと汎用部品照合済み。研究ブランチの既定に保存。
- 1 位は 161,011,437 点。順位データ SHA-256 `11293d19419acd7e8118126ffc1cfe4a0137629769108935c10a0bd3b554ce02` を照合。本番 200 件が配布入力と同一か未確認で、順位は未判定。次はシミュレーション総ステップを合わせた 6 手対 12 手を比較する。

### AHC015 同じシミュレーション量で深さを比較

- [追加実験の再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/3cbfb52d69faa1f03318633c1289710bbb7b4ec4/problems/ahc015/README.md)。最大仮想配置数を方向あたり 156（6 手×26 対 12 手×13）に揃え、6 手先と 12 手先を比較。
- 新規開発 24 件 640,541.50 → 607,777.38 点（9 勝）、別未調整 24 件 617,989.38 → 594,117.13 点（10 勝）。長い版は不採用。全 96 出力で公式・独立一致、最長 0.246614 秒、C++23 一ファイルを確認。
- 深さとサンプル数のどちらが主因かは未分離。次は同じ深さと本数で離れた同色成分をつなぐ評価を比較するか、両因子を独立に振る。

### AHC013 凍結版の本番公開 2,000 件

- 2,000 件すべて有効、合計 **6,951,707 点**、最大ソルバ実測 0.919261 秒。公式採点と独立採点を各件照合し、全入力 SHA-256 と保存記録の合計を再確認。公開された本番ケースでのローカル再現で、本番提出ではない。1 位の得点は公式順位表のログイン制限で未取得、厳密比較は未判定。
- [固定ソース](https://github.com/tokotoko7777/ahc-workspace/commit/7526fecad74457ce05f786e5127c4e45c0357203) の C++23 通常ビルドを `g++ -std=c++23 -O3 problems/ahc013/main.cpp -o motion` で再現し、元のバイナリ SHA-256 `c8b9ec743508c43fbf2135173cdde81f3aaebe1ee7086adbe48783197541eff8` と一致。公式 `gen` で公開本番シードを生成し、`vis` と [独立採点・再現スクリプト](https://github.com/tokotoko7777/ahc-workspace/blob/7526fecad74457ce05f786e5127c4e45c0357203/problems/ahc013/README.md) を用いる。

### AHC016 ノイズ付き次数による符号化

- [実装・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/b84d5b74efaa1410549101730df5a60c1cc3102e/problems/ahc016/README.md)。辺数だけでなく次数分布で候補グラフを識別。開発 40 件絶対点平均 31,956,874.08 → 36,791,740.40、未調整 40 件 31,882,017.48 → 38,032,050.10。後者で 32 勝 7 分 1 敗、平均誤答 29.65 → 12.775。160 対話で公式の得点・誤答と独立計算が一致。最大 0.070234 秒。研究ブランチ既定に保存。
- 本番 2,000 ケースの順位は相対点で、最終 1 位 1,780,676,618,015 点。順位アーカイブ SHA-256 `6be902402ee316cdfe59dbd0e6045507ff31d0dfd14bc68bc6f1258366ca54cb` を照合。各ケースの当時の最高絶対点が必要で、上記のローカル絶対点とは同じ物差しではない。比較は未判定。公式制限 5 秒。次は、構造符号に合うグラフ頂点数を誤りの確率とサイズ罰から選ぶ。

### AHC017 本番条件

- [公式問題文](https://atcoder.jp/contests/ahc017/tasks/ahc017_a) は、日ごと・頂点対ごとの最短路増加量を平均し整数化する絶対点を最小化。本番順位は 2,000 件で同一ケースの当時の最小絶対点を使う相対方式、制限 6 秒。
- [順位アーカイブ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc017.zip) の 1 位は 1,993,227,663,417（相対点）、SHA-256 `d0df8fa1417278e9651668acc68d3e015497d1da6b5f2ebe599be27be07efa5d`。まず同じ開発入力で、幾何・端点の分散と、標本最短路の干渉を抑える割当を比較する。

### AHC016 ノイズ推定で符号グラフのサイズを選ぶ

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/17d5f04c1837c2edec06ee64e33cb56d6213f5c6/problems/ahc016/README.md)。人工的なノイズだけを使い、頂点数を変えた場合の誤りとサイズ罰を推定し、入力条件に合うサイズを選ぶ。
- 開発 40 件平均絶対点 33,281,344.18 → 41,784,959.55（29 勝）、別の未調整 40 件 33,561,640.38 → 42,457,716.95（29 勝 11 分、敗戦なし）。未使用の平均誤答 12.825 → 6.075、高ノイズ群 34.13 → 15.73。160 出力の独立採点一致、最長 0.368434 秒、C++23 結合と決定性・期限検証を通過。研究ブランチに採用。
- 入力・ラベル列・seed は同一でも、異なるグラフサイズでは公式テスタのノイズ乱数消費が異なる。ローカル絶対点なので本番相対順位は未判定のまま。次はサイズを固定し、次数で区別しにくい候補を帯内の残差密度で分離する。

### AHC017 同日に工事する辺の迂回路干渉

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/6078ab465ffd5a225a464187bd443e4b62fcd38b/problems/ahc017/README.md)。辺の代替経路が同日に工事されるときの損失を、標本の最短路木と迂回距離から重みづけ。工事上限と連結性を守った日交換を行う。
- 低いほど良い絶対不満度: 開発 8 件 275,510,228 → 229,174,477（7 勝）、未調整 14 件 195,636,386 → 189,307,900（10 勝、−3.23%）。全 44 出力で日ごとの連結、容量、公式点と独立 APSP が一致。最大 0.045 秒、C++23 単一ファイルと汎用差分を検証。研究ブランチ既定に保存。
- 相対点に必要な各ケースの当時の最低値がなく、本番 2,000 ケースの順位は未判定。次は日が変わったときだけ標本始点の正確な距離を更新し、同じ予算を提案数の増加に使う案と比較する。

### AHC018 試し掘りと暗黙の重み付きネットワーク

- [実装と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/ff72f9a9577cff6a5ce466081ef959cdde836522/problems/ahc018/README.md)。平坦距離の接続と、離散的な少数の試掘を使った重み付き接続を比較。開発 32 件平均消費 226,792.16 → 202,702.41、独立 32 件 220,242.63 → 209,857.16。ただし未使用の 17 件で悪化し、ケース別比率中央値 1.0428、最大 2.963 倍。既定に採用しない。
- 全 128 対話が合法、公式得点と独立の水連結再生が一致。最長 0.060524 秒、C++23 単一ファイル検証。公式制限 5 秒、本番公開 3,000 ケース（C ごと 375）、相対式で当時の最低値が必要。順位表の 1 位点数も未取得、順位は未判定。
- 次は全面を試掘せず、暫定経路周辺だけ、改善できないと分かった時点で探索を打ち切る。

### AHC019 本番条件

- [公式問題文](https://atcoder.jp/contests/ahc019/tasks/ahc019_a) 通り両投影を一致させ、ブロックは面連結、共有 ID は平行移動と回転のみで一致（鏡映なし）。6 秒、本番公開 2,000 件（D=5〜14 各 200）。順位は当時のケース別最小値からの相対点。
- 1 位 1,886,240,066,109（相対点）、[順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc019.zip) SHA-256 `0d6fcdb3d26939918d3a5b49a81ced6e042b8351e667121e51396a00fd60efe8` 照合。単位ブロックを基準にし、両側で同じ連結ブロックを増やす案を比較する。

### AHC018 経路に限定した試掘と再計画

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/0cca8b704bae33b8ad70a32d5db6ae139fb5c054)。新規 dev/holdout 各32件で一様経路・再計画のみ・経路試掘を比較。holdout では一様に対し試掘が25勝7敗（費用比平均0.9202、中央値0.8278、最悪2.2002倍）。再計画だけに対して18勝14敗（費用比平均0.9674）。192対話で公式と独立再生が一致、最大0.217050秒。最悪ケースが大きく二案とも任意。次は予定経路を外れて調べる価値と場所を、不確実性と迂回費の推定から選ぶ。

### AHC019 共通ブロックの配置

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/7252e85e2d8a5d75038c0680f4ba43b4a43b3f05)。2つの立体で共通化できる形を24通りの正回転で探し、連結なブロックを置く。単位立方体だけの基準版に対し、dev10件 529,000,000,000→10,885,395,414、未調整20件 1,071,000,000,000→38,523,747,247（すべて改善、絶対ペナルティ −96.4%）。公式採点と独立の投影・面連結・正回転チェックは60出力で一致。最大約0.056秒、C++23の単一ファイルでも得点再現。ケース別基準がないため本番相対点は未判定。次は終盤に専用部品が残った例を、既存2〜3ブロックの部分再構築で修復し、同予算の再始動と比べる。

### AHC020 本番条件

[公式問題文](https://atcoder.jp/contests/ahc020/tasks/ahc020_a)。本番300ケースの絶対点を集計（各ケースで丸め）、2秒 / 1,024 MiB。1位504,832,157は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc020.zip)で確認（SHA-256 `5cdc8b811a4c6062f059beda557c49df3458948c979d2ef65eb69ccd75600866`）。公開ジェネレータを本番と同じケースとみなさない。まず送信半径を固定し、枝刈りした全域木と、送信局だけを順に最短経路で結ぶ方式を比較する。

### AHC019 同予算での部分再構築

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/79fa2df9de9de4de4d658ea3f374ff983d7e4753)。最初の共通ブロック解を揃え、その後の200msで全再始動と部分再構築を比較。未調整30件の絶対ペナルティは初版74,610,775,271、再始動49,583,909,998、部分再構築30,684,752,148（同予算で25勝、−38.1%）。片側専用ブロックの体積合計は28→8→0。全120出力を公式採点・独立検証で照合、最大0.261秒。問題別ブランチの既定解に採用。本番相対点は未判定。次は共通ブロックの葉だけを剥がし、専用ブロックなしを保ったまま共有ブロック数・逆体積和を減らす。

### AHC020 発信局を結ぶ配線の再構成

[MSTの枝刈りと発信局間の経路構築を比較](https://github.com/tokotoko7777/ahc-workspace/blob/96a97144e57022f452ce216eddf9c2d76adc3b32/problems/ahc020/README.md)。半径を全く同じにして、dev16件と未調整16件の全32件で改善。未調整平均は1,674,784.38→1,687,384.19。64出力のカバー・整数採点は公式と独立検証で一致、最大0.026912秒。別ケースなので1位を超えたとは判定しない。専用ブランチで採用。次は半径を決める探索にも、発信局集合ごとの配線費を入れて、枝刈り木の配線費を使う対照と比較する。

### AHC021 本番条件

[公式問題文](https://atcoder.jp/contests/ahc021/tasks/ahc021_a)。2秒 / 1,024 MiB、150ケース。三角格子で親子の順序違反が0なら100,000−5×交換回数、それ以外は50,000−50×違反数。1位13,589,420は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc021.zip)で確認（SHA-256 `aea95345b11743f87a02ec87c075e00d0686925de9c51d84f21d6f4d624dcd5a`）。公式ツール付属の100例を本番例とみなさない。リポジトリの既存ビームを対照に、最小・最大ラベルの両側から確定する案を試す。

### AHC020 半径と配線費の共同評価

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/411425e2f761d00f879c4965b8eee3b9e959e56a)。同じ初期半径・12,000試行・同じ最後の配線再構成で、探索中だけ枝刈りMSTと活動局集合ごとの再配線費を切り替えた。新規holdout16件の平均は1,654,711.81→1,665,150.44（12勝4敗、中央値+10,253）。devでは平均+6,164.13だが7勝9敗、中央値−706。全64出力が公式と独立採点で一致、最大0.340581秒。任意フラグとして保持。次は時間内に両評価で独立に探索し、最後に公式と同じ費用で選ぶポートフォリオを、同じ予算の独立再始動と比較する。

### AHC021 両端からの前線確定

[検証ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/9306c3d973e9b622e3a35431363f5f7f6ab64225)。リポジトリ既存の幅64ビームと、最大球も先に底側で確定できる両端方式を比較。dev10件は905,175→886,080、未調整20件は1,813,745→1,774,960（交換37,251→45,008、すべて悪化）。全60出力が順序違反ゼロ、公式と独立の得点一致、最大0.541秒。既存ビームを既定に残す。次はその交換列を末尾から調べ、後続交換が作る順列を使って、親子の順序を崩さず削れる操作を見つける。

### AHC022 本番条件

[公式問題文](https://atcoder.jp/contests/ahc022/tasks/ahc022_a)。対話式、最大10,000測定、4秒 / 1,024 MiB。本番3,000件でケース別最高点による相対評価。1位点は公開データから未確認。温度1点の繰り返し測定を基準に、複数方向の温度パターンと、候補が紛らわしい地点に測定を振り分ける方法を試す。

### AHC021 後続の置換を使った不要交換の削除

[追試](https://github.com/tokotoko7777/ahc-workspace/blob/ce5987e9b2769010a4568b3442108f2e02e29909/problems/ahc021/README.md)。同じ既存ビームの出力から、後続の交換により最後に影響する2頂点を特定し、ヒープ条件を維持できる操作だけを削除した。新規dev12件22,341→22,332交換、未調整20件37,088→37,075交換（10件改善、+65点、悪化なし）。64出力の公式点は独立再生と一致し全件完成、最大0.431秒。枝の既定に採用。次は2つの交換を同時に削除する近傍を調べる。

### AHC022 複数方向の温度を符号にする

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/blob/d255c7a5ce936552a98346c8d2bf764f78add849/problems/ahc022/README.md)。温度1点の反復測定を対照に、複数方向の温度と曖昧な位置への追加測定を使用。dev・holdout各30件（ノイズ30段階）で対応誤りゼロ。holdoutでは30件中29件で改善したが、最小ノイズ1件の悪化が大きく単純平均は11,857,279.70→9,850,827.03。相対点は未計測。120出力は公式テスタと独立再生が一致、候補最大0.059609秒・5,477測定。問題別ブランチのみ採用。次は盤面の温度差を減らして配置費を下げつつ、誤対応増加を避ける。

### AHC023 本番条件

[公式問題文](https://atcoder.jp/contests/ahc023/tasks/asprocon10_a)。2秒 / 1,024 MiB。収穫した作物の期間和を `1e6/(HWT)` 倍した絶対点の合計（本番2,000件）。[公式本番seed](https://img.atcoder.jp/ahc023/seeds.txt)は2,000件でハッシュ `0c2ffe8037c0c232ed9775b89a892fc1f68709a1582c83fd6bb609244cefcf32` を照合。1位点は未確認。通路を木で確保し葉に植える基準との比較で、子孫の植付・収穫日に干渉しない期間を探して内部マスも使う。

### AHC022 低配置費の符号盤面

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/86f0b8b8dc3d75a877626672b36b2dec88da2e05)。識別用の温度を保ち、温度差が大きい辺を減らす。新規の開発30件・未調整30件は両版とも全件誤対応0、低配置費版が全60勝。未調整のケース別絶対点比は平均1.666倍、中央値1.728倍。配置費比平均0.5673、計測回数平均2,007.47→2,033.40、最長0.081582秒・最大5,909計測。全120出力公式と独立再生が一致し、問題別ブランチの既定に採用。次はノイズ段階ごとに符号の温度差と測定予算を配分する。本番相対点とは比較しない。

### AHC023 木の通路を時期ごとに使う

[開発検証と実装](https://github.com/tokotoko7777/ahc-workspace/blob/52f76e1f5e905e5f4f11f0613285760073967281/problems/ahc023/README.md)。葉だけに作付けする版に対し、子孫の植付・収穫を妨げない時期に限って内側のマスを使う。dev12件5,447,400→6,551,125、未調整24件10,825,325→13,194,825（全勝、+21.9%）。72出力は公式と独立の毎日の到達判定が一致。

別に、開発比較で凍結したC++23版を[公式公開seed](https://img.atcoder.jp/ahc023/seeds.txt)で評価し、**2,000件合計1,100,483,250**、全件合法・独立採点と一致、最大0.035019秒。凍結バイナリSHA-256 `87a1ca2ae0f370e104b83d2f4b69590cb0eed6d79c5b625797adc0bbc51c5a69`。本番1位値は未確認なので比較結果は未判定。改善判断には本番ケースの結果を使わない。次は植付・収穫の同日順序を使って、内部マスの禁止期間を狭める。

### AHC024 本番条件

[公式問題文](https://atcoder.jp/contests/ahc024/tasks/ahc024_a)。空きマス数+1の本番150件合計、2秒 / 1,024 MiB。1位335,417は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc024.zip)で確認（SHA-256 `b4f02d3cbcea3200d3e7d29dcf27b1e0bcce1fb566ed249541683114f7986585`）。本番同一ケースは未確認。行列の直線圧縮に対し、色ごとの連結と色の接触関係を保つ曲がった切断を試す。

### AHC023 植付と収穫の順序を分けた期間制約

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/2b4a9df1aa5ad9cdc0d435bb8271d669d1fe43a6)。同じ日でも植付と収穫の開始・終了はアクセス条件が異なるので、安全な順序を使って期間を拡大。初版に対し新規dev16件8,777,400→10,001,200、未調整28件15,388,700→17,383,850（+13.0%、全件改善）。88出力が公式と独立再生で一致、改良版最大0.013410秒、単一ファイルでも得点一致。問題別ブランチの既定に採用。公式2,000件の既報得点は初版のもので、改良版の比較・調整には使わない。次は近隣の部分木の植付・収穫日を揃え、祖先マスの長い空き期間を作る。

公式2,000件の再現は、[公開済みの凍結版](https://github.com/tokotoko7777/ahc-workspace/tree/52f76e1f5e905e5f4f11f0613285760073967281)をチェックアウトし、`g++ -std=c++23 -O3 problems/ahc023/main.cpp -o windows`。公式ツールを `cargo build --release --locked --bin gen --bin vis` で作り、`gen official-seeds.txt` で入力を生成。各入力に対して凍結版を1回・2秒上限で実行し、`vis input.txt output.txt` と `problems/ahc023/benchmark.py` の独立採点が一致した2,000整数点を合計する。

### AHC024 曲がった切断で地図を圧縮

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/blob/c5b3b9f3d88b74ff5f9fa67446d16555dd509ef1/problems/ahc024/README.md)。直線圧縮を揃えた後で、各行で少しずつ位置をずらせる切断をDPで選ぶ。新規のdev24件平均1,588.25→2,004.71、未調整24件平均1,621.42→2,025.63（全48件改善）。96出力で公式得点と独立の色連結・接触関係の検査が一致、最大0.037359秒。問題別既定に採用。次は異なる切断経路を作る再始動と、同予算の単純な順番変更を比べる。

### AHC025 本番条件

[公式問題文](https://atcoder.jp/contests/ahc025/tasks/ahc025_a)。対話式・比較回数Qを厳守、2秒 / 1,024 MiB。絶対評価は重さ合計の母標準偏差を100倍し丸めて+1、低い方が良い。本番5,000件、各ケースで自分より小さい絶対値の提出数＋同値の半分を用いる順位相対点を集計。1位492,832,027,444は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc025.zip)で確認（SHA-256 `5f372be0c4814bc89cf1e5985c5b36e7816b1c0e01ae204d7f78f67c8505f667`）。歴史的ケース別全提出の絶対点は未取得。重さ順位の推定だけで割り振る基準と、比較回数を一部残して改善が確実な交換だけを行う案を比べる。

### AHC024 開始盤を多様化した再始動

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/a0aa3ad5ff0cf55d5f3f6ea9c8714d8ca19bcfe9)。初回得点と追加10再始動を同じにし、追加探索の開始盤を変えるかどうかだけ比較。dev24件平均2,052.25→2,061.17、未調整24件2,048.33→2,065.79（18勝1分5敗、得点差中央値+18.5）。未調整の異なる開始盤は平均1→9.96。全96出力の合法性と公式点が独立検証に一致、最大0.327639秒。問題別既定に採用。次は色の希少な接触や連結を壊すセルを事前に検出し、同じ試行数で合法な切断の候補を増やす。

### AHC025 比較回数を確実な交換に残す

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/blob/21034a6404bafe091bac193b8315b0ffd1ed0686/problems/ahc025/README.md)。個体の順位推定に全Q回を使う版と、厳密なグループ比較を使う改善へ一部残す版を比較。dev16件81,154,845→42,766,854（14勝）、未調整32件136,670,998→77,452,712（23勝、−43.3%）、96出力で厳密にQ回・合法、公式と独立採点が一致、最大約0.049秒。問題別既定に採用。低Qの小標本で悪化したため、順位推定を先に行う案を別の低Q標本で検証する。

[2023-10-23の公式追記](https://atcoder.jp/contests/ahc025/clarifications?lang=ja)によると、本番相対点の係数は問題文の10^9ではなく10^8で集計し、そのまま確定した。記録の1位はこの本番尺度。厳密比較には当時の各ケースにおける参加提出の絶対点分布が必要。

### AHC026 本番条件

[公式問題文](https://atcoder.jp/contests/ahc026/tasks/ahc026_a)。`max(1, 10000−移動費合計)` の本番150件合計、2秒 / 1,024 MiB。1位1,423,183は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc026.zip)で確認（SHA-256 `c9acb296ceff4948df2cb000459d8153af4ba8ddb60c987b565e19b628dd845a`）。本番同一ケースは未確認。既存のロールアウトを計測しつつ、ビーム内で次の箱の上にある塊を二分する手を試す。

### AHC025 低Q層で順位推定を優先する追試

[検証](https://github.com/tokotoko7777/ahc-workspace/tree/93cfb59980bf24166658b1b40f60363cd8e63cf3)。初回の小標本で見えた仮説を、独立の低Q層30件と高Q対照6件で検証。低Qの絶対ペナルティは従来197,259,539、順位推定の余りだけで改善する版336,043,600、順位推定だけ364,972,714（低い方が良い）。新案が従来に勝ったのは2/30。高Q対照6件では変更なし。162対話の回数・絶対点は公式と独立検査で一致、最長0.038秒。新案は任意に残す。次は固定比率を変えるのでなく、不確実なグループの重さ合計に情報が集中する比較を早い段階で選ぶ。

### AHC026 箱の塊を二分するビーム

[比較](https://github.com/tokotoko7777/ahc-workspace/tree/9d10bdadaa08a7ca84e01228951b4e965082f00b)。既存の決定的ロールアウトを無変更で対照にし、塊を一括移動するビームと二分移動を含めたビームを追加。分割版は一括版に全32勝（未調整16件で+129.25点/件）だが、既存版の9,344.63点/件に対し9,273.06点/件。96出力が公式と独立再生で一致、分割側は全件200段完了し最大0.139015秒。既存を既定に維持。次は既存解の途中から分割操作を入れ、残りを既存ロールアウトで評価する。

### AHC027 本番条件

[公式問題文](https://atcoder.jp/contests/ahc027/tasks/ahc027_a)。2秒 / 1,024 MiB。全マスを訪れ原点に戻る長さ10万以下の閉路を繰り返したときの平均汚れを丸めた絶対値を最小化。本番2,000件の相対点は当時のケース別最小値に依存し、その値は未取得。1位1,996,375,186,737は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc027.zip)で確認（SHA-256 `f557d24377e00d105d6d2a0c5c5413166b6fe87662bdea5b4f163e0b64f21e1b`）。DFS周回を基準に、立ち寄り追加・削除と経路の反転を実際の訪問間隔で比べる。

### AHC027 巡回の訪問間隔を直接評価する

[比較と実装](https://github.com/tokotoko7777/ahc-workspace/blob/3ec2d6d02ab6aea5b461002201803016ce56ecb8/problems/ahc027/README.md)。同じ48本のDFS閉路から、追加予算でDFSを生成し直す版と、立ち寄り追加・区間の短絡や反転を訪問間隔の正確な費用で選ぶ版を比較（両方440ms上限）。dev12件373,539,272→305,718,347（11勝）、未調整24件623,349,268→568,819,988（−8.75%、19勝）。72出力は公式visと、2周期分の汚れを逐次再生する独立検証が一致、最大0.456秒。問題別既定に採用。公式本番2,000件のseedはハッシュのみ確認、ケース別基準値未取得。次は汚れの増加率が高く訪問間隔の長いマスを選び、間隔の中央近くに短い寄り道を足す。

### AHC026 既存ロールアウトの途中に分割を挿入する

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/22fe2ca8760d43489acbd895b3d63a1c9933963f)。同じ24途中状態から一括・分割を各9候補、4種類の同じ固定方針で完走（各864回）。dev20件の平均は既存9,348.90、一括9,349.65、分割9,348.90。未調整20件は3方式とも全件同点で平均9,320.65。80出力で公式と独立再生が一致、最大0.449119秒。既定は変更しない。次は終盤で不利に見える途中状態だけを候補にして、固定方針ではなく既存の適応ロールアウト自体を再実行する。

### AHC027 長い訪問間隔を狙った寄り道

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/ba4622d04a7fede9644fee8bdf0408a364eaa1ec)。同じ既存閉路から+150msを追加のランダム部分変更に使う版と、増加率×訪問間隔²が大きいマスを間隔中央付近で訪問する版を比較。新規未調整20件の絶対点は追加なし368,444,085、同予算ランダム286,174,236、中央を狙う版275,950,662（対同予算16勝、−3.57%）。90出力が公式と独立再生で一致、改良最大0.596秒。問題別既定に採用。次は寄り道から同じ起点へ折り返さず、少し先の巡回点へ再合流して手数を削る。

### AHC028 本番条件

[公式問題文](https://atcoder.jp/contests/ahc028/tasks/ahc028_a)。2秒、本番150件の絶対点合計。全単語を含めたときは`max(10000−入力費,1001)`。1位1,105,471は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc028.zip)で確認（SHA-256 `7bc6df6d9f1b97c57d2a540f9c286d2acc7374775fef712b37f4923bda378035`）。100件の付属データを本番ケースとはみなさない。貪欲法を基準に、単語の重なりで順序を探索してから、文字の座標列をDPで厳密に選ぶ。

### AHC023 凍結初版の本番比較

[AtCoder公式採点アーカイブ](https://img.atcoder.jp/ahc_standings/index.html?contest=ahc023)の[ケース別点数CSV](https://img.atcoder.jp/ahc_standings/ahc023/result.csv)（SHA-256 `34fbe2ecf682d2d6bc6472dff1e636911bca02cd937f627db678ca1cf53acd00`）を集計。909人×2,000件、負点は0にして合計した。上位3人の順位は[公式結果JSON](https://atcoder.jp/contests/ahc023/results/json)と一致。1位1,782,120,500に対し凍結初版1,100,483,250、差681,637,250点。**初版は未達**。植付・収穫の順序を分けた版は同じ2,000件で未評価のため未判定。次は兄弟の葉を一括して最小費用流で再割当し、同予算を個別再割当へ使う対照と比較する。

### AHC028 単語順の厳密な局所編集

[初回](https://github.com/tokotoko7777/ahc-workspace/tree/84d5508c4284fa8e75116f1a038a8124946246ac)の単語間近似コストによる全体探索は、dev・未調整各24件で既存貪欲とすべて同点（最大0.030527秒、全96出力合法）。[追試](https://github.com/tokotoko7777/ahc-workspace/tree/a9f558c6c273ccb8f26433e8554a5e1aa8a2dc8d)では動かす部分の前後の座標DPを保存し、単語順の編集を全体の入力費で厳密に評価。別系列のdev24件7,150.42→7,170.13、未調整24件7,116.83→7,139.67（全48件改善、未調整中央値+22.5点）。全96出力の公式点は独立再生と一致、最大0.137921秒。問題別既定に採用し、次はより長い単語列の移動を同じ厳密評価で試す。

### AHC029 資金の時間価値を使う方針は不採用

[検証](https://github.com/tokotoko7777/ahc-workspace/tree/400b9b1a8b5f302c0d5eccacc48953bed87484bd)。基準方針と、先の仕事・購入の価値を割り引いて選ぶ方針を比較。dev14件796,926→677,707（4勝）、未調整28件5,802,068→1,497,970（3勝）。合計投資回数165→134。84対話で公式・独立の資金再生が一致、最大約0.127秒。既定は基準版のまま。次は投資時に使える現金を確保する制約と、古い仕事や手札の更新を試す。[公式採点アーカイブ](https://img.atcoder.jp/ahc_standings/index.html?contest=ahc029)から首位点を再集計（同じseed列かは別途確認）。2秒、本番3,000件、ケース別の当時の最高最終資金で正規化する。

### AHC030 本番条件

[公式問題文](https://atcoder.jp/contests/ahc030/tasks/ahc030_a)。3秒 / 1,024 MiB、対話上限2N²。本番3,000件の費用ベース相対点。1位2,140,937,483,257は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc030.zip)で確認（SHA-256 `d276d9c5f69390704a2d1692015273b0062a6b04372e9ec35c0c3cb8e05aa1c2`）。全マスを確実に探索する基準に対し、形状の配置候補を絞って不確実なマスを先に掘る。

### AHC030 形状配置の候補を対話で減らす

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/ff4f0b02b3b5d35bb7a63748604649304e7c5d15)。全探索の基準と、各形状の合法配置を維持して情報量が大きい地点を優先採掘する案を比較。dev40件平均233.525→78.975、未調整40件205.725→67.825（全80件改善、未調整のケース比中央値0.31375）。全160実行合法で最初の回答で正解、公式と独立採点が一致、テスタ込み最大0.038554秒。問題別既定に採用。次はグループ測定を各候補の重み付けにのみ使い、正解の可否は厳密照合で決める。

### AHC022・027・029 公式のケース別点数

[AtCoder公式アーカイブ](https://img.atcoder.jp/ahc_standings/index.html?contest=ahc022)の `result.csv`（絶対点）と `input.csv`（ケースファイル、seed、特徴量）を入手。公式ビューアは同じ添字で関連付け、3大会ともseed列は公式公開 `seeds.txt` と全件一致。整数の四捨五入とケース別最高値（022・029）／最低値（027）を使い、公式結果の上位3人の順序も再現した。

| 回 | 本番件数 | 首位相対点 | ケース点のSHA-256 |
| --- | ---: | ---: | --- |
| [AHC022](https://img.atcoder.jp/ahc_standings/ahc022/result.csv) | 3,000 | 2,808,216,532,758 | `5864a1b7c0699e66db295ed12611c53567e944f5bbd8965621ac8648b34e2cf1`、[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/8e7f5ef9c7866da0db52954c549e62fa5fc5c791) |
| [AHC027](https://img.atcoder.jp/ahc_standings/ahc027/result.csv) | 2,000 | 1,996,375,186,737 | `80d2b280d6310b9ff5b03b63d26e17a6b1053fdad2326f98385cdebf59651471` |
| [AHC029](https://img.atcoder.jp/ahc_standings/ahc029/result.csv) | 3,000 | 2,200,575,652,801 | `94099efcac74f59b7bd0261227702176ff4deada7f5e68c18584ef50758ec274`、[増資後の追試](https://github.com/tokotoko7777/ahc-workspace/tree/94c34b158aa59b9a2d75153bd452ffa67faab2bd) |

先にAHC022の凍結版を、生成パラメータと全ケース特徴量を突き合わせたうえで比較する。従来の未調整ケースに対する実験を、本番評価に読み替えない。

### AHC023 兄弟の葉の作物を一括で再割当

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/blob/ac291d5344ff4f4eca283e694da5ba3737b14218/problems/ahc023/README.md)。共通祖先・外側の割当を凍結し、同じ作付け可能日をもつ兄弟葉の作物をまとめて最小費用流で選ぶ。dev12件7,462,425、未調整24件14,779,075で、どちらも従来と全件同点。全72出力で公式と独立検証が一致、候補最大0.043439秒。一括の方が得になる人工例と、1,200件の全列挙比較は成立するが、実入力で改善せず任意。次は固定した葉の間だけでなく、祖先側の時期も同時にずらす再構築を試す。

### AHC022 初めての本番相対比較

[監査・再現コマンド](https://github.com/tokotoko7777/ahc-workspace/blob/8e7f5ef9c7866da0db52954c549e62fa5fc5c791/problems/ahc022/OFFICIAL_AUDIT.md)。問題別既定の低配置費版を凍結し、公式公開の一列 `seeds.txt` をそのまま公式ジェネレータに入力。全3,000件でアーカイブのseed・L・N・Sが一致した。Sの強制指定はヘッダが一致しても完全一致が926件だけだったため、別入力と明記して除外。全3,000件で誤対応0、公式テスタと独立採点が一致、テスタ込み最大0.224637秒。

凍結版 **271,023,954,031**、公式首位 **2,808,216,532,758**。当時のケース別最大値を超えたケースは0のため、両者を同じ基準で再計算しても点はそのまま。仮に加えると1,064人中307位、同点なし。**本番未達**でmain未マージ。次はより少ない計測回数とさらに低い配置費で同じ誤答水準を保つ案を、別の開発・未調整seedで検証する。

### AHC025・030 公式のケース別点数

[AHC025公式結果CSV](https://img.atcoder.jp/ahc_standings/ahc025/result.csv)は1,085人×5,000件、ヘッダ `rank_min,100000000`。正解の順位に他の同値正解者の半分を入れて丸め、公式首位492,832,027,444と順位を再現。失敗の提出も分母に入る。比較時は固定の当時分布と、仮想追加した分布とを区別する。[AHC030公式結果CSV](https://img.atcoder.jp/ahc_standings/ahc030/result.csv)は1,132人×3,000件、ケース別の最小値を用い首位2,140,937,483,257を再現（SHA-256 `2ba001dcd7955e7f3607e9eb5e53737dbfa5d61e3e35d34c984ed9e4ba01f252`）。どちらも公式公開seed列・ハッシュが `input.csv` と一致。候補の本番点はまだ未計測。

### AHC029 増資後に仕事と手札を入れ替える

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/94c34b158aa59b9a2d75153bd452ffa67faab2bd)。増資前まで同じ対照に揃えた新規系列で、dev12件の最終資金合計は既存390,924、入替214,820、入替＋現金留保194,627。未調整24件は989,623、1,865,944、1,854,949（入替が既存に15勝だが中央値は21,337.5→16,810）。初期探索分を含む252出力が公式と独立再生で一致、最大0.062秒。両方任意、既定維持。次は現在の手札から見た仕事完了までの時間と、更新カードを1手使うことを直接比較する。

### AHC030 グループ測定の追試と凍結版の本番比較

[測定を追加する追試](https://github.com/tokotoko7777/ahc-workspace/tree/a285e872f70c776469b2fe2780c9664de40bcc2b)では、別の未調整40件の平均費用は69.90→69.40362だが、ケース比平均1.02022、devは成績低下。0.078703秒で全出力合法だが既定にしない。

[公式比較](https://github.com/tokotoko7777/ahc-workspace/blob/3dfdfcce6f81b2b034351960f8d506b8cdba735b/problems/ahc030/OFFICIAL_AUDIT.md)は、追試前の厳密な配置候補版をそのまま凍結。公式公開seedで3,000件を生成し、公式アーカイブのseed列・N・M・ノイズ率・総埋蔵量と一致した。全件で最初の回答が正しく、テスタと独立採点も一致。最大0.068358秒。相対点は **323,222,495,387**、本番首位 **2,140,937,483,257**。歴史的最小値を更新したケースは0なので、同じケース別基準で比較できる。仮想順位171/1,133、同点なし。**未達**、mainへはマージしない。次は、掘る前にグループ測定を絞り込みへ効率よく使う案を、独立したseedで試す。

### AHC031 日をまたいで列を固定する

[公式問題文](https://atcoder.jp/contests/ahc031/tasks/ahc031_a)は3秒 / 1,024 MiB。絶対費用は不足面積×100と前日との差分の壁長に1を足し、最小化。2,000ケースの相対点を[公式アーカイブ](https://img.atcoder.jp/ahc_standings/ahc031/result.csv)から再計算すると、1位1,722,250,219,489（アーカイブSHA-256 `3bc6bdaf8e6e259a4b80a5e45cfde40ff63349ce7a72cbba07cca24f88245fea`）。公式公開seed列は `input.csv` と一致。

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/c1d15b819b45223d0c27bc3191918083c1b18362)。日ごとに独立配置する基準に対し、縦の境界は全日で固定して枠内で割当を変える。dev16件16,864,516→11,661,690（8勝）、未調整32件52,604,232→47,399,232（17勝15分、−9.89%）。96全出力の合法性と採点は公式と独立検証で一致、最大約0.090秒、C++23単一ファイルも一致。問題別既定のみ。本番ケースは未測定。次は固定された列の中で、面積を守りながら前日と横境界を揃える。

### AHC025 歴史的な順位分布での本番比較

[監査手順と実装](https://github.com/tokotoko7777/ahc-workspace/tree/a0f8cbdebfed6d111fead325f78dfed18fe21923/problems/ahc025)。低Q追試でなく、先に採用した比較回数を確実な交換に残す版を凍結。公式seedのまま生成した5,000件でN・D・Qをアーカイブに照合。全5,000件で公式・独立の絶対点が一致し、合計3,498,575回の合法な比較を各ケース指定どおりに行った。テスタ込み最大0.418925秒。

当時の1,085人分布を固定すると、候補354,364,239,662点、公式首位492,832,027,444点。仮に候補追加後の1,086人を共通母集団にして双方を再計算すると、候補354,498,342,544点（313位相当）、同じ首位492,836,970,699点。どちらでも**未達**、mainへのマージなし。次は個別順位を深追いせず、重さが不確実なグループを、他の組と直接比較する予定。

### AHC032 本番条件

[公式問題文](https://atcoder.jp/contests/ahc032/tasks/ahc032_a)。1位11,854,207,127,046は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc032.zip)から確認（SHA-256 `6d8a7a6831a00dfc7771a537e5417d2873d5f70bd6f35f84f4a60b20e0497f2a`、公式結果JSONで1位名一致）。付属100ケースを公式の150件とみなさない。既存のビームを出発点に、同じ初期解のスタンプを2手まとめて入れ替える案を試す。

### AHC031 面積を維持する横壁整列

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/fb4acc4255f313b63cc46c95b3965fc983b2d528)。列幅と部屋の所属を固定し、必要な面積を落とさない横壁位置を動的計画法の変数にした。前日・翌日の壁との一致を増やす。dev14件34,297,746→32,175,098（12勝）、未調整30件60,609,480→52,948,706（25勝5分、−12.64%）。不足面積分は31,090,500で変わらず、横壁の差分が29,518,950→21,858,176。全88出力の公式と独立検証が一致、最大約0.122秒。問題別の既定に採用。次は凍結版を同じ本番2,000件で測定し、その結果は改善に使わない。

### AHC032 スタンプの2手編集

[検証](https://github.com/tokotoko7777/ahc-workspace/tree/0f8a8d2946ce1c6691db7ce95eee5ffa8a51006e)。既存の幅9,000ビームは初期のサンプルで2秒を超えたので、基準・候補をともに幅3,500に揃えて全件49手まで実行。同じ解から重なる2手を取り除いて位置を厳密比較しても、dev16件・未調整16件で改善なし。未調整平均79,106,180,582.44。全64件が公式と独立検証で一致・合法、最大1.194476秒。2手編集は既定にしない。次は同じ総数のビーム候補を複数の盤面の向きに分けて解く。

### AHC033 本番条件

[公式問題文](https://atcoder.jp/contests/ahc033/tasks/ahc033_a)。5×5の搬送、3秒 / 1,024 MiB、本番2,000件。ターン数、出口順の逆転・誤配送・未配送のペナルティの和を小さくし、ケース別最小値で相対化する。大クレーンだけで順序どおり出す基準と、小クレーンが前半に並行して取り出せるよう準備する案を比較する。

### AHC032 同程度の評価回数を異なる向きへ分ける

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/890cee4f0a8b60a076d3c98d6d98d6bffed43099)。幅3,500の1方向に対し、幅1,750ずつを対向する2方向に使い、同じ入力で候補を比較。評価回数比は未調整で平均1.00291。未調整16件の平均は79,089,863,247.81→79,109,974,914.63だが5勝2分9敗・中央値約−895万、devも平均悪化。既定は1方向。全64出力が公式・独立採点で一致し最大1.395991秒。次は終盤で手数を使い切れるよう、手数別に候補を残す。

### AHC033 小クレーンで先に搬入を整える

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/131ef411f491cc1b6ed79c905fc98a8d52ffad5d)。各行の先頭1〜2個を小クレーンで同時に作業し、大クレーンの直列搬出につなぐ。dev16件5,411→4,870、未調整32件10,813→9,754ターン（−9.79%、全件改善）。全96出力が誤配送・逆順・未配送ゼロで、公式・独立検証が一致。最大約0.0071秒。問題別既定に採用。[公式結果JSON](https://atcoder.jp/contests/ahc033/results/json)の1位はbowwowforeach。公式の得点アーカイブが未取得なので、本番相対値は未判定。次は前処理後も、小クレーンの行内作業を衝突なく大クレーンと並行させる。

### AHC031 横壁整列版の本番比較

[監査手順](https://github.com/tokotoko7777/ahc-workspace/tree/fe55a2264364f3c6f25df1fe9b5e12ff314d4186/problems/ahc031)。横壁整列版を凍結し、公式公開seedのまま生成した2,000件でアーカイブのfile・seed・D・N・Eを照合。全件合法で公式・独立・C++埋込評価が一致。最大0.142618秒。凍結版 **126,909,397,521**、本番首位 **1,722,250,219,489**。当時のケース別最小値を更新したケースは0なので、仮に追加しても同じ基準で、1,060人中399位相当、同点なし。**未達**。今後は横壁位置だけでなく、同じ形を長く使い回せる日別の割当・幅を検討する。

### AHC034 積載量を含む経路の厳密な評価

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/6c2ea91bb86ef24c6c84cd99bf76df6355c16ce6)。蛇行経路を残量が非負になる位置から始める基準に対し、区間をつなぐときの移動費と必要積載量を正確に評価し、順序を変更。dev24件平均23,160,596.71→30,677,647.17（23勝1分）、未調整24件25,131,542.38→33,027,861.63（全勝、移動費比平均0.74009）。96出力で全マスが整地され、公式と独立再生が一致、最大0.159890秒。専用既定に採用。

本番150件・2秒。1位9,486,350,384は[AtCoder提供順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc034.zip)から確認（SHA-256 `a6c2666e596dee5a743d80cfcfac8cf1d90d80696a2c11acfd4891b7467ce889`）。同じ本番ケースと確認したデータはない。次は移動を固定し、積み降ろしだけを時間方向の最小費用流で組み直す。

### AHC033 小クレーンの先行搬出

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/81b3ab227a32818d6d4350351c5d4db5c9602b0f)。行内限定で正しい出口に先行搬出する作業も候補に追加し、大クレーンの完走手順を残す。新規dev12件3,681→3,368、未調整24件7,384→6,867ターン（−7.00%、全件改善）。72出力で順番違反・誤配送・未配送ゼロ、公式と独立検証が一致、最大0.009秒未満。問題別既定に採用。次は行をまたぐ作業を時刻予約で併行させる。本番相対点は未計測。

### AHC034 移動を固定して積み降ろしを最適化

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/e2c29731322790e744a17033d3fa937c450cede7)。移動操作を一切変えず、同じマスを複数回通るときにいつ積み降ろすかを、時刻方向の最小費用流で決める。別の未調整24件平均30,949,419.92→35,727,578.04（全勝、移動費比0.86627）。別系列の近い時間予算の試験でも、経路焼きなましを多く回す対照の未調整平均約31.227M→34.774M（全24勝、中央値の時間0.152→0.164秒）。計192出力で公式と独立検証が一致、最大0.273714秒。問題別既定に採用。本番ケースは未確認。次は小さな経路変更の候補を絞り、積み降ろしを再計算して選ぶ。

### AHC035 強い特徴を子に残す配置

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/5e9d58aed81aae5dbdbe8563408ea91a0fbfdf57)。親の総評価を高くする対照に対し、長所が補い合う隣接候補と、座標ごとの希少な最大特徴の生存を優先。dev36件平均720,290.53→852,186.06、未調整36件722,317.89→857,950.08（全72件改善、未調整中央値+131,131.5点）。全144出力で公式と独立再生が一致し、テスタ込み最大0.161581秒。問題別既定に採用。次は序盤を固定し、終盤だけ子の最大得点の分布を評価する。

[公式問題文](https://atcoder.jp/contests/ahc035/tasks/ahc035_a)。対話10世代、2秒 / 1,024 MiB。本番300件の絶対点1位274,272,291は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc035.zip)で確認（SHA-256 `e42f0a604bb873d05845c8db311d31074d9e4e987a9235b7bd771e1dd6c0913b`）。本番と同じケースは未確認。

### AHC036 本番条件

[公式問題文](https://atcoder.jp/contests/ahc036/tasks/ahc036_a)。3秒 / 1,024 MiB。移動・信号操作合わせて100,000以下。信号操作の回数を最小化し、本番2,000件のケース別最低値で相対化する。[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc036/result.csv)（SHA-256 `da6d10f8d1339445009649d4e746709ca24751329a3434eb3ed645043174dccb`）1094人×2,000件を再集計して1位1,872,786,019,642。上位3人の順位と公式seed列を照合。最短路に沿って1都市だけ更新する基準に対し、通り道に沿う都市を配列上に連続して並べ、複数都市の信号をまとめて更新する案を試す。

### AHC035 最終世代の最大得点分布

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/503ea8770af2217fea4f8b9f3a866c144949ac60)。最初9世代を完全に揃え、最後の代理評価、回数を増やした代理評価、各子の合計分布から最大の期待値を取る案を比較。新規未調整36件の平均は867,204.75、866,434.08、866,918.64。厳密評価は既定に対して12勝14分10敗だが平均−286、中央値0。216出力で公式と独立再生が一致、最大0.210698秒。ライブラリは残し問題の既定は維持。次は最終世代の評価だけでなく、最終配置で組み合わせやすい特徴を一つ前の世代へ残す。

### AHC036 都市並びを工夫して信号をまとめて更新

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/fb26d23b729e0e30e19da4d6181bd0421588196e)。同じ最短移動列を通りつつ、よく続けて訪問する都市を配列に連続して置いて、一度の操作で複数都市の信号を更新する。dev12件65,381→23,451、未調整24件132,076→47,549（−64.0%、すべて改善）。未調整の移動数175,121は同じ。72出力の公式・独立・C++内検証が一致、最大約0.037秒、最大合計操作11,289。問題別既定に採用。次は配列を同じにしても、既に更新済みの都市を伝う経路や更新区間を合わせて選ぶ。

### AHC036 更新済みの都市を伝う経路を使う

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/5f1878dec204f611343c7e8b7a9066b7a0d8e3c3)。同じ配列から、既に通れる成分と一回のコピーで通れる区間を探して経路も選ぶ。dev12件の信号操作24,816→20,815、新規未調整24件46,370→38,507（−16.96%、全件改善）。移動回数は179,814→227,168と増えるが、全操作のケース最大は13,260で上限内。72出力で公式・独立・内部の得点が一致、最大0.079秒未満。問題別既定に採用。次に本番2,000件の凍結評価を行い、その出力は調整に使わない。

### AHC037 供給木の中継点を合成

[公式問題文](https://atcoder.jp/contests/ahc037/tasks/ahc037_a)。2秒 / 1,024 MiB、各ケース最大5,000操作。費用を低くして絶対点を増やし、本番150件の合計。公式結果JSONの首位はRafbill、点数と同一ケースは未確認。[比較](https://github.com/tokotoko7777/ahc-workspace/tree/11e02827a706e05022f50a3f7956541fa388f9ea)では、既存の目的点を親に使う基準に対し、複数の目的点をまとめる中継点を合成。dev28件平均22,779,093.11→35,997,057.68、未調整28件22,842,660.79→35,832,854.21（全56件改善、未調整費用比0.63749）。全112出力公式と独立検証が一致、最大0.421438秒・1,696操作。問題別既定に採用。

[親付け替えの追試](https://github.com/tokotoko7777/ahc-workspace/tree/38ce41a25380bb0b6d5b409da8631270b8a3f62f)は同じ中継点集合から、目的点のみ／全中継点で親を付け替え。別のdev28件・未調整28件で3方式すべて得点・操作数同点。全168出力公式と独立検証が一致、最大0.460654秒。任意として残し、次は別の構築で得た中継点を混ぜて親を探す。

### AHC036 凍結版の本番比較

[監査手順](https://github.com/tokotoko7777/ahc-workspace/tree/2deddb2a4c35d9bfdbddaa21523042a8f17842c2/problems/ahc036)。更新済み区間を使う版を凍結し、公式seedのまま生成した2,000件を公式アーカイブと照合。全件合法で公式・独立・C++内得点が一致。信号更新合計3,095,915、全操作最大14,613、最大約0.164秒。ケース別最低値を更新したケースは0で、同じ基準で候補 **1,048,370,275,931**、本番首位 **1,872,786,019,642**。仮に加えると1,095人中197位、同点なし。**未達**でmainへはマージしない。次はよく使う区間を選ぶだけでなく、目的地間の移動経路を事前にまとめて見直す。

### AHC038 本番条件

[公式問題文](https://atcoder.jp/contests/ahc038/tasks/ahc038_a)。3秒 / 1,024 MiB、全対象を移動した場合は操作数、未完の場合は100,000+1,000×未完数を最小化。本番2,000件の相対点。[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc038/result.csv)（SHA-256 `f42a9700defa9c421eeeb52ae25c98e687ec25ecb982f5961f9a44e8ff5bac71`）から首位1,986,808,184,140を再現。CSVの首位ラベルはUSA、[公式結果JSON](https://atcoder.jp/contests/ahc038/results/json)の首位ラベルはnikaj。2〜3位は一致するが、データ上の表記差として記録する。既存ビームと、根の位置から複数指を一括で動かす案を比較中。

### AHC039 本番条件

[公式問題文](https://atcoder.jp/contests/ahc039/tasks/ahc039_a)。2秒、150件の絶対点合計。1位566,997は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc039.zip)で確認（SHA-256 `34d478f166334d6c8a0f7e0d03f6b5c17c557eeff9f163d5510ecc8605a9f3cd`）。同一の150件は未確認。矩形を基準に、隣り合う列で上下端を変えた多角形を厳密な得点・周長制約で比較する。

### AHC038 複数指の同時作業を一括で選ぶ

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/12ebdf8c71f19f11fac5b9c2053ae4267a053a18)。未調整22件の合計は時間上限を設定した既存ビーム8,811、既存の貪欲のみ9,948、新しい同時作業案8,888ターン。候補は貪欲より10.66%短いが、ビームには4勝18敗。128出力が全件完成し公式・独立検証が一致。最大時間はビーム2.258秒、候補0.039秒、貪欲0.044秒未満。既存ビームを既定、候補は軽量オプション。次は従来ビームに候補の完成解・同時作業のまとまりを渡し、同程度の計算量で比較する。

### AHC039 共通の行を持つ可変幅の多角形

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/39ff40a8cce317a2ed00157ebb8cefefeeff0b16)。矩形だけの基準に対し、各列の上下端を可変にして、全列が共通の行を含む多角形をDPで選ぶ。dev24件平均1,897.75→3,342.96、未調整24件1,913.29→3,429.71（全48件改善、未調整中央値+1,465点）。公式・独立の単純性、周長、魚の選別が全96出力で一致、最大0.121809秒、頂点最大64。問題別既定に採用。次は共通の行をなくし、隣り合う列だけが正の長さで重なる多角形まで許す。

### AHC038 軽量版の完成解をビームの初期候補にする

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/5cd3e84bba399d9fbccfc2c20273b840aacd02b5)。ビーム本体を同じ幅・256世代上限に揃え、軽い同時作業で生成した合法な完成解を保存候補に足す。新規dev10件2,390→2,066、未調整22件10,271→9,039（8勝14分、−12.0%）。全64出力が公式と独立検証で一致、最大1.488秒。以前の世代制限なし2.2秒版に勝ったことは示していないため、問題別でも任意。次は同時作業そのものをビームの遷移候補に入れる。

### AHC039 隣り合う列の重なりだけを要求する

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/afa01fc73184f6fe316465a5c67f924057b6f4b9)。全列に共通の行を要求せず、隣り合う列の区間が正の長さで重なればよいとした。新規未調整24件の平均3,326.54→3,403.46（20勝4分）。別系列の計算時間対照では、従来形状の探索を増やした版の平均3,323.75に対して3,429.67（20勝4分、中央値+97、時間中央値0.091874→0.085422秒）。全192出力の公式・独立検証が一致し単純多角形、最大0.170831秒・66頂点。問題別既定に採用。次は列に複数区間を持つ形を、周長上限と交差判定を保って追加する。

### AHC040 本番条件

[公式問題文](https://atcoder.jp/contests/ahc040/tasks/ahc040_a)。3秒 / 1,024 MiB、対話でT回配置し、幅＋高さ＋未配置ペナルティの最小値。本番3,000ケースの相対点。公式[ケース得点CSV](https://img.atcoder.jp/ahc_standings/ahc040/result.csv)（SHA-256 `7e3b823a5664c0df52d6f1571914303580a4d6fd6079ad01a01a3a3f644f8af5`）から、1位2,981,293,882,216（公式結果JSONと首位一致）を再現。棚状の配置を基準に、方向を混ぜる配置を同じ試行回数で比較中。

### AHC040 配置方向と計測用試行

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/c22499e74fe82e2fb3069820be96bd5ebc00b7bc)。試行数Tを揃えた棚配置と方向混合配置を比較。未調整32件の平均費用1,123,661.47→1,118,625.09、10勝8分14敗・費用比中央値1.0。128出力公式・独立一致、最大0.312888秒。既定は棚。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/fcf1bbf603140f033820232afe5fbc5dc7c01e05)は別のケースで、同じ幅計測を「無視・ベイズ更新」で比べ、計測なしも含めてTを揃えた。未調整32件の平均費用は計測なし1,102,156.41、無視1,104,546.00、更新1,105,206.84。更新は無視に12勝11分9敗だが平均は悪化。192対話で公式と独立が一致、最大0.086101秒。次は計測専用の試行を使わず、完全配置で安定な接触経路から寸法を更新する。本番未測定。

### AHC041 深さを制約した部分木の移動

[公式問題文](https://atcoder.jp/contests/ahc041/tasks/ahc041_a)。2秒 / 1,024 MiB、深さ10以下、頂点の美しさ×（深さ+1）の和+1を最大化。本番150件。[公式結果](https://atcoder.jp/contests/ahc041/results/json)の首位名はhitonanode、点数は未確認。
[初版](https://github.com/tokotoko7777/ahc-workspace/tree/6e69ff15a72cc8d3b673248c4335c435bfb344be)は同じ初期候補・140msで、DFS再始動に対し合法な部分木移動を焼きなまし。未調整24件11,534,138→11,915,002（+3.30%、全勝）。全72合法、公式と独立が一致、最大0.191秒。問題別既定に採用。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/3e6e4f9ea3ea3d9e65c0bd7e7a07e1e2fdccf908)は深すぎる枝を先に短縮してから移動する案。新規未調整24件11,911,222→11,915,249（13勝、+0.034%）。同予算、72出力が公式と一致で最大0.190秒だが、差が小さく任意。次は深さが空いている接続先候補を優先し、深い枝を含めた動きの採択効率を上げる。

### AHC042 本番条件

[公式問題文](https://atcoder.jp/contests/ahc042/tasks/ahc042_a)。2秒、本番150件の絶対点、最大1,600操作。1位468,807は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc042.zip)で確認（SHA-256 `cff8191b01d93c1790597f50ed8359c15ad172b52d82253b74d857b319d6f6ae`）。毎回完全に戻す合法な手順を基準に、途中まで・または戻さない候補をビームに追加する。

### AHC042 行列移動の部分復元

[初回](https://github.com/tokotoko7777/ahc-workspace/tree/e8796e286c2e197c2f1f5324755e4fb58da8f3f5)。全て戻す手順に対し、一部だけ／戻さない合法状態をビームで評価。dev32件281.125→111.469手/件、未調整32件282.875→111.125手/件、全64件で改善。全128出力で福を失わず鬼を全て除去、公式・独立検証が一致、最大0.188275秒。問題別既定に採用。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/98a9e65e998f57a10a8f0b3f2cf2514a0b73b4cf)。垂直方向へ小さな事前移動を許す。別の未調整32件の平均は幅22の直接108.219、幅30の直接106.594、幅22の事前移動105.156手。幅30に17勝3分12敗・中央値1手改善。全192出力公式・独立一致、最大0.223740秒。事前移動版は任意。次は戻し量と後続の複数回排出のまとまりを直接比較する。

### AHC043 資金を回収できる駅を増やす

[公式問題文](https://atcoder.jp/contests/ahc043/tasks/ahc043_a)。3秒 / 1,024 MiB、800ターン。自宅・職場の近くが同じ路線につながる利用者から収入を得て、最終資金を最大化。[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc043/result.csv)（SHA-256 `37d47e533839a2fe41dfc382a476617640aeb994dfb861539adfad7121637be2`）1,422人×2,000件から首位1,971,352,183,368を再現。上位3人が公式結果JSONと一致、公式seed列照合済み。
[初回](https://github.com/tokotoko7777/ahc-workspace/tree/2920bc508229a62f10b0070b68f3f1941fae75a4)。同じ初期の2駅に対し、追加コスト・投資回収を評価して駅やレールの改修を行う。dev12件 最終資金1,170,308→38,157,357、未調整24件2,236,615→63,926,853（全勝、単純な一路線の対照）。全72出力公式と独立検証が一致、最大0.058秒。問題別既定に採用。次は初期の路線を今後の展開も含めて比較する。本番未測定。

### AHC044 本番条件

[公式問題文](https://atcoder.jp/contests/ahc044/tasks/ahc044_a)。2秒、本番150件。50万回の担当割当で目標回数とのL1誤差を減らし、各ケース100万−誤差の合計を最大化。1位149,807,848は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc044.zip)で確認（SHA-256 `52b6cb057545c468eb48910bf634170e1a89735bb3505b39cf4e1607c2e67135`）。同一ケースは未確認。連結な交互割当の基準に対して、残差が小さくなる複数辺変更を試す。

### AHC043 最初の2駅と敷設経路の候補を増やす

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/46cbc1ff5548f82b99e16300ca1a5c3ca51984ca)。8回の建設シミュレーションを揃える。新規未調整24件の最終資金は49,972,611→52,340,760（+4.74%、12勝4敗8分）。追加の独立20件では+1.56%だが7勝9敗4分、最大の勝ちを除くと悪化したため、既定は変更しない。112出力の公式点と独立再生が一致、最大約0.310秒。この版は本番を測らない。次は最初の路線の候補に対し、初期の1〜2回の増設を実際に評価する。

### AHC044 50万ステップ後の割当誤差を減らす

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/95185faa94950764129d78ae2422bd9b46da73e4)は連結な割当先の不足を辺変更で改善し、50万手を再生して良い解だけを採用。未調整24件平均誤差55,037.42→41,167.08（9勝15分）、96出力が公式・独立一致、最大0.160253秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/39c6fccc3544603af106c420abf983f84ded74b1)。同じ初期解・各224回の50万手評価で、局所残差の対照と定常分布の感度で候補を選ぶ方式を比較。新規未調整24件の平均誤差は初期約43,738、残差約30,581、感度約19,314（対残差23勝1分、中央値8,162改善）。144出力で公式と独立検証が一致、最大0.679490秒、問題別既定に採用。次は全頂点に到達できる制約を残しながら、固定していた巡回辺も再設計する。

### AHC045 本番条件

[公式問題文](https://atcoder.jp/contests/ahc045/tasks/ahc045_a)。2秒 / 1,024 MiB、800都市を指定サイズで分け、各グループを木でつなぐ。最大400回・各2〜L頂点のMST比較を使え、実距離の切り捨て和を最小化。本番3,000件の相対方式。最初はグループと質問を揃え、既知のMST辺への割引と、非木辺／木上経路の不等式を利用する案を比較する。

### AHC045 MSTの候補辺の順序を推定する

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/ef908953cb79d12f7f5f71ef4aaef53d45fc4c04)。グループと質問を完全に揃え、MST見積もりを比較。dev14件の道路長4,360,242→3,625,385（13勝）、未調整28件8,123,736→6,910,990（−14.93%、26勝2分）。各側の未調整質問は計7,399回、84対話の公式・独立採点が一致、テスタ込み最大0.144秒。問題別既定に採用。[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc045/result.csv)（SHA-256 `126880ede7e13358d79d5217c5c0fbcbd0d069bed868af1df62a3a93c38dcaaf`）の1位2,988,855,588,054は公式結果の上位順とも一致。初版の凍結本番比較を開始した。

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/f594e37f013614a169ff675440bfcfec1e63d219)は同じ問い合わせの後、グループサイズを固定した都市交換を追加。新規未調整28件7,136,780→7,224,355（16勝11敗1分だが合計悪化）。質問は各側7,795回で同一、84出力の公式点が一致、最大0.211秒。任意、既定変更なし。次は交換の価値が大きそうな境界都市を特定して比較質問の割当から変える。

### AHC046 滑走の停止点を後の移動にも使う

[公式問題文](https://atcoder.jp/contests/ahc046/tasks/ahc046_a)。2秒 / 1,024 MiB、指定順39地点を訪問し、1,600操作以内。全完了なら1,640−操作数。本番150件の絶対点1位220,494は[AtCoder提供の順位・得点データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc046.zip)（SHA-256 `fdca36f73112443dd16d49be36e7c7266ad7f6d7d0f1ef07d3ce0538f7dc100a`）。同じ本番入力は未確認。

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/00edc1ce4028926dcc2cfb8316e64994f3fe89bb)では滑走のためのブロックを後にも再利用して、未調整30件277.400→238.100手（全勝）。120出力公式と独立一致、最大0.059997秒。[追試](https://github.com/tokotoko7777/ahc-workspace/tree/8c258bffae710ee7b3764fa8f0da9e95d59a4f6d)は同じ初期解を使い、配置状態を8本保持して進める。別の未調整30件は初期239.400、8再始動227.933、状態を保持する方式217.600（対8再始動29勝1分、中央値10.5手改善、時間中央値0.120→0.132秒）。180出力すべて全39地点・公式と独立一致、最大0.371476秒。問題別既定に採用。次は途中の移動線を横切る位置にも停止ブロック候補を作る。

### AHC047 本番条件

[公式問題文](https://atcoder.jp/contests/ahc047/tasks/ahc047_a)。2秒 / 1,024 MiB。12状態の出力分布を提案し、長さ100万の文字列で各パターンが1回以上出る確率の加重和を丸める、150件合計。[公式結果JSON](https://atcoder.jp/contests/ahc047/results/json)の首位はitigo、得点未確認。決定的な循環と、確率遷移を変えて正確な出現確率で判定する案を比較中。

### AHC045 凍結版の本番比較

[再現手順](https://github.com/tokotoko7777/ahc-workspace/tree/8d199405dc9bbdc8821997230ee282b8b4af168a/problems/ahc045)。グループ交換でなく、MSTの順序推定を採用した初版を凍結。公開公式seedのまま生成した3,000件をアーカイブのfile・seed・N・M・Q・L・Wと照合し、全件合法・公式と独立採点一致。道路長の合計764,179,938、質問880,324回（ケース最大400）、テスタ込み最大約0.242秒。候補 **1,809,922,568,865**、公式1位 **2,988,855,588,054**。ケース別最小値を更新した例は0なので、同じ基準で仮に追加すると1,300人中402位相当、同点なし。**未達**・main未マージ。本番評価は改善のフィードバックには使わない。

### AHC047 確率的な遷移と厳密な出現確率

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/e8aa0d41b813933b676a6b0cb442f1e51e7a92bc)。価値の高い文字列を確実に繰り返す決定的循環に対して、確率遷移を複数初期化し、厳密なパターン出現確率で採否を決める。dev24件平均13,671.67→23,499.75、未調整24件13,864.71→21,656.67（全48件改善、未調整中央値+5,523）。96出力の公式点と独立の行列計算が一致、最大0.097127秒。問題別既定に採用。次は複数の価値ある文字列が途中の遷移を共有できる初期グラフを、追加再始動を使う対照と比較する。

### AHC048 構成から着手

[公式問題文](https://atcoder.jp/contests/ahc048/tasks/ahc048_a)。3秒、指定色1,000件、出力上限はケースにより4,000〜64,000操作。顔料の使用費と、提出色のCMYのユークリッド距離誤差を最小にする。2マスの井戸を固定し、毎回独立に色を作る基準と、前に残った色を使いながらチューブから足す案を比較する。

### AHC047 文字列の経路を共有して初期化する

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/62f3f7888cb37d53f9c45bc289e04078ccf857d2)。同じ4初期解の後に、ランダムな初期解2つと、重なる文字列経路を一つの遷移グラフへ埋める初期解2つを比較。各4,500変更に固定。新規未調整24件の平均は追加なし21,549.25、ランダム22,646.17、経路共有26,281.33（対ランダム15勝6分3敗、中央値+1,131.5）。時間中央値0.088→0.087秒、最大0.114058秒。144出力公式と独立検証が一致。専用既定に採用。本番得点は未確認。次は共有パスの一部を状態の付け替えで組み換え、複数文字列の確率を同時に伸ばす。

### AHC048 余った色を次の混色に利用

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/ad4f7e6bcdfa76f80a9b92052ea060f72085dc06)。同じ2マス容量の井戸と色選択を使い、残った1gに新しい色を足せる案を追加した。dev14件15,520,443→13,428,201（13勝）、未調整28件26,983,603→22,488,549（−16.66%、26勝2分）。全84出力が公式・独立・C++内得点と一致、最大0.008秒、最大2,016操作、C++23単一ファイルでも一致。問題別既定は合法な2案の良い方。[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc048/result.csv)（SHA-256 `94707ef0dc8cdf6d2fac510234b75854a6f5b1aff78687d4210d00d9a23ab715`）1,205人×2,000件から、公式結果と整合する1位1,129,730,769,846を確認。次はより容量の大きい井戸で将来の色を見越し、凍結後に本番同一評価ができるか検証する。

### AHC048 井戸の容量を上げる追試と本番比較

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/2264494b2ec7fa3fa9cc1161c9cc7d3891649bcb)。容量2の井戸を使う既存案に、容量3で余りを持ち越す案を追加。未調整28件で費用22,479,484→15,417,918（−31.41%、25勝1敗2分）、合法な2案の良い方なら15,417,227。最大0.027秒、全84件の公式・独立と単一ファイルの得点一致。問題別既定に採用。

[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/bd0d1fd309af7f9ba834e6688b8391a6afa2c3af/problems/ahc048)。容量を選ぶ版を凍結し、公式seedの全2,000件でfile・seed・K・T・Dを照合。全件合法で公式・独立・C++内の評価一致。絶対費用合計1,213,262,536、最大2,237操作・0.083秒。相対点は候補 **121,378,823,586**、首位 **1,129,730,769,846**。ケース最小更新0のため同じ基準で集計でき、1,206人中355位相当、同点なし。**未達**、main未マージ。次は複数のチューブの配合を操作数・材料費と合わせて事前評価する。

### AHC049 箱の耐久余裕を使った運搬経路探索

[公式問題文](https://atcoder.jp/contests/ahc049/tasks/ahc049_a)。2秒 / 1,024 MiB、操作は16,000以下で耐久値を正に保つ。全搬出なら16,400−移動数の絶対点、本番150件。公式結果JSONの首位はRafbill、得点と同一ケースは未確認。
[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/a4688d135b0ecabf49fe877895782ced3e0aa5ca)。最短方向だけの積み方に対し、箱の追加と帰路の耐久性を毎回厳密にチェックして、少し離れた箱も幅14のビームで検討。dev28件6,426.14→5,077.00移動/件、未調整28件6,482.50→5,167.64（全56件改善、未調整中央値1,357移動短縮）。全112出力で公式・独立が一致し完了、最低耐久1・最大7,319操作・0.131841秒。問題別既定に採用。次は運搬の土台に選ぶ箱の候補を同予算で増やす。

### AHC049 最下段の箱を複数候補から選ぶ

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/508d161add61b6185f17ab86fd2ce819d164009c)。各深さの幅14を1種類の土台箱に使う対照と、距離が異なる2種類に7ずつ使う案を比較。新規dev・未調整各28件すべて改善。未調整平均5,126.57→4,861.64移動、中央値243移動短縮、実際の評価回数も211,110→173,933。112出力で公式と独立が一致、最大0.099297秒。問題別既定に採用。次は2回の運搬の間で箱を入れ替え、両方の耐久余裕を厳密に確認する。

### AHC050 岩を置いた後の生存率を3手先まで評価

[公式問題文](https://atcoder.jp/contests/ahc050/tasks/ahc050_a)。40×40、2秒 / 1,024 MiB。方向を等確率で選び滑走し、岩に当たったときの脱落を含む期待生存長を最大化。絶対点150件合計、公式結果JSONの首位yosupoの値と同一ケースは未確認。[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/c1084c6814a6b1a98fa2b6de854a1a2941f09098)。直近の脱落確率が最低になる岩の順序に対し、少数候補を3回分シミュレート。dev10件9,156,386→9,704,786、未調整20件18,411,827→19,455,464（全勝、+5.67%）。全60出力の公式・独立・C++内評価が一致、最大0.220秒未満。専用既定採用。次はすでに固定した岩の順序を一部交換し、後半の期待値がどこまで変わるかを逆向きに計算する。

### AHC051 分岐の真の確率で選別機を選ぶ

[公式問題文](https://atcoder.jp/contests/ahc051/tasks/ahc051_a)。交差しない分岐を配置し、正しい処理装置に着く確率を最大化。公式[ケース点CSV](https://img.atcoder.jp/ahc_standings/ahc051/result.csv)（SHA-256 `7c1b931d4c62deb3fd63d1a3401e398e9ecba0bffdc2d3439d6f58b2bf0ec838`）1,183人×2,000件の最小値相対点で、1位1,721,504,233,142を再現し公式結果JSONとも一致。[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/c7a55c1984e3da4992f23c94bdb1a5430db232c4)。同じ5つの合法形状を使い、平均属性で選別機を割り当てる基準と、到着確率・後段の価値で選別機を選ぶ案を比較。dev28件635,654,296→609,971,541、未調整28件675,522,621→640,624,514（27勝1分、費用比中央値0.94965）。112出力が合法で公式・独立厳密計算が一致、最大0.103215秒。問題別既定に採用。次は一部の経路を再合流させる構造も検証する。本番未測定。

### AHC050 隣接交換を、その後の生存分布まで見て選ぶ

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/369761882afe4a63f254cb9752a8563ce55f1697)。同じ初期列と隣接ペア、各5交互スイープで、2手だけの生存率と残りの全手の確率を逆向きに計算する方式を比較。新規dev10件9,760,937→9,792,548、未調整20件19,401,191→19,486,823（全件改善、+0.44%）。60出力公式と独立が一致、最大0.437秒。問題別既定に採用。次は離れた操作間の短区間も対象にする。

### AHC051 平面で再合流を許す追試と本番比較

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/15a685950c1423fb5e0dbeb864561b0f36e2e896)。合法な分岐木から、一部の出力先で経路を合流できる構造を追加した。同じ予算を木の再始動に使う案に対し、新規未調整28件の絶対費用は初期651,882,452、再始動635,754,453、再合流590,565,469（再始動に全28勝、差の中央値38,908,754）。全168出力で公式と独立の幾何・厳密確率が一致、中央値の時間も0.03918→0.02371秒。問題別既定に採用。

[公式比較](https://github.com/tokotoko7777/ahc-workspace/blob/f5ac7efb491be927e38eaae17aeb4620a3c8a964/problems/ahc051/OFFICIAL_AUDIT.md)。再合流版を凍結し、公開seedで本番2,000件を生成してfile・seed・N・M・Kを照合。全件合法・公式と独立採点一致（全処理機に到達するのは1,997件、未到達を含む残りも合法）。最大0.242449秒。相対点は **310,855,698,519**、公式首位 **1,721,504,233,142**。最小値更新0で同じ基準、仮に追加すると1,184人中208位相当、同点なし。**未達**、main未マージ。次は機械種類の局所変更より先に、合流点と出力深さをまとめて構成する。

### AHC052 ロボットごとに独立した操作を使う

[公式問題文](https://atcoder.jp/contests/ahc052/tasks/ahc052_a)。30×30、10台・10ボタン、1,800ターン以下。全塗布なら2,700−ターン数。本番150件、[公式結果JSON](https://atcoder.jp/contests/ahc052/results/json)の首位はnikaj、得点未確認。[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/528b3ca276d534fb1ca3ad330d0ab12bb9e8e565)。全員同じ方向に動く対照に対し、4つの共通ボタンは残し6個の独立ボタンを使う。dev12件27,445→28,931、未調整24件54,986→57,831（全件改善、ターン合計9,814→6,969）。全72出力が合法に全塗布、公式と独立が一致、最大0.044秒未満。完走した2案の良い方を専用既定に採用。次は未塗布の箇所を見てボタンの動作を付け替える。

### AHC053 固定デッキ上の一括割当

[公式問題文](https://atcoder.jp/contests/ahc053/tasks/ahc053_a)。要求値を知る前にカードを決める対話式、本番150件合計。[公式結果JSON](https://atcoder.jp/contests/ahc053/results/json)の首位はyosupo、得点は未確認。各ケース全要求を正確に満たせば150件の理論上限は150,000,000,000点だが、首位が達した証拠はない。[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/7f0257819434e8b706195937b14ee7f0b482b3ae)。同じ7段階デッキから一番近いカードを使う方法と、順序を保ち最大誤差を先に最小化する割当を比較。dev32件383,627,530→387,598,137、未調整32件383,046,985→386,994,135（18勝14敗、中央値+225,134）。全128出力公式・独立一致、最大約0.02205秒。既定を変更しない。次は2枚の組合せで要求値と残差を別々に表現できるデッキを、要求を見ずに設計する。

### AHC052 未処理の場所を見てボタンを変更する追試

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/227a790ee8ddad8e2cd9f7af15cdea5fe6fc0612)。既存解から追加の試行を8回ずつ行い、ランダム再始動と、後半の未処理マスを手掛かりにボタンを変更する方式を比較。dev14件は合計同点、新規未調整28件67,843→67,704（9勝16敗3分）。全84出力は合法・完了、公式と独立一致、最大0.126秒未満。既定は変更せず、次は地域ごとの軌道を組んで同時操作の似た方向をグループ化する。

### AHC053 残差を2枚組で作る固定デッキ

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/2f27c9a55ccab9756b345063427fc7d0899fa92b)。要求入力を読む前に全ケース共通のデッキを固定。基準値用80枚と、細かさの異なる140枚ずつ3組で残差を表す。新規5系列×20件の全100件で旧2案を上回った。独立未調整3組の平均得点はそれぞれ約595.09M、595.64M、596.34M（旧一括割当384.996M、385.560M、386.110M）。全300対話公式と独立検証が一致し、単一ファイルで再現、候補最大約0.00512秒。問題別既定に採用。公式150件での比較は未判定。次は残差割当の競合を、固定デッキ内で連鎖交換する。

### AHC054 通り道を長い誘導路にする

[検証と実装](https://github.com/tokotoko7777/ahc-workspace/tree/cc2c7b4e86abc712f7ce27bf8a992175adb45ae1)。木を置かない対照に対し、道が途中のどの地点からも近道にならないよう、通行可能性を毎回確認して長い経路を作る。dev24件の平均移動72.292→489.042、未調整24件56.042→404.375（いずれも全勝）。3方式・全144対話で公式と独立の観測再生が一致、最大0.104598秒。問題別既定に採用。次は本道の近道にならない枝を足し、目標の見え方も変える。
[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc054/result.csv)（SHA-256 `50617ebf3ae1d3a70dd27b4c712e61d72543b6a58ba7816e2fee6039dade0293`）1,190人×3,000件で首位2,409,003,089,874（saharan）。比較用の公式seedを確認し、初回は本番未測定。

### AHC055 構成から着手

[公式問題文](https://atcoder.jp/contests/ahc055/tasks/ahc055_a)。200箱で順に使用可能になる道具を使い、開封にかかる手数を最小化する。2秒、本番150件の絶対点。次の箱を安く開ける基準と、順序を固定して先に入手できる道具の使用先を全体で振り分ける方法を比較する。

### AHC034 固定移動費と積載費の下限診断

[証明・再現スクリプト](https://github.com/tokotoko7777/ahc-workspace/tree/c53951ab42ee40ce6a1347f820007bee1729502e/problems/ahc034)。初期高さの絶対和、格子の偶奇とManhattan MSTによる訪問手数下限、行・列の切断を横切る土量下限を分けた。未整地の得点についても、仮想の残作業の費用が公式のペナルティ以下なので同じ上限を適用できる。公式ジェネレータの新規20件で固定経路＋流量版を測り、全件で公式・独立採点が一致。得点平均33,596,853.25、緩い上限89,685,469.85。下限までの費用差は固定移動費115,660、積載移動費14,475.70、積み降ろし費0。積載だけで得られる改善は限られるため、次は再配分を保って経路を短くする。ただし移動下限は緩く、差の全額を削れるとは限らない。本番順位は判定しない。

### AHC054 行き止まりを足す追試と本番比較

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/5af1506cdf04eb57d5b756440c63cd02fadc44d6)。同じ本道から、本道の最短長を変えず一か所だけで接続する行き止まりを追加。新規未調整24件の平均は初期423.000、再始動増量429.833、行き止まり付き1,047.333（23勝1分、時間中央値0.053301→0.034253秒）。144出力公式と独立一致、最大0.230074秒。問題別既定に採用。

[公式比較](https://github.com/tokotoko7777/ahc-workspace/blob/f1e7725908d680be15c2dafaa5117f8eed29a688/problems/ahc054/OFFICIAL_AUDIT.md)。行き止まり版を凍結し、本番公開seedで全3,000件を生成してfile・seed・N・木の数Kを照合。全件合法で公式と独立再生一致、合計3,042,194移動、テスタ込み最大0.196776秒。相対点は候補 **419,674,926,324**、首位 **2,409,003,089,874**、ケース別最高更新0のため同じ基準で1,191人中505位相当。同点なし・**未達**。次は相手の視界が変わった時点で、接続を壊さず動的に木を置く案を試す。

### AHC055 道具の使用先と開封順

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/56c542146db54bea2ac321420789e560c2706453)は開封順を変えず、獲得済み道具の使用先を再配分。未調整24件の手数256,406→120,975（全勝、−52.82%）、得点1,177,595→1,313,026。全72出力公式と独立で一致、最大0.040秒未満。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/a41469d083df1817c2ff68de449305ff12ee9ba2)は隣の箱と順序を入れ替え、まだ使えない道具の割当だけ修復する。新規未調整28件は同じ20万追加提案の対照に対し157,961→137,988手（全勝、−12.64%）、得点1,539,919→1,559,892。全84出力が公式・独立一致、再計測最大0.0558秒、問題別既定に採用。[公式結果JSON](https://atcoder.jp/contests/ahc055/results/json)の首位はnikaj、点数は未確認。次は順序変更と割当を同時に最適化する。

### AHC056・057 着手

AHC056は[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc056/result.csv)（SHA-256 `1d27cbb3f26e08ec3ec8f81c2c63dae7bc7ff0a2b66945d3412591a3185c0116`）1,105人×2,000件で、最大値相対点の1位1,899,994,171,242（saharan）を再現。固定経路の各セルの行動を、時刻や色・状態を組み合わせてより少ない規則で表せるか比較する。
[AHC057](https://atcoder.jp/contests/ahc057/tasks/ahc057_a)は2秒、本番150件の絶対点合計。首位名yochanで点数は未確認。同じ点群分割を使い、1つの時点でまとめて結合する方法と、トーラス上で点群が近づく時刻を選んで順番に結合する方法を比べる。

### AHC056 同じ移動をより少ない状態・色で表す

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/76777c4e88aac27d028dfa893c47db2a540e4f05)。同じ最短移動列を固定し、事前に組んだ時刻の切替を使う対照と、同じ動作を起こす規則を後ろから再利用する案を比較。未調整32件の費用平均80.500→73.656（全勝、中央値7削減）、128出力が公式・独立一致、最大0.455221秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/fbc469f215b805732151a93d4f8213cb3071663f)。同じ長さの経路を5本ずつ候補にし、ランダムな最短路と圧縮しやすい最短路を比較。新規未調整32件の平均は凍結72.719、ランダム72.594、圧縮71.531（対ランダム21勝10分1敗、中央値1減、時間中央値0.02462→0.02537秒）。全192出力が公式と独立で一致、最大0.4059秒。問題別既定に採用。上位との隔たりが大きいので本番未測定。次は色と状態を作った後の統合を、組み合わせて見直す。

### AHC057 結合時刻とグループの両方を選ぶ

[初回](https://github.com/tokotoko7777/ahc-workspace/tree/597f72f03de27400f38dd4cff9f5ee779055c0be)は同じグループに対し、トーラス上で近づく時刻に順次結合。未調整28件の得点130,360,962→130,699,166（10勝18分）、費用−0.83%、84出力公式・独立一致、最大0.1603秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/743f74ba0a89e40350d866c59c3bb1ce6a036bf1)はグループのサイズを保つ32回の交換で、静止時の距離を使う案と将来の最近接を使う案を比較。新規未調整28件、初版130,487,478→静止距離130,959,921（25勝3分）、将来距離130,878,447で静止距離に4勝19敗5分。静止距離だけを問題別既定、最大約0.4312秒。次は単独の近さでなく、グループ全体の結合計画を使って、交換候補に優先順位をつける。

### AHC058 既存の購入順を焦点別の初期解と比較

[公式問題文](https://atcoder.jp/contests/ahc058/tasks/ahc058_a)。本番150件、最終りんご個数のlog得点を丸めて合計。首位fishyleneの点数と同一ケースは未確認。[初回](https://github.com/tokotoko7777/ahc-workspace/tree/c75e684905c14eb10a7d59da3c9e9d81d8f9ce7a)ではリポジトリ既存の購入順探索に、1つの機械を中心とした購入順の別初期解を追加し、同じ3,400提案で比較。新規dev・未調整各24件で最終個数もすべて同点。未調整平均5,187,704.58点、全96公式・独立一致、最大0.763907秒。既存を既定。次は序盤を固定し、後半の長い区間で同じ購入の集合を並べ替える。

### AHC059 本番条件

[公式問題文](https://atcoder.jp/contests/ahc059/tasks/ahc059_a)。20×20のカード盤面、2秒、カードを全て消去すると16,400−移動数。本番150件合計。首位chokudaiの点数と同一ケースは未確認。同じ初期経路から、ペアを隣接して入れ直す対照と、スタックの合法性を保って別々の位置に再挿入する近傍を比べる。

### AHC058 購入の集合を変えずに後半の順序を変える

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/23fa6eb16cc7acbe74babdf1ff9f57b7f993d483)。既存の前半を固定し、同じ購入の集合を、340回の細かな移動・交換と340回の部分列回転・反転などで比較。新規未調整16件の平均は前半固定5,189,948、細かな変更5,197,425、部分列5,201,445（対細かな変更15勝1敗、中央値+1,842点）。96出力で公式と独立が一致、最大0.655秒。差は小さく任意。次は序盤の購入を崩さず、後半の特定の機械群をまとめて再評価する。

### AHC059 スタックの合法性を保つ入れ子と部分列移動

[初回](https://github.com/tokotoko7777/ahc-workspace/tree/4848abd431bd612b8ab15199f2f5562fe1598974)。同じ初期解と2万回の提案で、ペアを隣接させる対照と、同じスタック祖先の2位置に再挿入する案を比較。未調整28件413,250→421,321、移動45,950→37,879（−17.57%、全勝）。全84出力で公式・独立と一致、最大0.172秒未満。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/905faa4a34b05a6b0d5ded139f08fd8414e366fd)。同じ最初の2万提案から、さらに2万回を単一ペアへ使う案と、対応が閉じた部分列の移動・反転に使う案を比較。新規未調整28件422,234→425,047、移動36,966→34,153（−7.61%、全勝）。全84出力公式・独立・内部評価が一致、最大0.299秒。問題別既定に採用。本番同一比較は未判定。次は2つの部分列をまたぐ入れ替えを試す。

### AHC060・061 着手

[AHC060](https://atcoder.jp/contests/ahc060/tasks/ahc060_a)は本番150件の絶対点、1位名Sweet_Sweet_Soul、得点は未確認。同じ初期解と展開数で、ランダムな経路の候補と、同じ商品・現在地・直前位置を重複除去する候補を比べる。
[AHC061](https://atcoder.jp/contests/ahc061/tasks/ahc061_a)の[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc061/result.csv)は1,171人×3,000件、ケース別最高値による相対点で1位2,424,667,397,941（cuthbert）、上位3名の順位が公式結果JSONと一致。相手方針の推定と手の評価を揃え、占有によって到達できる場所がどう変わるかを評価に加える案を比較する。

### AHC060 配送マクロの重複除去と色変え

[初回](https://github.com/tokotoko7777/ahc-workspace/tree/204933bff927c6bac159926766888181897cc748)。同じ暖機・展開上限1,000で、ランダムな配送経路と、直前地点・現在地・出力文字列を正確に照らして重複を除く方法を比較。未調整24件882.750→894.875（19勝5敗、中央値+14）、96出力で公式と独立の全10,000操作が一致、最大0.087508秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/6e7f42ee36c537681b0db68526aba19077582ad0)。幅と展開上限を同じにし、配送の途中で色を不可逆に1回変える候補を追加。新規未調整24件は凍結888.833、同幅の色固定887.042、色変え992.333（全勝、対色固定中央値+105点）。色変え版は試行数と時間も少なく、全144出力で公式・独立一致、最大0.111667秒。専用既定に採用、本番未判定。次は今色を変えることにより後で得られる文字列の候補がどの程度残るかを評価する。

### AHC061 領土の接続・進出可能性を評価する

[検証](https://github.com/tokotoko7777/ahc-workspace/tree/41ec433123816f9d026a4b719c5aeddbec16c29c)。同じ予測48プロファイルと衝突判定を使い、局所得点の評価に占領後の到達範囲・接続・前線を追加。dev14件1,426,669→1,452,720（8勝）、未調整28件2,596,131→2,511,876（14勝14敗、−3.25%）。全84対話で100ターン合法、公式・独立・内部得点が一致。既定は予測＋局所得点のまま。次は同じ相手方針乱数で次の同時手まで見て、得点比で比べる。公式3,000件のseed列は照合済みだが未測定。

### AHC062 本番条件

[公式問題文](https://atcoder.jp/contests/ahc062/tasks/ahc062_a)。3秒、200×200のキング移動で全セル訪問、訪問順×人口を使う各ケース絶対点、本番100件。公式結果JSONの首位gazelleの点数は未確認。蛇行路の同じ初期解から、区間反転に加えて端点を変える操作を同じ提案数で試す。

### AHC061 同時手のサンプリングと本番比較

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/defe7c3d7e75e3b82da58b624c0b98906c4960dd)。同じ相手サンプルと遷移数で、1手と2手先を比較。新規未調整28件の絶対点は局所評価3,100,724、1手3,507,123、2手3,377,746。さらに独立36件で局所3,640,071→1手4,342,282（26勝10敗、+19.29%、最大の勝ち2例を外しても+519,904）。全240対話で公式と独立・内部盤面が一致、最大約0.0914秒。1手を既定、2手は任意。

[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/f119f0ff3e0268f477468ab09112488312198e7b/problems/ahc061)。この1手版の公式公開3,000件をM・U・file・seedで照合。全件100ターン合法、公式と独立採点及び状態遷移が一致、絶対点合計372,124,627、テスタ込み最大0.07654秒。当時と同じ基準で **1,207,794,230,850** 対首位 **2,424,667,397,941**。最高値更新0のため追加しても同じ点、1,172人中627位、同点なし。**未達**。次は予測相手の行動分布を校正し、領土を隔てる捕獲を重視する。

### AHC062 順序付き得点を保つ経路反転

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/81151ac7b6335424f69b3bbb9da842061e296fb8)。同じ蛇行初期解・28万提案で2-optと端点を変える操作を比較。未調整18件405,684,739→405,405,613（10勝8敗、合計悪化）。72出力の全セル訪問・得点が公式と独立で一致、最大0.445166秒。2-optを既定、端点操作は任意。次は高得点セルを後半へ集めるよう、初期路を低い人口・高い人口の2本で構成する。

### AHC062 低い人口のセルと高い人口のセルをほぼ2本に分ける

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/083860ab332b39fa278b1736e3ca1e0561dc6765)。近接する2行ごとに、各列で小さい人口を早い経路、大きい人口を遅い経路に寄せる。2×2と4×2の接続部で全セルをちょうど1回訪れる道を組み、同じ28万回の2-optを後段に使う。新規dev18件の平均405,704,533→467,302,960、未調整18件405,801,993→467,170,089（全36勝、未調整中央値+61,601,892）。全72出力で公式と独立の経路・得点一致、最大0.698秒。専用既定に採用。本番同一ケースは未確認。次は接続口を制約し、2本をより多い層に分けられるか試す。

### AHC063・064 着手

[AHC063](https://atcoder.jp/contests/ahc063/tasks/ahc063_a)の[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc063/result.csv)は1,264人×2,000件、順位相対を整数丸めで再計算して1位1,985,255,142,455（rhoo）、上位3名は公式結果JSONと同じ。公式公開seedの順と特徴量N・M・Cを照合。静的に届かない色を、体が移動して空く時刻を使った経路で取得する案を比較する。
[AHC064](https://atcoder.jp/contests/ahc064/tasks/ahc064_a)は本番150件、公式結果JSONの1位chokudaiの点数は未確認。同じ車両移動を使い、操作依存を保ったまま同時刻に詰める際、同時操作数優先と後続の長い連鎖優先を比較する。

### AHC063 尾の移動と次の色を考えた回収

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/4a7f7b14ee8c06d079c94d10d19acd9df9d0f139)。同じ12再始動で、静的には届かない色について尾が退く時刻を使う経路を検索。未調整28件の費用7,018,745→6,639,383（−5.41%、15勝9敗4分）、完全一致4→6。84出力が公式・独立・内部一致、最大0.0084秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/6e48e0c6aea92464d043b408df772be340c25470)は同じ色の複数経路の中から次の指定色を取得しやすいものを選ぶ。新規未調整28件の費用は初版9,058,369、同予算の次の餌への探索9,249,104、次色を指定8,289,603（対同予算−10.37%、16勝12敗）。168出力が公式・独立・内部一致、最大0.0594秒。専用既定に採用。

[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/f7cb5923521b00e317025fa5eb2e1e1d2622a75a/problems/ahc063)は、この既定版の本番2,000件をすべてN・M・C・file・seed照合、全件合法・公式と独立が一致。完全一致362件、最大0.110657秒。当時1,264人の分布で候補 **1,189,793,908,234**、首位 **1,985,255,142,455**。候補を追加した同じ1,265人に再集計すると候補 **1,190,434,387,385**、首位 **1,985,266,007,770**、同点なし495位。**未達**、main未マージ。次は数色にわたって体を巻き戻せる経路を設計する。

### AHC064 直列の搬出を非交差な同時操作に並べ直す

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/63185be0c9a50743d00f846f73ca5f1ffab597e7)。各線路ごとの操作順と搬送内容を厳密に保ち、同時に行える非交差な操作を選ぶ。新規未調整30件の平均ターンは直列241.933、同時数最大136.333、後続の長い操作を優先128.833（対同時数全勝、中央値7減）。180出力公式・独立一致、最大0.038199秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/ab9244e54098611bbb35a322da00472d9bb95527)。共通の詰め替え器で各32候補を比較し、新規未調整30件は初版125.800、順序ランダム複数93.833、誘導順序101.067。後者はランダムに1勝1分28敗。180出力が公式と独立に一致、最大0.02161秒。ランダム複数を問題別既定。次は操作数でなく、詰め替え後の実ターンで搬出順を更新する。同じ本番150件の証拠はなく未判定。

### AHC065 ベルトを交差させる搬出

[公式問題文](https://atcoder.jp/contests/ahc065/tasks/ahc065_a)。2秒、本番150絶対点。首位rabotの点数・同一ケースは未確認。[初版](https://github.com/tokotoko7777/ahc-workspace/tree/37f1f97176dcc631de043411873fa74d279b9a1a)は全ての縦横隣接移動を実現できる交差配置で、同じ8経路候補を次の1箱と8箱の位置で比べた。未調整28件144,695,725→149,371,055（全勝）、操作155,828→138,801。84出力公式・独立一致、最大0.046秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/f531373eb2eaaee30443e6c6df6296f5a83da40a)は同じ全搬出シミュレーション8回を現在8本／現在4本と次4本に分けた。別の未調整28件148,896,314→146,429,429（勝ちなし、操作140,427→149,277）。84出力公式・独立一致、最大約0.0166秒。既定にはせず、今の候補数を減らさず上位数案だけ先を読む。

### AHC066 マクロを操作経路に組み込む

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/b69f28ac27f1bdfc0e4cf829de16762ea408ae35)。同じマクロ候補に対し、既存の経路を置換するだけでなく、マクロを1ボタンの辺として位置・向きの経路を引き直した。未調整28件288.750→248.679ボタン（全勝、中央値−24.5）、112出力公式・独立一致、最大0.030318秒。[追試](https://github.com/tokotoko7777/ahc-workspace/tree/84edb3f6302d8c76beb38d494282f833eb06cc74)は同じ追加10候補を、経路から抽出する案と幾何的なパターンから作る案で比較。新規未調整28件226.679／221.536／212.321ボタン（18勝7分3敗、中央値3.5削減）。全168出力が公式・独立一致、最大0.070245秒。問題別既定に採用。

[公式比較](https://github.com/tokotoko7777/ahc-workspace/blob/588082c3b7a07866a338ac201e65b2ac1887bab6/problems/ahc066/OFFICIAL_AUDIT.md)。公開公式seedで2,000件を生成し、file・seed・N・M・T・Wを公式アーカイブと照合。入力に出ないWは公式ジェネレータの一時コピーでstderrに出すだけとし、生成した全入力が公式版とバイト一致したことを確認、評価は公式版で行った。全件合法で公式・独立一致、ボタン合計504,280、最大0.167271秒。候補 **1,352,700,378,901**、公式首位 **1,893,330,847,366**、ケース最小値の更新0で共通基準上1,269人中472位相当、同点なし。**未達**。次はマクロ位置と配送する順番を一緒に最適化する。

### AHC067 必ず通る辺からスイッチへ迂回させる

[公式問題文](https://atcoder.jp/contests/ahc067/tasks/ahc067_a)。2秒、本番150件。位置と扉の状態の最短距離を長くする。首位udon1206の得点は未確認。[初版](https://github.com/tokotoko7777/ahc-workspace/tree/5799ad6db455dd0d973d5fcdea14cb56a32f98d2)は同じ開閉候補の壁に対し、必ず通る橋から側道のスイッチへ行く順序を設計。未調整28件63,940,985→100,997,213（+57.95%、全勝）、最短距離合計2,792→6,834。全84出力で公式と独立の状態BFSが一致、最大0.0403秒。問題別既定に採用。本番未判定。次は同じ数の候補削除で、ただの迷路距離より扉を置いたあとの距離を優先する。

### AHC067 壁を同数だけ交換する追試

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/822e6835f0772872f57224924e02be8bace7e852)。同じ初期配置と36候補で、静的な迷路距離と扉付きの迂回を評価する方式を比較。新規未調整28件の合計は初版100,794,511、静的選択103,218,534、迂回評価103,367,464（対初版27勝1分、対静的14勝11敗3分）。全168出力公式と独立が一致、最大約0.296秒。初版からの悪化を防ぐ方式で問題別既定に採用。次はスイッチの順番と閉じる壁を一緒に変更する。

### AHC068 有効な区間を半分ずらしてカードを運ぶ

[公式問題文](https://atcoder.jp/contests/ahc068/tasks/ahc068_a)。2秒、20×20の配置を整列させる。本番150件、首位PrussianBlueの得点は未確認。[初回](https://github.com/tokotoko7777/ahc-workspace/tree/532a0cf2a83e82849153637b8279bbd0d5c08d20)。同じ確定順で隣接交換と、壁や確定済み部分を含まない直線の半分同士を交換する案を比較。未調整26件6,243.58→1,528.73操作（全勝、中央値4,842.5削減）、104出力すべて完了、公式・独立一致、最大0.046352秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/02a539aa8e143b7e5f56313c4c1f7235bab8c9be)。近い予算で同じ合法幅候補を調べ、横に広げた交換で他のカードも目標へ近づく場合だけ実行。別の未調整26件の平均は既定1,641.077、狭い対照1,626.269、拡張1,485.500（全26勝、対狭い中央値129削減）。全156出力公式・独立一致、最大約0.01554秒。問題別既定採用。本番の同一ケースは未確認。次は複数の確定順を同じ予算で比較する。

### AHC069 満室時の移動を候補にする

[公式問題文](https://atcoder.jp/contests/ahc069/tasks/ahc069_a)。[公式採点CSV](https://img.atcoder.jp/ahc_standings/ahc069/result.csv)は1,128人×2,000件、ケース別最大値で相対化。1位1,981,252,894,878（asi1024）と上位順は公式結果と一致、公式seedとR・クラスタ数・池数を照合。
[検証](https://github.com/tokotoko7777/ahc-workspace/tree/bf3cc5a9c1c2ffa113f15916340896c6366c1c43)。配置のみの基準に対し、受け入れ不能なときに限り、将来支払いを上回る移動費でない一組を移す。未調整28件1,349,350,404→1,388,914,694（+2.93%、22勝6敗）、受入17,748→17,959。84出力公式と独立・内部一致、最大0.2287秒。専用既定候補。次は希少な空き地を保持するため、到着後の余裕を値付けする。本番未測定。

### AHC070 長い移動で配置の偏りを減らす

[公式問題文](https://atcoder.jp/contests/ahc070/tasks/ahc070_a)。本番150件の絶対点、公式結果JSONの首位はeijirou、得点未確認。[検証](https://github.com/tokotoko7777/ahc-workspace/tree/3737c5016654eb51756f431f8fc90cd54ab58314)。同じ既定解から各8候補の1万ターンを評価し、短い移動と長い移動を比較。未調整24件の危険度393,179.42→352,032.04、公式点2,544,383.00→2,840,819.13（開発を含め全48勝）。全96出力公式と独立一致、最大0.062739秒。問題別既定。次は二つの配置の効果が重なる部分を二重に数えない。本番未判定。

### AHC069 受け入れ余裕と本番比較

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/c26dbd7905022272bc37b18beb04e810239b361e)。今の空き地の逼迫度に応じて受け入れを制限する案は、別の未調整28件で1,430,559,731→1,557,336,616（+8.86%、25勝3敗）、追加24件で1,273,439,860→1,335,099,488（+4.84%、17勝6敗1分）。将来の空きを先取りする案は未調整28件で既定候補より悪く不採用。全216出力で公式・独立が一致、最大約0.3452秒。現在の逼迫度だけを既定。

[公式比較](https://github.com/tokotoko7777/ahc-workspace/tree/6bd633eade8df2dd1c67602130cc1aa76d52141a/problems/ahc069)。公開seedで全2,000件を再現し、R・クラスタ数・池数と入力ハッシュを公式アーカイブに照合。全件合法で公式点と独立の整数採点が一致。凍結C++の費用表示のみ、浮動小数丸めで5件が最大1ずれたが、ソルバーを変更せず公式点で比較。テスタ込み最大0.450324秒。相対候補 **1,336,637,386,355**、首位 **1,981,252,894,878**。ケース別最大更新なし、共通基準で1,129人中612位、同点なし。**未達**。次は空き枠を保存するだけでなく、頻出形状が入る小区画を優先する。

### AHC070 2箇所の被覆の重複を除く

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/2f7c8b2c237219e2af6660023b37a296c0ed8fcd)。同じ8試行と共通既定の下、2手目の限界効果から既に1手目で改善した分を除いた。別の未調整24件の危険度351,640.50→351,747.54（12勝12敗、中央値では72減）。96出力公式と独立が一致、最大0.062479秒。安定しないので任意。次は評価の係数でなく区間の始点・終点を固定した再計画。

### AHC071・072・073

[AHC071](https://atcoder.jp/contests/ahc071/tasks/ahc071_a)は本番150件で、首位rhooの値は未確認。1行ずつ組む基準に対し、上のレンガの中央支持を守って2行を同時に最適化する案を比較。
[AHC072・初版](https://github.com/tokotoko7777/ahc-workspace/tree/36a890d2c5c19a137f67de5233c58f245d1e01e5)。最遠から1匹ずつ運ぶのと、同じ色を最大7匹まで拾って一緒に運ぶ案を比較。新規dev30件855.9→433.0手、未調整30件639.93→345.67手（全60勝）。全120出力公式・独立一致、最大0.055364秒、問題別既定。[追試](https://github.com/tokotoko7777/ahc-workspace/tree/23a0895828bc6d484120851c8669595e5cb4c31e)では同じ追加8候補で取り出し順を狙って変える案とランダム再始動を比較、新規未調整30件は既定433.833・ランダム429.800・狙った順432.533。ランダムに1勝16分13敗、全180出力公式・独立一致、最大0.086292秒、任意。次は複数色の集合を支持を壊さず順に運ぶ。公式採点CSVから首位1,995,670,086,683（mech_39）を復元したが、本番未測定。
[AHC073](https://atcoder.jp/contests/ahc073/tasks/ahc073_a)は本番150件、首位hitonanodeの点数は未確認。着地点を合法に絞り、同数の貪欲の別初期解と円の状態を複数残すビームを比べる。

### AHC060 同じ歩行で色変えの時刻だけずらす

[後処理と再現手順](https://github.com/tokotoko7777/ahc-workspace/tree/b0a4bc8d0ba51540d7f457511cb406a67f8e423b/problems/ahc060)。同じ歩行と反転対象のまま、木を訪れた直後のどこへ反転を置くかを試す。候補時刻を一つ進めると、納品文字列の一文字だけが変わるため、店ごとの文字列出現数で重複による失点も正確に数える。新規dev6件6,005→6,012、未調整12件11,554→11,572（9勝3分）。全18件で移動列、反転対象、操作数不変、前後計36出力は公式と独立一致。長い文字列や未納品末尾を含め2,000件の合法な乱択歩行で愚直再生と照合、後処理最大約19ms。既定ソルバーとは別の任意ツール。本番未判定。

### AHC071 支持を保つ2行の再配置

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/ee229ea027b79e5e668683714e494d860d998f02)。1行ずつ置く方法との差を、2行で共有する支持・レンガ長を厳密に評価して選ぶ。未調整28件279,034→290,702（27勝1分）、費用56,994→45,326（−20.47%）。84出力公式・独立・内部一致、最大0.00842秒。
[追試](https://github.com/tokotoko7777/ahc-workspace/tree/58746cd7d8ae2eefb5147984a11c211581cd72cd)では上下の支持を固定した2行窓を156回ずつ再計画。新規未調整28件290,003→291,785、費用46,025→44,243（−3.87%、全勝）、交互順との比較はほぼ同点。全168出力公式と独立一致、最大0.04982秒。単純な順序を問題別既定に採用。本番1位は点数未確認。次は局所費用だけでなく上段の支持位置の選択肢を増やす。

### AHC073 反射円の異なる配置をビームで残す

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/614cf90a321ba056b75894fdb9d00da56428a94c)。同じ合法な跳躍候補と初期解を使い、24種類の貪欲解と幅24の円配置を区別したビームを比較。未調整28件11,592.54→11,155.00操作（22勝1分5敗、中央値340.5削減）、中央値の時間は0.00278→0.00288秒。全112出力で公式・独立の到達と操作検証が一致、最大0.005064秒。問題別既定に採用、本番比較は未判定。次は一回の跳躍だけでなく二回の反射で達成できる移動を試す。

### AHC073 2回の反射を許す追試

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/3468c6eddc918ad794084cb891015db9e20524f7)。試行数が近い新規未調整28件で、初回11,213.857、単発の探索増量11,172.000、2回反射10,966.607操作。単発との比較は13勝7分8敗、中央値は同点であり任意。時間中央値0.00417→0.00421秒、計336出力は公式・独立検証が一致。本番証拠なし。次は使った円を重複度で管理し、複数目標をまたぐ変更を試す。

### AHC074 右端を固定するバーコードの紙で区間を切り替える

[初版](https://github.com/tokotoko7777/ahc-workspace/tree/b2c94ae0786e0805d6f5e63f3fe62c25ac404ca1)。単一窓を切り替える合法な基準と、右端の窓を残しながら複数窓をまとめて切り替える紙を比較。各注文の覆う区間はDPで選び、次の注文のために全操作を戻せる。dev14件459,389→360,721操作、未調整28件918,184→724,334（−21.11%、全件改善）。全84対話で誤注文0、公式・独立・内部採点が一致、最大0.0914秒、C++23単一ファイルの出力も一致。問題別既定に採用。本番公開2,000seedのハッシュは照合したが、部門別の分布・首位点は未取得なので未判定。次は単一の紙の価値でなく、候補の紙を取り替えたときカタログ全体の注文の総操作数がどれだけ減るかで選ぶ。

### AHC074 実際にカタログを注文して紙の候補を選ぶ

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/96edf8020f4615c7d24c1bff857f1369614f6260)。開示済み256品目の並べ直しに対し、同じ64の置換候補を単独の一致区間と実際の操作総数で比較。dev14件360,647→359,790（8勝6敗）、未調整28件721,543→722,521（12勝16敗、合計悪化）。全168対話が誤注文0で公式・独立・内部一致、最大約0.2653秒。既定は変更しない。次は単一のカタログ順を使わず、複数の順序で同じ紙を評価する。

### AHC001 2巡目：異なる軸に分離する近傍

[反対軸を強制する追試](https://github.com/tokotoko7777/ahc-workspace/tree/b1d242906edb29702c985520de3a7ab3eecb6ff8)。同じシード、4.8秒の基準で、同じ近傍内の2長方形を異なる軸に分離させた。新規dev8件で7勝1敗・平均+667,237点だが、未調整8件で2勝6敗・平均−459,751点。423試行中154は設定できたものの、他を再配置して採用した例は0。全32出力公式・独立一致、最大約4.822秒。既定維持。
[同じ配置からの局所修復](https://github.com/tokotoko7777/ahc-workspace/tree/8e7640ec881c5e8577266c47f9e1d9a1ecd73099)は軸維持・反対軸それぞれ1,728形状、合法937／538でも改善0。新規dev6件・未調整6件を全36出力で公式・独立照合。終盤の配置を凍結し、近傍数個だけを切って戻す方法では範囲が狭い。次は序盤の空間に余裕がある時に競合する通路へ異なる軸の初期配置を与え、同じ4.8秒内で比べる。本番1,000件の凍結点は更新していない。

### AHC001 序盤の分離軸を先に決める案

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/1c5d828ca56079c0ac6e7b4b364f3dffb209a5f3)。一部の競合ペアで異なる軸を先に作り、残りを同じ順で配置。新規dev8・未調整12の20盤面で、124組中111組は両軸が実現可能だったが、変更した初期盤が通常版より良い例は0。全40出力は公式・独立一致し最大4.819233秒。未調整の最終差は平均−229,692.5点だが、初期変更を一度も採用していないため軸変更の効果ではない。既定維持。次は外側を固定し、少数の長方形候補を全体得点で同時に選ぶ。

### AHC003 2巡目：将来の経路差と観測の丸め

[将来の経路差を使う追試](https://github.com/tokotoko7777/ahc-workspace/tree/a52197f492be92d3c05990606bedf1bf0cd9f171)。同じ観測候補数で、現在の2経路だけ／過去に受けた依頼の別経路差も使う方式を比較。dev20件は既定比+0.0582%だが、未調整32件で既定31,273,998,616→31,270,797,554（−0.0102%）。全156対話公式・独立一致、試験版最大0.700秒。既定維持。

[丸め尤度](https://github.com/tokotoko7777/ahc-workspace/tree/60f60000cefda6f413c1e1fb223eac5f80b8b0a3)。測定値が一様倍率で変わり丸められる確率を使って、経路長の平均・分散を更新した。凍結既定に対しdev20件+0.1160%、未調整32件+0.0766%だが、同じ凍結版を追加確認した未調整24件で−0.0328%（11勝13敗）。未調整56件合計で約+0.0297%、本番の差0.0337%と事前基準に届かず既定維持。全152対話公式と独立に一致、候補最大1.036秒。2案とも本番を再実行していない。次は固定してしまったモデルを観測で切り替えられる案を試す。

### AHC001・003 近い本番記録への追加検証

- [AHC001 外側固定の集合探索](https://github.com/tokotoko7777/ahc-workspace/tree/e214648402a6a597240128a647b21384a3158851)。同じ凍結済みv64開始盤面から、外側を動かさず最大6長方形を切り替える候補を作り、局所探索と厳密分枝限定で比較。48窓すべて探索打切りなし、16件すべて局所探索と厳密法が同点。未調整10件は1件のみ+142点、残り9件とdev6件は既存と同点。全48出力の公式・独立一致、厳密修復最大0.007402秒。分枝限定より候補集合が改善の律速。既定はそのまま、本番の差は1件平均約411,789点で未達。次に戻る際は候補矩形の切り出しを機械的な少数形状から変える。
- [AHC003 モデルの再切替](https://github.com/tokotoko7777/ahc-workspace/tree/33977359f2b8a148cb1b1079ab362dea43317ca9)。同じ更新回数で、固定後に実測経路から両モデルの予測尤度を比較し、優勢側を再決定。新規dev20・未調整32の全52件で、1,000経路すべてが既定と一致。固定後の38,014チェックで切替0。104対話公式・独立一致、テスタ込み最大約0.885秒。既定そのまま、本番未再測定。次に戻る際は固定前の予測校正、または同じ経路長の異なる辺の組合せで見積もりを改善する。

差の小さい2回でも、検証済みの局所変更は本番で試せるほどの安定した改善を出せなかった。未達を維持し、公式の比較を再現できて別の仮説が残る回に一度戻る。

### AHC025 グループ全体の符号を優先する追試

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/ee7923e0b3c5aee4351b55d7bada7e61cf5f529e)。同じ比較回数と安全な交換の証明を使い、重さの差が大きいペアを選ぶ基準と、比較によって得られる情報が多いペアを先に選ぶ方式を比較。新規dev20件31,564,593→31,143,677だが、未調整36件79,502,512→87,588,864（低い方が良く、+10.17%悪化）。全112対話で質問数・公式・独立が一致、最大約0.2186秒。既定は更新せず。次は符号の不確実性ではなく、少ない比較で改善を証明できる交換を優先する。

### AHC066 配送順とマクロ候補を共同で選ぶ追試

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/ab27a013bb996b3dd7467cde28b71b6d9a4307ff)。同じ6本の追加マクロ候補から位置・向きの評価を作り、配送順を変えない方法と、実際の移動費で注文を入れ替える方法を比較。新規未調整24件の平均は既定225.542、固定順224.375、同時探索223.500ボタン。対固定は10勝9分5敗、中央値同点、devでは0.063ボタン悪化。全120出力は公式・独立一致、最大0.182033秒。既定は更新せず。次は状態に登録済みのマクロを持ち、再登録の費用と展開上限込みで複数区間を選ぶ。

### AHC007 代替路の辺が公開される時刻を狙う追試

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/f7a42be1fa79a332c8c3ed42eda203713e1b591a)。同じ相手シナリオと16判断分の試行で、最後の1判断を次に来る辺／代替路の最後に公開される辺に使う。新規未調整36件の合計は既定3,405,511,958、通常16判断3,406,602,708、代替時刻3,404,523,970（通常に11勝、合計−0.0610%）。全224出力で合法性と公式・独立採点が一致、最大約0.7103秒。既定維持。本番150件とテストの同一性も未確認。次は公開時刻の1辺でなく、代替候補がある時間帯を比べる。

### AHC034 土を先に拾える範囲で走行を短縮する

[単純な短絡](https://github.com/tokotoko7777/ahc-workspace/tree/553ffc16e7b4d56374c65116ad7a446be6ede499)は荷揚げすべき地点を全て訪れる条件で経路を短縮し、積み降ろしを流量で再最適化した。幾何的には短くても、拾う前に土を下ろす必要がある候補が52本中43本あった。新規未調整16件の平均は追加焼きなまし36,843,697.44→短絡38,443,379.38だが3勝12分1敗、中央値同点。全78出力公式・独立一致、任意。

[積載条件つきの追試](https://github.com/tokotoko7777/ahc-workspace/tree/5c08291e69e5a61ed806e8fb740108219f7401f9)は、各正の地点で最初に拾い、負の地点で最後に下ろすと仮定して、経路の途中で積載が負になる短絡を採用前に捨てる。その後に同じ積み降ろしの最小費用流で評価する。別の未調整16件の平均は流量版と追加焼きなまし35,225,693.81、幾何のみ37,054,395.38、積載条件つき43,686,637.44（流量版比+24.0%、15勝1分。幾何のみに13勝3分）。平均移動1,575.5→652.25、平均移動費201,719→160,202。積載量に応じる費用は増えたが固定費削減が上回った。全104出力公式・独立一致、最大0.391493秒。問題別既定に採用、本番未判定。次は平均約94,977の積載費も抑えるよう、荷を拾う位置と降ろす位置の間隔を短絡の優先度に入れる。

### AHC072 異色の小集団を、固定した足場の上で運ぶ

[追試・再現手順](https://github.com/tokotoko7777/ahc-workspace/tree/1962da2e33e2d5f72feef3e1b881fc7879690073/problems/ahc072/experiments/fixed_packet_v3)。部分列と向きで状態を持ち、最大高さ8の制限、跳躍時の反転、巣で上から帰る順序込みで、足場を動かさずに混色を運ぶ最短手数を求めた。4セルの例では1マスの跳躍だけだと盤面の偶奇と向きが固定されるが、足場を残して2マス跳ぶと2色を順に届けられる。700件の素朴な全スタック探索と照合。

別々に1匹ずつ運ぶより局所的に得な混色候補が未調整20件で328回あったが、従来の同色まとめ運びとの全体比較では、混色優先は平均425.05→564.30手（1勝19敗）。完走した良い方を採用しても425.05→424.75手（1勝19分、改善例は350→344手のみ）。全96出力公式と独立再生が一致、最大0.018611秒。任意に残し、次は単独の2体でなく、足場を残す同色2組の完走までと同じ開始盤で比較する。本番ケースは未評価。


### AHC008 公式配布100件の基準値と終盤の閉鎖

同一の公式配布100件を v3 テスタと別実装のログ再生で照合。既存適応分割
259,408,028、外周小部屋75,383,487。全200件合法。小部屋方式は人間平均到達面積
593.26/900 だが平均8.26匹が残った。適応分割が全員ペットなしを達成したのは7件。

別の新規100件では関節点閉鎖の2案とも基準値266,733,939を下回り、不採用。
単セル除去を全候補で評価できても、将来の仕切りを妨げれば点を失う。次は観測中の
ペットの位置に応じた小領域の囲いと、残った通路を使う終盤の捕獲を検証する。
ユーザー提示の100ケース目標と配布100の入力が同一とは未確認。本番2,000件は未実行。


### AHC008 並行囲い v1 を保存、面積損失が主な残課題

[固定版](https://github.com/tokotoko7777/ahc-workspace/tree/0f0079a9f94dad2e17ff27c7d395327721eacb3e/problems/ahc008)。
ペットの現在地に応じて複数班で小さい囲いを進め、最後は残った通路を仕切る。
公式配布100件で 1,153,430,892、基準259,408,028に対して84勝16敗。
公式 v3 と記録済み操作の別実装再生で100件合法、最長1.706秒。
別の未調整100件で 1,016,937,523（同一ケースの基準239,496,417、86勝14敗、全件合法）。
時間上限付近の枝で再実行値が僅かに変わる場合は、各実行時の操作を再生して照合する。

固定版の人間平均到達面積は配布100で188.84/900。現配置のまま全ペットを仮に除いても
約2.06Bが上限で、6.263Bには届かない。終盤で大きい面積を切り捨てる前に、小さい囲いで
取り切る方向へ。現在は最小カット形状、作業者の位置を考慮した即時閉鎖を同一条件で比較中。


### AHC008 最小カット囲い v2

[固定版と再現](https://github.com/tokotoko7777/ahc-workspace/tree/06c9f241f7c62ead5eadcdec68770a1236056f4e/problems/ahc008)。
既存の仕切りを利用しやすい候補を、頂点分割した最小カットで生成する。
公式配布100件 **1,338,602,186**（同一条件の v1 1,153,430,892）、全件合法、最長1.180秒。
別途生成100件も 1,483,627,404 対 v1 1,142,886,072、全件合法、最大1.259秒。
目標 6,263,333,336 は未達。同じ本番ケースだったことを検証できていないため比較は研究上の指標。

別の未調整100件の状態ログでは、最多人数の成分の面積はターン200で563、
225で520、300で202へ低下。猫は初期3.37からターン225で2.36匹が残る一方、
牛は3.03から0.78匹。終盤の広い分割でペット数を減らすだけでは目標に届かず、
面積を使わずに猫などを早期に減らす手段が必要。別案の作業者隣接壁による即時閉鎖は、
未調整100件で1,161,600,000から1,014,600,494へ悪化し不採用。


### AHC008 高速な標準版と独立検証を区別

配布100件の最高は面積優先案 **1,459,735,698**、全件合法、最大1.618秒。
独立生成100件の一組では標準v2 1,483,627,404 に対し1,453,107,690で、
標準置換しない。[面積優先案アーカイブ](https://github.com/tokotoko7777/ahc-workspace/tree/f49f4909311d4cf38d2997d5ec4b5d7cf83ced6d/problems/ahc008)。

[高速な標準v2と再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/codex/ahc008-benchmark-20261122/problems/ahc008/experiments/20261122/checkpoint.md)。
直線仕切りの行ビット化、最大流の作業領域再利用、安全な上界枝刈り。
配布・未調整各100件で全操作一致。配布100 最大1.104→.408秒、中央値.383→.225秒。
保存済み単一ファイルをGNU C++23で再構築して1,338,602,186点・合法100/100を再現。

猫の短期方向推定は精度が上がったが、予測位置に新設して完成した18個の囲いで対象猫捕獲0。
終盤の面積上限は標準版で仮に全ペットなし全員最適位置でも配布100で4.296B。
次は観測後に囲いを一から作るより、既存壁に接続する入口付きの短い通路を用意し、
入った瞬間に閉鎖できる配置を検証する。本番2,000件は未使用、mainに変更なし。


### AHC008 1時間時点: 面積優先を広い未調整入力で確認し、最終ターンを厳密評価

前段の面積優先 v3 高速版を、設定固定後に別の新規100ケース×3組で v2 と比較。
全3組で v3 が上、300件合計3,944,389,606→4,385,937,511（+11.2%）、全件合法。
[各ケースの計測、種、単一ソース](https://github.com/tokotoko7777/ahc-workspace/tree/edd579baeffc1a978718a3fc03c7b8a9fd631090/problems/ahc008/experiments/20261122)。

配布練習100件の最新は **1,465,133,201**（最長.450秒、100件合法）。
高速 v3 の最終1ターンだけ、壁と移動の合法な同時組合せを候補として比較する。
全100件で最初の299行動が同一。11件改善・89件同点。
独立生成100では1,453,107,690→1,460,793,810（13改善、悪化なし、最長.500秒）。
[現最良の単一C++23](https://github.com/tokotoko7777/ahc-workspace/blob/b448004e5c2c8df8972487a16396b8d23e0ec6b2/problems/ahc008/flex_terminal_submission.cpp)、
[再現と検証](https://github.com/tokotoko7777/ahc-workspace/tree/b448004e5c2c8df8972487a16396b8d23e0ec6b2/problems/ahc008/experiments/20261122)。
配布ケース目標6,263,333,336に未達。本番2,000件未使用、mainに変更なし。

事前に仕切りを作る別チーム案は未調整100で得点を落として不採用。
次は独立作業とせず、既存の仕切り・留まっている作業者を使って完成間際の候補を
評価し、作業の競合と残す面積を同時に見直す。

### AHC008 終盤の閉門準備と、捕獲率を軸にした次段階

1手だけ移動を変え、次ターンに動物位置を観測して合法なときだけ閉門。
公式配布100件で **1,465,133,201 → 1,512,932,629（+3.26%）**。22改善・66同点・12悪化。
公式テスタと独立再生が全件一致、最長0.448秒。調整に使っていない生成100件×2組でも +2.48%、+1.96%。
[提出用C++23](https://github.com/tokotoko7777/ahc-workspace/blob/c021799a9cc2c668a2d8ac7460216ec92bc44910/problems/ahc008/tactical_gate_submission.cpp)・
[比較ログと再現](https://github.com/tokotoko7777/ahc-workspace/tree/c021799a9cc2c668a2d8ac7460216ec92bc44910/problems/ahc008/experiments/20261122)。
目標未達。本番2,000件未使用、mainに変更なし。

残り1頭が人間側の該当成分の得点を半減するので、まず捕獲の確実性を上げ、
次に囲いの面積を削る。現標準は配布100件中、全人間が動物ゼロのケース10件、
1人以上が動物ゼロのケース65件、ケースごとの動物ゼロ人間比率の平均56.86%。
動物ゼロの人間が到達できる面積は平均205.53マス。独立生成100件×2組では全人間ゼロは各3件。
次は大きめの部屋を互いにつないで動物の通り道を確保し、観測に応じて安全な部屋をまとめて閉じる。
合計スコアに加え取り逃した頭数と、動物ゼロで残せた面積を計測する。

### AHC008 捕獲優先の4×4部屋版（得点最良版とは別保存）

部屋の間を通れる状態で仕切りを作り、動物がいない連結部屋へ全員を移して閉める。
公式配布100件で **98件**、未使用生成100件で **99件** が全員動物ゼロ（tacticalは10件、6件）。
配布の得点は912,125,586、全件合法、最長0.269秒。得点最良の1,512,932,629は更新なし。
[固定コードと公式・独立再生ログ](https://github.com/tokotoko7777/ahc-workspace/tree/1152bb06b27686f64bc64022a33751f8c1302d3f/problems/ahc008)。
配布の全員成功98件で、動物側の面積は平均665マス（900マス中）、
人間側は平均84マス。残留頭数を人間間で重複なしに数えると **5.09→0.36頭**（全100件）。
[面積の定義・追試用パッチ](experiments/ahc008/capture-topology.patch)・
[再生結果](experiments/ahc008/frozen-practice-area-summary.json)。
[静的な通り抜け罠の確率計算](experiments/ahc008/passive_capture_model.py)では、
2本の中央通路を作って待つだけだと、静的な盤面の定常分布から始めた牛の閉門可能率は300ターンで約10%。
動物を囲う側の小面積化は未達。初回の端部屋切断・待ち・密度別不等間隔は
開発30件で98〜99%相当の確実性と面積改善を両立しなかった。
次はすばやく閉められる避難先を残しつつ、隣の清浄区画を併合する。

### AHC008 人間の避難ではなく、動物を小さな区画に閉じ込める

- [犬の4マス罠](https://github.com/tokotoko7777/ahc-workspace/tree/e87e72de46ab3fe04c8f4111d13fa821ec3b7391/problems/ahc008/experiments/20261122/dog-transit)。
  盤端に細い通路を作り、反対側の人間を犬が追って通路に入ったところで両端を同時に閉める。別の80ケースで300ターンを公式テスタ検証し、犬のいる77件中65件で計91頭。捕らえた人間はゼロ、出力違反なし。2回目も試す方式では129頭。これは捕獲部品の結果であり、残りの動物も隔離する全体解ではない。
- これを既存の4×4解法に先行させると、公式配布100件の得点は912,125,586→897,546,719、全員動物ゼロは98→94。未使用生成100件でも875,592,712→849,152,563、99→97。組み込みは見送り。
- [牛・豚などの角の罠](https://github.com/tokotoko7777/ahc-workspace/tree/27fd50cdff7eb6cb990ae60965e87d61a8b944d3/problems/ahc008/experiments/compact-corners-20261122)。
  公式配布と未使用生成各100件で計21頭を3〜14マスに封鎖したが、全員動物ゼロが合計8件減り不採用。別の待機型3マス罠は開発30件で2頭を捕獲したが、そのために延べ812人ターンかかり不採用。
- 得点最良は **1,512,932,629**。次は犬以外について、全部を一から囲うのではなく、観測時点の仕切りと即応できる作業者を使って小区画を閉じる。捕獲頭数、動物側の合計面積、周囲に残った頭数を分けて測る。本番およびmainには変更なし。

### AHC008 4マスにそろえた形状別の捕獲比較

- [全5種を4マスに入れる合法性確認](experiments/ahc008/four-cell-witness/)。角の2×2へ動物を置いて4人が外から同時に壁を建てる構成例を、300ターンすべて公式テスタ・独立再生で確認。あらかじめ配置した検証用入力であり、自然な遭遇率を測ったのではない。
- [同条件の開発30件比較・再現](https://github.com/tokotoko7777/ahc-workspace/tree/c56071a5cd746cc9dfd7ff8b44a11d19ce8b3635/problems/ahc008/experiments/20261122)。1×4／2×2／L字で4マスに隔離した頭数は **5／10／9**、準備後に中を通った頭数は **6／15／14**。対象は牛・豚・ウサギ、どの方式も犬・猫の捕獲はゼロ。形ごとに場所も選び直す方式同士の比較であり、形だけの因果比較ではない。全方式で既存解より得点が低く、採用なし。
- 別の生成30件で準備後の最大待機を27→4ターンにすると、全形合計で捕獲17→8頭、既存計画から変更した行動6,080→3,423人ターン。2×2のみ捕獲効率は改善したが、頭数と点数は改善なし。既存の序盤の壁をそのまま使える完全な側面も確認できない。
- 盤端に10マスの細い通路を作り、動物を見てから両端の壁の位置を選ぶ別案を新規60件で公式シミュレータ検証。通路と人員を用意できても、待機中に牛・豚・ウサギが入ったのは28件。固定した4マスで10頭、位置を変える4マスで11頭。壁工事・配置の後さらに平均50.85ターン待機し、現時点では組み込みなし。[条件付きの確率モデル](experiments/ahc008/sliding_four_model.py)は入ってからの確率で、実戦の入口到達率とは分けている。

### AHC008 4マス・最後の壁を後で選ぶ方式、犬1頭の置き換え方式

- [2×2で最後の壁を後で決める2組×100件](https://github.com/tokotoko7777/ahc-workspace/blob/a0e9be1a7d85ebe677e0dcccfbe1a4b915c497dd/problems/ahc008/experiments/20261122/quad-deferred-validation.md)。
  公式配布で標準解／固定扉／後決め扉の最終4マス隔離は **3／32／26頭**、点数は1,512,932,629／1,250,699,651／1,476,533,852。
  未使用生成では5／20／22頭、1,688,074,201／1,317,138,009／1,579,164,236。固定扉比の変更行動あたり捕獲は両組で上がったが、標準解より点数が低く採用なし。
- [既存の序盤との競合を調べた再生ログ](https://github.com/tokotoko7777/ahc-workspace/tree/f7f7e2f4154385f15891e79e77520eaaa696d7ff/problems/ahc008/experiments/20261122)。
  動物の侵入も捕獲もなかったケースが変更行動の80〜88%を占め、3つの点数悪化事例では序盤の犬用の壁を取りやめた。犬用の斜め壁を3マス流用できる4マス形も、実際に人が揃う機会がほぼなく、組み込みなし。
- [犬が1頭のときだけ、大きな囲いを盤端の1×4に置き換える](https://github.com/tokotoko7777/ahc-workspace/blob/bd740df83aa878a2f1e6b1022ee216a0c97325bd/problems/ahc008/experiments/20261122/one-dog-tactical/README.md)。
  公式配布100件で **1,556,844,226（+2.90%）**、未使用生成100件で **1,839,946,910（−1.03%）**。
  2組の対象29件中28件で犬を4マスに隔離し、人間は閉じ込めない。対象外171件の行動は完全一致。
  全400件は公式・再生で合法、最長0.505秒。配布での数値最高を更新したが、未使用データで逆転したので、研究版として保存し標準解には未採用。本番同一条件による首位比較、mainマージともに未実施。

### AHC008 1×4の列を複数並べて、隣の担当者と繰り返し閉鎖

ユーザーが示した `A(=)B(=)C(=)D` を実装。1×4の中身の上下だけを柵で囲う3区画×12本。両端を閉めても人間の通路は残す。
[初期版のコードと開発30件](https://github.com/tokotoko7777/ahc-workspace/tree/76cfa6be1a7d8bbdbe81c12a86876c05d41c3625/problems/ahc008/experiments/20261122)で5種133頭、127閉鎖。固定の開始ターンまで待たず、[完成した区画から2人で捕獲開始](https://github.com/tokotoko7777/ahc-workspace/tree/f0712168cb7e922d97650311bd9236d829587fe7/problems/ahc008/experiments/20261122)する方式は、同じ30件で219頭、初捕獲中央値134→58ターン、点数69,288,715→232,815,017。ただし標準603,294,259には未達。
隣接レーンで余剰2人以上を固定の上下担当に分ける方式は、6人以上の別の生成30件で246→318頭、158,786,593→328,954,329。5人のケースは完全同一。別比較の横4区画は7人以上で215→256頭だが得点も全員動物ゼロの件数も悪化。全面採用なし。
[壁の面積と入口の理想確率](experiments/ahc008/tube_banks_passive_ceiling.py)は配置・スタッフ・他の動物を除いた条件付きモデル。残りの穴を横一線に埋めるだけの案では、実際の既設柵から盤面を横切るのにまだ18–30マスを埋める必要があり、実装に進まず。

### AHC008 既設柵を利用した不定形の捕獲

追加でユーザーが参考として指定した [siman](https://simanman.hatenablog.com/entry/2022/02/28/074838)・[TERRY](https://blog.terry-u16.net/entry/ahc008) の2記事のみ参照。simanの不定形案の詳細は公開記事にないため、TERRYの頂点最小カットを手がかりに別実装を設計。
[不定形の計測・実装](https://github.com/tokotoko7777/ahc-workspace/tree/606125bc81e6632352bcdff2977113b7bbd01909/problems/ahc008/experiments/20261122)。
即時準備済みは30件全て発動せず。実際に外から壁を建てられる作業者を拘束して最大5ターンだけ短期準備する版は、同じ開発30件の1件で、2枚の壁を置いてウサギ1頭を**6マスの非長方形**に隔離。全件合法・独立再生一致、最長0.595秒、合計541,987,952→542,036,564。残留頭数の合計は同じ。サンプル1件であり採用なし。現標準・研究最高・本番未判定は変わらず。
次は固定形の仕切りを早く作るため、[安全な蛇行経路を分担する補助部品と愚直比較テスト](experiments/ahc008/construction_partition/)を使い、従来の工事とだけ比較する。

### AHC008 捕獲が進まないケースで早めに方針を切り替える：研究標準更新

盤端・通路に作る 1×4 の檻と上下の固定担当は維持。160ターン目に人間側に5頭以上いるときだけ、早めに従来の捕獲・仕切りへ交代する。4頭以下なら225ターンまで1×4捕獲を続ける。
[提出形の単一C++23（引数不要）](https://github.com/tokotoko7777/ahc-workspace/blob/5b2cda366a4bd0bb7df28d51321636d65563cb59/problems/ahc008/experiments/20261122/parallel-banks-rescue-research.cpp)・[両100件のログと再現](https://github.com/tokotoko7777/ahc-workspace/tree/5b2cda366a4bd0bb7df28d51321636d65563cb59/problems/ahc008/experiments/20261122)。
引数なし版は同一ロジックで既定値のみ変更し、発動・非発動を含む4件の公式出力と採点が同一。

| 集合 | 旧標準 tactical | 繰り返し檻のみ（225まで） | 観測に応じて交代 |
|---|---:|---:|---:|
| 公式配布100合計 | 1,512,932,629 | 1,513,278,103 | **1,953,389,032** |
| 未使用生成100合計 | 1,377,143,264 | 1,230,044,649 | **1,969,989,107** |
| 公式配布・全員動物ゼロ | 10 | 12 | **31** |
| 公式配布・最終4マス隔離 | 3頭 | 946頭 | **700頭** |
| 公式配布・人間から到達可能な頭数 | 509 | 378 | **230** |

凍結後の2組で旧標準比 **+29.1% / +43.0%**。全600回で公式・独立再生一致、最大0.731秒（切替版）。檻のみと比べ、4マス捕獲数を一部落として全体得点を得る。公式配布では人間個別で動物ゼロになった人数の合計は旧標準419→279であり、**全員が動物ゼロになるケース**の増加と混同しない。
先行した[人8人以上・動物12頭以下の初期分岐](https://github.com/tokotoko7777/ahc-workspace/blob/36e3bd1ea6ccbdd4ab77a534b34ddd6edfb147f4/problems/ahc008/experiments/20261122/bank-portfolio/README.md)は配布1,789,105,112、別生成で−0.90%のため非採用。
研究枝のみ。目標6,263,333,336に未達、本番同一ケース未判定、main変更なし。次は本当に残り1頭の局面に限り、既設柵を使った非長方形を壁5–6枚・短時間で閉め、先行フェーズを崩さずに最後の1頭を減らせるか検証。

### AHC008 人が少ないときだけ2区画に減らす：研究標準更新

人が5〜6人の場合だけ、3区画×12本から2区画×13本に変更して準備する柵を156→112枚に減らす。残りの動物数で捕獲方式を切り替える160ターンのルールはそのまま。[単一GNU C++23（引数不要）](https://github.com/tokotoko7777/ahc-workspace/blob/3fbe0d94d8a5602d2d2063be2ff159dfe803fd4e/problems/ahc008/experiments/20261122/parallel-banks-narrow-rescue-research.cpp)・[凍結版と再現](https://github.com/tokotoko7777/ahc-workspace/tree/3fbe0d94d8a5602d2d2063be2ff159dfe803fd4e/problems/ahc008/experiments/20261122)。

| 集合 | 以前の3×12切替版 | 5〜6人のみ2×13 |
|---|---:|---:|
| 公式配布100合計 | 1,953,389,032 | **2,116,997,192（+8.38%）** |
| 未使用生成100合計 | 1,811,107,220 | **1,916,659,829（+5.83%）** |
| 公式配布・最終4マス隔離 | 700頭 | **706頭** |
| 未使用生成・最終4マス隔離 | 751頭 | **760頭** |
| 全員動物ゼロ・公式配布／未使用 | 31件／32件 | 30件／34件 |
| 人間から到達可能な頭数・公式配布／未使用 | 230頭／192頭 | 220頭／208頭 |

全400実行が公式・独立再生一致、切替対象外141件は出力バイトまで同一。新標準最長0.676秒。得点と4マス捕獲の改善を両100件で確認したため、得点重視の研究標準にする。全員隔離・残留頭数がどの評価集合でも改善したわけではない。
4区画に広げる案、進行方向で優先度を変える案、終盤の常時見張りは未採用。本番同一ケース未判定、6,263,333,336に未達。mainとAtCoderは変更なし。

### AHC008 不定形のS字4マス檻：配置から試す

単発の終盤囲いに加え、S字4マスを反復配置して両端の壁を同時に建てる方式を試した。5〜6人は現行のまま、7人以上の同じ開発30ケースを比較。[連結性の検査と理想条件の分析](https://github.com/tokotoko7777/ahc-workspace/commit/76c4079b421619528becf582698766fb121232c3)は、施工時間や見張りの移動を含まないため、下表の実測とは区別する。

| 指標 | 直線3区画 | S字4区画 | S字3区画 |
|---|---:|---:|---:|
| 採点合計（開発30） | 575,482,884 | 490,823,827 | 421,722,487 |
| 直接4マスに捕獲した動物 | 201 | 199 | 185 |
| 最終・全員動物ゼロ | 10 | 8 | 8 |
| 最終・動物のいない人間側連結領域の面積合計（重複なし） | 4,059 | 2,795 | 2,089 |

S字でも犬・猫を含め4マスに合法に隔離できた。各S字系列で対照と合わせ76実行が公式採点・独立再生で一致し、対象外8ケースは出力バイトまで同じ。4区画は終盤に人が使える無動物領域を維持できず、3区画は柵を減らせても捕獲機会も減った。どちらも不採用。研究標準は上記の直線版。次は直線の一部だけを、最初から一人で閉められる片口にした場合、動物が通り抜けなくなる欠点を補えるかを、まず移動モデルで検証する。

### AHC008 一人で閉じる片口檻と、直交する新配置

既存の直線36本から12本を選び、片側だけ先に塞いで一人で閉じられる檻にした。[開発30の比較・コード差分・再現](https://github.com/tokotoko7777/ahc-workspace/tree/3837ae5804f06fa927063da18831de680a874cd8/problems/ahc008/experiments/20261122)。同じ30件で真の4マス捕獲201→213頭（片口からは豚1・ウサギ4・猫1）、全員動物ゼロ10→12件、残留62→48頭。ただし動物のいない人間側の面積が4,059→3,104マス、得点は575,482,884→535,413,192（−6.96%）。片口で合法に閉じられた81局面のうち、見張りが実際に立っていたのは6局面。公式・独立再生とも全76実行合法で、対象外8件は出力同一。不採用、100ケースでの採否比較は未実施。

余った人を別の列の見張りに回す案も、既存22件で片側だけ見張りがいた82局面を再生したが、他の指示を変えず2手以内に反対側に揃えられる機会は0件。実装せず。次は、上側の平行21本と下側の直交13本を組み合わせる。全閉鎖でも通路が連結し、柵152枚で4マス檻34本。[配置の検証と、施工・見張りを除いた理想条件の比較](https://github.com/tokotoko7777/ahc-workspace/blob/116dcd387c557e2547b37b536b4c63bd162f5a08/docs/experiments/ahc008/orthogonal_bank_geometry.csv)。

同じ開発30ケースで、直交配置は直接4マス捕獲201→220頭（下側の直交列90頭）、160ターンの残り271→251頭。ところが終盤の捕獲が伸びず、最終残留62→75頭、全員動物ゼロ10→6件、得点575,482,884→498,274,502（−13.42%）。人間が到達できる最終面積の平均は344.4→423.1マスで、単純に面積を削り過ぎたわけではない。全76実行で公式・独立再生一致、対象外8件は全出力バイト同一。[コード差分と比較ログ](https://github.com/tokotoko7777/ahc-workspace/tree/21fff3d656b384a34e9c3e0f1796db722de9aa5c/problems/ahc008/experiments/20261122)。不採用、配布100のベストは更新なし。

終盤の仕事割りでは、既存の貪欲を厳密マッチングに換えても追加の即時作業は1,899回中3回だけで、未実装。[割当の観測記録](https://github.com/tokotoko7777/ahc-workspace/tree/6841b161aef2940481d3fa9b1caaa8f9d1ae18da/problems/ahc008/experiments/20261122)。次は人間が全員柵の外へ退避する時間を終盤の囲い選びに含める。現行ログでは、施工見込みより退避が5ターン以上遅い、または残り時間に間に合わない計画が25回あり、うち16回は閉鎖に至らなかった。

### AHC008 少人数版の追記：人数別に見ると

凍結済みの配布100と未使用生成100の得点差を、人数別に集計し直した。初回捕獲ターンは中央値、W/Lは同一ケース同士の勝敗。得点と標準コードに変更なし。

| 人数・集合 | 得点差、W/L | 初回捕獲 | 50ターン時点で完成した檻の合計 |
|---|---:|---:|---:|
| 5人・配布（20件） | +88,092,122、15/5 | 75→53.5 | 237→234 |
| 5人・未使用（16件） | +85,824,207、10/6 | 58→70 | 198→191 |
| 6人・配布（14件） | +75,516,038、8/6 | 53.5→59.5 | 218→220 |
| 6人・未使用（9件） | +19,728,402、6/3 | 73→61 | 142→135 |

初回捕獲が早まったのは全ての集合で再現したわけではない。改善上位3ケースを除く感度確認では、5人の合計差は両集合で正、6人は両集合で負。これは少数の大きな改善への依存度の確認で、正式な集計や採用条件は変えない。再現元は[配布ケース別台帳](https://github.com/tokotoko7777/ahc-workspace/blob/3fbe0d94d8a5602d2d2063be2ff159dfe803fd4e/problems/ahc008/experiments/20261122/parallel-banks-narrow-rescue-practice100.csv.gz)と[未使用ケース別台帳](https://github.com/tokotoko7777/ahc-workspace/blob/3fbe0d94d8a5602d2d2063be2ff159dfe803fd4e/problems/ahc008/experiments/20261122/parallel-banks-narrow-rescue-fresh100.csv.gz)。

### AHC008 速度更新：盤面が同じ間は連結成分を再利用

終盤の直線・斜めの囲い候補は、柵配置と元の人間側連結成分が同じ間だけ、分割結果を記憶。動物の位置・数と候補評価はその都度更新する。配置・得点ロジックは不変。[高速版の単一C++23](https://github.com/tokotoko7777/ahc-workspace/blob/be1cefb260f7f47b683f780eca2db2db603be3e0/problems/ahc008/experiments/20261122/parallel-banks-narrow-seam-cache.cpp)・[再現とケース別比較](https://github.com/tokotoko7777/ahc-workspace/blob/be1cefb260f7f47b683f780eca2db2db603be3e0/problems/ahc008/experiments/20261122/parallel-banks-seam-cache-notes.md)（`codex/ahc008-seam-cache`）。

| 同条件・公式v3テスタ含む逐次計測 | 配布100 | 未使用生成100 |
|---|---:|---:|
| 両版の得点 | 2,116,997,192 | 1,916,659,829 |
| 行動バイトが完全に一致 | 100/100 | 100/100 |
| 時間中央値、前→後 | 0.376→0.336秒 | 0.377→0.345秒 |
| ケースごとの時間変化の中央値 | −10.21% | −10.68% |
| 最大時間、前→後 | 0.620→0.541秒 | 0.665→0.530秒 |

開発30も含め230件すべて同一で、各版の全460実行が合法・独立再生と一致。採点内訳と目標未達は変わらない。高速版は同点の選択肢。AtCoderの本番環境での時間ではなく、mainも変更なし。[汎用化の提案](https://github.com/tokotoko7777/ahc-workspace/blob/be1cefb260f7f47b683f780eca2db2db603be3e0/docs/LIBRARY_PROPOSAL_GENERATION_TOPOLOGY_CACHE.md)。

### AHC008 全頭隔離後は柵を足さない：配布ベスト2,117,108,303

誰のいる連結領域にも動物がいなくなったら、追加の柵を建てずに待機する。動物は既存の壁を越えられず、柵を追加しても人が使える面積は増えない。同じ状態からの継続に対し悪化しないルールで、捕獲までの全行動はキャッシュ高速版と同じ。[現行ベストの単一C++23](https://github.com/tokotoko7777/ahc-workspace/blob/591d608f64e8ddd8f9ba4c42accc2069e07c85f4/problems/ahc008/experiments/20261122/parallel-banks-narrow-global-clear.cpp)・[証明、再現、ケース別台帳](https://github.com/tokotoko7777/ahc-workspace/blob/591d608f64e8ddd8f9ba4c42accc2069e07c85f4/problems/ahc008/experiments/20261122/parallel-banks-global-clear-notes.md)。

| 集合 | 高速版の得点 | 全頭隔離後に待機 | 勝／同／負 |
|---|---:|---:|---:|
| 公式配布100 | 2,116,997,192 | **2,117,108,303（+111,111）** | 1／99／0 |
| 未使用生成100 | 1,916,659,829 | **1,916,993,162（+333,333）** | 1／99／0 |

開発30も+111,111、1／29／0。各版計460実行が合法・独立再生一致。配布ケース0036は214ターンで全頭隔離後、以前の版が追加していた柵1枚を省いた。最長は配布0.528秒／独立0.539秒（ローカル公式テスタを含む）。捕獲数自体は変わらない。目標6,263,333,336に未達、本番同一ケース未判定、main変更なし。

7人の専任ペアは開発30で4マス捕獲206→214頭でも最終得点−13.25%、不採用。通路を残す不定形の囲いは[観測監査](https://github.com/tokotoko7777/ahc-workspace/commit/0d2fbbcb89f4e67603b550bcf5e3c187b64f8b64)で70候補に対し、6手以内に別々の人が担当できたのは従来版でも対象を捕獲した1候補のみ。候補生成を人から6手以内に制限しても、取り逃した動物を新たに捕獲できる候補はなかった。

### AHC008 記事の捕獲方針を一体で比較

いくつかの檻形状や終盤の単発処理だけを差し替える作業から、設営・到着時点を見た組合せ・残り1〜2頭の捕獲が連動する版へ移行。[TERRY](https://blog.terry-u16.net/entry/ahc008)と[siman](https://simanman.hatenablog.com/entry/2022/02/28/074838)の二記事のみ参照し、他の参加者のコード・解説は参照しない。仕上がった1×4の合法閉鎖は優先。動物が来そうなターンと両端に人が着くターンを合わせ、人・檻の重複を避ける。残り少数は、既存の柵を使った不定形の囲いを事前に選び、全員を外側に残して仕上げる。従来の固定160ターン切替もこの版では使用しない予定。

評価は得点だけでなく、全頭がどの人からも到達不能になった件数、最初に捕獲したターン、4マスと不定形の実捕獲、最終取り残し、実行時間。現行版の公式配布100は全頭隔離30件。記事案は着手中で、得点・全頭隔離率はまだ未測定。

### AHC008 記事統合版：配布で捕獲・得点増、全頭隔離は届かず

準備済みの1×4は即時閉鎖を優先し、それ以外は動物の予測と人が両端に到着する時刻から、競合しないペアをまとめて決める。最初の統合版は終盤の切替が遅く、開発30で4マス212→286頭でも全頭隔離18→5件。切替時間だけ従来の条件に戻した[統合v2と配布100の台帳](https://github.com/tokotoko7777/ahc-workspace/commit/10149e1a4ba15eeb389cd90d952542ec0b038267)で再検証。

| 公式配布100 | 全頭優先の現行版 | 記事v2 |
|---|---:|---:|
| 全頭をどの人からも隔離 | 30 | 25 |
| 4マスに隔離した動物 | 706 | 818 |
| 160ターンの残り | 809 | 705 |
| 最終の残り | 220 | 177 |
| 得点 | 2,117,108,303 | 2,427,202,305（+14.65%） |
| 最終盤面の面積だけの得点上限（概算） | 約41.01億 | 約49.91億 |
| 公式テスタ含む最大時間（ローカル） | 0.537秒 | 0.995秒 |

全200実行が合法・独立再生一致。ケース別得点は53勝47敗。全頭隔離を優先する今の標準は変更しない。面積だけの上限は、作った最終盤面を固定し、動物による減点を仮にゼロとした計算であり、達成できる解でも各ケースの目標でもない。統合版でもその上限は100ケース合計の目標62.63億に足りず、取り残しと使える面積を別々に改善する必要がある。

残り1頭だけに専用囲いを限定したv3は開発30でv2より全頭隔離13→11、得点834,282,356→753,886,522で、不採用。次は専用囲いの形や開始条件をいじらず、準備のため建てられる柵を、最後の閉鎖役が揃う前にも建てられるようにする。

### AHC008 記事統合v4：全頭隔離も両100で改善、研究標準更新

終盤の囲いで、全員の到着を待たず現在合法な柵から先に建てる。人は最後まで囲いの外側を移動し、残った閉鎖箇所を仕上げる。同じ1×4の動的ペア方式、t160切替、残り1〜2頭用の不定形計画は維持。[単一ファイルGNU C++23（引数不要）](https://github.com/tokotoko7777/ahc-workspace/blob/5ab6cda9f6a01b489139212f2d5f57fca17e05fe/problems/ahc008/experiments/20261122/article-capture/solve.cpp)・[再現と比較](https://github.com/tokotoko7777/ahc-workspace/blob/5ab6cda9f6a01b489139212f2d5f57fca17e05fe/problems/ahc008/experiments/20261122/article-capture/README.md)。作業枝は `codex/ahc008-article-capture`。

| 比較項目 | 公式配布100 旧標準→v4 | 新しい未使用生成100 旧標準→v4 |
|---|---:|---:|
| 全頭がどの人からも到達不能 | 30→**33件** | 33→**41件** |
| 4マス隔離 | 706→814頭 | 710→830頭 |
| 最後の取り残し | 220→166頭 | 222→152頭 |
| 得点 | 2,117,108,303→**2,598,735,635（+22.75%）** | 1,748,184,767→**2,519,824,436（+44.14%）** |
| 得点 勝／同／負 | 57／0／43 | 66／0／34 |
| v4ローカル公式テスタ込み最大時間 | 0.979秒 | 0.870秒 |

配布はv2を挟んだ2回の比較で、v2の配布100全行動ハッシュ・採点が同じことを照合。未使用は直接比較。全件合法、独立再生一致。[全結果・入力シードの凍結台帳](https://github.com/tokotoko7777/ahc-workspace/commit/4ae5db2cabc297b8715c8459efa4b1704877265d)。不定形4〜48マスだけの捕獲は配布224→207、未使用237→218と減っており、全種類の囲いで改善した意味ではない。目標62.63億と100件全頭隔離はいずれも未達、本番同一条件は未判定、main変更なし。

次は残り1〜2頭の囲いを選ぶ際、各柵の単独最短ではなく、互いに別の人を割り当てた実際の外周経路に掛かる時間を使う。一方、配布v4の最終面積上限は約48.60億点なので、捕獲だけでなく囲いに割く面積も削る必要がある。

### AHC008 同一行動の高速版：受動動物の予測を再利用

柵配置・開始位置・歩数が同じ場合だけ、牛・豚・ウサギの確率分布を共有し、長い予測が初めて必要になったときだけ延長する。犬・猫の予測、見張りと柵の選択は変えない。[引数不要の高速C++23](https://github.com/tokotoko7777/ahc-workspace/blob/304cca0fc37663a3ec51e7638d66995d7d92425d/problems/ahc008/experiments/20261122/article-capture/solve-passive-memo.cpp)・[検証／再現](https://github.com/tokotoko7777/ahc-workspace/blob/304cca0fc37663a3ec51e7638d66995d7d92425d/problems/ahc008/experiments/20261122/article-capture/PASSIVE_MEMO.md)。

配布100・未使用100の全200ケースで基準v4と全行動・捕獲イベント・得点が同一。1ケースごとの実行時間変化の中央値は配布−3.61%、未使用−2.81%（公式テスタ込みのローカル逐次測定、各ケースで実行順を交互にした）。最大時間は配布0.939→0.899秒、未使用0.967→0.873秒。全頭隔離33／41件、配布最高2,598,735,635点と、本番同一条件未判定は変わらない。

### AHC008 全頭捕獲を最優先：空いた檻で見張りを移すv7

同じ1×4＋終盤の不定形の方針で、まず全頭隔離の件数を追う。柵が揃った後、捕獲・施工に使っていない人が空の通路状の檻を通り、見張りがいない列へ移る。通過中はその檻の両口を閉めない。新しい柵は作らず、準備済みの捕獲が優先。片方の列にも人を残し、既存の捕獲と160ターン切替は同じ。[引数不要の現行C++23](https://github.com/tokotoko7777/ahc-workspace/blob/b7bb3d1bcae4c2a22dab85ad6d061841569370e8/problems/ahc008/experiments/20261122/article-capture/solve-v7.cpp)・[再現、領域と捕獲の明細](https://github.com/tokotoko7777/ahc-workspace/blob/b7bb3d1bcae4c2a22dab85ad6d061841569370e8/problems/ahc008/experiments/20261122/article-capture/V7_REBALANCE.md)（`codex/ahc008-article-capture`）。

| 同一条件のv4→v7 | 公式配布100 | 同じ未使用生成100 |
|---|---:|---:|
| 全頭をどの人からも隔離 | 33→**36** | 41→**44** |
| 全頭隔離と人平均300マス以上 | 22→25 | 24→26 |
| 4マス隔離 | 814→822 | 830→837 |
| 最後の取り残し | 166→165 | 152→139 |
| 得点 | 2,598,735,635→2,655,369,894 | 2,519,824,436→2,688,102,365 |

配布で新たに全頭隔離5件・失った2件、生成で新たに7件・失った4件。配布の新1件は人平均36マスと狭かったが、全頭隔離かつ100マス以上を数えても33→35。生成では12頭をすべて4マスに隔離し、人間側666マスを残す新規成功例もある。各版計400実行が公式合法・独立再生一致、全200の旧v4行動ハッシュを照合。両100の全頭隔離と広い領域を伴う件数が増え、研究標準をv7へ更新。[凍結台帳・入力と分類](https://github.com/tokotoko7777/ahc-workspace/commit/236d1f9255de15408ee3841ef74876313fa435e1)。スコア目標および100/100に未達、本番同一条件未判定、mainは無変更。

次に、序盤から多く残る猫を対象に、直前の移動方向と、観測履歴による目的地推定を同じ既設檻・同じ人の移動条件で比較する。まず予測値と実際の合法捕獲機会を照合するだけで、動作は変えない。

### AHC008 盤面全体に「＝」型を増設、少人数の移動を別に検証

1×4を共有柵で並べ、両端を同じターンに閉める方針を維持。v11は5〜6人で26→36基、7〜10人で36→52基を計画。開発30の100ターン時点の完成済み中央値29→36.5基、全頭隔離14→13件。未建設から動物が来そうな区画を優先するv12は開発30で14→15件、得点+24.0%だったが、独立計測で以下の通り再現しなかった。[v11/v12のコード、両100の逐次計測、ケース別台帳](https://github.com/tokotoko7777/ahc-workspace/blob/dc9105a7ac9669bd4ba3a063dfba5bc1c338fdb6/problems/ahc008/experiments/20261122/article-wide-v12/held/README.md)。

| v7→v12 | 公式配布100 | 固定の未使用生成100 |
|---|---:|---:|
| 完成した檻の中央値（t100） | 27→32 | 27→33 |
| 全頭隔離（人平均300マス以上も維持） | 36→36（25→23） | 44→33（26→20） |
| 得点 | 2,655,369,894→2,315,035,121 | 2,688,102,365→2,216,915,176 |

すべて公式採点と独立再生が一致。v7は初回の早い捕獲が多く、据え置く。早い段階から複数ペアを許すv17は短期の4マス捕獲が92→108頭でも、設営が遅れて全頭隔離13→9件（v11対比）。柵作業の評価を共有檻の完成見込みに変えた[v18](https://github.com/tokotoko7777/ahc-workspace/tree/e94a22c4a0fce055a81ce231fe9ae229a0e83a95/problems/ahc008/experiments/20261122/article-contractor-v18)でも13→11件。ペア間の同じ動物の重複を罰する[v13](https://github.com/tokotoko7777/ahc-workspace/tree/b58c7b2dc863ff5d3b6a6cd9bb9044b14d08d4fc/problems/ahc008/experiments/20261122/article-capture/diversified-arrival)は予測の重複が9.5%減り全頭隔離が15→17件だったが、人が平均300マス以上使える結果は13→10件、得点悪化で不採用。

少人数向けは[TERRYさんの解説](https://blog.terry-u16.net/entry/ahc008)のように、人数・動物の条件別に候補配置を分け、柵を建てる時間だけでなく担当者が通路へ着く時間も比較。v19の5〜6人向け3列・2横断路は開発10件で全頭2→2件、4マス捕獲76→58、最初の両端配置49.5→54ターンで不採用。7〜10人の20件は全行動一致。次に2列構造を保った中央横断路を比較する。別形状を作り足す終盤の追い込みは全頭14→10件で不採用。正式な比較目標は依然未判定で、main無変更。

### AHC008 TERRY解説の一式を統合、初期の捕獲と終盤が課題

[TERRYさんの記事](https://blog.terry-u16.net/entry/ahc008)の施工区間DP、最遅の持ち場到着を早める割当、到着時点のモンテカルロ予測、ペアと檻の重複を防ぐ割当、LowLinkによる通路の分断防止、余剰人員の異なる動物への割当、頂点最小カットと全柵の担当・退避、275ターンの比較、5盤面の選択モデルを一式として実装。図の幅が異なる箇所は、以前の1×4指定に合わせて調整し、5盤面×4方向を検証。80学習＋20校正の各ケース・各5盤面を公式採点に使用したが、記事と同じ学習規模ではない。[引数不要C++23・学習データとケース別台帳](https://github.com/tokotoko7777/ahc-workspace/blob/98771f9878c395b7c11e36a4b999dc9949d511b7/problems/ahc008/experiments/20261122/article-capture/terry-integration/PILOT_RESULT.md)。

| 厳密なv7→統合版 | 公式配布100 | 独立生成100 |
|---|---:|---:|
| 全頭隔離 | 36→3 | 44→4 |
| 得点 | 2,655,369,894→989,470,357 | 2,688,102,365→846,529,625 |
| 100ターンまでの4マス捕獲 | 433→101頭 | 456→114頭 |
| 全ターンの4マス捕獲 | 822→874頭 | 837→907頭 |
| 他の大きさでの捕獲 | 499→0頭 | 557→0頭 |

全400実行公式・独立再生一致で、全200のv7対照も凍結済みの行動と得点に一致。v7を研究標準に維持し、mainは変更しない。開発30で統合版の盤面だけv7に戻すと早期4マス捕獲35→103、全頭隔離0→3。さらに従来の終盤処理だけ戻すと3→13（v7は14）で、前半の行動が一致するため終盤の寄与も分離できた。別シードも含めた確認中。記事の実装そのものとの比較ではなく、この再実装についての計測。

### AHC008 v24：施工を優先し、持ち場の割当を毎ターン更新

ユーザーが許可したTERRYさんの提出を静的に確認し、原文の転載・実行はせずに[独立実装の試験記録](https://github.com/tokotoko7777/ahc-workspace/blob/b00f17ea845f49576353b9241e06afd1b7da534e/problems/ahc008/experiments/20261122/article-capture/terry-reference-review/REVIEW.md)を保存。v7の少人数向け2列／通常3列の盤面と従来の終盤補助を残す。担当の柵が一時的に塞がれても捨てず、施工を捕獲予定より先に割り当てる。施工中の残りと最寄り通路マスへの移動を見込み、毎ターン全員の持ち場を割り当て直す。最初の統合版とは別の[単一ファイル・テスト台帳](https://github.com/tokotoko7777/ahc-workspace/tree/abafd80edf423db5abea3c5725b265cb092233c8/problems/ahc008/experiments/20261122/article-capture/v24-source-release)。

| 旧v7→v24 | 公式配布100 | 独立生成100 |
|---|---:|---:|
| 得点 | 2,655,369,894→3,199,806,134 | 2,688,102,365→3,285,615,612 |
| 全頭隔離 | 36→47 | 44→44 |
| 全頭隔離かつ人平均≥300マス | 25→33 | 26→35 |
| 100ターンまで／全ターンの4マス捕獲 | 433→505／822→921 | 456→506／837→954 |
| 最終残留動物 | 165→117 | 139→148 |

400実行は公式合法・独立再生一致、旧v7の入力・行動・得点を全200件照合。新標準のテスタ込み最大は1.073秒。別seedの全頭数が同点、残留は9頭増えたため終盤の改善余地あり。公式配布と独立入力で得点と広い領域を伴う全頭件数が改善したため、v24を研究標準に採用。歴史的1位は本番条件で未判定、mainは変更なし。次は配置負担を増やさない片口4マスや、終盤の最後の1〜2頭を早く隔離する差分を測る。

### AHC008 5人時の盤面差と、不定形の閉鎖の切り分け

開発用の5人30ケースを固定し、1×4の基本配置6通りを各ケースで比較（180実行、公式テスタと独立再生一致）。現行の配置を強制すると30件すべての行動が一致。結果を見てからケースごとに配置を選ぶ診断でも、人側に平均300マス以上を残す全頭隔離は8件が上限。この6候補の範囲の話で、実戦の選択成績ではない。現行で全頭隔離したが狭かった8件のうち7件は、終盤の1手で人側が217〜732マス減少。[構成と柵1枚の試験記録](https://github.com/tokotoko7777/ahc-workspace/blob/364ed6bbb2a922b39632826e43f60d5b27798c04/problems/ahc008/experiments/20261122/article-capture/v24-five-worker-rescue/README.md)。

既設柵を使った不定形の両口同時閉鎖は同じ開発30で434,537,087→470,780,101点、全頭隔離10→11件、広い領域を残す全頭隔離2件のまま。増えた1件は人側106マス。公式100・別seed100の最良値はv24のまま。次は両口の一方にもう一人が1手で着けて、動物の最大移動距離から次ターンも柵を置ける場合の先回りを比較する。

### AHC008 5人時の失敗を再生：見張りの不足と終盤の門

[5人の配置、両口の早期配員、未使用100の結果](https://github.com/tokotoko7777/ahc-workspace/blob/39797bbaee14498751f6e290acb3b28944d3a6a7/problems/ahc008/experiments/20261122/article-capture/v24-mixed-crew/README.md)。開発30で最後まで残った猫16頭すべてに、両端に人さえいれば閉じられた1×4への入室があった。2人だけを予測で先回りさせる案は、別100のうち5人の13件で全頭5→2件、得点171,722,219→151,221,005で不採用。

[終盤の囲いを中断する条件と各追試](https://github.com/tokotoko7777/ahc-workspace/blob/f6e56d4947c80d75284126acf90f04071e174ca8/problems/ahc008/experiments/20261122/article-capture/v24-final-planner/README.md)。失敗ケースの51計画中42回で、動物が門予定地に乗った瞬間に計画を破棄していた。破棄条件を緩和しても開発30の全頭10・広く全頭2件は不変。各門への担当割当と最遅到着を厳密化した別案は、未使用の5人生成100で1,291,647,901→1,317,286,791点、全頭24→23、広く全頭6→5で不採用。現行v24の公式配布・別seedの点は据え置き。次は途中で置く柵も含め、閉鎖までの行動を比較する。mainは無変更。

### AHC008 v25：通路ごとの待機と、広い側を残す短い仕切り

5人だけ、先に割り当てられた施工や捕獲を変えず、空いた通路内の人を通路ごとに動物へ割り当てる。別通路では同じ動物を追ってよい。その後、通常の早期切替が起きた場合に限り、178〜204ターンで少ない壁により2頭以上を切り離し、広い人側を残すと見込める仕切りを候補に加える。[v25単一GNU C++23・計測と再現](https://github.com/tokotoko7777/ahc-workspace/blob/d3533eb5ca7386edb0d3db0ac6426aa5bcfe2b78/problems/ahc008/experiments/20261122/article-capture/v25-early-seam/README.md)。仕切りの面積条件は初期の見込みで、実際の最終面積は再生計測。

| v24 → v25 | 得点 | 全頭隔離 | 全頭隔離かつ人平均≥300 |
|---|---:|---:|---:|
| 公式配布100 | 3,199,806,134 → 3,294,019,394 | 47 → 47 | 33 → 34 |
| 別seed100 | 3,285,615,612 → 3,358,241,534 | 44 → 45 | 35 → 36 |
| 未使用の5人生成100 | 1,291,277,471 → 1,407,647,573 | 22 → 29 | 4 → 5 |

公式配布・別seedの400実行は公式テスタ・独立再生と一致。現研究基準として採用。配布の取り残しは117→123頭と増えたため、全頭捕獲までの過程に改善の余地がある。本番は未判定でmainは無変更。[共通化する小人数割当APIの提案](experiments/ahc008/optional-weighted-assignment.md)は別。次は早く予約した仕切りが205ターンを超えても広い側を維持できるか、現行と同じ入力で比較。

### AHC008 v25の高速化（方針・探索回数は据え置き）

プロファイルで捕獲予測がCPUの約21%を占めた。候補の檻ごとに同じ動物を数え直さず、位置ごとのビットマスクで[集計を一括化](https://github.com/tokotoko7777/ahc-workspace/blob/40208bbd9c5af527cb99f1d1b806b34c937eabb3/problems/ahc008/experiments/20261122/article-capture/v25-batched-windows/README.md)。同じ公式配布100と別seed100でCPU13.78%／14.47%減、全行動199/200一致、275手直前まで全200一致。配布の1件は、旧版の計測を繰り返しても再現する終盤の時間制限由来の試行数差。得点更新として扱わない。[ライブラリへの汎用化提案](https://github.com/tokotoko7777/ahc-workspace/blob/40208bbd9c5af527cb99f1d1b806b34c937eabb3/problems/ahc008/experiments/20261122/article-capture/v25-batched-windows/reuse-proposal.md)は別。

試行状態と通行可能な隣接セルを再利用した[v25-fast2](https://github.com/tokotoko7777/ahc-workspace/blob/d6eafd3ff9edb8f55421e1df903113d30efbd1ce/problems/ahc008/experiments/20261122/article-capture/v25-fast2/README.md)ではその高速版からさらにCPU3.41%／3.33%減。これら二つの100では高速版とv25-fast2の行動・得点は全件一致。CPU値は同一入力・交互の逐次実行によるローカル計測で、百分率の単純加算はしない。得点を更新したわけではなく、main無変更。余裕を使って5人の予測を64→128回にする案は、未使用開発30で160ターンまでの4マス捕獲197→197、全頭11→6で不採用。

### AHC008 TERRY手順の忠実化：5種類の盤面・片口と両口・終盤の全方針比較

ユーザーの指示により、以前の1×4だけのアレンジをいったん外した。指定されたTERRYさんの実装を静的に調査し、5種類の盤面、片口・両口で広さが異なる部屋、元の盤面選択、施工と通路割当、100試行×30ターンの同じ部屋に残る確率、275ターンの終盤切替を別のGNU C++23実装で再現した。[ソースと台帳・再現手順](https://github.com/tokotoko7777/ahc-workspace/blob/8e657891bf5181216e2ec66f4aca47a1cc54166d/problems/ahc008/experiments/20261122/article-capture/terry-faithful-cpp/README.md)。元提出の実行ではなく、公開しない原データ（盤面選択の係数）は手元で読み込んでビルドする。

| 同じ入力で v25-fast2 → 独立C++版 | 得点 | 全頭隔離 | 全頭隔離かつ人平均≥300 | 最終残留 |
|---|---:|---:|---:|---:|
| 公式配布100 | 3,293,908,283 → **6,082,666,664** | 47 → **98** | 34 → **98** | 123 → 2 |
| 独立生成100 | 3,358,241,534 → **6,037,488,888** | 45 → **96** | 36 → **96** | 126 → 5 |

全400実行で公式テスタ・公式再生・独立採点が一致。新研究基準を凍結。最大ローカルテスタ時間2.762秒、本番比較は未判定でmain無変更。配布100で目標点まで180,666,672点。残っている2頭を現状の面積で隔離した場合の得点上積みは約61.78百万点なので、次は人側に残す面積も調べる。

### AHC008 TERRY手順の高速化：通路配員の再計算省略と固定近傍のキャッシュ

施工を終えた全員が必要な通路にいるとき、既に配員の到着時間がゼロなので再計算を省略。次に、同じ予定柵を使う捕獲予測で通行可能な隣接セルを順序付きで計算して再利用した。[固定作業量の測定・コード再現](https://github.com/tokotoko7777/ahc-workspace/blob/5a7bd0a9d9c1314b9cb806a8952367f8b9de03f7/problems/ahc008/experiments/20261122/article-capture/terry-motion-cache/README.md)。当初の配員高速版からさらに配布100・別seed100のCPUを13.06%・12.17%削減、全行動と得点を再現。汎用案は[別紙](experiments/ahc008/static-grid-step-cache.md)、共通ライブラリは無変更。

[同じ通常時間予算での再検証](https://github.com/tokotoko7777/ahc-workspace/blob/ccd2cc40a9b19ec277112ecd0e74e7394aab2af6/problems/ahc008/experiments/20261122/article-capture/terry-motion-cache/DEFAULT_PRESET.md)では、配布100の得点6,082,666,664・全頭98/100は対照と一致し、終盤試行の合計は452→547。未使用の生成100では6,077,388,888→6,079,166,666、全頭98→98、終盤試行723→866。1件だけ切り替えが変わった結果で、得点勝ちの再現性を主張しない。高速版を以後の研究に用いる。配布の目標点は未達、本番順位は未判定でmain無変更。

### AHC008 短期の捕獲予測を厳密化

[3組の比較ログ・再現とコード](https://github.com/tokotoko7777/ahc-workspace/blob/08f71a465a41b0005a4ad0f9bcb674d93588d008/problems/ahc008/experiments/20261122/article-capture/terry-passive-exact-short/VALIDATION.md)。牛・豚・ウサギが1〜3ターン後も元の部屋にいる確率を、予定柵を固定した移動モデル上で厳密に求める。犬・猫、他の時間幅、元の乱数列はそのまま。各100件の開発・事前確定の未使用・公式配布で順に **+0.554%・+0.575%・+0.184%**。全頭は **98→99・98→99・98→98**。公式配布では新最良 **6,093,833,333点**、目標まで数値差169,500,003点。全600実行で公式テスタ・再生・独立採点が一致。研究基準に採用し、次は隔離する動物を減らさず人側の可到達面積を残す終盤最小カットの同値解を調べる。本番の順位は未判定、mainは無変更。汎用化は[短い確率伝播の提案](experiments/ahc008/exact-local-markov-propagation.md)を参照。

### AHC008 人側の面積不足を分解、短い門予測は不採用

[全頭捕獲59ケースの面積内訳](https://github.com/tokotoko7777/ahc-workspace/blob/609c1f8d3b8a716b74f412155718a233b90cff04/problems/ahc008/experiments/20261122/article-capture/terry-final-area-budget/README.md)では、300ターン目の人側の外は平均で設営柵253.75、他の柵26.44、動物側の床61.20、空の孤立床0.085マス。人間同士が分断された分は0。[同一費用の終盤カットだけを変える調査](https://github.com/tokotoko7777/ahc-workspace/blob/609c1f8d3b8a716b74f412155718a233b90cff04/problems/ahc008/experiments/20261122/article-capture/terry-chase-cut-ties/README.md)は別60の175状態から厳密候補9件、4盤面の最大を合計28マスで基準50未満。実解に適用しない。

[短期予測で動物自身が門をふさぐ確率を除く案](https://github.com/tokotoko7777/ahc-workspace/blob/61e493cf3de61652cc66ba7c6d318653447a8f8b/problems/ahc008/experiments/20261122/article-capture/terry-gate-safe-short/RESULT.md)は新規生成100で6,063,529,628→6,079,263,888点だが全頭98→97、残留2→6。追試・配布には進まず不採用。次は設営柵そのものを減らし、捕獲率を保てるかを検証。公式配布の最良値は6,093,833,333、本番順位未判定、main無変更。

### AHC008 右端の省柵配置と追い込み切替

[右側の広い範囲を縦の罠で置き換える案](https://github.com/tokotoko7777/ahc-workspace/blob/0e6f0655a0fb8015a29c6a2e46b7699a5115da4a/problems/ahc008/experiments/20261122/article-capture/terry-map3-belt/RELAY_RESULT.md)は、巡回担当が1歩ごとに旧通路へ戻される不具合を同じ盤面で再現・修正。修正後も別100の対象40で全頭39→23、非対象60は全行動一致。設営柵は67マス減ったが不採用。[元の両口の罠を残す右端のみの案](https://github.com/tokotoko7777/ahc-workspace/blob/f3ace1e2bcb7c772191fa0b0f0231a24bda1ab/problems/ahc008/experiments/20261122/article-capture/terry-map3-belt-outer/RESULT.md)は別100の対象35で設営柵9マス減、全頭35→32、共通全頭の人側面積+2.5マス。非対象65は全行動一致。どちらも追試・配布には進まない。

[終盤で追い込みの捕獲成功率を比較](https://github.com/tokotoko7777/ahc-workspace/blob/514b252567dc0ebda66888ce2d004b0cc4014085/problems/ahc008/experiments/20261122/article-capture/terry-terminal-dominance/README.md)。既存の独立生成200のうち追い込みを選んだ25件を固定試行数で再生し、全行動・得点と比較値を再現。25件とも、通常運転より全頭捕獲の推定が悪いという取り消し候補はなかったため実装採用なし。[猫の行先を使う別案](https://github.com/tokotoko7777/ahc-workspace/blob/514b252567dc0ebda66888ce2d004b0cc4014085/problems/ahc008/experiments/20261122/article-capture/terry-cat-init-audit/EXPOSURE.md)は生成60で275手目に人側に残る猫が3件だけで、追い込み判定の対象が少なすぎると判断。現行配布100の最良は6,093,833,333・全頭98、残った2頭は猫。本番未判定・main無変更。

### AHC008 設営の区間分担を厳密なまま高速化

[固定作業量での検証](https://github.com/tokotoko7777/ahc-workspace/blob/54cf9c1764a4fc02f876106f2ce8f679fc553b39/problems/ahc008/experiments/20261122/article-capture/terry-split-dp/RESULT.md)。担当者集合×仕事の先頭からの件数のDPで、残り人数では終えられない区間を除き、接近距離を前計算し、向きと復元の同点順も維持。360キュー照合・400公式テスタ実行に一致。配布／独立の各100でCPU **30.15%／25.93%短縮**。[汎用化提案](experiments/ahc008/ordered-route-split-dp.md)は別紙、共通APIは無変更。

[通常設定の検証](https://github.com/tokotoko7777/ahc-workspace/blob/791e3a555b05639cf776d0bbdbee49267b25c3d7/problems/ahc008/experiments/20261122/article-capture/terry-split-dp/DEFAULT_VALIDATION.md)では配布100 **6,093,833,333・全頭98/100**、全行動一致、終盤試行499→561。新規生成100は全頭97のまま1件の終盤切替で +20,865,740、試行878→970。通常設定の全400公式テスタと独立再生も一致。より速い同じ解を研究基準として採用。本番は未判定、main無変更。次は追い込みの到達範囲で重複BFSを省く。

### AHC008 追い込み計算の高速化と近い将来の入室

[多始点BFS](https://github.com/tokotoko7777/ahc-workspace/blob/6a2f3e68b93f9f9db4a26a3735371f8cf1e7f075/problems/ahc008/experiments/20261122/article-capture/terry-chase-multisource/RESULT.md)は固定作業量で全手一致、ただし終盤試行あり12件のCPU短縮1.17%で先行基準2%未満。[最大流の確保領域だけを再利用](https://github.com/tokotoko7777/ahc-workspace/blob/d44b9ec1183c68470f1422c034dc04c6a4a88448/problems/ahc008/experiments/20261122/article-capture/terry-flow-reuse/DEFAULT_VALIDATION.md)は400固定作業量ランで全行動・得点一致、終盤試行ありの配布／独立でCPU9.22%／10.38%短縮。通常の配布100は試行570→645・全頭98のまま1件で約155.6万点減。独立100は1件で約33.3万点増、全頭98は同じ。後者はアルゴリズムとして保存するが、採点基準版は設営DP高速版のまま（6,093,833,333）。

[門が閉まるまでの1〜2手を予測に加える案](https://github.com/tokotoko7777/ahc-workspace/blob/aa971413c83f85d2fbee09e021994026d23aab77/problems/ahc008/experiments/20261122/article-capture/terry-first-safe-window/RESULT.md)は独立生成60の事実再生で、他作業を保ちつつ閉められる新しい遅延例が1件のみ、先行基準5件・3盤面未満で本採点なし。次は動物がまだ部屋の外から入ってくる見込みを独立に評価する。mainは無変更、本番未判定。

### AHC008 入室を予測して先回りし、公式配布100の全頭捕獲を達成

[検証・再現と採用版](https://github.com/tokotoko7777/ahc-workspace/blob/54ac18edd37c60c891ade6f72f2438c27ab4141a/problems/ahc008/experiments/20261122/article-capture/terry-incoming-room/VALIDATION.md)。既存の部屋内予測を変えず、共有通路などまだ部屋と認定されない場所から1〜3手後に入る動物を加えた。各動物の短い整数確率伝播1回を対象の部屋すべてに集計。入った位置が自分で門をふさぐ場合は加えない。牛・豚・ウサギと元の2歩近似の猫が対象、犬は既存のまま。事前の事実再生60では、他人の作業を残して閉められる別の入室の機会が27盤面で38件。これは因果的な捕獲数ではない。

| 通常設定 | 対照 → 採用の点 | 全頭 | 取り残し |
|---|---:|---:|---:|
| 新規生成100 | 5,990,111,109 → 6,055,944,442 | 95→98 | 6→2 |
| 事前確定・未使用生成100 | 5,949,695,369 → 6,051,812,965 | 94→99 | 9→2 |
| 公式配布100 | 6,093,833,333 → **6,152,805,559** | **98→100** | **2→0** |

全600回で公式テスタ・公式再生・独立採点一致。配布の目標点までの数値差は110,527,777。次は全頭を維持して人の面積を増やす。本番と配布は別条件、順位未判定、mainは無変更。

### AHC008 全頭捕獲後の面積改善：柵と配置選び

[間仕切りの後回し](https://github.com/tokotoko7777/ahc-workspace/blob/3841bd49d09907146d44ffca1d8cc15df0acbfdf/problems/ahc008/experiments/20261122/article-capture/terry-brace-valves/TIMING_RESULT.md)は、採用版の未使用100の最終盤面では固定状態に限り平均10.88マスの余地があったが、現実の手順に沿って元の捕獲までに建てられるのは必要666件中440（66.1%）のみで実装採点なし。[門を1マス内側にする案](https://github.com/tokotoko7777/ahc-workspace/blob/51a7fdbf10ddac0a2ae75c25dacad2c2a80d18e4/problems/ahc008/experiments/20261122/article-capture/terry-inward-online/RESULT.md)も、事前の実装で選んだ238回のうち78回でだけ元の次手に合致し、50%基準を満たさず不採用。

[人が分断される柵のうち一つを省く案](https://github.com/tokotoko7777/ahc-workspace/blob/64bc56e6ec73569843118ad922490b8fe823096c/problems/ahc008/experiments/20261122/article-capture/terry-chase-no-split/RESULT.md)は新規100で全手・得点一致、実際の省略は0件で不採用。[つながった人員の通路を保護する終盤カット](https://github.com/tokotoko7777/ahc-workspace/blob/31750ac463061814a6a755bb6ac1924c799003af/problems/ahc008/experiments/20261122/article-capture/terry-chase-tree-protect/RESULT.md)は、既存の独立テープ375状態に検証候補4件／2盤面で証拠数不足。どちらも配布は未実施。

[既存5配置をすべて同一条件で比較した台帳](https://github.com/tokotoko7777/ahc-workspace/blob/328ed81d07400c36846cd7c39275fe02e6f2a942/problems/ahc008/experiments/20261122/article-capture/terry-area-map-atlas/RESULT.md)では、新規生成100×5を終盤切替なしで公式・独立採点。原配置でも全頭捕獲する盤面だけ、別の全頭配置を事後選択すると32盤面で合計1,193、全100平均11.93マスの余地。ただし元map2を一律map3に変えると全頭28→19。この試算は配置を実際に選ぶ方法でも、通常設定での得点更新でもない。次は因果的に観測できる初期の人配置と施工・配員順の条件で、安全な切替を検証。配布の最高は引き続き **6,152,805,559・全頭100**、mainは無変更。

### AHC008: 終盤の初手再利用を採用（2026-11-23 JST）

- 最新の採用版: [固定回数・通常時間の検証記録](https://github.com/tokotoko7777/ahc-workspace/blob/2043b6b508cf97c73e91c5bf47e4fae650a16783/problems/ahc008/experiments/20261122/article-capture/terry-first-chase-reuse/DEFAULT_VALIDATION.md)。作業ブランチは `codex/ahc008-article-capture`。歴史上の比較目標は 6,263,333,336、配布100ケースの現ベストは **6,152,805,559**（全頭100件）。試験入力は異なるので本番順位の判定は引き続き未判定。
- 同じ終盤試行の最初の一手だけを再利用。同一作業量で終盤試行ありケースの CPU 時間 −5.14% / −5.82%。通常の時間枠で全200組の行動と得点が同一。公式配布100ケースの試行620→657、別生成100ケース874→956、別生成の全頭は97件。
- [待機者の重複担当削減](https://github.com/tokotoko7777/ahc-workspace/blob/2043b6b508cf97c73e91c5bf47e4fae650a16783/problems/ahc008/experiments/20261122/article-capture/terry-idle-cap2/RESULT.md) は60件で準備行動を変えたが新しい翌手の捕獲0件。[両端の妨害込み確率](https://github.com/tokotoko7777/ahc-workspace/blob/2043b6b508cf97c73e91c5bf47e4fae650a16783/problems/ahc008/experiments/20261122/article-capture/terry-joint-next-gate/RESULT.md) はBrier誤差改善0.00725、事前基準0.010未達。いずれも不採用。
- 次: 別生成100ケースで残った4頭を公式再生の同一軌跡で分類し、失敗がどの工程で起きたかを切り分ける。設営分担DPでは有効な上界だけで枝刈りし、復元の同点規則も含めて同一性を検証する。

### AHC008: 設営の分担DPを高速化（2026-11-23 JST）

- [最新採用版・通常時間の再検証](https://github.com/tokotoko7777/ahc-workspace/blob/ce94f08fff8196f4485f0795237af69e81aa473a/problems/ahc008/experiments/20261122/article-capture/terry-split-bound/DEFAULT_VALIDATION.md)では公式配布100ケース **6,152,805,559点 / 全頭100件**、別生成100ケース 5,998,857,137点 / 全頭97件。両パネルの全行動列と得点が以前の採用版と一致。歴史上の目標6,263,333,336とは入力が異なり、本番1位超え未判定。
- 実行可能な割当で上界を作り、それを厳密に超える枝だけ除いた。同作業量の比較で全体CPU −9.25% / −6.22%。[汎用の分担DP改修案](https://github.com/tokotoko7777/ahc-workspace/blob/ce94f08fff8196f4485f0795237af69e81aa473a/problems/ahc008/experiments/20261122/article-capture/terry-split-bound/REUSE_PROPOSAL.md)と[ロールアウト初手の再利用案](https://github.com/tokotoko7777/ahc-workspace/blob/ce94f08fff8196f4485f0795237af69e81aa473a/problems/ahc008/experiments/20261122/article-capture/terry-first-chase-reuse/REUSE_PROPOSAL.md)を保存済み。
- [別入力で残った4頭](https://github.com/tokotoko7777/ahc-workspace/blob/ce94f08fff8196f4485f0795237af69e81aa473a/problems/ahc008/experiments/20261122/article-capture/terry-fresh-residual/RESULT.md)について、即時に合法・担当済みの捕獲候補は0。犬・豚は自身が柵の妨害範囲、ウサギは最短ETA2、猫は既存罠に入らず。[2手先回りの別パネル検証](https://github.com/tokotoko7777/ahc-workspace/blob/ce94f08fff8196f4485f0795237af69e81aa473a/problems/ahc008/experiments/20261122/article-capture/terry-late-reservation/RESULT.md)でも新規捕獲候補0で不採用。次は観測からの犬の動き推定と、面積を維持しながら不要な終盤計算を省けるかを検証。

### AHC008: 罠の間引きは捕獲率で不採用（2026-11-23 JST）

- [配布とは別の固定100ケースの実測](https://github.com/tokotoko7777/ahc-workspace/blob/29bd87e61d4aa98337ffd850f1b75de1de1b1616/problems/ahc008/experiments/20261122/article-capture/terry-retired-pairs/LIVE_RESULT.md): map2/3で16柵・8室を省く4箇所分散案。全合法だが **6,201,888,889 → 6,047,861,110点、全頭100→93**。対象両完捕73件で+9.60マスでも、追加の未捕獲ペナルティが上回る。別の既存100再生で元の省略対象罠は67盤面・160頭の初捕獲に使われ、114頭は160ターンより前。不採用で保留版の比較はしない。
- [犬の対象追跡](https://github.com/tokotoko7777/ahc-workspace/blob/29bd87e61d4aa98337ffd850f1b75de1de1b1616/problems/ahc008/experiments/20261122/article-capture/terry-dog-prior/RESULT.md)は予測誤差を減らしたが、12ケース/25頭の事前基準に11ケース/17頭で届かなかった。[Normal試行初手キャッシュ](https://github.com/tokotoko7777/ahc-workspace/blob/29bd87e61d4aa98337ffd850f1b75de1de1b1616/problems/ahc008/experiments/20261122/article-capture/terry-first-normal-reuse/RESULT.md)も正確だがCPU低下0.53%のみで不採用。
- 採用版は公式配布 **6,152,805,559 / 全頭100件**、目標6,263,333,336・本番順位は条件違いで未判定。次: 罠の数を減らさず複数部屋を同時に組み直して柵を減らせるか、先に厳密な幾何条件で調べる。

### AHC008: 中盤の取りこぼしを再生で切り分け（2026-11-23 JST）

- [固定80ペアの段階別診断](https://github.com/tokotoko7777/ahc-workspace/blob/549eee2f3f42359d3ab0386b50638221ef006195/problems/ahc008/experiments/20261122/article-capture/terry-retired-pairs/pipeline/RESULT.md)。100〜159ターン、存続罠の進入857→871、両端とも合法726→723、担当者が間に合う438→446。元の捕獲535→555と増えたが、省いた罠で元は94頭捕獲しており、全体629→555。合法に担当者が揃った頭は両版とも同ターンに捕獲された。特定の最終失敗に対する反実仮想ではない。
- [同数の3部屋を保つ詰め替え](https://github.com/tokotoko7777/ahc-workspace/blob/549eee2f3f42359d3ab0386b50638221ef006195/problems/ahc008/experiments/20261122/article-capture/terry-three-room-retile/RESULT.md)は固定した5×6範囲で認証できた7柵案0件。[完成囲いでの重複予測上限](https://github.com/tokotoko7777/ahc-workspace/blob/549eee2f3f42359d3ab0386b50638221ef006195/problems/ahc008/experiments/20261122/article-capture/terry-horizon-geometry/RESULT.md)と[全頭隔離済みなら待機する高速化](https://github.com/tokotoko7777/ahc-workspace/blob/549eee2f3f42359d3ab0386b50638221ef006195/problems/ahc008/experiments/20261122/article-capture/terry-all-isolated-freeze/RESULT.md)も採用条件未達。現ベストは配布100で **6,152,805,559 / 全頭100** のまま。
- 次: 罠は維持し、動物が柵を邪魔して設営担当が待つターンで、手元の別の合法な柵を先に置いて早められるかを調べる。

### AHC008: 片側3マス罠は通過交通で不採用（2026-11-23 JST）

- 元の75室を維持して16室を3マス片側入口へ、予定柵+2。[設営テスト](https://github.com/tokotoko7777/ahc-workspace/blob/bf1029d63bb7b56df202eee7268ce784a62d0a2d/problems/ahc008/experiments/20261122/article-capture/terry-single-runs/NORMALIZATION.md)では予定柵になる作業位置から5件の行き詰まりを特定、外の足場へ移して35/35通過。別100の[実測](https://github.com/tokotoko7777/ahc-workspace/blob/bf1029d63bb7b56df202eee7268ce784a62d0a2d/problems/ahc008/experiments/20261122/article-capture/terry-single-runs/LIVE_RESULT.md)は6,049,960,322→5,994,050,593、全頭95→94。両版全頭の変更70ケースでも面積平均−4.186マス、不採用。
- [154本の再生](https://github.com/tokotoko7777/ahc-workspace/blob/bf1029d63bb7b56df202eee7268ce784a62d0a2d/problems/ahc008/experiments/20261122/article-capture/terry-single-runs/pipeline/RESULT.md)では100ターンまでに奥柵1,209/1,232完成。使える対象室は1,106→1,129でも、100〜159ターンの同じ3マスへの進入が1,117→411（個体410→167）。早期設営や収容面積だけでなく、通り抜けの維持も条件にする。
- 配布100ケース採用版は **6,152,805,559点、全頭100ケース**、歴史上の別ケース目標に数値で110,527,777差。本番1位判定は保留。次: 元の通り抜けを残したまま、既存の短期担当を奪わず4〜6ターン先の捕獲位置へ先回りできるか調べる。

### AHC008: 終盤の最大流を高速化、得点版とは別に保存（2026-11-23 JST）

- [独立カーネル](https://github.com/tokotoko7777/ahc-workspace/blob/56d74d64b862a5491347742204cc4f858a936cd0/problems/ahc008/experiments/20261122/article-capture/terry-chase-csr/RESULT.md): 通れないマスを除いたCSR最大流で、実戦再生175局面と合成300局面の流量・切断・行動が完全一致。行動決定のCPU−53.2%／−38.0%。
- [100ケース×2の統合比較](https://github.com/tokotoko7777/ahc-workspace/blob/56d74d64b862a5491347742204cc4f858a936cd0/problems/ahc008/experiments/20261122/article-capture/terry-chase-csr/INTEGRATION.md): 同じ試行回数で全行動と得点一致、全体のCPU−25.64%／−25.10%。通常の時間制限では配布100のうち1件で方針が変わり、6,152,805,559→6,151,694,448点（全頭100件維持）。別生成100は+555,555点。高速化部品を保存し、得点版は変更しない。

### AHC008: 固定8試行による終盤の判定は不採用（2026-11-23 JST）

- 新規559ケースのうち275ターンで対象が残った最初の100件を[条件付きで比較](https://github.com/tokotoko7777/ahc-workspace/blob/387d14a9588b54a87f359a3041d4e67b4f6eddc6/problems/ahc008/experiments/20261122/article-capture/terry-terminal-k8/RESULT.md)。現行版5,258,994,445点（全頭78、有効77）、全件追い込み5,081,952,780点（79、75）、固定8組の平均による選択5,125,505,557点（74、73）。8組中7組以上勝つまで切り替えない案は4,705,611,111点（57、56）。いずれも配布100とは異なる条件で、切替ルール・得点版は変更しない。

### AHC008: 猫の履歴予測は方向で改善、選択への導入は未採用（2026-11-23 JST）

- [方向予測の確認](https://github.com/tokotoko7777/ahc-workspace/blob/17c77028d6ee998971a5d76e3a01dd16cbac79c6/problems/ahc008/experiments/20261122/article-capture/terry-target-active275/RESULT.md)は終盤に猫が残った45ケースのうち34件で予測誤差が改善。犬は5ケースのみで判定を行わなかった。
- [終盤方針の再評価](https://github.com/tokotoko7777/ahc-workspace/blob/17c77028d6ee998971a5d76e3a01dd16cbac79c6/problems/ahc008/experiments/20261122/article-capture/terry-cat-prior-terminal/RESULT.md)は保存済みの実測ラベルを選び直した比較で、均一な目的地からの予測より+46,388,889点相当・全頭+2。独立した試行組で安定して改善した判断は3件で、事前基準4件未達。得点版は6,152,805,559点・全頭100件のまま。

### AHC008: 停滞時の早期追い込みは全頭捕獲が増えたが不採用（2026-11-23 JST）

- [条件付き新規100ケースの検証](https://github.com/tokotoko7777/ahc-workspace/blob/b32be72d76a33271d7ef5c7dc52a8a231b30d9bb/problems/ahc008/experiments/20261122/article-capture/terry-stalled265/RESULT.md)。255〜265ターンで捕獲が進まなかった局面で、追い込みを265ターンから開始。全頭79→89件、残り23→14頭、ただし得点5,278,344,442→5,264,793,386（−13,551,056点）で、現行版のみ全頭捕獲のケースも5件。両版が全頭捕獲した74ケースでは、得点換算で作業者1人あたり平均約16.44マス減った。公式配布100や本番の順位との直接比較ではない。
- 現行ベストは公式配布100で **6,152,805,559点・全頭捕獲100件**。作業ブランチは `codex/ahc008-article-capture`。次は方針を変えず、追い込み試行で以降の得点が変わらないことを証明できる状態の早期終了を検証する。
- [再利用候補のC++23最大流（CSR Dinic）](https://github.com/tokotoko7777/ahc-workspace/blob/fc87aefa93c32c4aa16cd6f5cbbfc1d30441215a/problems/ahc008/experiments/20261122/article-capture/terry-chase-csr/library-candidate/README.md)に使用例とテストを添付。共通ライブラリと得点版には未導入。

### AHC008: 得点不変の仮想追い込み打ち切りを採用（2026-11-23 JST）

- 最多の作業者がいる連結領域から動物が消えると、その仮想試行で元の追い込みは以後ずっと待機する。壁・連結領域・人ごとの到達可能な動物数が変わらないため、仮想試行の得点をその場で計算して打ち切る。実際の方針や動物のモデルは変えない。
- [同じ試行数での対応比較](https://github.com/tokotoko7777/ahc-workspace/blob/79c20f92769c4dbc154bd04c97867db4d0f51046/problems/ahc008/experiments/20261122/article-capture/terry-virtual-chase-absorb/FIXED_RESULT.md): 配布100／別生成100の全行動・得点・判断が一致。CPU −2.86%／−4.23%。
- [通常2670msの比較](https://github.com/tokotoko7777/ahc-workspace/blob/79c20f92769c4dbc154bd04c97867db4d0f51046/problems/ahc008/experiments/20261122/article-capture/terry-virtual-chase-absorb/NATIVE_RESULT.md): 両方の100で全行動と得点が一致、完了試行670→698／1001→1102。配布100は **6,152,805,559点・全頭100件**、別生成100は5,998,857,137点・全頭97件。高速版を研究の基準に採用し点数記録は据え置き。mainへのマージや本番提出はなし。

### AHC008: 全頭捕獲を維持しながら小さい囲い・省ける設営柵を検証（2026-11-23 JST）

- [Map1の9→3マス](https://github.com/tokotoko7777/ahc-workspace/blob/25420f4bd41033e3a5ae061b365e7961b764db49/problems/ahc008/experiments/20261122/article-capture/terry-map1-remote-cap/RESULT.md): 配布100のMap1で形まで一致した大きい囲いは128回・153頭。配布／別生成のMap1全44ケースを完全再現し、4ターン前の情報だけで先回りを選んだ。元の捕獲を保ったのは各1件、各+4マス。役割の競合と元の捕獲を失う案があり、実戦評価せず見送り。
- [終盤の設営以外の追加柵](https://github.com/tokotoko7777/ahc-workspace/blob/a145d9ddeefced51cea0b4b93b52b8199a99069f/problems/ahc008/experiments/20261122/article-capture/terry-terminal-nonx-bound/RESULT.md): 旧100で全頭捕獲した99件の設営柵と最終位置を固定。後から追加柵を理想的に外せると仮定しても1人あたり平均+0.14マスで、実戦で柵を外せるという意味ではない。
- [省いた柵の場所を外側の足場にする案](https://github.com/tokotoko7777/ahc-workspace/blob/0fa80b7f38ae725aff04be0b23f19032659bde55/problems/ahc008/experiments/20261122/article-capture/terry-flank-valve/RESULT.md)は旧100で対象798局面、内側に柵を置ける321局面のうち既存の担当を守って届く場面は0。[設営済み各区画の状態](https://github.com/tokotoko7777/ahc-workspace/blob/d58dd5740bd57927dc08cd0e0b1ab4e1d978a622/problems/ahc008/experiments/20261122/article-capture/terry-x-lifecycle/RESULT.md)も確認し、設営柵25,808枚すべてで設置直前に両口または片口が全て開いた罠が少なくとも一つ関連していた。「既に全て役目を終えた」という条件で省ける柵は0。
- 次は、終盤の大きい囲いの閉鎖を小さい囲いに置き換えられる局面と、設営時の情報だけで柵1枚を保留する条件を別々に検証。ベストは公式配布100の **6,152,805,559点・全頭捕獲100件**、作業ブランチは `codex/ahc008-article-capture`。本番順位と同条件ではない。

### AHC008: 柵1枚の省略を実戦の判断で追試（2026-11-23 JST）

- [固定命令での省略](https://github.com/tokotoko7777/ahc-workspace/blob/fa6309c96728afc1f2592ce298629fb5b811647c/problems/ahc008/experiments/20261122/article-capture/terry-x-exact-replay/RESULT.md): 旧100は公式ジャッジで元の応答30,000行を完全再現。設営1枚を省いた82件はすべて後の固定命令で動物上または隣に柵を置こうとして失敗。省略後に判断し直す実装の評価ではない。
- [実際の応答で毎ターン判断し直す比較](https://github.com/tokotoko7777/ahc-workspace/blob/724ed75c703648f0263a9f07f2f37e0df2a8e3fb/problems/ahc008/experiments/20261122/article-capture/terry-x-once-live/RESULT.md): 公式配布100の全200実行が合法・独立採点一致、設営を省いた81件のうち43件は全頭捕獲かつ1人あたり+1マス以上。全体の全頭は100→98件、得点は6,152,805,559→6,070,247,621、共通全頭98件の合計は−274.95マスのため不採用。別seedは未生成。候補の最大実測は2.7992秒。
- [終盤の大きい囲い19件](https://github.com/tokotoko7777/ahc-workspace/blob/7cd23e2da514d56ed547b69701888fd5a46f5835/problems/ahc008/experiments/20261122/article-capture/terry-late-small-body/RESULT.md): 保存済みの実際の行動を再現し、同ターン・2手・3手前からの準備を比較したが、同じ動物を隔離できる合法な4マス以下の置換は0。フォローは係数変更でなく、設営をいったん保留し、必要時に戻せる条件を調べる。基準版とベストは据え置き。

### AHC008: 三通路の片側を幅4にするMap1配置（2026-11-23 JST）

- [盤面の静的検証](https://github.com/tokotoko7777/ahc-workspace/blob/04af5c85cce5cedd7d78d1acbbea35a3b1fd65d8/problems/ahc008/experiments/20261122/article-capture/terry-map1-hybrid14/RESULT.md): 三通路のまま片側に両端で閉める1×4を14個、反対側の囲いは維持。予定設営柵290→265枚。細い囲いを全部閉めても、その側と外周はつながる。
- [最初の動物なし設営比較](https://github.com/tokotoko7777/ahc-workspace/blob/d28bf7051d9fe67cc7f88a93e4224a511a8ea269/problems/ahc008/experiments/20261122/article-capture/terry-map1-hybrid14/DRY_RESULT.md): 5/6人の同じ初期位置36件で、160ターンまでの全設営＋配置が現行36/36、新配置21/36（5人6/18、6人15/18）。両方が完了したケースの新配置遅れは中央値34、最大46ターン。
- [下側にも出口を追加した配置](https://github.com/tokotoko7777/ahc-workspace/blob/3623ba4807fcd7157c81648f7833bb7ea2588d68/problems/ahc008/experiments/20261122/article-capture/terry-map1-hybrid14-gateway/RESULT.md)は設営柵264枚、全65囲い／細い囲い14。外周南側の作業位置から通路まで8〜24歩短くなった。[別の36件で設営比較](https://github.com/tokotoko7777/ahc-workspace/blob/a65a161e3c98612ecb0f4275bd6205d9560985fd/problems/ahc008/experiments/20261122/article-capture/terry-map1-hybrid14-gateway/DRY_RESULT.md)は現行36/36、新配置35/36、中央値13・最大24ターン遅れで、事前基準12・20を満たさなかった。設営順の変更だけでこの配置を採用せず、盤面に依存しない柵の割り当てと経路設計を別途検証中。
- どちらも実戦採点なし。ベストは公式配布100 **6,152,805,559点・全頭捕獲100件** のまま、作業ブランチ `codex/ahc008-article-capture`。

### AHC008: 狭い捕獲の担当と閉鎖形状の追加調査（2026-11-23 JST）

- [上下反転の動物なし比較](https://github.com/tokotoko7777/ahc-workspace/blob/d77a67d761c3ee350627f90b5075bd9aece330c4/problems/ahc008/experiments/20261122/article-capture/terry-map1-reflect/RESULT.md): 36件中11件で担当配置を遅らせず、小さい片口の囲いが早く使えた。[判定の高速化](https://github.com/tokotoko7777/ahc-workspace/blob/b64eb27c2e55a1f66e02a6485475d4c98855663c/problems/ahc008/experiments/20261122/article-capture/terry-map1-reflect/RADIUS3_SCOUT_RESULT.md)でも判断は全件一致したが、起動時の中央値186.9msが180msの事前上限を超え、実戦採点なし。
- [未割当だけを補う設営](https://github.com/tokotoko7777/ahc-workspace/blob/b4ad5d2eeb10e9ebef043dcd36c44acc6c94ce63/problems/ahc008/experiments/20261122/article-capture/terry-map1-gateway-localized/RESULT.md): 下出口案は新しい36件すべてで壁と指定通路への配置が完了したが、中央値+13・最大+22ターンで基準12・20に未達。 [壁担当を全面的に引き直す方式](https://github.com/tokotoko7777/ahc-workspace/blob/a354fb48713a5eb910a61a92b82cec022b091d48/problems/ahc008/experiments/20261122/article-capture/terry-stock-route-cover/RESULT.md)は既存地形48件で歩数が15,511→36,617に増え、不採用。
- [同じ担当者で小さい囲いに替える案](https://github.com/tokotoko7777/ahc-workspace/blob/9a3e94b2121a96010f5d3172e2700a60b19638fa/problems/ahc008/experiments/20261122/article-capture/terry-samecrew-compact/RESULT.md)は元の100件を再現、物理的候補203件・56盤面、独立した予測で支持0。 [待機位置の重複を解く案](https://github.com/tokotoko7777/ahc-workspace/blob/61cd903ca20d44f1b7cfa32a6a158306493878ff/problems/ahc008/experiments/20261122/article-capture/terry-idle-spacing/RESULT.md)は50盤面で近づけたが、既存作業を保った3手以内の追加捕獲0。いずれも実戦の得点改善を示さない。
- [両口1×4と片口L字3マスの局所配置](https://github.com/tokotoko7777/ahc-workspace/blob/d2469261b682d700a01d712d402f4cbe8a11a5f9/problems/ahc008/experiments/20261122/article-capture/terry-mixed-l-packet/RESULT.md)は設営20枚のまま4囲いと両方の偶奇を維持し、捕獲時の入口8→6、囲う面積16→14。ただし通常の通り抜け4→2。 [30行への展開](https://github.com/tokotoko7777/ahc-workspace/blob/c2a422484f920cfbd319fde9bf75e0995a3e5b3f/problems/ahc008/experiments/20261122/article-capture/terry-mixed-bank-band/RESULT.md)は省けたように見えた柵1枚が、誰も入れない1マスに置き換わっていただけで不採用。
- 既存の囲いで、出口側にいる別の動物だけが入口を塞ぐときの内寄せを次に確認。公式配布100の基準版 **6,152,805,559点・全頭捕獲100件** を維持。

### AHC008: 終盤の狭い切り直しと柵の省略

- 採用版は変更なし。公式配布100ケースは **6,152,805,559点・全頭捕獲100件**、独立生成100ケースは 5,998,857,137点・全頭捕獲97件。目標 6,263,333,336 との差 110,527,777。配布ケースの数値比較で、本番と同じケースではない。
- [Hall 制約を利用した終盤の切り直し](https://github.com/tokotoko7777/ahc-workspace/blob/e438ff189f9465e36a0339fb2c9b8a508c8ce006/problems/ahc008/experiments/20261122/article-capture/terry-chase-hall/RESULT.md)：現在版の Chase 23盤面・575状態を照合し、同じ動物を隔離できる割当可能な候補を追加で5件（4盤面）発見。盤面ごとの最良改善は計18マスで基準50未満。実プレイ未変更。
- 全頭捕獲以降に全員待機しても得点は不変（壁の追加で人の領域は広がらない）。現行版と同じ署名済みの公式配布100・独立生成100の行動列で確認済み。配布での初回全頭隔離は中央値228ターン。全頭隔離前に一部の人だけが動物のいない成分にいた場面を、取得済み60記録で調べたところ6ターンで、その成分での壁設置は0。後者は map1 と終盤寄りの調査で、全200件への一般化はしない。
- Map2/Map3 を途中まで共通設営する案：盤面の厳密な対称変換8通りで、最大共通141柵。それだけで完成する区画は Map2 が0、Map3 が片口14・両口0。両方で同じ配置として使える区画はなく、既存2配置の分岐案は見送り。次は配置の共通化より、片口・両口の役割を同一路線で切り替えられる構造を検討する。

### AHC008: 動物の移動で狭い区画に入るまで

- 採用版・点数は変更なし。公式配布100は 6,152,805,559／全頭100、独立生成100は 5,998,857,137／全頭97。目標差 110,527,777。
- [一人で二口を順に閉鎖する形](https://github.com/tokotoko7777/ahc-workspace/blob/7e521d3ff40a8022e4185207b70118eb2a125477/problems/ahc008/experiments/20261122/article-capture/terry-one-guard-dual-fallback/RESULT.md)：4マス以下では豚の両色とウサギに対して確実な開始位置を両立できない。6マスは可能だが、既存Map1の14区画を周囲の柵を維持したまま置換できず。
- [実際のMap1捕獲直前を起点にした診断](https://github.com/tokotoko7777/ahc-workspace/blob/5ab8fd920684f93c167c7a3e111c500592e76d40/problems/ahc008/experiments/20261122/article-capture/terry-map1-passage/RESULT.md)：9マスで捕獲された牛・豚・ウサギ計95頭に対し、他の動物や担当移動を省いて計算しても、25ターン以内に4マス以下の未閉鎖区画の安全位置へ到達する確率が80%以上のものはゼロ。中央値でなく平均は配布28.3%／独立30.0%。大きい捕獲を一律に待つ根拠なし。
- [完工盤面の幾何比較](https://github.com/tokotoko7777/ahc-workspace/blob/966dcc726e45d87ffe5a35df8218cec80a31d578/problems/ahc008/experiments/20261122/article-capture/terry-passive-opportunity-atlas/RESULT.md) でMap3の豚の弱い偶奇を確認。3マス区画8つの片口を建設時に塞ぐと、静的25ターン到達率の弱い色の下位10%値が14.0%→37.9%。[静的検証](https://github.com/tokotoko7777/ahc-workspace/blob/5ab8fd920684f93c167c7a3e111c500592e76d40/problems/ahc008/experiments/20261122/article-capture/terry-map3-pig-parity-caps/RESULT.md) は通過。しかし動物なし36件で、柵の追加を末尾に回すと配備遅れ中央値+12.5／最大+22、[元の道中に組み込む](https://github.com/tokotoko7777/ahc-workspace/blob/ac05cc1256dd5af1d3d4fd3940209877274ae95b/problems/ahc008/experiments/20261122/article-capture/terry-map3-caps-splice/RESULT.md) 案も +12／+22。上限+10／+20とも超過したため両案不採用。公式採点は実施せず。

### AHC008: 幅3の通り抜け区画と共用入口

- 採用版は変わらず、公式配布100ケースで6,152,805,559点・全頭100件。独立生成100ケースで5,998,857,137点・全頭97件。目標差110,527,777点。[基準・検証索引](https://github.com/tokotoko7777/ahc-workspace/blob/codex/ahc008-article-capture/problems/ahc008/experiments/20261122/article-capture/README.md)。
- [片側に入口を寄せる](https://github.com/tokotoko7777/ahc-workspace/blob/8843976e890a7370d853893a72c52dbc59fb2253/problems/ahc008/experiments/20261122/article-capture/terry-map3-staggered-same-end/RESULT.md)案は独立に二人で閉められる3マス区画を柵数増加なしで1か所確認。通り抜けが1本減り、片側に二人を要する。別の [Y字型の共用入口](https://github.com/tokotoko7777/ahc-workspace/blob/93211892ba8757021cecab4dd8f5027928bf9ae2/problems/ahc008/experiments/20261122/article-capture/terry-shared-y-edge/RESULT.md) は5×9上端パッチで3＋3マスを一人で閉められるが設営1枚増、両方に動物がいても最終面積は同じ。片側だけなら人側の床が4マス減る。
- [幅3で両側に出入口を残す配置の全列挙と上界](https://github.com/tokotoko7777/ahc-workspace/blob/c0465c724e0a3cbf35773c570e375dde1dcf6656/problems/ahc008/experiments/20261122/article-capture/terry-axial3-packing/RESULT.md)：高さ30、設営44枚のとき、15区画なら豚の両方の偶奇に対応できるのは最大3、14区画なら最大7。枚数制限なしならそれぞれ最大3／9。両偶奇の13区画には45枚が必要。この幅と役割に限る。
- [14区画の固定候補](https://github.com/tokotoko7777/ahc-workspace/blob/a2d4a91fc7a374b8d202bcdbbbee9c7fa492b3cf/problems/ahc008/experiments/20261122/article-capture/terry-map3-width3-14/TRAFFIC_RESULT.md) はMap3全体で74区画・233枚を検証。静的25ターンの豚の全体平均63.6%→68.2%、弱い色の下位10%値14.0%→34.0%だが、弱い色の平均60.7%で基準65%未満。動物入り設営・採点には進めず。
- [既存の終盤比較で乱数を対応づける試験](https://github.com/tokotoko7777/ahc-workspace/blob/3f5c38daf6f9ae66780b4764ed0dbf7ace66c4ec/problems/ahc008/experiments/20261122/article-capture/terry-virtual-crn/RESULT.md) は記録22件の仮想比較。分散比の中央値0.911が基準0.90を超え、平均分散も2.2%悪化。実プレイは変更なし。

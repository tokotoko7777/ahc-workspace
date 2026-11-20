# AHC 過去問研究の進捗

対象は終了済み AHC001〜AHC074。原則として番号順に着手し、長期戦や比較条件待ちの回はベストと次の仮説を保存して先へ進む。公式解説・参加者解説・第三者の提出コードは参照しない。

## 1 位超えの判定

公式最終 1 位を、同じ対象ケース、公式の採点・丸め・集計・有効性条件で**厳密に**上回った場合のみ「達成」。相対評価は両者に同一のケース別基準を使う。本番相当を再現できなければ「未判定」。同点や別シードでの改善を達成と混同しない。

| 問題 | 状態 | 本番最終 1 位 | 現在の基準値 | 比較条件 | 問題別ブランチ |
|---|---|---|---|---|---|
| [AHC001](https://atcoder.jp/contests/ahc001/tasks/ahc001_a) | 本番 1,000 件完走・未達 | **993,823,071,683** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc001.zip)） | 凍結 v64: **993,411,282,631** 点 | 本番公開シード全 1,000 件・公式採点。最長 4.838 秒（ローカル） | [codex/ahc001-coordinate-slide-20261120](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc001-coordinate-slide-20261120) |
| [AHC002](https://atcoder.jp/contests/ahc002/tasks/ahc002_a) | 既存版から着手・未判定 | **6,255,977** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc002.zip)） | V10（未調整 20 件 1,254,587） | 本番 100 件。通常配布の 0–99 は別ケース | [codex/ahc002-corridor-repair-20261120](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc002-corridor-repair-20261120) |
| [AHC003](https://atcoder.jp/contests/ahc003/tasks/ahc003_a) | 本番 3,000 件完走・未達 | **2,926,767,743,437** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc003.zip)） | 凍結既存版: **2,925,781,453,112** 点 | 本番公開シード全 3,000 件・公式対話テスタ・最大ローカル 0.897 秒 | [codex/ahc003-probe-design-20261120](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc003-probe-design-20261120) |
| [AHC004](https://atcoder.jp/contests/ahc004/tasks/ahc004_a) | ローカル改善・本番未判定 | **7,321,305,762** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc004.zip)） | 周期パターン初期化: 未調整 10 件 681,224,023（旧 662,753,350） | 本番 100 件。通常配布の 0–99 は別ケース | [codex/ahc004-contig-seed-20261120](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc004-contig-seed-20261120) |
| [AHC005](https://atcoder.jp/contests/ahc005/tasks/ahc005_a) | ローカル比較・本番未判定 | **22,318,770** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc005.zip)） | 既存版: 未調整 8 件 1,761,160 | 通常配布の 0–99 は本番 100 件と別 | [codex/ahc005-route-splice-20261120](https://github.com/tokotoko7777/ahc-workspace/tree/codex/ahc005-route-splice-20261120) |
| [AHC006](https://atcoder.jp/contests/ahc006/tasks/ahc006_a) | ローカル比較・本番未判定 | **2,220,425** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc006.zip)） | 既存版: 未調整 10 件の合計 214,630 | 本番 100 件。配布シードとの同一性未確認 | [空間初期化の分岐](https://github.com/tokotoko7777/ahc-workspace/tree/761d6839f93d0e347820919714402360766c4ad5) |
| [AHC007](https://atcoder.jp/contests/ahc007/tasks/ahc007_a) | 着手・未判定 | **14,241,733,941** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc007.zip)） | シナリオ版: 公開 150 件 **14,184,305,386** | 本番 150 件、2 秒。配布入力との同一性未確認 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/2793a9a782184c4b77376fab17482d24b353d6bd) |
| [AHC008](https://atcoder.jp/contests/ahc008/tasks/ahc008_a) | ローカル改善・本番未判定 | **121,980,050,607** 点（[AtCoder 提供の順位データ](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc008.zip)） | 適応分割版: 未調整 24 件合計 53,656,251 | 公式対話テスタ、本番公開 2,000 シード、3 秒 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/af41b4e70a89fef3c0389c689aa09dc633fffa16) |
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
| [AHC032](https://atcoder.jp/contests/ahc032/tasks/ahc032_a) | 着手・本番未判定 | **11,854,207,127,046**（絶対点） | 未調整16件 平均79,106,180,582.44 | 本番150件、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/0f8a8d2946ce1c6691db7ce95eee5ffa8a51006e)、[方向の追試](https://github.com/tokotoko7777/ahc-workspace/tree/890cee4f0a8b60a076d3c98d6d8d6bffed43099) |
| [AHC033](https://atcoder.jp/contests/ahc033/tasks/ahc033_a) | 着手・本番未判定 | bowwowforeach、得点未確認 | 新規未調整24件 合計6,867ターン | 本番2,000件、相対方式 | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/131ef411f491cc1b6ed79c905fc98a8d52ffad5d)、[先行搬出](https://github.com/tokotoko7777/ahc-workspace/tree/81b3ab227a32818d6d4350351c5d4db5c9602b0f) |
| [AHC034](https://atcoder.jp/contests/ahc034/tasks/ahc034_a) | 着手・本番未判定 | **9,486,350,384**（絶対点） | 積み降ろし再配分：新規未調整24件 平均35,727,578.04 | 本番150件・2秒、同一ケースの証拠なし | [問題別ブランチ](https://github.com/tokotoko7777/ahc-workspace/tree/6c2ea91bb86ef24c6c84cd99bf76df6355c16ce6)、[積み降ろし再配分](https://github.com/tokotoko7777/ahc-workspace/tree/e2c29731322790e744a17033d3fa937c450cede7) |
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
| [AHC052](https://atcoder.jp/contests/ahc052/tasks/ahc052_a) | 着手・本番未判定 | 確認中 | 開発比較準備中 | 確認中 | 検証中 |
| [AHC053](https://atcoder.jp/contests/ahc053/tasks/ahc053_a) | 着手・本番未判定 | 確認中 | 開発比較準備中 | 確認中 | 検証中 |
| AHC054〜074 | 未着手 | — | — | — | — |

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

[追試](https://github.com/tokotoko7777/ahc-workspace/tree/890cee4f0a8b60a076d3c98d6d8d6bffed43099)。幅3,500の1方向に対し、幅1,750ずつを対向する2方向に使い、同じ入力で候補を比較。評価回数比は未調整で平均1.00291。未調整16件の平均は79,089,863,247.81→79,109,974,914.63だが5勝2分9敗・中央値約−895万、devも平均悪化。既定は1方向。全64出力が公式・独立採点で一致し最大1.395991秒。次は終盤で手数を使い切れるよう、手数別に候補を残す。

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

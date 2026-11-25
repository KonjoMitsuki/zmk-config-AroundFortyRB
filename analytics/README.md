# Keyboard Analytics — イベント・時間・遷移のPoC

ZMK `v0.3.0` 向けの外部Zephyrモジュールです。**現在はイベント正確性、押下タイミング、キー遷移、直近の押下順を記録するPoC**で、仕様書のMVPは未実装です。

- 新しい位置イベントの生成関数だけをGNU linkerの `--wrap` で観測します。
- Central上で位置別のlocal／peripheral押下数、発生元別の解放イベント数、不正位置数を集計します。
- 押下時間、同一キーの押下間隔、直前の押下からの間隔を、位置別の有界ヒストグラムで記録します。
- 指定した最大16組のペアについて、押下開始間隔と押下の重なりをCC／PP／CP別に記録します。
- 全位置の「Aの次にB」を順序付きで数え、キー遷移の頻度を残します。
- 直近1024回の押下を、連番・時刻・位置・発生元付きでRAMに保持します。
- カウンタは`uint32_t`、ヒストグラムと遷移回数は`uint16_t`で飽和し、飽和フラグを立てます。解放では押下数を増やしません。
- ホットパスは定数時間のRAM更新だけです。イベントの引数と戻り値をそのまま引き継ぎます。
- 明示的に有効化した場合だけ、位置統計・ヒストグラムとキー遷移を2世代のsettingsレコードへ保存し、起動時に復元します。
- `CONFIG_ZMK_ANALYTICS` はデフォルトOFF。Peripheralでは有効にできません。

BLE取得、認可、停止／再開、リセット、ショートカット集計はまだありません。**Windows向けの認可なしUSB開発モード**を追加しました。[Windows実機確認手順](WINDOWS-USB-POC.md)に沿って、デバッガーなしで集計値の取得と明示保存ができます。USB開発モードも保存もデフォルトOFFです。保存の設計と障害時の扱いは[PERSISTENCE.md](PERSISTENCE.md)を参照してください。普段使いへの導入条件は、[仕様書](../Data/docs/zmk-keyboard-analytics-spec-v0.3.md)と[レビュー・検証記録](../Data/docs/zmk-keyboard-analytics-poc.md)を参照してください。

## 記録する内容

| データ | 粒度 | 再起動後 |
|---|---|---|
| 押下数・解放数・不正位置 | 位置×発生元、発生元別合計 | 残る |
| 押下時間 | 位置×8ビン（20/40/60/80/100/200/500ms） | 残る |
| 同一キー間隔 | 位置×8ビン（50/100/150/200/300/500/1000ms） | 残る |
| 直前押下からの間隔 | 位置×8ビン（同上） | 残る |
| 指定ペアのN1／N2 | 最大16組×CC/PP/CP×6ビン（20/40/60/80/100ms） | 残る |
| 有向2連 | 全位置のA→B、`uint16_t` | 残る |
| 直近の押下順 | 1024件、連番・時刻・位置・発生元・レイヤーID | 消える（RAMのみ） |
| 品質カウンタ | 重複down、孤立release、時刻破棄、ペア設定エラー | 残る |

ペアのN1は、直前の押下から250ms未満で始まった異なる位置の押下です。N2はN1のうち、直前のキーが押されたままのものです。位置の順序は問いません。ペアは`CONFIG_ZMK_ANALYTICS_PAIR_LIST`で指定し、既定は`0:1`です。

有向2連では、`counts[A][B]`が位置Aから位置Bへの連続押下の回数を表します。A→BとB→Aは別に数えます。3連以上や実際の並びは、再起動で消える直近順序から調べます。

## ビルド

### GitHub Actions

`build.yaml`は通常版の右・左・settings_resetに加え、Analytics対応の右側版を別名で生成します。push、pull request、手動実行で起動する通常のビルドworkflowを使います。

ビルドが成功したら、実行結果の **Artifacts → firmware** をダウンロードし、`AroundForty-RB_R-analytics-usb-poc.uf2` を選んでください。この版は集計・専用USB読出し・永続保存を有効にし、Studio USBと状態LEDも含みます。通常版のUF2ではAnalyticsは無効です。左側には通常の左側用UF2を使います。

読出しスクリプトは同じコミットの `analytics/tools/` にあります。導入と読出しは[Windows実機確認手順](WINDOWS-USB-POC.md)を参照してください。`Analytics PoC` workflowは回帰試験用で、UF2の配布先ではありません。

### ローカルビルド

`config/west.yml` の依存モジュールを取得済みのwest workspaceと、ZMK v0.3.0対応SDKを使います。次の変数は各環境の絶対パスへ置き換えてください。

```sh
REPO=/absolute/path/to/zmk-config-AroundFortyRB
ZMK=/absolute/path/to/workspace/zmk
```

west workspace内で、右側のPoCをビルドします。

```sh
west build -p always -s "$ZMK/app" -d build/analytics-right \
  -b seeeduino_xiao_ble -S studio-rpc-usb-uart -- \
  -DZMK_CONFIG="$REPO/config" \
  -DSHIELD="AroundForty-RB_R rgbled_adapter" \
  -DZMK_EXTRA_MODULES="$REPO;$REPO/analytics" \
  -DCONFIG_ZMK_ANALYTICS=y \
  -DCONFIG_ZMK_ANALYTICS_PAIR_LIST="0:1,1:2,8:9"
```

リポジトリ直下もZephyrモジュールです。状態LEDのbehaviorとKconfigを含むため、`ZMK_EXTRA_MODULES` の先頭へ `$REPO` を加えます。GitHub Actionsの設定リポジトリでは、`zephyr/module.yml` があるため自動で追加されます。

`CONFIG_ZMK_ANALYTICS_PAIR_LIST`はカンマ区切りの位置ペアです。順序は問わず、同じペアの重複は1組にまとめます。不正な項目と17組目以降は`pair_config_errors`に数えて読み飛ばします。この値は起動時の設定なので、保存値へ累積しません。

OFF版は別のbuild directoryで `-DCONFIG_ZMK_ANALYTICS=n` を指定します。モジュールの追加だけでは計測は有効になりません。既に `ZMK_EXTRA_MODULES` を指定している環境では、そのリストへセミコロン区切りで追加してください。

`cmake/audited-sources.cmake` の監査対象ファイルが異なる場合はビルドを拒否します。上流更新時には生成・再投入経路を再確認し、回帰試験後にハッシュを更新します。外部モジュールが独自に位置イベントを生成する構成も別途監査が必要です。LTOは無効にしてください。

## ホストでの検証

Cコンパイラ、CMake、CTestだけで境界・飽和処理を検証できます。

```sh
cmake -S analytics/tests -B build/analytics-unit -G Ninja
cmake --build build/analytics-unit
ctest --test-dir build/analytics-unit --output-on-failure
```

`collector`、`record`、`timing`の3試験が走ります。保存レコードv1/v2のroundtrip、全バイトの破損検出、ペア再マップ、ヒストグラム境界、重複down、孤立release、飽和、有向2連、直近順序のリング動作を含みます。USBプロトコルのJSON検証はPythonだけで実行できます。

```sh
python -B -m unittest discover -s analytics/tests -p test_read_poc.py -v
```

プロトコルversion 1〜3、timing／qualityの形、bigram行列、recent順序の検証9件が走ります。実機は不要です。

実際のZMKイベント管理・Combo・hold-tapを通す試験は、依存を取得済みのwest workspaceで実行します。Zephyr v3.5系の `native_posix_64` とホストGCCを使います。

```sh
python analytics/tests/run_native.py \
  --workspace /absolute/path/to/workspace \
  --zmk /absolute/path/to/workspace/zmk \
  --build-dir /absolute/path/to/analytics-native-results --jobs 2
```

ZMKのCombo試験25件について、生成元の押下数との一致と既存HID／behaviorスナップショットとの一致を検証します。`results.json` と合成入力のログを指定先へ出力します。`--case press-release` 等で対象を限定できます。

発生元と同一タイムスタンプ、4種類の再投入経路は別のprobeで検証します。

```sh
ZEPHYR_TOOLCHAIN_VARIANT=host west build -p always \
  -s "$ZMK/app" -d build/analytics-source-probe -b native_posix_64 -- \
  -DZMK_CONFIG="$REPO/analytics/tests/source-probe" \
  -DZMK_EXTRA_MODULES="$REPO/analytics;$REPO/analytics/tests/source-probe" \
  -DCONFIG_ZMK_ANALYTICS=y -DCONFIG_ASSERT=y
timeout 20s build/analytics-source-probe/zephyr/zmk.exe
```

probeの成功時は `PASS: new local/peripheral events...` を出力します。これはPeripheral発生元を模擬した試験であり、実際のBLE転送や切断を検証するものではありません。

ZephyrパッケージをCMakeに登録していない環境では、ビルド引数へ `-DZephyr_DIR=/absolute/path/to/workspace/zephyr/share/zephyr-package/cmake` を追加します。SDK 0.16.8とPython 3.12で確認済みです。Studioビルドの旧nanopb生成器には `setuptools<81` と `grpcio-tools` が必要でした。

保存処理は、native_posix_64の実Zephyr settings／NVSとシミュレートしたフラッシュファイルを使い、プロセス再起動をまたいで検証します。新しい出力先を指定してください。

```sh
python analytics/tests/run_storage.py \
  --workspace /absolute/path/to/workspace \
  --zmk /absolute/path/to/workspace/zmk \
  --build-dir /absolute/path/to/new-storage-test-directory
```

保存、再読込み、再読込みの重複加算防止、有向2連の復元、物理配置ID不一致、片側破損からの復旧、両側破損時の上書き禁止、書込み失敗時の旧世代維持、無関係なsettingsキーの保持を確認します。

実機では2026-11-04に、世代2を保存後に未保存のQ×3を作り、右側を電源OFF/ONして、同じ世代・同じカウンタへ復元されることを確認しました。保存中の電断、NVSのGC、書換寿命、長期運用は未検証です。再起動をまたぐ比較は [`tools/watch_reboot.py`](tools/watch_reboot.py) で自動化できます。

## USB開発モードでの取得

専用CDCへ1行のコマンドを送り、1行のJSONを受け取ります。StudioのCOMポートとは別です。

| コマンド | 内容 |
|---|---|
| `GET` | 押下数、品質カウンタ、timingヒストグラム |
| `SAVE` | RAM集計をキーボードのフラッシュへ保存 |
| `BIGRAMS` | 全位置の有向2連 |
| `RECENT` | 直近1024件の押下順 |

```sh
py analytics/tools/read_poc.py --port COM7
py analytics/tools/read_poc.py --port COM7 --bigrams
py analytics/tools/read_poc.py --port COM7 --recent --recent-limit 100
py analytics/tools/read_poc.py --port COM7 --save
```

`--bigrams`は遷移回数の多い順に上位30件を行合計に対する比率付きで表示します。`--recent`は古い順に、連番・起動後の経過ms・位置・発生元を表示します。取得したJSONを残す場合は`--out trial-01.json`を追加します。同名ファイルは上書きしません。

## ブラウザで打鍵データを分析する

[dashboard/index.html](dashboard/index.html) をブラウザで開きます。読み込んだデータはブラウザ内で処理します。「サンプルを見る」で、手元にデータがなくても画面を試せます。

USBで直接取得するには、Analytics対応ファームウェアを使っているキーボードをPCにつなぎ、デスクトップ版のChromeまたはEdgeで「USBから取得」を押します。初回はStudioとは別のAnalytics用COMポートを選んでください。打鍵集計・キー遷移・直近の打鍵が画面に表示されます。同じページを開いている間は「USBから再取得」で同じポートを使えます。「取得結果を保存」を押すと、3種類のデータを1つのJSONにまとめて保存できます。

保存したデータは「JSONを開く」から選ぶか、画面にドロップして読み込みます。取得ツールが個別に出力したJSONも開けます。

打鍵集計（PoC v1〜v3）では、42キーの打鍵分布、ランキング、計測品質を確認できます。v3では、キーごとの押下時間や押下間隔、ファームウェアで指定したペアの分布も表示します。キー遷移と直近の打鍵には、それぞれ別のJSONが必要です。Windowsで3種類を取得する例です。ポート番号は実機に合わせてください。

```powershell
py analytics/tools/read_poc.py --port COM7 --out snapshot.json
py analytics/tools/read_poc.py --port COM7 --bigrams --out bigrams.json
py analytics/tools/read_poc.py --port COM7 --recent --out recent.json
```

3種類を同時に開けます。同じ種類のファイルを開き直すと、前のファイルを置き換えます。種類ごとに取得時点が異なるので、各画面の件数は同じ期間の集計とは限りません。配列IDのない42キーのファイルは、AroundForty RBの物理配列として表示します。Win Base／Mac Baseの切り替えは表示ラベルに適用されます。集計値は物理位置に対応します。

キーマップの変更を表示ラベルに反映するときは、リポジトリのルートで `python3 analytics/tools/generate_dashboard_keymap.py` を実行してください。

### 遷移の流量図とレイヤー別ヒートマップ

キー遷移の流量図では、回数が多いほど線が太くなります。矢印は押下順を示し、同じキーへの遷移は輪で表します。上位12組を表示し、遷移元を選ぶとそのキーから始まる組に絞れます。

新しい右側用の Analytics ファームウェアは、押下を受け付けた時点のレイヤー ID を記録します。その時点で有効なレイヤーのうち、そのキーに透過以外の割り当てがある最上位のレイヤーを採用します。長押し判定や Combo の処理で、そのあとに使われるレイヤーや出力が変わる場合があります。

更新後に「USBから取得」を押すと、直近の押下のレイヤー別ヒートマップを確認できます。長期履歴は、DBを更新して `analytics-dashboard.json` を開き直してください。更新前の記録はレイヤー不明として取り込み、全体の打鍵数には含めます。

## OneDriveに打鍵履歴を貯める

[構成図](architecture.html)では、各PCで記録する流れと、同期済みの記録からDBを更新する流れを確認できます。DB更新用の端末は、普段使っているPCのうち1台で構いません。

各端末の OneDrive で同じフォルダを同期してください。記録先の既定値は `OneDrive/AroundFortyAnalytics` です。個人用と組織用を使い分けている場合は、両端末で同じ共有フォルダを `--directory "C:\Users\名前\OneDrive\AroundFortyAnalytics"` のように明示します。このオプションは、下記のコマンドと両方のランチャーに指定できます。

1. Windows で [record_analytics.cmd](tools/record_analytics.cmd) をダブルクリックします。初回は Analytics 用 COM ポートを入力してください。成功すると、その端末にポート番号を記憶し、OneDrive の `captures` フォルダに取得記録を追加します。ポートが変わったら `record_analytics.cmd --port COM7` のように指定します。Python と `pyserial` が必要です（`py -3 -m pip install pyserial`）。
2. DB の更新に使う端末を1台決めます。各端末の取得記録が同期されたあと、その端末で [build_analytics_db.cmd](tools/build_analytics_db.cmd) をダブルクリックします。`analytics.sqlite3` が生成され、OneDrive でほかの端末にも同期されます。更新するたびに `captures` から DB を作り直すため、取得記録は残してください。

10秒ごとに記録する場合は、Windows の PowerShell で次を実行します。ポートは実機に合わせてください。接続エラーの間は再試行し、`Ctrl+C` で停止します。

```powershell
py -3 analytics/tools/record_recent.py capture --port COM7 --interval 10
```

Windowsのサインイン時に記録を始めるには、PowerShellで次を一度実行します。保存済みのCOMポートとOneDriveフォルダを使い、10秒ごとに記録するショートカットを作ります。必要に応じて `-Port COM7` と `-Directory "共有フォルダのパス"` を指定してください。停止するときは `shell:startup` から `AroundForty Analytics.lnk` を削除し、動作中の記録ウィンドウを閉じます。

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\analytics\tools\install_analytics_startup.ps1
```

各端末は別名の取得ファイルを作り、更新用の端末が完成した DB を OneDrive に置きます。DB は閲覧用として開いてください。以前の取得記録が同期されていない場合は、古い DB を残したまま更新を中止します。OneDrive の同期が済み、DB を開いているアプリを閉じた状態で再実行してください。

DB を更新すると、同じフォルダにダッシュボード用の `analytics-dashboard.json` も作成します。同期が済んだら、どの端末からでも [打鍵分析 GUI](dashboard/index.html) の「JSONを開く」でこのファイルを選べます。取得期間の打鍵分布とキー遷移が表示されます。最新の結果を見るときは、DB を更新してからファイルを開き直してください。

DB の `presses` に打鍵、`captures` に取得時刻、`segments` に履歴の区切り、`gaps` に区間内で確認できない連番の範囲が入ります。重なりが一致する取得記録は同じ区間にまとめます。再起動などでつながりを確認できない場合は区間を分けるため、区間をまたぐ連番の比較で欠落数を算出することはできません。ファームウェアに起動 ID がないため、区間の判定には限界があります。起動後の経過時間と、PC で取得した時刻は別々に記録します。

## データの意味と限界

PoCが数えるのは**生成元から到達したdownイベント**です。受信前に失われた打鍵、上流キュー溢れ、チャタリング、動的マッピング変更は追跡しません。切断時の合成解放も解放イベント数に含みます。カウンタを物理打鍵の完全な実測値と表示しないでください。

時刻をIDとして重複排除しないため、異なる押下が同じmsに到達しても両方を数えます。Comboが捕捉したイベントの再投入では生成関数を呼ばないため、押下数を増やしません。

押下時間と間隔は、Centralがイベントを受け取った時刻から計算します。Peripheral側のキーはBLE通知を受けた時刻であり、物理スイッチが接触した時刻ではありません。時刻が逆行した場合は、その1件を`timing_discards`に数えてヒストグラムへ加えません。ヒストグラムは16bitで飽和し、上限に達すると`histogram_saturated`が立ちます。有向2連も16bitで65535に達すると`saturated`が立ちます。長期運用で飽和した場合にカウントを半減するなどの対策は未実装です。

直近の押下順はRAMのリングバッファだけにあり、再起動で消えます。連番は押下ごとに増えるため、同じmsの押下でも順序を復元できます。生のイベント列をフラッシュへ永続保存する処理はありません。

集計値は、保存を有効にしない限り再起動で消えます。保存版でも、最後の正常保存以降の入力は電源断で失われ得ます。`zmk_analytics_poc_copy()` はファームウェア内部の診断用APIで、ホスト認可を提供するAPIではありません。`CONFIG_ZMK_ANALYTICS_POC_REPORT` による終了時出力は、BLE／USB無効のnative試験専用です。USB開発モードの `CONFIG_ZMK_ANALYTICS_USB_POC` は別機能であり、専用CDCを使い、StudioのUARTを共有しません。ユーザーの希望により、この明示的に有効化する開発モードに限ってアンロックを省略しています。MVPの認可要件を満たしたという意味ではありません。

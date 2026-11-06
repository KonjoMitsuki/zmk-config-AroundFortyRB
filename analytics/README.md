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

BLE取得、認可、停止／再開、リセット、レイヤー／ショートカット集計、ダッシュボードはまだありません。**Windows向けの認可なしUSB開発モード**を追加しました。[Windows実機確認手順](WINDOWS-USB-POC.md)に沿って、デバッガーなしで集計値の取得と明示保存ができます。USB開発モードも保存もデフォルトOFFです。保存の設計と障害時の扱いは[PERSISTENCE.md](PERSISTENCE.md)を参照してください。普段使いへの導入条件は、[仕様書](../Data/docs/zmk-keyboard-analytics-spec-v0.3.md)と[レビュー・検証記録](../Data/docs/zmk-keyboard-analytics-poc.md)を参照してください。

## 記録する内容

| データ | 粒度 | 再起動後 |
|---|---|---|
| 押下数・解放数・不正位置 | 位置×発生元、発生元別合計 | 残る |
| 押下時間 | 位置×8ビン（20/40/60/80/100/200/500ms） | 残る |
| 同一キー間隔 | 位置×8ビン（50/100/150/200/300/500/1000ms） | 残る |
| 直前押下からの間隔 | 位置×8ビン（同上） | 残る |
| 指定ペアのN1／N2 | 最大16組×CC/PP/CP×6ビン（20/40/60/80/100ms） | 残る |
| 有向2連 | 全位置のA→B、`uint16_t` | 残る |
| 直近の押下順 | 1024件、連番・時刻・位置・発生元 | 消える（RAMのみ） |
| 品質カウンタ | 重複down、孤立release、時刻破棄、ペア設定エラー | 残る |

ペアのN1は、直前の押下から250ms未満で始まった異なる位置の押下です。N2はN1のうち、直前のキーが押されたままのものです。位置の順序は問いません。ペアは`CONFIG_ZMK_ANALYTICS_PAIR_LIST`で指定し、既定は`0:1`です。

有向2連はダッシュボードの線の太さに使う集計で、`counts[A][B]`が位置Aから位置Bへの連続押下の回数です。A→BとB→Aは別に数えます。3連以上や実際の並びは、再起動で消える直近順序から調べます。

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

## データの意味と限界

PoCが数えるのは**生成元から到達したdownイベント**です。受信前に失われた打鍵、上流キュー溢れ、チャタリング、動的マッピング変更は追跡しません。切断時の合成解放も解放イベント数に含みます。カウンタを物理打鍵の完全な実測値と表示しないでください。

時刻をIDとして重複排除しないため、異なる押下が同じmsに到達しても両方を数えます。Comboが捕捉したイベントの再投入では生成関数を呼ばないため、押下数を増やしません。

押下時間と間隔は、Centralがイベントを受け取った時刻から計算します。Peripheral側のキーはBLE通知を受けた時刻であり、物理スイッチが接触した時刻ではありません。時刻が逆行した場合は、その1件を`timing_discards`に数えてヒストグラムへ加えません。ヒストグラムは16bitで飽和し、上限に達すると`histogram_saturated`が立ちます。有向2連も16bitで65535に達すると`saturated`が立ちます。長期運用で飽和した場合にカウントを半減するなどの対策は未実装です。

直近の押下順はRAMのリングバッファだけにあり、再起動で消えます。連番は押下ごとに増えるため、同じmsの押下でも順序を復元できます。生のイベント列をフラッシュへ永続保存する処理はありません。

集計値は、保存を有効にしない限り再起動で消えます。保存版でも、最後の正常保存以降の入力は電源断で失われ得ます。`zmk_analytics_poc_copy()` はファームウェア内部の診断用APIで、ホスト認可を提供するAPIではありません。`CONFIG_ZMK_ANALYTICS_POC_REPORT` による終了時出力は、BLE／USB無効のnative試験専用です。USB開発モードの `CONFIG_ZMK_ANALYTICS_USB_POC` は別機能であり、専用CDCを使い、StudioのUARTを共有しません。ユーザーの希望により、この明示的に有効化する開発モードに限ってアンロックを省略しています。MVPの認可要件を満たしたという意味ではありません。

# Keyboard Analytics — イベント正確性PoC

ZMK `v0.3.0` 向けの外部Zephyrモジュールです。**現在はイベント正確性と保存のPoC**で、仕様書のMVPは未実装です。

- 新しい位置イベントの生成関数だけをGNU linkerの `--wrap` で観測します。
- Central上で位置別のlocal／peripheral押下数、発生元別の解放イベント数、不正位置数を集計します。
- カウンタは`uint32_t`で飽和し、飽和フラグを立てます。解放では押下数を増やしません。
- ホットパスは定数時間のRAM更新だけです。イベントの引数と戻り値をそのまま引き継ぎます。
- 明示的に有効化した場合だけ、2世代のsettingsレコードへ保存し、起動時に復元します。
- `CONFIG_ZMK_ANALYTICS` はデフォルトOFF。Peripheralでは有効にできません。

BLE取得、認可、停止操作、リセット、レイヤー／ペア統計はまだありません。**Windows向けの認可なしUSB開発モード**を追加しました。[Windows実機確認手順](WINDOWS-USB-POC.md)に沿って、デバッガーなしで集計値の取得と明示保存ができます。USB開発モードも保存もデフォルトOFFです。保存の設計と障害時の扱いは[PERSISTENCE.md](PERSISTENCE.md)を参照してください。普段使いへの導入条件は、[仕様書](../Data/docs/zmk-keyboard-analytics-spec-v0.3.md)と[レビュー・検証記録](../Data/docs/zmk-keyboard-analytics-poc.md)を参照してください。

## ビルド

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
  -DZMK_EXTRA_MODULES="$REPO/analytics" \
  -DCONFIG_ZMK_ANALYTICS=y
```

OFF版は別のbuild directoryで `-DCONFIG_ZMK_ANALYTICS=n` を指定します。モジュールの追加だけでは計測は有効になりません。既に `ZMK_EXTRA_MODULES` を指定している環境では、そのリストへセミコロン区切りで追加してください。

`cmake/audited-sources.cmake` の監査対象ファイルが異なる場合はビルドを拒否します。上流更新時には生成・再投入経路を再確認し、回帰試験後にハッシュを更新します。外部モジュールが独自に位置イベントを生成する構成も別途監査が必要です。LTOは無効にしてください。

## ホストでの検証

Cコンパイラ、CMake、CTestだけで境界・飽和処理を検証できます。

```sh
cmake -S analytics/tests -B build/analytics-unit -G Ninja
cmake --build build/analytics-unit
ctest --test-dir build/analytics-unit --output-on-failure
```

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

保存、再読込み、再読込みの重複加算防止、物理配置ID不一致、片側破損からの復旧、両側破損時の上書き禁止、書込み失敗時の旧世代維持、無関係なsettingsキーの保持を確認します。

実機では2026-11-04に、世代2を保存後に未保存のQ×3を作り、右側を電源OFF/ONして、同じ世代・同じカウンタへ復元されることを確認しました。保存中の電断、NVSのGC、書換寿命、長期運用は未検証です。再起動をまたぐ比較は [`tools/watch_reboot.py`](tools/watch_reboot.py) で自動化できます。

## データの意味と限界

PoCが数えるのは**生成元から到達したdownイベント**です。受信前に失われた打鍵、上流キュー溢れ、チャタリング、動的マッピング変更は追跡しません。切断時の合成解放も解放イベント数に含みます。カウンタを物理打鍵の完全な実測値と表示しないでください。

時刻をIDとして重複排除しないため、異なる押下が同じmsに到達しても両方を数えます。Comboが捕捉したイベントの再投入では生成関数を呼ばないため、押下数を増やしません。生のイベント列をモジュール内に保存する処理はありません。

集計値は、保存を有効にしない限り再起動で消えます。保存版でも、最後の正常保存以降の入力は電源断で失われ得ます。`zmk_analytics_poc_copy()` はファームウェア内部の診断用APIで、ホスト認可を提供するAPIではありません。`CONFIG_ZMK_ANALYTICS_POC_REPORT` による終了時出力は、BLE／USB無効のnative試験専用です。USB開発モードの `CONFIG_ZMK_ANALYTICS_USB_POC` は別機能であり、専用CDCを使い、StudioのUARTを共有しません。ユーザーの希望により、この明示的に有効化する開発モードに限ってアンロックを省略しています。MVPの認可要件を満たしたという意味ではありません。

# 統計の保存・復元（実験中）

位置別のlocal／peripheral押下数、解放イベント数、不正位置数、飽和フラグをキーボード内に保存する機能です。レイヤー・ペア・ショートカットの統計や停止／リセット機能は、まだ含みません。

## 動作

- デフォルトは無効です。保存版は `CONFIG_ZMK_ANALYTICS_PERSISTENCE=y` を明示します。
- RAM上の集計を、変更がある場合だけ10分間隔で保存します。キー入力処理ではフラッシュへ書きません。
- USB開発モードの `SAVE` でも保存できます。変更なしなら再書込みせず、書込みの試行は最短10秒間隔に制限します。
- 起動時に最新の有効な保存世代を復元します。settingsの読込み前に到達したイベントも、復元値へ飽和加算して保持します。
- 突然の電源断では、最後の正常保存以降の未保存分を失い得ます。スリープ直前の保存は今回未実装です。
- 以前のRAM専用ファームウェア上の値は、この機能の導入時に自動移行されません。

## 保存形式と障害時の扱い

既存のZephyr settings／NVSを使い、専用キー `analytics_poc/a` と `analytics_poc/b` だけを交互に更新します。BluetoothやStudioの設定キーを削除したり、パーティションを再定義したりしません。

42位置で1レコード408バイトです。magic、保存スキーマ1、全長、位置数、世代番号、物理配置IDのSHA256、各カウンタ、CRC-32/ISO-HDLCをlittle-endianで保存します。C構造体のpaddingは保存しません。CRCは破損検出用で、認証機能ではありません。

- 最新側が破損していれば、有効な旧世代へ復旧して `recovered: true` を返します。
- 両側が存在しない初回は `no_valid_snapshot` です。最初の保存を許可します。
- 有効な世代がなく破損レコードがある場合は `error` とし、自動上書きを禁止します。
- 新しい有効世代の物理配置IDが違う場合は `incompatible_layout` とし、読込みも上書きも禁止します。古い配置の世代へ勝手に戻しません。
- 世代番号のUINT32上限では保存を拒否し、番号を0へ戻しません。
- settingsの保存エラーはRAM上の集計と旧世代を維持し、次回の保存で再試行します。エラーの内容と起動中の失敗回数を取得できます。

NVSが返す保存成功を採用します。実際のフラッシュ消去中の電断、共有領域の枯渇、書換寿命、入力遅延・電力の実測は別途必要です。二世代方式は独立した物理メディアへの冗長化ではありません。

## 物理配置ID

`persistence.conf` は `aroundforty-rb:matrix-v1` を指定します。SHA256の入力はこの文字列のUTF-8バイト列そのもので、改行や正規化は追加しません。

これは**手動管理のID**です。マトリクス変換・位置対応を変更する際はIDも変更してください。実際の配置変更を自動検出する仕組みではありません。複数の物理レイアウトを持つビルドは現在拒否します。
論理キーマップをStudioで変更しても、今回保存するのは物理位置統計だけです。論理割当の追跡・別世代管理を実装済みとは扱いません。

## ビルド

通常のUSB PoCビルドに、次の2ファイルを追加設定として渡します。詳細は [Windows USB手順](WINDOWS-USB-POC.md) を参照してください。

```sh
-DEXTRA_CONF_FILE="$REPO/analytics/usb-poc.conf;$REPO/analytics/persistence.conf" \
-DEXTRA_DTC_OVERLAY_FILE="$REPO/analytics/usb-poc.overlay"
```

## Windowsから取得・保存

USBシリアルポートはStudio用とは別です。以下のCOM7は例で、接続時の一覧から確認します。

```powershell
py -3.11 .\read_poc.py --list
py -3.11 .\read_poc.py --port COM7
py -3.11 .\read_poc.py --port COM7 --save
```

`--save` はPCのファイルではなく**キーボードのフラッシュへ保存**します。`--out trial.json` は取得したJSONをPCへ保存する別の操作です。
保存版の通信プロトコルはversion 2。クライアントは旧version 1も読めます。

`storage` の主なフィールド:

| フィールド | 意味 |
|---|---|
| `state` | `loading` / `no_valid_snapshot` / `ok` / `error` / `incompatible_layout` / `disabled` |
| `generation` | 最後に成功した保存、または起動時に採用した保存世代 |
| `dirty` | 現在のRAM集計がその保存世代と異なるか。保存世代がなければtrue |
| `error_count_session` | 今回の起動中の保存／復元エラー回数。再起動で0へ戻る |
| `last_error` | 最後の保存／復元エラー。正常保存時に0へ戻る |
| `recovered` | この起動時に、片方の破損を検出して有効なもう片方から復旧したか |

`persistent: true` は保存機能が有効という意味で、未保存分がないことの保証ではありません。`state` と `dirty` も確認してください。

## 実機確認の順序

1. 保存版を書き込み、GETでversion 2と保存状態を確認します。
2. キーを押してからSAVEし、`state: ok`、世代番号、`dirty: false` と対象キーの値を控えます。
3. 保存せずにキーを数回押し、未保存のRAM差分を作ります。
4. 右側を電源OFF/ONまたはリセットします。USBの抜き差しだけでは、バッテリー駆動のままRAMが残る場合があります。
5. キーボードを触らずGETし、同じ世代・保存済みの値が戻り、手順3の未保存分が消えていることを確認します。
6. キーを追加して10分以上待ち、自動保存で世代が進むことを確認します。
7. 通常入力・Combo・トラックボール・左右通信・Studioを使い、退行がないことを確認します。

`watch_reboot.py` を使うと、保存、COMポートの切断・再接続待ち、再起動後の比較を自動化できます。

```powershell
py -3.11 .\watch_reboot.py --port COM7 --timeout 600
```

実行中はキーボードを触らず、保存後にQを3回押して電源を切って入れ直します。`PASS: counters restored exactly at the same generation` が出れば成功です。

2026-11-04の実機確認では、世代2を保存後にQ×3を未保存のまま電源OFF/ONし、再起動後も世代2、`dirty: false`、位置0のPeripheral押下数8が保存時と一致しました。保存中の電断、NVSのGC、書換寿命、長期運用は未検証です。

書込み・再起動は利用者が行い、Windowsからのコマンド実行はこの環境のWSL側からも行えます。settings_resetやペアリングの消去は必要ありません。

## 自動試験

```sh
cmake -S analytics/tests -B build/analytics-unit -G Ninja
cmake --build build/analytics-unit
ctest --test-dir build/analytics-unit --output-on-failure
python -B -m unittest discover -s analytics/tests -p test_read_poc.py -v
python analytics/tests/run_storage.py \
  --workspace /absolute/path/to/workspace --zmk /absolute/path/to/workspace/zmk \
  --build-dir /absolute/path/to/new-storage-test-directory
```

storage試験はnative_posix_64の実際のZephyr settings／NVSを使い、シミュレートしたフラッシュファイルを引き継いでプロセスを再起動します。既存ファイルを誤って再利用しないよう、新しい出力ディレクトリだけを受け付けます。実機の設定領域には触れません。

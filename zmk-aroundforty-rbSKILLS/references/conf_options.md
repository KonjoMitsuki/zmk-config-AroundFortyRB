# AroundForty-RB .conf オプション参照

## どちらの .conf に書くか

| 設定カテゴリ | _R.conf（右/Central） | _L.conf（左/Peripheral） |
|---|---|---|
| トラックボール | ✅ 必須 | ❌ 不要 |
| ZMK Studio | ✅ 必須 | ❌ 不要 |
| RGB LED ウィジェット | ✅ | ✅ |
| BLE 安定性設定 | ✅ | ✅（同じ値を） |
| バッテリーレポート | ✅（Proxy/Fetching も） | ✅ |
| スリープ設定 | ✅ | ✅ |

---

## BLE 接続設定

```conf
# BLE を有効化（両方必須）
CONFIG_ZMK_BLE=y

# 実験的機能（macOS 安定性のため無効推奨）
CONFIG_ZMK_BLE_EXPERIMENTAL_FEATURES=n

# 接続間隔（1.25ms × 値）。12 = 15ms（応答性と安定性のバランス）
CONFIG_BT_PERIPHERAL_PREF_MIN_INT=12
CONFIG_BT_PERIPHERAL_PREF_MAX_INT=12

# TX 出力パワーを最大に（右側のみ。接続が切れやすい場合に有効）
CONFIG_BT_CTLR_TX_PWR_PLUS_8=y

# 接続数上限（左右ともに 6 推奨）
CONFIG_BT_MAX_CONN=6
CONFIG_BT_MAX_PAIRED=6

# BLE バッファサイズ（251 = DLE 最大値。安定性向上）
CONFIG_BT_BUF_ACL_RX_SIZE=251
CONFIG_BT_BUF_ACL_TX_SIZE=251

# RX/TX スタックサイズ
CONFIG_BT_RX_STACK_SIZE=2048
CONFIG_BT_HCI_TX_STACK_SIZE=1024

# 2M PHY を無効化（安定性優先）
CONFIG_BT_CTLR_PHY_2M=n
```

---

## スリープ・アイドル設定

```conf
# ディープスリープを無効化（接続が切れやすい場合は n 推奨）
CONFIG_ZMK_SLEEP=n

# アイドルタイムアウト（ms）。30000 = 30秒後に省電力モード
CONFIG_ZMK_IDLE_TIMEOUT=30000

# ディープスリープまでの時間（CONFIG_ZMK_SLEEP=y の場合のみ有効, ms）
CONFIG_ZMK_IDLE_SLEEP_TIMEOUT=900000
```

---

## トラックボール（PMW3610）設定（右側のみ）

```conf
# トラックボール/ポインティング有効化
CONFIG_ZMK_POINTING=y

# PMW3610 ドライバ有効化
CONFIG_PMW3610=y

# スマートアルゴリズム（ノイズ低減）
CONFIG_PMW3610_SMART_ALGORITHM=y

# レポート間隔（BLE 接続間隔と同期させる。15ms 推奨）
CONFIG_PMW3610_REPORT_INTERVAL_MIN=15

# ダウンシフト時間（ms）。静止後に省電力モードへ移行する時間
CONFIG_PMW3610_RUN_DOWNSHIFT_TIME_MS=3264
CONFIG_PMW3610_REST1_SAMPLE_TIME_MS=20
CONFIG_PMW3610_REST3_SAMPLE_TIME_MS=300

# 起動時の追加遅延（ms）。接続安定化のため
CONFIG_PMW3610_INIT_POWER_UP_EXTRA_DELAY_MS=1000
```

> **CPI（感度）の変更**: `.conf` ではなく `_R.overlay` の `cpi = <400>;` を編集する。

---

## ZMK Studio

```conf
# ZMK Studio 有効化（右側のみ）
CONFIG_ZMK_STUDIO=y

# Studio 接続時のロック無効化（開発中は n でOK）
CONFIG_ZMK_STUDIO_LOCKING=n
```

---

## RGB LED ウィジェット（両側）

```conf
CONFIG_RGBLED_WIDGET=y

# バッテリー残量が高いと判定する閾値（%）
CONFIG_RGBLED_WIDGET_BATTERY_LEVEL_HIGH=30

# バッテリー残量が危機的と判定する閾値（%）
CONFIG_RGBLED_WIDGET_BATTERY_LEVEL_CRITICAL=10

# レイヤー切り替え時に LED 表示（右側のみ有効）
CONFIG_RGBLED_WIDGET_SHOW_LAYER_CHANGE=y
```

---

## バッテリーレポート

```conf
# バッテリー残量レポート（両側）
CONFIG_ZMK_BATTERY_REPORTING=y
CONFIG_BT_BAS=y

# 右側（Central）のみ：左側のバッテリーを代理表示
CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_PROXY=y
CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y
```

---

## スレッド・スタックサイズ

```conf
# BLE 処理スレッド
CONFIG_ZMK_BLE_THREAD_STACK_SIZE=2048

# 低優先度タスク
CONFIG_ZMK_LOW_PRIORITY_THREAD_STACK_SIZE=2048

# Input subsystem
CONFIG_INPUT_THREAD_STACK_SIZE=2048

# Split Peripheral 側
CONFIG_ZMK_SPLIT_BLE_PERIPHERAL_STACK_SIZE=2048

# Split Central 側ランタイム（右側のみ）
CONFIG_ZMK_SPLIT_BLE_CENTRAL_SPLIT_RUN_STACK_SIZE=3096

# ヒープメモリ
CONFIG_HEAP_MEM_POOL_SIZE=16384
```

---

## その他ハードウェア設定

```conf
# NFC ピンを GPIO として使用（nRF52840 固有。両側必須）
CONFIG_NFCT_PINS_AS_GPIOS=y

# SPI バス（右側のみ、PMW3610 用）
CONFIG_SPI=y

# Input subsystem（両側）
CONFIG_INPUT=y
```

---

## トラブルシューティング早見表

| 症状 | 確認・変更すべき設定 |
|---|---|
| BLE が切れやすい | `CONFIG_BT_CTLR_PHY_2M=n`, `CONFIG_ZMK_BLE_EXPERIMENTAL_FEATURES=n` |
| 接続が遅い / ラグがある | `CONFIG_BT_PERIPHERAL_PREF_MIN_INT=12` と `MAX_INT=12` を確認 |
| トラックボールがカクカクする | `CONFIG_PMW3610_REPORT_INTERVAL_MIN` を BLE 間隔と同期させる |
| 起動直後にトラックボールが反応しない | `CONFIG_PMW3610_INIT_POWER_UP_EXTRA_DELAY_MS` を増やす |
| バッテリー残量が表示されない | `CONFIG_ZMK_BATTERY_REPORTING=y` と `CONFIG_BT_BAS=y` を両側に追加 |
| Studio に接続できない | `CONFIG_ZMK_STUDIO=y`（右側のみ）、`build.yaml` に `snippet: studio-rpc-usb-uart` があるか確認 |
| スリープ後に接続が戻らない | `CONFIG_ZMK_SLEEP=n` に設定 |

# 左右confの実値

確認: 2026-10-07、main 000496ebf。以下は宣言値でありビルド後のeffective .configではない。Kconfigの依存・default・重複設定を確認する。未設定は機能の無効を意味しない。

| symbol | R | L |
|---|---|---|
| `CONFIG_BT_BAS` | `y` | `y` |
| `CONFIG_BT_BUF_ACL_RX_SIZE` | `251` | `251` |
| `CONFIG_BT_BUF_ACL_TX_SIZE` | `251` | `251` |
| `CONFIG_BT_CTLR_PHY_2M` | `n` | `n` |
| `CONFIG_BT_CTLR_TX_PWR_PLUS_8` | `y` | `未設定` |
| `CONFIG_BT_DEVICE_NAME` | `"AroundFortyRB"` | `"AroundFortyRB"` |
| `CONFIG_BT_DEVICE_NAME_MAX` | `16` | `16` |
| `CONFIG_BT_HCI_TX_STACK_SIZE` | `1024` | `1024` |
| `CONFIG_BT_MAX_CONN` | `6` | `6` |
| `CONFIG_BT_MAX_PAIRED` | `6` | `6` |
| `CONFIG_BT_PERIPHERAL_PREF_MAX_INT` | `12` | `12` |
| `CONFIG_BT_PERIPHERAL_PREF_MIN_INT` | `12` | `12` |
| `CONFIG_BT_RX_STACK_SIZE` | `2048` | `2048` |
| `CONFIG_HEAP_MEM_POOL_SIZE` | `未設定` | `16384` |
| `CONFIG_INPUT` | `y` | `y` |
| `CONFIG_INPUT_THREAD_STACK_SIZE` | `2048` | `2048` |
| `CONFIG_NFCT_PINS_AS_GPIOS` | `y` | `y` |
| `CONFIG_PMW3610` | `y` | `未設定` |
| `CONFIG_PMW3610_INIT_POWER_UP_EXTRA_DELAY_MS` | `1000` | `未設定` |
| `CONFIG_PMW3610_REPORT_INTERVAL_MIN` | `15` | `未設定` |
| `CONFIG_PMW3610_REST1_SAMPLE_TIME_MS` | `20` | `未設定` |
| `CONFIG_PMW3610_REST3_SAMPLE_TIME_MS` | `300` | `未設定` |
| `CONFIG_PMW3610_RUN_DOWNSHIFT_TIME_MS` | `3264` | `未設定` |
| `CONFIG_PMW3610_SMART_ALGORITHM` | `y` | `未設定` |
| `CONFIG_RGBLED_WIDGET` | `y` | `y` |
| `CONFIG_RGBLED_WIDGET_BATTERY_LEVEL_CRITICAL` | `10` | `10` |
| `CONFIG_RGBLED_WIDGET_BATTERY_LEVEL_HIGH` | `30` | `30` |
| `CONFIG_RGBLED_WIDGET_SHOW_LAYER_CHANGE` | `y` | `未設定` |
| `CONFIG_SPI` | `y` | `未設定` |
| `CONFIG_ZMK_BATTERY_REPORTING` | `y` | `y` |
| `CONFIG_ZMK_BLE` | `y` | `y` |
| `CONFIG_ZMK_BLE_EXPERIMENTAL_FEATURES` | `n` | `n` |
| `CONFIG_ZMK_BLE_THREAD_STACK_SIZE` | `2048` | `2048` |
| `CONFIG_ZMK_IDLE_TIMEOUT` | `30000` | `30000` |
| `CONFIG_ZMK_KEYBOARD_NAME` | `"AroundFortyRB"` | `"AroundFortyRB"` |
| `CONFIG_ZMK_LOW_PRIORITY_THREAD_STACK_SIZE` | `2048` | `2048` |
| `CONFIG_ZMK_POINTING` | `y` | `未設定` |
| `CONFIG_ZMK_SLEEP` | `n` | `n` |
| `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING` | `y` | `未設定` |
| `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_PROXY` | `y` | `未設定` |
| `CONFIG_ZMK_SPLIT_BLE_CENTRAL_SPLIT_RUN_STACK_SIZE` | `3096` | `未設定` |
| `CONFIG_ZMK_SPLIT_BLE_PERIPHERAL_STACK_SIZE` | `2048` | `2048` |
| `CONFIG_ZMK_STUDIO` | `y` | `未設定` |
| `CONFIG_ZMK_STUDIO_LOCKING` | `n` | `未設定` |

接続間隔の宣言は12×1.25ms=15ms。ただしホストとのnegotiation結果を保証しない。idle timeout 30000は30秒、sleep=nとは区別する。TX power +8はRのみ宣言。最高出力や大バッファを一般に最適と断定しない。Studioは実行時keymapを保存できるため、実機の配置がrepoと異なる可能性を確認する。

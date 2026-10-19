# CLIP STUDIO PAINT レイヤー

このレイヤーは Windows の JIS 配列を前提にしています。左手側の22キーに希望の22機能を割り当てています。図は各段の左から右への物理順です。

| 段 | 左手側の物理順 |
| --- | --- |
| 1 | Undo `Ctrl+Z` / Redo `Ctrl+Y` / 変形 `Ctrl+T` / 新規ラスターレイヤー `Ctrl+Shift+N` / 回転リセット `F13` |
| 2 | `Left Ctrl` / ブラシ `B` / 消しゴム `E` / スポイト `I` / 透明色 `C` |
| 3 | `Left Shift` / 選択範囲 `M` / 回転 `R` / `Ctrl+Alt` / `Ctrl+Space` / 下のレイヤー `Alt+[`（長押しでSettings） |
| 4 | `Delete` / 選択解除 `Ctrl+D` / `Left Alt` / `Enter` / キャンバス移動 `Space` / 上のレイヤー `Alt+]` |

左3段目の中央側端キー `L3C0` は、Win-Baseではタップで `PRINTSCREEN`、長押しでSettingsです。従来の長押し先Num/ScrollをSettingsへ変更しました。CLIP_STUDIO中は同じキーをタップで `Alt+[`、長押しでSettingsにします。約200ms長押ししてからSettingsの `L1C3` を押すと `&tog` でCLIP_STUDIOのON/OFFが切り替わります。キーを離すとSettingsが解除されます。SettingsはCLIP_STUDIOより高いレイヤーです。

`Ctrl`、`Shift`、`Alt`、`Space`、`Ctrl+Alt`、`Ctrl+Space`、`R` はキーを押している間、押下状態が続き、離すと解除されます。`Ctrl+Alt` と `Ctrl+Space` はマクロによる一瞬の入力ではありません。

CLIP STUDIO PAINT側では、キャンバス回転リセットに `F13` を割り当ててください。ほかの機能も実際のショートカット設定と照合し、表と異なる場合は表の組み合わせに設定してください。レイヤー上下移動の `Alt+[` / `Alt+]` はJISホスト向けの `JP_LBKT` / `JP_RBKT` を使っています。Mac-Baseは既存の `mac_ime` キーを維持しており、このページのCtrl系・JIS記号系の割り当てはMac向けには調整していません。

現行ファームウェアは右手がsplit central、左手がperipheralです。左手だけをPCに接続して使うことはできません。左手側のキーだけでCLIPの操作とモード切替をする場合も、右手側の電源と接続が必要です。

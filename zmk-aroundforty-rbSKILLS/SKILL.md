---
name: zmk-aroundforty-rb
description: >
  AroundForty-RB キーボード専用の ZMK ファームウェア編集スキル。
  キーマップの変更・レイヤー編集・.conf 設定の追加や変更を行う際に必ず使うこと。
  「キーを変えたい」「レイヤーを追加したい」「BLE の設定を変えたい」「トラックボールの速度を変えたい」
  「スクロール感度を調整したい」「スリープ設定を変えたい」といったあらゆる ZMK 設定変更に適用すること。
  座標指定（例: P1C3 のキーを変えて）によるキーマップ編集に対応している。
---

# ZMK AroundForty-RB スキル

## このスキルの使い方

1. ユーザーがやりたいことを確認（キーマップ変更 / .conf 変更 / どちらも）
2. 座標システムでキー位置を特定（→ `references/layout.md`）
3. 編集対象ファイルを特定して変更を提案・適用

---

## 対象ファイル構成

| ファイル | 役割 |
|---|---|
| `config/AroundForty-RB.keymap` | 全レイヤーのキーマップ |
| `config/boards/shields/AroundForty-RB/AroundForty-RB_R.conf` | 右手側（Central）設定 |
| `config/boards/shields/AroundForty-RB/AroundForty-RB_L.conf` | 左手側（Peripheral）設定 |
| `config/boards/shields/AroundForty-RB/AroundForty-RB_R.overlay` | 右手 DTS オーバーレイ（トラックボール設定） |
| `config/west.yml` | 依存モジュール管理 |

---

## 座標システム

キーの位置は `PxCy` 形式で指定する（x=行, y=列）。  
詳細な座標マップと現在のキー割り当ては **`references/layout.md`** を参照。

### 座標クイックリファレンス（物理配置）

```
行1（最上段）:  [P1C1][P1C2][P1C3][P1C4][P1C5]  |||  [P1C6][P1C7][P1C8][P1C9][P1C10]
行2:           [P2C1][P2C2][P2C3][P2C4][P2C5]  |||  [P2C6][P2C7][P2C8][P2C9][P2C10]
行3:      [P3C1][P3C2][P3C3][P3C4][P3C5][P3C6] ||| [P4C8][P3C7][P3C8][P3C9][P3C10][P3C11]
行4:      [P4C1][P4C2][P4C3][P4C4][P4C5][P4C6] |||       [P4C9][P4C7]    [P4C10][P4C11]
                                                    ^^^注: P4C8は行3の右中央に物理配置
```

> P4C8（`[`）は行4ではなく**行3の右中央付近**に物理的に配置されている点に注意。

---

## キーマップ変更の手順

### 1. 対象座標を特定する

ユーザーが「Qキーを変えたい」→ P1C1。  
ユーザーが座標を直接指定している場合はそのまま使用。

### 2. 対象レイヤーを特定する

現在のレイヤー構成（詳細は `references/layout.md`）:

| レイヤー | 名前 | 説明 |
|---|---|---|
| 0 | Win-Base | Windows JIS 基本レイヤー |
| 1 | Mac-Base | macOS US 基本レイヤー |
| 2 | Win-Fnc | Windows 記号・マウス・修飾 |
| 3 | Mac-Fnc | Mac 記号・マウス・修飾 |
| 4 | Win-Common | Windows ナビゲーション |
| 5 | Mac-Common | Mac ナビゲーション |
| 6 | Num_Scroll | 数字・ファンクション（Win JIS） |
| 7 | Mac_Num_Scroll | 数字・ファンクション（Mac US） |
| 8 | V_Scroll | 縦スクロール専用 |
| 9 | Settings | BT 選択・リセット・ブートローダー |
| 10 | AML | トラックボール操作レイヤー |

### 3. keymap ファイルのバインディング構造を理解する

`.keymap` の各レイヤーは `bindings = < ... >;` 内に **行順で左から右へ** キーが並ぶ。  
物理配置との対応は `references/layout.md` の「プログラム上の行→座標マッピング」を参照。

```
行1: 10キー（左5 + 右5）
行2: 10キー（左5 + 右5）
行3: 11キー（左6 + 右5）
行4: 11キー（左6 + 右5）  ※ うち空白2つあり
```

### 4. 変更を記述する

#### よく使う ZMK バインディング構文

```c
&kp KEY              // 通常キー入力
&lt LAYER KEY        // ホールドでレイヤー、タップでキー
&mt MOD KEY          // ホールドでモディファイア、タップでキー
&mo LAYER            // ホールド中のみレイヤー有効
&to LAYER            // レイヤー切り替え（固定）
&mkp MB1/MB2/MB3     // マウスボタン
&msc SCRL_UP/DOWN    // スクロール
&trans               // 下のレイヤーに透過
&kp LC(A)            // Ctrl+A（LC=Left Ctrl）
&kp LG(C)            // Cmd+C（LG=Left GUI）
&kp LS(TAB)          // Shift+Tab
```

#### カスタムビヘイビア（このキーボード固有）

```c
&mt_a_ctrl LCTRL A   // A キーの hold-tap（長めのtapping-term）
&mt_z_shift LEFT_SHIFT Z  // Z キーの hold-tap
&lt_num LAYER KEY    // quick-tap 付き hold-tap（Enter 連打向け）
&swapper             // Alt+Tab スワッパー（tri-state）
&mac_ime             // macOS IME 切り替え（Ctrl+Space）
&ime_tog             // Windows IME 切り替え（Alt+Grave）
```

#### JIS 記号マクロ（Win 用）

```c
JP_LT JP_GT JP_CARET JP_PRCNT JP_DLLR
JP_LPAR JP_RPAR JP_AT JP_AMPS JP_DQT
JP_COLON JP_LBKT JP_RBKT JP_EXCL JP_QMARK
JP_SQT JP_YEN JP_HASH JP_MINUS JP_PLUS
JP_AST JP_EQUAL JP_SEMI JP_UNDER JP_TILDE
JP_PIPE JP_GRAVE JP_LBRC JP_RBRC
```

---

## .conf 変更の手順

**どちらの .conf を変えるか**を先に確認する：
- トラックボール・Studio・ZMK Studio → **_R.conf（右/Central）**
- BLE 安定性・バッテリー → **両方**に同じ設定

詳細なオプション一覧は **`references/conf_options.md`** を参照。

### よく変更する設定（クイックリファレンス）

```conf
# トラックボール速度（CPI）→ _R.overlay の cpi = <400>; を変更
# スクロール感度 → _R.overlay の zip_scroll_scaler を変更
CONFIG_PMW3610_REPORT_INTERVAL_MIN=15   # レポート間隔（ms）
CONFIG_ZMK_POINTING=y                   # トラックボール有効化（R のみ）
CONFIG_ZMK_SLEEP=n                      # ディープスリープ無効
CONFIG_ZMK_IDLE_TIMEOUT=30000           # アイドルタイムアウト（ms）
CONFIG_BT_PERIPHERAL_PREF_MIN_INT=12    # BLE 接続間隔（1.25ms×12=15ms）
CONFIG_BT_PERIPHERAL_PREF_MAX_INT=12
CONFIG_ZMK_STUDIO=y                     # ZMK Studio 有効（R のみ）
CONFIG_RGBLED_WIDGET=y                  # RGB LED ウィジェット有効
```

---

## トラックボール設定（.overlay）

トラックボールの動作は `AroundForty-RB_R.overlay` で設定する。

```dts
trackball: trackball@0 {
    cpi = <400>;   // ← ここで感度変更（推奨: 200〜800）
    ...
};
```

スクロール感度（`zip_scroll_scaler`）:
```dts
<&zip_scroll_scaler 1 45>   // 分子 分母 → 小さいほど速い（45→30で速くなる）
```

X 軸反転（`INPUT_TRANSFORM_X_INVERT`）は現在有効。反転を戻す場合は削除。

---

## 編集後の確認チェックリスト

- [ ] `bindings` のキー数が各レイヤーで正しいか（42キー）
- [ ] `trans` で埋めるべき空位置が漏れていないか
- [ ] カスタムマクロ（`JP_*`）は `#include` 前に `.keymap` 先頭で定義されているか
- [ ] GitHub Actions でビルドが通るか（push 後に確認）

---

## 参照ファイル

- **`references/layout.md`** — 座標マップ、全レイヤーの現在のキー割り当て、バインディング行→座標変換表
- **`references/conf_options.md`** — .conf オプション全リストと説明

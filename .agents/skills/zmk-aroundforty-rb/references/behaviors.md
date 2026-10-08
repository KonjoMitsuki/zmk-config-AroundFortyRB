# カスタムbehaviorと落とし穴

確認版: 000496ebf、2026-10-07。定義はconfig/AroundForty-RB.keymapを再確認する。

| label | 設定・意味 |
|---|---|
| mt | balanced、quick-tap-ms=0。標準mtのoverride |
| mt_a_ctrl | kp/kp、balanced、250ms、quick-tap=0 |
| mt_z_shift | kp/kp、balanced、250ms、quick-tap=0 |
| lt_num | mo/kp、150ms、quick-tap=150ms |
| lt_typ | mo/kp、tap-preferred、200ms、require-prior-idle=150ms |
| settings_hold_tap | mo/aml_toggle_td、balanced、200ms、quick-tap=0 |
| lt_to_layer_0 | mo/to_layer_0、200ms。利用箇所を検索してから変更 |
| swapper | zmk-tri-state。kt LALT → kp TAB → kt LALT |
| mac_ime | Ctrlを押しSpaceをtap、Ctrlを離す |
| ime_tog | Altを押しGraveをtap、Altを離す。定義と使用箇所を区別 |
| aml_off | to 0、tog 11。全layer解除後にAML-Offを有効化 |
| aml_toggle_td | tap-dance 300ms。1回=aml_off、2回=to 0 |
| Escape combo | positions 0,1（P1C1+P1C2）でESCAPE |

Win P2C3は`&lt_typ 6 D`、Macは`&lt_typ 7 D`。
Win P4C7は`&lt_num 6 ENTER`だがMacは`&lt_num 10 ENTER`。
これは現行実装の差であり、MacもNumの7へ自動修正しない。
P4C10は`&settings_hold_tap 9 0`。第2引数0は通常キーコードではなく、
tap側tap-danceを呼ぶbinding引数。tap/holdとsingle/double tapを別々に検証する。

aml_offもaml_toggle_tdのdouble tapもWin baseへ戻す。
Mac modeを保持する処理ではない。改善案ではこの副作用を明示する。

keymap内JP_RBKTはNON_US_HASH、JP_RBRCはLS(NON_US_HASH)。
include/dt-bindings/zmk/keys_ja.hはBACKSLASHを使う等の差がある。
inline defineが実際に使われるため、別headerの一覧を正としない。

# カスタムbehaviorと落とし穴

確認版: 23966d9、2026-10-14。定義はconfig/AroundForty-RB.keymapを再確認する。

| label | 設定・意味 |
|---|---|
| mt | balanced、quick-tap-ms=0。標準mtのoverride |
| mt_a_ctrl | kp/kp、balanced、250ms、quick-tap=0 |
| mt_z_shift | kp/kp、balanced、250ms、quick-tap=0 |
| lt_num | mo/kp、150ms、quick-tap=150ms |
| lt_typ | mo/kp、tap-preferred、200ms、require-prior-idle=150ms |
| settings_hold_tap | mo/aml_toggle_td、balanced、200ms、quick-tap=0 |
| mac_settings_hold_tap | mo/mac_aml_toggle_td、balanced、200ms、quick-tap=0 |
| lt_to_layer_0 | mo/to_layer_0、200ms。利用箇所を検索してから変更 |
| swapper | zmk-tri-state。kt LALT → kp TAB → kt LALT |
| mac_ime | Ctrlを押しSpaceをtap、Ctrlを離す |
| ime_tog | Altを押しGraveをtap、Altを離す。定義と使用箇所を区別 |
| aml_off | to 0、tog 11。全layer解除後にAML-Offを有効化 |
| aml_toggle_td | tap-dance 300ms。1回=aml_off、2回=to 0 |
| mac_aml_off | to 1、tog 11。Mac baseでAML-Offを有効化 |
| mac_aml_toggle_td | tap-dance 300ms。1回=mac_aml_off、2回=to 1 |
| Escape combo | positions 0,1（L1C5+L1C4）でESCAPE |

Win L2C3は`&lt_typ 6 D`、Macは`&lt_typ 7 D`。
R4C2はWinが`&lt_num 6 ENTER`、Macが`&lt_num 7 ENTER`。
R4C3はWinが`&settings_hold_tap 9 0`、Macが`&mac_settings_hold_tap 12 0`。
第2引数0は通常キーコードではなく、
tap側tap-danceを呼ぶbinding引数。tap/holdとsingle/double tapを別々に検証する。

Win側のaml_off/aml_toggle_tdはWin base（0）へ戻し、
Mac側のmac_aml_off/mac_aml_toggle_tdはMac base（1）へ戻す。
Settings（9）/Mac-Settings（12）のL3C1でWin、L4C1でMacへ切り替える。

keymap内JP_RBKTはNON_US_HASH、JP_RBRCはLS(NON_US_HASH)。
include/dt-bindings/zmk/keys_ja.hはBACKSLASHを使う等の差がある。
inline defineが実際に使われるため、別headerの一覧を正としない。

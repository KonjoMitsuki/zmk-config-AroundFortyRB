# 構造と実装の案内

確認: 2026-10-07、main `000496ebfc86823ff2c5f797dcfded358e1243bf`。
取得元: https://github.com/KonjoMitsuki/zmk-config-AroundFortyRB/tree/000496ebfc86823ff2c5f797dcfded358e1243bf
以下はスナップショット。現在のcheckoutを優先する。

| 対象 | 現行パス |
|---|---|
| 全keymap・behavior・macro・combo | config/AroundForty-RB.keymap |
| kscan・transform・physical layout | config/boards/shields/AroundForty-RB/AroundForty-RB.dtsi |
| R/L overlay・conf・Kconfig | config/boards/shields/AroundForty-RB/ |
| モジュール | config/west.yml |
| build matrix | build.yaml |
| CI | .github/workflows/build.yml |
| 物理表ツール・対応表 | tools/zmk_keymap_physical.py、tools/around_forty_rb_mapping.json |
| 旧表生成ツール | scripts/generate_keymap_table.py |
| 既存の説明用表 | keymap.md |

Boardはseeeduino_xiao_ble。Kconfig.defconfigでRはsplit central、Lはsplit peripheral。
物理layoutとtransformは42エントリ。右overlayのcol-offsetは6。

| index | node名 | 機能 |
|---|---|---|
| 0 | Win-Base | Windows JIS |
| 1 | Mac-Base | macOS US |
| 2 | Win-Fnc | Win記号・mouse・shortcut |
| 3 | Mac-Fnc | Mac記号・mouse・shortcut |
| 4 | Win-Common | Win navigation |
| 5 | Mac-Common | Mac navigation |
| 6 | Num_Scroll | Win数字・記号・scroll |
| 7 | Mac_Num_Scroll | Mac数字・記号・scroll |
| 8 | V_Scroll | 垂直scroll |
| 9 | Settings | OS、BT、reset、Studio |
| 10 | AML | 一時mouse layer |
| 11 | AML-Off | 全transのAML抑止フラグ |

display-nameがないnodeもある。番号はnode順から読む。
Settingsからの`&to 0`/`&to 1`をOS切替と解釈するが、
`&to`は他のactive layerを解除する副作用もある。

| west project | remote | revision |
|---|---|---|
| zmk | zmkfirmware | v0.3.0 |
| zmk-pmw3610-driver | badjeff | zmk-0.3 |
| zmk-rgbled-widget | caksoylar | v0.3-branch |
| zmk-tri-state | dhruvinsh | main |

モジュールのbranchは可変。診断時にはwest manifestの解決済commitを記録する。

# 物理座標とbinding index

確認: 2026-10-14、main `23966d9`。現行keymap・physical attrs・編集用テンプレートと照合済み。

`L` / `R`は左手 / 右手、続く数字は上からの段、`C`の数字は各段の実在キーを中央から外側へ数えた順。空白は数えない。
左手は図の右から左へ、右手は左から右へ番号が増える。Cの番号は各段で数え直すため、上下の同じ番号が同じx座標とは限らない。

| 段 | 左手 | 右手 |
|---|---|---|
| 1 | L1C1〜L1C5 | R1C1〜R1C5 |
| 2 | L2C1〜L2C5 | R2C1〜R2C5 |
| 3 | L3C1〜L3C6 | R3C1〜R3C6 |
| 4 | L4C1〜L4C6 | R4C1〜R4C4 |

座標の確認先は`keymap-physical-template.md`。生成ツールは`tools/around_forty_rb_mapping.json`の`rows`（物理図）と`binding_order`（0始まりindex順）を参照する。
indexはkeymapのphandle binding順。座標の段からソース上の行を推測しない。
R3C1は物理3段目の右中央だがbinding index 38、R4C1は39、R4C2は37。

| index | 座標 | Win-Base | Mac-Base |
|---|---|---|---|
| 0 | L1C5 | `&kp Q` | `&kp Q` |
| 1 | L1C4 | `&kp W` | `&kp W` |
| 2 | L1C3 | `&kp E` | `&kp E` |
| 3 | L1C2 | `&kp R` | `&kp R` |
| 4 | L1C1 | `&kp T` | `&kp T` |
| 5 | R1C1 | `&kp Y` | `&kp Y` |
| 6 | R1C2 | `&kp U` | `&kp U` |
| 7 | R1C3 | `&kp I` | `&kp I` |
| 8 | R1C4 | `&kp O` | `&kp O` |
| 9 | R1C5 | `&kp P` | `&kp P` |
| 10 | L2C5 | `&mt_a_ctrl LCTRL A` | `&mt_a_ctrl LCTRL A` |
| 11 | L2C4 | `&kp S` | `&kp S` |
| 12 | L2C3 | `&lt_typ 6 D` | `&lt_typ 7 D` |
| 13 | L2C2 | `&kp F` | `&kp F` |
| 14 | L2C1 | `&kp G` | `&kp G` |
| 15 | R2C1 | `&kp H` | `&kp H` |
| 16 | R2C2 | `&kp J` | `&kp J` |
| 17 | R2C3 | `&kp K` | `&kp K` |
| 18 | R2C4 | `&kp L` | `&kp L` |
| 19 | R2C5 | `&lt 8 MINUS` | `&lt 8 MINUS` |
| 20 | L3C6 | `&mt_z_shift LEFT_SHIFT Z` | `&mt_z_shift LEFT_SHIFT Z` |
| 21 | L3C5 | `&kp X` | `&kp X` |
| 22 | L3C4 | `&kp C` | `&kp C` |
| 23 | L3C3 | `&kp V` | `&kp V` |
| 24 | L3C2 | `&kp B` | `&kp B` |
| 25 | L3C1 | `&lt 6 PRINTSCREEN` | `&mac_ime` |
| 26 | R3C2 | `&kp N` | `&kp N` |
| 27 | R3C3 | `&kp M` | `&kp M` |
| 28 | R3C4 | `&kp COMMA` | `&kp COMMA` |
| 29 | R3C5 | `&kp DOT` | `&kp DOT` |
| 30 | R3C6 | `&lt 10 SLASH` | `&lt 10 SLASH` |
| 31 | L4C6 | `&kp LCTRL` | `&kp LCTRL` |
| 32 | L4C5 | `&kp LGUI` | `&kp LEFT_ALT` |
| 33 | L4C4 | `&kp LALT` | `&kp LEFT_GUI` |
| 34 | L4C3 | `&mt LSHFT LANGUAGE_1` | `&mt LSHFT LANGUAGE_1` |
| 35 | L4C2 | `&lt 2 SPACE` | `&lt 3 SPACE` |
| 36 | L4C1 | `&lt 4 LANGUAGE_2` | `&lt 5 LANGUAGE_2` |
| 37 | R4C2 | `&lt_num 6 ENTER` | `&lt_num 7 ENTER` |
| 38 | R3C1 | `&kp JP_AT` | `&kp LEFT_BRACKET` |
| 39 | R4C1 | `&kp BSPC` | `&kp BSPC` |
| 40 | R4C3 | `&settings_hold_tap 9 0` | `&mac_settings_hold_tap 12 0` |
| 41 | R4C4 | `&kp DEL` | `&kp DEL` |

42 bindingのソース上の区切りは10/10/11/11。物理図の段ごとのキー数は10/10/12/10。
matrix row/col、binding index、人が使う物理座標は別の表現。transformは既存の順序を維持する。
row4のtransformはRC(3,0)..RC(3,5), RC(3,9), RC(3,7), RC(3,8), RC(3,6), RC(3,10)。

指の目安はtools/around_forty_rb_mapping.jsonを読み、実際の運指は本人に確認する。

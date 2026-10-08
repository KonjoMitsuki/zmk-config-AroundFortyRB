# 物理座標とbinding index

確認: 2026-10-07、main 000496ebf。現行transformとmappingで再確認する。

indexは0始まり、順番はkeymapのphandle binding順。P4C8は物理3行目右中央、P4C9は右親指側、P4C7はその右。見た目の空白はキーではない。

| index | 座標 | Win-Base | Mac-Base |
|---|---|---|---|
| 0 | P1C1 | `&kp Q` | `&kp Q` |
| 1 | P1C2 | `&kp W` | `&kp W` |
| 2 | P1C3 | `&kp E` | `&kp E` |
| 3 | P1C4 | `&kp R` | `&kp R` |
| 4 | P1C5 | `&kp T` | `&kp T` |
| 5 | P1C6 | `&kp Y` | `&kp Y` |
| 6 | P1C7 | `&kp U` | `&kp U` |
| 7 | P1C8 | `&kp I` | `&kp I` |
| 8 | P1C9 | `&kp O` | `&kp O` |
| 9 | P1C10 | `&kp P` | `&kp P` |
| 10 | P2C1 | `&mt_a_ctrl LCTRL A` | `&mt_a_ctrl LCTRL A` |
| 11 | P2C2 | `&kp S` | `&kp S` |
| 12 | P2C3 | `&lt_typ 6 D` | `&lt_typ 7 D` |
| 13 | P2C4 | `&kp F` | `&kp F` |
| 14 | P2C5 | `&kp G` | `&kp G` |
| 15 | P2C6 | `&kp H` | `&kp H` |
| 16 | P2C7 | `&kp J` | `&kp J` |
| 17 | P2C8 | `&kp K` | `&kp K` |
| 18 | P2C9 | `&kp L` | `&kp L` |
| 19 | P2C10 | `&lt 8 MINUS` | `&lt 8 MINUS` |
| 20 | P3C1 | `&mt_z_shift LEFT_SHIFT Z` | `&mt_z_shift LEFT_SHIFT Z` |
| 21 | P3C2 | `&kp X` | `&kp X` |
| 22 | P3C3 | `&kp C` | `&kp C` |
| 23 | P3C4 | `&kp V` | `&kp V` |
| 24 | P3C5 | `&kp B` | `&kp B` |
| 25 | P3C6 | `&lt 6 PRINTSCREEN` | `&mac_ime` |
| 26 | P3C7 | `&kp N` | `&kp N` |
| 27 | P3C8 | `&kp M` | `&kp M` |
| 28 | P3C9 | `&kp COMMA` | `&kp COMMA` |
| 29 | P3C10 | `&kp DOT` | `&kp DOT` |
| 30 | P3C11 | `&lt 10 SLASH` | `&lt 10 SLASH` |
| 31 | P4C1 | `&kp LCTRL` | `&kp LCTRL` |
| 32 | P4C2 | `&kp LGUI` | `&kp LEFT_ALT` |
| 33 | P4C3 | `&kp LALT` | `&kp LEFT_GUI` |
| 34 | P4C4 | `&mt LSHFT LANGUAGE_1` | `&mt LSHFT LANGUAGE_1` |
| 35 | P4C5 | `&lt 2 SPACE` | `&lt 3 SPACE` |
| 36 | P4C6 | `&lt 4 LANGUAGE_2` | `&lt 5 LANGUAGE_2` |
| 37 | P4C7 | `&lt_num 6 ENTER` | `&lt_num 10 ENTER` |
| 38 | P4C8 | `&kp LEFT_BRACKET` | `&kp LEFT_BRACKET` |
| 39 | P4C9 | `&kp BSPC` | `&kp BSPC` |
| 40 | P4C10 | `&settings_hold_tap 9 0` | `&settings_hold_tap 9 0` |
| 41 | P4C11 | `&kp DEL` | `&kp DEL` |

42 bindingは10/10/11/11に区切られる。表示上の行とmatrix row/colと座標名を混同しない。row4のtransformはRC(3,0)..RC(3,5), RC(3,9), RC(3,7), RC(3,8), RC(3,6), RC(3,10)。

指の目安はtools/around_forty_rb_mapping.jsonを読み、実際の運指は本人に確認する。

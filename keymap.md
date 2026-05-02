# Coordinate table: 物理座標マッピング

以下のテーブルは `keymap.md` 内の物理配列テーブルに対応する座標を示します。
各セルは `(row, col)` の形式で、行は上から `1`〜`4`、列は左から `1`〜`13` を割り当てています。

| 行 / 指 | 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | :--: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | (1,1) | (1,2) | (1,3) | (1,4) | (1,5) | (1,6) | 中心 | (1,8) | (1,9) | (1,10) | (1,11) | (1,12) | (1,13) |
| 2 | (2,1) | (2,2) | (2,3) | (2,4) | (2,5) | (2,6) | 中心 | (2,8) | (2,9) | (2,10) | (2,11) | (2,12) | (2,13) |
| 3 | (3,1) | (3,2) | (3,3) | (3,4) | (3,5) | (3,6) | 中心 | (3,8) | (3,9) | (3,10) | (3,11) | (3,12) | (3,13) |
| 4 | (4,1) | (4,2) | (4,3) | (4,4) | (4,5) | (4,6) | 中心 | (4,8) | (4,9) | (4,10) | (4,11) | (4,12) | (4,13) |

# Win-Base 実配列（物理配置）

以下は `Win-Base` レイヤーの、プログラム上の位置を現実のキーボード配列に並べ替えた表です。

- 表記ルール:
  - 長押しと単押しで挙動が変わるキーは「長押し,単押し」の順で記載します。長押しで入力されるものは太字にします。
  - レイヤー番号は`で囲みます（例: `8``）。
  - 左右分割キーボードのため中央を示す列を追加しています。

| 小               | 薬   | 中   | 人                   | 人            | 親                  | 中心 | 親   | 人            | 人  | 中  | 薬      | 小            |
| ---------------- | ---- | ---- | -------------------- | ------------- | ------------------- | :--: | ---- | ------------- | --- | --- | ------- | ------------- |
| Q                | W    | E    | R                    | T             |                     | 中心 |      | Y             | U   | I   | O       | P             |
| **LCTRL**,A      | S    | D    | F                    | G             |                     | 中心 |      | H             | J   | K   | L       | **`8`**,MINUS |
| **LEFT_SHIFT**,Z | X    | C    | V                    | B             | **`6`**,PRINTSCREEN | 中心 | [    | N             | M   | ,   | .       | **`10`**,/    |
| LCTRL            | LGUI | LALT | **LSHFT**,LANGUAGE_1 | **`2`**,SPACE | **`4`**,LANGUAGE_2  | 中心 | BSPC | **`6`**,ENTER |     |     | **`9`** | DEL           |

（注）左側が小指列、右側が小指列になるよう物理配列に配置しています。中心列は視認用です。

### Mac-Base 実配列（物理配置）

| 小               | 薬       | 中       | 人                   | 人            | 親                 | 中心 | 親   | 人             | 人  | 中  | 薬      | 小            |
| ---------------- | -------- | -------- | -------------------- | ------------- | ------------------ | :--: | ---- | -------------- | --- | --- | ------- | ------------- |
| Q                | W        | E        | R                    | T             |                    | 中心 |      | Y              | U   | I   | O       | P             |
| **LCTRL**,A      | S        | D        | F                    | G             |                    | 中心 |      | H              | J   | K   | L       | **`8`**,MINUS |
| **LEFT_SHIFT**,Z | X        | C        | V                    | B             | mac_ime            | 中心 | [    | N              | M   | ,   | .       | **`10`**,/    |
| LCTRL            | LEFT_ALT | LEFT_GUI | **LSHFT**,LANGUAGE_1 | **`3`**,SPACE | **`5`**,LANGUAGE_2 | 中心 | BSPC | **`10`**,ENTER |     |     | **`9`** | DEL           |

### Win-Fnc 実配列（物理配置）

| 小             | 薬       | 中      | 人       | 人       | 親    | 中心 | 親    | 人        | 人      | 中      | 薬  | 小  |
| -------------- | -------- | ------- | -------- | -------- | ----- | :--: | ----- | --------- | ------- | ------- | --- | --- |
| LC(A)          | LC(X)    | LC(C)   | LC(V)    | LC(F)    |       | 中心 |       | <         | >       | ^       | %   | ¥   |
| TAB            | LEFT_ALT | LS(TAB) | mkp(MB1) | mkp(MB2) |       | 中心 |       | (         | )       | @       | &   | "   |
| **LEFT_SHIFT** | trans    | trans   | swapper  | mkp(MB3) | trans | 中心 | [     | N         | M       | !       | ?   | '   |
| LCTRL          | trans    | trans   | trans    | trans    | trans | 中心 | trans | BACKSPACE | JP_DLLR | JP_HASH |     |     |

### Mac-Fnc 実配列（物理配置）

| 小             | 薬       | 中      | 人       | 人       | 親    | 中心 | 親    | 人        | 人      | 中        | 薬           | 小               |
| -------------- | -------- | ------- | -------- | -------- | ----- | :--: | ----- | --------- | ------- | --------- | ------------ | ---------------- |
| LG(A)          | LG(X)    | LG(C)   | LG(V)    | LG(F)    |       | 中心 |       | LS(COMMA) | LS(DOT) | LS(N6)    | LS(N5)       | BACKSLASH        |
| TAB            | LEFT_GUI | LS(TAB) | mkp(MB1) | mkp(MB2) |       | 中心 |       | LS(N8)    | LS(N9)  | LS(N2)    | LS(N7)       | LS(SINGLE_QUOTE) |
| **LEFT_SHIFT** | trans    | trans   | swapper  | mkp(MB3) | LG(R) | 中心 | [     | ]         | LS(N1)  | LS(SLASH) | SINGLE_QUOTE |                  |
| LCTRL          | trans    | trans   | trans    | trans    | trans | 中心 | trans | BACKSPACE | LS(N4)  | LS(N3)    |              |                  |

### Win-Common 実配列（物理配置）

| 小    | 薬       | 中      | 人             | 人              | 親          | 中心 | 親             | 人         | 人              | 中          | 薬        | 小  |
| ----- | -------- | ------- | -------------- | --------------- | ----------- | :--: | -------------- | ---------- | --------------- | ----------- | --------- | --- |
| ESC   | trans    | trans   | LG(UP_ARROW)   | LA(UP_ARROW)    | LC(W)       | 中心 | LA(LEFT_ARROW) | mkp(MB3)   | LA(RIGHT_ARROW) | HOME        |           |     |
| TAB   | LEFT_ALT | LS(TAB) | LG(LEFT_ARROW) | LG(RIGHT_ARROW) | LC(PAGE_UP) | 中心 | mkp(MB1)       | UP_ARROW   | mkp(MB2)        | PAGE_UP     |           |     |
| LSHFT | trans    | trans   | LG(DOWN_ARROW) | LA(DOWN_ARROW)  | LG(LS(S))   | 中心 | LC(PAGE_DOWN)  | LEFT_ARROW | DOWN_ARROW      | RIGHT_ARROW | PAGE_DOWN |     |
| LCTRL | trans    | trans   | trans          | trans           | trans       | 中心 | trans          | LC(T)      | trans           | trans       | END       |     |

### Mac-Common 実配列（物理配置）

| 小    | 薬       | 中      | 人             | 人              | 親         | 中心 | 親             | 人         | 人              | 中          | 薬        | 小  |
| ----- | -------- | ------- | -------------- | --------------- | ---------- | :--: | -------------- | ---------- | --------------- | ----------- | --------- | --- |
| ESC   | trans    | trans   | LC(UP_ARROW)   | LG(UP_ARROW)    | LG(W)      | 中心 | LG(LEFT_ARROW) | mkp(MB3)   | LG(RIGHT_ARROW) | HOME        |           |     |
| TAB   | LEFT_GUI | LS(TAB) | LC(LEFT_ARROW) | LC(RIGHT_ARROW) | LC(TAB)    | 中心 | mkp(MB1)       | UP_ARROW   | mkp(MB2)        | PAGE_UP     |           |     |
| LSHFT | trans    | trans   | LC(DOWN_ARROW) | LG(DOWN_ARROW)  | LG(LS(N4)) | 中心 | LS(LC(TAB))    | LEFT_ARROW | DOWN_ARROW      | RIGHT_ARROW | PAGE_DOWN |     |
| LCTRL | trans    | trans   | trans          | trans           | trans      | 中心 | trans          | LG(T)      | trans           | trans       | END       |     |

### Num_Scroll 実配列（物理配置）

| 小    | 薬    | 中      | 人      | 人       | 親    | 中心 | 親         | 人       | 人   | 中  | 薬  | 小       |
| ----- | ----- | ------- | ------- | -------- | ----- | :--: | ---------- | -------- | ---- | --- | --- | -------- |
| F1    | F2    | F3      | F4      | F5       |       | 中心 |            | JP_PLUS  | N7   | N8  | N9  | JP_MINUS |
| F6    | F7    | F8      | F9      | F10      |       | 中心 |            | JP_EQUAL | N4   | N5  | N6  | JP_SEMI  |
| F11   | F12   | JP_LBRC | JP_RBRC | JP_COLON | trans | 中心 | kp(JP_AST) | N1       | N2   | N3  | N0  |          |
| LCTRL | trans | &#96;   | ~       | &#124;   | trans | 中心 | ENTER      | &        | BSPC | /   | \_  |          |

### Mac_Num_Scroll 実配列（物理配置）

| 小    | 薬    | 中               | 人                | 人           | 親    | 中心 | 親               | 人        | 人   | 中  | 薬  | 小        |
| ----- | ----- | ---------------- | ----------------- | ------------ | ----- | :--: | ---------------- | --------- | ---- | --- | --- | --------- |
| F1    | F2    | F3               | F4                | F5           |       | 中心 |                  | LS(EQUAL) | N7   | N8  | N9  | MINUS     |
| F6    | F7    | F8               | F9                | F10          |       | 中心 |                  | EQUAL     | N4   | N5  | N6  | SEMICOLON |
| F11   | F12   | LS(LEFT_BRACKET) | LS(RIGHT_BRACKET) | SINGLE_QUOTE | trans | 中心 | LS(SINGLE_QUOTE) | N1        | N2   | N3  | N0  |           |
| LCTRL | trans | &#96;            | ~                 | &#124;       | trans | 中心 | ENTER            | LS(N7)    | BSPC | /   | \_  |           |

### V_Scroll 実配列（物理配置）

| 小    | 薬    | 中    | 人    | 人    | 親  | 中心 | 親  | 人    | 人    | 中      | 薬    | 小  |
| ----- | ----- | ----- | ----- | ----- | --- | :--: | --- | ----- | ----- | ------- | ----- | --- |
| trans | trans | trans | trans | trans |     | 中心 |     | trans | trans | trans   | trans |     |
| trans | trans | trans | trans | trans |     | 中心 |     | trans | trans | mo(`6`) | trans |     |
| trans | trans | trans | trans | trans |     | 中心 |     | trans | trans | trans   | trans |     |
| trans | trans | trans | trans | trans |     | 中心 |     | trans | trans | trans   | trans |     |

### Settings 実配列（物理配置）

| 小    | 薬    | 中    | 人    | 人    | 親           | 中心 | 親           | 人            | 人           | 中           | 薬             | 小  |
| ----- | ----- | ----- | ----- | ----- | ------------ | :--: | ------------ | ------------- | ------------ | ------------ | -------------- | --- |
| trans | trans | trans | trans | trans | bt(BT_SEL 0) | 中心 | bt(BT_SEL 1) | bt(BT_SEL 2)  | bt(BT_SEL 3) | bt(BT_SEL 4) |                |     |
| trans | trans | trans | trans | trans | trans        | 中心 | trans        | trans         | trans        | trans        |                |     |
| trans | trans | trans | trans | trans | to(`0`)      | 中心 | trans        | trans         | trans        | trans        | bt(BT_CLR)     |     |
| trans | trans | trans | trans | trans | to(`1`)      | 中心 | sys_reset    | studio_unlock | bootloader   | trans        | bt(BT_CLR_ALL) |     |

### AML 実配列（物理配置）

| 小    | 薬    | 中    | 人    | 人    | 親  | 中心 | 親       | 人       | 人       | 中                 | 薬    | 小  |
| ----- | ----- | ----- | ----- | ----- | --- | :--: | -------- | -------- | -------- | ------------------ | ----- | --- |
| trans | trans | trans | trans | trans |     | 中心 |          | trans    | trans    | msc(SCRL_UP)       | trans |     |
| trans | trans | trans | trans | trans |     | 中心 | mkp(MB1) | mkp(MB1) | mkp(MB2) | lt(`8`, RA(LA(A))) |       |
| trans | trans | trans | trans | trans |     | 中心 | trans    | trans    | trans    | mkp(MB3)           | trans |     |
| trans | trans | trans | trans | trans |     | 中心 |          | trans    | trans    | trans              | trans |     |

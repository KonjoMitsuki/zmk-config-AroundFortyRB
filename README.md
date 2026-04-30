# zmk-config-AroundFortyRB

Around Forty RBのファームウェアです。

---

## mainブランチで実装済み

🟢Zmkfirmware v0.3に対応。（tsunoshuu様、PR感謝します）

🟢PMW3610のドライバを「badjeff/zmk-pmw3610-driver」に変更

🟢ZMK Studioに対応

🟢全角半角の切り替えマクロ：全角半角のトグルが一つのキーで可能

🟡Prospector Scannerの対応はいったん見送っています　/ ※Bluetooth接続が不安定になるため

以下、ご利用ガイドです。

https://note.com/razily/n/n0b3c5ff58d92

---

## 以下はmainブランチには未実装の開発版（dev-main）のみの機能です

🟢Slow Curor layer：カーソル速度を一時的に遅くて精密操作をしやすくします

🟢2種類のScroll Layer：上下左右のスクロールができるレイヤーと、縦限定スクロールができるレイヤーがあります

---

# keymap

以下は、レイヤーごとのキーマップをMarkdownテーブルとして表現したものです。記号キーはその記号で、関数タイプのキーは名前や変数を使って記載しています。trans は transparent を意味します。

### レイヤー: Win-Base (layer 0)

| 行  | 1                         | 2    | 3    | 4                     | 5            | 6                  | 7                | 8   | 9    | 10           | 11            |
| --- | ------------------------- | ---- | ---- | --------------------- | ------------ | ------------------ | ---------------- | --- | ---- | ------------ | ------------- |
| 1   | Q                         | W    | E    | R                     | T            | Y                  | U                | I   | O    | P            |               |
| 2   | mt_a_ctrl(LCTRL, A)       | S    | D    | F                     | G            | H                  | J                | K   | L    | lt(8, MINUS) |               |
| 3   | mt_z_shift(LEFT_SHIFT, Z) | X    | C    | V                     | B            | lt(6, PRINTSCREEN) | N                | M   | ,    | .            | lt(10, SLASH) |
| 4   | LCTRL                     | LGUI | LALT | mt(LSHFT, LANGUAGE_1) | lt(2, SPACE) | lt(4, LANGUAGE_2)  | lt_num(6, ENTER) | [   | BSPC | mo(9)        | DEL           |

### レイヤー: Mac-Base (layer 1)

| 行  | 1                         | 2        | 3        | 4                     | 5            | 6                 | 7                 | 8   | 9    | 10           | 11            |
| --- | ------------------------- | -------- | -------- | --------------------- | ------------ | ----------------- | ----------------- | --- | ---- | ------------ | ------------- |
| 1   | Q                         | W        | E        | R                     | T            | Y                 | U                 | I   | O    | P            |               |
| 2   | mt_a_ctrl(LCTRL, A)       | S        | D        | F                     | G            | H                 | J                 | K   | L    | lt(8, MINUS) |               |
| 3   | mt_z_shift(LEFT_SHIFT, Z) | X        | C        | V                     | B            | mac_ime           | N                 | M   | ,    | .            | lt(10, SLASH) |
| 4   | LCTRL                     | LEFT_ALT | LEFT_GUI | mt(LSHFT, LANGUAGE_1) | lt(3, SPACE) | lt(5, LANGUAGE_2) | lt_num(10, ENTER) | [   | BSPC | mo(9)        | DEL           |

### レイヤー: Win-Fnc (layer 2)

| 行  | 1          | 2        | 3       | 4        | 5        | 6     | 7     | 8     | 9         | 10  | 11  |
| --- | ---------- | -------- | ------- | -------- | -------- | ----- | ----- | ----- | --------- | --- | --- |
| 1   | LC(A)      | LC(X)    | LC(C)   | LC(V)    | LC(F)    | <     | >     | ^     | %         | ¥   |     |
| 2   | TAB        | LEFT_ALT | LS(TAB) | mkp(MB1) | mkp(MB2) | (     | )     | @     | &         | "   |     |
| 3   | LEFT_SHIFT | trans    | trans   | swapper  | mkp(MB3) | trans | [     | ]     | !         | ?   | '   |
| 4   | LCTRL      | trans    | trans   | trans    | trans    | trans | trans | trans | BACKSPACE | $   | #   |

### レイヤー: Mac-Fnc (layer 3)

| 行  | 1          | 2        | 3       | 4        | 5        | 6     | 7     | 8     | 9         | 10  | 11  |
| --- | ---------- | -------- | ------- | -------- | -------- | ----- | ----- | ----- | --------- | --- | --- |
| 1   | LG(A)      | LG(X)    | LG(C)   | LG(V)    | LG(F)    | <     | >     | ^     | %         | \   |     |
| 2   | TAB        | LEFT_GUI | LS(TAB) | mkp(MB1) | mkp(MB2) | (     | )     | "     | '         | \*  |     |
| 3   | LEFT_SHIFT | trans    | trans   | swapper  | mkp(MB3) | LG(R) | [     | ]     | !         | ?   | '   |
| 4   | LCTRL      | trans    | trans   | trans    | trans    | trans | trans | trans | BACKSPACE | $   | #   |

### レイヤー: Win-Common (layer 4)

| 行  | 1     | 2        | 3       | 4              | 5               | 6           | 7              | 8          | 9               | 10          | 11        |
| --- | ----- | -------- | ------- | -------------- | --------------- | ----------- | -------------- | ---------- | --------------- | ----------- | --------- |
| 1   | ESC   | trans    | trans   | LG(UP_ARROW)   | LA(UP_ARROW)    | LC(W)       | LA(LEFT_ARROW) | mkp(MB3)   | LA(RIGHT_ARROW) | HOME        |           |
| 2   | TAB   | LEFT_ALT | LS(TAB) | LG(LEFT_ARROW) | LG(RIGHT_ARROW) | LC(PAGE_UP) | mkp(MB1)       | UP_ARROW   | mkp(MB2)        | PAGE_UP     |           |
| 3   | LSHFT | trans    | trans   | LG(DOWN_ARROW) | LA(DOWN_ARROW)  | LG(LS(S))   | LC(PAGE_DOWN)  | LEFT_ARROW | DOWN_ARROW      | RIGHT_ARROW | PAGE_DOWN |
| 4   | LCTRL | trans    | trans   | trans          | trans           | trans       | trans          | LC(T)      | trans           | trans       | END       |

### レイヤー: Mac-Common (layer 5)

| 行  | 1     | 2        | 3       | 4              | 5               | 6          | 7              | 8          | 9               | 10          | 11        |
| --- | ----- | -------- | ------- | -------------- | --------------- | ---------- | -------------- | ---------- | --------------- | ----------- | --------- |
| 1   | ESC   | trans    | trans   | LC(UP_ARROW)   | LG(UP_ARROW)    | LG(W)      | LG(LEFT_ARROW) | mkp(MB3)   | LG(RIGHT_ARROW) | HOME        |           |
| 2   | TAB   | LEFT_GUI | LS(TAB) | LC(LEFT_ARROW) | LC(RIGHT_ARROW) | LC(TAB)    | mkp(MB1)       | UP_ARROW   | mkp(MB2)        | PAGE_UP     |           |
| 3   | LSHFT | trans    | trans   | LC(DOWN_ARROW) | LG(DOWN_ARROW)  | LG(LS(N4)) | LS(LC(TAB))    | LEFT_ARROW | DOWN_ARROW      | RIGHT_ARROW | PAGE_DOWN |
| 4   | LCTRL | trans    | trans   | trans          | trans           | trans      | trans          | LG(T)      | trans           | trans       | END       |

### レイヤー: Num_Scroll (layer 6)

| 行  | 1     | 2     | 3     | 4   | 5      | 6     | 7     | 8   | 9    | 10  | 11  |
| --- | ----- | ----- | ----- | --- | ------ | ----- | ----- | --- | ---- | --- | --- |
| 1   | F1    | F2    | F3    | F4  | F5     | +     | 7     | 8   | 9    | -   |     |
| 2   | F6    | F7    | F8    | F9  | F10    | =     | 4     | 5   | 6    | ;   |     |
| 3   | F11   | F12   | [     | ]   | :      | trans | \*    | 1   | 2    | 3   | 0   |
| 4   | LCTRL | trans | &#96; | ~   | &#124; | trans | ENTER | &   | BSPC | /   | \_  |

### レイヤー: Mac_Num_Scroll (layer 7)

| 行  | 1     | 2     | 3     | 4   | 5      | 6     | 7     | 8   | 9    | 10  | 11  |
| --- | ----- | ----- | ----- | --- | ------ | ----- | ----- | --- | ---- | --- | --- |
| 1   | F1    | F2    | F3    | F4  | F5     | =     | 7     | 8   | 9    | -   |     |
| 2   | F6    | F7    | F8    | F9  | F10    | =     | 4     | 5   | 6    | ;   |     |
| 3   | F11   | F12   | {     | }   | '      | trans | \*    | 1   | 2    | 3   | 0   |
| 4   | LCTRL | trans | &#96; | ~   | &#124; | trans | ENTER | &   | BSPC | /   | \_  |

### レイヤー: V_Scroll (layer 8)

| 行  | 1     | 2     | 3     | 4     | 5     | 6     | 7     | 8     | 9     | 10    | 11    |
| --- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| 1   | trans | trans | trans | trans | trans | trans | trans | trans | trans | trans |       |
| 2   | trans | trans | trans | trans | trans | trans | trans | trans | mo(6) | trans |       |
| 3   | trans | trans | trans | trans | trans | trans | trans | trans | trans | trans | trans |
| 4   | trans | trans | trans | trans | trans | trans | trans | trans | trans | trans | trans |

### レイヤー: Settings (layer 9)

| 行  | 1     | 2     | 3     | 4     | 5     | 6            | 7            | 8             | 9            | 10           | 11             |
| --- | ----- | ----- | ----- | ----- | ----- | ------------ | ------------ | ------------- | ------------ | ------------ | -------------- |
| 1   | trans | trans | trans | trans | trans | bt(BT_SEL 0) | bt(BT_SEL 1) | bt(BT_SEL 2)  | bt(BT_SEL 3) | bt(BT_SEL 4) |                |
| 2   | trans | trans | trans | trans | trans | trans        | trans        | trans         | trans        | trans        |                |
| 3   | trans | trans | trans | trans | trans | to(0)        | trans        | trans         | trans        | trans        | bt(BT_CLR)     |
| 4   | trans | trans | trans | trans | trans | to(1)        | sys_reset    | studio_unlock | bootloader   | trans        | bt(BT_CLR_ALL) |

### レイヤー: AML (layer 10)

| 行  | 1     | 2     | 3     | 4     | 5     | 6     | 7        | 8        | 9            | 10               | 11    |
| --- | ----- | ----- | ----- | ----- | ----- | ----- | -------- | -------- | ------------ | ---------------- | ----- |
| 1   | trans | trans | trans | trans | trans | trans | trans    | trans    | msc(SCRL_UP) | trans            |       |
| 2   | trans | trans | trans | trans | trans | trans | mkp(MB1) | mkp(MB1) | mkp(MB2)     | lt(8, RA(LA(A))) |       |
| 3   | trans | trans | trans | trans | trans | trans | trans    | trans    | trans        | mkp(MB3)         | trans |
| 4   | trans | trans | trans | trans | trans | trans | trans    | trans    | trans        | trans            | trans |

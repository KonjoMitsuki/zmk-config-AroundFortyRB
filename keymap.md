# AroundFortyRB キーマップ

`config/AroundForty-RB.keymap` から生成した物理配置表です。WinはJIS、MacはUS配列を前提とします。

太字は長押し、カンマの後はタップの動作です。数字はレイヤー番号、`trans` は下位レイヤーの動作を使います。
Settingsキーは1回タップでAML自動OFF、2回タップで自動ON、長押しでSettingsです。

### 物理座標

`L` / `R` は左手 / 右手、続く数字は上から数えた段です。3・4段目の中央寄りの親指キーは `C0`、隣の縦列は `C1` です。1・2段目は `C1` から始まります。空白は数えず、4段目右側の欠けた縦列 `C2`・`C3` は飛ばします。

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| L1C5 | L1C4 | L1C3 | L1C2 | L1C1 |  | 中心 |  | R1C1 | R1C2 | R1C3 | R1C4 | R1C5 |
| L2C5 | L2C4 | L2C3 | L2C2 | L2C1 |  | 中心 |  | R2C1 | R2C2 | R2C3 | R2C4 | R2C5 |
| L3C5 | L3C4 | L3C3 | L3C2 | L3C1 | L3C0 | 中心 | R3C0 | R3C1 | R3C2 | R3C3 | R3C4 | R3C5 |
| L4C5 | L4C4 | L4C3 | L4C2 | L4C1 | L4C0 | 中心 | R4C0 | R4C1 |  |  | R4C4 | R4C5 |

### Win-Base Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| Q | W | E | R | T |  | 中心 |  | Y | U | I | O | P |
| **LCTRL**,A | S | **`6`**,D | F | G |  | 中心 |  | H | J | K | L | **`8`**,- |
| **LEFT_SHIFT**,Z | X | C | V | B | **`10`**,PRINTSCREEN | 中心 | @ | N | M | , | . | **`11`**,/ |
| LCTRL | LGUI | LALT | **LSHFT**,LANGUAGE_1 | **`2`**,SPACE | **`4`**,LANGUAGE_2 | 中心 | BSPC | **`6`**,ENTER |  |  | **`10`**,AML切替 | DEL |

### Mac-Base Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| Q | W | E | R | T |  | 中心 |  | Y | U | I | O | P |
| **LCTRL**,A | S | **`7`**,D | F | G |  | 中心 |  | H | J | K | L | **`8`**,- |
| **LEFT_SHIFT**,Z | X | C | V | B | mac_ime | 中心 | [ | N | M | , | . | **`11`**,/ |
| LCTRL | LEFT_ALT | LEFT_GUI | **LSHFT**,LANGUAGE_1 | **`3`**,SPACE | **`5`**,LANGUAGE_2 | 中心 | BSPC | **`7`**,ENTER |  |  | **`13`**,AML切替 | DEL |

### Win-Fnc Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| LC(A) | LC(X) | LC(C) | LC(V) | LC(F) |  | 中心 |  | < | > | ^ | % | ¥ |
| TAB | LEFT_ALT | LS(TAB) | mkp(MB1) | mkp(MB2) |  | 中心 |  | ( | ) | @ | & | " |
| LEFT_SHIFT | trans | trans | swapper | mkp(MB3) | trans | 中心 | trans | [ | ] | ! | ? | ' |
| LCTRL | trans | trans | trans | trans | trans | 中心 | BSPC | trans |  |  | $ | # |

### Mac-Fnc Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| LG(A) | LG(X) | LG(C) | LG(V) | LG(F) |  | 中心 |  | < | > | ^ | % | \ |
| TAB | LEFT_GUI | LS(TAB) | mkp(MB1) | mkp(MB2) |  | 中心 |  | * | ( | @ | & | " |
| LEFT_SHIFT | trans | trans | swapper | mkp(MB3) | LG(R) | 中心 | trans | [ | ] | ! | ? | ' |
| LCTRL | trans | trans | trans | trans | trans | 中心 | BSPC | trans |  |  | $ | # |

### Win-Common Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| ESC | ime_tog | trans | LG(UP_ARROW) | LA(UP_ARROW) |  | 中心 |  | LC(W) | LA(LEFT_ARROW) | mkp(MB3) | LA(RIGHT_ARROW) | HOME |
| TAB | LEFT_ALT | LS(TAB) | LG(LEFT_ARROW) | LG(RIGHT_ARROW) |  | 中心 |  | LC(PAGE_UP) | mkp(MB1) | UP_ARROW | mkp(MB2) | PAGE_UP |
| LSHFT | trans | trans | LG(DOWN_ARROW) | LA(DOWN_ARROW) | LG(LS(S)) | 中心 | LC(T) | LC(PAGE_DOWN) | LEFT_ARROW | DOWN_ARROW | RIGHT_ARROW | PAGE_DOWN |
| LCTRL | trans | trans | trans | trans | trans | 中心 | trans | trans |  |  | trans | END |

### Mac-Common Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| ESC | mac_ime | trans | LC(UP_ARROW) | LG(UP_ARROW) |  | 中心 |  | LG(W) | LG(LEFT_ARROW) | mkp(MB3) | LG(RIGHT_ARROW) | HOME |
| TAB | LEFT_GUI | LS(TAB) | LC(LEFT_ARROW) | LC(RIGHT_ARROW) |  | 中心 |  | LC(TAB) | mkp(MB1) | UP_ARROW | mkp(MB2) | PAGE_UP |
| LSHFT | trans | trans | LC(DOWN_ARROW) | LG(DOWN_ARROW) | LG(LS(N4)) | 中心 | LG(T) | LS(LC(TAB)) | LEFT_ARROW | DOWN_ARROW | RIGHT_ARROW | PAGE_DOWN |
| LCTRL | trans | trans | trans | trans | trans | 中心 | trans | trans |  |  | trans | END |

### Num_Scroll Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| F1 | F2 | F3 | F4 | F5 |  | 中心 |  | + | 7 | 8 | 9 | - |
| F6 | # | { | } | : |  | 中心 |  | = | 4 | 5 | 6 | ; |
| F7 | _ | &#96; | ~ | &#124; | trans | 中心 | & | * | 1 | 2 | 3 | , |
| LCTRL | F8 | F9 | F10 | F11 | F12 | 中心 | BSPC | ENTER |  |  | 0 | _ |

### Mac_Num_Scroll Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| F1 | F2 | F3 | F4 | F5 |  | 中心 |  | + | 7 | 8 | 9 | - |
| F6 | # | { | } | : |  | 中心 |  | = | 4 | 5 | 6 | ; |
| F7 | _ | &#96; | ~ | &#124; | trans | 中心 | & | * | 1 | 2 | 3 | , |
| LCTRL | F8 | F9 | F10 | F11 | F12 | 中心 | BSPC | ENTER |  |  | 0 | _ |

### V_Scroll Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | **`6`** | trans |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans |  |  | trans | trans |

### CLIP_STUDIO Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| LC(Z) | LC(Y) | LC(T) | LC(LS(N)) | F13 |  | 中心 |  | trans | trans | trans | trans | trans |
| LCTRL | B | E | I | C |  | 中心 |  | trans | trans | trans | trans | trans |
| LSHFT | M | R | LC(LALT) | LC(SPACE) | **`10`**,LA(JP_LBKT) | 中心 | trans | trans | trans | trans | trans | trans |
| DELETE | LC(D) | LALT | ENTER | SPACE | LA(JP_RBKT) | 中心 | trans | trans |  |  | trans | trans |

### Settings Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| to(`0`) | aml_toggle_td | tog(`9`) | trans | trans |  | 中心 |  | bt(BT_SEL 0) | bt(BT_SEL 1) | bt(BT_SEL 2) | bt(BT_SEL 3) | bt(BT_SEL 4) |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans | to(`0`) | 中心 | studio_unlock | trans | trans | trans | trans | bt(BT_CLR) |
| trans | trans | trans | trans | trans | to(`1`) | 中心 | bootloader | sys_reset |  |  | trans | bt(BT_CLR_ALL) |

### AML Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | msc(SCRL_UP) | trans |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | mkp(MB1) | mkp(MB1) | mkp(MB2) | **`8`**,RA(LA(A)) |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | trans | trans | mkp(MB3) | trans |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans |  |  | trans | trans |

### AML-Off Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans |  |  | trans | trans |

### Mac-Settings Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| to(`1`) | mac_aml_toggle_td | tog(`9`) | trans | trans |  | 中心 |  | bt(BT_SEL 0) | bt(BT_SEL 1) | bt(BT_SEL 2) | bt(BT_SEL 3) | bt(BT_SEL 4) |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans | to(`0`) | 中心 | studio_unlock | trans | trans | trans | trans | bt(BT_CLR) |
| trans | trans | trans | trans | trans | to(`1`) | 中心 | bootloader | sys_reset |  |  | trans | bt(BT_CLR_ALL) |

generated

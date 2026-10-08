# AroundFortyRB キーマップ

`config/AroundForty-RB.keymap` から生成した物理配置表です。WinはJIS、MacはUS配列を前提とします。

太字は長押し、カンマの後はタップの動作です。数字はレイヤー番号、`trans` は下位レイヤーの動作を使います。
Settingsキーは1回タップでAML自動OFF、2回タップで自動ON、長押しでSettingsです。

### 物理座標

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| P1C1 | P1C2 | P1C3 | P1C4 | P1C5 |  | 中心 |  | P1C6 | P1C7 | P1C8 | P1C9 | P1C10 |
| P2C1 | P2C2 | P2C3 | P2C4 | P2C5 |  | 中心 |  | P2C6 | P2C7 | P2C8 | P2C9 | P2C10 |
| P3C1 | P3C2 | P3C3 | P3C4 | P3C5 | P3C6 | 中心 | P4C8 | P3C7 | P3C8 | P3C9 | P3C10 | P3C11 |
| P4C1 | P4C2 | P4C3 | P4C4 | P4C5 | P4C6 | 中心 | P4C9 | P4C7 |  |  | P4C10 | P4C11 |

### Win-Base Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| Q | W | E | R | T |  | 中心 |  | Y | U | I | O | P |
| **LCTRL**,A | S | **`6`**,D | F | G |  | 中心 |  | H | J | K | L | **`8`**,- |
| **LEFT_SHIFT**,Z | X | C | V | B | **`6`**,PRINTSCREEN | 中心 | @ | N | M | , | . | **`10`**,/ |
| LCTRL | LGUI | LALT | **LSHFT**,LANGUAGE_1 | **`2`**,SPACE | **`4`**,LANGUAGE_2 | 中心 | BSPC | **`6`**,ENTER |  |  | **`9`**,AML切替 | DEL |

### Mac-Base Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| Q | W | E | R | T |  | 中心 |  | Y | U | I | O | P |
| **LCTRL**,A | S | **`7`**,D | F | G |  | 中心 |  | H | J | K | L | **`8`**,- |
| **LEFT_SHIFT**,Z | X | C | V | B | mac_ime | 中心 | [ | N | M | , | . | **`10`**,/ |
| LCTRL | LEFT_ALT | LEFT_GUI | **LSHFT**,LANGUAGE_1 | **`3`**,SPACE | **`5`**,LANGUAGE_2 | 中心 | BSPC | **`7`**,ENTER |  |  | **`12`**,AML切替 | DEL |

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
| ESC | trans | trans | LG(UP_ARROW) | LA(UP_ARROW) |  | 中心 |  | LC(W) | LA(LEFT_ARROW) | mkp(MB3) | LA(RIGHT_ARROW) | HOME |
| TAB | LEFT_ALT | LS(TAB) | LG(LEFT_ARROW) | LG(RIGHT_ARROW) |  | 中心 |  | LC(PAGE_UP) | mkp(MB1) | UP_ARROW | mkp(MB2) | PAGE_UP |
| LSHFT | trans | trans | LG(DOWN_ARROW) | LA(DOWN_ARROW) | LG(LS(S)) | 中心 | LC(T) | LC(PAGE_DOWN) | LEFT_ARROW | DOWN_ARROW | RIGHT_ARROW | PAGE_DOWN |
| LCTRL | trans | trans | trans | trans | trans | 中心 | trans | trans |  |  | trans | END |

### Mac-Common Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| ESC | trans | trans | LC(UP_ARROW) | LG(UP_ARROW) |  | 中心 |  | LG(W) | LG(LEFT_ARROW) | mkp(MB3) | LG(RIGHT_ARROW) | HOME |
| TAB | LEFT_GUI | LS(TAB) | LC(LEFT_ARROW) | LC(RIGHT_ARROW) |  | 中心 |  | LC(TAB) | mkp(MB1) | UP_ARROW | mkp(MB2) | PAGE_UP |
| LSHFT | trans | trans | LC(DOWN_ARROW) | LG(DOWN_ARROW) | LG(LS(N4)) | 中心 | LG(T) | LS(LC(TAB)) | LEFT_ARROW | DOWN_ARROW | RIGHT_ARROW | PAGE_DOWN |
| LCTRL | trans | trans | trans | trans | trans | 中心 | trans | trans |  |  | trans | END |

### Num_Scroll Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| trans | trans | trans | trans | trans |  | 中心 |  | + | 7 | 8 | 9 | - |
| F13 | # | { | } | : |  | 中心 |  | = | 4 | 5 | 6 | ; |
| trans | _ | &#96; | ~ | &#124; | trans | 中心 | & | * | 1 | 2 | 3 | , |
| LCTRL | trans | trans | trans | trans | trans | 中心 | BSPC | ENTER |  |  | 0 | _ |

### Mac_Num_Scroll Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| trans | trans | trans | trans | trans |  | 中心 |  | + | 7 | 8 | 9 | - |
| F13 | # | { | } | : |  | 中心 |  | = | 4 | 5 | 6 | ; |
| trans | _ | &#96; | ~ | &#124; | trans | 中心 | & | * | 1 | 2 | 3 | , |
| LCTRL | trans | trans | trans | trans | trans | 中心 | BSPC | ENTER |  |  | 0 | _ |

### V_Scroll Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | **`6`** | trans |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans |  |  | trans | trans |

### Settings Physical Layout

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| to(`0`) | aml_toggle_td | trans | trans | trans |  | 中心 |  | bt(BT_SEL 0) | bt(BT_SEL 1) | bt(BT_SEL 2) | bt(BT_SEL 3) | bt(BT_SEL 4) |
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
| to(`1`) | mac_aml_toggle_td | trans | trans | trans |  | 中心 |  | bt(BT_SEL 0) | bt(BT_SEL 1) | bt(BT_SEL 2) | bt(BT_SEL 3) | bt(BT_SEL 4) |
| trans | trans | trans | trans | trans |  | 中心 |  | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans | to(`0`) | 中心 | studio_unlock | trans | trans | trans | trans | bt(BT_CLR) |
| trans | trans | trans | trans | trans | to(`1`) | 中心 | bootloader | sys_reset |  |  | trans | bt(BT_CLR_ALL) |

generated

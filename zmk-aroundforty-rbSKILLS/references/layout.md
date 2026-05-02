# AroundForty-RB レイアウト参照

## 目次
1. [座標マップ（物理配置）](#1-座標マップ物理配置)
2. [バインディング行→座標 変換表](#2-バインディング行座標-変換表)
3. [全レイヤー現在のキー割り当て](#3-全レイヤー現在のキー割り当て)

---

## 1. 座標マップ（物理配置）

```
╔══════════════════════════════════════╦══════════════════════════════════════════╗
║  左手側                               ║  右手側                                   ║
╠══════╦══════╦══════╦══════╦══════╣  ╠══════╦══════╦══════╦══════╦══════╣
║P1C1  ║P1C2  ║P1C3  ║P1C4  ║P1C5  ║  ║P1C6  ║P1C7  ║P1C8  ║P1C9  ║P1C10 ║  ← 行1
╠══════╬══════╬══════╬══════╬══════╣  ╠══════╬══════╬══════╬══════╬══════╣
║P2C1  ║P2C2  ║P2C3  ║P2C4  ║P2C5  ║  ║P2C6  ║P2C7  ║P2C8  ║P2C9  ║P2C10 ║  ← 行2
╠══════╬══════╬══════╬══════╬══════╬══╬══╦═══╬══════╬══════╬══════╬══════╬══╣
║P3C1  ║P3C2  ║P3C3  ║P3C4  ║P3C5  ║P3C6  ║P4C8  ║P3C7  ║P3C8  ║P3C9  ║P3C10 ║P3C11 ║  ← 行3
╠══════╬══════╬══════╬══════╬══════╬══╬══╬════╬══════╬══════╬══╤══╬══════╬══╣
║P4C1  ║P4C2  ║P4C3  ║P4C4  ║P4C5  ║P4C6  ║      ║P4C9  ║P4C7  ║(空)║P4C10 ║P4C11 ║  ← 行4
╚══════╩══════╩══════╩══════╩══════╩══════╩══════╩══════╩══════╩════╩══════╩══╝
  小指   薬指   中指   人差  人差  親指         親指   人差  (空) 薬指   小指
```

### 指と座標の対応（概略）

| 行 | 小指(左) | 薬指(左) | 中指(左) | 人差(左) | 人差(左) | 親指(左) | → 中心 ← | 親指(右) | 人差(右) | 人差(右) | 中指(右) | 薬指(右) | 小指(右) |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | P1C1 | P1C2 | P1C3 | P1C4 | P1C5 | - | 中心 | - | P1C6 | P1C7 | P1C8 | P1C9 | P1C10 |
| 2 | P2C1 | P2C2 | P2C3 | P2C4 | P2C5 | - | 中心 | - | P2C6 | P2C7 | P2C8 | P2C9 | P2C10 |
| 3 | P3C1 | P3C2 | P3C3 | P3C4 | P3C5 | P3C6 | 中心 | P4C8 | P3C7 | P3C8 | P3C9 | P3C10 | P3C11 |
| 4 | P4C1 | P4C2 | P4C3 | P4C4 | P4C5 | P4C6 | 中心 | P4C9 | P4C7 | (空) | (空) | P4C10 | P4C11 |

> **注意**: P4C8（`[`キー）は行4ではなく**行3の右中央**に物理配置。

---

## 2. バインディング行→座標 変換表

`.keymap` の `bindings = < ... >;` 内のキーは以下の順序で並んでいる。

| 順序 | 座標 | Win-Base での現在値 |
|---|---|---|
| 1 | P1C1 | Q |
| 2 | P1C2 | W |
| 3 | P1C3 | E |
| 4 | P1C4 | R |
| 5 | P1C5 | T |
| 6 | P1C6 | Y |
| 7 | P1C7 | U |
| 8 | P1C8 | I |
| 9 | P1C9 | O |
| 10 | P1C10 | P |
| 11 | P2C1 | mt_a_ctrl(LCTRL, A) |
| 12 | P2C2 | S |
| 13 | P2C3 | D |
| 14 | P2C4 | F |
| 15 | P2C5 | G |
| 16 | P2C6 | H |
| 17 | P2C7 | J |
| 18 | P2C8 | K |
| 19 | P2C9 | L |
| 20 | P2C10 | lt(8, MINUS) |
| 21 | P3C1 | mt_z_shift(LEFT_SHIFT, Z) |
| 22 | P3C2 | X |
| 23 | P3C3 | C |
| 24 | P3C4 | V |
| 25 | P3C5 | B |
| 26 | P3C6 | lt(6, PRINTSCREEN) |
| 27 | P3C7 | N |
| 28 | P3C8 | M |
| 29 | P3C9 | COMMA |
| 30 | P3C10 | DOT |
| 31 | P3C11 | lt(10, SLASH) |
| 32 | P4C1 | LCTRL |
| 33 | P4C2 | LGUI |
| 34 | P4C3 | LALT |
| 35 | P4C4 | mt(LSHFT, LANGUAGE_1) |
| 36 | P4C5 | lt(2, SPACE) |
| 37 | P4C6 | lt(4, LANGUAGE_2) |
| 38 | P4C7 | lt_num(6, ENTER) |
| 39 | P4C8 | LEFT_BRACKET |
| 40 | P4C9 | BSPC |
| 41 | P4C10 | mo(9) |
| 42 | P4C11 | DEL |

> **バインディング内の順序メモ**: `.keymap` では 4 行に分けて書かれているが、  
> 実際の並びは「行1の10キー → 行2の10キー → 行3の11キー → 行4の11キー」の計42キー。  
> 行3と行4はそれぞれ左6＋右5（または右側に空きあり）の11要素。

---

## 3. 全レイヤー現在のキー割り当て

### Win-Base（Layer 0）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| Q | W | E | R | T | | 中心 | | Y | U | I | O | P |
| **LCTRL**,A | S | D | F | G | | 中心 | | H | J | K | L | **`8`**,- |
| **LSHIFT**,Z | X | C | V | B | **`6`**,PRTSC | 中心 | [ | N | M | , | . | **`10`**,/ |
| LCTRL | LGUI | LALT | **LSHFT**,言1 | **`2`**,SPACE | **`4`**,言2 | 中心 | BSPC | **`6`**,ENTER | | | **`9`** | DEL |

### Mac-Base（Layer 1）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| Q | W | E | R | T | | 中心 | | Y | U | I | O | P |
| **LCTRL**,A | S | D | F | G | | 中心 | | H | J | K | L | **`8`**,- |
| **LSHIFT**,Z | X | C | V | B | mac_ime | 中心 | [ | N | M | , | . | **`10`**,/ |
| LCTRL | LALT | LGUI | **LSHFT**,言1 | **`3`**,SPACE | **`5`**,言2 | 中心 | BSPC | **`10`**,ENTER | | | **`9`** | DEL |

### Win-Fnc（Layer 2）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| LC(A) | LC(X) | LC(C) | LC(V) | LC(F) | | 中心 | | < | > | ^ | % | ¥ |
| TAB | LALT | LS(TAB) | MB1 | MB2 | | 中心 | | ( | ) | @ | & | " |
| **LSHIFT** | trans | trans | swapper | MB3 | trans | 中心 | [ | N | M | ! | ? | ' |
| LCTRL | trans | trans | trans | trans | trans | 中心 | trans | BSPC | | | $ | # |

### Mac-Fnc（Layer 3）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| LG(A) | LG(X) | LG(C) | LG(V) | LG(F) | | 中心 | | < | > | ^ | % | \ |
| TAB | LGUI | LS(TAB) | MB1 | MB2 | | 中心 | | ( | ) | " | ' | * |
| **LSHIFT** | trans | trans | swapper | MB3 | LG(R) | 中心 | [ | ] | ! | ? | ' | |
| LCTRL | trans | trans | trans | trans | trans | 中心 | trans | BSPC | | | $ | # |

### Win-Common（Layer 4）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| ESC | trans | trans | LG(↑) | LA(↑) | LC(W) | 中心 | LA(←) | MB3 | LA(→) | HOME | | |
| TAB | LALT | LS(TAB) | LG(←) | LG(→) | LC(PgUp) | 中心 | MB1 | ↑ | MB2 | PgUp | | |
| LSHFT | trans | trans | LG(↓) | LA(↓) | LG(LS(S)) | 中心 | LC(PgDn) | ← | ↓ | → | PgDn | |
| LCTRL | trans | trans | trans | trans | trans | 中心 | trans | LC(T) | | | trans | END |

### Mac-Common（Layer 5）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| ESC | trans | trans | LC(↑) | LG(↑) | LG(W) | 中心 | LG(←) | MB3 | LG(→) | HOME | | |
| TAB | LGUI | LS(TAB) | LC(←) | LC(→) | LC(TAB) | 中心 | MB1 | ↑ | MB2 | PgUp | | |
| LSHFT | trans | trans | LC(↓) | LG(↓) | LG(LS(N4)) | 中心 | LS(LC(TAB)) | ← | ↓ | → | PgDn | |
| LCTRL | trans | trans | trans | trans | trans | 中心 | trans | LG(T) | | | trans | END |

### Num_Scroll（Layer 6）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| F1 | F2 | F3 | F4 | F5 | | 中心 | | + | 7 | 8 | 9 | - |
| F6 | F7 | F8 | F9 | F10 | | 中心 | | = | 4 | 5 | 6 | ; |
| F11 | F12 | { | } | : | trans | 中心 | * | 1 | 2 | 3 | 0 | |
| LCTRL | trans | ` | ~ | \| | trans | 中心 | ENTER | & | BSPC | / | \_ | |

### Mac_Num_Scroll（Layer 7）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| F1 | F2 | F3 | F4 | F5 | | 中心 | | + | 7 | 8 | 9 | - |
| F6 | F7 | F8 | F9 | F10 | | 中心 | | = | 4 | 5 | 6 | ; |
| F11 | F12 | { | } | ' | trans | 中心 | * | 1 | 2 | 3 | 0 | |
| LCTRL | trans | ` | ~ | \| | trans | 中心 | ENTER | ' | BSPC | / | \_ | |

### V_Scroll（Layer 8）

全て `trans`（P2C9/P2C10 ホールド後に `mo(6)` で Num レイヤーへ遷移できる）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| trans | trans | trans | trans | trans | | 中心 | | trans | trans | trans | trans | |
| trans | trans | trans | trans | trans | | 中心 | | trans | trans | **`6`** | trans | |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | trans | trans | trans | trans |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | | | trans | trans |

### Settings（Layer 9）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| trans | trans | trans | trans | trans | BT_SEL 0 | 中心 | BT_SEL 1 | BT_SEL 2 | BT_SEL 3 | BT_SEL 4 | | |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | trans | trans | | |
| trans | trans | trans | trans | trans | to(0) | 中心 | trans | trans | trans | trans | BT_CLR | |
| trans | trans | trans | trans | trans | to(1) | 中心 | sys_reset | studio_unlock | bootloader | | trans | BT_CLR_ALL |

### AML（Layer 10）

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
|---|---|---|---|---|---|:---:|---|---|---|---|---|---|
| trans | trans | trans | trans | trans | | 中心 | | trans | trans | SCRL_UP | trans | |
| trans | trans | trans | trans | trans | | 中心 | MB1 | MB1 | MB2 | **`8`**,RA(LA(A)) | | |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | trans | MB3 | trans | |
| trans | trans | trans | trans | trans | trans | 中心 | trans | trans | | | trans | trans |

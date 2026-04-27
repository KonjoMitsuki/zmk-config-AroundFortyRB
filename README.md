# zmk-config-AroundFortyRB

Around Forty RBのファームウェアです。

-------------------------------------------------------------------------
mainブランチで実装済み
-------------------------------------------------------------------------

🟢Zmkfirmware v0.3に対応。（tsunoshuu様、PR感謝します）

🟢PMW3610のドライバを「badjeff/zmk-pmw3610-driver」に変更

🟢ZMK Studioに対応

🟢全角半角の切り替えマクロ：全角半角のトグルが一つのキーで可能

🟡Prospector Scannerの対応はいったん見送っています　/ ※Bluetooth接続が不安定になるため

以下、ご利用ガイドです。

https://note.com/razily/n/n0b3c5ff58d92

-------------------------------------------------------------------------
以下はmainブランチには未実装の開発版（dev-main）のみの機能です
-------------------------------------------------------------------------

🟢Slow Curor layer：カーソル速度を一時的に遅くて精密操作をしやすくします

🟢2種類のScroll Layer：上下左右のスクロールができるレイヤーと、縦限定スクロールができるレイヤーがあります


---
# keymap

以下は、レイヤーごとのキーマップをMarkdownテーブルとして表現したものです。記号キーはその記号で、関数タイプのキー（例：`&mt` や `&lt`）は名前や変数を使って記載しています。

---

### レイヤー：Win-Base (layer 0)

| Q   | W   | E   | R   | T   | Y   | U   | I   | O   | P   |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| LCTRL + A | S   | D   | F   | G   | H   | J   | K   | L   | LT (8, ...) |
| LSHFT + Z | X   | C   | V   | B   | LT (6, PrintScreen) | N   | M   | ,   | .   |
| LCTRL | LGUI | LALT | LSHFT + Language_1 | LT (2, Space) | LT (4, Language_2) | Enter | [   | BSPC | ... |

---

### レイヤー：Mac-Base (layer 1)

| Q   | W   | E   | R   | T   | Y   | U   | I   | O   | P   |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| LCTRL + A | S   | D   | F   | G   | H   | J   | K   | L   | LT (8, ...) |
| LSHFT + Z | X   | C   | V   | B   | mac_ime | N   | M   | ,   | .   |
| LCTRL | LALT | LGUI | LSHFT + Language_1 | LT (3, Space) | LT (5, Language_2) | Enter | [   | BSPC | ... |

---

### レイヤー：Win-Fnc (layer 2)

| LC(A) | LC(X) | LC(C) | LC(V) | LC(F) | JP_LT | JP_GT | JP_CARET | JP_PRCNT | ... |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Tab | LALT | LS(TAB) | Mouse Button 1 | Mouse Button 2 | JP_LPAR | JP_RPAR | JP_AT | JP_AMPS | ... |
| LSHFT | ... | ... | swapper | Mouse Button 3 | ... | JP_LBKT | JP_RBKT | JP_EXCL | ... |
| LCTRL | ... | ... | ... | ... | ... | ... | BackSpace | ... | ... |

---

### レイヤー：Mac-Fnc (layer 3)

| LG(A) | LG(X) | LG(C) | LG(V) | LG(F) | LS(<) | LS(>) | LS(^) | LS(%) | ... |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Tab | LGUI | LS(Tab) | Mouse Button 1 | Mouse Button 2 | LS(( | LS()) | LS(@) | LS(&) | ... |
| LSHFT | ... | ... | swapper | Mouse Button 3 | LS(R) | [   | ]   | LS(!) | ... |
| LCTRL | ... | ... | ... | ... | ... | ... | BackSpace | ... | ... |

---

### レイヤー：Num_Scroll (layer 6)

| F1 | F2 | F3 | F4 | F5 | +   | 7   | 8   | 9   | /   |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| F6 | F7 | F8 | F9 | F10 | =   | 4   | 5   | 6   | *   |
| F11 | F12 | {   | }   | :   | ... | *   | 1   | 2   | 3   |
| LCTRL | ... | `   | ~   | |   | ... | Enter | ... | BSPC | ... |

---e

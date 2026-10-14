# zmk-config-AroundFortyRB

Around Forty RBのファームウェアです。

---

## mainブランチで実装済み

🟢Zmkfirmware v0.3に対応。（tsunoshuu様、PR感謝します）

🟢PMW3610のドライバを「badjeff/zmk-pmw3610-driver」に変更

🟢ZMK Studioに対応

🟢全角半角の切り替えマクロ：全角半角のトグルが一つのキーで可能

🟢2種類のScroll Layer：上下左右のスクロールができるレイヤーと、縦限定スクロールができるレイヤーがあります

🟡Prospector Scannerの対応はいったん見送っています　/ ※Bluetooth接続が不安定になるため

以下、ご利用ガイドです。

https://note.com/razily/n/n0b3c5ff58d92

---

## 以下はmainブランチには未実装の開発版（dev-main）のみの機能です

🟢Slow Curor layer：カーソル速度を一時的に遅くて精密操作をしやすくします

---

# keymap

全13レイヤーの現行配置は [keymap.md](keymap.md) にあります。
設定の本体は [config/AroundForty-RB.keymap](config/AroundForty-RB.keymap) です。
WindowsはJIS、macOSはUS配列を前提としています。

座標は `L/R`（左手/右手）＋上からの段＋`C`＋各段の中央から外側への順番です。
空白は数えず、各段で番号を数え直します。例: `L1C1(T)`、`L4C2(SPACE)`、`R4C2(ENTER)`。
キー名付きの物理図は [keymap-physical-template.md](keymap-physical-template.md) を参照してください。

| 操作 | Windows | macOS |
| --- | --- | --- |
| R4C2をタップ／長押し | Enter／Num（6） | Enter／Mac-Num（7） |
| R4C3を1回タップ | AML自動OFF、Win配列へ復帰 | AML自動OFF、Mac配列へ復帰 |
| R4C3を2回タップ | AML自動ON、Win配列へ復帰 | AML自動ON、Mac配列へ復帰 |
| R4C3を長押し | Settings（9） | Mac-Settings（12） |
| Settingsを保持してL1C5 | AML自動ON、Win配列へ復帰 | AML自動ON、Mac配列へ復帰 |
| Settingsを保持してL1C4を1回／2回タップ | AML自動OFF／ON、Win配列へ復帰 | AML自動OFF／ON、Mac配列へ復帰 |
| R3C1をタップ | @ | [ |

R3C1は見た目の3行目右中央のキーです。座標は `keymap.md` 冒頭の表を参照してください。
Settingsを保持してL3C1を押すとWin、L4C1を押すとMacへ切り替わります。

AML中のR1C4（スクロール上）とR3C5（中クリック）は、押してもAMLを解除しません。
AMLは最後のトラックボール入力から2000msで解除され、その他の解除対象キーでも解除されます。
AML-Off（11）中も数字レイヤーと縦スクロールレイヤーでのスクロールは使えます。

配置表の再生成:

```bash
python3 tools/zmk_keymap_physical.py --src config/AroundForty-RB.keymap --out keymap.md --map tools/around_forty_rb_mapping.json
```

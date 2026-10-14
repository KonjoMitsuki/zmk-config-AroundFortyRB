# 相談開始用スナップショット

2026-10-14、main `23966d9`。ローカルkeymapと編集用テンプレートを照合。
以下を現在値として固定せず、相談ごとに実keymapを読む。

| 座標 | index | Win base | Mac base |
|---|---|---|---|
| L2C5 | 10 | mt_a_ctrl LCTRL A | 同左 |
| L2C3 | 12 | lt_typ 6 D | lt_typ 7 D |
| L3C6 | 20 | mt_z_shift LEFT_SHIFT Z | 同左 |
| L3C1 | 25 | lt 6 PRINTSCREEN | mac_ime |
| L4C3 | 34 | mt LSHFT LANGUAGE_1 | 同左 |
| L4C2 | 35 | lt 2 SPACE | lt 3 SPACE |
| L4C1 | 36 | lt 4 LANGUAGE_2 | lt 5 LANGUAGE_2 |
| R4C2 | 37 | lt_num 6 ENTER | lt_num 7 ENTER |
| R3C1 | 38 | kp JP_AT | kp LEFT_BRACKET |
| R4C1 | 39 | kp BSPC | 同左 |
| R4C3 | 40 | settings_hold_tap 9 0 | mac_settings_hold_tap 12 0 |
| R4C4 | 41 | kp DEL | 同左 |

座標はL/R＋上からの段＋C＋各段の中央から外側への番号。
R3C1は物理3段目の右中央で、bindingの第4行にある。
WinのJP_ATは@、MacのLEFT_BRACKETはUS配列で[。
詳細な42位置の対応はzmk-aroundforty-rbのreferences/layout.mdと
keymap-physical-template.md、mappingのbinding_orderを照合する。

Commonの矢印はR2C3=UP、R3C3=LEFT、R3C4=DOWN、R3C5=RIGHT。
HJKLの物理位置R2C1～R2C4とは異なる。これは逆T配置として説明する。

Win-Fncの括弧はR2C1=JP_LPAR、R2C2=JP_RPAR、
R3C2=JP_LBKT、R3C3=JP_RBKT。
Win-NumにはJP_LBRC/JP_RBRCもあるため、
括弧相談ではNumも読む。[]だけ見て配置を移動しない。

AML（10）での非trans:
R1C4=SCRL_UP、R2C2/R2C3=MB1、R2C4=MB2、
R2C5=lt 8 RA(LA(A))、R3C5=MB3。
R.overlayのexcludedは7/8/16/17/18/19/28/29/30。
scroll upとMB3もexcludedに含まれる。同じ一覧とは限らないため解除とクリックのタイミングを確認する。

AML-Off（11）は全trans。Settingsのaml_toggle_td（Macはmac_aml_toggle_td）と
R4C3のtap動作で切り替える。Winはaml_offがto 0→tog 11、double tapがto 0。
Macはmac_aml_offがto 1→tog 11、double tapがto 1で、Mac baseを維持する。
R4C3をholdするとWinはSettings（9）、MacはMac-Settings（12）。
R4C2のEnter holdはWinがNum（6）、MacがMac-Num（7）。

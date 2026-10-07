# 相談開始用スナップショット

2026-10-07、main 000496ebfc86823ff2c5f797dcfded358e1243bf。
ソース: https://github.com/KonjoMitsuki/zmk-config-AroundFortyRB/blob/000496ebfc86823ff2c5f797dcfded358e1243bf/config/AroundForty-RB.keymap
以下を現在値として固定せず、相談ごとに実keymapを読む。

| 座標 | index | Win base | Mac base |
|---|---|---|---|
| P2C1 | 10 | mt_a_ctrl LCTRL A | 同左 |
| P2C3 | 12 | lt_typ 6 D | lt_typ 7 D |
| P3C1 | 20 | mt_z_shift LEFT_SHIFT Z | 同左 |
| P3C6 | 25 | lt 6 PRINTSCREEN | mac_ime |
| P4C4 | 34 | mt LSHFT LANGUAGE_1 | 同左 |
| P4C5 | 35 | lt 2 SPACE | lt 3 SPACE |
| P4C6 | 36 | lt 4 LANGUAGE_2 | lt 5 LANGUAGE_2 |
| P4C7 | 37 | lt_num 6 ENTER | lt_num 10 ENTER |
| P4C8 | 38 | kp LEFT_BRACKET | 同左 |
| P4C9 | 39 | kp BSPC | 同左 |
| P4C10 | 40 | settings_hold_tap 9 0 | 同左 |
| P4C11 | 41 | kp DEL | 同左 |

P4C8は見た目の3行目右中央に置かれた位置。bindingの第4行にある。
Win/JISでLEFT_BRACKETの出力を「[」と断定しない。
従来layout.mdのD=通常kp、Settings=moという記載は古い。

Commonの矢印はP2C8=UP、P3C8=LEFT、P3C9=DOWN、P3C10=RIGHT。
HJKLの物理位置P2C6～P2C9とは異なる。これは逆T配置として説明する。

Win-Fncの括弧はP2C6=JP_LPAR、P2C7=JP_RPAR、
P3C7=JP_LBKT、P3C8=JP_RBKT。
Win-NumにはJP_LBRC/JP_RBRCもあるため、
括弧相談ではNumも読む。[]だけ見て配置を移動しない。

AML（10）での非trans:
P1C9=SCRL_UP、P2C7/P2C8=MB1、P2C9=MB2、
P2C10=lt 8 RA(LA(A))、P3C10=MB3。
R.overlayのexcludedは7/16/17/18/19/28/30。
同じ一覧ではないため解除とクリックのタイミングを確認する。

AML-Off（11）は全trans。Settingsのaml_toggle_tdと
P4C10のtap動作で切り替える。aml_offはto 0→tog 11、
double tapはto 0で、Mac base維持ではない。
Mac Enter holdがNumでなく10を指すこととあわせ、
現行実装の要確認点として説明し、相談なしに修正しない。

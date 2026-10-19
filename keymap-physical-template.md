# AroundFortyRB キーマップ対応表（編集用テンプレート）

Win-Base のキー割り当てを物理座標に対応付けた表です。表記は `座標(キー)` です。

`L` / `R` は左手 / 右手、続く数字は上から数えた段、`C` の数字は中央からそろえた縦列番号です。3・4段目の中央寄りの親指キーを `C0` とし、1・2段目は `C1` から始めます。
左手は図の右から左へ、右手は左から右へ番号が増えます。空白の縦列も番号を飛ばして残します。

| 段 | 左手 | 右手 |
| --- | --- | --- |
| 1 | L1C1〜L1C5 | R1C1〜R1C5 |
| 2 | L2C1〜L2C5 | R2C1〜R2C5 |
| 3 | L3C0〜L3C5 | R3C0〜R3C5 |
| 4 | L4C0〜L4C5 | R4C0、R4C1、R4C4、R4C5 |

以下は `.keymap` の binding 順です。物理図の段とソース上の行は一致しない箇所があります。
0始まりの binding index と座標の対応は `tools/around_forty_rb_mapping.json` の `binding_order` にあります。
括弧内はタップ時のキー名です。R4C4 は1回タップでAML自動OFF、2回タップで自動ONになります。

```text
Win-Base { // layer 0
    L1C5(Q)  L1C4(W)  L1C3(E)  L1C2(R)  L1C1(T)                                           R1C1(Y)  R1C2(U)  R1C3(I)  R1C4(O)  R1C5(P)
    L2C5(A)  L2C4(S)  L2C3(D)  L2C2(F)  L2C1(G)                                           R2C1(H)  R2C2(J)  R2C3(K)  R2C4(L)  R2C5(-)
    L3C5(Z)  L3C4(X)  L3C3(C)  L3C2(V)  L3C1(B)  L3C0(PRINTSCREEN)                        R3C1(N)  R3C2(M)  R3C3(,)  R3C4(.)  R3C5(/)
    L4C5(LCTRL)  L4C4(LGUI)  L4C3(LALT)  L4C2(LANGUAGE_1)  L4C1(SPACE)  L4C0(LANGUAGE_2)  R4C1(ENTER)  R3C0(JP_AT)  R4C0(BSPC)  R4C4(AML切替)  R4C5(DEL)
}
```

## 物理配置図

Win-Base の割り当てを `座標(キー)` で記載しています。左右の中央列は基板中央です。
R3C0(JP_AT)、R4C0(BSPC)、R4C1(ENTER) はそれぞれ binding index 38、39、37 に対応します。

| 小 | 薬 | 中 | 人 | 人 | 親 | 中心 | 親 | 人 | 人 | 中 | 薬 | 小 |
| --- | --- | --- | --- | --- | --- | :--: | --- | --- | --- | --- | --- | --- |
| L1C5(Q) | L1C4(W) | L1C3(E) | L1C2(R) | L1C1(T) |  | 中心 |  | R1C1(Y) | R1C2(U) | R1C3(I) | R1C4(O) | R1C5(P) |
| L2C5(A) | L2C4(S) | L2C3(D) | L2C2(F) | L2C1(G) |  | 中心 |  | R2C1(H) | R2C2(J) | R2C3(K) | R2C4(L) | R2C5(-) |
| L3C5(Z) | L3C4(X) | L3C3(C) | L3C2(V) | L3C1(B) | L3C0(PRINTSCREEN) | 中心 | R3C0(JP_AT) | R3C1(N) | R3C2(M) | R3C3(,) | R3C4(.) | R3C5(/) |
| L4C5(LCTRL) | L4C4(LGUI) | L4C3(LALT) | L4C2(LANGUAGE_1) | L4C1(SPACE) | L4C0(LANGUAGE_2) | 中心 | R4C0(BSPC) | R4C1(ENTER) |  |  | R4C4(AML切替) | R4C5(DEL) |

## メモ

- hold/tap など、1キーに複数動作がある場合の表記：
- Win と Mac で差がある箇所：
- その他：

        Win-Base { // layer 0
            display-name = "Win-Base";
            bindings = <
                &kp Q                    &kp W          &kp E          &kp R                  &kp T                                                           &kp Y                   &kp U            &kp I             &kp O             &kp P
                &mt_a_ctrl LCTRL A       &kp S          &lt_typ 6 D      &kp F                  &kp G                                                         &kp H                   &kp J            &kp K             &kp L             &lt 8 MINUS
                &mt_z_shift LEFT_SHIFT Z &kp X          &kp C          &kp V                  &kp B                  &lt 10 PRINTSCREEN                       &kp N            &kp M             &kp COMMA         &kp DOT   &lt 11 SLASH
                &kp LCTRL                &kp LGUI       &kp LALT       &mt LSHFT LANGUAGE_1   &lt 2 SPACE            &lt 4 LANGUAGE_2        &lt_num 6 ENTER  &kp JP_AT         &kp BSPC          &settings_hold_tap 10 0  &kp DEL
            >;
        };

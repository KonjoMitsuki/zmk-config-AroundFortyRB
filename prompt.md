# プログラム上の配列と現実の配列の違い
.keymapで定義した配列と現実のキーボードの配列は違いがある。
それぞれの例を示すので参考にしてほしい。
## プログラム上の配列
```
        Win-Base { // layer 0
            display-name = "Win-Base";
            bindings = <
                &kp Q                    &kp W          &kp E          &kp R                  &kp T                  &kp Y                   &kp U            &kp I             &kp O             &kp P
                &mt_a_ctrl LCTRL A       &kp S          &kp D          &kp F                  &kp G                  &kp H                   &kp J            &kp K             &kp L             &lt 8 MINUS
                &mt_z_shift LEFT_SHIFT Z &kp X          &kp C          &kp V                  &kp B                  &lt 6 PRINTSCREEN      &kp N            &kp M             &kp COMMA         &kp DOT   &lt 10 SLASH
                &kp LCTRL                &kp LGUI       &kp LALT       &mt LSHFT LANGUAGE_1   &lt 2 SPACE              &lt 4 LANGUAGE_2          &lt_num 6 ENTER    &kp LEFT_BRACKET  &kp BSPC          &mo 9     &kp DEL
            >;
        };
```
テーブルのすると以下になる

| --- | ------------------------- | ---- | ---- | --------------------- | ------------ | ------------------ | ---------------- | --- | ---- | ------------ | ------------- |
| 1   | Q                         | W    | E    | R                     | T            | Y                  | U                | I   | O    | P            |               |
| 2   | mt_a_ctrl(LCTRL, A)       | S    | D    | F                     | G            | H                  | J                | K   | L    | lt(8, MINUS) |               |
| 3   | mt_z_shift(LEFT_SHIFT, Z) | X    | C    | V                     | B            | lt(6, PRINTSCREEN) | N                | M   | ,    | .            | lt(10, SLASH) |
| 4   | LCTRL                     | LGUI | LALT | mt(LSHFT, LANGUAGE_1) | lt(2, SPACE) | lt(4, LANGUAGE_2)  | lt_num(6, ENTER) | [   | BSPC | mo(9)        | DEL           |

## 現実の配列

|              |      |      |                  |            |                  |      |           |     |     |     |          |
| ------------ | ---- | ---- | ---------------- | ---------- | ---------------- | ---- | --------- | --- | --- | --- | -------- |
| Q            | W    | E    | R                | T          |                  |      | Y         | U   | I   | O   | P        |
| LCTRL,A      | S    | D    | F                | G          |                  |      | H         | J   | K   | L   | 8, MINUS |
| LEFT_SHIFT,Z | X    | C    | V                | B          | `6`, PRINTSCREEN | [    | N         | M   | ,   | .   | 10, /    |
| LCTRL        | LGUI | LALT | LSHFT,LANGUAGE_1 | `2`,SPACE) | `4`,LANGUAGE_2   | BSPC | `6`,ENTER |     |     | `9` | DEL      |
keymapにはプログラム上の位置を現実の位置に配置したものを出力して。

# 書式
- README.mdにkeymapを追加してほしいです。mdのtable形式で書いてください。記号キーはその記号で、関数タイプのキーは名前や変数を使って記載して
- ただしltやmtなどの長押しと単押しで挙動が変わるものは'長押し,単押し'のように書いて
- 長押しで入力されるものを\*\*で囲って太字にしてください。
- ltなどのレイヤーを表す数字は\`\`で囲ってコードにしてください。
- 一行めには指を入れてください。
- 左右分割のキーボードのため中心が分かるような列を追加して。

# 理想的なテーブルの例

| 小           | 薬   | 中   | 人               | 人         | 親               | 中心 | 親   | 人        | 人  | 中  | 薬  | 小       |
| ------------ | ---- | ---- | ---------------- | ---------- | ---------------- |:----:| ---- | --------- | --- | --- | --- | -------- |
| Q            | W    | E    | R                | T          |                  | 中心 |      | Y         | U   | I   | O   | P        |
| LCTRL,A      | S    | D    | F                | G          |                  | 中心 |      | H         | J   | K   | L   | 8, MINUS |
| LEFT_SHIFT,Z | X    | C    | V                | B          | `6`, PRINTSCREEN | 中心 | [    | N         | M   | ,   | .   | 10, /    |
| LCTRL        | LGUI | LALT | LSHFT,LANGUAGE_1 | `2`,SPACE) | `4`,LANGUAGE_2   | 中心 | BSPC | `6`,ENTER |     |     | `9` | DEL      |

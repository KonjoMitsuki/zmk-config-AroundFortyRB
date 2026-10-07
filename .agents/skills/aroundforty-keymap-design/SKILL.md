---
name: aroundforty-keymap-design
description: >
  AroundFortyRBのキーマップ相談・比較・改善設計。キー配置、記号、数字、
  hold-tap、修飾キー、combo、レイヤー移動、Neovim、terminal、プログラミング、
  日本語IME、Windows/macOS、トラックボール、AMLとの競合を相談する際に使う。
  現行ソースと物理座標を確認し、最大3案のトレードオフを示す。
  実際のZMK編集にはzmk-aroundforty-rbを併用する。
---

# AroundFortyRB キーマップ相談

## 相談の基準を確定する

1. 現在のcheckoutを特定する。なければ指定GitHub repoを読む。
   既定対象はhttps://github.com/KonjoMitsuki/zmk-config-AroundFortyRB。
   設定が読めなければ具体bindingを捏造せず、一般案と未確認事項を区別する。
2. keymap、dtsi/transform、mapping、R.overlayとbehaviorを読む。
   zmk-aroundforty-rbが使えればその物理座標・診断手順も適用する。
   このSKILL単独でもソースから42位置とlayer順を確定する。
3. [current-keymap.md](references/current-keymap.md)は相談開始の案内として読む。
   snapshotと現行ソースの差を確認する。基準コミットを示す。
4. Neovim、terminal、プログラミング、日本語入力を主要用途として扱う。
   Windows JIS/Mac USは現行設定の対応。Macを実際に使う頻度、
   dominant hand、運指、痛み、入力速度を決めつけない。
   判断が変わる情報だけ短く尋ね、分かる範囲の比較は進める。

## 現在配置と操作を説明する

- 対象の座標、0始まりindex、tap/hold、押すlayerキー、出力を表にする。
- 同位置のBase/Fnc/Common/Num/V_Scroll/Settings/AML/AML-Offを読む。
  OS対応レイヤーも含め、透過と高位layerを解決する。
- 印字文字・HID keycode・OS上の出力を区別する。
  日本語IME、JIS/US、Neovim側mappingが結果を変える場合は前提を添える。
- 「[]が使いづらい」のような相談では具体的な使用例
  （Vimの移動、配列、添字など）を特定する。
  Vim標準shortcutとユーザーのdotfiles固有bindingを混同しない。
  dotfilesを参照する場合は実ファイルを確認する。

## 最大3案で比較する

[design-principles.md](references/design-principles.md)に従い、
現在の長所を残した小変更から提案する。
各案について以下を同じ表で比較する:

- 座標と旧→新binding、OSごとの違い
- 具体的な押下手順と必要なlayer操作
- 指の移動・同指連続・同時hold・modifier操作
- hold-tapの遅延、roll/連打の誤判定、comboとの競合
- trackball使用中のAML/scrollとの競合
- 他の用途で失う機能と代替アクセス

測定せずWPM・誤爆率・最適性を断定しない。
既存のEscape combo、IME、Backspace、Enter、Settingsへの到達を確認する。
不具合候補は観測した実装と推測を分け、自動修正しない。
最後に推奨案と理由、短い試用手順を示す。

## 相談と実装を扱う

- 「どう思う」「案を考えて」だけなら相談として扱い、設定ファイルを変更しない。
- 「変更して」「実装して」なら明示範囲の可逆な編集を進める。
  確認済みの案をもう一度承認待ちにしない。
- 1回の試用では配置とtapping-term等を同時に大きく変更しない。
- 変更後はall layerのbinding、表示表、v0.3互換性を確認する。
  ビルドできない/実機がない場合は検証の限界を明示する。
- 新しい好みを恒久的な事実として保存する前に、ユーザーが採用した判断か確認する。
  相談中の案を現在keymapとして扱わない。

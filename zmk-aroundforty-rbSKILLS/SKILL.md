---
name: zmk-aroundforty-rb
description: >
  AroundFortyRB専用のZMK設定編集・診断。KonjoMitsuki/zmk-config-AroundFortyRBの
  keymap、behavior、レイヤー、JIS/US記号、IME、PMW3610、AML、スクロール、
  split BLE、Studio、GPIO、Kconfig、ビルドを変更・調査する際に使う。
  P1C1～P4C11の物理座標指定に対応する。配置の比較相談には
  aroundforty-keymap-designも併用する。
---

# AroundFortyRB 開発

## 現行状態を確定する

1. 作業対象のチェックアウトまたはユーザーが指定したGitHubリポジトリを特定する。
   対象は https://github.com/KonjoMitsuki/zmk-config-AroundFortyRB 。
   このSKILL自身の保存先を作業リポジトリと混同しない。
2. Gitの差分、ブランチ、コミット、適用されるAGENTS.mdを確認する。
   未コミット変更を保持する。ローカル変更があればremote mainより優先する。
3. `rg --files --hidden -g '!.git' -g '!build*'` で実ファイルを探す。
   パスやレイヤー番号を参照資料だけから決めない。
4. keymap、build.yaml、west.ymlを読み、関連するoverlay/conf/dtsi/Kconfigと
   behavior定義を読む。参照資料は2026-10-07時点の案内であり実装を正とする。
5. ZMKは現在v0.3.0。汎用ZMK/Zephyr SKILLの新しい例を使う際は、
   manifestが取り込むZephyr、モジュールの実binding/Kconfigで互換性を確認する。
   調整に便乗したZMK・モジュールのアップグレードを行わない。

## 必要な参照を読む

- 構造・レイヤー・依存関係: [architecture.md](references/architecture.md)
- 物理座標と0始まりbinding index: [layout.md](references/layout.md)
- hold-tap、IME、combo、AML切替: [behaviors.md](references/behaviors.md)
- PMW3610、AML、scroll processor: [pointing.md](references/pointing.md)
- ターゲット・検証・表生成: [build.md](references/build.md)
- 左右confの実値と設定の注意: [conf-options.md](references/conf-options.md)

## キーマップを変更する

1. 要求されたレイヤー・物理座標を実bindingに対応付ける。
   同じ文字が複数箇所にあれば文字だけで編集対象を推定しない。
   binding数は現在42。表示上の空白をbindingとして追加しない。
2. 対象座標を全レイヤーで照合する。透過先、OS base、
   layer-tapの保持中、AMLの一時有効化を調べる。
   `&trans`と`&none`を区別する。
3. `&lt`/`&mt`/独自behaviorの引数をその定義の順に解釈する。
   空白の幅や物理行からindexを数えず、phandle単位で数える。
4. WinはJIS、MacはUSという現行前提を確認する。
   `JP_*`は現在keymap内のdefineが有効。
   keys_ja.hと名前・値が異なるため、無条件でincludeを追加しない。
   ホスト配列とIME設定が不明なら出力文字を確定したと言わない。
5. layer番号を変えるときはkeymap内の全参照、overlayのlayers、
   temp-layer、Settings/macro、RGB関連設定を検索する。
   AML-Off（11）は全transでもポインター動作を変更するため削除しない。
6. 明示された変更を最小差分で実装する。配置相談だけなら編集しない。
   具体的な編集指示があれば通常の可逆な編集を再承認待ちにしない。

## ハードウェア・設定を変更する

- Right=Central、Left=Peripheralを現行Kconfigで確認する。
- PMW3610とStudioは主にR側を調べる。BLEや電源は両側を比較し、
  roleに応じて適用する。「左右の全設定を同値にする」を規則にしない。
- CPI変更とscaler変更を区別し、一度に一つの変数を調整する。
  `1/45`は乗数。分母を小さくすると入力あたりの出力が増える。
- GPIO、SPI、pinctrlの既存配線を保持し、回路未確認の配線変更を推定しない。
- firmwareの書き込み、settings_reset、BT_CLR/BT_CLR_ALL、
  Studio設定の消去は通常のテキスト編集検証に含めない。
  必要な場合は効果と再ペアリング等の手順を説明してユーザーの指示に従う。

## 検証して報告する

1. 全レイヤーのbinding数、behaviorの引数数、定義済ラベル、
   layer参照、combo/index、AML excluded-positionsを確認する。
2. keymap表を更新する必要があれば、生成スクリプトを読む。
   一時ファイルへ生成し、12レイヤーと42座標が失われないか確認してから反映する。
   詳細はbuild.mdに従う。
3. 利用可能なv0.3対応ツールチェーンでR/Lをビルドする。
   利用できなければ静的確認までと明示する。CI成功を推定しない。
4. 実機でのIME、連打、roll、hold、scroll、AML復帰、BLEの確認項目を示す。
   static/build検証と実機検証を区別する。
5. 変更ファイル・座標・旧→新binding・理由・検証結果を短く報告する。

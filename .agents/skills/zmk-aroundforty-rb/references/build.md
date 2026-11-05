# ビルド・静的確認・表示表

確認版: 23966d9、2026-10-14。

| board | shield | snippet |
|---|---|---|
| seeeduino_xiao_ble | AroundForty-RB_R rgbled_adapter | studio-rpc-usb-uart |
| seeeduino_xiao_ble | AroundForty-RB_L rgbled_adapter | なし |
| seeeduino_xiao_ble | settings_reset | なし |

workflowはpush/pull_request/workflow_dispatchで起動し、
zmkfirmware/zmk/.github/workflows/build-user-config.yml@v0.3を使用する。
ユーザーがCIの実行やPRを依頼した場合はこのmatrixを確認する。
ローカルでv0.3対応west workspaceが既に構築済みなら、実際のzmk/appから例えば:

```bash
west build -p always -b seeeduino_xiao_ble -d build/aroundforty-r -s app \
  -S studio-rpc-usb-uart -- \
  -DZMK_CONFIG=/absolute/path/to/zmk-config-AroundFortyRB/config \
  -DSHIELD="AroundForty-RB_R rgbled_adapter"
west build -p always -b seeeduino_xiao_ble -d build/aroundforty-l -s app -- \
  -DZMK_CONFIG=/absolute/path/to/zmk-config-AroundFortyRB/config \
  -DSHIELD="AroundForty-RB_L rgbled_adapter"
```

上例はzmkリポジトリrootをcwdとする。app配下からなら-sとbuild出力先を調整する。
west/toolchain/manifestの準備なしに、設定repoだけでビルドできると言わない。
settings_resetはmatrixのfirmware生成と実機への書込みを区別する。

## 静的確認

- keymap nodeからlayerを列挙する。現行の全nodeについて、各42phandle bindingを確認する。
- 行は10/10/11/11。コメントを除外し、binding引数をキー数として数えない。
- layer参照、ラベル、binding-cells、comboとexcluded indexを照合する。
- dtsiのtransformとphysical attrsも42個あることを確認する。
- 構文確認・ビルド・実機確認を別々に報告する。

## 表生成の注意

`tools/zmk_keymap_physical.py`が表生成の本体。
`tools/around_forty_rb_mapping.json`のrowsとbinding_orderを使い、
各レイヤー42bindingをL/R座標の物理図へ並べ直す。
`keymap-physical-template.md`とreferences/layout.mdの座標・キー対応も照合する。
座標名を変更する際はmappingと生成ツールを同時に更新する。

`scripts/generate_keymap_table.py`は同じ生成ツールへの互換入口。
`--apply`なしでは出力のみ、指定するとバックアップを作って反映する。
一時出力で現行の全layer/各42位置を確認してからkeymap.mdへ反映する。
JIS/USの記号表示は現行keymapのdefineとmappingの表示ルールも確認する。
生成物の確認だけではfirmwareビルドの代わりにならない。

## MarkdownとHTMLの同期

キーマップ変更時は`keymap.md`だけでなく
`Data/Firmware/aroundforty_rb_keymap.html`も同時に更新する。

```bash
python3 tools/zmk_keymap_physical.py --src config/AroundForty-RB.keymap \
  --map tools/around_forty_rb_mapping.json --out /tmp/aroundforty-keymap.md
```

Markdownの生成結果を確認して反映する。生成ツール内の共通操作説明も、
変更したbehaviorと一致させる。現在HTML用の生成スクリプトはないため、
HTMLの該当キー・説明を直接更新する。Markdown用スクリプトでHTMLを上書きしない。

- HTMLの`section id="lN"`とナビゲーションを現行レイヤー順・名前に揃える。
- 各レイヤー内の42個の`.k`をbinding順で照合し、座標はmappingで確認する。
- 表示ラベル、タップ／長押しの補足、`data-tooltip`の機能説明を更新する。
  アプリ側の割当が不明なキーは、送信キーと設定依存である旨を示す。
- レイヤー切替やAMLの挙動・LED表示が変わった場合は、ページ内の説明も揃える。
- 座標情報、テーマ対応、ツールチップ、キーボード操作など既存機能を維持する。
- Markdown・HTMLとも現行の全レイヤーがあり、各42キーが残っていることを確認する。

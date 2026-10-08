# ビルド・静的確認・表示表

確認版: 000496ebf、2026-10-07。

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

- keymap nodeからlayerを列挙する。現在12node、各42phandle binding。
- 行は10/10/11/11。コメントを除外し、binding引数をキー数として数えない。
- layer参照、ラベル、binding-cells、comboとexcluded indexを照合する。
- dtsiのtransformとphysical attrsも42個あることを確認する。
- 構文確認・ビルド・実機確認を別々に報告する。

## 表生成の注意

既存references/layout.mdはDのlt_typ、settings_hold_tap、AML-Off等が古い。
keymap.mdやhtmlも生成物なのでsource keymapより優先しない。

scripts/generate_keymap_table.pyはdisplay-nameを持つlayerのみ抽出し、
2個以上の空白でbindingを分割する。現行6/8を落とし、
1空白で記述されたAML-Offを誤って数える可能性がある。
そのまま--applyでkeymap.mdを置換しない。

tools/zmk_keymap_physical.pyも未登録custom behaviorやJIS表示の解釈を確認する。
どちらもコードを読み、--helpを確認し、一時出力で12layer/42位置を照合する。
必要ならユーザーの変更範囲内でparserを修正するか、手動で該当表を更新する。
JIS baseのLEFT_BRACKETを印字された「[」と断定しない。
生成物の確認だけではfirmwareビルドの代わりにならない。

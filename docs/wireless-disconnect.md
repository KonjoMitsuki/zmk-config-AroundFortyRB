# Analytics版の無線切断調査

## 報告された症状

2026-11-14: Analytics版で、入力中にも左右とも入力できなくなる。
発生までの時間は約1分との目安で、右側の電源入れ直し・リセットで復帰する。
実機の切断理由やクラッシュログは未取得。

## 見つかった不具合と修正

`analytics/src/timing.c` のペア間隔ヒストグラムでは、
境界値 `{20, 40, 60, 80, 100}` の5要素に対し、
`bin_index()` にヒストグラムのビン数6を渡していた。
登録ペア（既定では位置0/1）を100〜249ms間隔で押すと、6番目の境界値を
範囲外から読む。読み取った値次第で戻り値が6となり、6要素の
`pair_near` / `pair_overlap` の範囲外にも書き込む。
最後のペア・CPクラスでは統計構造体の末尾も越え得る。

修正前のコードに境界値テストを追加し、AddressSanitizerで
`global-buffer-overflow`（`bin_index()` の2バイト読取り）を再現した。
この不具合は停止や集計値破損の原因になり得るが、実機の切断と
同じ原因であることはまだ確認できていない。

修正では探索する境界数をビン数−1とし、境界数＋1がビン数に
一致することをコンパイル時に検査する。0〜250msの境界前後を
CC / PP / CP両方向で検査し、CIのホスト試験にもASan / UBSanを追加した。
統計の保存形式や物理配置IDの変更は不要。

## 検証

```sh
cmake -S analytics/tests -B /tmp/aroundforty-analytics-check -G Ninja \
  -DANALYTICS_TEST_SANITIZERS=ON
cmake --build /tmp/aroundforty-analytics-check
ctest --test-dir /tmp/aroundforty-analytics-check --output-on-failure
python3 -B -m unittest discover -s analytics/tests -p test_read_poc.py -v
```

ホストC試験3件とPython試験9件は修正後に成功。
ptrace制約のある実行環境ではLeakSanitizerが動かないため、
`ASAN_OPTIONS=detect_leaks=0` を付けてC試験を実行した。
アドレス検査と未定義動作検査は有効のまま。

2026-11-15にローカルビルド環境を構築し、ファームウェアも生成した。
実機での切断再現・解消確認は未実施。

| ビルド | Flash | RAM | 結果 |
|---|---:|---:|---|
| 右 Analytics USB＋保存 | 282,096 B | 171,902 B | UF2生成成功 |
| 左 | 177,500 B | 67,952 B | UF2生成成功 |
| 右 通常版（比較用） | 271,768 B | 89,438 B | UF2生成成功 |

右の実効設定はAnalytics／USB取得／永続化が有効、自動保存600秒、
物理配置IDは `aroundforty-rb:matrix-v1`。左右ともスリープ無効を確認した。
右ELFに計数用wrapperとtiming処理が含まれることも確認した。
既存の非推奨設定や上流コード由来の警告は残るが、ビルドは成功。

構築した環境で追加検証し、上流Combo／hold-tap 25件、実際のsettings／NVSを使う
保存・復元8件、local／peripheral発生元とイベント再投入の計数probe 1件が成功。
メモリ検査付きホストC 3件とPython 9件も、同環境で成功した（合計46件）。

### ローカルビルド環境

環境はGit管理外の `build/local-zephyr/` に隔離した。
ZMKと依存モジュールは既存manifestと同じ版で、リポジトリのmanifestは変更していない。

- ZMK: `edf5c0814fd3ea202e43aad2d68fd32e882a518c`（v0.3.0）
- Zephyr: `dacab4875df72109b96cc8977547a0dc04875bcd`
- SDK: Zephyr SDK 0.16.8 / ARM GCC 12.2.0（公式SHA-256と照合済み）
- Python: 3.10.12、west: 1.5.0
- 旧nanopbが必要とする `pkg_resources` のため、隔離環境のsetuptoolsを80.9.0に固定
- 解決済み依存: `build/local-zephyr/manifest-frozen.yml`
- Python依存: `build/local-zephyr/python-requirements.txt`

再ビルドはリポジトリルートで実行する。

```sh
bash tools/build_local_firmware.sh analytics
bash tools/build_local_firmware.sh left
bash tools/build_local_firmware.sh right
```

各UF2は `build/firmware/aroundforty-r-analytics/zephyr/zmk.uf2`、
`build/firmware/aroundforty-l/zephyr/zmk.uf2`、
`build/firmware/aroundforty-r/zephyr/zmk.uf2` に生成される。
ビルドログは `build/local-zephyr/*build.log`。
書込み用のファイルと検証記録を `build/firmware/wireless-fix-20261115/` にまとめた。

## 書き換え後に確認すること

1. `build/firmware/wireless-fix-20261115/AroundForty-RB_R-analytics-timing-fix.uf2`
   を右側へ書き込む。この修正の反映は右側だけでよい。
   ペアリングや保存統計を消去する操作は必要ない。
2. USBを外して左右のキー・トラックボールを使い、10分以上連続入力する。
   登録ペアの位置0/1を100〜249ms程度離して押す操作も含める。
3. 自動保存が動く10分を越えて使用し、切断しないことを確認する。
   USBをつないでGET / BIGRAMS / SAVEと保存後の再起動・復元も確認する。
4. 再発する場合は、接続先OS、右のリセットからの経過時間、
   切断後にUSB入力・Analytics取得が動くかを記録する。
   通常版との比較とBLE切断理由／クラッシュログで次を切り分ける。

左右のconfでは `CONFIG_ZMK_SLEEP=n`。自動保存は600秒間隔なので、
約1分という目安だけからスリープや自動保存が原因とは判断しない。

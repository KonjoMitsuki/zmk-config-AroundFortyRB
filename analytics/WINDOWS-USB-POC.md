# Windowsで打鍵数とタイミングを確認する

デバッガーは不要です。検証用ファームウェアを右側へ書き込み、USB接続してPythonから集計値を読みます。
この開発モードではアンロック操作は不要です。接続したPCのアプリから集計値を読めますが、文字列や入力履歴を返す機能はありません。この版は位置別の押下数、押下時間、キー間隔、指定ペアのタイミング、有向2連、品質カウンタをキーボード内へ保存し、右側を再起動しても復元します。直近の押下順はRAMだけに保持し、再起動で消えます。最後の保存以降の入力は電源断で失われ得ます。

USB通信とHID／Studio共存は前版で確認済みです。フラッシュ保存・復元はホスト試験に加え、2026-11-04に実機の電源OFF/ONでも確認しました。最初に通常入力ができることを確認してください。
検証版のUSB製品名は `AroundFortyRB Analytics PoC` です。USBシリアルポートが2つ現れることを確認してから、読出しを試します。1つしかない場合は、COM番号を変え続けるのではなく、実際に動いているファームウェアを確認します。

## 1. 用意するファイル

GitHub Actionsの通常のビルドworkflowが成功したら、実行結果の **Artifacts → firmware** をダウンロードして展開します。Analytics対応版は `AroundForty-RB_R-analytics-usb-poc.uf2` です。通常版の右側UF2はAnalytics無効のため、集計には使えません。

読出しスクリプトはUF2には同梱されません。ビルドしたコミットの `analytics/tools/` から、次の3ファイルをWindows側へコピーしてください。

| ファイル | 用途 |
|---|---|
| `AroundForty-RB_R-analytics-usb-poc.uf2` | Actionsで生成する右側用・集計＋USB読出し＋保存・識別用USB名あり |
| `read_poc.py` | Windowsで読み出す／保存するスクリプト |
| `watch_reboot.py` | 保存後に再起動を待ち、復元値を自動比較するスクリプト |
| `windows_usb_descriptor.py` | 実機のUSB構成を読む診断ツール |

これらをWindows側のフォルダへコピーします。WSL上のリポジトリなら、エクスプローラーからそのWSLファイルへアクセスできます。
左側はまず現在のファームウェアのまま試します。新しい分割通信の仕様は追加していませんが、現在の左側との互換性は実機で確認が必要です。

## 2. 右側を書き換える

1. 可能なら、現在使っている右側UF2を戻すために手元へ残します。Actionsの通常版も戻すために使えますが、現在の実機のファームウェアそのものではありません。
2. 右側をUSBでWindowsへ接続し、普段のファームウェア更新時と同じ方法でブートローダーを開きます。
3. 表示されたドライブへ **R用** `AroundForty-RB_R-analytics-usb-poc.uf2` をコピーします。L側へR用UF2を書かないでください。
4. 再起動後、文字入力とトラックボールが動くことを確認します。

settings_resetやペアリングの消去は不要です。戻す場合も同じ方法で元の右側UF2またはActionsの通常版を書き込みます。

## 3. PowerShellから読む

Python 3が必要です。`read_poc.py` のあるフォルダでPowerShellを開き、実行します。

```powershell
py -m pip install pyserial
py .\read_poc.py --list
```

Studio用とは別にAnalytics用のCOMポートが追加されます。Windowsのデバイスマネージャーで、接続前後の「ポート (COMとLPT)」を確認してください。ポート説明にAnalyticsが表示されるかはWindows依存で、番号の大小だけでは判別できません。

Studioに接続しているCOMは使わず、もう一方のCOM番号を指定します。以下の `COM7` は例です。

```powershell
py .\read_poc.py --port COM7
```

番号が分からなければ `--list` の結果をそのまま教えてください。スクリプトは全ポートへ自動送信しません。
「no complete snapshot」ならポートの選択、USB接続、他アプリが使用中でないかを確認します。

`pip install` が成功しても読み込めない場合は、`py -3.11 .\read_poc.py --list` のように、インストールしたPythonの版を明示してください。修正版は別のPythonを選ばせる先頭のshebangを削除し、読み込み失敗時に実行中のPythonと例外を表示します。

開発者は次の読取り専用コマンドで、Windowsのキャッシュ情報ではなくUSBハブ経由の実機descriptorを取得できます。ドライバーの置換や設定変更は行いません。

```powershell
py -3.11 .\windows_usb_descriptor.py
```

通常のStudioのみの構成はinterface数3（CDC control/data＋HID）、Analytics追加版は5（CDC 2組＋HID）です。USB製品名も併記します。

## 4. 最初の確認

1. 一度取得して位置0／1の `left/peripheral` の値を控えます。
2. Qをゆっくり10回押して離し、再取得します。位置0が10増えることを確認します。
3. Q+WのEsc Comboを10回押して離し、再取得します。位置0と位置1がそれぞれ10増えることを確認します。
4. 普通の入力・トラックボール・Studioも以前と同じように動くか確認します。

取得コマンド自体をキーボードで入力すると、その分も打鍵数に入ります。**試行後はPowerShellの↑で前のコマンドを呼び出す**と、Q/Wを余分に入力せずに再取得できます。

結果は「Qだけ10回後の増分」「Combo10回後の増分」「入力・トラックボール・Studioの異常の有無」を教えてください。差が合わなくても、通信欠落・ポート不調・再投入の問題を切り分ける材料になります。

### タイミング・遷移・直近順序

同じポートで、次のコマンドも試せます。取得コマンド自体の打鍵も集計に入ります。

```powershell
py .\read_poc.py --port COM7 --bigrams
py .\read_poc.py --port COM7 --recent --recent-limit 100
```

- `--bigrams` は「A→B」の回数を多い順に表示します。Qの次にWを押す操作を10回行い、位置0→1の回数が10増えることを確認します。
- `--recent` は直近の押下を古い順に、連番・起動後の経過ms・位置・発生元で表示します。Q、W、Eの順に押したとき、その順と間隔が表示されることを確認します。
- 通常の `GET` は押下時間・同一キー間隔・直前押下からの間隔・指定ペアのヒストグラムを表示します。位置0のQをゆっくり5回押すと、`hold`と`same_key`の合計が5増えます。
- 指定ペアの表示は `CONFIG_ZMK_ANALYTICS_PAIR_LIST` に含まれる組だけです。既定は`0:1`です。
- `timing_discards`、`duplicate_downs`、`orphan_releases`、`pair_config_errors` は0のままが期待値です。増えた場合は、その値と操作内容を控えてください。

`RECENT`はRAMだけなので、再起動後は空になります。保存・復元の判定には使わず、`GET`と`BIGRAMS`で確認します。

## 5. 保存と再起動の確認

`--save` はPCのファイルではなく、**キーボードのフラッシュ**へ保存します。`--out` は取得したJSONをPCへ保存する別の操作です。

```powershell
py .\read_poc.py --port COM7 --save
```

表示される `saved: true` と `storage` を確認します。`dirty: false`、`state: ok` なら、その時点のRAM集計が保存済みです。

復元を確実に確認するには、USBの抜き差しだけでは不十分です。バッテリー駆動のままUSBだけ抜くと、右側が動き続けてRAMが残る場合があります。次の順序で、保存後の未保存差分が再起動で消えることを確認します。

1. `--save` で保存し、`generation`、対象位置の値、timingの合計、`BIGRAMS`の遷移回数を控えます。
2. 保存せずにQを3回押します（RAM上だけ増えます）。
3. 右側を電源OFF/ONまたはリセットします。
4. キーボードを触らず、`GET`と`BIGRAMS`で取得します。

`generation` が保存時と同じで、Q×3が加算されておらず、`dirty: false` なら復元成功です。`BIGRAMS`も保存時の遷移回数と一致することを確認します。再起動後に行った入力は復元値へ加算されます。自動保存は変更がある場合だけ10分間隔です。すぐに確認したいときは `--save` を使ってください。

この手順を自動化した `watch_reboot.py` も同梱しています。保存、COMポートの切断・再接続待ち、再起動後の比較をまとめて行います。

```powershell
py .\watch_reboot.py --port COM7 --timeout 600
```

`watch_reboot.py` の実行中はキーボードを触らないでください。保存後にQを3回押し、電源を切って入れ直します。`PASS: statistics and bigrams restored exactly at the same generation` が出れば成功です。

2026-11-04の実機確認では、世代2を保存後にQ×3を未保存のまま電源OFF/ONし、再起動後も世代2、`dirty: false`、位置0のPeripheral押下数8が保存時と一致しました。

うまくいかない場合に確認する値:

| 表示 | 意味 |
|---|---|
| `state: loading` | 起動直後でsettings読込みが終わっていない。数秒待って再取得 |
| `state: no_valid_snapshot` | まだ一度も保存していない。`--save` を実行 |
| `state: incompatible_layout` | 保存時の物理配置IDが現在のファームウェアと違う。上書きせず停止する |
| `state: error` | 保存レコードが両方とも壊れている、または読書きに失敗した。`last_error` を控える |
| `recovered: true` | 片方の破損を検出し、有効な旧世代から復元した |
| `saved: false, error: -11` | 10秒の保存間隔制限中、またはまだ読込み中。10秒以上待って再実行 |
| `saved: false, error: -13` | 破損または配置不一致のため上書きを拒否した |

JSONを保存したい場合だけ、次のように指定します。同名ファイルは上書きしません。

```powershell
py .\read_poc.py --port COM7 --out trial-01.json
```

## 開発者向けビルド差分

既存の右側ビルドに `ZMK_EXTRA_MODULES` でリポジトリ直下と `analytics/` を追加します。直下のモジュールには状態LEDが入っています。保存も有効にする場合は、設定ファイルのリストへ `analytics/persistence.conf` を追加します。StudioのUSB snippetは残します。

```sh
-DEXTRA_CONF_FILE="$REPO/analytics/usb-poc.conf;$REPO/analytics/persistence.conf" \
-DEXTRA_DTC_OVERLAY_FILE="$REPO/analytics/usb-poc.overlay"
```

プロトコルは専用CDCへの1行コマンドと1行JSONです。`GET\n`は押下数・品質・timing、`BIGRAMS\n`は有向2連、`RECENT\n`はRAM上の直近順序、`SAVE\n`は`{"saved":true,"error":0}`を返します。JSONの`version`は3で、クライアントは旧version 1と2も読めます。不明なコマンドはエラーのみ返します。消去コマンドはありません。集計は1回コピーしてから送信し、通信中に更新された値を混ぜません。途中のJSONはクライアントが採用しません。保存形式と障害時の扱いは [PERSISTENCE.md](PERSISTENCE.md) を参照してください。

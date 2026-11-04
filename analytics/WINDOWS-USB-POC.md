# Windowsで打鍵数を確認する

デバッガーは不要です。検証用ファームウェアを右側へ書き込み、USB接続してPythonから集計値を読みます。
この開発モードではアンロック操作は不要です。接続したPCのアプリから集計値を読めますが、文字列や入力履歴を返す機能はありません。統計はRAMのみで、右側を再起動すると消えます。

**AnalyticsのUSB通信・HID／Studio共存はまだ実機未確認です。** ビルドは成功していますが、最初に通常入力ができることを確認してください。
検証版のUSB製品名は `AroundFortyRB Analytics PoC` です。USBシリアルポートが2つ現れることを確認してから、読出しを試します。1つしかない場合は、COM番号を変え続けるのではなく、実際に動いているファームウェアを確認します。

## 1. 用意するファイル

識別用USB名を付けた修正版は `build/analytics-windows-poc-v2/` に用意しています。

| ファイル | 用途 |
|---|---|
| `AroundForty-RB_R-analytics-usb-poc-v2.uf2` | 右側用・USB読出し付き・識別用USB名あり |
| `AroundForty-RB_R-analytics-off.uf2` | 同じソースから作った右側用・計測なし |
| `read_poc.py` | Windowsで読み出すスクリプト |
| `windows_usb_descriptor.py` | 実機のUSB構成を読む診断ツール |

これらをWindows側のフォルダへコピーします。WSL上のリポジトリなら、エクスプローラーからそのWSLファイルへアクセスできます。
左側はまず現在のファームウェアのまま試します。新しい分割通信の仕様は追加していませんが、現在の左側との互換性は実機で確認が必要です。

## 2. 右側を書き換える

1. 可能なら、現在使っている右側UF2を戻すために手元へ残します。同梱OFF版は今回のソースからのビルドで、現在の実機のファームウェアそのものではありません。
2. 右側をUSBでWindowsへ接続し、普段のファームウェア更新時と同じ方法でブートローダーを開きます。
3. 表示されたドライブへ **R用** `AroundForty-RB_R-analytics-usb-poc-v2.uf2` をコピーします。L側へR用UF2を書かないでください。
4. 再起動後、文字入力とトラックボールが動くことを確認します。

settings_resetやペアリングの消去は不要です。戻す場合も同じ方法で元の右側UF2または同梱OFF版を書き込みます。

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

JSONを保存したい場合だけ、次のように指定します。同名ファイルは上書きしません。

```powershell
py .\read_poc.py --port COM7 --out trial-01.json
```

## 開発者向けビルド差分

既存の右側ビルドに `ZMK_EXTRA_MODULES` で `analytics/` を追加したうえで、次の2引数を指定します。StudioのUSB snippetは残します。

```sh
-DEXTRA_CONF_FILE="$REPO/analytics/usb-poc.conf" \
-DEXTRA_DTC_OVERLAY_FILE="$REPO/analytics/usb-poc.overlay"
```

プロトコルは専用CDCへの `GET\n` → 1行JSON（protocol `zmk-analytics-poc`、version 1）。不明なコマンドはエラーのみ返します。読出し専用で、消去・設定変更のコマンドはありません。集計は1回コピーしてから送信します。通信中に更新された値を混ぜず、途中のJSONはクライアントが採用しません。

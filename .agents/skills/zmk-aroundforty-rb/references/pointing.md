# PMW3610とAML

確認版: 23966d9、2026-10-14。R.overlayを実装の正とする。

| 項目 | 現行値 |
|---|---|
| device | pixart,pmw3610、SPI0 |
| CPI | 400 |
| SPI最大周波数 | 2000000 |
| CS | gpio0 9、active low |
| IRQ | gpio0 2、active low + pull-up |
| SCK | P0.05 |
| MOSI/MISO | 両方P0.04。推定で別pinに分離しない |
| wake | force-awake |
| report interval | R.confで15ms |
| AML | layer 10、2000ms |
| excluded positions | 7 8 16 17 18 19 28 29 30 |

通常input-processorsはX反転 → XY scaler 1/1 → AML temp-layer。
layer別overrideは記載順に6/7、8、11。
6/7はXY→scroll、scroll Y反転、scroll scaler 1/45。
8はX scaler 0/1 → XY→scroll → Y反転 → scaler 1/45。
11はX反転とXY scaler 1/1で、AML temp-layerを含まない。
overrideの実際の優先規則・継承はv0.3実装で確認する。

| excluded index（0始まり） | 座標 |
|---|---|
| 7 | R1C3 |
| 8 | R1C4 |
| 16 | R2C2 |
| 17 | R2C3 |
| 18 | R2C4 |
| 19 | R2C5 |
| 28 | R3C4 |
| 29 | R3C5 |
| 30 | R3C6 |

excluded位置とAMLで非transの位置は必ずしも一致しない。
AMLのscroll upはindex8/R1C4、MB3はindex29/R3C5で、
どちらも現在のexcluded一覧に含まれ、押してもAMLを維持する。
挙動変更を提案する際はtemp-layerのkeyイベント処理を確認し、
実機でレイヤー解除のタイミングを検証する。勝手に同期させない。

CPIとcursor scalerはカーソル出力に関わり、scroll scalerはスクロール変換後に働く。
感度調整時はどのmodeの速度を変えるか示す。`1/45→1/30`は約1.5倍。

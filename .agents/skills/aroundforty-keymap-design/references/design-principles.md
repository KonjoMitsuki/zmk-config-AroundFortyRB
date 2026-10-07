# 設計と試用の基準

## 課題別の観点

| 課題 | 確認する操作 | 比較する案 |
|---|---|---|
| []{}() | 配列、Vim移動、IME中の括弧 | 対称配置、同layer隣接、頻出対だけ近づける |
| 数字 | 単発、連続、F13、算術記号 | D長押し、Enter長押し、既存Num内の変更 |
| Ctrl/Shift誤爆 | 単語中のA/Z、roll、Ctrl shortcut、長押し連打 | 配置を維持し1設定調整、専用mod利用、必要なら位置変更 |
| navigation | browser、editor insert mode、Vim normal mode | 現行逆T、HJKL対応、用途ごとの現行配置保持 |
| AML | pointer直後の文字、click、drag、scroll | excludedと解除条件確認、button位置の最小移動、ON/OFF操作改善 |
| IME | 日本語/英語切替、修飾キー残留 | LANGキー、既存macro、ホスト設定に合わせる |

HJKLの矢印化は比較候補。Neovimを使うことだけを理由に現行逆Tを変更しない。
Home Row Modsを全面導入することを前提にしない。
親指と人差し指の担当はmappingの概略であり、実際の運指はユーザーに合わせる。

## 1変更を試す

1. 変更前に実際に困る操作を2～3個挙げる。
2. 配置1か所かhold-tap設定1項目を変える。
3. 日本語、コード、terminal、browserで同じ操作を試す。
4. A/Z/Dの連打・roll、layer hold中の連続入力、
   pointer直後のtyping、click/drag/scroll、AML-Offからの復帰を確認する。
5. 使用感を聞き、採用/調整/撤回を決める。
   firmwareの実機書込みをエージェントが完了したと主張しない。

OS互換性が課題ならJIS/US両方の出力とMac base復帰を別に検証する。

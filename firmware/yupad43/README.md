# 右側yuPad43用ファームウェア

| ファイル | 対象 |
|---|---|
| `nickey44a_l.uf2` | 左キーボードのみ（パッドなし） |
| `nickey44a_r.uf2` | 右yuPad43通常版、USB Studio対応 |
| `nickey44a_r_debug.uf2` | 右yuPad43診断版、USBシリアルログ有効、Studio無効 |

通常版はスクロール量を初回の3倍に調整。
診断版も入力設定は同じ。指本数・座標・ATI品質警告・通信エラーを確認できる。
実機の指本数誤検出・カーソル停止が解決したことを示すものではない。
診断版を使用後、通常版の右UF2を書けばStudio有効へ戻る。

ZMK: `3931facea7507151a53f209ca4f8a717fa6bd400`

yuPad43ドライバー: `5dfa6d3c480ebb5162f5425fa367823d73ddb13e`（公式例の基準版）

左右通常版・右診断版のローカルビルド成功。
通常版の生成Devicetree・Kconfig検証はPASS。
診断版のUSBコンソール、debugログレベル、Studio無効も生成設定で確認済み。
ZMK由来の非推奨設定・combo配列境界等の警告あり。

初回の左右パッド版は`initial-dual/`に保管。比較用に書き戻せる。

配線・診断手順・ビルド方法は[右側yuPad43](../../docs/dual-yupad43.md)を参照。

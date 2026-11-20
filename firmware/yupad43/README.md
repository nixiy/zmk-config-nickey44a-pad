# 右側yuPad43用ファームウェア（最小構成）

| ファイル | 対象 |
|---|---|
| `nickey44a_l.uf2` | 左キーボードのみ |
| `nickey44a_r.uf2` | 右yuPad43、90度回転、USB Studio対応 |

公式XIAO例の解像度1300×1192、慣性無効、scroll scaler 1/12を使用。
タップ等の個別調整と独自processorを削除し、ドライバーの既定値へ戻した。
診断版は配布対象から削除。実機の指本数誤検出・カーソル停止の解決は未確認。

ZMK: `3931facea7507151a53f209ca4f8a717fa6bd400`

yuPad43ドライバー: `5dfa6d3c480ebb5162f5425fa367823d73ddb13e`

初回の左右パッド版は`initial-dual/`に保管。
配線と設定は[右側yuPad43](../../docs/dual-yupad43.md)を参照。

再確認でNickey44A用Kconfigの左右分割・右central・NFC GPIO設定を復元。
右のUSB初期化設定も実際のshield条件へ適用。左右の通常ビルドと生成設定検証はPASS。
この組み合わせでは左右両方のUF2を更新する。実機の指判定・カーソル動作は未確認。
詳細と監査前状態への戻し方は上記の設定文書を参照。

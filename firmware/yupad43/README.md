# 右側yuPad43用ファームウェア（Nixプロファイル）

| ファイル | 対象 |
|---|---|
| `nickey44a_l.uf2` | 左キーボードのみ |
| `nickey44a_r.uf2` | 右yuPad43、90度回転、USB Studio対応 |

解像度1300×1192、カーソル慣性無効・スクロール慣性有効、scroll scaler 1/16を使用。
タップ等の個別調整と独自processorを削除し、ドライバーの既定値へ戻した。
診断版は配布対象から削除。実機の指本数誤検出・カーソル停止の解決は未確認。

ZMK: `3931facea7507151a53f209ca4f8a717fa6bd400`

yuPad43ドライバー: `58e590adbe448924ac104747ee6366d79f16d04c`

[nix.patch](https://github.com/yuchamichami/yupad43-nickey44a/blob/main/profiles/nix.patch)
を取り込んだ構成。ピン配置・90度回転・キーマップは維持。

初回の左右パッド版は`initial-dual/`に保管。
配線と設定は[右側yuPad43](../../docs/dual-yupad43.md)を参照。

再確認でNickey44A用Kconfigの左右分割・右central・NFC GPIO設定を復元。
右のUSB初期化設定も実際のshield条件へ適用。左右の通常ビルドと生成設定検証はPASS。
この組み合わせでは左右両方のUF2を更新する。実機の指判定・カーソル動作は未確認。
詳細と監査前状態への戻し方は上記の設定文書を参照。

2026-11-21: nix.patch指定のドライバーで左右通常版をローカルビルドし、
生成Devicetree・Kconfigの検証はPASS。UF2を更新。実機での動作は未確認。

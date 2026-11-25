# 慣性スクロール切り替え版（2026-11-25）

- `nickey44a_l.uf2`: 左peripheral。
- `nickey44a_r.uf2`: 右central、yuPad43、USB Studio対応。
- レイヤー3（0始まり）のF位置を押すたびにスクロール慣性をON/OFF。
- 起動時はON。解放時は反転しない。設定のフラッシュ保存は行わない。
- ドライバー: `1c4a52376e8169a5b59ace6e128b177ad46c6073`。
- ZMK: `3931facea7507151a53f209ca4f8a717fa6bd400`。

左右ビルド・生成設定・ホストCテストを確認済み。実機への書き込みと操作は未検証。
既存UF2を保持するため、このディレクトリに別途出力した。

[仕様と実機確認手順](../../../docs/runtime-scroll-inertia.md)

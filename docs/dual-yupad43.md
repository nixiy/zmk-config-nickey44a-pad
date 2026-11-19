# Nickey44A-PAD: 右側yuPad43の最小構成

[公式XIAO設定例](https://github.com/yuchamichami/yuPad43/tree/main/firmware/xiao-example)
を基準に、右centralへyuPad43を1台接続する。左はキーボードのみ。
既存のキー配列・Bluetooth・Studio・バッテリー・sleep設定を維持する。

## ピン配置

| yuPad43端子 | 信号 | MCU GPIO |
|---|---|---|
| 1 | RDY | P0.15、Active Low、Pull Up |
| 2 | RST / MCLR | P0.19へ配線済み。ファームでは駆動せず基板上のpull-upで解除 |
| 3 | GND | 共通GND |
| 4 | 3V3 | 3.3 V |
| 5 | SCL | P1.01 |
| 6 | SDA | P0.09（NFC無効） |

端子番号は部品面を見てケーブル出口を下にしたyuPad43側の番号。
I²Cは100 kHz、アドレス0x56。ドライバーはソフトリセットを使用する。

## 公式例から残した設定

- I²C、input、IQS9151、pointing、smooth scrollingを有効化。
- 解像度1300×1192、カーソルとスクロールの慣性は無効。
- listenerは右PADに1つ。scroll scalerは公式例と同じ1/12。
- 取り付け方向に合わせて90度回転。

タップ・ホールド・ピンチ等の個別上書き、3本指スワイプ抑止processor、
追加のスクロール3倍調整、RST GPIO hog、診断用confを削除した。
ジェスチャの動作は公式ドライバーの既定値に従う。
公式例のUSB専用設定・BLE無効化・Studioと競合するserial無効化は移植しない。

## ビルドと検証

ZMK: `3931facea7507151a53f209ca4f8a717fa6bd400`

yuPad43ドライバー: `5dfa6d3c480ebb5162f5425fa367823d73ddb13e`（公式例の固定版）

`config/west.yml`で依存を取得し、`build.yaml`の通常構成でビルドする。
右側の例（west環境内で実行）:

```powershell
west build -s zmk/app -b seeeduino_xiao_ble -d build/right -- `
  -DSHIELD="nickey44a_r rgbled_adapter" `
  -DZMK_CONFIG=<このrepo>/config `
  -DCONFIG_ZMK_STUDIO=y -DSNIPPET=studio-rpc-usb-uart
```

生成設定の検証:

```powershell
python scripts/check_dual_yupad43.py `
  --zephyr-base <Zephyrのパス> `
  --left-build <左buildディレクトリ> `
  --right-build <右buildディレクトリ>
```

ビルド済みUF2は[`firmware/yupad43/`](../firmware/yupad43/)。
ビルド・設定検証と実機の安定性は別。指本数の誤検出・カーソル停止は未解決。
カバーを固定して触れずに起動し、カーソル・タップ・スクロールを再確認する。
通常更新でペアリング情報の消去は不要。
初回の左右パッド版UF2は`firmware/yupad43/initial-dual/`へ保存してある。

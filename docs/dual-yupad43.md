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

## 現在の設定

- I²C、input、IQS9151、pointing、smooth scrollingを有効化。
- 解像度1300×1192、カーソル慣性は無効、スクロール慣性は有効。
- listenerは右PADに1つ。ジェスチャ割り当てprocessorの後に、Nixプロファイルのscroll scaler（1/16）を適用。
- 横操作はSWIPEモード。2本指の左スワイプで戻る、右スワイプで進む（判定距離200）。横スクロールは出力しない。
- 1〜3本指のタップドラッグは無効。タップによるクリックは有効。
- 取り付け方向に合わせて90度回転。

横操作はドライバーの排他モードで選択し、アクションは右overlayの
`yupad43_swipe_actions`で専用イベントに割り当てる。`codes`と`bindings`は順番に対応する。
横スクロールへ戻す場合は、右confの`CONFIG_INPUT_IQS9151_2F_HORIZONTAL_SWIPE=y`を
`CONFIG_INPUT_IQS9151_2F_HORIZONTAL_SCROLL=y`へ置き換えて再ビルドする。
3本指スワイプは専用イベントとして検出されるが、このFWでは操作を割り当てていない。
追加のスクロール3倍調整、RST GPIO hog、診断用confは使用しない。
公式例のUSB専用設定・BLE無効化・Studioと競合するserial無効化は移植しない。

## ビルドと検証

ZMK: `3931facea7507151a53f209ca4f8a717fa6bd400`

yuPad43ドライバー: `9a8b7b8639136989bdf49bdb715d8330cd31041d`（排他的な横操作モード・専用スワイプイベント対応版）

[nix.patch](https://github.com/yuchamichami/yupad43-nickey44a/blob/main/profiles/nix.patch)
の4ファイル分の変更を取り込み済み。スクロール倍率は従来の1/12から1/16
（出力量75%）へ変更し、スクロール慣性を有効化。追加済みのNFC・USB初期化・
GPIO/pinctrl競合チェックは維持する。左右スワイプのモードと割り当ても生成設定から検証する。

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

## 接続図・競合確認・以前の再確認記録

```text
yuPad43（部品面から見た基板端子）       Nickey44A 右XIAO
  1 RDY  --------------------------> P0.15（入力・Active Low）
  2 RST  --------------------------- P0.19（駆動せず、パッド上でpull-up）
  3 GND  --------------------------- GND
  4 3V3  <-------------------------- 3.3 V
  5 SCL  <-------------------------- P1.01
  6 SDA  <-------------------------> P0.09（NFC無効）

右PAD → 右MCU I²C → yupad43_listener → ZMK HID → USB / BLE
左キーボード → BLE split → 右central → ZMK HID → USB / BLE
```

ホスト側FPC端子番号・接点面・ケーブルの表裏は、回路図がこのリポジトリにないため
未確認。ここでは既存TPS43のGPIO設定とユーザーから確認済みの同一配線を根拠にする。
基板リビジョン・カバーの材質と厚さ・スクロール方向の実測も未確認。

生成Devicetreeで、PADのGPIOと有効なpinctrl参照を確認する。
キーマトリクスはD1〜D10（P0.03/28/29/04/05、P1.11〜15）、
LEDはP0.26/30/06、既存UART0のP1.11/12は無効。
PADのP0.09/P1.01/P0.15/P0.19とは重複しない。
PADは右centralへ直結するため、パッド用input-splitは不要。

### 対象ファイル

| ファイル | 役割 |
|---|---|
| `config/west.yml` | 公式基準のyuPad43ドライバーを追加・固定 |
| `boards/shields/nickey44a/nickey44a_yupad43.dtsi` | 既存配線に合うI²C・GPIO・センサーノード |
| `boards/shields/nickey44a/nickey44a_r.overlay` | PADのlistenerを1つ登録・公式scroll scaler |
| `config/nickey44a_r.conf` | 公式例の基本設定・90度回転 |
| `boards/shields/nickey44a/Kconfig.defconfig` | Nickey44Aの左右分割・右central・NFC GPIO・USB初期化 |

今回、`Kconfig.defconfig`が公式テスト用の`SHIELD_YUPAD43_TEST`に置き換わっていたため、
`SHIELD_NICKEY44A_R/L`の条件とsplit設定を復元した。
右側の`USB_DEVICE_INITIALIZE_AT_BOOT=n`は、実際の右shield条件内へ移した。
ZMKとドライバーのrevision、PADの感度・ジェスチャ個別設定は変更しない。

### 保存と戻し方

再確認前のtrackedファイルはコミット
`9a8619c48d6d15f38a8449a0ae4eb6479af399b0`に保存されている。
さらに`%TEMP%/nickey-yupad43-before-review.zip`へgit archiveで保存した。
これは監査前の状態であり、実機で正常と確認された版ではない。

今回のKconfig修正だけ戻す場合:

```powershell
git restore --source=9a8619c48d6d15f38a8449a0ae4eb6479af399b0 -- boards/shields/nickey44a/Kconfig.defconfig
```

この戻し方はテスト用shield条件に戻すため、比較用に限る。
FWの比較用コピーは`%TEMP%/nickey-yupad43-before-review-fw/`に保存。
通常のUF2書き込みでアプリだけ入れ替え、settings_resetやブートローダー消去は行わない。

### 実機確認の状態

これまでにユーザーから、動作はするが指本数の誤判定とカーソル停止が報告されている。
Arduino診断スケッチは起動メッセージまで確認済み。初期化・校正・指データのログは未取得。
再起動後・端部・無接触時・スクロール方向/速度・キーとの同時使用・USB/BLE再接続は、
今回の設定について実機確認が必要。ビルド成功を動作改善の確認として扱わない。

今回の結果: 左右の通常ビルド成功。右central・左peripheral・NFC GPIO・Studio・
バッテリー・sleep・PADのアドレス/回転/1つのlistenerとGPIO/pinctrl競合の検証はPASS。
UF2は`firmware/yupad43/nickey44a_l.uf2`と`nickey44a_r.uf2`へ更新。
非推奨設定と既存ZMKのcombo配列境界等の警告あり。実機への書き込みは未実施。
ローカルの実行ログは`%TEMP%/nickey-yupad43-build.log`。

## 2026-11-22: 左右スワイプ対応の検証結果

左右通常構成（右Studio有効）のビルド成功。生成DevicetreeとKconfigで、
SWIPEのみ有効、2本指の左＝戻る/右＝進む、縦スクロールと慣性有効、
1〜3本指のタップドラッグ無効を検証した。既存のピン競合・左右の役割・Studio等もPASS。
`firmware/yupad43/nickey44a_l.uf2`と`nickey44a_r.uf2`へ反映済み。
非推奨設定、既存ZMKのcombo配列境界、未使用behavior等の警告あり。
実機への書き込み・スワイプ方向・USB/BLEでの操作確認は未実施。

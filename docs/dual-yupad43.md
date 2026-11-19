# Nickey44A-PAD: 右側yuPad43

現在は右centralにyuPad43を1台接続する構成。左peripheralはキーボードのみ。
左のパッド初期化・input-split転送・central側proxyは無効化した。

## 接続

移行前のTPS43 overlayと同じMCU GPIOを使用する。
部品面を見てケーブル出口を下にしたyuPad43側の端子番号は以下。
ホスト側FPC端子番号とケーブル接点面は実物・回路図で確認する。

| yuPad43端子 | 信号 | MCU接続・設定 |
|---|---|---|
| 1 | RDY | P0.15、Active Low、Pull Up |
| 2 | RST / MCLR | P0.19、入力として予約し開放相当。基板上でHighへプルアップ |
| 3 | GND | 共通GND |
| 4 | 3V3 | 3.3 V電源 |
| 5 | SCL | P1.01 |
| 6 | SDA | P0.09、NFCを無効化 |

I²Cは100 kHz、アドレス0x56。ドライバーはソフトリセットを使用する。
同じセンサーのinput-listenerは右に1つだけ。

## 操作と設定

1本指でカーソル、1本指タップで左クリック、2本指タップで右クリック、
2本指移動で縦横スクロール。旧TPS43の独自操作・軸補正は廃止。
慣性・タップドラッグ・ピンチ・3本指タップは無効。
ドライバーが出す3本指スワイプのBTN_3〜6はホスト出力前に抑止する。

電極10 RX×9 TX、ATI目標400、タッチ閾値30/26、Finger Split Factor 0、
解像度1300×1192は公式例に準拠。回転0度。
スクロール倍率はホスト側processorで1/4（初回の1/12から3倍）。
スムーズスクロールは右central側だけで有効化し、倍率processorは1段のみ。
センサーの電極・校正・指検出閾値は変更していない。

## 不安定な入力の切り分け

初回FWでユーザーから「カーソル移動が不安定、スクロール量が少ない、
1本タップが2本タップになる」と報告あり。カバーあり、右側のみ実装。
カバー材質・厚さ・密着状態および起動時の条件は未確認。

スクロール量は設定で3倍に調整。指本数の誤検出は未解決。
ドライバーはfinger_count=1のときだけカーソルを出すため、
誤って2本と検出されるとカーソル抑止にもつながる可能性がある。
2本指タップを単に無効にしても指本数の誤検出は直らない。

カバーを固定し、パッドから手を離した状態で電源を入れ直す。
再校正後も不安定なら診断版で実測する。

### USB診断版

`firmware/yupad43/nickey44a_r_debug.uf2`は右専用。
通常版と同じパッド設定で、USBシリアルログを有効化しStudioだけ無効化する。
キーボード・マウスのUSB/BLE機能は維持する。診断後は通常版に戻せる。

1. 右に診断版を書き込み、USBでPCへ接続する。
2. 表示されたCOMポートをシリアル端末で開く（115200、8N1、フロー制御なし）。
3. 1本指で移動→離す、1本指タップ、2本指移動を別々に行う。
4. `rel x=... y=... info=... tp=... finger=... f1x=... f1y=... f2x=... f2y=...`
   の行と`ATI`・`frame read failed`等の警告を保存する。

fingerはICが報告した指本数。tpのbit8/9は各指のconfidence。
infoのbit3はATI品質警告。ログによって生の誤検出とホスト側の変換を区別する。
ログを有効にしたことでタイミングが変わる可能性があるため通常版でも比較する。

## 基準版・ビルド・検証

[公式組み込み用プロンプト](https://github.com/yuchamichami/yuPad43/blob/main/docs/zmk-integration-prompt.md)
に照合し、ドライバーのmanifest固定値を公式例の
`5dfa6d3c480ebb5162f5425fa367823d73ddb13e`へ揃えた。
初回の`4c2adbfab1052a8d6dcff58ef15d630d025e7233`との差分はREADMEのみで、
ドライバー・電極設定・Kconfigは同一。版変更自体による動作改善は見込まない。
ZMKは`3931facea7507151a53f209ca4f8a717fa6bd400`、RGBモジュールの固定値も維持。

通常版は`build.yaml`。診断版は`zmk-usb-logging` snippetと
`config/yupad43-debug.conf`を使用する（Studio snippetは付けない）。

```powershell
west build -s <zmk>/app -b seeeduino_xiao_ble -d <build-right-debug> -- `
  -DSHIELD="nickey44a_r rgbled_adapter" `
  -DZMK_CONFIG=<このrepo>/config `
  -DSNIPPET=zmk-usb-logging `
  -DEXTRA_CONF_FILE=<このrepo>/config/yupad43-debug.conf

python scripts/check_dual_yupad43.py `
  --zephyr-base <Zephyrのパス> `
  --left-build <左buildディレクトリ> `
  --right-build <右通常版buildディレクトリ>
```

ビルドと生成設定の検証は、実機での校正・BLE・Deep Sleepの合格とは別。
左右のキー、右カーソル・クリック・スクロール、再接続・復帰も実機で確認する。
既存のペアリング情報やブートローダーは消去しない。

初回の左右パッド版UF2は`firmware/yupad43/initial-dual/`に保管。
元の動作へ比較のため戻す場合は、初回の右UF2を書き込む。

# TPS43設定の移行

現在は右側にyuPad43を使用します。旧TPS43の独自ジェスチャ設定は廃止しました。
現在の配線・操作・検証手順は[右側yuPad43](dual-yupad43.md)を参照してください。

## コードに合わせたピンアサイン

現在の定義元は[`nickey44a_yupad43.dtsi`](../boards/shields/nickey44a/nickey44a_yupad43.dtsi)です。
旧TPS43と同じ配線を使用しますが、現在のセンサー設定はyuPad43用です。

| 信号・設定 | 右側MCUのGPIO／設定 | コード上の定義 |
|---|---|---|
| SDA | P0.09 | `NRF_PSEL(TWIM_SDA, 0, 9)` |
| SCL | P1.01 | `NRF_PSEL(TWIM_SCL, 1, 1)` |
| RDY | P0.15、Active Low、Pull Up | `irq-gpios = <&gpio0 15 (GPIO_ACTIVE_LOW \| GPIO_PULL_UP)>` |
| RST / NRST / MCLR | P0.19（配線コメントのみ。GPIOとして駆動しない） | `rst-gpios`などの設定なし。ドライバーのソフトリセットを使用 |
| I²Cバス | `i2c0`、100 kHz | `I2C_BITRATE_STANDARD` |
| I²Cアドレス | `0x56`（現在のyuPad43） | `reg = <0x56>` |

P0.09はNFCピンのため、`nfct-pins-as-gpios`でGPIOとして使用します。
現在の左側overlayにはパッドの定義がなく、左側はキーボードのみです。

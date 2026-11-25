# キーによる慣性スクロールの切り替え

## 割り当て

behavior名は `&scroll_inertia`（引数なし）。`config/nickey44a.keymap` の
レイヤー3（0始まり、4枚目）のF位置（左手中段・左から5番目）に割り当て済み。
レイヤー1を押しながら、レイヤー1上の `&mo 3` を押してレイヤー3へ入り、
F位置を1回押すたびに慣性スクロールを反転する。

```dts
// レイヤー3のF位置のbinding:
&scroll_inertia
```

押下時に1要求を受理し、解放時は何もしない。起動時は既存の
`CONFIG_INPUT_IQS9151_SCROLL_INERTIA_ENABLE=y` によりON。
カーソル慣性や通常スクロール、その他のジェスチャ設定は変更しない。

## デバイス・実行文脈

- ドライバーrevision: `1c4a52376e8169a5b59ace6e128b177ad46c6073`。
- 右側がcentral、左側がperipheral。対象は右側の `&yupad43`。
- 共通DTSでbehaviorをdisabledとして宣言し、右overlayだけで有効化して
  `device = <&yupad43>` を指定する。左ではドライバーAPIへの参照も生成しない。
- localityはCENTRAL。左キーの押下もsplitのpositionイベントを経由して右側で実行する。
- 通常のキーはZMKのkscan work/event処理からbehaviorに到達する。
  input processor等からの呼び出しも考慮し、behavior内ではAPIを呼ばない。
  APIはシステムworkqueueの遅延workから呼ぶ。IQS9151専用workerとは別スレッド。
- `device_is_ready()` を受理時と処理時に確認する。

## 直列化・再試行・エラー

- このキーボードでは有効なbehaviorインスタンスは1つに制限する。
  複数のキーで同じ `&scroll_inertia` を使ってもよい。
- 処理中を含め最大32要求をカウンターで保持する。workが集約されても
  押下回数は集約しない。33件目以降は空きができるまで `-ENOSPC` で拒否しログに残す。
- 各要求はget成功後だけsetで反転する。独自のON/OFF状態は保持しない。
- get/setの `-EBUSY` は10 ms後に再試行する。初回に加えて最大100回再試行。
  予約する待ち時間は最大合計1秒。workqueueの混雑・スリープによる実際の遅延は別途発生し得る。
- 再試行時はgetからやり直す。APIは失敗時に状態を変えず、set成功時は要求を
  直ちに完了するため、同じ要求を二重適用しない。
- 再試行上限・その他のAPIエラーではその1要求を失敗としてログに残し、後続要求へ進む。
  非readyの押下は受理しない。スケジュール失敗時は保留要求を破棄して件数とエラーを記録する。
- behaviorの戻り値0は非同期要求の受理を意味し、設定変更の成功通知ではない。
  APIエラーが起きた場合、最終状態が物理押下回数の偶奇と一致する保証はない。
- RAM保持のsleep中はドライバーの設定を維持する。MCU再起動時は初期値ONへ戻る。
  永続保存、接続先別管理、状態表示は追加しない。

## 実機確認手順

1. レイヤー3のF位置に割り当てた左右のFWを書き込む。
2. 電源投入後、スクロールして離指し、慣性ONを確認する。
3. 1回押して離し、通常スクロールは動くが離指後の慣性が出ないことを確認する。
4. 次の押下でONに戻ること、長押し・解放だけで追加反転しないことを確認する。
5. 慣性動作中にOFFにし、新しい慣性出力が止まることを確認する。
   既に入力/HIDキューへ渡ったイベントやOS側の慣性は取り消せない。
6. OFF→ON後、指を動かさず待ち、古い慣性が再開しないことを確認する。
7. 素早く奇数回／偶数回押し、処理完了後の結果を確認する。入力競合時の
   再試行上限・queue full・API失敗はログと照合する。
8. OFFにしてRAM保持sleepから復帰し、OFFの維持を確認する。
9. カーソル、通常スクロール、タップ、既存のドラッグ・ピンチ設定、横スワイプを確認する。
10. 同じbehaviorを左キーと右キーへ一時的に割り当て、どちらも右PADを操作することを確認する。

Windows/Mac、USB/BLEの両方で確認する。ビルド・自動テストは実機確認を代替しない。

## 変更ファイル

| ファイル | 内容 |
|---|---|
| `src/behavior_scroll_inertia.c` | 押下要求の受付、非同期反転、再試行・エラー処理 |
| `dts/bindings/behaviors/zmk,behavior-scroll-inertia.yaml` | 引数なしbehaviorと対象deviceのbinding |
| `boards/shields/nickey44a/nickey44a.dtsi` | 左右共通のdisabledノード |
| `boards/shields/nickey44a/nickey44a_r.overlay` | 右centralだけで有効化、対象PAD指定 |
| `CMakeLists.txt` / `zephyr/module.yml` | Cソースとbinding検索パスの登録 |
| `config/west.yml` | API対応ドライバーへのrevision更新 |
| `scripts/check_dual_yupad43.py` | 既存設定検証にbehaviorの実行側・phandle確認を追加 |
| `tests/scroll-inertia/` | 実装Cを読み込むホストテストとZephyr/APIのモック |
| `README.md` / `docs/dual-yupad43.md` / 本書 | 設定方法・仕様・確認手順 |

## 2026-11-25の検証結果

- FWリポジトリ内に `AGENTS.md` は見つからなかった。
- manifestの旧revisionとローカルcheckoutを確認し、クリーンなドライバーcheckout
  `1c4a52376e8169a5b59ace6e128b177ad46c6073` をビルドに使用した。ドライバー自体は変更していない。
- ZMKはmanifestと同じ `3931facea7507151a53f209ca4f8a717fa6bd400`。
  Zephyr 3.5.0 / SDK 0.16.3、`seeeduino_xiao_ble`。
- 通常構成の左右ビルド成功（右はStudio有効）。初期化順序チェックも成功。
- 別の一時キーマップでレイヤー3の左A位置と右Y位置に同じbehaviorを割り当て、
  独立したビルドディレクトリで左右ともビルド成功。生成DTSへの割り当て反映も確認。
  その後、ユーザー指定により本キーマップのレイヤー3のF位置へ割り当てた。
- 生成設定チェック成功：右central・左peripheral、右だけPADとbehaviorが有効、
  初期慣性ON・カーソル慣性OFF、既存のピン・横スワイプ・通常入力設定を確認。
- ホストCテスト実行成功：押下／解放、連打、容量超過、get/setのEBUSY、
  再試行上限、その他のAPIエラー、非ready、処理途中の追加押下、ドライバー状態の参照。
  Zephyrの実スケジューラーや無線通信の実行テストではない。
- 既存の非推奨設定等の警告あり。実機への書き込みは行っていない。
  上の実機確認手順1〜10の操作・使用感は未検証。
- レイヤー3のF位置への最終割り当て後も左右ビルド・生成設定を確認。
  既存のUF2は上書きせず、成果物は `firmware/yupad43/runtime-inertia/` に保存。
  コミット・pushは行っていない。

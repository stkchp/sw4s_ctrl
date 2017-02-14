# sw4s_ctrl

U2H-SW4Sの電源管理ユーティリティ(Linux用)

## 説明

ELECOMの[U2H-SW4S](http://www2.elecom.co.jp/cable/usb-hub/u2h-sw4s/)をLinuxから
操作する為のユーティリティ。現在のON/OFFの状態やそれぞれのポートをON/OFFすること
ができる。
(なお、製品自体は販売終了しており、後継機は出ていない模様。)


## デモ

- 現在の状態を表示
```
 $ sw4s_ctrl show
available device:
/dev/hidraw0

target device:
/dev/hidraw0

status:
port1=on
port2=on
port3=on
port4=on
```
- 2番ポートをOFF
```
 $ sw4s_ctrl -2 off
available device:
/dev/hidraw0

target device:
/dev/hidraw0

Command successful.
```
- 再度状態を表示
```
 $ sw4s_ctrl show
available device:
/dev/hidraw0

target device:
/dev/hidraw0

status:
port1=on
port2=off
port3=on
port4=on
```

## 関連

iteclubさんが公開されていた`sw4ctrl`がいつの間にか消えていたので、
習作として本ユーティリティを作成。udevがあればきっと動くはず。

一通り動いたところで[nonakap/u2hsw4ctl](https://github.com/nonakap/u2hsw4sctl)を発見...orz


## 動作環境

- udev
- U2H-SW4Sがhidrawとして認識されるkernel(作者はkernel 4.9.6で動作確認)
- sw4ctrlが動いていたPCであればきっと動くはず


## 使い方

- `sw4s_ctrl -h` 出力
```
Usage: sw4s_ctrl [options] show
   or: sw4s_ctrl [options] on
   or: sw4s_ctrl [options] off

Options: [ d1234h ]
  -d device		target hidraw device
  -1,-2,-3,-4		target port(on/off only)
```

基本的に `sw4s_ctrl -1 on` `sw4s_ctrl -1 off` のように使用する。
復数デバイスがある場合は`sw4s_ctrl -d /dev/hidraw2` のようにデバイス指定が可能。


## インストール

- githubからダウンロード
- `make`
- 所定の場所に置く

/dev/hidraw0等にアクセスする権限が必要。
sticky bitを立てるか、デバイスの権限をudevあたりで変更するなりする。

## ライセンス

[GPLv3](COPYING)


## 作者

- [stkchp](https://github.com/stkchp)

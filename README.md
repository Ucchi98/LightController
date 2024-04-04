# Plant Cultivation Light Controller

植物育成ライトコントローラー

## Getting Started

植物育成ライトを日出・日没時刻に合わせて点灯・消灯を制御するハードを制御するプログラムです。
ESP32C3 で動作します。

日出・日没時刻は国立天文台の Web サイトより12年分を確認し１分程度の差しかないため、
2024年のデータを毎年使用することにしました。
時刻は Wifi に接続して SNTP サーバに同期するようにしています。
Web サーバを内蔵させて、PC やスマホのウェブブラウザからアクセスして、
ライトの点灯状態の確認や、点灯状態の設定を行えるようにしました。

回路の作製は [Youtube](https://www.youtube.com/@UCCHI08) で紹介しています。

### 準備

Expressif Systems 社の ESP32 開発環境である esp-idf が必要です。

- [esp-idf](https://github.com/espressif/esp-idf)

### ビルド

    git clone https://github.com/Ucchi98/LightController.git
    cd light_controller

ビルド対象の CPU を設定

    idf.py set-target esp32c3

使用する Wifi Access Point の SSID と パスワードを設定

    idf.py menuconfig

ビルド

    idf.py build

書き込み

    idf.py flash

## 動作確認

以下のような ESP32C3 を搭載した開発ボードがあれば動作確認をすることができます。

  - [Seeed Studio XIAO ESP32C3](https://akizukidenshi.com/catalog/g/g117454/)

モニターを起動しておき、内臓 Web サーバにブラウザでアクセスすれば動作を確認することができます。

モニターで割り当てられた IP アドレスを確認

    idf.py monitor

ウェブブラウザで内臓 Web サーバにアクセス

## Authors

  - **Ucchi98** - *Plant Cultivation Light Controller* -
    [Ucchi98](https://github.com/Ucchi98)

## License

MIT License - see the [LICENSE.md](LICENSE.md) file for
details


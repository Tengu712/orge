# orge

## What's this?

オレオレゲームエンジン。

ゲームエンジンと言っても、ゲームに用いられることを想定した、ただのVulkanとSDL3の薄いラッパーフレームワーク。

## Features

- Windows/macOS/Linux対応
- シンプルなAPIによる他言語対応
- 緩いライセンス
- ウィンドウ/フルスクリーン対応
- マルチパスレンダリング
- テキストレンダリング
- キーボード入力
- WAV/OGG再生

## How to Use

1. GitHubのReleaseからダウンロードするか[BUILD.md](./docs/BUILD.md)を参考にビルドしてorgeを入手する。
2. [CONFIG.md](./docs/CONFIG.md)を参考にconfigファイルを書く。
3. [bin/assetzip](./bin/assetzip/)を使って必要なアセットを.datファイルにまとめる。
4. 自前のアプリケーションからorgeを呼び出せるようにする:
   - orgeライブラリを静的にリンクするか動的にリンクできるようにする。
   - .datファイルを実行時ルートディレクトリに配置する。

Orgeではorgeで扱うすべてのアセットファイルを.datファイルにまとめる。
この.datファイルは[bin/assetzip](./bin/assetzip/)によって作成する。

Orgeではorgeで扱うすべてのメッシュデータをアセットとして指定する。
このアセットは[bin/mesher](./bin/mesher/)によって作成する。

> [!WARNING]
> 現在、orgeは非同期的なAPI呼出しに対応していない。
> そのため、orgeのAPI呼出しは必ず同一のスレッドから行うこと。

> [!NOTE]
> Linuxでorge (静的ライブラリ)をリンクする場合、システムにインストールされたVulkanローダもリンクすること。

## License

Orge自体はCC0 1.0 Universalの下に公開されている。
要は著作権を放棄しているので煮るなり焼くなりお好きにどうぞ。

ただし、orgeが依存するライブラリは各々のライセンスの下に公開されている。
従って、orgeを利用する場合はこの各々のライセンスに従う必要がある。
中でもライセンス表示義務のあるものは[licenses](./licenses/)を参照すること。

Orgeの依存するすべてのライブラリおよびその開発者に感謝申し上げます。

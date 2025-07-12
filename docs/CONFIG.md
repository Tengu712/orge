# Config

## Why use YAML?

orgeでは初期化時の設定をYAMLファイルあるいはYAML形式の文字列で与える。
これは初期化関数のAPIをなるべく簡潔に保つためである。

レンダーパスやグラフィックスパイプラインの定義を平坦な引数で与えると煩雑になってしまう。
確かに、これらの設定だけ別途C++で与えるようにしても良い。
あるいは、例外的に構造体を定義してもらうようにしても良い。
しかし、orgeのコンセプトはFFIを持つどの言語からも楽に使えることである。
そのため、このような設計になっている。

## Parameters

- `title`: (`string`)ウィンドウタイトル
- `width`: (`uint`)ウィンドウの内側の幅
- `height`: (`uint`)ウィンドウの内側の高さ
- `attachments`: 描画処理で使われるアタッチメントの配列
  - `id`: (`string`)アタッチメントのID
  - `format`: (`string`)アタッチメントのフォーマット
    - `render-target`: 描画先のフォーマット
  - `discard`: (`bool`)レンダーパス終了時にアタッチメントの内容を破棄するか(省略可能。標準値は`false`)
  - `final-layout`: (`string`)レンダーパス終了時に変換されるアタッチメントのレイアウト
    - `color-attachment`
    - `depth-stencil-attachment`
    - `present-src`
  - `clear-value`: (`float[4]|float`)アタッチメントのクリア値
    - カラーアタッチメントであれば`float[4]`を、
    - 深度アタッチメントであれば`float`を指定する
- `subpasses`: サブパスの配列
  - `id`: (`string`)サブパスのID
  - `inputs`: 入力となるアタッチメントの配列(省略可能)
    - `id`: (`string`)入力となるアタッチメントのID
    - `layout`: (`string`)そのアタッチメントの処理中のレイアウト
      - `depth-stencil-read-only`
      - `shader-read-only`
  - `outputs`: (`string[]`)出力となるカラーアタッチメントのIDの配列
  - `depth`: 関連付ける深度アタッチメント(省略可能)
    - `id`: (`string`)アタッチメントのID
    - `read-only`: (`bool`)読取り専用か
- `dependencies`: サブパスの依存関係の配列(省略可能)
  - `src`: (`string`)依存関係上前のサブパスのID
  - `dst`: (`string`)依存関係上後のサブパスのID
- `pipelines`: パイプラインの配列
  - `id`: パイプラインのID
  - `vertex-shader`: (`string`)バーテックスシェーダのファイル名
  - `fragment-shader`: (`string`)フラグメントシェーダのファイル名
  - `desc-sets`: ディスクリプタセットの配列(省略可能)
    - `count`: (`uint`)アロケートするこのディスクリプタセットの個数
    - `bindings`: ディスクリプタの配列
      - `type`: (`string`)ディスクリプタの種類
        - `combined-image-sampler`
        - `uniform-buffer`
        - `storage-buffer`
        - `input-attachment`
      - `count`: (`uint`)ディスクリプタの数(省略可能。標準値は`1`)
      - `stages`: (`string[]`)ディスクリプタが見えるパイプラインステージの配列
        - `vertex`
        - `fragment`
  - `vertex-input-attributes`: (`uint[]`)各頂点入力属性のサイズの配列(各値は`1`から`4`まで)
  - `culling`: (`bool`)カリングを行う(省略可能。標準値は`false`)
  - `color-blends`: (`bool[]`)各カラーアタッチメントのカラーブレンドを行うかの配列
  - `subpass`: (`string`)対応づけるサブパスのID

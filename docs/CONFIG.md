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
- `width`: (`int`)ウィンドウの内側の幅
- `height`: (`int`)ウィンドウの内側の高さ
- `attachments`: 描画処理で使われるアタッチメントの配列
  - `id`: (`string`)アタッチメントのID
  - `format`: (`string`)アタッチメントのフォーマット
    - TODO:
    - `render-target`: 描画先のフォーマット
  - `discard`: (`bool`)レンダーパス終了時にアタッチメントの内容を破棄するか(省略可能。標準値は`false`)
  - `final-layout`: (`string`)レンダーパス終了時に変換されるアタッチメントのレイアウト
    - `general`
    - `color-attachment`
    - `depth-stencil-attachment`
    - `transfer-src`
    - `present-src`
  - `clear-value`: (`float[4]|float`)アタッチメントのクリア値
    - カラーアタッチメントであれば`float[4]`を、
    - 深度アタッチメントであれば`float`を指定する
- `subpasses`: サブパスの配列
  - `id`: (`string`)サブパスのID
  - `inputs`: 入力となるアタッチメントの配列(省略可能)
    - `id`: (`string`)入力となるアタッチメントのID
    - `layout`: (`string`)そのアタッチメントの処理中のレイアウト
      - `general`
      - `depth-stencil-read-only`
      - `shader-read-only`
  - `outputs`: 出力となるカラーアタッチメントの配列
    - `id`: (`string`)入力となるアタッチメントのID
    - `layout`: (`string`)そのアタッチメントの処理中のレイアウト
      - `general`
      - `color-attachment`
  - `depth`: 関連付ける深度アタッチメント(省略可能)
    - `id`: (`string`)入力となるアタッチメントのID
    - `layout`: (`string`)そのアタッチメントの処理中のレイアウト
      - `general`
      - `depth-stencil-attachment`
      - `depth-stencil-read-only`
- `subpass-deps`: サブパスの依存関係の配列(省略可能)
  - `src`: (`string`)依存関係上前のサブパスのID
  - `dst`: (`string`)依存関係上後のサブパスのID

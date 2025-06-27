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
  - `id`: (`string`)アタッチメントのID(任意)
  - `format`: (`string`)アタッチメントのフォーマット
  - `loadOp`: (`string`)アタッチメントに対するレンダーパスにおける初回利用時の処理
    - `load`であれば読込み
    - `clear`であればクリア
    - `dont-care`であれば何もしない
  - `storeOp`: (`string`)アタッチメントに対するレンダーパスにおける最終利用時の処理
    - `store`であれば保存
    - `dont-care`であれば何もしない
  - `initialLayout`: (`string`)レンダーパス開始時に期待されるアタッチメントのレイアウト
    - `undefined`
    - `general`
    - `color-attachment`
    - `depth-stencil-attachment`
    - `depth-stencil-read-only`
    - `shader-read-only`
    - `transfer-src`
    - `transfer-dst`
    - `preinitialized`
  - `finalLayout`: (`string`)レンダーパス終了時に変換されるアタッチメントのレイアウト
    - `general`
    - `color-attachment`
    - `depth-stencil-attachment`
    - `depth-stencil-read-only`
    - `shader-read-only`
    - `transfer-src`
    - `transfer-dst`
    - `present-src`
- `subpasses`: 描画処理の各処理の配列
  - `inputs`: 入力となるアタッチメントの配列(省略可能)
    - `id`: (`string`)入力となるアタッチメントのID
    - `layout`: (`string`)そのアタッチメントの処理中のレイアウト
      - `general`
      - `depth-stencil-read-only`
      - `shader-read-only`
  - `colors`: 出力となるカラーアタッチメントの配列
    - `id`: (`string`)入力となるアタッチメントのID
    - `layout`: (`string`)そのアタッチメントの処理中のレイアウト
      - `general`
      - `color-attachment`
  - `depth`: 関連付ける深度ステンシルアタッチメント(省略可能)
    - `id`: (`string`)入力となるアタッチメントのID
    - `layout`: (`string`)そのアタッチメントの処理中のレイアウト
      - `general`
      - `color-attachment`

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

```yaml
# ========== Basic Settings ==================== #

# ウィンドウタイトル
title: string

# ウィンドウの内側のサイズ
width: uint
height: uint

# ========== Attachment Definition ============= #

attachments: []
  - id: string

    # アタッチメントのフォーマット
    # 取りうる値は以下:
    #   - render-target
    format: string

    # レンダーパス終了時にアタッチメントの内容を破棄するか
    # 省略された場合、falseとみなされる
    discard: bool

    # レンダーパス終了時に変換されるアタッチメントのレイアウト
    # 取りうる値は以下:
    #   - color-attachment
    #   - depth-stencil-attachment
    #   - present-src
    final-layout: string

    # アタッチメントのクリア値
    # カラーアタッチメントであればfloat[4]を、深度アタッチメントであればfloatを指定する
    clear-value: float[4] | float

# ========== Subpass Definition ================ #

subpasses: []
  - id: string

    # 入力となるアタッチメントの配列
    # 省略可能
    inputs: []
      - id: string

        # そのアタッチメントの処理中のレイアウト
        # 取りうる値は以下:
        #   - depth-stencil-read-only
        #   - shader-read-only
        layout: string

    # 出力となるカラーアタッチメントのIDの配列
    outputs: string[]

    # 関連付ける深度アタッチメント
    # 省略可能
    depth:
        id: string
        read-only: bool

# ========== Dependency Definition ============= #

# サブパスの依存関係を表すサブパスIDの配列
# 省略可能
dependencies: []
  - src: string
    dst: string

# ========== Pipeline Definition =============== #

# 省略可能
pipelines: []
  - id: string

    # シェーダのファイル名
    vertex-shader: string
    fragment-shader: string

    # ディスクリプタセットの配列
    # 省略可能
    desc-sets: []
        # アロケートするこのディスクリプタセットの個数
      - count: uint

        # ディスクリプタの配列
        bindings: []
            # ディスクリプタの種類
            # 取りうる値は以下:
            #   - combined-image-sampler
            #   - uniform-buffer
            #   - storage-buffer
            #   - input-attachment
          - type: string

            # ディスクリプタの数
            # 省略された場合、1とみなされる
            count: uint

            # ディスクリプタが見えるパイプラインステージ
            # 取りうる値は以下:
            #   - vertex
            #   - fragment
            #   - vertex-and-fragment
            stages: string

    # 各頂点入力属性のサイズの配列
    # 各値は1から4まで
    vertex-input-attributes: uint[]

    # カリングを行うか
    # 省略された場合、falseとみなされる
    culling: bool

    # 各カラーアタッチメントのカラーブレンドを行うかの配列
    color-blends: bool[]

    # 対応づけるサブパスID
    subpass: string
```

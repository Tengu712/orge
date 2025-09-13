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
# フルスクリーン時はこのサイズのアスペクト比を保ってスケーリングされる
width: unsigned int
height: unsigned int

# フルスクリーン状態で開始するか
# 省略された場合、falseとみなされる
fullscreen: bool

# 垂直同期を無効化するか
# 省略された場合、falseとみなされる
disable-vsync: bool

# Alt+ReturnあるいはCmd+Returnでのフルスクリーン・非フルスクリーンを許可するか
# 省略された場合、trueとみなされる
alt-return-toggle-fullscreen: bool

# 音声チャンネルの数
# 省略された場合、16とみなされる
audio-channel-count: unsigned int

# ========== Assets Definition ================= #

# アセットファイル名
# 省略可能
assets: string[]

# ========== Meshes Definition ================= #

# メッシュアセット名
# 省略可能
meshes: []
  - vertices: string
    indices: string

# ========== Text Rendering Settings =========== #

# 省略可能
fonts:
  - id: string

    # フォントアセット名
    file: string

    # 1文字の幅および高さの最大値 (px)
    # 0より大きい整数であること
    char-size: unsigned int

    # テクスチャアトラスにロードする文字の列・行数
    # 0より大きい整数であること
    char-atlus-col: unsigned int
    char-atlus-row: unsigned int

# 最大文字数
# 省略された場合、256とみなされる
char-count: unsgined int

# ========== Attachment Definition ============= #

attachments: []
  - id: string

    # アタッチメントのフォーマット
    # 取りうる値は以下:
    #   - render-target
    #   - depth-buffer
    #   - share-color-attachment
    #   - signed-share-color-attachment
    format: string

    # レンダーパス終了時にアタッチメントの内容を破棄するか
    # 省略された場合、falseとみなされる
    discard: bool

    # アタッチメントのクリア値
    # - カラーアタッチメントであればfloat[4]を、
    # - 深度アタッチメントであればfloatを指定する
    clear-value: float[4] | float

# ========== Pipeline Definition =============== #

# 省略可能
pipelines: []
  # 通常のレンダリングパイプライン
  - id: string

    # シェーダアセット名
    vertex-shader: string
    fragment-shader: string

    # ディスクリプタセットの配列
    # 省略可能
    desc-sets: []
        # アロケートするこのディスクリプタセットの個数
      - count: unsigned int

        # ディスクリプタの配列
        bindings: []
            # ディスクリプタの種類
            # 取りうる値は以下:
            #   - texture
            #   - sampler
            #   - uniform-buffer
            #   - storage-buffer
            #   - input-attachment
          - type: string

            # ディスクリプタの数
            # 省略された場合、1とみなされる
            count: unsigned int

            # ディスクリプタが見えるパイプラインステージ
            # 取りうる値は以下:
            #   - vertex
            #   - fragment
            #   - vertex-and-fragment
            stage: string

    # 各頂点入力属性のサイズの配列
    # 各値は1から4まで
    # 省略された場合、空配列とみなされる
    vertex-input-attributes: unsigned int[]

    # シェーダ内でメッシュを構築するか
    # 省略された場合、falseとみなされる
    mesh-in-shader: bool

    # カリングを行うか
    # 省略された場合、falseとみなされる
    culling: bool

    # 深度テストを行うか
    # 省略された場合、falseとみなされる
    depth-test: bool

    # 各カラーアタッチメントのカラーブレンドを行うかの配列
    color-blends: bool[]

# ========== Render Pass Definition ================ #

render-passes: []
  - id: string

    subpasses: []
      - id: string

        # 入力となるアタッチメントのIDの配列
        # 省略可能
        inputs: string[]

        # 出力となるカラーアタッチメントのIDの配列
        outputs: string[]

        # 関連付ける深度アタッチメント
        # 省略可能
        depth:
            id: string
            read-only: bool

        # 依存するサブパスのIDの配列
        # 既に定義されているサブパスのIDであること
        # 省略可能
        depends: string[]

        # 対応するすべてパイプラインID
        pipelines: string[]
```

#pragma once

#include <stdint.h>

// ================================================================================================================== //
//     Error                                                                                                          //
// ================================================================================================================== //

/// orgeの直近のエラーメッセージを取得する関数
const char *orgeGetErrorMessage();

// ================================================================================================================== //
//     Initialization & Termination                                                                                   //
// ================================================================================================================== //

/// orgeを初期化する関数 (yaml文字列指定)
int orgeInitialize(const char *yaml);

/// orgeを初期化する関数 (yamlファイル指定)
int orgeInitializeWith(const char *yamlFilePath);

/// orgeを終了する関数
void orgeTerminate(void);

// ================================================================================================================== //
//     Window Event                                                                                                   //
// ================================================================================================================== //

/// orgeのウィンドウイベントをすべて処理する関数
///
/// 毎フレーム初めに呼ぶこと。
/// 特に、メインループの条件式に使うと良い。
///
/// 0以外であればウィンドウが閉じられていないことを意味する。
int orgePollEvents(void);

// ================================================================================================================== //
//     Graphics Resources                                                                                             //
// ================================================================================================================== //

/// orgeにバッファを追加する関数
int orgeCreateBuffer(const char *id, uint64_t size, int isStorage);

/// バッファを更新する関数
///
/// dataはバッファ作成時に指定したサイズ分データを持つこと。
int orgeUpdateBuffer(const char *id, const void *data);

/// バッファディスクリプタを更新する関数
///
/// - bufferId: バッファID
/// - pipelineId: パイプラインID
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
int orgeUpdateBufferDescriptor(
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding
);

/// orgeにイメージを追加する関数 (pngファイル指定)
///
/// - id: イメージID
/// - path: pngファイルパス
/// - linearMagFilter: 拡大時のフィルタリング設定
///     - 0以外なら線形補間
///     - 0なら最も近いテクセルを参照
/// - linearMinFilter: 縮小時のフィルタリング設定
///     - 0以外なら線形補間
///     - 0なら最も近いテクセルを参照
/// - repeat: [0-1]の範囲外のUV座標における設定
///     - 0以外ならテクスチャを繰り返して参照
///     - 0なら0あるいは1の境界値を参照
int orgeCreateImageFromFile(const char *id, const char *path, int linearMagFilter, int linearMinFilter, int repeat);

/// イメージディスクリプタを更新する関数
///
/// - imageId: イメージID
/// - pipelineId: パイプラインID
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
int orgeUpdateImageDescriptor(
	const char *imageId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding
);

/// orgeにメッシュを追加する関数
///
/// - id: メッシュID
/// - vertexCount: verticesの要素数
/// - vertices: 頂点データ
/// - indexCount: indicesの要素数
/// - indices: インデックスデータ
int orgeCreateMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
);

// ================================================================================================================== //
//     Rendering                                                                                                      //
// ================================================================================================================== //

/// orgeの描画を開始する関数
int orgeBeginRender(void);

/// パイプラインをバインドする関数
///
/// 競合するパイプラインがある場合、
/// - 後からバインドされるパイプラインがバインドされる。
/// - ただし、同じAPIコールで指定された場合、最終的にどのパイプラインがバインドされるかわからない。
int orgeBindPipelines(uint32_t pipelineCount, const char *const *pipelines);

/// ディスクリプタセットをバインドする関数
///
/// - id: パイプラインID
/// - indices: 各セットにおいて何個目のセットを使うか
///
/// indicesはディスクリプタセットの個数分データを持つこと。
///
/// 例えば、set = 0とset = 1があり、それぞれ2個と3個確保されている場合、
/// indicesの要素数は2、indicesの各要素は0-1と0-2を取る。
int orgeBindDescriptorSets(const char *id, uint32_t const *indices);

/// 描画関数
///
/// meshはバインドするメッシュのID。
/// meshがnullptrである場合、メッシュはバインドされず、同じ描画内で直近にバインドされたメッシュが使われる。
int orgeDraw(const char *mesh, uint32_t instanceCount, uint32_t instanceOffset);

/// orgeの描画を終了する関数
int orgeEndRender(void);

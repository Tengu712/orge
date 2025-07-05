#pragma once

#include <stdint.h>

/// orgeのエラーメッセージを取得する関数
const char *orgeGetErrorMessage();

/// orgeを初期化する関数 (yaml文字列指定)
int orgeInitialize(const char *const yaml);

/// orgeを初期化する関数 (yamlファイル指定)
int orgeInitializeWith(const char *const yamlFilePath);

/// orgeにメッシュを追加する関数
int orgeCreateMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
);

/// orgeのウィンドウイベントをすべて処理する関数
///
/// 毎フレーム初めに呼ぶこと。
/// 特に、メインループの条件式に使うと良い。
///
/// 0以外であればウィンドウが閉じられていないことを意味する。
int orgePollEvents(void);

/// orgeの描画を開始する関数
int orgeBeginRender();

/// 描画関数
///
/// pipelinesはバインドするパイプラインのID列。
/// 一度バインドされたパイプラインはorgeEndRender()が呼ばれるまでバインドされたままになる。
/// 競合するパイプラインがある場合、最終的にどのパイプラインがバインドされるかわからない。
/// pipelinesがnullptrである場合、パイプラインはバインドされない。
///
/// meshはバインドするメッシュのID。
/// 一度バインドされたメッシュはorgeEndRender()が呼ばれるまでバインドされたままになる。
/// meshがnullptrである場合、メッシュはバインドされない。
int orgeDraw(
	uint32_t pipelineCount,
	const char *const *pipelines,
	const char *mesh,
	uint32_t instanceCount,
	uint32_t instanceOffset
);

/// orgeの描画を終了する関数
int orgeEndRender();

/// orgeを終了する関数
void orgeTerminate(void);

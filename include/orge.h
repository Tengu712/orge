//! orgeのAPIが網羅されたヘッダーファイル
//!
//! 原則bit数固定の型を使っている。
//! 文字列のみ例外的にconst char *を使っている。
//! 尚、文字列の長さは指定されないため、null文字終端でなければ壊れるかもしれない。

#pragma once

#include <stdint.h>

// ================================================================================================================== //
//     Information                                                                                                    //
// ================================================================================================================== //

enum OrgeDialogType {
	ORGE_DIALOG_TYPE_ERROR = 0,
	ORGE_DIALOG_TYPE_WARNING,
	ORGE_DIALOG_TYPE_INFORMATION,
};

/// ダイアログを表示する関数
///
/// - dtype: ダイアログの種類 (OrgeDialogType)
/// - title: ダイアログのタイトル
/// - message: ダイアログのメッセージ
///
/// ダイアログが閉じられるまでスレッドを停止する。
void orgeShowDialog(uint32_t dtype, const char *title, const char *message);

/// orgeの直近のエラーメッセージを取得する関数
const char *orgeGetErrorMessage(void);

// ================================================================================================================== //
//     Lifetime Managiment                                                                                            //
// ================================================================================================================== //

/// orgeを初期化する関数 (yaml文字列指定)
uint8_t orgeInitialize(const char *yaml);

/// orgeを初期化する関数 (yamlファイル指定)
uint8_t orgeInitializeWith(const char *yamlFilePath);

/// orgeを終了する関数
void orgeTerminate(void);

/// orgeを更新する関数
///
/// 毎フレーム初めに必ず呼ぶこと。
///
/// 0以外であればウィンドウが閉じられていないことを意味する。
uint8_t orgeUpdate(void);

// ================================================================================================================== //
//     Window                                                                                                         //
// ================================================================================================================== //

/// フルスクリーン状態を取得する関数
uint8_t orgeIsFullscreen(void);

/// フルスクリーン状態を設定する関数
void orgeSetFullscreen(uint8_t toFullscreen);

// ================================================================================================================== //
//     Graphics Resources                                                                                             //
// ================================================================================================================== //

/// orgeにバッファを追加する関数
///
/// - id: バッファID
/// - size: バッファのサイズ (バイト数)
/// - isStorage: ストレージバッファか (falseの場合ユニフォームバッファとみなされる)
uint8_t orgeCreateBuffer(const char *id, uint64_t size, uint8_t isStorage);

/// バッファを破棄する関数
void orgeDestroyBuffer(const char *id);

/// バッファを更新する関数
///
/// dataはバッファ作成時に指定したサイズ分データを持つこと。
uint8_t orgeUpdateBuffer(const char *id, const void *data);

/// バッファディスクリプタを更新する関数
///
/// - bufferId: バッファID
/// - pipelineId: パイプラインID
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
/// - offset: 配列上のオフセット (ディスクリプタが配列でないなら0)
uint8_t orgeUpdateBufferDescriptor(
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
);

/// orgeにイメージを追加する関数 (ビットマップデータ指定)
///
/// pixelsはビットマップデータ。
/// 必ずRGBAで並んでいること。
uint8_t orgeCreateImage(const char *id, uint32_t width, uint32_t height, const uint8_t *pixels);

/// orgeにイメージを追加する関数 (pngファイル指定)
///
/// - id: イメージID
/// - path: pngファイルパス
uint8_t orgeCreateImageFromFile(const char *id, const char *path);

/// イメージを破棄する関数
void orgeDestroyImage(const char *id);

/// イメージディスクリプタを更新する関数
///
/// - imageId: イメージID
/// - pipelineId: パイプラインID
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
/// - offset: 配列上のオフセット (ディスクリプタが配列でないなら0)
uint8_t orgeUpdateImageDescriptor(
	const char *imageId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
);

/// orgeにサンプラを追加する関数
///
/// - id: サンプラID
/// - linearMagFilter: 拡大時のフィルタリング設定
///     - 0以外なら線形補間
///     - 0なら最も近いテクセルを参照
/// - linearMinFilter: 縮小時のフィルタリング設定
///     - 0以外なら線形補間
///     - 0なら最も近いテクセルを参照
/// - repeat: [0-1]の範囲外のUV座標における設定
///     - 0以外ならテクスチャを繰り返して参照
///     - 0なら0あるいは1の境界値を参照
uint8_t orgeCreateSampler(const char *id, uint8_t linearMagFilter, uint8_t linearMinFilter, uint8_t repeat);

/// サンプラを破棄する関数
void orgeDestroySampler(const char *id);

/// サンプラディスクリプタを更新する関数
///
/// - samplerId: サンプラID
/// - pipelineId: パイプラインID
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
/// - offset: 配列上のオフセット (ディスクリプタが配列でないなら0)
uint8_t orgeUpdateSamplerDescriptor(
	const char *samplerId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
);

/// orgeにメッシュを追加する関数
///
/// - id: メッシュID
/// - vertexCount: verticesの要素数
/// - vertices: 頂点データ
/// - indexCount: indicesの要素数
/// - indices: インデックスデータ
uint8_t orgeCreateMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
);

/// メッシュを破棄する関数
void orgeDestroyMesh(const char *id);

// ================================================================================================================== //
//     Rendering                                                                                                      //
// ================================================================================================================== //

/// orgeの描画を開始する関数
uint8_t orgeBeginRender(void);

/// ディスクリプタセットをバインドする関数
///
/// - pipelineId: パイプラインID
/// - indices: 各セットにおいて何個目のセットを使うか
///
/// indicesはディスクリプタセットの個数分データを持つこと。
///
/// 例えば、set = 0とset = 1があり、それぞれ2個と3個確保されている場合、
/// indicesの要素数は2、indicesの各要素は0-1と0-2を取る。
uint8_t orgeBindDescriptorSets(const char *pipelineId, uint32_t const *indices);

/// 描画関数
///
/// pipelineIdはバインドするパイプラインのID。
/// pipelineIdがnullptrであったり、既にバインドされているパイプラインのIDである場合、バインドはスキップされる。
///
/// meshIdはバインドするメッシュのID。
/// meshIdがnullptrであったり、既にバインドされているメッシュのIDである場合、バインドはスキップされる。
uint8_t orgeDraw(const char *pipelineId, const char *meshId, uint32_t instanceCount, uint32_t instanceOffset);

/// 次のサブパスへ移るための関数
uint8_t orgeNextSubpass(void);

/// orgeの描画を終了する関数
uint8_t orgeEndRender(void);

// ================================================================================================================== //
//     Input                                                                                                          //
// ================================================================================================================== //

enum OrgeScancode {
	ORGE_SCANCODE_A = 0,
	ORGE_SCANCODE_B,
	ORGE_SCANCODE_C,
	ORGE_SCANCODE_D,
	ORGE_SCANCODE_E,
	ORGE_SCANCODE_F,
	ORGE_SCANCODE_G,
	ORGE_SCANCODE_H,
	ORGE_SCANCODE_I,
	ORGE_SCANCODE_J,
	ORGE_SCANCODE_K,
	ORGE_SCANCODE_L,
	ORGE_SCANCODE_M,
	ORGE_SCANCODE_N,
	ORGE_SCANCODE_O,
	ORGE_SCANCODE_P,
	ORGE_SCANCODE_Q,
	ORGE_SCANCODE_R,
	ORGE_SCANCODE_S,
	ORGE_SCANCODE_T,
	ORGE_SCANCODE_U,
	ORGE_SCANCODE_V,
	ORGE_SCANCODE_W,
	ORGE_SCANCODE_X,
	ORGE_SCANCODE_Y,
	ORGE_SCANCODE_Z,
	ORGE_SCANCODE_1,
	ORGE_SCANCODE_2,
	ORGE_SCANCODE_3,
	ORGE_SCANCODE_4,
	ORGE_SCANCODE_5,
	ORGE_SCANCODE_6,
	ORGE_SCANCODE_7,
	ORGE_SCANCODE_8,
	ORGE_SCANCODE_9,
	ORGE_SCANCODE_0,
	ORGE_SCANCODE_RETURN,
	ORGE_SCANCODE_ESCAPE,
	ORGE_SCANCODE_BACKSPACE,
	ORGE_SCANCODE_TAB,
	ORGE_SCANCODE_SPACE,
	ORGE_SCANCODE_MINUS,
	ORGE_SCANCODE_EQUALS,
	ORGE_SCANCODE_LEFTBRACKET,
	ORGE_SCANCODE_RIGHTBRACKET,
	ORGE_SCANCODE_BACKSLASH,
	ORGE_SCANCODE_NONUSHASH,
	ORGE_SCANCODE_SEMICOLON,
	ORGE_SCANCODE_APOSTROPHE,
	ORGE_SCANCODE_GRAVE,
	ORGE_SCANCODE_COMMA,
	ORGE_SCANCODE_PERIOD,
	ORGE_SCANCODE_SLASH,
	ORGE_SCANCODE_RIGHT,
	ORGE_SCANCODE_LEFT,
	ORGE_SCANCODE_DOWN,
	ORGE_SCANCODE_UP,
};

/// キー入力状態を取得する関数
///
/// 返戻眙:
/// - = -1: キーアップ
/// - = 0: ニュートラル
/// - > 0: キーダウン (押されているフレーム数)
int32_t orgeGetKeyState(uint32_t scancode);

//! orgeのAPIが網羅されたヘッダーファイル
//!
//! 原則bit数固定の型を使っている。
//! 文字列のみ例外的にconst char *を使っている。
//! 尚、文字列の長さは指定されないため、null文字終端でなければ壊れるかもしれない。

#pragma once

#ifdef _WIN32
# ifdef ORGE_BUILD_SHARED
#  define API_EXPORT __declspec(dllexport)
# elif ORGE_USE_SHARED
#  define API_EXPORT __declspec(dllimport)
# else
#  define API_EXPORT
# endif
#else
# define API_EXPORT
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
API_EXPORT void orgeShowDialog(uint32_t dtype, const char *title, const char *message);

/// orgeの直近のエラーメッセージを取得する関数
API_EXPORT const char *orgeGetErrorMessage(void);

// ================================================================================================================== //
//     Lifetime Managiment                                                                                            //
// ================================================================================================================== //

/// orgeを初期化する関数
API_EXPORT uint8_t orgeInitialize(void);

/// orgeを終了する関数
API_EXPORT void orgeTerminate(void);

/// orgeを更新する関数
///
/// 毎フレーム初めに必ず呼ぶこと。
///
/// 0以外であればウィンドウが閉じられていないことを意味する。
API_EXPORT uint8_t orgeUpdate(void);

// ================================================================================================================== //
//     Window                                                                                                         //
// ================================================================================================================== //

/// フルスクリーン状態を取得する関数
API_EXPORT uint8_t orgeIsFullscreen(void);

/// フルスクリーン状態を設定する関数
API_EXPORT void orgeSetFullscreen(uint8_t toFullscreen);

// ================================================================================================================== //
//     Graphics Resources                                                                                             //
// ================================================================================================================== //

/// orgeにバッファを追加する関数
///
/// - id: バッファID
/// - size: バッファのサイズ (バイト数)
/// - isStorage: ストレージバッファか (falseの場合ユニフォームバッファとみなされる)
API_EXPORT uint8_t orgeCreateBuffer(const char *id, uint64_t size, uint8_t isStorage);

/// バッファを破棄する関数
API_EXPORT void orgeDestroyBuffer(const char *id);

/// バッファを更新する関数
///
/// dataはバッファ作成時に指定したサイズ分データを持つこと。
API_EXPORT uint8_t orgeUpdateBuffer(const char *id, const uint8_t *data);

/// バッファディスクリプタを更新する関数
///
/// - renderPassId: レンダーパスID
/// - pipelineId: パイプラインID
/// - id: バッファID
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
/// - offset: 配列上のオフセット (ディスクリプタが配列でないなら0)
API_EXPORT uint8_t orgeUpdateBufferDescriptor(
	const char *renderPassId,
	const char *pipelineId,
	const char *id,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
);

/// orgeにイメージを追加する関数
///
/// - file: アセットファイル名
API_EXPORT uint8_t orgeLoadImage(const char *file);

/// イメージを破棄する関数
API_EXPORT void orgeDestroyImage(const char *file);

/// イメージディスクリプタを更新する関数
///
/// - renderPassId: レンダーパスID
/// - pipelineId: パイプラインID
/// - id: イメージID (アセットファイル名)
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
/// - offset: 配列上のオフセット (ディスクリプタが配列でないなら0)
API_EXPORT uint8_t orgeUpdateImageDescriptor(
	const char *renderPassId,
	const char *pipelineId,
	const char *id,
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
API_EXPORT uint8_t orgeCreateSampler(const char *id, uint8_t linearMagFilter, uint8_t linearMinFilter, uint8_t repeat);

/// サンプラを破棄する関数
API_EXPORT void orgeDestroySampler(const char *id);

/// サンプラディスクリプタを更新する関数
///
/// - renderPassId: レンダーパスID
/// - pipelineId: パイプラインID
/// - id: サンプラID
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
/// - offset: 配列上のオフセット (ディスクリプタが配列でないなら0)
API_EXPORT uint8_t orgeUpdateSamplerDescriptor(
	const char *renderPassId,
	const char *pipelineId,
	const char *id,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
);

/// アタッチメントディスクリプタを更新する関数
///
/// - renderPassId: レンダーパスID
/// - pipelineId: パイプラインID
/// - id: アタッチメントID
/// - set: ディスクリプタセット番号
/// - index: 何個目のディスクリプタセットか
/// - binding: バインディング番号
/// - offset: 配列上のオフセット (ディスクリプタが配列でないなら0)
///
/// 更新先のディスクリプタがtexture2DでもsubpassInputでも構わない。
/// アタッチメントをディスクリプタに関連づけるときにこの関数を用いる。
///
/// WARN: 描画が開始されていること。
API_EXPORT uint8_t orgeUpdateInputAttachmentDescriptor(
	const char *renderPassId,
	const char *pipelineId,
	const char *id,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
);

/// orgeにメッシュを追加する関数
///
/// - id: メッシュID
API_EXPORT uint8_t orgeLoadMesh(const char *id);

/// メッシュを破棄する関数
API_EXPORT void orgeDestroyMesh(const char *id);

// ================================================================================================================== //
//     Text Rendering                                                                                                 //
// ================================================================================================================== //

// enum OrgeTextLocationHorizontal {
// 	ORGE_TEXT_LOCATION_HORIZONTAL_LEFT = 0,
// 	ORGE_TEXT_LOCATION_HORIZONTAL_CENTER,
// 	ORGE_TEXT_LOCATION_HORIZONTAL_RIGHT,
// };
// 
// enum OrgeTextLocationVertical {
// 	ORGE_TEXT_LOCATION_VERTICAL_TOP = 0,
// 	ORGE_TEXT_LOCATION_VERTICAL_MIDDLE,
// 	ORGE_TEXT_LOCATION_VERTICAL_BOTTOM,
// };
// 
// /// 文字をラスタライズする関数
// ///
// /// - id: フォントID
// /// - s: UTF-8文字列
// ///
// /// sの各文字がラスタライズされる。
// /// 結合文字は結合されず、別々の文字として扱われる。
// API_EXPORT uint8_t orgeRasterizeCharacters(const char *id, const char *s);
// 
// /// このフレームで描画する文字列を追加する関数
// ///
// /// - pipelineId: この文字列を描画するテキストレンダリングパイプラインのID
// /// - fontId: フォントID
// /// - text: UTF-8文字列
// /// - x: 基準点X座標
// /// - y: 基準点Y座標
// /// - height: 文字の基準高
// /// - horizontal: 水平方向の配置位置 (OrgeTextLocationHorizontal)
// /// - vertical: 垂直方向の配置位置 (OrgeTextLocationVertical)
// ///
// /// 毎フレームorgeBeginRender()の前に呼ぶこと。
// ///
// /// 存在しない文字はスキップされる。
// API_EXPORT uint8_t orgePutText(
// 	const char *pipelineId,
// 	const char *fontId,
// 	const char *text,
// 	float x,
// 	float y,
// 	float height,
// 	uint32_t horizontal,
// 	uint32_t vertical
// );
// 
// /// orgePutText()で追加された文字列をすべて描画する関数
// API_EXPORT uint8_t orgeDrawTexts(const char *pipelineId);

// ================================================================================================================== //
//     Rendering                                                                                                      //
// ================================================================================================================== //

/// orgeの描画を開始する関数
API_EXPORT uint8_t orgeBeginRender(void);

/// orgeの描画を終了する関数
API_EXPORT uint8_t orgeEndRender(void);

/// メッシュをバインドする関数
///
/// 既に同一のメッシュがバインドされている場合、処理はスキップされる。
///
/// WARN: 描画が開始されていること。
API_EXPORT uint8_t orgeBindMesh(const char *meshId);

/// レンダーパスを開始する関数
///
/// WARN: 描画が開始されていること。
/// WARN: レンダーパスがまだ開始されていない・あるいは既に終了されていること。
API_EXPORT uint8_t orgeBeginRenderPass(const char *renderPassId);

/// レンダーパスを開始する関数
///
/// レンダーパスが開始されていない場合、処理はスキップされる。
///
/// WARN: 描画が開始されていること。
API_EXPORT uint8_t orgeEndRenderPass(void);

/// 次のサブパスへ移るための関数
///
/// WARN: レンダーパスが開始されていること。
/// WARN: orgeはサブパスを移行できるかを検証しない。
///       実際のレンダーパスに即して呼ぶこと。
API_EXPORT uint8_t orgeNextSubpass(void);

/// パイプラインをバインドする関数
///
/// - pipelineId: パイプラインID
/// - indices: 各ディスクリプタセットにおいて何個目のセットを使うか。
///            ディスクリプタセットの個数分データを持つこと。
///            例えば、set = 0とset = 1があり、それぞれ3個と4個確保されている場合、
///            indicesの要素数は2、indicesの各要素は0-2と0-3を取る。
///
/// WARN: レンダーパスが開始されていること。
API_EXPORT uint8_t orgeBindPipeline(const char *pipelineId, uint32_t const *indices);

/// 描画関数
///
/// - instanceCount: 描画するインスタンスの個数
/// - instanceOffset: 参照するインスタンスバッファのオフセット
///
/// WARN: パイプラインがバインドされていること。
/// WARN: メッシュがバインドされていること。
API_EXPORT uint8_t orgeDraw(uint32_t instanceCount, uint32_t instanceOffset);

/// 描画関数 (メッシュをシェーダ内で構築する場合)
///
/// - vertexCount: メッシュの頂点数
/// - instanceCount: 描画するインスタンスの個数
/// - instanceOffset: 参照するインスタンスバッファのオフセット
///
/// WARN: パイプラインがバインドされていること。
API_EXPORT uint8_t orgeDrawDirectly(uint32_t vertexCount, uint32_t instanceCount, uint32_t instanceOffset);

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

enum OrgeMouseButton {
	ORGE_MOUSE_BUTTON_LEFT = 0,
	ORGE_MOUSE_BUTTON_MIDDLE,
	ORGE_MOUSE_BUTTON_RIGHT,
};

/// キー入力状態を取得する関数
///
/// 返戻値:
/// - = -1: キーアップ
/// - = 0: ニュートラル
/// - > 0: キーダウン (押されているフレーム数)
API_EXPORT int32_t orgeGetKeyState(uint32_t scancode);

/// マウスカーソルのX座標を取得する関数
///
/// 座標系はウィンドウの左上が0、右端が幅。
/// ウィンドウ外の場合は最後に記録されたウィンドウ内の座標を返す。
API_EXPORT uint32_t orgeGetCursorX(void);

/// マウスカーソルのY座標を取得する関数
///
/// 座標系はウィンドウの上端が0、下端が高さ。
/// ウィンドウ外の場合は最後に記録されたウィンドウ内の座標を返す。
API_EXPORT uint32_t orgeGetCursorY(void);

/// マウスボタンの押下状態を取得する関数
///
/// - button: マウスボタン (OrgeMouseButton)
///
/// 返戻値:
/// - = -1: ボタンアップ
/// - = 0: ニュートラル
/// - > 0: ボタンダウン (押されているフレーム数)
API_EXPORT int32_t orgeGetMouseButtonState(uint32_t button);

// ================================================================================================================== //
//     Audio                                                                                                          //
// ================================================================================================================== //

/// 音声チャンネルの音量を取得する関数
///
/// - index: 音声チャンネルのインデックス
///
/// 不明なindexが指定された場合や、内部で予期せぬ例外が発生した場合は-1.0fが返る。
API_EXPORT float orgeGetAudioChannelVolume(uint32_t index);

/// 音声チャンネルの音量を設定する関数
///
/// - index: 音声チャンネルのインデックス
/// - volume: 音量 ([0.0, 1.0])
API_EXPORT uint8_t orgeSetAudioChannelVolume(uint32_t index, float volume);
 
/// orgeにWAVEを追加する関数
///
/// - file: アセットファイル名
/// - startPosition: ループ開始位置
API_EXPORT uint8_t orgeLoadWave(const char *file, uint32_t startPosition);

/// WAVEを破棄する関数
API_EXPORT void orgeDestroyWave(const char *file);

/// WAVEを再生する関数
///
/// - file: アセットファイル名
/// - index: 音声チャンネルのインデックス
/// - loop: ループ再生するか
///
/// 初めてindex番目の音声チャンネルで音声が再生されるとき、音声ストリームが作成される。
/// index番目の音声チャンネルとfileのWAVEの音声互換性がない場合、音声ストリームが再作成される。
/// index番目の音声チャンネルが音声を再生している場合、その音声を中断してfileのWAVEを再生する。
API_EXPORT uint8_t orgePlayWave(const char *file, uint32_t index, uint8_t loop);

#ifdef __cplusplus
}
#endif

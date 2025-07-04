#pragma once

/// orgeのエラーメッセージを取得する関数
const char *orgeGetErrorMessage();

/// orgeを初期化する関数 (yaml文字列指定)
int orgeInitialize(const char *const yaml);

/// orgeを初期化する関数 (yamlファイル指定)
int orgeInitializeWith(const char *const yamlFilePath);

/// orgeのウィンドウイベントをすべて処理する関数
///
/// 毎フレーム初めに呼ぶこと。
/// 特に、メインループの条件式に使うと良い。
///
/// 0以外であればウィンドウが閉じられていないことを意味する。
int orgePollEvents(void);

/// orgeの描画を開始する関数
int orgeBeginRender();

/// orgeの描画を終了する関数
int orgeEndRender();

/// orgeを終了する関数
void orgeTerminate(void);

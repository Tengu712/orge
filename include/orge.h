#pragma once

/// orgeのエラーメッセージを取得する関数
const char *orgeConvertErrorMessage(int from);

/// orgeを初期化する関数 (yaml文字列指定)
///
/// エラー値を返す。
/// 0以外であれば失敗を意味する。
int orgeInitialize(const char *const yaml);

/// orgeを初期化する関数 (yamlファイル指定)
///
/// エラー値を返す。
/// 0以外であれば失敗を意味する。
int orgeInitializeWith(const char *const yamlFilePath);

/// orgeのウィンドウイベントをすべて処理する関数
///
/// 毎フレーム初めに呼ぶこと。
/// 特に、メインループの条件式に使うと良い。
///
/// ウィンドウが閉じられたとき、0が返される。
int orgePollEvents(void);

/// orgeのウィンドウに描画する関数
///
/// エラー値を返す。
/// 以外であれば失敗を意味する。
int orgeRender();

/// orgeを終了する関数
void orgeTerminate(void);

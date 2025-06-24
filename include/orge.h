#pragma once

/// orgeのエラーメッセージを取得する関数
const char *orgeConvertErrorMessage(int from);

/// orgeを初期化する関数
///
/// 次が行われる:
/// - ウィンドウの生成
///
/// エラー値を返す。
/// 0以外であれば失敗を意味する。
int orgeInitialize(const char *windowTitle, int windowInnerWidth, int windowInnerHeight);

/// orgeのウィンドウイベントをすべて処理する関数
///
/// 毎フレーム初めに呼ぶこと。
/// 特に、メインループの条件式に使うと良い。
///
/// ウィンドウが閉じられたとき、0が返される。
int orgePollEvents(void);

/// orgeを終了する関数
void orgeTerminate(void);

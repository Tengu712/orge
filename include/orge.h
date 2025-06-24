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

/// orgeを終了する関数
void orgeTerminate();

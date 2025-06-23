//! グラフィックスに関するモジュール
//!
//! 依存関係上一番上に存在する。

#pragma once

#include "../error.hpp"

namespace graphics {

Error initialize(const char *title, int width, int height);

void terminate();

} // namespace graphics

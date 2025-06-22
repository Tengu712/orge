//! グラフィックスに関するモジュール
//!
//! サブモジュールを統括する。

#pragma once

#include "../error.hpp"

namespace graphics {

Error initialize(const char *title, int width, int height);

void terminate();

} // namespace graphics

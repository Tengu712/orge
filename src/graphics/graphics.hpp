//! グラフィックスに関するモジュール
//!
//! 依存関係上一番上に存在する。

#pragma once

#include "../error.hpp"

#include <string>

namespace graphics {

Error initialize(const std::string &title, int width, int height);

void terminate();

} // namespace graphics

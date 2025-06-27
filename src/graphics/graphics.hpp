//! グラフィックスに関するモジュール
//!
//! 依存関係上一番上に存在する。

#pragma once

#include "../config.hpp"
#include "../error.hpp"

#include <string>

namespace graphics {

Error initialize(const Config &config);

void terminate();

} // namespace graphics

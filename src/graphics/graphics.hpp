//! グラフィックスに関するモジュール

#pragma once

#include "../config.hpp"
#include "../error.hpp"

namespace graphics {

void initialize(const Config &config);

void render();

void terminate();

} // namespace graphics

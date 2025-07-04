//! グラフィックスに関するモジュール

#pragma once

#include "../config.hpp"
#include "../error.hpp"

namespace graphics {

void initialize(const Config &config);

void beginRender();

void endRender();

void terminate();

} // namespace graphics

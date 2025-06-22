//! ウィンドウに関するモジュール

#pragma once

#include "../error.hpp"

namespace graphics::window {

Error createWindow(const char *title, int width, int height);

void terminate();

} // namespace graphics::window

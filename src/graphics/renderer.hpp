//! レンダラに関するモジュール

#pragma once

#include "../error.hpp"

namespace graphics::renderer {

Error createInstance();
Error createDevice();
void terminate();

} // namespace graphics::renderer

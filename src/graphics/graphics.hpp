#pragma once

#include "../config/config.hpp"

namespace graphics {

void terminate();

void initialize(const config::Config &config);

void toggleFullscreen(const config::Config &config);

} // namespace graphics

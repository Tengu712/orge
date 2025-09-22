#pragma once

#include <orge/orge.h>

#include "../graphics/window/swapchain.hpp"

namespace api {

inline void isFullscreen(OrgeIsFullscreenParam *param) {
	*param->out = static_cast<uint8_t>(graphics::window::swapchain().isFullscreen());
}

inline void setFullscreen(OrgeSetFullscreenParam *param) {
	// NOTE: 発生する例外はすべて致命的であり、TRY_DISCARDで強制終了されるので、返戻型はvoid。
	graphics::window::swapchain().setFullscreen(static_cast<bool>(param->fullscreen));
}

} // namespace api

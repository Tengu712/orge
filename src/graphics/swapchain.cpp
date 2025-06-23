#include "swapchain.hpp"

namespace graphics::swapchain {

vk::SurfaceKHR g_surface;

Error initialize(const vk::SurfaceKHR &surface) {
	g_surface = surface;
	return Error::None;
}

} // namespace graphics::swapchain

#include "rendering.hpp"

namespace graphics::rendering {

Error initialize(const Config &config, const vk::Device &device) {
	return Error::None;
}

void terminate(const vk::Device &device) {
}

} // namespace graphics::rendering

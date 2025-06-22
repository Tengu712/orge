#include "renderer.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::renderer {

vk::Instance g_instance;

Error createInstance() {
	const auto ai = vk::ApplicationInfo()
		.setPEngineName("orge")
		.setApiVersion(VK_API_VERSION_1_1);
	const auto ci = vk::InstanceCreateInfo()
		.setPApplicationInfo(&ai);
	try {
		g_instance = vk::createInstance(ci);
	} catch (...) {
		return Error::CreateInstance;
	}
	return Error::None;
}

void terminate() {
	if (g_instance) {
		g_instance.destroy();
		g_instance = nullptr;
	}
}

} // namespace graphics::renderer

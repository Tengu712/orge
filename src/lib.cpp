#include <vulkan/vulkan.hpp>
#include <iostream>

namespace {
	vk::Instance g_instance = nullptr;
}

void orgeInitialize() {
	vk::InstanceCreateInfo ci;
	g_instance = vk::createInstance(ci);
	std::cout << "Hello, world!" << std::endl;
}

void orgeTerminate() {
	g_instance.destroy();
}

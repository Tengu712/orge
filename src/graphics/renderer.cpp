#include "renderer.hpp"

#include <optional>
#include <vulkan/vulkan.hpp>

namespace graphics::renderer {

vk::Instance g_instance;
vk::PhysicalDevice g_physicalDevice;
vk::Device g_device;

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

Error selectPhysicalDevice() {
	try {
		const auto devices = g_instance.enumeratePhysicalDevices();
		if (devices.empty()) {
			return Error::SelectPhysicalDevice;
		}
		// TODO: 適切なものを選ぶ
		g_physicalDevice = devices.front();
	} catch (...) {
		return Error::SelectPhysicalDevice;
	}
	return Error::None;
}

std::optional<size_t> getQueueFamilyIndex() {
	const auto props = g_physicalDevice.getQueueFamilyProperties();
	for (size_t i = 0; i < props.size(); ++i) {
		if (props[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			return i;
		}
	}
	return std::nullopt;
}

Error createDevice() {
	CHECK(selectPhysicalDevice());

	const auto queueFamilyIndex = getQueueFamilyIndex();
	if (!queueFamilyIndex) {
		return Error::CreateDevice;
	}

	const auto priority = 1.0f;
	const auto qci = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(static_cast<uint32_t>(queueFamilyIndex.value()))
		.setQueuePriorities(priority);
	const auto ci = vk::DeviceCreateInfo()
		.setQueueCreateInfos(qci);
	try {
		g_device = g_physicalDevice.createDevice(ci);
	} catch(...) {
		return Error::CreateDevice;
	}

	return Error::None;
}

void terminate() {
	if (g_device) {
		g_device.destroy();
		g_device = nullptr;
	}
	if (g_instance) {
		g_instance.destroy();
		g_instance = nullptr;
	}
}

} // namespace graphics::renderer

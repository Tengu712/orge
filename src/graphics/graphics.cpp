#include "graphics.hpp"

#include "platform.hpp"

#include <SDL3/SDL_vulkan.h>

namespace graphics {

std::vector<const char *> getInstanceExtensions() {
	const auto platformExtensions = platformInstanceExtensions();

	Uint32 count = 0;
	const auto windowExtensions_ = SDL_Vulkan_GetInstanceExtensions(&count);
	const auto windowExtensions = std::span(windowExtensions_, static_cast<size_t>(count));

	std::vector<const char *> extensions;
	extensions.reserve(platformExtensions.size() + windowExtensions.size());
	extensions.insert(extensions.end(), platformExtensions.begin(), platformExtensions.end());
	extensions.insert(extensions.end(), windowExtensions.begin(), windowExtensions.end());

	return extensions;
}

std::vector<const char *> getInstanceLayers() {
#ifdef ENABLE_VVL
	return {"VK_LAYER_KHRONOS_validation"};
#else
	return {};
#endif
}

vk::Instance createInstance() {
	const auto extensions = getInstanceExtensions();
	const auto layers = getInstanceLayers();
	const auto ai = vk::ApplicationInfo()
		.setPEngineName("orge")
		.setApiVersion(VK_API_VERSION_1_1);
	const auto ci = vk::InstanceCreateInfo()
		.setFlags(platformInstanceCreateFlags())
		.setPApplicationInfo(&ai)
		.setPEnabledExtensionNames(extensions)
		.setPEnabledLayerNames(layers);
	return vk::createInstance(ci);
}

vk::PhysicalDevice selectPhysicalDevice(const vk::Instance &instance) {
	const auto devices = instance.enumeratePhysicalDevices();
	if (devices.empty()) {
		throw "no physical device found.";
	}
	// TODO: 適切なものを選ぶ
	return devices.front();
}

uint32_t getQueueFamilyIndex(const vk::PhysicalDevice &physicalDevice) {
	const auto props = physicalDevice.getQueueFamilyProperties();
	const auto iter = std::find_if(
		props.cbegin(),
		props.cend(),
		[](const auto &n) { return n.queueFlags & vk::QueueFlagBits::eGraphics; }
	);
	if (iter == props.cend()) {
		throw "failed to get a queue family index.";
	}
	return static_cast<uint32_t>(std::distance(props.cbegin(), iter));
}

std::vector<const char *> getDeviceExtensions() {
	const auto platformExtensions = platformDeviceExtensions();

	std::vector<const char *> extensions;
	extensions.reserve(platformExtensions.size() + 1);
	extensions.insert(extensions.end(), platformExtensions.begin(), platformExtensions.end());
	extensions.push_back("VK_KHR_swapchain");

	return extensions;
}

vk::Device createDevice(const vk::PhysicalDevice &physicalDevice, uint32_t queueFamilyIndex) {
	const auto extensions = getDeviceExtensions();
	const auto priority = 1.0f;
	const auto qci = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(queueFamilyIndex)
		.setQueuePriorities(priority);
	const auto ci = vk::DeviceCreateInfo()
		.setQueueCreateInfos(qci)
		.setPEnabledExtensionNames(extensions);
	return physicalDevice.createDevice(ci);
}

Graphics::Graphics(const config::Config &config) :
	_instance(createInstance()),
	_physicalDevice(selectPhysicalDevice(_instance)),
	_queueFamilyIndex(getQueueFamilyIndex(_physicalDevice)),
	_device(createDevice(_physicalDevice, _queueFamilyIndex)),
	_queue(_device.getQueue(_queueFamilyIndex, 0)),
	_commandPool(_device.createCommandPool(
		vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(_queueFamilyIndex)
	)),
	_renderer(config, _instance, _physicalDevice, _device, _commandPool)
{
	initializeUtils(_device, _commandPool);
}

} // namespace graphics

#include "graphics.hpp"

#include <SDL3/SDL_vulkan.h>

namespace graphics {

std::vector<const char *> getInstanceExtensions() {
	Uint32 count = 0;
	const auto extensions_ = SDL_Vulkan_GetInstanceExtensions(&count);
	auto extensions = std::vector(extensions_, extensions_ + static_cast<size_t>(count));
#ifdef __APPLE__
	extensions.push_back("VK_KHR_portability_enumeration");
#endif
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
#ifdef __APPLE__
	const auto flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#else
	const auto flags = vk::InstanceCreateFlags();
#endif
	const auto extensions = getInstanceExtensions();
	const auto layers = getInstanceLayers();
	const auto ai = vk::ApplicationInfo()
		.setPEngineName("orge")
		.setApiVersion(VK_API_VERSION_1_1);
	const auto ci = vk::InstanceCreateInfo()
		.setFlags(flags)
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

vk::Device createDevice(const vk::PhysicalDevice &physicalDevice, uint32_t queueFamilyIndex) {
#ifdef __APPLE__
	const std::array<const char *, 2> extensions{"VK_KHR_swapchain", "VK_KHR_portability_subset"};
#else
	const std::array<const char *, 1> extensions{"VK_KHR_swapchain"};
#endif
	const auto priority = 1.0f;
	const auto qci = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(queueFamilyIndex)
		.setQueuePriorities(priority);
	const auto ci = vk::DeviceCreateInfo()
		.setQueueCreateInfos(qci)
		.setPEnabledExtensionNames(extensions);
	return physicalDevice.createDevice(ci);
}

Graphics::Graphics() :
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
	_renderer(_instance, _physicalDevice, _device, _commandPool)
{
	initializeUtils(_device, _commandPool);
	_charAtlus = CharAtlus::create(_physicalDevice.getMemoryProperties(), _device, _queue, "MPLUS1p-Regular.ttf");
}

} // namespace graphics

#include "core.hpp"

#include <SDL3/SDL_vulkan.h>

namespace {

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

} // namespace

namespace graphics::core {

struct Core {
	const vk::Instance instance;
	const vk::PhysicalDevice physicalDevice;
	const uint32_t queueFamilyIndex;
	const vk::Device device;
	const vk::Queue queue;
	const vk::CommandPool commandPool;

	Core(const Core &) = delete;
	Core(const Core &&) = delete;
	Core &operator =(const Core &) = delete;
	Core &operator =(const Core &&) = delete;

	Core():
		instance(createInstance()),
		physicalDevice(selectPhysicalDevice(instance)),
		queueFamilyIndex(getQueueFamilyIndex(physicalDevice)),
		device(createDevice(physicalDevice, queueFamilyIndex)),
		queue(device.getQueue(queueFamilyIndex, 0)),
		commandPool(device.createCommandPool(
		vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(queueFamilyIndex)
		))
	{}

	~Core() {
		device.waitIdle();
		device.destroy(commandPool);
		device.destroy();
		instance.destroy();
	}
};

std::optional<Core> g_core;

void ensureCoreInitialized() {
	if (!g_core) {
		throw "core objects not initialized.";
	}
}

void initializeCore() {
	if (g_core) {
		throw "core objects already initialized.";
	}
	g_core.emplace();
}

void destroyCore() noexcept {
	g_core.reset();
}

const vk::Instance &instance() {
	ensureCoreInitialized();
	return g_core->instance;
}

const vk::PhysicalDevice &physicalDevice() {
	ensureCoreInitialized();
	return g_core->physicalDevice;
}

const vk::Device &device() {
	ensureCoreInitialized();
	return g_core->device;
}

const vk::Queue &queue() {
	ensureCoreInitialized();
	return g_core->queue;
}

const vk::CommandPool &commandPool() {
	ensureCoreInitialized();
	return g_core->commandPool;
}

} // namespace graphics::core

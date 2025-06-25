#include "graphics.hpp"

#include "swapchain.hpp"
#include "window.hpp"

#include <array>
#include <algorithm>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace graphics {

#ifdef __APPLE__
constexpr auto INSTANCE_FLAGS = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
constexpr std::array<const char *const, 1> INSTANCE_EXTENSIONS = {"VK_KHR_portability_enumeration"};
constexpr std::array<const char *const, 1> DEVICE_EXTENSIONS = {"VK_KHR_portability_subset"};
#else
constexpr auto INSTANCE_FLAGS = vk::InstanceCreateFlags();
constexpr std::array<const char *const, 0> INSTANCE_EXTENSIONS = {};
constexpr std::array<const char *const, 0> DEVICE_EXTENSIONS = {};
#endif

vk::Instance g_instance;
vk::PhysicalDevice g_physicalDevice;
vk::Device g_device;
vk::Queue g_queue;
vk::CommandPool g_commandPool;

std::vector<const char *> getInstanceExtensions() {
	const auto window_extensions = window::getExtensions();

	std::vector<const char *> extensions;
	extensions.reserve(INSTANCE_EXTENSIONS.size() + window_extensions.size());
	extensions.insert(extensions.end(), INSTANCE_EXTENSIONS.begin(), INSTANCE_EXTENSIONS.end());
	extensions.insert(extensions.end(), window_extensions.begin(), window_extensions.end());

	return extensions;
}

Error createInstance() {
	const auto extensions = getInstanceExtensions();

	const auto ai = vk::ApplicationInfo()
		.setPEngineName("orge")
		.setApiVersion(VK_API_VERSION_1_1);
	const auto ci = vk::InstanceCreateInfo()
		.setFlags(INSTANCE_FLAGS)
		.setPApplicationInfo(&ai)
		.setPEnabledExtensionNames(extensions);

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

std::optional<uint32_t> getQueueFamilyIndex() {
	const auto props = g_physicalDevice.getQueueFamilyProperties();
	const auto iter = std::find_if(props.cbegin(), props.cend(), [](const auto &n) { return n.queueFlags & vk::QueueFlagBits::eGraphics; });
	if (iter == props.cend()) {
		return std::nullopt;
	}
	return static_cast<uint32_t>(std::distance(props.cbegin(), iter));
}

std::vector<const char *> getDeviceExtensions() {
	const auto swapchain_extensions = swapchain::getExtensions();

	std::vector<const char *> extensions;
	extensions.reserve(DEVICE_EXTENSIONS.size() + swapchain_extensions.size());
	extensions.insert(extensions.end(), DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
	extensions.insert(extensions.end(), swapchain_extensions.begin(), swapchain_extensions.end());

	return extensions;
}

Error createDevice(uint32_t queueFamilyIndex) {
	const auto extensions = getDeviceExtensions();

	const auto priority = 1.0f;
	const auto qci = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(queueFamilyIndex)
		.setQueuePriorities(priority);
	const auto ci = vk::DeviceCreateInfo()
		.setQueueCreateInfos(qci)
		.setPEnabledExtensionNames(extensions);

	try {
		g_device = g_physicalDevice.createDevice(ci);
	} catch (...) {
		return Error::CreateDevice;
	}

	return Error::None;
}

void getQueue(uint32_t queueFamilyIndex) {
	g_queue = g_device.getQueue(queueFamilyIndex, 0);
}

Error createCommandPool(uint32_t queueFamilyIndex) {
	const auto ci = vk::CommandPoolCreateInfo()
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(queueFamilyIndex);
	try {
		g_commandPool = g_device.createCommandPool(ci);
	} catch (...) {
		return Error::CreateCommandPool;
	}
	return Error::None;
}

Error initialize(const char *title, int width, int height) {
	// ウィンドウ作成
	// NOTE: 予めSDLにVulkanを使うことを伝えなければならないのでここで。
	CHECK(window::createWindow(title, width, height));

	// インスタンス
	CHECK(createInstance());

	// デバイス
	CHECK(selectPhysicalDevice());
	const auto queueFamilyIndex = getQueueFamilyIndex();
	if (!queueFamilyIndex) {
		return Error::GetQueueFamilyIndex;
	}
	CHECK(createDevice(queueFamilyIndex.value()));

	// キュー
	getQueue(queueFamilyIndex.value());

	// コマンドプール
	CHECK(createCommandPool(queueFamilyIndex.value()));

	// スワップチェイン
	// NOTE: ウィンドウから得たサーフェスはスワップチェイン側で管理してもらう。
	const auto surface = window::createSurface(g_instance);
	if (!surface) {
		return Error::CreateSurface;
	}
	CHECK(swapchain::initialize(g_physicalDevice, g_device, surface.value()));

	return Error::None;
}

void terminate() {
	if (g_device) {
		g_device.waitIdle();
	}
	swapchain::terminate(g_device);
	if (g_device && g_commandPool) {
		g_device.destroyCommandPool(g_commandPool);
		g_commandPool = nullptr;
	}
	if (g_device) {
		g_device.destroy();
		g_device = nullptr;
	}
	if (g_instance) {
		g_instance.destroy();
		g_instance = nullptr;
	}
}

} // namespace graphics

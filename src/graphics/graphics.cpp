#include "graphics.hpp"

#include "mesh.hpp"
#include "pipeline/buffer.hpp"
#include "pipeline.hpp"
#include "platform.hpp"
#include "rendering.hpp"
#include "swapchain.hpp"
#include "window.hpp"

#include <algorithm>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace graphics {

vk::Instance g_instance;
vk::PhysicalDevice g_physicalDevice;
vk::Device g_device;
vk::Queue g_queue;
vk::CommandPool g_commandPool;

std::vector<const char *> getInstanceExtensions() {
	const auto platform_extensions = platform::getInstanceExtensions();
	const auto window_extensions = window::getInstanceExtensions();

	std::vector<const char *> extensions;
	extensions.reserve(platform_extensions.size() + window_extensions.size());
	extensions.insert(extensions.end(), platform_extensions.begin(), platform_extensions.end());
	extensions.insert(extensions.end(), window_extensions.begin(), window_extensions.end());

	return extensions;
}

std::vector<const char *> getInstanceLayers() {
#ifdef ENABLE_VVL
	return {"VK_LAYER_KHRONOS_validation"};
#else
	return {};
#endif
}

void createInstance() {
	const auto extensions = getInstanceExtensions();
	const auto layers = getInstanceLayers();

	const auto ai = vk::ApplicationInfo()
		.setPEngineName("orge")
		.setApiVersion(VK_API_VERSION_1_1);
	const auto ci = vk::InstanceCreateInfo()
		.setFlags(platform::getInstanceCreateFlags())
		.setPApplicationInfo(&ai)
		.setPEnabledExtensionNames(extensions)
		.setPEnabledLayerNames(layers);
	g_instance = vk::createInstance(ci);
}

void selectPhysicalDevice() {
	const auto devices = g_instance.enumeratePhysicalDevices();
	if (devices.empty()) {
		throw "no physical device found.";
	}
	// TODO: 適切なものを選ぶ
	g_physicalDevice = devices.front();
}

uint32_t getQueueFamilyIndex() {
	const auto props = g_physicalDevice.getQueueFamilyProperties();
	const auto iter = std::find_if(props.cbegin(), props.cend(), [](const auto &n) { return n.queueFlags & vk::QueueFlagBits::eGraphics; });
	if (iter == props.cend()) {
		throw "failed to get a queue family index.";
	}
	return static_cast<uint32_t>(std::distance(props.cbegin(), iter));
}

std::vector<const char *> getDeviceExtensions() {
	const auto platform_extensions = platform::getDeviceExtensions();
	const auto swapchain_extensions = swapchain::getDeviceExtensions();

	std::vector<const char *> extensions;
	extensions.reserve(platform_extensions.size() + swapchain_extensions.size());
	extensions.insert(extensions.end(), platform_extensions.begin(), platform_extensions.end());
	extensions.insert(extensions.end(), swapchain_extensions.begin(), swapchain_extensions.end());

	return extensions;
}

void createDevice(uint32_t queueFamilyIndex) {
	const auto extensions = getDeviceExtensions();

	const auto priority = 1.0f;
	const auto qci = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(queueFamilyIndex)
		.setQueuePriorities(priority);
	const auto ci = vk::DeviceCreateInfo()
		.setQueueCreateInfos(qci)
		.setPEnabledExtensionNames(extensions);
	g_device = g_physicalDevice.createDevice(ci);
}

void getQueue(uint32_t queueFamilyIndex) {
	g_queue = g_device.getQueue(queueFamilyIndex, 0);
}

void createCommandPool(uint32_t queueFamilyIndex) {
	const auto ci = vk::CommandPoolCreateInfo()
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(queueFamilyIndex);
	g_commandPool = g_device.createCommandPool(ci);
}

void initialize(const config::Config &config) {
	// ウィンドウ作成
	// NOTE: 予めSDLにVulkanを使うことを伝えなければならないのでここで。
	window::initialize(config.title, config.width, config.height);

	// インスタンス
	createInstance();

	// デバイス
	selectPhysicalDevice();
	const auto queueFamilyIndex = getQueueFamilyIndex();
	createDevice(queueFamilyIndex);

	// キュー
	getQueue(queueFamilyIndex);

	// コマンドプール
	createCommandPool(queueFamilyIndex);

	// スワップチェイン
	swapchain::initialize(g_instance, g_physicalDevice, g_device);

	// 描画処理オブジェクト
	rendering::initialize(config, g_device, g_commandPool);
}

void createBuffer(const char *id, uint64_t size, int isStorage) {
	pipeline::buffer::create(g_device, g_physicalDevice.getMemoryProperties(), id, size, isStorage);
}

void updateBuffer(const char *id, const void *data) {
	pipeline::buffer::update(g_device, id, data);
}

void updateBufferDescriptor(
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding
) {
	pipeline::updateBufferDescriptor(g_device, bufferId, pipelineId, set, index, binding);
}

void createMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
) {
	mesh::createMesh(g_physicalDevice.getMemoryProperties(), g_device, id, vertexCount, vertices, indexCount, indices);
}

void beginRender() {
	rendering::beginRender(g_device);
}

void endRender() {
	rendering::endRender(g_device, g_queue);
}

void terminate() {
	if (g_device) {
		g_device.waitIdle();
	}

	pipeline::buffer::terminate(g_device);
	mesh::terminate(g_device);
	rendering::terminate(g_device);
	swapchain::terminate(g_instance, g_device);

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

	window::terminate();
}

} // namespace graphics

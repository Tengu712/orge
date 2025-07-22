#include "graphics.hpp"

#include "platform.hpp"
#include "rendering/swapchain/swapchain.hpp"
#include "rendering/rendering.hpp"

#include <vulkan/vulkan.hpp>
#include <SDL3/SDL_vulkan.h>

namespace graphics {

vk::Instance g_instance;
vk::PhysicalDevice g_physicalDevice;
vk::Device g_device;
vk::Queue g_queue;
vk::CommandPool g_commandPool;
std::unique_ptr<rendering::Renderer> g_renderer;

void terminate() {
	if (g_device) {
		g_device.waitIdle();
	}

	g_renderer->destroy(g_instance, g_device);

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

void createInstance() {
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
	const auto platformExtensions = platformDeviceExtensions();

	std::vector<const char *> extensions;
	extensions.reserve(platformExtensions.size() + 1);
	extensions.insert(extensions.end(), platformExtensions.begin(), platformExtensions.end());
	extensions.push_back("VK_KHR_swapchain");

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
	createInstance();

	selectPhysicalDevice();
	const auto queueFamilyIndex = getQueueFamilyIndex();
	createDevice(queueFamilyIndex);

	getQueue(queueFamilyIndex);

	createCommandPool(queueFamilyIndex);

	g_renderer = std::make_unique<rendering::Renderer>(config, g_instance, g_physicalDevice, g_device, g_commandPool);
}

void toggleFullscreen(const config::Config &config) {
	g_renderer->toggleFullscreen(config, g_instance, g_physicalDevice, g_device);
}

} // namespace graphics

#include "../error/error.hpp"
#include "mesh/mesh.hpp"
#include "rendering/pipeline/buffer/buffer.hpp"
#include "rendering/pipeline/image/image.hpp"
#include "rendering/pipeline/pipeline.hpp"
#include "rendering/pipeline/sampler/sampler.hpp"

int orgeCreateBuffer(const char *id, uint64_t size, int isStorage) {
	TRY(graphics::rendering::pipeline::buffer::create(
		graphics::g_physicalDevice.getMemoryProperties(),
		graphics::g_device,
		id,
		size,
		isStorage
	));
}

void orgeDestroyBuffer(const char *id) {
	graphics::rendering::pipeline::buffer::destroy(graphics::g_device, id);
}

int orgeUpdateBuffer(const char *id, const void *data) {
	TRY(graphics::rendering::pipeline::buffer::update(graphics::g_device, id, data));
}

int orgeUpdateBufferDescriptor(
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(graphics::rendering::pipeline::updateBufferDescriptor(
		graphics::g_device,
		bufferId,
		pipelineId,
		set,
		index,
		binding,
		offset
	));
}

int orgeCreateImage(const char *id, uint32_t width, uint32_t height, const unsigned char *pixels) {
	TRY(graphics::rendering::pipeline::image::create(
		graphics::g_physicalDevice.getMemoryProperties(),
		graphics::g_device,
		graphics::g_queue,
		id,
		width,
		height,
		pixels
	));
}

int orgeCreateImageFromFile(const char *id, const char *path) {
	TRY(graphics::rendering::pipeline::image::createFromFile(
		graphics::g_physicalDevice.getMemoryProperties(),
		graphics::g_device,
		graphics::g_queue,
		id,
		path
	));
}

void orgeDestroyImage(const char *id) {
	graphics::rendering::pipeline::image::destroy(graphics::g_device, id);
}

int orgeUpdateImageDescriptor(
	const char *imageId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(graphics::rendering::pipeline::updateImageDescriptor(
		graphics::g_device,
		imageId,
		pipelineId,
		set,
		index,
		binding,
		offset
	));
}

int orgeCreateSampler(const char *id, int linearMagFilter, int linearMinFilter, int repeat) {
	TRY(graphics::rendering::pipeline::sampler::create(
		graphics::g_device,
		id,
		linearMagFilter,
		linearMinFilter,
		repeat
	));
}

void orgeDestroySampler(const char *id) {
	graphics::rendering::pipeline::sampler::destroy(graphics::g_device, id);
}

int orgeUpdateSamplerDescriptor(
	const char *samplerId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(graphics::rendering::pipeline::updateSamplerDescriptor(
		graphics::g_device,
		samplerId,
		pipelineId,
		set,
		index,
		binding,
		offset
	));
}

int orgeCreateMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
) {
	TRY(graphics::mesh::createMesh(
		graphics::g_physicalDevice.getMemoryProperties(),
		graphics::g_device,
		id,
		vertexCount,
		vertices,
		indexCount,
		indices
	));
}

void orgeDestroyMesh(const char *id) {
	graphics::mesh::destroy(graphics::g_device, id);
}

int orgeBeginRender() {
	TRY(graphics::g_renderer->beginRender(graphics::g_device));
}

int orgeDraw(const char *pipelineId, const char *meshId, uint32_t instanceCount, uint32_t instanceOffset) {
	TRY(
		graphics::g_renderer->bindPipeline(graphics::g_device, pipelineId);
		graphics::g_renderer->bindMesh(meshId);
		graphics::g_renderer->draw(instanceCount, instanceOffset);
	)
}

int orgeEndRender() {
	TRY(graphics::g_renderer->endRender(graphics::g_queue));
}

int orgeBindDescriptorSets(const char *id, uint32_t const *indices) {
	TRY(graphics::g_renderer->bindDescriptorSets(id, indices));
}

void orgeNextSubpass() {
	graphics::g_renderer->nextSubpass();
}

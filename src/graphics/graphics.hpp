#pragma once

#include "../error/error.hpp"
#include "charatlus.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "mesh.hpp"
#include "renderer.hpp"

#include <string>
#include <unordered_map>

namespace graphics {

class Graphics {
private:
	const vk::Instance _instance;
	const vk::PhysicalDevice _physicalDevice;
	const uint32_t _queueFamilyIndex;
	const vk::Device _device;
	const vk::Queue _queue;
	const vk::CommandPool _commandPool;
	Renderer _renderer;
	std::unordered_map<std::string, Buffer> _buffers;
	std::unordered_map<std::string, Image> _images;
	std::unordered_map<std::string, vk::Sampler> _samplers;
	std::unordered_map<std::string, Mesh> _meshes;
	std::unordered_map<std::string, CharAtlus> _charAtluss;

public:
	Graphics(const Graphics &)  = delete;
	Graphics(const Graphics &&) = delete;
	Graphics &operator =(const Graphics &)  = delete;
	Graphics &operator =(const Graphics &&) = delete;

	Graphics();

	~Graphics() {
		_device.waitIdle();
		terminateUtils(_device);
		for (const auto &n: _charAtluss) {
			n.second.destroy(_device);
		}
		for (const auto &n: _meshes) {
			n.second.destroy(_device);
		}
		for (const auto &n: _samplers) {
			_device.destroySampler(n.second);
		}
		for (const auto &n: _images) {
			n.second.destroy(_device);
		}
		for (const auto &n: _buffers) {
			n.second.destroy(_device);
		}
		_renderer.destroy(_instance, _device);
		_device.destroyCommandPool(_commandPool);
		_device.destroy();
		_instance.destroy();
	}

	void createBuffer(const std::string &id, uint64_t size, bool isStorage) {
		_buffers.emplace(id, Buffer(_physicalDevice.getMemoryProperties(), _device, size, isStorage));
	}

	void destroyBuffer(const std::string &id) noexcept {
		if (_buffers.contains(id)) {
			_buffers.at(id).destroy(_device);
			_buffers.erase(id);
		}
	}

	void updateBuffer(const std::string &id, const void *data) const {
		error::at(_buffers, id, "buffers").update(_device, data);
	}

	void updateBufferDescriptor(
		const std::string &bufferId,
		const std::string &pipelineId,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset
	) const {
		_renderer
			.getPipeline(pipelineId)
			.updateBufferDescriptor(_device, error::at(_buffers, bufferId, "buffers"), set, index, binding, offset);
	}

	void createImage(const std::string &id, uint32_t width, uint32_t height, const uint8_t *pixels) {
		_images.emplace(id, Image(_physicalDevice.getMemoryProperties(), _device, _queue, width, height, pixels, false));
	}

	void createImage(const std::string &id, const std::string &path) {
		_images.emplace(id, Image::fromFile(_physicalDevice.getMemoryProperties(), _device, _queue, path));
	}

	void destroyImage(const std::string &id) noexcept {
		if (_images.contains(id)) {
			_images.at(id).destroy(_device);
			_images.erase(id);
		}
	}

	void updateImageDescriptor(
		const std::string &imageId,
		const std::string &pipelineId,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset
	) const {
		const auto &image = _charAtluss.contains(imageId)
			? _charAtluss.at(imageId).get()
			: error::at(_images, imageId, "images");
		_renderer
			.getPipeline(pipelineId)
			.updateImageDescriptor(_device, image, set, index, binding, offset);
	}

	void createSampler(const std::string &id, bool linearMagFilter, bool linearMinFilter, bool repeat) {
		_samplers.emplace(id, _device.createSampler(
			vk::SamplerCreateInfo()
				.setMagFilter(linearMagFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
				.setMinFilter(linearMinFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
				.setMipmapMode(vk::SamplerMipmapMode::eLinear)
				.setAddressModeU(repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge)
				.setAddressModeV(repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge)
				.setMaxLod(vk::LodClampNone)
		));
	}

	void destroySampler(const std::string &id) noexcept {
		if (_samplers.contains(id)) {
			_device.destroySampler(_samplers.at(id));
			_samplers.erase(id);
		}
	}

	void updateSamplerDescriptor(
		const std::string &samplerId,
		const std::string &pipelineId,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset
	) const {
		_renderer
			.getPipeline(pipelineId)
			.updateSamplerDescriptor(_device, error::at(_samplers, samplerId, "samplers"), set, index, binding, offset);
	}

	void createMesh(
		const std::string &id,
		const uint32_t vertexCount,
		const float *vertices,
		const uint32_t indexCount,
		const uint32_t *indices
	) {
		_meshes.emplace(id, Mesh(
			_physicalDevice.getMemoryProperties(),
			_device,
			_queue,
			vertexCount,
			vertices,
			indexCount,
			indices
		));
	}

	void destroyMesh(const std::string &id) noexcept {
		if (_meshes.contains(id)) {
			_meshes.at(id).destroy(_device);
			_meshes.erase(id);
		}
	}

	void putString(const std::string &id, const std::string &s) {
		error::atMut(_charAtluss, id, "fonts").putString(_physicalDevice.getMemoryProperties(), _device, _queue, s);
	}

	void beginRender() {
		_renderer.beginRender(_device);
	}

	void bindDescriptorSets(const std::string &pipelineId, uint32_t const *indices) const {
		_renderer.bindDescriptorSets(pipelineId, indices);
	}

	void draw(const std::string &pipelineId, const std::string &meshId, uint32_t instanceCount, uint32_t instanceOffset) {
		_renderer.bindPipeline(_device, pipelineId);
		_renderer.bindMesh(meshId, error::at(_meshes, meshId, "meshes"));
		_renderer.draw(instanceCount, instanceOffset);
	}

	void nextSubpass() const {
		_renderer.nextSubpass();
	}

	void endRender() {
		_renderer.endRender(_device, _queue);
	}

	void resetRendering() {
		_device.waitIdle();
		_renderer.resetRendering(_device);
	}

	void recreateSwapchain() {
		_device.waitIdle();
		_renderer.recreateSwapchain(_physicalDevice, _device);
	}

	void recreateSurface() {
		_device.waitIdle();
		_renderer.recreateSurface(_instance, _physicalDevice, _device);
	}

	bool isFullscreen() const noexcept {
		return _renderer.isFullscreen();
	}

	void setFullscreen(bool toFullscreen) const {
		_device.waitIdle();
		_renderer.setFullscreen(toFullscreen);
	}
};

} // namespace graphics

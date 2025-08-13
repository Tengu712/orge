#pragma once

#include "../error/error.hpp"
#include "charatlus.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "mesh.hpp"
#include "renderer.hpp"

#include <orge.h>
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
	std::unordered_map<std::string, uint32_t> _charCounts;
	std::unordered_map<std::string, size_t> _textOffset;

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

	void loadImage(const std::string &file) {
		_images.emplace(file, Image::fromFile(_physicalDevice.getMemoryProperties(), _device, _queue, file));
	}

	void destroyImage(const std::string &file) noexcept {
		if (_images.contains(file)) {
			_images.at(file).destroy(_device);
			_images.erase(file);
		}
	}

	void updateImageDescriptor(
		const std::string &imageFile,
		const std::string &pipelineId,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset
	) const {
		_renderer
			.getPipeline(pipelineId)
			.updateImageDescriptor(_device, error::at(_images, imageFile, "images"), set, index, binding, offset);
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

	void createMesh(const std::string &id) {
		_meshes.emplace(id, Mesh(_physicalDevice.getMemoryProperties(), _device, _queue, id));
	}

	void destroyMesh(const std::string &id) noexcept {
		if (_meshes.contains(id)) {
			_meshes.at(id).destroy(_device);
			_meshes.erase(id);
		}
	}

	void rasterizeCharacters(const std::string &id, const std::string &s) {
		error::atMut(_charAtluss, id, "fonts")
			.rasterizeCharacters(_physicalDevice.getMemoryProperties(), _device, _queue, s);
	}

	void putText(
		const std::string &pipelineId,
		const std::string &fontId,
		const std::string &text,
		float x,
		float y,
		float height,
		OrgeTextLocationHorizontal horizontal,
		OrgeTextLocationVertical vertical
	);

	void drawTexts(const std::string &pipelineId) {
		if (!_textOffset.contains(pipelineId)) {
			return;
		}
		const uint32_t indices[] = {0, 0};
		_renderer.bindDescriptorSets(pipelineId, indices);
		_renderer.bindPipeline(_device, pipelineId);
		_renderer.drawDirectly(4, static_cast<uint32_t>(_textOffset[pipelineId]), 0);
	}

	void beginRender();

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
		_textOffset.clear();
	}

	void resetRendering() {
		_device.waitIdle();
		_textOffset.clear();
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

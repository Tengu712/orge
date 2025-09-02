#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::resource {

class Mesh {
private:
	const std::string &_id;
	const uint32_t _iCount;
	const vk::Buffer _vb;
	const vk::Buffer _ib;
	const vk::DeviceMemory _vbMemory;
	const vk::DeviceMemory _ibMemory;

public:
	Mesh() = delete;
	Mesh(const std::string &id);
	~Mesh();

	const std::string &id() const noexcept {
		return _id;
	}

	uint32_t indexCount() const noexcept {
		return _iCount;
	}

	void bind(const vk::CommandBuffer &commandBuffer) const noexcept {
		const VkDeviceSize offset = 0;
		commandBuffer.bindVertexBuffers(0, 1, &_vb, &offset);
		commandBuffer.bindIndexBuffer(_ib, offset, vk::IndexType::eUint32);
	}
};

void destroyAllMeshes() noexcept;

void addMesh(const std::string &id);

void destroyMesh(const std::string &id) noexcept;

const Mesh &getMesh(const std::string &id);

} // namespace graphics::resource

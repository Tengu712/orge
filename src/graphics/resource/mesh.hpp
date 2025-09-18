#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::resource {

class Mesh {
private:
	const std::string &_id;
	const uint32_t _iCount;
	const vk::UniqueBuffer _vb;
	const vk::UniqueBuffer _ib;
	const vk::UniqueDeviceMemory _vbMemory;
	const vk::UniqueDeviceMemory _ibMemory;

public:
	Mesh() = delete;
	Mesh(const Mesh &) = delete;
	Mesh &operator =(const Mesh &) = delete;

	Mesh(const std::string &id);

	const std::string &id() const noexcept {
		return _id;
	}

	uint32_t indexCount() const noexcept {
		return _iCount;
	}

	void bind(const vk::CommandBuffer &commandBuffer) const noexcept {
		const VkDeviceSize offset = 0;
		commandBuffer.bindVertexBuffers(0, 1, &_vb.get(), &offset);
		commandBuffer.bindIndexBuffer(_ib.get(), offset, vk::IndexType::eUint32);
	}
};

void destroyAllMeshes() noexcept;

void addMesh(const std::string &id);

void destroyMesh(const std::string &id) noexcept;

const Mesh &getMesh(const std::string &id);

} // namespace graphics::resource

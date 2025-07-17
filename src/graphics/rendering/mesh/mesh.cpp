#include "mesh.hpp"

#include "../../utils.hpp"

#include <unordered_map>

namespace graphics::rendering::mesh {

struct Mesh {
	const uint32_t iCount;
	const vk::Buffer vb;
	const vk::Buffer ib;
	const vk::DeviceMemory vbMemory;
	const vk::DeviceMemory ibMemory;
};

std::unordered_map<std::string, Mesh> g_meshes;

void terminate(const vk::Device &device) {
	for (auto &n: g_meshes) {
		device.freeMemory(n.second.ibMemory);
		device.freeMemory(n.second.vbMemory);
		device.destroyBuffer(n.second.ib);
		device.destroyBuffer(n.second.vb);
	}
	g_meshes.clear();
}

void createMesh(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
) {
	const auto vbSize = sizeof(float) * vertexCount;
	const auto ibSize = sizeof(uint32_t) * indexCount;

	const auto vb = device.createBuffer(
		vk::BufferCreateInfo()
			.setSize(vbSize)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
	);
	const auto ib = device.createBuffer(
		vk::BufferCreateInfo()
			.setSize(ibSize)
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer)
	);

	const auto vbMemory = allocateBufferMemory(
		memoryProps,
		device,
		vb,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	const auto ibMemory = allocateBufferMemory(
		memoryProps,
		device,
		ib,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	copyDataToMemory(device, vbMemory, vertices, vbSize);
	copyDataToMemory(device, ibMemory, indices,  ibSize);

	g_meshes.emplace(id, Mesh {
		static_cast<uint32_t>(indexCount),
		vb,
		ib,
		vbMemory,
		ibMemory,
	});
}

uint32_t bind(const vk::CommandBuffer &commandBuffer, const char *id) {
	const VkDeviceSize offset = 0;
	commandBuffer.bindVertexBuffers(0, 1, &g_meshes.at(id).vb, &offset);
	commandBuffer.bindIndexBuffer(g_meshes.at(id).ib, offset, vk::IndexType::eUint32);
	return g_meshes.at(id).iCount;
}

} // namespace graphics::rendering::mesh

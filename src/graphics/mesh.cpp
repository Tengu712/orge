#include "mesh.hpp"

#include "utils.hpp"

#include <unordered_map>

namespace graphics::mesh {

struct Mesh {
	const uint32_t indicesCount;
	const vk::Buffer vertexBuffer;
	const vk::Buffer indexBuffer;
	const vk::DeviceMemory vertexBufferMemory;
	const vk::DeviceMemory indexBufferMemory;
};

std::unordered_map<std::string, Mesh> g_meshes;

void createMesh(
	const vk::PhysicalDeviceMemoryProperties &physicalDeviceMemoryProps,
	const vk::Device &device,
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
) {
	const auto vertexBuffer = device.createBuffer(
		vk::BufferCreateInfo()
			.setSize(sizeof(float) * vertexCount)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
	);
	const auto indexBuffer = device.createBuffer(
		vk::BufferCreateInfo()
			.setSize(sizeof(float) * indexCount)
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer)
	);

	const auto vbMemoryReqs = device.getBufferMemoryRequirements(vertexBuffer);
	const auto ibMemoryReqs = device.getBufferMemoryRequirements(indexBuffer);

	const auto vbMemoryType = utils::findMemoryType(
		physicalDeviceMemoryProps,
		vbMemoryReqs.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	const auto ibMemoryType = utils::findMemoryType(
		physicalDeviceMemoryProps,
		ibMemoryReqs.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	const auto vertexBufferMemory = device.allocateMemory(vk::MemoryAllocateInfo(vbMemoryReqs.size, vbMemoryType));
	const auto indexBufferMemory  = device.allocateMemory(vk::MemoryAllocateInfo(ibMemoryReqs.size, ibMemoryType));

	const auto vbData = static_cast<uint8_t *>(device.mapMemory(vertexBufferMemory, 0, vbMemoryReqs.size));
	memcpy(vbData, vertices, sizeof(float) * vertexCount);
	device.unmapMemory(vertexBufferMemory);

	const auto ibData = static_cast<uint8_t *>(device.mapMemory(indexBufferMemory, 0, ibMemoryReqs.size));
	memcpy(ibData, indices, sizeof(float) * indexCount);
	device.unmapMemory(indexBufferMemory);

	device.bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);
	device.bindBufferMemory(indexBuffer,  indexBufferMemory,  0);

	g_meshes.emplace(id, Mesh {
		static_cast<uint32_t>(indexCount),
		vertexBuffer,
		indexBuffer,
		vertexBufferMemory,
		indexBufferMemory
	});
}

uint32_t bind(const vk::CommandBuffer &commandBuffer, const char *id) {
	const VkDeviceSize offset = 0;
	commandBuffer.bindVertexBuffers(0, 1, &g_meshes.at(id).vertexBuffer, &offset);
	commandBuffer.bindIndexBuffer(g_meshes.at(id).indexBuffer, offset, vk::IndexType::eUint32);
	return g_meshes.at(id).indicesCount;
}

void terminate(const vk::Device &device) {
	for (auto &n: g_meshes) {
		device.freeMemory(n.second.indexBufferMemory);
		device.freeMemory(n.second.vertexBufferMemory);
		device.destroyBuffer(n.second.indexBuffer);
		device.destroyBuffer(n.second.vertexBuffer);
	}
	g_meshes.clear();
}

} // namespace graphics::mesh

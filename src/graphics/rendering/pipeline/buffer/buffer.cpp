#include "buffer.hpp"

#include "../../../utils.hpp"

#include <unordered_map>

namespace graphics::rendering::pipeline::buffer {

std::unordered_map<std::string, Buffer> g_buffers;

void terminate(const vk::Device &device) {
	for (const auto &n: g_buffers) {
		device.freeMemory(n.second.memory);
		device.destroyBuffer(n.second.buffer);
	}
	g_buffers.clear();
}

void create(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const char *id,
	uint64_t size,
	int isStorage
) {
	const auto storage = static_cast<bool>(isStorage);
	const auto ci = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(storage ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eUniformBuffer)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto buffer = device.createBuffer(ci);
	const auto memory = allocateBufferMemory(
		memoryProps,
		device,
		buffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	g_buffers.emplace(id, Buffer{storage, size, buffer, memory});
}

void update(const vk::Device &device, const char *id, const void *data) {
	const auto &buffer = g_buffers.at(id);
	copyDataToMemory(device, buffer.memory, data, buffer.size);
}

const Buffer &get(const char *id) {
	return g_buffers.at(id);
}

} // namespace graphics::rendering::pipeline::buffer

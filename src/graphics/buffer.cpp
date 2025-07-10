#include "buffer.hpp"

#include "utils.hpp"

#include <unordered_map>

namespace graphics::buffer {

std::unordered_map<std::string, Buffer> g_buffers;

void create(
	const vk::Device &device,
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const char *id,
	uint64_t size,
	int isStorage
) {
	const auto ci = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(isStorage ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eUniformBuffer)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto buffer = device.createBuffer(ci);
	const auto memory = utils::allocateMemory(
		device,
		memoryProps,
		buffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	g_buffers.emplace(id, Buffer{size, buffer, memory});
}

void update(const vk::Device &device, const char *id, const void *data) {
	const auto &buffer = g_buffers.at(id);
	const auto p = device.mapMemory(buffer.memory, 0, buffer.size);
	memcpy(p, data, buffer.size);
	device.unmapMemory(buffer.memory);
}

const Buffer &get(const std::string &id) {
	return g_buffers.at(id);
}

void terminate(const vk::Device &device) {
	for (const auto &n: g_buffers) {
		device.freeMemory(n.second.memory);
		device.destroyBuffer(n.second.buffer);
	}
	g_buffers.clear();
}

} // namespace graphics::buffer

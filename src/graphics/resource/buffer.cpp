#include "buffer.hpp"

#include "../../error/error.hpp"
#include "../core/core.hpp"

#include <format>
#include <unordered_map>

namespace graphics::resource {

Buffer::Buffer(uint64_t size, bool isStorage, bool isHostCoherent):
	_isStorage(isStorage),
	_size(static_cast<vk::DeviceSize>(size)),
	_buffer(core::device().createBufferUnique(
		vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(_isStorage ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eUniformBuffer)
			.setSharingMode(vk::SharingMode::eExclusive)
	)),
	_memory(allocateMemory(
		_buffer.get(),
		isHostCoherent
			? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			: vk::MemoryPropertyFlagBits::eHostVisible
	))
{}

std::unordered_map<std::string, Buffer> g_buffers;

void destroyAllBuffers() noexcept {
	g_buffers.clear();
}

void addBuffer(const std::string &id, uint64_t size, bool isStorage, bool isHostCoherent) {
	if (g_buffers.contains(id)) {
		throw std::format("buffer '{}' already created.", id);
	}
	g_buffers.try_emplace(id, size, isStorage, isHostCoherent);
}

void destroyBuffer(const std::string &id) noexcept {
	if (g_buffers.contains(id)) {
		g_buffers.erase(id);
	}
}

const Buffer &getBuffer(const std::string &id) {
	return error::at(g_buffers, id, "buffers");
}

} // namespace graphics::resource

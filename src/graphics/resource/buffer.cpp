#include "buffer.hpp"

#include "../../error/error.hpp"
#include "../core/core.hpp"

#include <format>

namespace graphics::resource {

Buffer::Buffer(uint64_t size, bool isStorage):
	_isStorage(isStorage),
	_size(static_cast<vk::DeviceSize>(size)),
	_buffer(core::device().createBuffer(
		vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(_isStorage ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eUniformBuffer)
			.setSharingMode(vk::SharingMode::eExclusive)
	)),
	_memory(allocateBufferMemory(_buffer, vk::MemoryPropertyFlagBits::eHostVisible))
{}

Buffer::~Buffer() {
	core::device().free(_memory);
	core::device().destroy(_buffer);
}

std::unordered_map<std::string, Buffer> g_buffers;

void destroyAllBuffers() noexcept {
	g_buffers.clear();
}

void addBuffer(const std::string &id, uint64_t size, bool isStorage) {
	if (g_buffers.contains(id)) {
		throw std::format("buffer '{}' already created.", id);
	}
	g_buffers.try_emplace(id, size, isStorage);
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

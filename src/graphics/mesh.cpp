#include "mesh.hpp"

#include "../asset/asset.hpp"
#include "../config/config.hpp"
#include "../error/error.hpp"
#include "utils.hpp"

namespace graphics {

const std::span<const unsigned char> getVerticesData(const std::string &id) {
	const auto vertices = error::at(config::config().meshes, id, "meshes").vertices;
	const auto assetId = error::at(config::config().assetMap, vertices, "assets");
	return asset::getAsset(assetId);
}

const std::span<const unsigned char> getIndicesData(const std::string &id) {
	const auto indices = error::at(config::config().meshes, id, "meshes").indices;
	const auto assetId = error::at(config::config().assetMap, indices, "assets");
	return asset::getAsset(assetId);
}

Mesh::Mesh(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const std::string &id
) :
	_iCount(static_cast<uint32_t>(getIndicesData(id).size() / sizeof(uint32_t))),
	_vb(device.createBuffer(
		vk::BufferCreateInfo()
			.setSize(static_cast<uint32_t>(getVerticesData(id).size()))
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
	)),
	_ib(device.createBuffer(
		vk::BufferCreateInfo()
			.setSize(static_cast<uint32_t>(getIndicesData(id).size()))
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst)
	)),
	_vbMemory(allocateBufferMemory(memoryProps, device, _vb, vk::MemoryPropertyFlagBits::eDeviceLocal)),
	_ibMemory(allocateBufferMemory(memoryProps, device, _ib, vk::MemoryPropertyFlagBits::eDeviceLocal))
{
	uploadBuffer(
		memoryProps,
		device,
		queue,
		_vb,
		static_cast<const void *>(getVerticesData(id).data()),
		getVerticesData(id).size(),
		vk::PipelineStageFlagBits::eVertexShader
	);
	uploadBuffer(
		memoryProps,
		device,
		queue,
		_ib,
		static_cast<const void *>(getIndicesData(id).data()),
		getIndicesData(id).size(),
		vk::PipelineStageFlagBits::eVertexShader
	);
}

} // namespace graphics

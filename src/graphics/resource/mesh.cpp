#include "mesh.hpp"

#include "../../asset/asset.hpp"
#include "../../config/config.hpp"
#include "../../error/error.hpp"
#include "../core/core.hpp"
#include "../utils.hpp"

namespace graphics::resource {

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

Mesh::Mesh(const std::string &id):
	_id(id),
	_iCount(static_cast<uint32_t>(getIndicesData(id).size() / sizeof(uint32_t))),
	_vb(core::device().createBuffer(
		vk::BufferCreateInfo()
			.setSize(static_cast<uint32_t>(getVerticesData(id).size()))
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
	)),
	_ib(core::device().createBuffer(
		vk::BufferCreateInfo()
			.setSize(static_cast<uint32_t>(getIndicesData(id).size()))
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst)
	)),
	_vbMemory(allocateBufferMemory(_vb, vk::MemoryPropertyFlagBits::eDeviceLocal)),
	_ibMemory(allocateBufferMemory(_ib, vk::MemoryPropertyFlagBits::eDeviceLocal))
{
	uploadBuffer(
		_vb,
		static_cast<const void *>(getVerticesData(id).data()),
		getVerticesData(id).size(),
		vk::PipelineStageFlagBits::eVertexShader
	);
	uploadBuffer(
		_ib,
		static_cast<const void *>(getIndicesData(id).data()),
		getIndicesData(id).size(),
		vk::PipelineStageFlagBits::eVertexShader
	);
}

Mesh::~Mesh() {
	core::device().free(_ibMemory);
	core::device().free(_vbMemory);
	core::device().destroy(_ib);
	core::device().destroy(_vb);
}

std::unordered_map<std::string, Mesh> g_meshes;

void destroyAllMeshes() noexcept {
	g_meshes.clear();
}

void addMesh(const std::string &id) {
	if (g_meshes.contains(id)) {
		throw std::format("mesh '{}' already created.", id);
	}
	g_meshes.emplace(id, Mesh(id));
}

void destroyMesh(const std::string &id) noexcept {
	if (g_meshes.contains(id)) {
		g_meshes.erase(id);
	}
}

const Mesh &getMesh(const std::string &id) {
	return error::at(g_meshes, id, "meshes");
}

} // namespace graphics::resource

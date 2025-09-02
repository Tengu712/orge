#include "mesh.hpp"

#include "utils.hpp"

namespace config {

MeshConfig::MeshConfig(const YAML::Node &node): vertices(s(node, "vertices")), indices(s(node, "indices")) {
	checkUnexpectedKeys(node, {"id", "vertices", "indices"});
}

std::unordered_map<std::string, MeshConfig> parseMeshConfigs(const YAML::Node &node) {
	std::unordered_map<std::string, MeshConfig> meshes;
	for (const auto &n: node["meshes"]) {
		const auto id = s(n, "id");
		if (meshes.contains(id)) {
			throw std::format("config error: mesh '{}' duplicated.", id);
		}
		meshes.emplace(id, MeshConfig(n));
	}
	return meshes;
}

} // namespace

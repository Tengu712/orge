#include "dependency.hpp"

#include "utils.hpp"

namespace config {

std::vector<vk::SubpassDependency> parseDependencies(
	const YAML::Node &node,
	const std::unordered_map<std::string, uint32_t> &subpassMap
) {
	std::vector<vk::SubpassDependency> dependencies;
	if (node["dependencies"]) {
		for (const auto &n: node["dependencies"]) {
			dependencies.emplace_back(
				subpassMap.at(s(n, "src")),
				subpassMap.at(s(n, "dst")),
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eAllCommands,
				vk::AccessFlagBits::eMemoryWrite,
				vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite
			);
		}
	}
	return dependencies;
}

} // namespace config

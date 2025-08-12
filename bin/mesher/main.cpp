#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

void fromYaml(const std::string &path, const std::string &filename) {
	const auto node = YAML::LoadFile(path);

	if (!node["vertices"]) {
		throw std::runtime_error("'vertices' not found.");
	}
	if (!node["vertices"].IsSequence()) {
		throw std::runtime_error("'vertices' must be a sequence.");
	}

	std::vector<float> vertices;
	for (const auto &n: node["vertices"]) {
		if (!n.IsMap()) {
			throw std::runtime_error("invalid element found in 'vertices'.");
		}
		for (const auto &m: n) {
			try {
				auto a = m.second.as<std::vector<float>>();
				vertices.insert(vertices.end(), std::make_move_iterator(a.begin()), std::make_move_iterator(a.end()));
			} catch (...) {
				throw std::runtime_error(std::format("'{}' must be float sequence.", m.first.as<std::string>()));
			}
		}
	}

	std::ofstream meshv(filename + ".mesh.v", std::ios::binary);
	if (!meshv) {
		throw std::runtime_error(std::format("failed to create '{}.mesh.v'.", filename));
	}
	meshv.write(reinterpret_cast<const char *>(vertices.data()), sizeof(float) * vertices.size());

	if (!node["indices"]) {
		throw std::runtime_error("'indices' not found.");
	}
	if (!node["indices"].IsSequence()) {
		throw std::runtime_error("'indices' must be a sequence.");
	}

	std::vector<uint32_t> indices;
	for (const auto &n: node["indices"]) {
		if (!n.IsMap()) {
			throw std::runtime_error("invalid element found in 'indices'.");
		}
		for (const auto &m: n) {
			try {
				auto a = m.second.as<std::vector<uint32_t>>();
				indices.insert(indices.end(), std::make_move_iterator(a.begin()), std::make_move_iterator(a.end()));
			} catch (...) {
				throw std::runtime_error(std::format("'{}' must be unsigned int sequence.", m.first.as<std::string>()));
			}
		}
	}

	std::ofstream meshi(filename + ".mesh.i", std::ios::binary);
	if (!meshi) {
		throw std::runtime_error(std::format("failed to create '{}.mesh.i'.", filename));
	}
	meshi.write(reinterpret_cast<const char *>(indices.data()), sizeof(uint32_t) * indices.size());
}

void dispatch(const std::string &path) {
	const std::filesystem::path p(path);
	const auto filename = p.stem().string();
	const auto extension = p.extension().string();

	if (extension == ".yml" || extension == ".YML" || extension == ".yaml" || extension == ".YAML") {
		fromYaml(path, filename);
	} else {
		throw std::runtime_error(std::format("the extension '{}' is not supported.", extension));
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "usage: mesher <file>" << std::endl;
		return 1;
	}

	try {
		dispatch(argv[1]);
	} catch (const YAML::Exception &e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "failed to create mesh binaries." << std::endl;
		return 1;
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "failed to create mesh binaries." << std::endl;
		return 1;
	}

	return 0;
}

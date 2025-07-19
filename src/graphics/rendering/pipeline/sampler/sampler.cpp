#include "sampler.hpp"

#include <unordered_map>

namespace graphics::rendering::pipeline::sampler {

std::unordered_map<std::string, vk::Sampler> g_samplers;

void terminate(const vk::Device &device) {
	for (auto &n: g_samplers) {
		device.destroySampler(n.second);
	}
	g_samplers.clear();
}

void create(const vk::Device &device, const char *id, int linearMagFilter, int linearMinFilter, int repeat) {
	const auto ci = vk::SamplerCreateInfo()
		.setMagFilter(linearMagFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
		.setMinFilter(linearMinFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setAddressModeU(repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge)
		.setMaxLod(vk::LodClampNone);
	const auto sampler = device.createSampler(ci);
	g_samplers.emplace(id, sampler);
}

void destroy(const vk::Device &device, const char *id) {
	if (!g_samplers.contains(id)) {
		return;
	}
	auto &n = g_samplers.at(id);
	device.destroySampler(n);
	g_samplers.erase(id);
}

const vk::Sampler &get(const char *id) {
	return g_samplers.at(id);
}

} // namespace graphics::rendering::pipeline::sampler

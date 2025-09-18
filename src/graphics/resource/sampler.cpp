#include "sampler.hpp"

#include "../../error/error.hpp"
#include "../core/core.hpp"

#include <unordered_map>

namespace graphics::resource {

std::unordered_map<std::string, vk::UniqueSampler> g_samplers;

void destroyAllSamplers() noexcept {
	g_samplers.clear();
}

void addSampler(const std::string &id, bool linearMagFilter, bool linearMinFilter, bool repeat) {
	if (g_samplers.contains(id)) {
		throw std::format("sampler '{}' already created.", id);
	}
	g_samplers.emplace(id, core::device().createSamplerUnique(
		vk::SamplerCreateInfo()
			.setMagFilter(linearMagFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
			.setMinFilter(linearMinFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge)
			.setMaxLod(vk::LodClampNone)
	));
}

void destroySampler(const std::string &id) noexcept {
	if (g_samplers.contains(id)) {
		g_samplers.erase(id);
	}
}

const vk::Sampler &getSampler(const std::string &id) {
	return error::at(g_samplers, id, "samplers").get();
}

} // namespace graphics::resource

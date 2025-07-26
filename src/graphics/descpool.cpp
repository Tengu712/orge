#include "descpool.hpp"

#include "../config/config.hpp"

namespace graphics {

vk::DescriptorPool createDescriptorPool(const vk::Device &device) {
	// 集計
	uint32_t maxSets = 0;
	std::unordered_map<config::DescriptorType, uint32_t> sizesMap;
	for (const auto &n: config::config().pipelines) {
		for (const auto &m: n.descSets) {
			maxSets += m.count;

			std::unordered_map<config::DescriptorType, uint32_t> map;
			for (const auto &b: m.bindings) {
				if (!map.contains(b.type)) {
					map.emplace(b.type, 0);
				}
				map[b.type] += b.count;
			}

			for (const auto &[k, v]: map) {
				sizesMap[k] += v * m.count;
			}
		}
	}

	// ディスクリプタセットが不要ならディスクリプタプールも不要
	if (maxSets == 0) {
		return nullptr;
	}

	// マップからベクタへ
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (const auto &[k, v]: sizesMap) {
		poolSizes.emplace_back(
			k == config::DescriptorType::Texture
				? vk::DescriptorType::eSampledImage
				: k == config::DescriptorType::Sampler
				? vk::DescriptorType::eSampler
				: k == config::DescriptorType::UniformBuffer
				? vk::DescriptorType::eUniformBuffer
				: k == config::DescriptorType::StorageBuffer
				? vk::DescriptorType::eStorageBuffer
				: k == config::DescriptorType::InputAttachment
				? vk::DescriptorType::eInputAttachment
				: throw,
			v
		);
	}

	// 作成
	const auto ci = vk::DescriptorPoolCreateInfo()
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(maxSets)
		.setPoolSizes(poolSizes);
	return device.createDescriptorPool(ci);
}

} // namespace graphics

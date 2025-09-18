#include "descpool.hpp"

#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../core/core.hpp"

namespace graphics::resource {

std::optional<vk::UniqueDescriptorPool> g_descpool;

void initializeDescriptorPool() {
	if (g_descpool) {
		throw "descriptor pool already initialized.";
	}

	// 集計
	uint32_t maxSets = 0;
	std::unordered_map<vk::DescriptorType, uint32_t> sizesMap;
	const auto fontCount = config::config().fonts.size();

	// グラフィックスパイプライン用のディスクリプタセットを追加
	for (const auto &[_, n]: config::config().renderPasses) {
		for (const auto &m: n.subpasses) {
			for (const auto &o: m.pipelines) {
				// テキストレンダリングパイプライン
				if (o == "@text@") {
					maxSets += 2;
					sizesMap[vk::DescriptorType::eStorageBuffer] += 1;
					sizesMap[vk::DescriptorType::eSampledImage] += static_cast<uint32_t>(fontCount);
					sizesMap[vk::DescriptorType::eSampler] += 1;
					continue;
				}

				// 一般パイプライン
				const auto &plconfig = config::config().pipelines.at(o);
				for (const auto &d: plconfig.descSets) {
					maxSets += d.count;

					std::unordered_map<config::DescriptorType, uint32_t> map;
					for (const auto &b: d.bindings) {
						const auto t = config::convertDescriptorType(b.type);
						sizesMap[t] += b.count * d.count;
					}
				}
			}
		}
	}

	// コンピュートパイプライン用のディスクリプタセットを追加
	for (const auto &[_, n]: config::config().computePipelines) {
		for (const auto &m: n.descSets) {
			maxSets += m.count;

			std::unordered_map<config::DescriptorType, uint32_t> map;
			for (const auto &b: m.bindings) {
				const auto t = config::convertComputeDescriptorType(b.type);
				sizesMap[t] += b.count * m.count;
			}
		}
	}

	// ディスクリプタセットが不要ならディスクリプタプールも不要
	if (maxSets == 0) {
		return;
	}

	// マップからベクタへ
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (const auto &[k, v]: sizesMap) {
		poolSizes.emplace_back(k, v);
	}

	// 作成
	const auto ci = vk::DescriptorPoolCreateInfo()
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(maxSets)
		.setPoolSizes(poolSizes);
	g_descpool = core::device().createDescriptorPoolUnique(ci);
}

void destroyDescriptorPool() noexcept {
	if (g_descpool) {
		g_descpool.reset();
	}
}

const vk::DescriptorPool &descpool() {
	if (g_descpool) {
		return g_descpool.value().get();
	} else {
		throw "descriptor pool not initialized.";
	}
}

} // namespace graphics::resource
